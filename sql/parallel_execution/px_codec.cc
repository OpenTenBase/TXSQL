#include "px_codec.h"
#include "px_mq.h"
#include "px_exchange_channel.h"
#include "sql/item.h"
#include "sql/sql_executor.h"
#include "sql/log.h"
#include "px_executor.h"
#include "px.h"

#define PX_HIDDEN_FIELD_COUNT 4
static char bool_item_field[8] = {0, 0, 0, 1, 1, 0, 1, 1};

char *const_item_and_field_flag(uint value) {
  assert(value < 4);
  return bool_item_field + 2 * value;
}

PX_compact_codec::PX_compact_codec(THD *thd, bool use_item, Temp_table_param *temp_table_param)
    : m_thd(thd), m_use_item(use_item), m_temp_table_param(temp_table_param) {}

PX_compact_codec::~PX_compact_codec() {
  if (m_compact_row) {
    destroy(m_compact_row);
  }

  if (m_skip_array) {
    destroy(m_skip_array);
  }

  if (m_skip_flag) {
    destroy(m_skip_flag);
  }
}

/**
  Initialize the members.

  @return 0 success, 1 fails
*/
int PX_compact_codec::init(mem_root_deque<Item *> *items, std::vector<Field *> *fields) {
  THD *thd = get_thd();
  assert(thd);
  size_t field_size = m_use_item ? items->size() : fields->size();
  m_items = items;
  m_fields = fields;
  m_compact_row = new (thd->mem_root) PX_field_data[field_size + PX_HIDDEN_FIELD_COUNT];
  DBUG_EXECUTE_IF("px_codec_init_error", {
    if (m_compact_row) destroy(m_compact_row);
    m_compact_row = nullptr;
  });

  if (!m_compact_row) goto oom;

  /*
    m_skip_array is the encoded bool array for const_item and null field.
    Each item use two bool in m_skip_array. The first bool indicates the
    item is const string item, the sencond bool indicates the result field
    of the item is null field. So, There are four cases:
      (0, 0)   =>  NOT_CONST_ITEM & NON_NULL_FIELD
      (0, 1)   =>  NOT_CONST_ITEM & NULL_FIELD
      (1, 0)   =>  CONST_ITEM & NON_NULL_FIELD
      (1, 1)   =>  CONST_ITEM & NULL_FIELD
    m_skip_flag use tow bits to encoded a item. After all items has been
    checked, build m_skip_flag according to m_skip_array.
  */
  m_skip_array = new (thd->mem_root) bool[2 * field_size];
  if (!m_skip_array) goto oom;
  m_skip_flag = new (thd->mem_root) char[field_size / PX_HIDDEN_FIELD_COUNT + 2];
  if (!m_skip_flag) goto oom;

  return 0;

oom:
  my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "PX_compact_codec::init()");
  return 1;
}

/**
  Reset for next encode.
*/
void PX_compact_codec::reset() {
  null_len = 0;
  total_copy_bytes = 0;
  null_num = 0;
}

/**
  Encode fields or items into compact row format in memory truncks.

  @return 0 success, 1 fails.
*/
int PX_compact_codec::encode(std::vector<PX_iovec> &memory_trunks) {
  assert(memory_trunks.empty());

  PX_PRINT_DEBUG("encode %lu fields", m_use_item ? m_items->size() : m_fields->size());

  reset();
  if (m_use_item) {
    if (compact_items()) return 1;
  } else {
    if (compact_fields()) return 1;
  }

  /*
    Make the m_skip_flag according to to m_skip_array.
    Each field use 2 bits to indicate its const/null flag.
    So 4 field use 1 byte in m_skip_flag and the first byte
    is reserved for the header(4 fields).
  */
  null_len = ((null_num % 8 == 0) ? null_num / 8 : null_num / 8 + 1) + 1;
  memset(m_skip_flag, 0, null_len);

  uint i, j;
  for (i = 0; i < null_num; i++) {
    if (m_skip_array[i]) {
      j = (i >> 3) + 1;
      m_skip_flag[j] += 1 << (7 - (i & 7));
    }
  }

  m_compact_row[3].m_ptr = (uchar *)m_skip_flag;
  m_compact_row[3].m_len = null_len;
  total_copy_bytes += null_len;

  m_compact_row[2].m_ptr = (uchar *)&null_len;
  m_compact_row[2].m_len = 2;
  total_copy_bytes += 2;

  /* The m_encoded_row_data[1] is reserved for stable output. */
  m_compact_row[1].m_need_send = false;

  m_compact_row[0].m_ptr = (uchar *)&total_copy_bytes;
  m_compact_row[0].m_len = 4;

  auto compact_fields_size = m_use_item ? (m_items->size() + PX_HIDDEN_FIELD_COUNT) :
      (m_fields->size() + PX_HIDDEN_FIELD_COUNT);
  try {
    memory_trunks.reserve(compact_fields_size);
    for (uint idx = 0; idx < compact_fields_size; ++idx) {
      if (!m_compact_row[idx].m_need_send) {
      continue;
      } else if (!m_compact_row[idx].m_packlength) {
        PX_iovec fld;
        fld.data = (const char*)m_compact_row[idx].m_ptr;
        fld.len = m_compact_row[idx].m_len;
        memory_trunks.push_back(fld);
      } else {
        PX_iovec fld_length, fld_data;
        fld_length.data = (const char*)m_compact_row[idx].m_ptr;
        fld_length.len = m_compact_row[idx].m_packlength;

        void **blob_pointer = (void **)(m_compact_row[idx].m_ptr + m_compact_row[idx].m_packlength);
        fld_data.data = (const char*)(*blob_pointer);
        fld_data.len = m_compact_row[idx].m_len;

        memory_trunks.push_back(fld_length);
        memory_trunks.push_back(fld_data);
      }
    }
  } catch (std::exception &e) {
    PX_PRINT_ERROR("bad alloc in PX_compact_codec::encode");
    return 1;
  }

  return 0;
}

bool PX_compact_codec::compact_items() {
  assert(m_use_item && m_items);
  size_t fields_idx = PX_HIDDEN_FIELD_COUNT;
  Field *result_field = nullptr;

  for (Item *item : *m_items) {
    /*
      Skip the send of const items (except FIELD_ITEM) and items that are of
      type NULL_ITEM, STRING_ITEM in compact row format. Const items that are of
      type FIELD_ITEM should be sent, because the sender and receiver created
      temporary tables, respectively, the item is not const on the Receiver.
    */
    assert(fields_idx < m_items->size() + PX_HIDDEN_FIELD_COUNT);
    if ((item->const_item() && item->type() != Item::FIELD_ITEM) ||
        item->type() == Item::NULL_ITEM || item->type() == Item::STRING_ITEM) {
      assert(item->const_item() || item->basic_const_item());

      m_skip_array[null_num++] = 1;
      m_skip_array[null_num++] = 0;
      m_compact_row[fields_idx++].m_need_send = false;
      continue;
    }

    /*
      1)For Item_field, sent the field directly because there is no need
      to copy it to parallel execution sender tmp table.
      2)For Item_func, write the result to parallel execution sender tmp
      table by save_in_field firstly.
    */
    if (item->type() == Item::FIELD_ITEM) {
      result_field = down_cast<Item_field *>(item)->field;
    } else {
      result_field = item->get_result_field();
      const type_conversion_status ret =
          item->save_in_field(result_field, true);
      // TODO: Push warning to coordinator
      if (ret != TYPE_OK && ret != TYPE_NOTE_TIME_TRUNCATED &&
          ret != TYPE_NOTE_TRUNCATED) {
        PX_PRINT_ERROR("compact item %s got conversion status %d",
                       item->full_name(), (int)ret);
        return true;
      }
    }

    m_skip_array[null_num++] = 0;
    m_skip_array[null_num++] = result_field->is_null() ? 1 : 0;

    /* Skip the send of null field. */
    if (m_skip_array[null_num - 1]) {
      m_compact_row[fields_idx++].m_need_send = false;
      continue;
    }

    uint32 field_length = make_compact_field(result_field, &m_compact_row[fields_idx]);
    if (field_length == UINT32_MAX) {
      return true;
    }
    total_copy_bytes += field_length;
    fields_idx++;
  }

  return false;
}

bool PX_compact_codec::compact_fields() {
  assert(!m_use_item && !m_items);
  /*
    Copy fields and calculate functions to the tmp table of exchange.
  */
  if (m_temp_table_param && copy_fields_and_funcs(m_temp_table_param, get_thd())) {
    return true; /* purecov: inspected */
  }

  size_t fields_idx = PX_HIDDEN_FIELD_COUNT;
  for (Field *field : *m_fields) {
    assert(fields_idx < m_fields->size() + PX_HIDDEN_FIELD_COUNT);
    if (MYSQL_TYPE_NULL == field->type()) {
      m_skip_array[null_num++] = 1;
      m_skip_array[null_num++] = 0;
      m_compact_row[fields_idx++].m_need_send = false;
      continue;
    }

    m_skip_array[null_num++] = 0;
    m_skip_array[null_num++] = field->is_null() ? 1 : 0;

    /* Skip the send of null field. */
    if (m_skip_array[null_num - 1]) {
      m_compact_row[fields_idx++].m_need_send = false;
      continue;
    }

    uint32 field_length = make_compact_field(field, &m_compact_row[fields_idx]);
    if (field_length == UINT32_MAX) {
      return true;
    }
    total_copy_bytes += field_length;
    fields_idx++;
  }
  return false;
}

uint32 PX_compact_codec::make_compact_field(Field *field, PX_field_data *px_field) {
  switch (field->type()) {
   case MYSQL_TYPE_BOOL:
   case MYSQL_TYPE_DECIMAL:
   case MYSQL_TYPE_TINY:
   case MYSQL_TYPE_SHORT:
   case MYSQL_TYPE_LONG:
   case MYSQL_TYPE_FLOAT:
   case MYSQL_TYPE_DOUBLE:
   case MYSQL_TYPE_NULL:
   case MYSQL_TYPE_TIMESTAMP:
   case MYSQL_TYPE_LONGLONG:
   case MYSQL_TYPE_INT24:
   case MYSQL_TYPE_DATE:
   case MYSQL_TYPE_TIME:
   case MYSQL_TYPE_DATETIME:
   case MYSQL_TYPE_YEAR:
   case MYSQL_TYPE_NEWDATE:

   case MYSQL_TYPE_TIMESTAMP2:
   case MYSQL_TYPE_DATETIME2:
   case MYSQL_TYPE_TIME2:
   case MYSQL_TYPE_TYPED_ARRAY:

   case MYSQL_TYPE_NEWDECIMAL:
   case MYSQL_TYPE_ENUM:
   case MYSQL_TYPE_SET:
   case MYSQL_TYPE_STRING:{
     px_field->m_ptr = field->field_ptr();
     px_field->m_len = field->pack_length();
     break;
   }

   case MYSQL_TYPE_BIT: {
     // Not supported yet.
     assert(0);
     break;
   }

   case MYSQL_TYPE_VARCHAR:
   case MYSQL_TYPE_VAR_STRING: {
     Field_varstring *from = static_cast<Field_varstring *>(field);
     px_field->m_ptr = from->field_ptr();
     px_field->m_len = from->get_length_bytes() + from->data_length();
     break;
   }

   case MYSQL_TYPE_JSON:
   case MYSQL_TYPE_TINY_BLOB:
   case MYSQL_TYPE_MEDIUM_BLOB:
   case MYSQL_TYPE_LONG_BLOB:
   case MYSQL_TYPE_BLOB: {
     /*
      The lob field whose actual data length is less than
      txsql_parallel_execution_max_lob_size supports data exchange between parallel
      query exchange operators. If a lob field whose actual data length is greater
      than txsql_parallel_execution_max_lob_size is found during the exchange
      process, the execution of the parallel query will be terminated.
     */
      THD *thd = get_thd();
      assert(thd);
      Field_blob *from = static_cast<Field_blob *>(field);
      uint32 data_length = from->get_length();
      if (data_length > thd->variables.txsql_parallel_execution_max_lob_size) {
        PX_PRINT_ERROR("Found lob field whose data size [%d] exceeds the limit.", data_length);
        my_error(ER_PX_OUT_OF_LOB, MYF(0), data_length);
        return UINT32_MAX;
      }
      px_field->m_ptr = from->field_ptr();
      px_field->m_len = from->data_length();
      px_field->m_packlength = from->row_pack_length();
      break;
   }
   case MYSQL_TYPE_GEOMETRY: {
     assert(0);
     break;
   }
   default: {
     // MYSQL_TYPE_INVALID
     assert(0);
     break;
   }
  }

  px_field->m_need_send = true;
  return px_field->m_len + px_field->m_packlength;
}

/**
  Decode a compact row into fields.

  @return 0 success 1 fails.
*/
int PX_compact_codec::decode(uchar *data, Size len) {
  /*
    In this stage, just copy each item to the
    PX receiver tmp table. It is better to
    re-adjust the ptr of fields to the compact
    row directly.
  */
  auto size_field = m_fields->size();
  // len
  assert(*(uint32*)data + sizeof(uint32) == len);
  data += sizeof(uint32);
  // bitmap_len
  uint null_len = *(uint16 *)data;
  data = data + sizeof(uint16);
  uchar *null_flag = (uchar *)data;
  bool is_null_field = false;
  bool is_const_item = false;
  uint bit_value;
  char *status_flag = nullptr;
  uint null_offset = 0;
  // field
  uint ptr_offset = null_len;
  Field *field = nullptr;

  PX_PRINT_DEBUG("decode %lu bytes into %lu fields", len, size_field);

  uint i = 0, j;
  /*
    For a field, the first phase is check the field is result_field
    of a CONST_ITEM or a NULL_FIELD according the null_flag.
  */
  for (; i < size_field; i++) {
    field = m_fields->at(i);
    assert(field);
    /*
      1) Determine whether it is a result field of CONST_ITEM
      or NULL_FIELD.
      2.1) Fill data into table->record[0] only when the field
      is a result field of NOT_CONST_ITEM & NOT_NULL_FIELD.
      2.2) Otherwise, just set null for NULL_FIELD.
    */
    j = (null_offset >> 3) + 1;
    assert((null_offset & 1) == 0);
    bit_value = (null_flag[j] >> (6 - (null_offset & 7))) & 3;
    status_flag = const_item_and_field_flag(bit_value);
    is_const_item = *status_flag;
    is_null_field = *(status_flag + 1);

    if (!is_const_item) {
      if (decompact_field(field, is_null_field, data, ptr_offset) ||
          DBUG_EVALUATE_IF("px_decode_error", true, false)) {
        PX_PRINT_ERROR("decompact field in PX_receiver fails.");
        my_error(ER_PX_DECODE_ERROR, MYF(0));
        return 1;
      }
    }

    null_offset += 2;
  }

  return 0;
}

/**
  Set field by null flag and data from compact row, and move offset.

  @param[out]    field       Field to store result
  @param         data        The compact row buffer
  @param[inout]  ptr_offset  Current offset in the row buffer

  @return 0 success, 1 fail.
*/
int PX_compact_codec::decompact_field(
    Field *field, bool is_null, uchar *data, uint &ptr_offset) {
  if (is_null) {
    if (field->is_nullable() || field->is_tmp_nullable()) {
      field->set_null();
    } else {
      field->table->set_null_row();
    }
    return 0;
  }

  if (field->is_nullable() || field->is_tmp_nullable()) {
    field->set_notnull();
  } else {
    field->table->reset_null_row();
  }

  int result = 0;
  switch (field->type()) {
   case MYSQL_TYPE_BOOL:
   case MYSQL_TYPE_DECIMAL:
   case MYSQL_TYPE_TINY:
   case MYSQL_TYPE_SHORT:
   case MYSQL_TYPE_LONG:
   case MYSQL_TYPE_FLOAT:
   case MYSQL_TYPE_DOUBLE:
   case MYSQL_TYPE_NULL:
   case MYSQL_TYPE_TIMESTAMP:
   case MYSQL_TYPE_LONGLONG:
   case MYSQL_TYPE_INT24:
   case MYSQL_TYPE_DATE:
   case MYSQL_TYPE_TIME:
   case MYSQL_TYPE_DATETIME:
   case MYSQL_TYPE_YEAR:
   case MYSQL_TYPE_NEWDATE:

   case MYSQL_TYPE_TIMESTAMP2:
   case MYSQL_TYPE_DATETIME2:
   case MYSQL_TYPE_TIME2:
   case MYSQL_TYPE_TYPED_ARRAY:

   case MYSQL_TYPE_NEWDECIMAL:
   case MYSQL_TYPE_ENUM:
   case MYSQL_TYPE_SET:
   case MYSQL_TYPE_STRING:{
     uint pack_length = field->pack_length();
     memcpy(field->field_ptr(), &data[ptr_offset], pack_length);
     ptr_offset += pack_length;
     break;
   }

   case MYSQL_TYPE_BIT: {
     result = 1;
     // Not supported yet.
     assert(0);
     break;
   }

   case MYSQL_TYPE_VARCHAR:
   case MYSQL_TYPE_VAR_STRING: {
     Field_varstring *field_var = static_cast<Field_varstring *>(field);
     uint payload_length = (field_var->get_length_bytes() == 1)
         ? (uint)data[ptr_offset] : uint2korr(&data[ptr_offset]);
     uint pack_length = payload_length + field_var->get_length_bytes();
     memcpy(field_var->field_ptr(), &data[ptr_offset], pack_length);
     ptr_offset += pack_length;
     break;
   }

   case MYSQL_TYPE_JSON:
   case MYSQL_TYPE_TINY_BLOB:
   case MYSQL_TYPE_MEDIUM_BLOB:
   case MYSQL_TYPE_LONG_BLOB:
   case MYSQL_TYPE_BLOB: {
     Field_blob *field_blob = static_cast<Field_blob *>(field);
     uint pack_length = field_blob->row_pack_length();
     memcpy(field_blob->field_ptr(), &data[ptr_offset], pack_length);
     ptr_offset += pack_length;

     // set the pointer of blob data
     uchar *blob = (uchar *)field_blob->field_ptr() + pack_length;
     void *blob_data = &data[ptr_offset];
     memcpy(blob, &blob_data, sizeof(blob_data));

     assert(get_thd() && field_blob->data_length() <=
            get_thd()->variables.txsql_parallel_execution_max_lob_size);
     ptr_offset += field_blob->data_length();
     break;
   }
   case MYSQL_TYPE_GEOMETRY: {
     result = 1;
     // Not supported yet.
     assert(0);
     break;
   }
   default: {
     // MYSQL_TYPE_INVALID
     result = 1;
     assert(0);
     break;
   }
  }

  return result;
}

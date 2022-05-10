#include "sql/item.h"
#include "sql/sql_class.h"
#include "px_sender.h"
#include "px_exchange_info.h"
#include "px_exchange_channel.h"
#include "include/my_dbug.h"
#include "sql/query_result.h"
#include "sql/log.h"
#include "sql/sql_class.h"
#include "field_types.h"

#define PX_HIDDEN_FIELD_COUNT 4

PX_sender::PX_sender(THD *thd, uint sender_no, PX_exchange_info *pei,
                     unique_ptr_destroy_only<RowIterator> source, TABLE *table,
                     mem_root_deque<Item *> *send_fields,
                     mem_root_deque<Item *> *shuffle_key,
                     Temp_table_param *temp_table_param,
                     unique_ptr_destroy_only<RowIterator> table_path)
    : RowIterator(thd),
      m_thd(thd),
      m_sender_no(0),
      m_pei(pei),
      m_source(move(source)),
      m_table(table),
      m_send_fields(send_fields),
      m_reshuffle_key(shuffle_key),
      m_temp_table_param(temp_table_param),
      m_materialize(table_path),
      m_table_path(move(table_path)) {}

bool PX_sender::init() {
  assert(m_pei);
  m_sender_no = m_thd->task_executor_id;
  /*
    The init of sender contains three phases:
    1) register sender, create the execution context of sender for channel. 
    2) init sender, create the sender's backend handle.
    3) attach sender, set the countpart's handle for sender, this phase must
    execute after the register of receivers of corresponding channels.
  */
  if (m_pei->register_sender(m_thd, m_sender_no)) {
    return true;
  }

  if (m_pei->init_sender(m_sender_no)) {
    return true;
  }

  return false;
}

/**
  Set the worker_handle to sender.
  If attach success, which indicates the receiver has
  register to channel, so the sender can start to send
  data.

  @returns
    false  - if the sender attach success
    true  - if the receiver has not register.
*/
bool PX_sender::attach() {
  if (init()) return true;

  bool result = m_source->Init();
  if (result) goto err;

  if (m_materialize) {
    if (!m_table->is_created()) {
      if (instantiate_tmp_table(thd(), m_table)) {
        goto err;
      }
      empty_record(m_table);
    } else {
      m_table->file->ha_index_or_rnd_end();  // @todo likely unneeded => remove
      m_table->file->ha_delete_all_rows();
    }

    while (true) {
      int error = m_source->Read();
      if (error > 0 || thd()->is_error())
        goto err;
      else if (error < 0)
        break;
      else if (thd()->killed) {
        thd()->send_kill_message();
        goto err;
      }

      error = m_table->file->ha_write_row(m_table->record[0]);
      if (error == 0) {
        continue;
      }

      // create_ondisk_from_heap will generate error if needed.
      if (!m_table->file->is_ignorable_error(error)) {
        bool is_duplicate;
        if (create_ondisk_from_heap(thd(), m_table, error, true, true, &is_duplicate))
          goto err; /* purecov: inspected */
        // Table's engine changed; index is not initialized anymore.
        if (m_table->hash_field) m_table->file->ha_index_init(0, false);
        // if (!is_duplicate) ++*stored_rows;
      } else {
        // An ignorable error means duplicate key, ie. we deduplicated
        // away the row. This is seemingly separate from
        // check_unique_constraint(), which only checks hash indexes.
      }
    }
    if (m_table_path->Init()) goto err;
  }

  schedule_post(); // post for schedule the dfo pair.
  synchronize(); // synchronize the receiver and sender init.

  if (m_pei->attach_sender(m_sender_no)) {
    assert(0);
    return true;
  }

  for (Field **pfield = m_table->field; *pfield != nullptr; ++pfield) {
    Field *field = *pfield;
    if (bitmap_is_set(m_table->read_set, field->field_index()))
      m_fields.push_back(field);
  }

  return false;

 err:
  schedule_post(); // post for schedule the dfo pair.

  return true;
}

/**
  Send data to PX_receiver through exchange channel.
  There are mutli kinds of strategies for send:
  1) send row with compact format
  2) send row directly
  3) send csi chunk

  @return 0 for success, -1 for EOF and 1 for error
*/
int PX_sender::send() {
  /*
    The field counts is 4096 at most, so the null len is less than
    2 * 2^12(4096) / 8 = 2^10 bytes. We can use 2 bytes to store
    the skip flag len info.
  */
  int result = 0;
  uint16 null_len = 0;
  uint32 total_copy_bytes = 0;
  auto send_format = m_pei->format();

  if (!m_materialize) {
    result = m_source->Read();
    if (result != 0) return result;
    if (m_temp_table_param && copy_fields_and_funcs(m_temp_table_param, thd()))
      return true; /* purecov: inspected */
  } else {
    result = m_table_path->Read();
    if (result != 0) return result;
  }

  // For some reason, items might not store information in fields. (testcase
  // type_bit_innodb:80) we need to do this manually.
  // for (Item *item : *m_send_fields) {
  //   Field *result_field = item->get_result_field();
  //   if (item->const_item() && result_field) {
  //     item->save_in_field(result_field, true);
  //   }
  // }
  // for (Field **pfield = m_table->field; *pfield != nullptr; ++pfield) {
  //   Field *field = *pfield;
  // }

  switch (send_format) {
   case PX_COMPACT_ROW: {
     if (prepare_compact_row() || make_compact_row(null_len, total_copy_bytes)) {
       return 1;
     }

     if (send_compact_row()) {
       m_pei->detach_sender(m_sender_no);
       return 1;
     }

     break;
   }
   default:
     break;
  }

  return result;
}

bool PX_sender::send_compact_row() {
  auto compact_fields_size = m_fields.size() + PX_HIDDEN_FIELD_COUNT;
  std::vector<PX_iovec> out_fields;
  out_fields.reserve(compact_fields_size);

  for (uint i = 0; i < compact_fields_size; ++i) {
    if (!m_compact_row[i].m_need_send) {
      continue;
    } else {
      PX_iovec fld;
      fld.data = (const char*)m_compact_row[i].m_ptr;
      fld.len = m_compact_row[i].m_len;
      out_fields.push_back(fld);
    }
  }

  /*
    Find the channels should send data to firstly.
    Then send data.
    For PX_GATHER_EXCHANGE, the sender only need to
    send row to a gather receiver.
    For PX_RESHUFFLE_EXCHANGE, the sender should
    calculate the receiver no according to the
    m_reshuffle_key throush a hash function. Then
    send row to the receiver.
    For PX_BOARDCAST_EXCHANGE, the sender should
    send the row to every receivers.
  */
  std::vector<PX_exchange_channel *> channels;
  m_pei->get_sender_channel(m_sender_no, channels);
  PX_mq_result res;

  for (auto channel : channels) {
    assert(channel);

    if (m_pei->type() == PX_RESHUFFLE_EXCHANGE) {
      assert(m_reshuffle_key);

      uint receiver_no = cal_reshuffle_channel(m_reshuffle_key);

      if (m_pei->find_channel_no(m_sender_no, receiver_no) != channel->get_channel_no()) {
        continue;
      }
    }

    res = (PX_mq_result)channel->send_row(out_fields.data(), out_fields.size(), /*nowait=*/false);

    switch (res) {
     case PX_MQ_DETACHED:
     case PX_MQ_ERROR:
     case PX_MQ_INTERRUPTED: {
       goto error;
       break;
     }
     case PX_MQ_SUCCESS:
     case PX_MQ_WOULD_BLOCK:
       break;
     default:
       assert(0);
       break;
    }
  }

  return false;

error:
  sql_print_error("Send compact row to exchange channel fail!");
  return true;
}

/**
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
bool PX_sender::prepare_compact_row() {
  auto field_size = m_fields.size() + PX_HIDDEN_FIELD_COUNT;
  m_compact_row = new PX_field_data[field_size];
  m_skip_array = new bool[2 * m_fields.size()];
  m_skip_flag = new char[m_fields.size() / PX_HIDDEN_FIELD_COUNT + 2];

  if (!m_compact_row || !m_skip_array || !m_skip_flag) {
    my_error(ER_OUTOFMEMORY, MYF(0));
    sql_print_error("PX_sender::prepare_compact_row error!");
    return true;
  }

  return false;
}

bool PX_sender::make_compact_row(uint16 &null_len, uint32 &total_copy_bytes) {
  /* The send data can't be empty row. */
  // DBUG_ASSERT(m_send_fields->size());


  uint i, j;
  uint null_num = 0;
  // Field *result_field = nullptr;
  int fields_idx = PX_HIDDEN_FIELD_COUNT;

  // /*
  //   Skip the send of Item::NULL_ITEM or const_string_item
  //   in compact row format.
  // */
  // for (Item *item : *m_send_fields) {
  //   if (item->type() == Item::NULL_ITEM || item->type() == Item::STRING_ITEM) {
  //     DBUG_ASSERT(item->const_item() || item->basic_const_item());

  //     m_skip_array[null_num++] = 1;
  //     m_skip_array[null_num++] = 0;
  //     m_compact_row[fields_idx++].m_need_send = false;
  //     continue;
  //   }

  //   /*
  //     1)For Item_field, sent the field directly because there is no need
  //     to copy it to parallel execution sender tmp table.
  //     2)For Item_func, write the result to parallel execution sender tmp
  //     table by save_in_field firstly.
  //   */
  //   result_field = item->get_result_field();

  //   if (!result_field) {
  //     DBUG_ASSERT(item->type() == Item::FIELD_ITEM);
  //     result_field = down_cast<Item_field *>(item)->field;
  //   } else {
  //     if (item->save_in_field(result_field, true)) {
  //       return true;
  //     }
  //   }

  //   m_skip_array[null_num++] = 0;
  //   m_skip_array[null_num++] = result_field->is_null() ? 1 : 0;

  //   /* Skip the send of null field. */
  //   if (m_skip_array[null_num - 1]) {
  //     m_compact_row[fields_idx++].m_need_send = false;
  //     continue;
  //   }

  //   total_copy_bytes += make_compact_field(result_field, &m_compact_row[fields_idx]);
  //   fields_idx++;
  // }

  for (Field *field : m_fields) {
    // if (item->type() == Item::NULL_ITEM || item->type() == Item::STRING_ITEM) {
    //   DBUG_ASSERT(item->const_item() || item->basic_const_item());

    //   m_skip_array[null_num++] = 1;
    //   m_skip_array[null_num++] = 0;
    //   m_compact_row[fields_idx++].m_need_send = false;
    //   continue;
    // }
    if (MYSQL_TYPE_NULL == field->type()) {
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
    // result_field = item->get_result_field();

    // if (!result_field) {
    //   DBUG_ASSERT(item->type() == Item::FIELD_ITEM);
    //   result_field = down_cast<Item_field *>(item)->field;
    // } else {
    //   if (item->save_in_field(result_field, true)) {
    //     return true;
    //   }
    // }

    m_skip_array[null_num++] = 0;
    m_skip_array[null_num++] = field->is_null() ? 1 : 0;

    /* Skip the send of null field. */
    if (m_skip_array[null_num - 1]) {
      m_compact_row[fields_idx++].m_need_send = false;
      continue;
    }

    total_copy_bytes += make_compact_field(field, &m_compact_row[fields_idx]);
    fields_idx++;
  }

  /*
    Make the m_skip_flag according to to m_skip_array.
    Each field use 2 bits to indicate its const/null flag.
    So 4 field use 1 byte in m_skip_flag and the first byte
    is reserved for the header(4 fields).
  */
  null_len = ((null_num % 8 == 0) ? null_num / 8 : null_num / 8 + 1) + 1;
  memset(m_skip_flag, 0, null_len);

  for (i = 0; i < null_num; i++) {
    if (m_skip_array[i]) {
      j = (i >> 3) + 1;
      m_skip_flag[j] += 1 << (7 - (i & 7));
    }
  }

  /*
    Make the compact row hidden fields. There are 4
    hidden fields for a compact row.
    1) total_copy_bytes: the legnth of payload will be
    sent to exchange channel.
    2) handle::ref: the ref of the row used in stable
    output.
    3) null_len: the length of m_skip_flag in bytes.
    4) m_skip_flag: the bit array indicates a item
    is const_item or has a null result field in
    sender tmp table.
  */
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

  return false;
}

uint32 PX_sender::make_compact_field(Field *field, PX_field_data *px_field) {
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
   case MYSQL_TYPE_BLOB:
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
  return px_field->m_len;
}

uint PX_sender::cal_reshuffle_channel(mem_root_deque<Item *> *reshuffle_key) {
  // @TODO: the reshuffle has not yes supported!
  return 0;
}

void PX_sender::end() {
  if (m_compact_row) {
    delete [] m_compact_row;
    m_compact_row = nullptr;
  }

  if (m_skip_array) {
    delete [] m_skip_array;
    m_skip_array = nullptr;
  }

  if (m_skip_flag) {
    delete [] m_skip_flag;
    m_skip_flag = nullptr;
  }
}

void PX_sender::schedule_post() {
  if (m_pei->is_top_exchange()) m_pei->schedule_senders_post();
}

void PX_sender::synchronize() {
  m_role == SYN_KEY ? m_pei->exchange_senders_post()
                    : m_pei->exchange_wait();
}

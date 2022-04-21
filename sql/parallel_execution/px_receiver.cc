#include "px_atomic.h"
#include "px_receiver.h"
#include "px_sender.h"
#include "px_exchange_info.h"
#include "px_mq.h"
#include "include/my_dbug.h"
#include "sql/log.h"
#include "sql/field.h"

#include "sql/sql_class.h"
#include "sql/sql_optimizer.h"

static char bool_item_field[8] = {0, 0, 0, 1, 1, 0, 1, 1};

char *const_item_and_field_flag(uint value) {
  assert(value < 4);
  return bool_item_field + 2 * value;
}

void SwitchSlice(JOIN *join, int slice_num) {
  if (-1 != slice_num && !join->ref_items[slice_num].is_null()) {
    join->set_ref_item_slice(slice_num);
  }
}

PX_receiver::PX_receiver(THD *thd, uint receiver_no, PX_exchange_info *pei,
  JOIN *join, unique_ptr_destroy_only<RowIterator> source,
  TABLE *table, int ref_slice)
    : RowIterator(thd),
      m_thd(thd),
      m_receiver_no(receiver_no),
      m_pei(pei),
      m_join(join),
      m_source(move(source)),
      m_table(table),
      m_ref_slice(ref_slice),
      m_fields() {}

PX_receiver::PX_receiver(THD *thd, uint receiver_no,
  PX_exchange_info *pei, unique_ptr_destroy_only<RowIterator> source, TABLE *table)
    : RowIterator(thd),
      m_thd(thd),
      m_receiver_no(receiver_no),
      m_pei(pei),
      m_source(move(source)),
      m_table(table) {}

/**
  Exchange receiver need three phases:
  1) register receiver
  2) init receiver
  3) attach receiver 
*/
bool PX_receiver::init() {
  assert(m_pei);

  THD *thd = get_thd();

  if (!thd->variables.cdb_parallel_execution_enabled && thd->lex->only_one_exchange()) {
    assert(m_source.get()->type() == PHY_PX_SEND);
    PX_sender *sender = static_cast<PX_sender*>(m_source->real_iterator());
    sender->init();
  }

  if (m_pei->register_receiver(get_thd(), m_receiver_no)) {
    return true;
  }

  if (m_pei->init_receiver(m_receiver_no)) {
    return true;
  }

  m_pei->get_receiver_channel(m_receiver_no, m_channels);
  m_active_channels = m_channels.size();

  // m_join = thd->lex->current_select()->join;
  if (m_join) m_input_slice = m_join->get_ref_item_slice();

  return false;
}

/**
  Set the worker_handle to receiver.
  If attach success, which indicates the sender has
  register to channel, so the receiver can start to
  receive data.

  @returns
    false  - if the receiver attach success
    true  - if the sender has not register.
*/
bool PX_receiver::attach() {
  THD *thd = get_thd();

  for (Field **pfield = m_table->field; *pfield != nullptr; ++pfield) {
    Field *field = *pfield;
    if (bitmap_is_set(m_table->read_set, field->field_index()))
      m_fields.push_back(field);
  }

  if (!thd->variables.cdb_parallel_execution_enabled && thd->lex->only_one_exchange()) {
    assert(m_source.get()->type() == PHY_PX_SEND);
    PX_sender *sender = static_cast<PX_sender*>(m_source->real_iterator());
    if (sender->attach()) return true;
  }

  if (m_pei->attach_receiver(m_receiver_no)) {
    assert(0);
    return true;
  }

  return false;
}

/**
  Read from exchange chennels responding to the receiver.
  
  @return true error occurs or killed, fail read success!
*/
bool PX_receiver::next() {
  // if (m_join->select_count) {
  //   // When true, UnqualifiedCountIterator would be used. This iterator directly
  //   // use join->fields, which is the last ref_slice in ref_items actually.
  //   SwitchSlice(m_join, m_ref_slice);
  // } else {
  if (m_join) SwitchSlice(m_join, m_input_slice);
  // }
  if (!thd()->variables.cdb_parallel_execution_enabled &&
      thd()->lex->only_one_exchange()) {
    assert(m_source.get()->type() == PHY_PX_SEND);
    PX_sender *sender = static_cast<PX_sender *>(m_source->real_iterator());
    if (-1 == sender->Read()) {
      m_pei->detach_sender(thd()->worker_id);
      sender->end();
      // TODO: only support one-stage parallel.
      sql_print_information("finish single thread mode receiver execution.");
    }
  }
  bool result = false;
  uchar *data = nullptr;
  Size msg_len = 0;

  if (m_pei->format() == PX_COMPACT_ROW) {
    result = read_compact_row((void **)&data, &msg_len);
  } else {
    // Has not support yet!
    assert(0);
  }

  /*
    When all channels read done or has error
    occur or receiver killed, detach the
    receiver from the channels.
  */
  if (result || m_thd->killed) {
    m_pei->detach_receiver(m_receiver_no);
    return true;
  }

  if (m_pei->format() == PX_COMPACT_ROW) {
    decompact_row(data, msg_len);
  }

  if (m_join) SwitchSlice(m_join, m_ref_slice);
  return result;
}

void PX_receiver::end() {

}

/**
  Read compact row from exchange channels in robin-hood
  manner. If current active channel is empty but has not
  detach, just move to next active channel to read row.

  @return false read success, true fail.
*/
bool PX_receiver::read_compact_row(void **datap, Size *len) {
  uint nvisited = 0;
  THD *thd = get_thd();

  /*
    Acquire the data according to the round-robin method.
    If can't read data from current channel, just read
    from next active channel.
  */
  while (!thd->is_killed()) {
    PX_exchange_channel *channel = m_channels[m_next_channel];
    PX_mq_result read_result = (PX_mq_result)channel->receive(datap, len, /*nowait=*/true);

    if (read_result == PX_MQ_SUCCESS) {
      return false;
    }

    if (read_result == PX_MQ_WOULD_BLOCK) {
      m_next_channel++;
      nvisited++;

      if (m_next_channel >= m_active_channels) {
        m_next_channel = 0;
      }

      if (nvisited >= m_active_channels) {
        m_pei->receiver_wait(m_receiver_no);
        nvisited = 0;
      }
    }

    if (read_result == PX_MQ_ERROR || read_result == PX_MQ_INTERRUPTED) {
      sql_print_error("Recevie data from a channel fail!");
      return true;
    }

    if (read_result == PX_MQ_DETACHED) {
      /*
        If the worker has detached from the channel, just
        remove the worker and move to next active channel.
        If all the worker has detached, return true.
      */
      m_active_channels--;

      if (m_active_channels == 0) {
        return true;
      }

      // Remove the read done channel.
      mqueue_mmove(m_next_channel, m_active_channels);

      if (m_next_channel >= m_active_channels) {
        m_next_channel = 0;
      }

      continue;
    }
  }

  return false;
}

/**
  when the exchange format of data between PX_sender and
  PX_receiver is PX_COMPACT_ROW, After read a compact row,
  we need to decompact it.

  @return: true if success, false if fail.
*/
bool PX_receiver::decompact_row(uchar *data, Size msg_len) {
  memset(m_table->record[0], 255, m_table->s->reclength);
  /*
    In this stage, just copy each item to the
    PX receiver tmp table. It is better to
    re-adjust the ptr of fields to the compact
    row directly.
  */
  auto size_field = m_fields.size();

  /*
    A compct row format without ref info just like follows:
    total_bytes | null_len  | null_flag| field   |     varchar field      |
    |-----4-----|-----2-----|-null_len-| pack_len| length_bytes + data_len|
  */
  // skip the data length.
  data += sizeof(uint32);

  uint null_len = *(uint16 *)data;
  data = data + sizeof(uint16);
  uchar *null_flag = (uchar *)data;

  bool is_null_field = false;
  bool is_const_item = false;
  uint bit_value;
  char *status_flag = nullptr;

  uint null_offset = 0;
  uint ptr_offset = null_len;
  Field *item_field = nullptr;

  uint i = 0, j;
  /*
    For a item, the first phase is check the field is result_field
    of a CONST_ITEM or a NULL_FIELD according the null_flag.

  */
  for (; i < size_field; i++) {
    item_field = m_fields[i];
    // Determine whether it is a CONST_ITEM or NULL_FIELD
    j = (null_offset >> 3) + 1;
    assert((null_offset & 1) == 0);
    bit_value = (null_flag[j] >> (6 - (null_offset & 7))) & 3;
    status_flag = const_item_and_field_flag(bit_value);
    is_const_item = *status_flag;
    is_null_field = *(status_flag + 1);

    /*
      Fill data into table->record[0] only when the field
      is a result of NOT_CONST_ITEM & NOT_NULL_FIELD.
      Otherwise, just set null for NULL_FIELD.
    */
    if (!is_const_item && !is_null_field) {
      decompact_field(item_field, data, ptr_offset);
    }

    if (!is_const_item) {
      if (is_null_field) {
        item_field->set_null();
      } else {
        item_field->set_notnull();
      }
    }

    null_offset += 2;
  }

  return true;
}

/**
  Fill the content of a field from the comapct data. 
  In compact format, a var-length field only contains
  the length_bytes and valid data, skip the padding
  data.
*/
void PX_receiver::decompact_field(Field *field, uchar *data, uint &ptr_offset) {
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
}

void PX_receiver::mqueue_mmove(uint next_channel, uint active_channels) {
  memmove(&m_channels[next_channel],
          &m_channels[next_channel + 1],
          sizeof(PX_exchange_channel *) * (active_channels - next_channel));
}

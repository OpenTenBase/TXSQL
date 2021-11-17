#include "sql/px_exchange.h"

#ifndef DBUG_OFF
/*
  Print a text, SQL-like record representation into dbug trace, `error.log`
  generally.

  Note: this function is a work in progress: at the moment
   - column read bitmap is ignored (can print garbage for unused columns)
   - there is no quoting
*/
static void dbug_print_record(TABLE *table, bool print_rowid, const char *str, uint slice = -1) {
  char buff[1024];
  Field **pfield;
  String tmp(buff, sizeof(buff), &my_charset_bin);
  DBUG_LOCK_FILE;

  fprintf(DBUG_FILE, "record: %s, %d (", str, slice);
  for (pfield = table->field; *pfield; pfield++)
    fprintf(DBUG_FILE, "%s%s", (*pfield)->field_name, (pfield[1]) ? ", " : "");
  fprintf(DBUG_FILE, ") = ");

  fprintf(DBUG_FILE, "(");
  for (pfield = table->field; *pfield; pfield++) {
    Field *field = *pfield;

    if (field->is_null()) {
      if (fwrite("NULL", sizeof(char), 4, DBUG_FILE) != 4) {
        goto unlock_file_and_quit;
      }
    }

    if (field->type() == MYSQL_TYPE_BIT)
      (void)field->val_int_as_str(&tmp, true);
    else
      field->val_str(&tmp);

    if (fwrite(tmp.ptr(), sizeof(char), tmp.length(), DBUG_FILE) !=
        tmp.length()) {
      goto unlock_file_and_quit;
    }

    if (pfield[1]) {
      if (fwrite(", ", sizeof(char), 2, DBUG_FILE) != 2) {
        goto unlock_file_and_quit;
      }
    }
  }
  fprintf(DBUG_FILE, ")");
  if (print_rowid) {
    fprintf(DBUG_FILE, " rowid ");
    for (uint i = 0; i < table->file->ref_length; i++) {
      fprintf(DBUG_FILE, "%x", table->file->ref[i]);
    }
  }
  fprintf(DBUG_FILE, "\n");
unlock_file_and_quit:
  DBUG_UNLOCK_FILE;
}
#endif

namespace {

void SwitchSlice(JOIN *join, int slice_num) {
  if (slice_num != -1 && !join->ref_items[slice_num].is_null()) {
    join->set_ref_item_slice(slice_num);
  }
}

}  // namespace

int PX_Gather::Read() {
  if (m_join->select_count) {
    // When true, UnqualifiedCountIterator would be used. This iterator directly
    // use join->fields, which is the last ref_slice in ref_items actually.
    SwitchSlice(m_join, m_ref_slice);
  } else {
    SwitchSlice(m_join, m_input_slice);
  }
  int res = m_source->Read();
  if (res != 0) return res;

  if (DBUG_EVALUATE_IF("exchange_inject_test_gather", false, true))
    if (m_temp_table_param && copy_fields_and_funcs(m_temp_table_param, thd()))
      return true; /* purecov: inspected */

  DBUG_EXECUTE_IF("exchange_inject_print",
                  dbug_print_record(table(), false, "PX_Gather", m_ref_slice););

  SwitchSlice(m_join, m_ref_slice);
  return 0;
}

void PX_Gather_base::convert_record_to_field() {
  const uchar *record_curse = m_record_buffer + m_null_flag_offset + 1;
  const uchar *null_flag_curse = m_record_buffer + m_null_flag_offset;

  uint cur_field_num = 0;
  uchar cur_null_flag = 1;
  for (Field *field : *m_fields) {
    if (*null_flag_curse & cur_null_flag) {
      field->set_null();
    } else {
      field->set_notnull();
      record_curse = field->unpack(record_curse);
    }
    ++cur_field_num;
    cur_null_flag <<= 1;
    if (cur_field_num % 8 == 0) {
      cur_null_flag = 1;
      --null_flag_curse;
    }
  }
}

int PX_Send::Read() {
  int res = m_source->Read();
  if (res != 0) return res;

  if (m_temp_table_param && copy_fields_and_funcs(m_temp_table_param, thd()))
    return true; /* purecov: inspected */

  // For some reason, items might not store information in fields. (testcase
  // type_bit_innodb:80) we need to do this manually.
  if (m_send_fields) {
    for (Item *item : *m_send_fields) {
      Field *result_field = item->get_result_field();
      if (result_field &&
          (item->const_item() || (m_temp_table_param->precomputed_group_by &&
                                  item->type() == Item::SUM_FUNC_ITEM))) {
        item->save_in_field(result_field, true);
      }
    }
  }

  DBUG_EXECUTE_IF("exchange_inject_print",
                  dbug_print_record(table(), false, "PX_Send"););

  return 0;
  // return (convert_field_to_record() ? 1 : 0);
}

bool PX_Send::convert_field_to_record() {
  memset(m_record_buffer, 0, EXCHANGE_BUFFER_SIZE);

  uchar *record_cur = m_record_buffer + m_null_flag_offset + 1;
  uchar *null_flag_cur = m_record_buffer + m_null_flag_offset;
  uchar *record_end = m_record_buffer + EXCHANGE_BUFFER_SIZE;

  uint cur_field_num = 0;
  uchar cur_null_flag = 1;

  for (Field *field : *m_fields) {
    if (field->is_null()) {
      *null_flag_cur |= cur_null_flag;
    } else {
      record_cur = field->pack(record_cur, field->field_ptr(),
                               record_end - record_cur);
      assert(record_cur < record_end);  // Out of memory
    }
    ++cur_field_num;
    cur_null_flag <<= 1;
    if (cur_field_num % 8 == 0) {
      cur_null_flag = 1;
      --null_flag_cur;
    }

    if (field->reset())  // Just to prove that gather read this from buffer.
      return true;
  }
  return false;
}

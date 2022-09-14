/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0
*/

#include <mysql/service_parser.h>
#include <string.h>
#include <sys/types.h>

#include "firewall_cdb.h"
#include "firewall_table_service.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/sql_base.h"
#include "sql/table.h"
#include "sql/transaction.h"
#include "user_mode_cache.h"
#include "whitelist_cache.h"

namespace firewall_table_service {
typedef std::vector<std::pair<uint, std::string>> Field_idx_values_sequence;
static void *firewall_callback(void *p_args) {
  Firewall_callback_args *args = pointer_cast<Firewall_callback_args *>(p_args);
  switch (args->type) {
    case WHITELIST_FLUSH: {
      Whitelist_Cache *me = pointer_cast<Whitelist_Cache *>(args->me);
      (me->do_flush_to_disk)(args->thd);
      break;
    }
    case WHITELIST_LOAD: {
      Whitelist_Cache *me = pointer_cast<Whitelist_Cache *>(args->me);
      (me->do_load_data_from_table)(args->thd);
      break;
    }
    case USERMODE_LOAD: {
      Userhost_Mode_Cache *me = pointer_cast<Userhost_Mode_Cache *>(args->me);
      (me->do_load_data_from_table)(args->thd);
      break;
    }
    default:
      break;
  }

  return nullptr;
}

void load_or_flush_data(Callback_type type) {
  THD *thd = mysql_parser_open_session();

  Firewall_callback_args *args = nullptr;
  switch (type) {
    case USERMODE_LOAD:
      args = new Firewall_callback_args(
          cdb_firewall_manager.userhost_mode_cache, thd, type);
      break;
    case WHITELIST_LOAD:
    case WHITELIST_FLUSH:
      args = new Firewall_callback_args(cdb_firewall_manager.whitelist_cache,
                                        thd, type);
      break;

    default:
      break;
  }

  my_thread_handle handle;

  mysql_parser_start_thread(thd, firewall_callback, args, &handle);

  mysql_parser_join_thread(&handle);
  delete args;
}

static void add_column(MY_BITMAP *map, Cursor::column_id column) {
  if (column != Cursor::ILLEGAL_COLUMN_ID) bitmap_set_bit(map, column);
}

/**
  Writeable cursor that allows reading and updating rows
  in firewall table.
*/
Cursor::Cursor(THD *thd, const char *db_name, const char *table_name)
    : m_thd(thd), m_table_list(nullptr), m_is_finished(true) {
  m_inited = false;
  m_table_list = new TABLE_LIST(db_name, strlen(db_name), table_name,
                                strlen(table_name), "alias", TL_WRITE_DEFAULT);
  if (m_table_list == nullptr) return;  // Error

  m_table_list->updating = true;

  if (open_and_lock_tables(m_thd, m_table_list,
                           MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY))
    return;  // Error

  TABLE *table = m_table_list->table;
  if (table == nullptr) return;  // ERROR
}

Cursor::~Cursor() {
  if (m_table_list != nullptr && m_table_list->table != nullptr)
    m_table_list->table->file->ha_rnd_end();
  delete m_table_list;
}

int Cursor::field_index(const char *field_name) {
  TABLE *table = m_table_list->table;
  for (uint i = 0; i < table->s->fields; ++i)
    if (strcmp(table->field[i]->field_name, field_name) == 0) return i;
  return -1;
}

int Cursor::read() {
  TABLE *table = m_table_list->table;
  m_last_read_status = table->file->ha_rnd_next(table->record[0]);
  if (m_last_read_status != 0) m_is_finished = true;
  return m_last_read_status;
}

const char *Cursor::fetch_string(int fieldno) {
  Field **fields = m_table_list->table->field;
  Field *field = fields[fieldno];
  if (field->is_null()) return nullptr;
  String value_buf;
  String *value = field->val_str(&value_buf);
  size_t length = value->length();
  char *res = new char[length + 1];
  strncpy(res, value->ptr(), length);
  res[length] = '\0';
  return res;
}

void free_string(const char *str) { delete[] str; }

void Cursor::copy_and_set(std::string *property, int colno) {
  const char *value = fetch_string(colno);
  if (value != nullptr) property->assign(value);
  free_string(value);
}

bool Cursor::store_field_values_and_flush(
    Field_idx_values_sequence &field_idx_and_values) {
  TABLE *table = m_table_list->table;
  Field **fields = table->field;

  empty_record(table);
  for (auto it = field_idx_and_values.begin(); it != field_idx_and_values.end();
       it++) {
    if (fields[it->first]->store(it->second.c_str(), it->second.length(),
                                 &my_charset_bin))
      return true;
  }
  if (table->file->ha_write_row(table->record[0])) return true;

  return false;
}

User_Mode_Cursor::User_Mode_Cursor(THD *thd)
    : Cursor(thd, get_db_name(), get_table_name()) {
  if (m_table_list == nullptr || m_table_list->table == nullptr) return;  // Error
  TABLE *table = m_table_list->table;
  m_userhost_column = field_index("USERHOST");
  m_mode_column = field_index("MODE");

  if (m_userhost_column == ILLEGAL_COLUMN_ID ||
      m_mode_column == ILLEGAL_COLUMN_ID) {
    trans_rollback_stmt(m_thd);
    close_thread_tables(m_thd);
    delete m_table_list;
    m_table_list = nullptr;
    m_table_is_malformed = true;
    return;  // Error
  } else
    m_table_is_malformed = false;

  add_column(table->read_set, userhost_column());
  add_column(table->read_set, mode_column());
  add_column(table->write_set, userhost_column());
  add_column(table->write_set, mode_column());

  if (m_table_list->table->file->ha_rnd_init(true) != 0) return;  // Error

  // No error occurred, set this to false.
  m_is_finished = false;

  read();
  m_inited = true;
}

Whitelist_Cursor::Whitelist_Cursor(THD *thd)
    : Cursor(thd, get_db_name(), get_table_name()) {
  if (m_table_list == nullptr || m_table_list->table == nullptr) return;  // Error
  TABLE *table = m_table_list->table;
  m_userhost_column = field_index("USERHOST");
  m_digest_column = field_index("DIGEST");

  if (m_userhost_column == ILLEGAL_COLUMN_ID ||
      m_digest_column == ILLEGAL_COLUMN_ID) {
    trans_rollback_stmt(m_thd);
    close_thread_tables(m_thd);
    delete m_table_list;
    m_table_list = nullptr;
    m_table_is_malformed = true;
    return;  // Error
  } else
    m_table_is_malformed = false;

  add_column(table->read_set, userhost_column());
  add_column(table->read_set, digest_column());
  add_column(table->write_set, userhost_column());
  add_column(table->write_set, digest_column());

  if (m_table_list->table->file->ha_rnd_init(true) != 0) return;  // Error

  // No error occurred, set this to false.
  m_is_finished = false;

  read();
  m_inited = true;
}

}  // namespace firewall_table_service

/* Copyright (c) 2020, tencent and/or its affiliates. All rights reserved.
 */

#include <atomic>
#include "sql/recycle_bin.h"
#include "sql/transaction.h"
#include "sql/sql_class.h"
#include "sql/handler.h"
#include "sql/sql_base.h"
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"       // DD_bootstrap_ctx
#include "sql/dd/impl/transaction_impl.h"
#include "sql/sql_initialize.h"                        // opt_initialize_insecure
#include "sql/sql_lex.h"
#include "sql/mysqld.h"
#include "sql/sql_parse.h"
#include "sql/event_parse_data.h"
#include "sql/tztime.h"                               // my_tz_find, my_tz_OFFSET
#include "sql/event_queue.h"
#include "sql/dd/impl/types/event_impl.h"
#include "sql/auth/sql_security_ctx.h"
#include "include/mysql/components/services/log_builtins.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/protocol.h"
#include "sql/sql_table.h"
#include "sql/thd_raii.h"
#include "sql/auth/auth_acls.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/sp_cache.h"
#include "sql/sp.h"
/**
  @addtogroup  recycle_bin
  @{
*/

const LEX_CSTRING Recycle_bin_access_context::TABLE_NAME = {
    STRING_WITH_LEN("recycle_bin_info")};
const LEX_CSTRING Recycle_bin_access_context::DB_NAME = {
    STRING_WITH_LEN("mysql")};
const uint Recycle_bin_persistor::key_parts[] = {1, 1, 1, 2};

static TABLE_LIST *build_table_list(THD *thd, const char *db_name,
                                    const char *table_name) {
  TABLE_LIST *table_list = new (thd->mem_root) TABLE_LIST;
  if (table_list == nullptr) {
    return nullptr;
  }

  table_list->db = thd->mem_strdup(db_name);
  table_list->db_length = strlen(db_name);
  table_list->table_name = thd->mem_strdup(table_name);
  table_list->table_name_length = strlen(table_name);
  table_list->open_type = OT_BASE_ONLY;
  table_list->alias = table_list->table_name;
  table_list->internal_tmp_table = false;
  MDL_REQUEST_INIT(&table_list->mdl_request, MDL_key::TABLE, table_list->db,
                   table_list->table_name, MDL_EXCLUSIVE, MDL_TRANSACTION);
  table_list->next_global = nullptr;
  table_list->next_local = nullptr;

  return table_list;
}

void Recycle_bin_access_context::before_open(THD *thd) {
  DBUG_TRACE;

  m_flags = MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY;
}

bool Recycle_bin_access_context::init(THD **thd, TABLE **table, bool is_write) {
  DBUG_TRACE;

  if (!(*thd)) *thd = m_drop_thd_object = this->create_thd();
  if (!(*thd)->is_cmd_skip_readonly()) {
    (*thd)->set_skip_readonly_check();
  }
  m_is_write = is_write;
  if (m_is_write) {
    /* Disable binlog temporarily */
    m_tmp_disable_binlog__save_options = (*thd)->variables.option_bits;
    (*thd)->variables.option_bits &= ~OPTION_BIN_LOG;
  }

  bool ret = this->open_table(
      *thd, DB_NAME, TABLE_NAME, Recycle_bin_persistor::FIELDS_COUNT,
      m_is_write ? TL_WRITE : TL_READ, table, &m_backup);

  return ret;
}

void Recycle_bin_access_context::deinit(THD *thd, TABLE *table) {
  if (table) table->file->ha_index_or_rnd_end();

  close_table(thd, table);

  /* Re-enable binlog */
  if (m_is_write)
    thd->variables.option_bits = m_tmp_disable_binlog__save_options;
  if (this->m_skip_readonly_set) {
    thd->reset_skip_readonly_check();
    this->m_skip_readonly_set = false;
  }
  if (m_drop_thd_object) this->drop_thd(m_drop_thd_object);
}

THD *Recycle_bin_access_context::create_thd() {
  THD *thd = System_table_access::create_thd();
  /*
    This is equivalent to a new "statement". For that reason, we call
    both lex_start() and mysql_reset_thd_for_next_command.
  */
  lex_start(thd);
  mysql_reset_thd_for_next_command(thd);
  thd->set_skip_readonly_check();
  return (thd);
}

void Recycle_bin_access_context::drop_thd(THD *thd) {
  thd->reset_skip_readonly_check();
  System_table_access::drop_thd(thd);
}

void Recycle_bin_access_context::close_table(THD *thd, TABLE *table) {
  Query_tables_list query_tables_list_backup;
  /*
    In order not to break execution of current statement we have to
    backup/reset/restore Query_tables_list part of LEX, which is
    accessed and updated in the process of closing tables.
  */
  if (table) {
    thd->lex->reset_n_backup_query_tables_list(&query_tables_list_backup);
    close_thread_tables(thd);
    thd->lex->restore_backup_query_tables_list(&query_tables_list_backup);
    thd->restore_backup_open_tables_state(&m_backup);
  }
}

bool Recycle_bin_persistor::fill_fields(Field **fields,
                                        const Recycle_table_record *rec) {
  fields[FIELD_TABLE_NAME]->set_notnull();
  fields[FIELD_ORIGIN_SCHEMA]->set_notnull();
  fields[FIELD_ORIGIN_TABLE]->set_notnull();
  fields[FIELD_DROP_TIME]->set_notnull();
  fields[FIELD_PURGE_TIME]->set_notnull();

  if (fields[FIELD_TABLE_NAME]->store(rec->table_name().c_str(),
                                      rec->table_name().length(),
                                      &my_charset_bin) ||
      fields[FIELD_ORIGIN_SCHEMA]->store(rec->origin_schema().c_str(),
                                         rec->origin_schema().length(),
                                         &my_charset_bin) ||
      fields[FIELD_ORIGIN_TABLE]->store(rec->origin_table().c_str(),
                                        rec->origin_table().length(),
                                        &my_charset_bin)) {
    return true;
  }
  my_timeval dt = rec->drop_time(), pt = rec->purge_time();
  fields[FIELD_DROP_TIME]->store_timestamp(&dt);
  fields[FIELD_PURGE_TIME]->store_timestamp(&pt);
  return false;
}

int Recycle_bin_persistor::write_row(TABLE *table,
                                     const Recycle_table_record *record) {
  DBUG_TRACE;
  int error = 0;
  Field **fields = nullptr;

  fields = table->field;
  empty_record(table);

  if (fill_fields(fields, record)) return -1;

  /* Inserts a new row into the recycle_bin_info table. */
  error = table->file->ha_write_row(table->record[0]);
  if (error) {
    table->file->print_error(error, MYF(0));
    return -1;
  }

  return 0;
}

int Recycle_bin_persistor::save(THD *thd, const Recycle_table_record *record) {
  DBUG_TRACE;
  int error = 0;
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;

  if (table_access_ctx.init(&thd, &table, true)) {
    error = -1;
    goto end;
  }

  if (write_row(table, record)) {
    error = -1;
    goto end;
  }

end:
  table_access_ctx.deinit(thd, table);

  return error;
}

int Recycle_bin_persistor::save(THD *thd, std::vector<Recycle_table_record> &records) {
  DBUG_TRACE;
  int error = 0;
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;

  if (table_access_ctx.init(&thd, &table, true)) {
    error = -1;
    goto end;
  }

  for (auto record : records) {
    if (write_row(table, &record)) {
      error = -1;
      goto end;
    }
  }

end:
  table_access_ctx.deinit(thd, table);

  return error;
}

int Recycle_bin_persistor::delete_row(TABLE *table,
                                      const Recycle_table_record *rec) {
  DBUG_TRACE;

  empty_record(table);
  dd::Raw_record record{table};
  if (record.store(FIELD_TABLE_NAME, dd::String_type(rec->table_name().data())))
    return -1;

  uchar user_key[MAX_KEY_LENGTH]  = { 0 };
  key_copy(user_key, table->record[0], table->key_info,
           table->key_info->key_length);

  int error = 0;

  error = table->file->ha_index_read_idx_map(table->record[0],
                                             table->s->primary_key, user_key,
                                             HA_WHOLE_KEY,
                                             HA_READ_PREFIX_LAST);

  if (error || table->file->ha_delete_row(table->record[0])) {
    error = -1;
  }

  return error;
}

int Recycle_bin_persistor::drop(THD *thd, const Recycle_table_record *record) {
  DBUG_TRACE;

  bool error = 0;
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;

  if (table_access_ctx.init(&thd, &table, true)) {
    error = 1;
    goto end;
  }

  if (delete_row(table, record)) {
    error = -1;
    goto end;
  }

end:
  table_access_ctx.deinit(thd, table);

  return error;
}

int Recycle_bin_persistor::drop(THD *thd,
                                std::vector<Recycle_table_record> &records) {
  DBUG_TRACE;

  bool error = 0;
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;

  if (table_access_ctx.init(&thd, &table, true)) {
    error = 1;
    goto end;
  }

  for (auto record : records) {
    if (delete_row(table, &record)) {
      error = -1;
      goto end;
    }
  }

end:
  table_access_ctx.deinit(thd, table);

  return error;
}

int Recycle_bin_persistor::drop(THD *thd,
                                Prealloced_array<TABLE_LIST *, 1> &tables) {
  DBUG_TRACE;

  bool error = 0;
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;

  if (table_access_ctx.init(&thd, &table, true)) {
    error = 1;
    goto end;
  }

  for (auto t : tables) {
    Recycle_table_record r;
    assert(t->get_table_name());
    r.set_table_name(t->get_table_name());
    if (delete_row(table, &r)) {
      error = -1;
      goto end;
    }
  }

end:
  table_access_ctx.deinit(thd, table);

  return error;
}

int Recycle_bin_persistor::show_recycle_bin(THD *thd) {
  int err = 0;
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;

  /* Prepare list */
  mem_root_deque<Item *> field_list(thd->mem_root);
  Protocol *protocol = thd->get_protocol();
  MYSQL_TIME time;

  field_list.push_back(new Item_empty_string("db", NAME_LEN));
  field_list.push_back(new Item_empty_string("table", NAME_LEN));
  field_list.push_back(new Item_empty_string("recycle_table", NAME_LEN));
  field_list.push_back(new Item_temporal(
      MYSQL_TYPE_DATETIME, Name_string("drop_time", sizeof("drop_time") - 1), 0,
      0));
  field_list.push_back(new Item_temporal(
      MYSQL_TYPE_DATETIME, Name_string("purge_time", sizeof("purge_time") - 1),
      0, 0));

  if (thd->send_result_metadata(field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    return true;
  }

  if (table_access_ctx.init(&thd, &table, true)) {
    err = -1;
    goto end;
  }
  /* Traverse all the record in recycle_bin_info. */
  {
    empty_record(table);
    dd::Raw_record record{table};

    if ((err = table->file->ha_index_init(0, true))) {
      err = -1;
      goto end;
    }

    for (err = table->file->ha_index_first(table->record[0]); !err;
         err = table->file->ha_index_next(table->record[0])) {
      protocol->start_row();
      protocol->store(record.read_str(FIELD_ORIGIN_SCHEMA).c_str(),
                      system_charset_info);
      protocol->store(record.read_str(FIELD_ORIGIN_TABLE).c_str(),
                      system_charset_info);
      protocol->store(record.read_str(FIELD_TABLE_NAME).c_str(),
                      system_charset_info);
      thd->variables.time_zone->gmt_sec_to_TIME(
          &time, (my_time_t)record.read_timestamp(FIELD_DROP_TIME).m_tv_sec);
      protocol->store_datetime(time, 0);
      thd->variables.time_zone->gmt_sec_to_TIME(
          &time, (my_time_t)record.read_timestamp(FIELD_PURGE_TIME).m_tv_sec);
      protocol->store_datetime(time, 0);
      if (protocol->end_row()) {
        break;
        err = -1;
      }
    }

    table->file->ha_index_end();

    if (err != HA_ERR_END_OF_FILE) err = -1;
    else err = 0;

    my_eof(thd);
  }

end:
  table_access_ctx.deinit(thd, table);

  return err;
}

bool Recycle_bin_persistor::match_schema_table_key(
    dd::Raw_record &record, const char *db, const char *table) {
  bool match_field_db =
      db != nullptr &&
      strcmp(record.read_str(FIELD_ORIGIN_SCHEMA).c_str(), db) == 0;
  bool match_tield_table =
      table == nullptr ||
      strcmp(record.read_str(FIELD_ORIGIN_TABLE).c_str(), table) == 0;
  return match_field_db && match_tield_table;
}

bool Recycle_bin_persistor::match_primary_key(
    dd::Raw_record &record, const char *recycle_name) {
  bool match =
      recycle_name != nullptr &&
      strcmp(record.read_str(FIELD_TABLE_NAME).c_str(), recycle_name) == 0;
  return match;
}

std::string Recycle_bin_persistor::find_latest_table(THD *thd,
                                                     const char *db_name,
                                                     const char *table_name,
                                                     time_t timestamp,
                                                     bool &error) {
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;
  std::string latest_recycle_table;
  int err = 0;
  if (table_access_ctx.init(&thd, &table, false)) {
    goto end;
  }

  {
    empty_record(table);
    dd::Raw_record record{table};

    err = record.store(FIELD_ORIGIN_SCHEMA, dd::String_type(db_name)) ||
          record.store(FIELD_ORIGIN_TABLE, dd::String_type(table_name));
    if (err) goto end;

    uchar user_key[MAX_KEY_LENGTH] = {0};
    uint key_no = KEY_SCHEMA_TABLE;
    KEY *key_info = table->key_info + key_no;
    my_timeval latest_time = {0, 0};
    latest_recycle_table.clear();

    key_copy(user_key, table->record[0], key_info,
             key_info->key_length);

    if ((err = table->file->ha_index_init(key_no, true))) {
      table->file->print_error(err, MYF(0));
      goto end;
    }

    for ((err = table->file->ha_index_read_map(
              table->record[0], user_key,
              make_prev_keypart_map(key_parts[key_no]), HA_READ_PREFIX_LAST));
         !err;
         (err = table->file->ha_index_prev(table->record[0]))) {
      if (!match_schema_table_key(record, db_name, table_name)) continue;
      my_timeval time = record.read_timestamp(FIELD_DROP_TIME);
      if (timestamp && timestamp != time.m_tv_sec) continue;

      if (latest_time.m_tv_sec < time.m_tv_sec ||
          (latest_time.m_tv_sec == time.m_tv_sec &&
           latest_time.m_tv_usec < time.m_tv_usec)) {
        latest_time = time;
        latest_recycle_table.assign(record.read_str(FIELD_TABLE_NAME));
      }
    }

    table->file->ha_index_end();
    if (err == HA_ERR_END_OF_FILE || err == HA_ERR_KEY_NOT_FOUND) err = 0;
  }
end:
  table_access_ctx.deinit(thd, table);
  error = err;
  return latest_recycle_table;
}

std::string Recycle_bin_persistor::find_latest_table_by_reycle_name(
    THD *thd, const char *recycle_name, time_t timestamp, bool &error) {
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;
  std::string latest_recycle_table;
  int err = 0;
  if (table_access_ctx.init(&thd, &table, false)) {
    goto end;
  }

  {
    empty_record(table);
    dd::Raw_record record{table};

    err = record.store(FIELD_TABLE_NAME, dd::String_type(recycle_name));
    if (err) goto end;

    uchar user_key[MAX_KEY_LENGTH] = {0};
    uint key_no = KEY_PRIMARY;
    KEY *key_info = table->key_info + key_no;
    my_timeval latest_time = {0, 0};
    latest_recycle_table.clear();

    key_copy(user_key, table->record[0], key_info,
             key_info->key_length);

    if ((err = table->file->ha_index_init(key_no, true))) {
      table->file->print_error(err, MYF(0));
      goto end;
    }

    for ((err = table->file->ha_index_read_map(
              table->record[0], user_key,
              make_prev_keypart_map(key_parts[key_no]), HA_READ_PREFIX_LAST));
         !err;
         (err = table->file->ha_index_prev(table->record[0]))) {
      if (!match_primary_key(record, recycle_name)) continue;
      my_timeval time = record.read_timestamp(FIELD_DROP_TIME);
      if (timestamp && timestamp != time.m_tv_sec) continue;

      if (latest_time.m_tv_sec < time.m_tv_sec ||
          (latest_time.m_tv_sec == time.m_tv_sec &&
           latest_time.m_tv_usec < time.m_tv_usec)) {
        latest_time = time;
        latest_recycle_table.assign(record.read_str(FIELD_TABLE_NAME));
      }
    }

    table->file->ha_index_end();
    if (err == HA_ERR_END_OF_FILE || err == HA_ERR_KEY_NOT_FOUND) err = 0;
  }
end:
  table_access_ctx.deinit(thd, table);
  error = err;
  return latest_recycle_table;
}

bool Recycle_bin_persistor::find_tables_before_time(
    THD *thd, time_t before_time, const char *db_name, const char *table_name,
    bool all, std::vector<std::string> &tables) {
  if (db_name == nullptr || table_name == nullptr) {
    my_error(ER_RECYCLE_BIN_WITHOUT_DB_NAME_OR_TABLE_NAME, MYF(0));
    return true;
  }
  int err = 0;
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;

  if (table_access_ctx.init(&thd, &table, true)) {
    err = -1;
    goto end;
  }

  {
    empty_record(table);
    dd::Raw_record record{table};

    if (record.store(FIELD_ORIGIN_SCHEMA, dd::String_type(db_name)) ||
        record.store(FIELD_ORIGIN_TABLE, dd::String_type(table_name)))
      return true;

    uchar user_key[MAX_KEY_LENGTH];
    uint key_no = KEY_SCHEMA_TABLE;
    KEY *key_info = table->key_info + key_no;
    key_copy(user_key, table->record[0], key_info, key_info->key_length);

    if ((err = table->file->ha_index_init(key_no, true))) {
      table->file->print_error(err, MYF(0));
      goto end;
    }

    my_timeval oldest_table = {INT64_MAX, INT64_MAX};
    for ((err = table->file->ha_index_read_map(
              table->record[0], user_key,
              make_prev_keypart_map(key_parts[KEY_SCHEMA_TABLE]),
              HA_READ_PREFIX_LAST));
         !err;
         (err = table->file->ha_index_prev(table->record[0]))) {
      if (!match_schema_table_key(record, db_name, table_name)) continue;
      my_timeval now = record.read_timestamp(FIELD_DROP_TIME);
      if (before_time && now.m_tv_sec >= before_time) break;

      if (!all && (now.m_tv_sec < oldest_table.m_tv_sec ||
                   (now.m_tv_sec == oldest_table.m_tv_sec &&
                    now.m_tv_usec < oldest_table.m_tv_usec))) {
        tables.clear();
        tables.push_back(record.read_str(FIELD_TABLE_NAME).c_str());
      } else if (all) {
        tables.push_back(record.read_str(FIELD_TABLE_NAME).c_str());
      }
    }
    table->file->ha_index_end();
    if (err == HA_ERR_END_OF_FILE || err == HA_ERR_KEY_NOT_FOUND) err = 0;
  }

end:
  table_access_ctx.deinit(thd, table);

  return err;
}

bool Recycle_bin_persistor::find_tables_before_time(
    THD *thd, time_t before_time, bool all, std::vector<std::string> &tables) {
  int err = 0;
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;
  if (table_access_ctx.init(&thd, &table, true)) {
    err = -1;
    goto end;
  }

  {
    empty_record(table);
    dd::Raw_record record{table};

    if ((err = table->file->ha_index_init(KEY_DROP_TIME, true))) {
      table->file->print_error(err, MYF(0));
      goto end;
    }

    for (err = table->file->ha_index_first(table->record[0]); !err;
         err = table->file->ha_index_next(table->record[0])) {
      my_timeval now = record.read_timestamp(FIELD_DROP_TIME);
      if (before_time && now.m_tv_sec >= before_time) break;
      tables.push_back(record.read_str(FIELD_TABLE_NAME).c_str());
      if (!all) break;
    }

    table->file->ha_index_end();
    if (err == HA_ERR_END_OF_FILE) err = 0;
  }

end:
  table_access_ctx.deinit(thd, table);

  return err;
}

bool Recycle_bin_persistor::find_tables_from_db(
    THD *thd, const char *db_name, std::vector<Recycle_table_record> &tables) {
  assert(db_name);
  int err = 0;
  TABLE *table = nullptr;
  Recycle_bin_access_context table_access_ctx;
  if (table_access_ctx.init(&thd, &table, true)) {
    err = -1;
    goto end;
  }
  {
    empty_record(table);
    dd::Raw_record record{table};

    if (record.store(FIELD_ORIGIN_SCHEMA, dd::String_type(db_name)))
      return true;

    uint key_part_cnt = 1;
    uchar user_key[MAX_KEY_LENGTH];
    uint key_no = KEY_SCHEMA_TABLE;
    KEY *key_info = table->key_info + key_no;
    key_copy(user_key, table->record[0], key_info, key_info->key_length);

    if ((err = table->file->ha_index_init(KEY_SCHEMA_TABLE, true))) {
      table->file->print_error(err, MYF(0));
      goto end;
    }

    for ((err = table->file->ha_index_read_map(table->record[0], user_key,
                                               make_prev_keypart_map(key_part_cnt),
                                               HA_READ_PREFIX_LAST));
         !err;
         (err = table->file->ha_index_prev(table->record[0]))) {
      if (!match_schema_table_key(record, db_name, nullptr)) continue;
      assert(record.read_str(FIELD_ORIGIN_SCHEMA) == dd::String_type(db_name));
      Recycle_table_record rec;
      rec.set_origin_table(record.read_str(FIELD_ORIGIN_TABLE).c_str());
      rec.set_table_name(record.read_str(FIELD_TABLE_NAME).c_str());
      rec.set_origin_schema(record.read_str(FIELD_ORIGIN_SCHEMA).c_str());
      tables.push_back(rec);
    }
    table->file->ha_index_end();
    if (err == HA_ERR_END_OF_FILE || err == HA_ERR_KEY_NOT_FOUND) err = 0;
  }

end:
  table_access_ctx.deinit(thd, table);

  return err;
}

/**
   Mark wheather the recycle bin feature is turned on.
 */ 
bool recycle_bin_enabled(THD *thd) {
  return (txsql_recycle_bin_enabled && !thd->is_bootstrap_system_thread() &&
          !thd->is_server_upgrade_thread()) ||
         thd->system_thread == SYSTEM_THREAD_SLAVE_SQL ||
         thd->system_thread == SYSTEM_THREAD_SLAVE_WORKER;
}

bool recycle_bin_enabled_in_user_thread(THD *thd) {
  return txsql_recycle_bin_enabled && !thd->is_bootstrap_system_thread() &&
         !thd->is_server_upgrade_thread() &&
         thd->system_thread != SYSTEM_THREAD_SLAVE_SQL &&
         thd->system_thread != SYSTEM_THREAD_SLAVE_WORKER;
}

/**
  Gets the name of the table put into the recycle bin.
 */
LEX_CSTRING get_recycle_bin_table_name(THD *thd) {
  /*
    This variable is mainly used to distinguish the uniqueness of the table
    name obtained when the function is executed in the same microsecond.
  */
  static std::atomic_ullong recycle_bin_table_id(0);
  ++recycle_bin_table_id;
  /*
     The length length of ulonglong decimal string is 20. therefore, the 
     maximum length of the current table name is 45 and will not exceed 64.
  */
  char name_buf[65];
  String table_name_string(name_buf, sizeof(name_buf), system_charset_info);
  table_name_string.length(0);
  if (RECYCLE_BIN_SCHEMA_NAME == ORIGINA_RECYCLE_BIN_SCHEMA_NAME)
    table_name_string.append("__cdb_");
  else
    table_name_string.append("__txsql_");
  table_name_string.append_ulonglong(my_micro_time());
  table_name_string.append("_");
  table_name_string.append_ulonglong(recycle_bin_table_id);
  table_name_string.append("__");
  LEX_CSTRING table_name;
  table_name.str= thd->strmake(table_name_string.c_ptr(),
                               table_name_string.length());
  table_name.length= table_name_string.length();
  return table_name;
}


static bool is_recycle_bin_table_to_be_access(TABLE_LIST *tables) {
  for (TABLE_LIST *table= tables; table; table= table->next_global) {
    assert(table->db && table->table_name);
    if (is_recycle_bin_db(table->db, table->db_length))
      return true;
  }
  return false;
}

/**
  Check if write access to recycle_bin system schema is allowed
*/
bool deny_access_recycle_bin_schema(THD *thd, TABLE_LIST *all_tables) {
  DBUG_TRACE;

  /* Handling data dictionary tables in bootstrap is allowed. */
  if ((dd::bootstrap::DD_bootstrap_ctx::instance().get_stage() <
         dd::bootstrap::Stage::FINISHED) ||
      opt_initialize ||
      opt_initialize_insecure)
    return false;

  /*
    Allow binlog relative thread user to access the
    recycle bin.
  */
  if (thd->is_system_thread() ||
      thd->security_context()->check_access(SUPER_ACL))
    return false;

  /*
    Allow some sql command can assess recycle bin.
  */
  if (sql_command_flags[thd->lex->sql_command] &
        CF_ALLOW_ACCESS_CDB_RECYCLE_BIN_SCHEMA)
    return false;

  /* The user's drop table statement can access the recycle bin. */
  if (thd->lex->recycle_bin_op == RB_RECYCLE_TABLE_BY_DROP ||
      thd->lex->recycle_bin_op == RB_RECYCLE_TABLE_BY_TRUNCATE)
    return false;

  if (is_recycle_bin_table_to_be_access(all_tables))
    return true;

  if(thd->lex->sql_command == SQLCOM_DROP_DB &&
     is_recycle_bin_db(thd->lex->name.str, thd->lex->name.length))
    return true;

  return false;
}

/**
   Provide the definition of the static member as well as the declaration.
 */

constexpr LEX_CSTRING Recycle_bin_event::m_event_name;
constexpr LEX_CSTRING Recycle_bin_event::m_definer;

constexpr LEX_CSTRING Recycle_bin_event::m_definer_user;
constexpr LEX_CSTRING Recycle_bin_event::m_definer_host;

constexpr LEX_CSTRING Recycle_bin_event::m_definition;

/**
  A help function to initialize based class event_basic

  @param[in]     thd      Thread handle.
  @param[out]    event    A event to be initialized.

  @return False on success, true if it failed.
 */
bool Recycle_bin_event::init_event_basic(THD *thd, Event_basic &event) {
  MEM_ROOT *mem_root = event.get_mem_root();
  event.m_schema_name = make_lex_cstring(mem_root, RECYCLE_BIN_SCHEMA_NAME);
  event.m_event_name = make_lex_cstring(mem_root, m_event_name);
  event.m_definer = make_lex_cstring(mem_root, m_definer);
  String str(m_time_zone, &my_charset_latin1);
  event.m_time_zone = my_tz_find(thd, &str);
  if (event.m_time_zone == nullptr) return true;
  else return false;
}

/**
   Initialize the element of event_queue in event_scheduler

  @param[in]     thd            Thread handle.
  @param[out]    event          A queue event to be initialized.
  @param[in]     interval_time  interval time of scheduler

  @return False on success, true if it failed.

  The description about field of fake queue event is as table below.

   | Member            | INFORMATION_SCHEMA.EVENTS   | Description           |
   |-------------------|-----------------------------|-----------------------|
   | m_originatore     |  ORIGINATOR                 | Server ID of creator  |
   | m_last_executed   |  LAST_EXECUTED              |                       |
   | m_execute_at      |  EXECUTE_AT                 |                       |
   | m_starts          |  STARTS                     |                       |
   | m_ends            |  ENDS                       |                       |
   | m_starts_null     |                             | STARTS is not null    |
   | m_ends_null       |                             | ENDS is not null      |
   | m_execute_at_null |                             | EXECUTE_AT is not null|
   | m_expression      | EVERY 30                    | "every 30"            |
   | m_interval        | SECOND                      | time unit             |
   | m_dropped         |                             | exceeds end_time      |
*/

bool Recycle_bin_event::init_queue_element(THD* thd,
                                           Event_queue_element &event,
                                           ulong interval_time) {

  if (init_event_basic(thd, event)) return true;

  event.m_on_completion = Event_parse_data::ON_COMPLETION_DROP;
  event.m_status = Event_parse_data::ENABLED;
  event.m_originator = 0;
  event.m_last_executed = 0;
  event.m_execute_at = 0;
  event.m_starts = thd->query_start_timeval_trunc(2).m_tv_sec;
  event.m_ends = 0;
  event.m_starts_null = false;
  event.m_ends_null = true;
  event.m_execute_at_null= true;
  event.m_expression = interval_time;
  event.m_interval = INTERVAL_SECOND;
  event.m_dropped = false;
  event.m_execution_count = 0;
  return false;
}

/**
   Queue the initialized queue event into event_queue of event_scheduler.

  @param[in]        thd            Thread handle.
  @param[in,out]    event_queue    The priority queue which the
                                   queue event will insert into.

  @return False on success, true if it failed.
 */

bool Recycle_bin_event::queue_event(THD *thd, Event_queue *event_queue) {

  /**
    @todo: 1. set txsql_recycle_scheduler_interval as rw variable
              instead of read only -by dct
  */
  ulong interval_time = txsql_recycle_scheduler_interval;
  if (interval_time == 0 || txsql_recycle_bin_enabled == false) {
    LogErr(SYSTEM_LEVEL,
            ER_CDB_SYS_RECYCLE_BIN_PURGE_SCHEDULER_DISABLED);
    return false;
  }

  assert(thd);
  assert(event_queue);

  /*
    Note: Can't use (thd->mem_root) to allocate the Event_queue_element,
    Because the thd will clear mem_root at each wile loop in
    Event_scheduler::run.
  */
  std::unique_ptr<Event_queue_element> et(new (std::nothrow)
                                              Event_queue_element());
  if (!et) {
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), sizeof(Event_queue_element));
    return true;
  }

  if (init_queue_element(thd, *(et.get()), interval_time)) return true;
  bool created = false;
  if (event_queue->create_event(thd, et.get(), &created))  return true;
  if (created) {
    et.release();
    LogErr(SYSTEM_LEVEL,
            ER_CDB_SYS_RECYCLE_BIN_PURGE_SCHEDULER_INIT_SUCCESSFULLY);
    return false;
  } else {
    LogErr(SYSTEM_LEVEL,
            ER_CDB_SYS_RECYCLE_BIN_PURGE_SCHEDULER_FAILED);
    my_error(ER_CDB_RECYCLE_BIN_PURGE_SCHEDULER_FAILED, MYF(0));
    return true;
  }
}

/**
   Judge Whether the event is recycle bin event.

  @param[in]        db_name            The db name of event.
  @param[in]        event_name         The name of event.

  @return True if the event is recycle bin event, otherwise return false.
 */

bool Recycle_bin_event::is_recycle_bin_event(LEX_CSTRING &db_name,
                                             LEX_CSTRING &event_name) {
  if (txsql_recycle_scheduler_interval == 0) {
    return false;
  }
  bool is_equal = false;
  if (lower_case_table_names) {
    is_equal = my_strcasecmp(system_charset_info, db_name.str,
                             RECYCLE_BIN_SCHEMA_NAME.str) == 0 &&
               my_strcasecmp(system_charset_info, event_name.str,
                             m_event_name.str) == 0;
  } else {
    is_equal = strcmp(db_name.str, RECYCLE_BIN_SCHEMA_NAME.str) == 0 &&
               strcmp(event_name.str, m_event_name.str) == 0;
  }
  return is_equal;
}


/**
   Initialize the job data to be used in event worker thread.

  @param[in]     thd      Thread handle.
  @param[out]    event    A job data event to be initialized.

  @return False on success, true if it failed.
*/

bool Recycle_bin_event::init_job_data(THD *thd, Event_job_data &event) {
  MEM_ROOT *mem_root = event.get_mem_root();

  if (init_event_basic(thd, event)) goto error;
  event.m_definition = make_lex_string(mem_root, m_definition);
  event.m_definer_user = make_lex_cstring(mem_root, m_definer_user);
  event.m_definer_host = make_lex_cstring(mem_root, m_definer_host);

  {
    dd::Event_impl dd_info;
    dd_info.set_client_collation_id(m_client_collation_id);
    dd_info.set_connection_collation_id(m_connection_collation_id);
    dd_info.set_schema_collation_id(m_schema_collation_id);
    create_event_creation_ctx(dd_info, &(event.m_creation_ctx));
  }
  if (event.m_creation_ctx == nullptr) goto error;

  event.m_sql_mode = m_sql_mode;
  return false;

error:
  DBUG_PRINT(RB_DEBUG_INFO, ("purge table job data init failed."));
  my_error(ER_CDB_RECYCLE_BIN_PURGE_WORKER_FAILED, MYF(0));
  return true;
}

size_t recycle_bin_data_size() {
  char path[2 * FN_REFLEN + 16];
  build_table_filename(path, sizeof(path) - 1,
                       RECYCLE_BIN_SCHEMA_NAME.str, "", "", 0);

  MY_DIR *dir = my_dir(path, MYF(MYF(MY_WANT_STAT)));

  if (!dir) {
    return 0;
  }

  size_t size = 0;
  for (uint i = 0; i < dir->number_off_files; ++i) {
    if (strcmp(dir->dir_entry[i].name, ".") == 0 ||
        strcmp(dir->dir_entry[i].name, "..") == 0) {
      continue;
    }
    size += dir->dir_entry[i].mystat->st_size;
  }

  my_dirend(dir);

  return size;
}

/** Show information of tables inside recycle bin */
bool show_recycle_bin(THD *thd) {
  Recycle_bin_persistor recycle_bin_info;
  int err = recycle_bin_info.show_recycle_bin(thd);
  if (err) my_error(ER_RECYCLE_BIN_READ_FAILED, MYF(0));
  return err;
}

static TABLE_LIST *find_table_name_from_recycle_bin(THD *thd,
                                                    const char *db_name,
                                                    const char *table_name,
                                                    const char *recycle_name,
                                                    time_t timestamp) {
  Recycle_bin_persistor recycle_bin_info;
  TABLE_LIST *table_list = nullptr;
  bool error = false;
  bool with_recycle_name = thd->lex->recycle_name.str != nullptr;
  std::string latest_recycle_table;
  if (with_recycle_name)
    latest_recycle_table = recycle_bin_info.find_latest_table_by_reycle_name(
        thd, recycle_name, timestamp, error);
  else
    latest_recycle_table = recycle_bin_info.find_latest_table(
        thd, db_name, table_name, timestamp, error);
  if (error) {
    my_error(ER_RECYCLE_BIN_READ_FAILED, MYF(0));
    return nullptr;
  } else if (latest_recycle_table.length() == 0) {
    my_error(ER_RECYCLE_BIN_NOT_FOUND, MYF(0));
    return nullptr;
  }
  table_list = build_table_list(thd, RECYCLE_BIN_SCHEMA_NAME.str,
                                latest_recycle_table.c_str());
  if (table_list == nullptr) {
    my_error(ER_DA_OOM, MYF(0));
  }
  return table_list;
}

bool mysql_clear_tables(THD *thd, time_t before_time, Table_ident *table_ident,
                        bool all) {
  Recycle_bin_persistor recycle_bin_info;
  std::vector<std::string> tables_to_clear;

  if (check_global_access(thd, SUPER_ACL))
    return true;

  bool err = false;
  if (table_ident)
    err = recycle_bin_info.find_tables_before_time(
        thd, before_time,
        table_ident->db.str == nullptr ? thd->db().str : table_ident->db.str,
        table_ident->table.str, all, tables_to_clear);
  else
    err = recycle_bin_info.find_tables_before_time(thd, before_time,
                                                   true, tables_to_clear);

  if (err) {
    my_error(ER_RECYCLE_BIN_READ_FAILED, MYF(0));
    return true;
  } else if (tables_to_clear.size() == 0) {
    if (before_time || table_ident) {
      my_error(ER_RECYCLE_BIN_NOT_FOUND, MYF(0));
      return true;
    } else {
      push_warning(thd, Sql_condition::SL_WARNING, ER_RECYCLE_BIN_NOT_FOUND,
                   ER_THD(thd, ER_RECYCLE_BIN_NOT_FOUND));
    }
  }

  if (tables_to_clear.empty()) {

    my_ok(thd);
    return false;
  }

  TABLE_LIST *head = nullptr;;
  TABLE_LIST *end = nullptr;
  for (auto table_name : tables_to_clear) {
    TABLE_LIST *tl =
        build_table_list(thd, RECYCLE_BIN_SCHEMA_NAME.str, table_name.c_str());
    if (tl != nullptr) {
      if (head == nullptr) {
        head = tl;
        end = tl;
      } else {
        end->next_local = tl;
        end->next_global = tl;
        end = tl;
      }
    }
  }

  thd->lex->recycle_bin_op = RB_PURGE_TABLE;

  bool error =  mysql_rm_table(thd, head, false, false);

  return error;
}

bool mysql_restore_db(THD *thd, const char *db_name) {
  thd->lex->recycle_bin_op = RB_RECOVERY_TABLE_BY_RESTORE;

  if (check_global_access(thd, SUPER_ACL))
    return true;

  std::vector<Recycle_table_record> tables;
  Recycle_bin_persistor recycle_bin_info;
  bool err = recycle_bin_info.find_tables_from_db(thd, db_name, tables);

  if (err) {
    my_error(ER_RECYCLE_BIN_READ_FAILED, MYF(0));
    return true;
  } else if (tables.size() == 0) {
    my_error(ER_RECYCLE_BIN_NOT_FOUND, MYF(0));
    return true;
  }

  std::sort(tables.begin(), tables.end(),
            [](Recycle_table_record &a, Recycle_table_record &b) {
              return strcmp(a.origin_table().c_str(), b.origin_table().c_str());
            });
  std::string tmp = "";
  for (auto &r : tables) {
    if (strcmp(tmp.c_str(), r.origin_table().c_str()) == 0) {
      my_error(ER_RECYCLE_BIN_DUPLICATE_TABLE, MYF(0), db_name,
               r.origin_table().c_str());
      return true;
    }
    tmp = r.origin_table();
  }

  /* Now let's build table list for moving */
  TABLE_LIST *head = nullptr;
  TABLE_LIST *curr = nullptr;
  for (auto table : tables) {
    TABLE_LIST *from = build_table_list(thd, RECYCLE_BIN_SCHEMA_NAME.str,
                                        table.table_name().c_str());
    TABLE_LIST *to = build_table_list(thd, table.origin_schema().c_str(),
                                      table.origin_table().c_str());
    if (from == nullptr || to == nullptr) {
      my_error(ER_DA_OOM, MYF(0));
      return true;
    }

    if (curr != nullptr) {
      curr->next_local = from;
      curr->next_global = from;
    }

    from->next_local = to;
    from->next_global = to;

    if (head == nullptr) {
      head = from;
    }

    curr = to;
  }

  bool error = false;

  if (head != nullptr) {
    error = mysql_rename_tables(thd, head);
  }

  if (!error) {
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

    /* Check if target database exists */
    if (head == nullptr && lock_schema_name(thd, db_name)) {
      return true;
    }

    const dd::Schema *target_schema = nullptr;
    if (thd->dd_client()->acquire(db_name, &target_schema)) {
      return true;
    }

    if (target_schema == nullptr) {
      /* doesn't exist, return error */
      my_error(ER_RECYCLE_BIN_TARGET_NOT_CREATED, MYF(0), db_name);
      return true;
    }

    if (lock_db_routines(thd, *target_schema)) {
      return true;
    }

    // Vector for the stored routines of the schema.
    std::vector<const dd::Routine *> routines;
    // Fetch stored routines of the schema.
    if (thd->dd_client()->fetch_schema_components(target_schema, &routines))
      return true;
    for (const dd::Routine *routine : routines) {
      thd->dd_client()->invalidate(routine);
    }

    if (head == nullptr) my_ok(thd);
  }

  return error;
}

bool mysql_restore_table(THD *thd, const char *db, const char *table_name,
                         const char *recycle_name, time_t timestamp) {
  const char *db_name = (db == nullptr ? thd->db().str : db);

  if (check_global_access(thd, SUPER_ACL))
    return true;

  if (db_name == nullptr) {
    my_error(ER_NO_DB_ERROR, MYF(0));
    return true;
  }

  /* Create table list for table */
  TABLE_LIST *old_tl = build_table_list(thd, db_name, table_name);
  assert(old_tl != nullptr);

  /** Check access to database */
  if (check_access(thd, INSERT_ACL | CREATE_ACL, old_tl->db,
        &old_tl->grant.privilege,
        &old_tl->grant.m_internal, false, false)) {
    return true;
  }

  /** Check access to table */
  if (check_grant(thd, INSERT_ACL | CREATE_ACL, old_tl, false, 1, false)) {
    return true;
  }

  TABLE_LIST *tl = find_table_name_from_recycle_bin(
      thd, (db == nullptr ? thd->db().str : db), table_name, recycle_name,
      timestamp);
  if (tl == nullptr) {
    return true;
  }

  tl->next_local = old_tl;
  tl->next_global = old_tl;
  old_tl->next_local = nullptr;
  old_tl->next_global = nullptr;

  bool error = mysql_rename_tables(thd, tl);

  return error;
}

/* Map drop table to rename table. */
bool mysql_recycle_tables(THD *thd, TABLE_LIST *table_list) {
  TABLE_LIST *curr = nullptr;
  TABLE_LIST *next = nullptr;
  curr = table_list;

  if (check_global_access(thd, SUPER_ACL))
    return true;

  while (curr != nullptr) {
    next = curr->next_local;
    LEX_CSTRING table_name = get_recycle_bin_table_name(thd);

    TABLE_LIST *ptr =
        build_table_list(thd, RECYCLE_BIN_SCHEMA_NAME.str, table_name.str);
    if (ptr == nullptr) {
      my_error(ER_DA_OOM, MYF(0));
      return true;
    }
    ptr->query_block = curr->query_block;
    ptr->set_tableno(0);
    ptr->set_lock({TL_IGNORE, THR_DEFAULT});

    /* Lock the new table name */
    if (lock_table_names(thd, ptr, nullptr, thd->variables.lock_wait_timeout,
                         0)) {
      return true;
    }

    curr->next_local = ptr;
    ptr->next_local = next;

    curr = next;
  }

  return mysql_rename_tables(thd, table_list);
}

TABLE_LIST* mysql_recycle_list(THD* thd, TABLE_LIST *tables, bool& error, bool& failback) {
  if (check_global_access(thd, SUPER_ACL)) {
    error = true;
    return tables;
  }

  error = false;
  if (tables == nullptr) {
    return nullptr;
  }

  failback = false;

  if (recycle_bin_data_size() >= g_recycle_bin_max_size) {
    if (!opt_drop_if_exceed_recycle_limit) {
      /* through an error and set error to true*/
      my_error(ER_RECYCLE_BIN_LIMIT_EXCEED, MYF(0), g_recycle_bin_max_size);
      error = true;
      return nullptr;
    } else {
      push_warning_printf(
          thd, Sql_condition::SL_WARNING,
          ER_RECYCLE_BIN_LIMIT_EXCEED,
          ER_THD(thd, ER_RECYCLE_BIN_LIMIT_EXCEED), g_recycle_bin_max_size);
      failback = true;
      return tables;
    }
  }

  for (TABLE_LIST *curr = tables; curr != nullptr; curr = curr->next_local) {
    if (is_recycle_bin_db(curr->db, curr->db_length)) {
      failback = true;
      return tables;
    }
  }

  std::vector<TABLE_LIST*> not_recycle_tables;
  not_recycle_tables.clear();

  TABLE_LIST *curr;
  TABLE_LIST *prev = nullptr;
  TABLE_LIST *head = tables;

  for (curr = tables; curr != nullptr; curr = curr->next_local) {
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
    const dd::Table *table_def = nullptr;
    if (thd->dd_client()->acquire(curr->db, curr->table_name,
          &table_def)) {
      /* Error should have been reported by data-dictionary subsystem. */
      error = true;
      return nullptr;
    }

    /* Moving view from one schema to another is not allowed. So we don't
    recycle view. */
    if (!(table_def &&
          table_def->type() == dd::enum_table_type::BASE_TABLE)) {
      not_recycle_tables.push_back(curr);

      if (prev == nullptr) {
        /* This is first table, remove it from head */
        head = curr->next_local;
      } else {
        /* Remove it from list. */
        prev->next_local = curr->next_local;
      }

      continue;
    }

    prev = curr;
  }

  if (head != nullptr) {
    error = mysql_recycle_tables(thd, head);

    if (error) {
      /* error happens, return directly */
      return nullptr;
    }
  }

  /* Construct new list to be dropped */
  if (not_recycle_tables.empty()) {
    return nullptr;
  }

  head = *(not_recycle_tables.begin());
  prev = nullptr;
  for (auto tl : not_recycle_tables) {
    if (prev != nullptr) {
      prev->next_local = tl;
    }

    tl->next_local = nullptr;

    prev = tl;
  }

  return head;
}

/**
  @} (End of group recycle_bin)
*/
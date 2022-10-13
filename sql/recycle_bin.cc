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
/**
  @addtogroup  recycle_bin
  @{
*/

const LEX_CSTRING Recycle_bin_access_context::TABLE_NAME = {
    STRING_WITH_LEN("recycle_bin_info")};
const LEX_CSTRING Recycle_bin_access_context::DB_NAME = {
    STRING_WITH_LEN("mysql")};

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

/**
   Mark wheather the recycle bin feature is turned on.
 */ 
bool recycle_bin_enabled(THD *thd) {
  return cdb_recycle_bin_enabled ||
         thd->system_thread == SYSTEM_THREAD_SLAVE_SQL ||
         thd->system_thread == SYSTEM_THREAD_SLAVE_WORKER;
}

bool recycle_bin_enabled_in_user_thread(THD *thd) {
  return cdb_recycle_bin_enabled &&
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
  if (thd_system_privilege(thd))
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
    @todo: 1. set cdb_recycle_scheduler_interval as rw variable
              instead of read only -by dct
  */
  ulong interval_time = cdb_recycle_scheduler_interval;
  if (interval_time == 0 || cdb_recycle_bin_enabled == false) {
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
  return ((cdb_recycle_scheduler_interval != 0) &&
          (my_strcasecmp(system_charset_info, db_name.str,
                         RECYCLE_BIN_SCHEMA_NAME.str) == 0 &&
           my_strcasecmp(system_charset_info, event_name.str,
                         m_event_name.str) == 0));
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

/**
  @} (End of group recycle_bin)
*/
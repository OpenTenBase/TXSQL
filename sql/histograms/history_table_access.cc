/**
  @file sql/histograms/history_table_access.cc
*/

#include "sql/histograms/history_table_access.h"

#include <chrono>  // std::system_clock

#include "scope_guard.h"
#include "sql-common/json_dom.h"             // Json_*
#include "sql/dd/impl/raw/raw_record.h"      // Raw_record
#include "sql/dd/types/column_statistics.h"  // Column_statistics
#include "sql/histograms/histogram.h"        // Histogram
#include "sql/mysqld.h"                      // histogram_history_versions_limit
#include "sql/sql_base.h"
#include "sql/sql_class.h"               // THD
#include "sql/sql_system_table_check.h"  // System_table_intact
#include "sql/table.h"                   // TABLE

namespace histograms {

enum {
  FIELD_SCHEMA_NO,
  FIELD_TABLE_NO,
  FIELD_COLUMN_NO,
  FIELD_HISTOGRAM_NO,
  FIELD_VERSION_NO,
  FIELD_CREATION_TIME_NO,
  FIELD_OPTION_NO,
  NUMBER_OF_FIELDS,
};

static const uint history_key_parts = 3;
static constexpr LEX_CSTRING s_db_name = {STRING_WITH_LEN("mysql")};
static constexpr LEX_CSTRING s_tb_name = {
    STRING_WITH_LEN("column_statistics_history")};

static const TABLE_FIELD_TYPE s_history_table_fields[NUMBER_OF_FIELDS] = {
    {{STRING_WITH_LEN("schema_name")},
     {STRING_WITH_LEN("varchar(64)")},
     {STRING_WITH_LEN("utf8mb3")}},
    {{STRING_WITH_LEN("table_name")},
     {STRING_WITH_LEN("varchar(64)")},
     {STRING_WITH_LEN("utf8mb3")}},
    {{STRING_WITH_LEN("column_name")},
     {STRING_WITH_LEN("varchar(64)")},
     {STRING_WITH_LEN("utf8mb3")}},
    {{STRING_WITH_LEN("histogram")}, {STRING_WITH_LEN("json")}, {nullptr, 0}},
    {{STRING_WITH_LEN("version")}, {STRING_WITH_LEN("bigint")}, {nullptr, 0}},
    {{STRING_WITH_LEN("creation_time")},
     {STRING_WITH_LEN("datetime")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("option")},
     {STRING_WITH_LEN("mediumtext")},
     {STRING_WITH_LEN("utf8mb3")}}};

static const TABLE_FIELD_DEF s_history_table_def = {NUMBER_OF_FIELDS,
                                                    s_history_table_fields};

/**
  Set key prefix fields in the record buffer.

  The key is defined as (schema_name, table_name, column_name, version) for
  mysql.column_statistics.

  @param [out] record  the record buffer
  @param [in] schema_name  schema name
  @param [in] table_name  table name
  @param [in] column_name  column name

  @return true if error, or false otherwise
 */
static bool store_key_prefix(dd::Raw_record &record,
                             const dd::String_type &schema_name,
                             const dd::String_type &table_name,
                             const dd::String_type &column_name) {
  return record.store(FIELD_SCHEMA_NO, schema_name) ||
         record.store(FIELD_TABLE_NO, table_name) ||
         record.store(FIELD_COLUMN_NO, column_name);
}

/**
  Test if the key prefix fields match the record buffer.

  The key is defined as (schema_name, table_name, column_name, version) for
  mysql.column_statistics.

  @param [in] record  the record buffer
  @param [in] schema_name  schema name
  @param [in] table_name  table name
  @param [in] column_name  column name

  @return true if it is an exact match, or false otherwise
 */
static bool match_key_prefix(const dd::Raw_record &record,
                             const dd::String_type &schema_name,
                             const dd::String_type &table_name,
                             const dd::String_type &column_name) {
  return record.read_str(FIELD_SCHEMA_NO) == schema_name &&
         record.read_str(FIELD_TABLE_NO) == table_name &&
         record.read_str(FIELD_COLUMN_NO) == column_name;
}

History_table_access_context::History_table_access_context() {}

History_table_access_context::~History_table_access_context() {
  assert(m_deinited);
}

bool History_table_access_context::init(THD *thd, bool is_write) {
  DBUG_TRACE;

  m_thd = thd;
#ifndef DBUG_OFF
  m_deinited = false;
#endif

  if (m_thd == nullptr) {
    m_thd = History_table_access_context::create_thd();
    m_thd_need_drop = true;
  }

  m_error = System_table_access::open_table(
      m_thd, s_db_name, s_tb_name, NUMBER_OF_FIELDS,
      is_write ? TL_WRITE : TL_READ, &m_table, &m_backup);

  if (m_error) {
    return true;
  }

  System_table_intact table_intact(thd);
  m_error = table_intact.check(thd, m_table, &s_history_table_def);

  return m_error;
}

bool History_table_access_context::deinit() {
  DBUG_TRACE;

#ifndef DBUG_OFF
  m_deinited = true;
#endif

  if (System_table_access::close_table(m_thd, m_table, &m_backup, m_error != 0,
                                       true)) {
    return true;
  }

  if (m_thd_need_drop) {
    History_table_access_context::drop_thd(m_thd);
  }

  return false;
}

THD *History_table_access_context::create_thd() {
  DBUG_TRACE;

  m_thd_save = current_thd;
  m_mem_save = THR_MALLOC;

  THD *thd = System_table_access::create_thd();
  thd->system_thread = NON_SYSTEM_THREAD;
  return thd;
}

void History_table_access_context::drop_thd(THD *thd) {
  DBUG_TRACE;

  System_table_access::drop_thd(thd);

  THR_MALLOC = m_mem_save;
  current_thd = m_thd_save;
}

void History_table_access_context::before_open(THD *) {
  DBUG_TRACE;

  m_flags = (MYSQL_OPEN_IGNORE_FLUSH | MYSQL_LOCK_IGNORE_TIMEOUT |
             MYSQL_OPEN_IGNORE_KILLED);
}

bool History_table_persistor::save(const dd::String_type &schema_name,
                                   const dd::String_type &table_name,
                                   const dd::String_type &column_name,
                                   const Histogram *histogram) {
  DBUG_TRACE;

  bool res = false;
  History_table_access_context table_ctx;
  if (table_ctx.init(m_thd, true)) {
    res = true;
    goto end;
  }

  if (write_row(table_ctx, schema_name, table_name, column_name, histogram)) {
    res = true;
    goto end;
  }

  res = shrink(table_ctx, schema_name, table_name, column_name);

end:
  res = table_ctx.deinit() || res;
  return res;
}

bool History_table_persistor::load(const dd::String_type &schema_name,
                                   const dd::String_type &table_name,
                                   const dd::String_type &column_name,
                                   int64_t version, Histogram **histogram) {
  DBUG_TRACE;

  bool res = false;
  History_table_access_context table_ctx;
  if (table_ctx.init(m_thd)) {
    res = true;
    goto end;
  }

  if (read_row(table_ctx, schema_name, table_name, column_name, version,
               histogram)) {
    res = true;
    goto end;
  }
  assert(*histogram != nullptr);

end:
  res = table_ctx.deinit() || res;
  return res;
}

bool History_table_persistor::drop(const dd::String_type &schema_name,
                                   const dd::String_type &table_name,
                                   const dd::String_type &column_name) {
  DBUG_TRACE;

  bool res = false;
  History_table_access_context table_ctx;

  if (table_ctx.init(m_thd, true)) {
    res = true;
    goto end;
  }

  if (delete_stats(table_ctx, schema_name, table_name, column_name)) {
    res = true;
    goto end;
  }

end:
  res = table_ctx.deinit() || res;
  return res;
}

bool History_table_persistor::rename(const dd::String_type &old_schema_name,
                                     const dd::String_type &old_table_name,
                                     const dd::String_type &new_schema_name,
                                     const dd::String_type &new_table_name,
                                     const dd::String_type &column_name) {
  DBUG_TRACE;

  bool res = false;
  History_table_access_context table_ctx;

  if (table_ctx.init(m_thd, true)) {
    res = true;
    goto end;
  }

  if (rename_stats(table_ctx, old_schema_name, old_table_name, new_schema_name,
                   new_table_name, column_name)) {
    res = true;
    goto end;
  }

end:
  res = table_ctx.deinit() || res;
  return res;
}

bool History_table_persistor::write_row(History_table_access_context &table_ctx,
                                        const dd::String_type &schema_name,
                                        const dd::String_type &table_name,
                                        const dd::String_type &column_name,
                                        const Histogram *histogram) {
  using namespace std::chrono;
  DBUG_TRACE;

  Json_object json_dom;
  histogram->histogram_to_json(&json_dom);
  Json_wrapper json{&json_dom, true};

  int64_t micro_time = time_point_cast<microseconds>(system_clock::now())
                           .time_since_epoch()
                           .count();
  my_timeval time_value;
  my_micro_time_to_timeval(micro_time, &time_value);

  TABLE *table = table_ctx.m_table;
  empty_record(table);
  dd::Raw_record record{table};

  if (store_key_prefix(record, schema_name, table_name, column_name) ||
      record.store_json(FIELD_HISTOGRAM_NO, json) ||
      record.store(FIELD_VERSION_NO, static_cast<longlong>(micro_time)) ||
      record.store_timestamp(FIELD_CREATION_TIME_NO, time_value)) {
    return true;
  }

  table_ctx.m_error = table->file->ha_write_row(table->record[0]);
  return table_ctx.m_error != 0;
}

bool History_table_persistor::read_row(History_table_access_context &table_ctx,
                                       const dd::String_type &schema_name,
                                       const dd::String_type &table_name,
                                       const dd::String_type &column_name,
                                       int64_t version, Histogram **histogram) {
  DBUG_TRACE;

  TABLE *table = table_ctx.m_table;
  empty_record(table);
  dd::Raw_record record{table};

  // Compose key prefix fields.
  if (store_key_prefix(record, schema_name, table_name, column_name)) {
    return true;
  }

  // The convention is positive numbers are exact versions, and
  // negative numbers are relative: -1 means last, -2 means second to last.
  assert(version != Histogram::INVALID_VERSION);

  // Complete the key if a concrete numeric version is specified.
  if (version > Histogram::INVALID_VERSION &&
      record.store(FIELD_VERSION_NO, static_cast<longlong>(version))) {
    return true;
  }

  uchar user_key[MAX_KEY_LENGTH];
  key_copy(user_key, table->record[0], table->key_info,
           table->key_info->key_length);

  {
    int error = 0;
    if ((error = table->file->ha_index_init(0, true))) {
      table->file->print_error(error, MYF(0));
      DBUG_PRINT("info", ("ha_index_init error"));
      return true;
    }

    auto guard = create_scope_guard([table] { table->file->ha_index_end(); });

    if (version > Histogram::INVALID_VERSION) {
      error = table->file->ha_index_read_map(table->record[0], user_key,
                                             HA_WHOLE_KEY, HA_READ_KEY_EXACT);
    } else if (-version <= histogram_history_versions_limit) {
      // Negative numbers are relative: -1 means last, -2 means second to last.
      uint idx = 0, target = static_cast<uint>(-version);
      for ((error = table->file->ha_index_read_map(table->record[0], user_key,
                                                   make_prev_keypart_map(history_key_parts),
                                                   HA_READ_PREFIX_LAST));
           !error &&
           match_key_prefix(record, schema_name, table_name, column_name);
           (error = table->file->ha_index_prev(table->record[0]))) {
        if (++idx >= target) break;
      }
      if (idx != target) {
        DBUG_PRINT("info", ("Row not found"));
        return true;
      }
    } else {
      DBUG_PRINT("info", ("Row not found"));
      return true;
    }

    if (error) {
      DBUG_PRINT("info", ("Row not found"));
      return true;
    }
  }
  DBUG_PRINT("info", ("Row found"));
  assert(match_key_prefix(record, schema_name, table_name, column_name));

  Json_wrapper wrapper;
  if (record.read_json(FIELD_HISTOGRAM_NO, &wrapper)) return true;

  Json_dom *json_dom = wrapper.to_dom();
  if (json_dom == nullptr ||
      json_dom->json_type() != enum_json_type::J_OBJECT) {
    return true;
  }

  const Json_object *json_object = down_cast<const Json_object *>(json_dom);
  histograms::Error_context context;
  *histogram = histograms::Histogram::json_to_histogram(
      &m_mem_root, schema_name.data(), table_name.data(), column_name.data(),
      *json_object, &context);

  return *histogram == nullptr;
}

bool History_table_persistor::shrink(History_table_access_context &table_ctx,
                                     const dd::String_type &schema_name,
                                     const dd::String_type &table_name,
                                     const dd::String_type &column_name) {
  DBUG_TRACE;

  TABLE *table = table_ctx.m_table;
  empty_record(table);
  dd::Raw_record record{table};

  if (store_key_prefix(record, schema_name, table_name, column_name)) {
    return true;
  }

  uchar user_key[MAX_KEY_LENGTH];
  key_copy(user_key, table->record[0], table->key_info,
           table->key_info->key_length);

  {
    int error = 0;
    /* Initialize the handle to sort by primary key. */
    if ((error = table->file->ha_index_init(0, true))) {
      table->file->print_error(error, MYF(0));
      DBUG_PRINT("info", ("ha_index_init error"));
      return true;
    }

    auto guard = create_scope_guard([table] { table->file->ha_index_end(); });

    /*
      Reversely traverse the records of the column, keep the last N records
      and delete the rest, where N depends on global system variable
      histogram_history_versions_limit. Because records are sorted by primary
      key, records with smaller version are deleted and records with larger
      version are retained.

      Because histogram_history_versions_limit will not be greater than 100,
      the traversal operation will not last long.
    */
    uint count = 0;
    for ((error = table->file->ha_index_read_map(table->record[0], user_key,
                                                 make_prev_keypart_map(history_key_parts),
                                                 HA_READ_PREFIX_LAST));
         !error &&
         match_key_prefix(record, schema_name, table_name, column_name);
         (error = table->file->ha_index_prev(table->record[0]))) {
      if (count++ >= histogram_history_versions_limit &&
          table->file->ha_delete_row(table->record[0])) {
        return false;
      }
    }
  }

  return false;
}

bool History_table_persistor::delete_stats(
    History_table_access_context &table_ctx, const dd::String_type &schema_name,
    const dd::String_type &table_name, const dd::String_type &column_name) {
  DBUG_TRACE;

  TABLE *table = table_ctx.m_table;
  empty_record(table);
  dd::Raw_record record{table};

  if (store_key_prefix(record, schema_name, table_name, column_name)) {
    return true;
  }

  uchar user_key[MAX_KEY_LENGTH];
  key_copy(user_key, table->record[0], table->key_info,
           table->key_info->key_length);

  {
    int error = 0;
    /* Initialize the handle to sort by primary key. */
    if ((error = table->file->ha_index_init(0, true))) {
      table->file->print_error(error, MYF(0));
      DBUG_PRINT("info", ("ha_index_init error"));
      return true;
    }

    auto guard = create_scope_guard([table] { table->file->ha_index_end(); });

    for ((error = table->file->ha_index_read_map(table->record[0], user_key,
                                                 make_prev_keypart_map(history_key_parts),
                                                 HA_READ_PREFIX_LAST));
         !error &&
         match_key_prefix(record, schema_name, table_name, column_name);
         (error = table->file->ha_index_prev(table->record[0]))) {
      if ((table_ctx.m_error = table->file->ha_delete_row(table->record[0]))) {
        return true;
      }
    }
  }

  return false;
}

bool History_table_persistor::rename_stats(
    History_table_access_context &table_ctx,
    const dd::String_type &old_schema_name,
    const dd::String_type &old_table_name,
    const dd::String_type &new_schema_name,
    const dd::String_type &new_table_name, const dd::String_type &column_name) {
  DBUG_TRACE;

  TABLE *table = table_ctx.m_table;
  empty_record(table);
  dd::Raw_record record{table};

  if (store_key_prefix(record, old_schema_name, old_table_name, column_name)) {
    return true;
  }

  uchar user_key[MAX_KEY_LENGTH];
  key_copy(user_key, table->record[0], table->key_info,
           table->key_info->key_length);

  {
    int error = 0;
    /* Initialize the handle to sort by primary key. */
    if ((error = table->file->ha_index_init(0, true))) {
      table->file->print_error(error, MYF(0));
      DBUG_PRINT("info", ("ha_index_init error"));
      return true;
    }

    auto guard = create_scope_guard([table] { table->file->ha_index_end(); });

    /* Traverse the records for same column, update matching rows. */
    for ((error = table->file->ha_index_read_map(table->record[0], user_key,
                                                 make_prev_keypart_map(history_key_parts),
                                                 HA_READ_PREFIX_LAST));
         !error &&
         match_key_prefix(record, old_schema_name, old_table_name, column_name);
         (error = table->file->ha_index_prev(table->record[0]))) {
      store_record(table, record[1]);

      if (store_key_prefix(record, new_schema_name, new_table_name,
                           column_name)) {
        table_ctx.m_error = 1;
        return true;
      }

      if ((table_ctx.m_error = table->file->ha_update_row(
               table->record[1], table->record[0])) != 0) {
        return true;
      }

      restore_record(table, record[1]);
    }
  }

  return false;
}

}  // namespace histograms

/*
   Copyright (c) 2014, 2022, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/opt_statistics.h"

#include <assert.h>
#include <algorithm>

#include "my_base.h"

#include "my_macros.h"
#include "sql/handler.h"
#include "sql/key.h"    // rec_per_key_t, KEY
#include "sql/table.h"  // TABLE
#include "sql/sql_class.h"
#include "sql/table.h"
#include "sql/tztime.h"
#include "sql/sql_lex.h"
#include "sql/mysqld.h"

#include <utility>
#include <algorithm>

using std::max;

/**
  This code for computing a guestimate for records per key is based on
  code in Optimize_table_order::find_best_ref().

  Assume that the first key part matches 1% of the file and that the
  whole key matches 10 (duplicates) or 1 (unique) records. For small
  tables, ensure there are at least ten different key values.  Assume
  also that more key matches proportionally more records. This gives
  the formula:

    records = a - (x-1)/(c-1)*(a-b)

  where

    b = records matched by whole key
    a = records matched by first key part (1% of all records?)
    c = number of key parts in key
    x = used key parts (1 <= x <= c)

  @todo Change Optimize_table_order::find_best_ref() to use this function.
*/

rec_per_key_t guess_rec_per_key(const TABLE *const table, const KEY *const key,
                                uint used_keyparts) {
  assert(used_keyparts >= 1);
  assert(used_keyparts <= key->actual_key_parts);
  assert(!key->has_records_per_key(used_keyparts - 1));

  const ha_rows table_rows = table->file->stats.records;

  /*
    Make an estimates for how many records the whole key will match.
    If there exists index statistics for the whole key we use this.
    If not, we assume the whole key matches ten records for a non-unique
    index and 1 record for a unique index.
  */
  rec_per_key_t rec_per_key_all;
  if (key->has_records_per_key(key->user_defined_key_parts - 1))
    rec_per_key_all = key->records_per_key(key->user_defined_key_parts - 1);
  else {
    if (key->actual_flags & HA_NOSAME)
      rec_per_key_all = 1.0f;  // Unique index
    else {
      rec_per_key_all = 10.0f;  // Non-unique index

      /*
        Assume the index contains at least ten unique values. Need to
        adjust the records per key estimate for small tables. For an
        empty table we assume records per key is 1.
      */
      rec_per_key_all =
          std::min(rec_per_key_all, max(rec_per_key_t(table_rows) / 10, 1.0f));
    }
  }

  rec_per_key_t rec_per_key;

  /* rec_per_key estimate for first key part (1% of records). */
  const rec_per_key_t rec_per_key_first = table_rows * 0.01f;

  if (rec_per_key_first < rec_per_key_all) {
    rec_per_key = rec_per_key_all;
  } else {
    if (key->user_defined_key_parts > 1) {
      /* See formula above. */
      rec_per_key =
          rec_per_key_first - (rec_per_key_t(used_keyparts - 1) /
                               (key->user_defined_key_parts - 1)) *
                                  (rec_per_key_first - rec_per_key_all);
    } else {
      /* Single column index. */
      if (key->actual_flags & HA_NOSAME)
        rec_per_key = 1.0f;                             /* Unique index */
      else
        rec_per_key = rec_per_key_first;                /* Non-unique index */
    }

    assert(rec_per_key >= rec_per_key_all);
  }

  return rec_per_key;
}

/**
  A simple wrapper around a RW lock:
  Grabs the lock in the CTOR, releases it in the DTOR.
  The lock may be NULL, in which case this is a no-op.

  Based on Mutex_lock from include/mutex_lock.h
  Copied from srv_session.cc
*/
class Auto_rw_lock_read
{
public:
  explicit Auto_rw_lock_read(mysql_rwlock_t *lock) : rw_lock(NULL)
  {
    if (lock && 0 == mysql_rwlock_rdlock(lock))
      rw_lock = lock;
  }

  ~Auto_rw_lock_read()
  {
    if (rw_lock)
      mysql_rwlock_unlock(rw_lock);
  }
private:
  mysql_rwlock_t *rw_lock;

  Auto_rw_lock_read(const Auto_rw_lock_read&);         /* Not copyable. */
  void operator=(const Auto_rw_lock_read&);            /* Not assignable. */
};


class Auto_rw_lock_write
{
public:
  explicit Auto_rw_lock_write(mysql_rwlock_t *lock) : rw_lock(NULL)
  {
    if (lock && 0 == mysql_rwlock_wrlock(lock))
      rw_lock = lock;
  }

  ~Auto_rw_lock_write()
  {
    if (rw_lock)
      mysql_rwlock_unlock(rw_lock);
  }
private:
  mysql_rwlock_t *rw_lock;

  Auto_rw_lock_write(const Auto_rw_lock_write&);        /* Non-copyable */
  void operator=(const Auto_rw_lock_write&);            /* Non-assignable */
};

sql_statistics::sql_statistics()
{
  mysql_rwlock_init(0, &m_lock);
}

sql_statistics::~sql_statistics()
{
  mysql_rwlock_destroy(&m_lock);

  std::map<uchar*, sql_statistics_info*, digest_compare>::iterator iter =
    m_map.begin();

  while (iter != m_map.end()) {
    delete iter->second;
    iter->second = NULL;
    m_map.erase(iter++);
  }

  m_map.clear();
}

bool sql_statistics::is_satisfied_sql_command(enum enum_sql_command sql_command)
{
  if (sql_command == SQLCOM_DELETE ||
      sql_command == SQLCOM_UPDATE ||
      sql_command == SQLCOM_INSERT_SELECT)
    return true;

  return false;
}

/**
  Update info in sql_statistics after excuting a query successfully.

  @param thd  Thread handle.
  @param affected_rows affected_rows in my_ok(), the affected_rows of the query.
*/
void sql_statistics::update_info(THD *thd, ulonglong affected_rows)
{
  if (!cdb_sql_statistics)
    return;

  if (!thd->lex || !is_satisfied_sql_command(thd->lex->sql_command))
    return;

  uchar digest[PARSER_SERVICE_DIGEST_LENGTH];
  if (mysql_parser_get_statement_digest(thd, digest))
    return;

  Auto_rw_lock_write lock(&m_lock);
  std::map<uchar*, sql_statistics_info*, digest_compare>::iterator iter =
    m_map.find(digest);
  sql_statistics_info *info = NULL;
  if (iter == m_map.end()) {
    info = new sql_statistics_info();
    memcpy(info->digest, digest, PARSER_SERVICE_DIGEST_LENGTH);
    compute_digest_text(&thd->m_digest->m_digest_storage, &info->digest_text);
    info->sql_command = thd->lex->sql_command;
    time(&info->first_update_timestamp);

    m_map.insert(std::make_pair(info->digest, info));
  } else {
    info = (sql_statistics_info*)iter->second;
  }

  time(&info->last_update_timestamp);
  info->execute_count++;
  info->total_affected_rows += affected_rows;
  info->last_affected_rows = affected_rows;
  info->aver_affected_rows = info->total_affected_rows/info->execute_count;
}

/**
  Check if the sql have the possibility to binlog statement format.

  For the reason, before the first mark after decide_logging_format,
  the value alway is true. It means every SQL would try at the first time,
  then they would be marked correctly.

  @param thd  Thread handle.
  @return true if the sql have the possibility to binlog statement format.
*/
bool sql_statistics::stmt_binlog_format_if_possible(THD *thd)
{
  assert(thd->lex->sql_command == SQLCOM_DELETE ||
              thd->lex->sql_command == SQLCOM_UPDATE ||
              thd->lex->sql_command == SQLCOM_INSERT_SELECT);

  if (!cdb_optimize_large_trans_binlog)
    return false;

  uchar digest[PARSER_SERVICE_DIGEST_LENGTH];
  if (mysql_parser_get_statement_digest(thd, digest))
    return false;

  Auto_rw_lock_read lock(&m_lock);
  std::map<uchar*, sql_statistics_info*, digest_compare>::iterator iter =
    m_map.find(digest);

  if (iter == m_map.end())
    return false;

  sql_statistics_info *info = (sql_statistics_info*)iter->second;

  time(&info->last_access_timestamp);

  if ((info->stmt_binlog_format_if_possible == false) ||
      (info->aver_affected_rows <
       cdb_optimize_large_trans_binlog_aver_affected_rows_threshold &&
       info->last_affected_rows <
       cdb_optimize_large_trans_binlog_last_affected_rows_threshold))
    return false;

  return true;
}

/**
  Mark if the sql have the possibility to binlog statement format.

  For the reason, before the first mark after decide_logging_format,
  the value alway is true. It means every SQL would try at the first time,
  then they would be marked correctly.

  @param thd  Thread handle.
*/
void sql_statistics::mark_non_stmt_binlog_of_sql_digest(THD *thd)
{
  if (!cdb_optimize_large_trans_binlog)
    return;

  if (!is_satisfied_sql_command(thd->lex->sql_command))
    return;

  /*
    After_decide_logging_format, the binlog format would be known.
    If the the try was happened before,
    we would mark stmt_binlog_format_if_possible false. Otherwise,
    nothing would be done.
  */
  if (thd->tx_isolation ==
      (enum_tx_isolation)thd->variables.transaction_isolation &&
      thd->variables.binlog_format == thd->get_save_binlog_format())
    return;

  if (!thd->is_current_stmt_binlog_format_row())
    return;

  uchar digest[PARSER_SERVICE_DIGEST_LENGTH];
  if (mysql_parser_get_statement_digest(thd, digest))
    return;

  Auto_rw_lock_write lock(&m_lock);
  std::map<uchar*, sql_statistics_info*, digest_compare>::iterator iter =
    m_map.find(digest);

  if (iter != m_map.end()) {
    sql_statistics_info *info = (sql_statistics_info*)iter->second;
    info->stmt_binlog_format_if_possible = false;
  }
}

/**
  The backgroud thread to clean extra sql_statistics_info that
  exceed cdb_sql_statistics_info_threshold.
*/
void sql_statistics::clear_expired_info_by_bg_thread()
{

  time_t cur_time = 0;
  time(&cur_time);

  /* clear expired info per 5 seconds */
  if (cur_time % 5 != 0)
    return;

  std::vector< std::pair<uchar*, sql_statistics_info*> > info_vec;
  ulonglong extra_info_count = 0;

  /*
    Avoid to sort costs too long, here release the lock before sorting,
    then acquire the lock after sorting.
  */
  {
    Auto_rw_lock_read lock(&m_lock);
    if (m_map.size() == 0)
      return;

    ulonglong tmp_cdb_sql_statistics_info_threshold =
      cdb_sql_statistics_info_threshold;
    if (m_map.size() <= tmp_cdb_sql_statistics_info_threshold)
      return;

    extra_info_count = m_map.size() - tmp_cdb_sql_statistics_info_threshold;
    info_vec.insert(info_vec.begin(), m_map.begin(), m_map.end());
  }

  std::sort(info_vec.begin(), info_vec.end(), timestamp_compare());

  {
    Auto_rw_lock_write lock(&m_lock);
    for (ulonglong i = 0; i < extra_info_count; i++) {
      uchar *to_be_clean = info_vec[i].first;
      sql_statistics_info *info = info_vec[i].second;
      m_map.erase(to_be_clean);
      delete info;
    }
  }
}

std::string sql_statistics::print_digest(const unsigned char *digest)
{
  const size_t string_size= PARSER_SERVICE_DIGEST_LENGTH * 2;
  char digest_str[string_size + sizeof('\0')];
  for (int i= 0; i < PARSER_SERVICE_DIGEST_LENGTH; ++i)
    snprintf(digest_str + i * 2, string_size, "%02x", digest[i]);
  return digest_str;
}

/**
  Fill handlerton based CDB_SQL_STATISTICS tables.

  @param thd  Thread handle.
  @TABLE table  Information Schema tables to fill
  @return Operation status
*/
int sql_statistics::fill_i_s(THD* thd, TABLE *table)
{
  Auto_rw_lock_read lock(&m_lock);

  MYSQL_TIME time;
  CHARSET_INFO *cs= system_charset_info;
  std::map<uchar*, sql_statistics_info*, digest_compare>::iterator iter =
    m_map.begin();

  while (iter != m_map.end()) {
    sql_statistics_info *info = (sql_statistics_info*)iter->second;
    std::string str = print_digest(info->digest);
    table->field[0]->store(str.c_str(), str.size(), cs);
    table->field[1]->store(info->digest_text.ptr(),
                           info->digest_text.length(), cs);

    str = sql_statement_names[info->sql_command].str;
    std::transform(str.begin(), str.end(),str.begin(), ::toupper);
    table->field[2]->store(str.c_str(),
                           sql_statement_names[info->sql_command].length, cs);

    thd->variables.time_zone->gmt_sec_to_TIME(
        &time, (my_time_t) info->first_update_timestamp);
    table->field[3]->store_time(&time);
    table->field[3]->set_notnull();

    thd->variables.time_zone->gmt_sec_to_TIME(
        &time, (my_time_t) info->last_update_timestamp);
    table->field[4]->store_time(&time);
    table->field[4]->set_notnull();

    thd->variables.time_zone->gmt_sec_to_TIME(
        &time, (my_time_t) info->last_access_timestamp);
    table->field[5]->store_time(&time);
    table->field[5]->set_notnull();

    table->field[6]->store(info->execute_count);
    table->field[7]->store(info->total_affected_rows);
    table->field[8]->store(info->aver_affected_rows);
    table->field[9]->store(info->last_affected_rows);
    char field10[8] = {0};
    info->stmt_binlog_format_if_possible ? strcpy(field10, "TRUE") :
                                           strcpy(field10, "FALSE");
    table->field[10]->store(field10, strlen(field10), cs);
    if (schema_table_store_record(thd, table)) {
        return 1;
    }
    iter++;
  }

  return 0;
}

/**
  Enables digests in the parser state if any feature needs it.

  @param ps This parser state will have digests enabled if any plugin
  needs it.
*/
void enable_digest_if_any_feature_needs_it(Parser_state *ps)
{
  if (cdb_optimize_large_trans_binlog || cdb_sql_statistics) {
    ps->m_input.m_compute_digest= true;
  }
}

static sql_statistics* _sql_statistics = NULL;

void sql_statistics_init()
{
  _sql_statistics = new sql_statistics();
}

void sql_statistics_deinit()
{
  if (_sql_statistics)
    delete _sql_statistics;

  _sql_statistics = NULL;
}

void sql_statistics_update_info(THD *thd, ulonglong affected_rows)
{
  _sql_statistics->update_info(thd, affected_rows);
}

bool sql_statistics_stmt_binlog_format_if_possible(THD *thd)
{
  return _sql_statistics->stmt_binlog_format_if_possible(thd);
}

void sql_statistics_mark_non_stmt_binlog_of_sql_digest(THD *thd)
{
  _sql_statistics->mark_non_stmt_binlog_of_sql_digest(thd);
}

void sql_statistics_clear_expired_info_by_bg_thread()
{
  _sql_statistics->clear_expired_info_by_bg_thread();
}

int sql_statistics_fill_i_s(THD* thd, TABLE *table)
{
  return _sql_statistics->fill_i_s(thd, table);
}

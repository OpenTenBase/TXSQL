/* Copyright (c) 2020, tencent and/or its affiliates. All rights reserved.
 */

/**
  @file storage/perfschema/table_deadlock_history.cc
  Table DEADLOCK_HISTORY (implementation).
*/

#include "sql/deadlock_history.h"
#include "m_ctype.h"
#include "sql/handler.h"
#include "sql/tztime.h"

/**
  A simple wrapper around a RW lock:
  Grabs the lock in the CTOR, releases it in the DTOR.
  The lock may be NULL, in which case this is a no-op.

  Based on Mutex_lock from include/mutex_lock.h
  Copied from opt_statistis.cc
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

/** Globally unique deadlock_history object */
static deadlock_history *_deadlock_history = nullptr;
/** Used to get the next deadlock loop id. */
std::atomic<uint> deadlock_history::m_group_id(0);

void row_deadlock::init_index_name(
    const char *index_name_str, uint index_name_len) {
  m_index_name_len = std::min(DEADLOCK_MAX_NAME_LEN, index_name_len);
  memcpy(m_index_name, index_name_str, m_index_name_len);
}

void row_deadlock::init_table_name(
    const char *table_name_str, uint table_name_len){
  m_table_name_len = std::min(DEADLOCK_MAX_NAME_LEN, table_name_len);
  memcpy(m_table_name, table_name_str, m_table_name_len);
}

void row_deadlock::init_query(const char *query_str, uint query_len) {
  m_query_len = std::min(DEADLOCK_MAX_QUERY_LEN, query_len);
  memcpy(m_query, query_str, m_query_len);
}

void row_deadlock::init_lock_data(
    const char *lock_data_str, uint lock_data_len){
  m_lock_data_len = std::min(DEADLOCK_MAX_LOCKDATA_LEN, lock_data_len);
  memcpy(m_lock_data, lock_data_str, m_lock_data_len);
}

void row_deadlock::init_wait_lock_data(
    const char *wait_lock_data_str, uint wait_lock_data_len){
  m_wait_lock_data_len = std::min(DEADLOCK_MAX_LOCKDATA_LEN, wait_lock_data_len);
  memcpy(m_wait_lock_data, wait_lock_data_str, m_wait_lock_data_len);
}

void row_deadlock::init_host(const char *host_str, uint host_len) {
  m_host_len = std::min(DEADLOCK_MAX_NAME_LEN, host_len);
  memcpy(m_host, host_str, m_host_len);
}

void row_deadlock::init_user(const char *user_str, uint user_len) {
  m_user_len = std::min(DEADLOCK_MAX_NAME_LEN, user_len);
  memcpy(m_user, user_str, m_user_len);
}

void row_deadlock::init_lock_type(
    const char *lock_type_str, uint lock_type_len) {
  m_lock_type_len = std::min(DEADLOCK_MAX_NAME_LEN, lock_type_len);
  memcpy(m_lock_type, lock_type_str, m_lock_type_len);
}

void row_deadlock::init_lock_mode(
    const char *lock_mode_str, uint lock_mode_len) {
  m_lock_mode_len = std::min(DEADLOCK_MAX_NAME_LEN, lock_mode_len);
  memcpy(m_lock_mode, lock_mode_str, m_lock_mode_len);
}

/** Store a deadlock record in the list m_deadlock_list. */
bool deadlock_history::store(const row_deadlock &deadlock_record) {
  Auto_rw_lock_write lock(&m_lock);
  row_deadlock *record = new row_deadlock;
  memcpy(record, &deadlock_record, sizeof(row_deadlock));
  m_deadlock_list.push_back(record);
  if (m_deadlock_list.size() > m_full_size) {
    row_deadlock *front_record = m_deadlock_list.front();
    m_deadlock_list.pop_front();
    delete front_record;
  }
  return false;
}

/** Initialize the globally unique object deadlock_history. */
int deadlock_history::fill_i_s(THD *thd ,TABLE *table) {
  Auto_rw_lock_read lock(&m_lock);

  CHARSET_INFO *cs= system_charset_info;
  MYSQL_TIME time;
  for (auto iter : m_deadlock_list) {
    table->field[FIELD_GROUP_ID]->store(iter->m_group_id);
    table->field[FIELD_LOOP_ID]->store(iter->m_loop_id);
    table->field[FIELD_THREAD_ID]->store(iter->m_thread_id);
    table->field[FIELD_TRX_ID]->store(iter->m_trx_id);
    table->field[FIELD_LOCK_TYPE]->store(
        iter->m_lock_type, iter->m_lock_type_len, cs);
    table->field[FIELD_LOCK_MODE]->store(
        iter->m_lock_mode, iter->m_lock_mode_len, cs);
    table->field[FIELD_INDEX_NAME]->store(
        iter->m_index_name, iter->m_index_name_len, cs);
    table->field[FIELD_TABLE_NAME]->store(
        iter->m_table_name, iter->m_table_name_len, cs);
    table->field[FIELD_QUERY]->store(
        iter->m_query, iter->m_query_len, cs);
    table->field[FIELD_LOCK_DATA]->store(
        iter->m_lock_data, iter->m_lock_data_len, cs);
    table->field[FIELD_WAIT_LOCK_DATA]->store(
        iter->m_wait_lock_data, iter->m_wait_lock_data_len, cs);
    table->field[FIELD_HOST]->store(
        iter->m_host, iter->m_host_len, cs);
    table->field[FIELD_USER]->store(
        iter->m_user, iter->m_user_len, cs);
    thd->variables.time_zone->gmt_sec_to_TIME(
        &time, (my_time_t) iter->trx_start_time);
    table->field[FIELD_TRX_START_TIME]->store_time(&time);
    thd->variables.time_zone->gmt_sec_to_TIME(
        &time, (my_time_t) iter->m_detection_time);
    table->field[FIELD_DETECTION_TIME]->store_time(&time);
    if (schema_table_store_record(thd, table)) {
      return 1;
    }
  }
  return 0;
}

/** Resize m_deadlock_list */
void deadlock_history::resize(uint size) {
  if (size == m_full_size)
    return;

  Auto_rw_lock_write lock(&m_lock);

  if (size < m_deadlock_list.size()) {
    row_deadlock *deadlock_record;
    for (uint i = 0; i < size - m_deadlock_list.size(); i++) {
      deadlock_record = m_deadlock_list.front();
      m_deadlock_list.pop_front();
      delete deadlock_record;
    }
  }
  m_full_size = size;
}

/** Clear m_deadlock_list and release memory
occupied by historical deadlocks. */
void deadlock_history::clear_history() {
  Auto_rw_lock_write lock(&m_lock);

  for (auto iter : m_deadlock_list) {
    delete iter;
  }
  m_deadlock_list.clear();
}

/** Get the id of the next deadlock loop. */
uint deadlock_history::get_next_deadlock_group_id() {
  return m_group_id.fetch_add(1);
}

void deadlock_history_init()
{
  _deadlock_history = new deadlock_history();
}

bool store_deadlock_record(const row_deadlock &deadlock_reocrd) {
  return _deadlock_history->store(deadlock_reocrd);
}

int deadlock_history_fill_i_s(THD *thd ,TABLE *table) {
  return _deadlock_history->fill_i_s(thd, table);
}

void deadlock_history_deinit() {
  if (!_deadlock_history) return;

  _deadlock_history->clear_history();
  delete _deadlock_history;
}

void deadlock_history_resize(uint size) {
  _deadlock_history->resize(size);
}
/* Copyright (c) 2020, tencent and/or its affiliates. All rights reserved.
 */

#ifndef DEADLOCK_HISTORY_H
#define DEADLOCK_HISTORY_H

/**
  @file storage/perfschema/table_deadlock_history.h
  DEADLOCK_HISTORY (declarations).
*/

#include "my_inttypes.h"
#include <list>
#include "sql/table.h"  // TABLE
#include "sql/sql_class.h"

#include "sql/sql_lex.h"

/**
  @addtogroup deadlock_histroy
  @{
*/

static constexpr uint DEADLOCK_MAX_NAME_LEN = 64;
static constexpr uint DEADLOCK_MAX_QUERY_LEN = 256;
static constexpr uint DEADLOCK_MAX_LOCKDATA_LEN = 256;

/** the row struct in table deadlock_history. */
struct row_deadlock {
  ulonglong m_group_id;
  ulonglong m_loop_id;
  ulonglong m_thread_id;
  ulonglong m_trx_id;
  char m_lock_type[DEADLOCK_MAX_NAME_LEN];
  uint m_lock_type_len;
  char m_lock_mode[DEADLOCK_MAX_LOCKDATA_LEN];
  uint m_lock_mode_len;
  char m_index_name[DEADLOCK_MAX_NAME_LEN];
  uint m_index_name_len;
  char m_table_name[DEADLOCK_MAX_NAME_LEN];
  uint m_table_name_len;
  char m_query[DEADLOCK_MAX_QUERY_LEN];
  uint m_query_len;
  ulonglong query_id;
  char m_lock_data[DEADLOCK_MAX_LOCKDATA_LEN];
  uint m_lock_data_len;
  char m_wait_lock_data[DEADLOCK_MAX_LOCKDATA_LEN];
  uint m_wait_lock_data_len;
  char m_host[DEADLOCK_MAX_NAME_LEN];
  uint m_host_len;
  char m_user[DEADLOCK_MAX_NAME_LEN];
  uint m_user_len;
  time_t trx_start_time;
  time_t m_detection_time;

  void init_index_name(const char *index_name_str, uint index_name_len);
  void init_table_name(const char *table_name_str, uint table_name_len);
  void init_query(const char *query_str, uint query_len);
  void init_lock_data(const char *lock_data_str, uint lock_data_len);
  void init_wait_lock_data(
      const char *wait_lock_data_str, uint wait_lock_data_len);
  void init_host(const char *host_str, uint host_len);
  void init_user(const char *user_str, uint user_len);
  void init_lock_type(const char *lock_type_str, uint lock_type_len);
  void init_lock_mode(const char *lock_mode_str, uint lock_mode_len);
};

class deadlock_history {
 public:
  /** Store a deadlock record in the list m_deadlock_list.
  Hold the write lock. */
  bool store(const row_deadlock &deadlock_reocrd);
  /** Store the deadlock information in m_deadlock_list
  into a memory table for display */
  int fill_i_s(THD *thd ,TABLE *table);
  /** Resize m_deadlock_list */
  void resize(uint size);
  /** Clear m_deadlock_list and release memory
  occupied by historical deadlocks. */
  void clear_history();
  /** Get the id of the next deadlock loop. */
  static uint get_next_deadlock_group_id();
  enum Field_type {
    FIELD_GROUP_ID,
    FIELD_LOOP_ID,
    FIELD_THREAD_ID,
    FIELD_TRX_ID,
    FIELD_LOCK_TYPE,
    FIELD_LOCK_MODE,
    FIELD_INDEX_NAME,
    FIELD_TABLE_NAME,
    FIELD_QUERY,
    FIELD_LOCK_DATA,
    FIELD_WAIT_LOCK_DATA,
    FIELD_HOST,
    FIELD_USER,
    FIELD_TRX_START_TIME,
    FIELD_DETECTION_TIME,
    NUMBER_OF_FIELDS
  };

  static std::atomic<uint> m_group_id;

 private:
  std::list<row_deadlock *> m_deadlock_list;
  uint m_full_size;
  mysql_rwlock_t m_lock;
};

/** Store a deadlock record in the list m_deadlock_list. */
bool store_deadlock_record(const row_deadlock &deadlock_reocrd);
/** Initialize the globally unique object deadlock_history. */
void deadlock_history_init();
/** Store the deadlock information into the memory table for display */
int deadlock_history_fill_i_s(THD *thd ,TABLE *table);
/** Clear the memory occupied by historical deadlocks. */
void deadlock_history_deinit();
/** Resize m_deadlock_list */
void deadlock_history_resize(uint resize);

/** @} */

#endif
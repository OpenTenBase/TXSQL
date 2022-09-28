/* Copyright (c) 2006, 2014, Oracle and/or its affiliates. All rights reserved.

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

#ifndef STATISTICS_MANAGER_INCLUDED
#define STATISTICS_MANAGER_INCLUDED


#include <sys/types.h>
#include <time.h>
#include <atomic>
#include <vector>
#include <memory>
#include <set>
#include <string>
#include "lex_string.h"
#include "priority_queue.h"          // Priority_queue
#include "my_psi_config.h"
#include "my_time.h"
#include "my_alloc.h"
#include "my_icp.h"
#include "mysql_com.h"
#include "mysql/components/services/bits/mysql_cond_bits.h"
#include "mysql/components/services/bits/mysql_mutex_bits.h"
#include "mysql/components/services/bits/psi_cond_bits.h"
#include "mysql/components/services/bits/psi_mutex_bits.h"
#include "mysql/components/services/bits/psi_stage_bits.h"
#include "mysql/psi/mysql_thread.h"
#include "sql/malloc_allocator.h"
#include "sql/mem_root_allocator.h"
#include "sql/histograms/histogram.h"
#include "sql/table.h"

using namespace histograms;
class THD;

#ifdef HAVE_PSI_INTERFACE
extern PSI_mutex_key key_LOCK_stats_task_queue;
extern PSI_mutex_key key_LOCK_stats_task_element;

extern PSI_thread_key key_thread_statistics_manager;
extern PSI_thread_key key_thread_task_worker;

extern PSI_cond_key key_task_manager_COND_state;
extern PSI_cond_key key_task_element_COND_state;
#endif /* HAVE_PSI_INTERFACE */

/* Always defined, for SHOW PROCESSLIST. */
extern PSI_stage_info stage_waiting_for_task_to_stop;
extern PSI_stage_info stage_waiting_for_manager_to_stop;

enum enum_on_task_completion {
    ON_PENDING = 0,
    ON_DOING = 1,
    FINISHED_TASK = 2,
    DELETED_TASK = 3,
    FAILED_TASK = 4,
    TO_BE_DELETED = 5,
};

typedef std::vector<Field *, Histogram_key_allocator<Field *>> column_name_set;

class Statistics_task_element {
 protected:
  MEM_ROOT mem_root;
 public:
  std::atomic<int32> m_status;
  /** Column DB. */
  char m_dbname[NAME_LEN];
  /** Length in bytes of @c m_dbname. */
  uint m_dbname_length;
  /** Column Table. */
  char m_tablename[NAME_LEN];
  /** Length in bytes of @c m_tablename. */
  uint m_tablename_length;
  int m_num_buckets;
  THD *thd;

  my_time_t m_last_executed_at;
  my_time_t m_finished_at;
  my_time_t m_execute_at;
  my_time_t m_create_at;

  /** counts statistics_worker_thread failures and apply_statistics_thread failures. */
  uint m_execution_count;

  /** LOCK_task_queue is the mutex which protects the access to the queue. */
  mysql_mutex_t LOCK_task_element;
  mysql_cond_t COND_task_state;

  using vector_type = std::vector<std::string, Mem_root_allocator<std::string>>;

  vector_type m_columns;

  Statistics_task_element();

  ~Statistics_task_element();

  bool operator==(const Statistics_task_element &other) const {
    if (m_dbname_length != other.m_dbname_length ||
        m_tablename_length != other.m_tablename_length) {
      return false;
    } else if (strncmp(m_dbname, other.m_dbname, m_dbname_length) != 0) {
      return false;
    } else if (strncmp(m_tablename, other.m_tablename, m_tablename_length) != 0) {
      return false;
    }
    return true;
  }

  void set_base_info(const char* db_name,
      const int db_name_length, const char* table_name,
      const int table_name_length, const int num_buckets,
      const my_time_t start_time);

  bool add_columns(const std::vector<std::string> &columns, const int new_num_bucket);

  bool drop_columns(const columns_set &columns);

  bool kill_task();

  bool do_apply_statistics_task(THD *thd);

  bool update_histogram_using_data(char* json_str MY_ATTRIBUTE((unused)),
      const char* field_name, const char **errmsg);

  void refresh_max_buckets(const int new_num_bucket);

  void lock_data(const char *func, uint line);

  void unlock_data(const char *func, uint line);

  void cond_wait(THD *thd, struct timespec *abstime,
                 const PSI_stage_info *stage, const char *src_func,
                 const char *src_file, uint src_line);
};

struct Task_queue_less {
  /// Maps compare function to strict weak ordering required by Priority_queue.
  bool operator()(Statistics_task_element *left, Statistics_task_element *right) {
    if (left->m_status == ON_PENDING) {
      return pending_task_element_compare_q(left, right) > 0;
    }
    return running_task_element_compare_q(left, right) > 0;
  }

  /**
    Compares the m_create_at members of two ON_PENDING Statistics_task_element
    instances. Used as compare operator for the prioritized queue when
    shifting tasks inside.

    SYNOPSIS
      pending_task_element_compare_q()
      @param left     First Statistics_task_element object
      @param right    Second Statistics_task_element object

    @retval
     -1   left->m_create_at < right->m_create_at
      0   left->m_create_at == right->m_create_at
      1   left->m_create_at > right->m_create_at

    @remark
      m_create_at.second_part is not considered during comparison
  */
  int pending_task_element_compare_q(Statistics_task_element *left,
                                    Statistics_task_element *right) {
    my_time_t lhs = left->m_create_at;
    my_time_t rhs = right->m_create_at;
    return (lhs < rhs ? -1 : (lhs > rhs ? 1 : 0));
  }

  /**
    Compares the m_execute_at members of two ON_DOING/FINISHED
    Statistics_task_element instances. Used as compare operator for the
    prioritized queue when shifting tasks inside.

    SYNOPSIS
      running_task_element_compare_q()
      @param left     First Statistics_task_element object
      @param right    Second Statistics_task_element object

    @retval
     -1   left->m_execute_at < right->m_execute_at
      0   left->m_execute_at == right->m_execute_at
      1   left->m_execute_at > right->m_execute_at

    @remark
      m_execute_at.second_part is not considered during comparison
  */
  int running_task_element_compare_q(Statistics_task_element *left,
                                    Statistics_task_element *right) {
    my_time_t lhs = left->m_execute_at;
    my_time_t rhs = right->m_execute_at;
    return (lhs < rhs ? -1 : (lhs > rhs ? 1 : 0));
  }
};

/**
  Queue of active tasks awaiting execution.
*/

class Statistic_task_queue {
 public:
  Statistic_task_queue();
  ~Statistic_task_queue();

  bool init_queue();

  /* Methods for queue management follow */
  Statistics_task_element* top_queue();

  bool pop_queue();

  Statistics_task_element* top_and_pop_queue();

  bool push(Statistics_task_element* new_task);

  bool drop_task(const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, bool direct_drop = false);

  uint size() {
    return queue.size();
  }

  void get_all_tasks_for_display(std::vector<Statistics_task_element*> &ret);

  bool add_columns_to_task(const char* db_name,
      const int db_name_length, const char* table_name,
      const int table_name_length, const std::vector<std::string> &columns,
      const int num_buckets, const my_time_t start_time);

  Statistics_task_element* drop_columns_from_task(const char* db_name,
      const int db_name_length, const char* table_name,
      const int table_name_length, const columns_set &columns);

  /* The sorted queue with the Statistics_task_element objects */
  typedef Priority_queue<Statistics_task_element *,
                 std::vector<Statistics_task_element *,
                             Malloc_allocator<Statistics_task_element *>>,
                 Task_queue_less> queue_type;

  typedef typename queue_type::const_iterator queue_iterator;

  void empty_queue();

 private:
  void deinit_queue();
  /* helper functions for working with mutexes & conditionals */
  void lock_data(const char *func, uint line);

  void unlock_data(const char *func, uint line);

  /* LOCK_task_queue is the mutex which protects the access to the queue. */
  mysql_mutex_t LOCK_task_queue;

  queue_type queue;

  bool mutex_queue_data_locked;
};


class Statistics_manager {
 public:
  static const char* errmsg;

  static bool start();

  static bool stop(bool need_lock = true);

  static bool init();

  static void init_mutexes();

  static void deinit();

  /**
    Need to be public because has to be called from the function
    passed to my_thread_create.
  */
  static bool run(THD *thd);

  static void cond_wait(THD *thd, struct timespec *abstime,
                 const PSI_stage_info *stage, const char *src_func,
                 const char *src_file, uint src_line);

  static bool retry_task(Statistics_task_element *failed_task);

  static bool re_apply_task(Statistics_task_element *failed_task);

  static bool is_running();

  static bool set_to_running(THD *thd);

  static bool set_to_initialized(THD *thd);

  static bool apply_histograms();

  static bool check_histograms_status(bool force_kill = false);

  static bool add_columns_to_task(const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, const column_name_set &columns,
    const int num_buckets, const my_time_t start_time);

  static bool auto_add_statistics_task(const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, const std::vector<std::string> &columns,
    const int num_buckets, const my_time_t start_time);

  static void drop(const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, bool is_rename,
    const columns_set &columns);

  static inline uint32 worker_count() {
    return workers;
  }

  static inline void inc_worker_count() {
    workers++;
  }

  static inline void dec_worker_count() {
    if (workers) workers--;
  }

  /*
    INITIALIZED - queues created
    RUNNING - manager thread created
    STOPPING - requested shutdown
  */
  enum enum_state { UNINITIALIZED = 0, INITIALIZED, RUNNING, STOPPING };
  /* This is the current status of the life-cycle of the manager. */
  static std::atomic<int32> state;

  static THD *manager_thd;

  /* store all tasks in vector ret */
  static void get_all_tasks_for_display(THD *thd);

  static int stats_task_fill_i_s(THD* thd, TABLE *table);
 private:
  /* helper functions */
  static bool execute_top(Statistics_task_element *task);

  // TODO: add dd for persistence
  static Statistic_task_queue *statistics_task_pending_queue;
  static Statistic_task_queue *statistics_task_running_queue;
  static Statistic_task_queue *statistics_task_apply_queue;
  static std::atomic<uint32> workers;
};

#endif /* STATISTICS_MANAGER_INCLUDED */

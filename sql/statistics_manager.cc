/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/*
 * statistics_manager.cc
 * This thread manages auto statistic tasks.
 */
#include "sql/statistics_manager.h"

#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <atomic>
#include <memory>

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_systime.h"
#include "my_thread.h"  // my_thread_t
#include "thr_mutex.h"
#include "scope_guard.h"         // create_scope_guard
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/bits/mysql_cond_bits.h"
#include "mysql/components/services/bits/mysql_mutex_bits.h"
#include "mysql/thread_pool_priv.h" // thd_set_net_read_write
#include "mysql_com.h"
#include "mysql_time.h"
#include "mysqld_error.h"
#include "sql_string.h"
#include "sql/log.h"
#include "sql/mysqld.h"
#include "sql/sql_parse.h"
#include "sql/lock.h"            // lock_object_name
#include "sql/malloc_allocator.h"
#include "sql/mdl.h"
#include "sql/psi_memory_key.h"  // key_memory_statistics_manager
#include "sql/sql_class.h"       // THD
#include "sql/sql_lex.h"
#include "sql-common/json_dom.h"        // Json_*
#include "sql/item_json_func.h"  // parse_json
#include "sql/protocol.h" // show
#include "sql/histograms/histogram.h"    // Histogram, Histogram_comparator
#include "sql/histograms/history_table_access.h"  // History_table_persistor
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/sql_base.h"   // close_thread_tables,open_and_lock_tables, etc
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_rollback_stmt, trans_commit_stmt
#include "sql/sql_system_table_check.h"  // System_table_intact
#include <sql/current_thd.h>
#include <sql/mysqld_thd_manager.h>

using namespace histograms;

#define TASK_QUEUE_INITIAL_SIZE 300
#define COLUMN_QUEUE_MAX_SIZE 128

#define LOCK_TASK_DATA() lock_data(__func__, __LINE__)
#define UNLOCK_TASK_DATA() unlock_data(__func__, __LINE__)

#ifdef HAVE_PSI_INTERFACE
PSI_mutex_key key_LOCK_stats_task_element;
PSI_mutex_key key_LOCK_stats_task_queue;

static PSI_mutex_info all_tasks_mutexes[]=
{
  { &key_LOCK_stats_task_queue, "LOCK_statistics_tasks_queue", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
  { &key_LOCK_stats_task_element, "LOCK_statistics_tasks_element", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}
};

PSI_cond_key key_task_manager_COND_state;
PSI_cond_key key_task_element_COND_state;

static PSI_cond_info all_tasks_conds[] = {
    {&key_task_manager_COND_state, "COND_manager_state",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_task_element_COND_state, "COND_task_state",
    PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
};

PSI_thread_key key_thread_statistics_manager;
PSI_thread_key key_thread_task_worker;

static PSI_thread_info all_tasks_threads[] = {
    {&key_thread_statistics_manager, "statistics_manager", "stat_manager",
     PSI_FLAG_USER, 0, PSI_DOCUMENT_ME},
    {&key_thread_task_worker, "task_worker", "task_worker",
     0, 0, PSI_DOCUMENT_ME}};
#endif /* HAVE_PSI_INTERFACE */

PSI_stage_info stage_waiting_for_worker_to_stop = {
    0, "Waiting for the worker to stop", 0, PSI_DOCUMENT_ME};
PSI_stage_info stage_waiting_for_manager_to_stop = {
    0, "Waiting for the manager to stop", 0, PSI_DOCUMENT_ME};

static mysql_cond_t COND_manager_state;

#ifdef HAVE_PSI_INTERFACE
PSI_stage_info *all_tasks_stages[] = {&stage_waiting_for_worker_to_stop,
                                       &stage_waiting_for_manager_to_stop};

static void init_statistics_manager_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_tasks_mutexes));
  mysql_mutex_register(category, all_tasks_mutexes, count);

  count = static_cast<int>(array_elements(all_tasks_conds));
  mysql_cond_register(category, all_tasks_conds, count);

  count = static_cast<int>(array_elements(all_tasks_threads));
  mysql_thread_register(category, all_tasks_threads, count);

  count = static_cast<int>(array_elements(all_tasks_stages));
  mysql_stage_register(category, all_tasks_stages, count);
}
#endif /* HAVE_PSI_INTERFACE */

static const LEX_CSTRING manager_states_names[] = {
    {STRING_WITH_LEN("UNINITIALIZED")},
    {STRING_WITH_LEN("INITIALIZED")},
    {STRING_WITH_LEN("RUNNING")},
    {STRING_WITH_LEN("STOPPING")}};

/*
  Performs post initialization of structures in a new thread.

  SYNOPSIS
    post_init_statistics_thread()
      thd  Thread

  NOTES
      Before this is called, one should not do any DBUG_XXX() calls.

*/

bool post_init_statistics_thread(THD *thd) {
  if (my_thread_init()) {
    return true;
  }
  thd->store_globals();

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  thd_manager->add_thd(thd);
  thd_manager->inc_thread_running();
  return false;
}

/*
  Performs pre- mysql_thread_create() initialisation of THD. Do this
  in the thread that will pass THD to the child thread. In the
  child thread call post_init_statistics_thread().

  SYNOPSIS
    pre_init_manager_thread()
      thd  The THD of the thread. Has to be allocated by the caller.

  NOTES
    1. The host of the thead is my_localhost
    2. thd->net is initted with NULL - no communication.
*/

void pre_init_manager_thread(THD *thd) {
  DBUG_TRACE;
  thd->security_context()->set_master_access(0);
  thd->security_context()->cache_current_db_access(0);
  thd->security_context()->set_user_ptr(STRING_WITH_LEN("auto_statistics_manager"));

  thd->set_new_thread_id();

  /*
    Guarantees that we will see the thread in SHOW PROCESSLIST though its
    vio is NULL.
  */
  thd->system_thread = SYSTEM_THREAD_STATISTICS_MANAGER;
  thd->set_command(COM_DAEMON);

  /* assign all privileges to this thread. */
  thd->security_context()->skip_grants();
  thd->security_context()->set_host_or_ip_ptr(my_localhost,
                                                  strlen(my_localhost));

  /* Do not use timeout value for system threads. */
  thd->variables.long_query_time = AUTO_STATS_LONG_QUERY_DEFAULT_VALUE;
  DBUG_PRINT("info", ("Forking new thread for auto statistics manager. THD: %p", thd));
}

/*
  Performs pre- mysql_thread_create() initialisation of THD. Do this
  in the thread that will pass THD to the child thread. In the
  child thread call post_init_statistics_thread().

  SYNOPSIS
    pre_init_worker_thread()
      thd  The THD of the thread. Has to be allocated by the caller.

  NOTES
    1. The host of the thead is my_localhost
    2. thd->net is initted with NULL - no communication.
*/

void pre_init_worker_thread(THD *thd) {
  DBUG_TRACE;
  thd->security_context()->set_master_access(0);
  thd->security_context()->cache_current_db_access(0);
  thd->security_context()->set_user_ptr(STRING_WITH_LEN("auto_statistics_worker"));

  thd->set_new_thread_id();

  /* assign all privileges to this thread. */
  thd->security_context()->skip_grants();
  /*
    Guarantees that we will see the thread in SHOW PROCESSLIST though its
    vio is NULL.
  */
  thd->system_thread = SYSTEM_THREAD_STATISTICS_WORKER;
  thd->set_command(COM_QUERY);

  /* use long query timeout value for worker threads. */
  thd->variables.long_query_time =
      double2ulonglong(global_system_variables.long_query_time_double * 1e6);
  DBUG_PRINT("info", ("Forking new thread for . THD: %p", thd));
}

/*
  Cleans up the THD and the threaded environment of the thread.

  SYNOPSIS
    deinit_statistics_thread()
      thd  Thread
*/

void deinit_statistics_thread(THD *thd) {
  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();

  thd->set_proc_info("Clearing");
  DBUG_PRINT("exit", ("Auto statistics thread finishing"));
  if (thd->system_thread == SYSTEM_THREAD_STATISTICS_MANAGER)
    thd->release_resources();
  thd_manager->remove_thd(thd);
  thd_manager->dec_thread_running();
}

bool is_maintenance_window(THD *thd) {
  assert(auto_stats_interval_duration >= 1 &&
      auto_stats_interval_duration <= 23);
  int hh,mm;
  hh = (auto_stats_interval_begin[0] - '0') * 10 +
      (auto_stats_interval_begin[1] - '0');
  mm = (auto_stats_interval_begin[3] - '0') * 10 +
      (auto_stats_interval_begin[4] - '0');

  time_t lclock = time(nullptr);  // Get the date struct
  struct tm *t = localtime(&lclock);
  int min_h = std::min(hh, (hh + auto_stats_interval_duration)%24);
  int max_h = std::max(hh, (hh + auto_stats_interval_duration)%24);
  if ((t->tm_hour * 60 + t->tm_min >= min_h * 60 + mm) &&
      (t->tm_hour * 60 + t->tm_min <= max_h * 60 + mm)) {
    return ((hh + auto_stats_interval_duration < 24) ||
        (hh + auto_stats_interval_duration == 24 && mm == 0));
  }
  return ((hh + auto_stats_interval_duration > 24) ||
      (hh + auto_stats_interval_duration == 24 && mm > 0));
}


extern "C" {
/*
  Function that executes the Statistics_manager,

  SYNOPSIS
    cdb_statistics_manager_thread()
    arg  Pointer to thd

  RETURN VALUE
    0  OK
*/

static void *cdb_statistics_manager_thread(void *arg __attribute__((unused)))
{
  bool res;
  THD *thd = (THD *)arg;
  /* needs to be first for thread_stack */
  thd->thread_stack = (char *)&thd;  // remember where our stack is
  res = post_init_statistics_thread(thd);

  mysql_thread_set_psi_id(thd->thread_id());

#ifdef HAVE_PSI_THREAD_INTERFACE
  /* Update the thread instrumentation. */
  PSI_THREAD_CALL(set_thread_account)
  (thd->security_context()->user().str, thd->security_context()->user().length,
   thd->security_context()->host_or_ip().str,
   thd->security_context()->host_or_ip().length);
  PSI_THREAD_CALL(set_thread_command)(thd->get_command());
  PSI_THREAD_CALL(set_thread_start_time)(thd->query_start_in_secs());
#endif /* HAVE_PSI_THREAD_INTERFACE */

  DBUG_PRINT("info", ("Setting state go RUNNING"));

  {
    DBUG_TRACE;
    if (!res) {
      while (!connection_events_loop_aborted() && !thd->killed
          && Statistics_manager::is_running()) {
        // check maintenance window and apply histograms
        if (is_maintenance_window(thd) &&
            DBUG_EVALUATE_IF("auto_statistic_flush_pending", false, true)) {
          if (Statistics_manager::apply_histograms()) {
            LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_WORKER_FAILURE,
                  "apply histograms failure");
          }
        }

        // signal manager thread to quit as soon as possible
        if (!cdb_auto_statistics_enabled ||
            cdb_statistics_port == 0) {
          my_sleep(auto_stats_thread_monitor_interval * 1000000);
          continue;
        }

        // check histogram_statistics_concurrency
        if (Statistics_manager::worker_count() >= histogram_statistics_concurrency ||
            DBUG_EVALUATE_IF("auto_statistic_execution_pending", true, false)) {
          DBUG_PRINT("info", ("statistic thread busy"));
          my_sleep(3 * 1000000);
        } else {
          DBUG_PRINT("info", ("statistic thread pool a new task"));
          Statistics_manager::run(thd);
        }

        my_sleep(auto_stats_thread_monitor_interval * 1000000);
      }
    } else {
      thd->set_proc_info("Clearing");
    }
  }  // Against gcc warnings

  deinit_statistics_thread(thd);
  delete thd;
  Statistics_manager::manager_thd = nullptr;

  Statistics_manager::state = Statistics_manager::INITIALIZED;

  mysql_cond_broadcast(&COND_manager_state);

  my_thread_end();
  return nullptr;
}

/**
  Function that executes a task in a child thread. Setups the
  environment for the task execution and cleans after that.

  SYNOPSIS
    statistics_worker_thread()
      arg  The task object to be processed

  RETURN VALUE
    0  OK
*/

static void *statistics_worker_thread(void *arg) {
  Statistics_task_element *task = (Statistics_task_element *)arg;
  THD *thd = task->thd;
  mysql_thread_set_psi_id(thd->thread_id());

#ifdef HAVE_PSI_THREAD_INTERFACE
  /* Update the thread instrumentation. */
  PSI_THREAD_CALL(set_thread_account)
  (thd->security_context()->user().str, thd->security_context()->user().length,
   thd->security_context()->host_or_ip().str,
   thd->security_context()->host_or_ip().length);
  PSI_THREAD_CALL(set_thread_command)(thd->get_command());
  PSI_THREAD_CALL(set_thread_start_time)(thd->query_start_in_secs());
#endif /* HAVE_PSI_THREAD_INTERFACE */

  thd->thread_stack = (char *)&thd;

  task->lock_data(__func__, __LINE__);
  if (task->m_status == TO_BE_DELETED || post_init_statistics_thread(thd)) {
    delete thd;
    task->thd = nullptr;
    task->m_status = DELETED_TASK;
    mysql_cond_broadcast(&task->COND_task_state);
    task->unlock_data(__func__, __LINE__);
    my_thread_end();
    return nullptr;  // Can't return anything here
  }
  task->m_status = ON_DOING;
  task->unlock_data(__func__, __LINE__);

  // step 0: build histogram SQL
  char llbuf[DECIMAL_LONGLONG_DIGITS];
  llstr((ulonglong)(task->m_num_buckets), llbuf);
  const char cols_format[] = "HISTOGRAM(%s,%s)";
  char cols[(NAME_LEN + 15) * COLUMN_QUEUE_MAX_SIZE];
  for (auto it = task->m_columns.begin();
      it != task->m_columns.end(); ++it) {
    char tmp_col[NAME_LEN + 15];
    sprintf(tmp_col, cols_format, (*it).c_str(), llbuf);
    if (it == task->m_columns.begin()) {
      strcpy(cols, tmp_col);
    } else {
      strcat(cols, ", ");
      strcat(cols, tmp_col);
    }
  }

  const char query_format[] = "SELECT %s FROM %s.%s;";
  char query[sizeof(query_format) - 6 + sizeof(cols)
             + sizeof(task->m_dbname)
             + sizeof(task->m_tablename)];
  sprintf(query, query_format, cols, task->m_dbname, task->m_tablename);
  DBUG_PRINT("info", ("statistic remote SQL: %s", query));

  MYSQL_RES *master_res = nullptr;
  MYSQL_ROW master_row;
  MYSQL *statistics_mysql = nullptr;
  const char *errmsg;
  char info_mesg[128];
  int master_row_it;
  auto col_it = task->m_columns.begin();

start:
  // step 1: init statistics client
  /* initiate connection */
  statistics_mysql = mysql_init(statistics_mysql);
  DBUG_PRINT("info", ("Creating statistics_mysql"));


  // step 2: connect to statistics node
  if (task->m_status != ON_DOING) {
    mysql_close(statistics_mysql);
    goto be_deleted;
  }
  if (!mysql_real_connect(statistics_mysql, cdb_statistics_host,
                          "tencentroot", "", nullptr,
                          cdb_statistics_port, nullptr, 0)) {
    LogErr(INFORMATION_LEVEL,
          ER_CDB_FAILED_TO_CONNECT_STATISTICS_CLIENT,
          "tencentroot", cdb_statistics_host, cdb_statistics_port);
    goto err;
  } else {
    LogErr(INFORMATION_LEVEL,
          ER_CDB_MASTER_CONNECTED_TO_STATISTICS_NODE_STARTED,
          "tencentroot", cdb_statistics_host, cdb_statistics_port);
  }

  // step 3: send commond to statistics node
  if (DBUG_EVALUATE_IF("auto_statistic_slave_sleep", true, false)){
    strcpy(query,"select sleep(300);");
  }

  // quit as soon as possible
  if (task->m_status == TO_BE_DELETED) {
    mysql_close(statistics_mysql);
    goto be_deleted;
  }

  if (!mysql_real_query(statistics_mysql, query,
                        static_cast<ulong>(strlen(query))) &&
      (master_res = mysql_store_result(statistics_mysql)) &&
      (master_row = mysql_fetch_row(master_res))) {
    DBUG_PRINT("info", ("statistic SQL rsp: %s", master_row[0]));
  } else {
    if (is_network_error(mysql_errno(statistics_mysql))) {
      LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_WORKER_FAILURE,
          "Get unknown err");
      mysql_free_result(master_res);
      mysql_close(statistics_mysql);
      goto err;
    }
    sprintf(info_mesg, "network error: %s", mysql_error(statistics_mysql));
    mysql_free_result(master_res);
    mysql_close(statistics_mysql);
    LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_WORKER_FAILURE,
          info_mesg);
    goto err;
  }

  mysql_close(statistics_mysql);

  // quit as soon as possible
  if (task->m_status == TO_BE_DELETED) {
    mysql_free_result(master_res);
    goto be_deleted;
  }

  /*
    The task may droped by ddl/rename (fun:kill_task()).
    To ensure save histogram before drop histogram,
    update_histogram_using_data needs to hold the task lock.
  */
  for (master_row_it = 0;
      col_it != task->m_columns.end() && task->m_status == ON_DOING;
      ++col_it, ++master_row_it) {
    if (master_row[master_row_it]){
      if (task->update_histogram_using_data(master_row[master_row_it],
                                            (*col_it).c_str(), &errmsg)) {
        LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_WORKER_FAILURE,
               errmsg);
        mysql_free_result(master_res);
        goto err;
      }
    }
  }

  // finish
  mysql_free_result(master_res);
  master_res = nullptr;

  if (task->m_status != ON_DOING) {
    goto be_deleted;
  } else {
    task->m_finished_at = (my_time_t)time(0);
    task->m_status = FINISHED_TASK;
    mysql_cond_broadcast(&task->COND_task_state);
  }

  my_thread_end();
  thd->release_resources();
  return nullptr;

err:
  if (task->m_status != TO_BE_DELETED && task->m_status != DELETED_TASK){
    if (task->m_execution_count < 10){
      task->m_execution_count++;
      task->m_last_executed_at = (long)time(0);
      statistics_mysql = nullptr;
      goto start;
    }
    task->m_status = FAILED_TASK;
  } else {
    task->m_status = DELETED_TASK;
  }
  mysql_cond_broadcast(&task->COND_task_state);
  my_thread_end();
  thd->release_resources();
  return nullptr;

be_deleted:
  task->m_status = DELETED_TASK;
  mysql_cond_broadcast(&task->COND_task_state);
  my_thread_end();
  thd->release_resources();
  return nullptr;  // Can't return anything here
}
}  // extern "C"


/*
  Constructor

  SYNOPSIS
    Statistics_task_element::Statistics_task_element()
*/
Statistics_task_element::Statistics_task_element()
    : mem_root(key_memory_statistics_task, 256),
      m_status(ON_PENDING),
      m_num_buckets(0),
      m_last_executed_at(0),
      m_finished_at(0),
      m_execute_at(0),
      m_create_at(0),
      m_execution_count(0),
      m_columns(Mem_root_allocator<std::string>(&mem_root)) {
  m_columns.reserve(COLUMN_QUEUE_MAX_SIZE);
  mysql_mutex_init(key_LOCK_stats_task_element,
                   &LOCK_task_element,
                   MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_task_element_COND_state, &COND_task_state);
}

void Statistics_task_element::set_base_info(const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, const int num_buckets,
    const my_time_t start_time) {
  strcpy(m_dbname ,db_name);
  strcpy(m_tablename, table_name);
  m_dbname_length = db_name_length;
  m_tablename_length = table_name_length;
  m_num_buckets = num_buckets;
  m_create_at = start_time;
}

bool Statistics_task_element::add_columns(
    const std::vector<std::string> &columns, const int new_num_bucket) {
  LOCK_TASK_DATA();

  // save the max bucket num
  refresh_max_buckets(new_num_bucket);

  if (m_columns.size() == COLUMN_QUEUE_MAX_SIZE ||
      m_columns.size() + columns.size() > COLUMN_QUEUE_MAX_SIZE) {
    return true;
  }

  // duplicate remove
  for (auto col : columns) {
    uint id = 0;
    for (auto elem : m_columns) {
      if (elem.compare(col.c_str()) == 0) {
        break;
      }
      id++;
    }
    if (id == m_columns.size()) {
      m_columns.emplace_back(std::move(col.c_str()));
    }
  }

  UNLOCK_TASK_DATA();
  return false;
}

bool Statistics_task_element::drop_columns(const columns_set &columns) {
  LOCK_TASK_DATA();
  for (const std::string &column_name : columns) {
    int id = 0;
    for (auto elem : m_columns) {
      if (elem.compare(column_name.c_str()) == 0) {
        m_columns.erase(std::begin(m_columns) + id);
        break;
      }
      id++;
    }
  }
  UNLOCK_TASK_DATA();
  return false;
}

bool Statistics_task_element::kill_task() {
  THD *curr_thd = current_thd;
  DBUG_TRACE;
  LOCK_TASK_DATA();
  DBUG_PRINT("enter", ("thd: %p", curr_thd));

  if (m_status == ON_PENDING) {
    m_status = TO_BE_DELETED;
    /* Synchronously wait until the worker stops. */
    while (m_status == TO_BE_DELETED)
      cond_wait(curr_thd, nullptr, &stage_waiting_for_worker_to_stop,
                __func__, __FILE__, __LINE__);
    goto end;
  } else if (m_status == FINISHED_TASK || m_status == FAILED_TASK) {
    m_status = DELETED_TASK;
    goto end;
  }

  // abstime for waiting statistics worker
  struct timespec abstime;
  set_timespec(&abstime, 1);

  /* Guarantee we don't catch spurious signals */
  do {
    if (!thd)
      goto end;

    m_status = TO_BE_DELETED;
    thd->thread_stack = (char *)&thd;
    DBUG_PRINT("info",
                  ("worker thread has id %u", thd->thread_id()));

    /* Lock from delete */
    mysql_mutex_lock(&thd->LOCK_thd_data);

    /* This will wake up the thread if it waits on conditional */
    LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_TRY_TO_KILL, "worker",
           manager_states_names[Statistics_manager::state].str,
           thd->thread_id());
    /* kill connection */
#ifndef _WIN32
    int err MY_ATTRIBUTE((unused)) = pthread_kill(thd->real_id, SIGALRM);
    assert(err != EINVAL);
#endif

    /*
      To close mysql client (statistics_mysql).
      Error codes from pthread_kill are:
      EINVAL: invalid signal number (can't happen)
      ESRCH: thread already killed (can happen, should be ignored)
    */
    thd->awake(THD::KILL_CONNECTION);

    mysql_mutex_unlock(&thd->LOCK_thd_data);

    cond_wait(curr_thd, &abstime, &stage_waiting_for_worker_to_stop,
              __func__, __FILE__, __LINE__);
  } while(m_status == TO_BE_DELETED);

  DBUG_PRINT("info", ("worker thread has stoped up."));
  LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_STOPPING, "worker");

end:
  m_status = DELETED_TASK;
  UNLOCK_TASK_DATA();
  return false;
}

/*
  Destructor

  SYNOPSIS
    Statistics_task_element::Statistics_task_element()
*/
Statistics_task_element::~Statistics_task_element() {
  m_columns.clear();
  mysql_mutex_destroy(&LOCK_task_element);
  mysql_cond_destroy(&COND_task_state);
  mem_root.Clear();
}

/*
  Auxiliary function for locking Statistics_task_element.
  Used by the LOCK_TASK_DATA().

  SYNOPSIS
    Statistics_task_element::lock_data()
*/

void Statistics_task_element::lock_data(const char *func, uint line) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("func=%s line=%u", func, line));
  mysql_mutex_lock(&LOCK_task_element);
}

/*
  Auxiliary function for unlocking Statistics_task_element.
  Used by the UNLOCK_TASK_DATA().

  SYNOPSIS
    Statistics_task_element::unlock_data()
*/

void Statistics_task_element::unlock_data(const char *func, uint line) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("func=%s line=%u", func, line));
  mysql_mutex_unlock(&LOCK_task_element);
}

bool Statistics_task_element::update_histogram_using_data(
    char* json_str, const char* field_name, const char **errmsg) {
  Json_dom_ptr dom;
  {
    JsonParseDefaultErrorHandler parse_handler(__func__, 0);
    String str{json_str, static_cast<uint32>(strlen(json_str)),
        &my_charset_utf8mb4_bin};
    {
      if (parse_json(str, &dom, true, parse_handler, 
          JsonDocumentDefaultDepthHandler)) {
        *errmsg = "Message::JSON_FORMAT_ERROR";
        return true;
      }
      if (dom->json_type() != enum_json_type::J_OBJECT) {
        *errmsg = "Message::JSON_NOT_AN_OBJECT";
        return true;
      }
    }
  }

  MEM_ROOT local_mem_root;
  init_sql_alloc(key_memory_histograms, &local_mem_root, 256);

  // Create a histogram for the json object.
  histograms::Error_context context;
  context.set_not_binary_format();
  std::string schema_name(m_dbname, m_dbname_length);
  std::string table_name(m_tablename, m_tablename_length);
  std::string col_name(field_name);

  histograms::Histogram *histogram = Histogram::json_to_histogram(
     &local_mem_root, schema_name, table_name, col_name,
      *down_cast<Json_object *>(dom.get()), &context);

  // Store it to persistent storage.
  if (histogram == nullptr) {
    *errmsg = "Message::json_to_histogram error";
    return true;
  }

  {
    History_table_persistor persistor{thd};
    if (persistor.save(schema_name.data(), table_name.data(), col_name.data(),
                       histogram)) {
      *errmsg = "Message::HISTOGRAM_HISTORY_VERSION_CREATE_FAILURE";
      return true;
    }
  }
  return false;
}

void Statistics_task_element::refresh_max_buckets(const int new_num_bucket) {
  m_num_buckets = m_num_buckets > new_num_bucket? new_num_bucket : m_num_buckets;
}

Statistics_task_element* Statistic_task_queue::top_queue() {
  DBUG_TRACE;
  LOCK_TASK_DATA();
  if (queue.size() > 0) {
    UNLOCK_TASK_DATA();
    return queue.top();
  }
  UNLOCK_TASK_DATA();
  return nullptr;
}

bool Statistic_task_queue::pop_queue() {
  DBUG_TRACE;
  LOCK_TASK_DATA();
  if (queue.size() > 0) {
    queue.pop();
    UNLOCK_TASK_DATA();
    return false;
  }
  UNLOCK_TASK_DATA();
  return true;
}

/*
  Constructor of class Statistic_task_queue.

  SYNOPSIS
    Statistic_task_queue::Statistic_task_queue()
*/

Statistic_task_queue::Statistic_task_queue()
    : queue(Task_queue_less(),
            Malloc_allocator<Statistics_task_element *>(
                key_memory_statistics_manager)),
      mutex_queue_data_locked(false) {
  mysql_mutex_init(key_LOCK_stats_task_queue,
                   &LOCK_task_queue,
                   MY_MUTEX_INIT_FAST);
}

Statistic_task_queue::~Statistic_task_queue() {
  deinit_queue();
  mysql_mutex_destroy(&LOCK_task_queue);
}

/*
  This is a queue's constructor. Until this method is called, the
  queue is unusable.  We don't use a C++ constructor instead in
  order to be able to check the return value. The queue is
  initialized once at server startup.

  SYNOPSIS
    Statistic_task_queue::init_queue()

  RETURN VALUE
    false  OK
    true   Error
*/

bool Statistic_task_queue::init_queue() {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("this: %p", this));

  LOCK_TASK_DATA();

  if (queue.reserve(TASK_QUEUE_INITIAL_SIZE)) {
    LogErr(ERROR_LEVEL, ER_CDB_STATISTICS_WORKER_FAILURE,
        "cant init task queue");
    goto err;
  }

  UNLOCK_TASK_DATA();
  return false;

err:
  UNLOCK_TASK_DATA();
  return true;
}

/*
  Deinits the queue. Remove all elements from it and destroys them
  too.

  SYNOPSIS
    Statistic_task_queue::deinit_queue()
*/

void Statistic_task_queue::deinit_queue() {
  DBUG_TRACE;

  empty_queue();
}


/*
  Empties the queue and destroys the Statistics_task_element objects in the
  queue.

  SYNOPSIS
    Statistic_task_queue::empty_queue()

  NOTE
    Should be called with LOCK_task_queue locked
*/

void Statistic_task_queue::empty_queue() {
  DBUG_TRACE;
  LOCK_TASK_DATA();

  DBUG_PRINT("enter", ("Purging the queue. %u element(s)",
                       static_cast<unsigned>(queue.size())));
  /* empty the queue */
  queue.delete_elements();

  UNLOCK_TASK_DATA();
}

Statistics_task_element* Statistic_task_queue::top_and_pop_queue() {
  DBUG_TRACE;
  LOCK_TASK_DATA();
  while (queue.size() > 0) {
    Statistics_task_element* task = queue.top();
    queue.pop();
    UNLOCK_TASK_DATA();
    return task;
  }
  UNLOCK_TASK_DATA();
  return nullptr;
}

bool Statistic_task_queue::push(Statistics_task_element* new_task) {
  DBUG_TRACE;
  LOCK_TASK_DATA();
  int ret = true;
  if (queue.size() < TASK_QUEUE_INITIAL_SIZE) {
    ret = queue.push(new_task);
  } else {
    LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_WORKER_FAILURE,
              "full task queue");
  }
  UNLOCK_TASK_DATA();
  return ret;
}

bool Statistic_task_queue::drop_task(const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, bool direct_drop) {
  DBUG_TRACE;
  LOCK_TASK_DATA();

  Statistics_task_element *tmp_task = new (std::nothrow) Statistics_task_element();
  if (!tmp_task) {
    return true;
  }
  tmp_task->set_base_info(db_name, db_name_length,
                          table_name, table_name_length,
                          0 /* ignore value*/, 0 /* ignore value*/);

  // find <schema.table> task
  for (queue_iterator it = queue.begin(); it != queue.end(); ++it) {
    if(**it == *tmp_task) {
      bool ret = false;
      if (!direct_drop) {
        // findout existing db.table task
        ret = (*it)->kill_task();
      }
      delete tmp_task;
      UNLOCK_TASK_DATA();
      return ret;
    }
  }

  delete tmp_task;
  UNLOCK_TASK_DATA();
  return false;
}

Statistics_task_element* Statistic_task_queue::drop_columns_from_task(
    const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, const columns_set &columns) {
  DBUG_TRACE;
  LOCK_TASK_DATA();

  Statistics_task_element *tmp_task = new (std::nothrow) Statistics_task_element();
  if (!tmp_task) {
    return nullptr;
  }
  tmp_task->set_base_info(db_name, db_name_length,
                          table_name, table_name_length,
                          0 /* ignore value*/, 0 /* ignore value*/);

  // find db.table task
  for (queue_iterator it = queue.begin(); it != queue.end(); ++it) {
    if(**it == *tmp_task) {
      // findout existing db.table task
      (*it)->drop_columns(columns);
      delete tmp_task;
      UNLOCK_TASK_DATA();
      return *it;
    }
  }

  delete tmp_task;
  UNLOCK_TASK_DATA();
  return nullptr;
}


/**
  Function that applies a task.

  SYNOPSIS
    task:  The task object to be processed

  RETURN VALUE
    0  OK
*/

bool Statistics_task_element::do_apply_statistics_task(THD *thd) {
  DBUG_TRACE;
  LOCK_TASK_DATA();
  
  bool apply_ret = false;

  History_table_persistor persistor{thd};

  dd::String_type db_name(m_dbname);
  dd::String_type table_name(m_tablename);
  Histogram *histogram = nullptr;

start:
  thd->thread_stack = (char *)&thd;  // remember where our stack is
  for (auto it = m_columns.begin();
      it != m_columns.end(); ++it) {
    dd::String_type column_name((*it).c_str());

    if ((apply_ret = persistor.load(db_name, table_name, column_name, -1, &histogram))) {
      // column may deleted by drop_histograms
      break;
    }

    if ((apply_ret = histogram->store_histogram_worker(thd))) {
      break;
    }
  }

  if (apply_ret) {
    /*
      do not try again when the failure exceeds 20 times.
      m_execution_count counts statistics_worker_thread failures
      and apply_statistics_thread failures.
    */
    if (m_execution_count >= 20) {
      LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_WORKER_FAILURE,
                    "storing histogram failed too many times");
    } else {
      m_execution_count++;
      goto start;
    }
  }

  UNLOCK_TASK_DATA();
  return false;
}


/*
  Wrapper for mysql_cond_wait/timedwait

  SYNOPSIS
    Statistics_task_element::cond_wait()
      thd     Thread (Could be NULL during shutdown procedure)
      msg     Message for thd->proc_info
      abstime If not null then call mysql_cond_timedwait()
      func    Which function is requesting cond_wait
      line    On which line cond_wait is requested
*/

void Statistics_task_element::cond_wait(THD *curr_thd, struct timespec *abstime,
                            const PSI_stage_info *stage, const char *src_func,
                            const char *src_file, uint src_line) {
  DBUG_TRACE;
  curr_thd->enter_cond(&COND_task_state, &LOCK_task_element, stage, nullptr,
                  src_func, src_file, src_line);

  if (!curr_thd->killed) {
    if (!abstime)
      mysql_cond_wait(&COND_task_state, &LOCK_task_element);
    else
      mysql_cond_timedwait(&COND_task_state, &LOCK_task_element, abstime);
  }

  /*
    Need to unlock before exit_cond, so we need to relock.
    Not the best thing to do but we need to obey cond_wait()
  */
  unlock_data(src_func, src_line);
  curr_thd->exit_cond(nullptr, src_func, src_file, src_line);
  lock_data(src_func, src_line);
}

bool Statistic_task_queue::add_columns_to_task(
    const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, const std::vector<std::string> &columns,
    const int num_buckets, const my_time_t start_time) {
  DBUG_TRACE;
  LOCK_TASK_DATA();

  Statistics_task_element *tmp_task = new (std::nothrow) Statistics_task_element();
  if (!tmp_task) {
    UNLOCK_TASK_DATA();
    return true;
  }
  tmp_task->set_base_info(db_name, db_name_length,
                          table_name, table_name_length,
                          num_buckets, start_time);

  // find <schema.table> task
  for (queue_iterator it = queue.begin(); it != queue.end(); ++it) {
    if(**it == *tmp_task) {
      // findout existing db.table task
      if ((*it)->add_columns(columns, num_buckets)) {
        break;
      }
      UNLOCK_TASK_DATA();
      delete tmp_task;
      return false;
    }
  }

  // create new task with to-be-added columns
  if (tmp_task->add_columns(columns, num_buckets) || queue.push(tmp_task)) {
    delete tmp_task;
    UNLOCK_TASK_DATA();
    return true;
  }

  UNLOCK_TASK_DATA();
  return false;
}

/*
  Auxiliary function for locking Statistic_task_queue.
  Used by the LOCK_TASK_DATA().

  SYNOPSIS
    Statistic_task_queue::lock_data()
*/

void Statistic_task_queue::lock_data(const char *func, uint line) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("func=%s line=%u", func, line));
  mysql_mutex_lock(&LOCK_task_queue);
  mutex_queue_data_locked = true;
}

/*
  Auxiliary function for unlocking Statistic_task_queue.
  Used by the UNLOCK_TASK_DATA().

  SYNOPSIS
    Statistic_task_queue::unlock_data()
*/

void Statistic_task_queue::unlock_data(const char *func, uint line) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("func=%s line=%u", func, line));
  mutex_queue_data_locked = false;
  mysql_mutex_unlock(&LOCK_task_queue);
}

void Statistic_task_queue::get_all_tasks_for_display(
    std::vector<Statistics_task_element*> &ret) {
  LOCK_TASK_DATA();
  for (queue_iterator it = queue.begin();
      it != queue.end(); ++it) {
    ret.push_back(*it);
  }
  UNLOCK_TASK_DATA();
}

Statistic_task_queue *Statistics_manager::statistics_task_pending_queue;
Statistic_task_queue *Statistics_manager::statistics_task_running_queue;
Statistic_task_queue *Statistics_manager::statistics_task_apply_queue;
std::atomic<int32> Statistics_manager::state{0};
std::atomic<uint32> Statistics_manager::workers{0};
THD * Statistics_manager::manager_thd;
const char * Statistics_manager::errmsg;


bool Statistics_manager::init() {
  DBUG_TRACE;
  DBUG_PRINT("info",
             ("state before action %s", manager_states_names[state].str));

  bool res = false;

  mysql_cond_init(key_task_manager_COND_state, &COND_manager_state);

  if (!(statistics_task_pending_queue = new (std::nothrow) Statistic_task_queue)) {
    res = true; /* fatal error */
    goto end;
  }

  if (statistics_task_pending_queue->init_queue()) {
    res = true; /* fatal error */
    goto end;
  }

  if (!(statistics_task_running_queue = new (std::nothrow) Statistic_task_queue)) {
    res = true; /* fatal error */
    goto end;
  }

  if (statistics_task_running_queue->init_queue()) {
    res = true; /* fatal error */
    goto end;
  }

  if (!(statistics_task_apply_queue = new (std::nothrow) Statistic_task_queue)) {
    res = true; /* fatal error */
    goto end;
  }

  if (statistics_task_apply_queue->init_queue()) {
    res = true; /* fatal error */
    goto end;
  }

end:
  if (res) {
    /* fatal error: request unireg_abort */
    LogErr(ERROR_LEVEL, ER_CDB_STATISTICS_CANT_INIT_POOL);

    delete statistics_task_running_queue;
    statistics_task_running_queue = nullptr;
    if (statistics_task_pending_queue)
      delete statistics_task_pending_queue;
    statistics_task_pending_queue = nullptr;
    if (statistics_task_apply_queue)
      delete statistics_task_apply_queue;
    statistics_task_apply_queue = nullptr;

    return res;
  }

  DBUG_PRINT("info", ("Setting state go INITIALIZED"));
  state = INITIALIZED;
  workers = 0;

  return cdb_auto_statistics_enabled && start();
}

/*
  Inits Statistics_manager mutexes

  SYNOPSIS
    Statistics_manager::init_mutexes()
      thd  Thread
*/

void Statistics_manager::init_mutexes() {
#ifdef HAVE_PSI_INTERFACE
  init_statistics_manager_psi_keys();
#endif
}

static void release_thread_id(my_thread_id thread_id) {
  Global_THD_manager::get_instance()->release_thread_id(thread_id);
}

/**
  Starts the statistics manager (again). Creates a new THD and passes
  it to a forked thread. Does not wait for acknowledgement from the
  new thread that it has started. Asynchronous starting. Most of the
  needed initializations are done in the current thread to minimize
  the chance of failure in the spawned thread.

  @param[out] err_no - errno indicating type of error which caused
                       failure to start manager thread.

  @retval false Success.
  @retval true  Error.
*/

bool Statistics_manager::start() {
  DBUG_PRINT("info",
             ("state before action %s", manager_states_names[state].str));
  bool ret = false;
  int err_no;
  THD *new_thd = nullptr;
  my_thread_handle thread_handle;

  mysql_mutex_lock(&LOCK_stats_manager);

  if (state != INITIALIZED || manager_thd) goto end;

  if (!(new_thd = new (std::nothrow) THD)) {
    LogErr(ERROR_LEVEL, ER_CDB_STATISTICS_WORKER_FAILURE, "failure to create new thd");
    ret = true;
    goto end;
  }
  pre_init_manager_thread(new_thd);  

  // Keep the thd for async stop.
  manager_thd = new_thd;
  state = RUNNING;

  if ((err_no = mysql_thread_create(key_thread_statistics_manager, &thread_handle,
                                     &connection_attrib,
                                     cdb_statistics_manager_thread,
                                     (void *)manager_thd))) {
    DBUG_PRINT("error", ("cannot create a new thread"));
    LogErr(ERROR_LEVEL, ER_CDB_STATISTICS_CANT_INIT_MANAGER, err_no)
        .os_errno(err_no);

    release_thread_id(new_thd->thread_id());
    delete new_thd;

    state = INITIALIZED;
    manager_thd = nullptr;

    ret = true;
  }

end:
  mysql_mutex_unlock(&LOCK_stats_manager);
  return ret;
}

/*
  The main loop of the manager.

  SYNOPSIS
    Statistics_manager::run()
      thd  Thread

  RETURN VALUE
    false  OK
    true   Error (Serious error)
*/

bool Statistics_manager::run(THD *thd) {
  bool res = false;
  DBUG_TRACE;
  LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_THREAD_STARTED, thd->thread_id());

  while (Statistics_manager::is_running()) {
    if (worker_count() >= histogram_statistics_concurrency) {
      DBUG_PRINT("info", ("statistic thread busy"));
      break;
    }

    if (statistics_task_pending_queue->size() == 0) {
      /* no more tasks in the queue */
      /* check the status of running tasks */
      check_histograms_status(false);
      break;
    } else {
      DBUG_PRINT("info", ("get top for execution"));

      mysql_mutex_lock(&LOCK_stats_manager);
      Statistics_task_element *task = statistics_task_pending_queue->top_and_pop_queue();

      // task may dropped by fun:drop()
      if (!task) {
        mysql_mutex_unlock(&LOCK_stats_manager);
        break;
      }

      // rename table
      if (task->m_status == DELETED_TASK) {
        delete task;
        mysql_mutex_unlock(&LOCK_stats_manager);
        continue;
      }

      task->m_execution_count++;
      task->m_last_executed_at = (my_time_t)time(0);
      task->m_execute_at = task->m_last_executed_at;

      res = execute_top(task);
      statistics_task_running_queue->push(task);
      mysql_mutex_unlock(&LOCK_stats_manager);
    }
  }

  return res;
}

/*
  Creates a new THD instance and then forks a new thread, while passing
  the task to it.

  SYNOPSIS
    Statistics_manager::execute_top()

  RETURN VALUE
    false  OK
    true   Error (Serious error)
*/

bool Statistics_manager::execute_top(Statistics_task_element *task) {
  DBUG_PRINT("info", ("Task %s@%s ready for execute", task->m_dbname,
                      task->m_tablename));
  for (auto elem : task->m_columns)
  {
    DBUG_PRINT("info", ("\t col:%s", elem.c_str()));
  }

  THD *new_thd;
  my_thread_handle thread_handle;
  int res = 0;

  if (!(new_thd = new (std::nothrow) THD)){
    goto error;
  }

  pre_init_worker_thread(new_thd);
  task->thd = new_thd;
  task->m_last_executed_at = (my_time_t)time(0);
  inc_worker_count();

  if ((res =
           mysql_thread_create(key_thread_task_worker,
                               &thread_handle, &connection_attrib,
                               statistics_worker_thread, task))) {
    LogErr(ERROR_LEVEL, ER_CDB_STATISTICS_MANAGER_FAILED_TO_CREATE_WORKER, res);
    new_thd->set_proc_info("Clearing");

    goto error;
  }

  DBUG_PRINT("info", ("task is in THD: %p", new_thd));
  return false;

error:
  DBUG_PRINT("error", ("Statistics_manager::execute_top() res: %d", res));
  if (new_thd) {
    deinit_statistics_thread(new_thd);
    delete new_thd;
  }
  task->thd = nullptr;
  task->m_status = FAILED_TASK;
  return true;
}

/*
  Checks whether the state of the manager is RUNNING

  SYNOPSIS
    Statistics_manager::is_running()

  RETURN VALUE
    true   RUNNING
    false  Not RUNNING
*/

bool Statistics_manager::is_running() {
  bool ret = (state == RUNNING);
  return ret;
}

/*
  reset the state of the manager to INITIALIZED

  SYNOPSIS
    Statistics_manager::set_to_initialized()

  RETURN VALUE
    true   succ
    false  failed
*/

bool Statistics_manager::set_to_initialized(THD *thd) {
  bool ret = false;
  if (state == RUNNING) {
    thd->set_proc_info("waiting for start auto statistics");
    state = INITIALIZED;
    ret = true;
  }
  return ret;
}


/*
  set the state of the manager to RUNNING

  SYNOPSIS
    Statistics_manager::set_to_running()

  RETURN VALUE
    true   succ
    false  failed
*/

bool Statistics_manager::set_to_running(THD *thd) {
  bool ret = false;
  if (state == INITIALIZED) {
    thd->set_proc_info("waiting for statistic tasks");
    state = RUNNING;
    ret = true;
  }
  return ret;
}

/*
  Wrapper for mysql_cond_wait/timedwait

  SYNOPSIS
    Statistics_manager::cond_wait()
      thd     Thread (Could be NULL during shutdown procedure)
      msg     Message for thd->proc_info
      abstime If not null then call mysql_cond_timedwait()
      func    Which function is requesting cond_wait
      line    On which line cond_wait is requested
*/

void Statistics_manager::cond_wait(THD *curr_thd, struct timespec *abstime,
                            const PSI_stage_info *stage, const char *src_func,
                            const char *src_file, uint src_line) {
  DBUG_TRACE;
  curr_thd->enter_cond(&COND_manager_state, &LOCK_stats_manager, stage, nullptr,
                       src_func, src_file, src_line);

  if (!curr_thd->killed) {
    if (!abstime)
      mysql_cond_wait(&COND_manager_state, &LOCK_stats_manager);
    else
      mysql_cond_timedwait(&COND_manager_state, &LOCK_stats_manager, abstime);
  }

  /*
    Need to unlock before exit_cond, so we need to relock.
    Not the best thing to do but we need to obey cond_wait()
  */
  mysql_mutex_unlock(&LOCK_stats_manager);
  curr_thd->exit_cond(nullptr, src_func, src_file, src_line);
  mysql_mutex_lock(&LOCK_stats_manager);
}

bool Statistics_manager::apply_histograms() {
  DBUG_TRACE;

  while (statistics_task_apply_queue->size() > 0) {
    DBUG_PRINT("info", ("Tasks ready for apply"));
    mysql_mutex_lock(&LOCK_stats_manager);

    Statistics_task_element *task = statistics_task_apply_queue->top_queue();

    // task may dropped by fun::drop()
    if (!task) {
      mysql_mutex_unlock(&LOCK_stats_manager);
      break;
    }

    // direct delete failed/deleted tasks without apply
    if (task->m_status == FINISHED_TASK) {
      task->do_apply_statistics_task(manager_thd);
    }
    statistics_task_apply_queue->pop_queue();
    delete task;

    mysql_mutex_unlock(&LOCK_stats_manager);
  }

  return false;
}

bool Statistics_manager::check_histograms_status(bool force_kill) {
  DBUG_TRACE;
  if (!force_kill)
    mysql_mutex_lock(&LOCK_stats_manager);

  Statistics_task_element *task;
  while ((task = statistics_task_running_queue->top_and_pop_queue())) {
    if (force_kill || (long)(time(0) - task->m_last_executed_at - 10) > 
          auto_stats_thread_monitor_interval) {
      // This is an async operation.
      task->kill_task();
      deinit_statistics_thread(task->thd);
      delete task->thd;
      delete task;
      dec_worker_count();
      continue;
    }

    if (task->m_status == DELETED_TASK ||
        task->m_status == FAILED_TASK) {
      deinit_statistics_thread(task->thd);
      delete task->thd;
      delete task;
      dec_worker_count();
      continue;
    }
    if (task->m_status == FINISHED_TASK) {
      deinit_statistics_thread(task->thd);
      delete task->thd;
      task->thd = nullptr;
      dec_worker_count();
      statistics_task_apply_queue->push(task);
      continue;
    }

    // for TO_BE_DELETED/ON_PENDING tasks
    // continue executing the task (check next round)
    statistics_task_running_queue->push(task);
    break;
  }

  if (!force_kill)
    mysql_mutex_unlock(&LOCK_stats_manager);

  return false;
}

bool Statistics_manager::add_columns_to_task(
    const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, const column_name_set &columns,
    const int num_buckets, const my_time_t start_time) {
  std::vector<std::string> str_columns;
  for (const Field *field : columns) {
    str_columns.push_back(std::move(field->field_name));
  }
  bool ret = auto_add_statistics_task(db_name, db_name_length,
                                      table_name, table_name_length,
                                      str_columns, num_buckets, start_time);
  return ret;
}

bool Statistics_manager::auto_add_statistics_task(
    const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, const std::vector<std::string> &columns,
    const int num_buckets, const my_time_t start_time) {
  mysql_mutex_lock(&LOCK_stats_manager);
  bool ret =
      statistics_task_pending_queue->add_columns_to_task(
                          db_name, db_name_length,
                          table_name, table_name_length,
                          columns, num_buckets, start_time);
  mysql_mutex_unlock(&LOCK_stats_manager);
  return ret;
}

void Statistics_manager::drop(
    const char* db_name,
    const int db_name_length, const char* table_name,
    const int table_name_length, bool is_rename,
    const columns_set &columns) {
  if (state < INITIALIZED) {
    return;
  }

  check_histograms_status(false);

  mysql_mutex_lock(&LOCK_stats_manager);

  if (statistics_task_running_queue->size() > 0)
    statistics_task_running_queue->drop_task(db_name,
                                             db_name_length,
                                             table_name,
                                             table_name_length);

  if (is_rename) {
    if (statistics_task_pending_queue->size() > 0)
      statistics_task_pending_queue->drop_task(db_name,
                                               db_name_length,
                                               table_name,
                                               table_name_length,
                                               true);
    if (statistics_task_apply_queue->size() > 0)
      statistics_task_apply_queue->drop_task(db_name,
                                             db_name_length,
                                             table_name,
                                             table_name_length,
                                             true);
  } else {
    if (statistics_task_pending_queue->size() > 0)
      statistics_task_pending_queue->drop_columns_from_task(db_name,
                                                            db_name_length,
                                                            table_name,
                                                            table_name_length,
                                                            columns);
    if (statistics_task_apply_queue->size() > 0)
      statistics_task_apply_queue->drop_columns_from_task(db_name,
                                                            db_name_length,
                                                            table_name,
                                                            table_name_length,
                                                            columns);
  }

  mysql_mutex_unlock(&LOCK_stats_manager);
  return;
}

/**
  Stops the Statistics_manager (again). Waits for acknowledgement
  from the Statistics_manager that it has stopped - synchronous stopping.

  Already running tasks will not be stopped. If the user needs
  them stopped manual intervention is needed.

  SYNOPSIS
    Statistics_manager::stop()

  RETURN VALUE
    false  OK
    true   Error (not reported)
*/

bool Statistics_manager::stop(bool need_lock) {
  DBUG_TRACE;
  DBUG_PRINT("info",
             ("state before action %s", manager_states_names[state].str));
  THD *curr_thd = current_thd;

  if (need_lock)
    mysql_mutex_lock(&LOCK_stats_manager);
  // successive stop request
  if (state <= INITIALIZED || !manager_thd) {
    goto end;
  }

  if (state != RUNNING) {
    /* Synchronously wait until the scheduler stops. */
    while (state != INITIALIZED)
      cond_wait(curr_thd, nullptr, &stage_waiting_for_manager_to_stop,
                __func__, __FILE__, __LINE__);
    goto end;
  }

  do {
    state = STOPPING;
    manager_thd->thread_stack = (char *)&manager_thd;
    DBUG_PRINT("info",
                 ("manager thread has id %u", manager_thd->thread_id()));

    mysql_mutex_lock(&manager_thd->LOCK_thd_data);

    /* This will wake up the thread if it waits on Queue's conditional */
    LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_TRY_TO_KILL, "manager",
           manager_states_names[state].str, manager_thd->thread_id());
    manager_thd->awake(THD::KILL_CONNECTION);

    mysql_mutex_unlock(&manager_thd->LOCK_thd_data);

    cond_wait(curr_thd, nullptr, &stage_waiting_for_manager_to_stop,
              __func__, __FILE__, __LINE__);
  } while (state == STOPPING);

  if (!need_lock)
    state = UNINITIALIZED;

  LogErr(INFORMATION_LEVEL, ER_CDB_STATISTICS_STOPPING, "manager");

  // kill active tasks
  check_histograms_status(true);

  statistics_task_pending_queue->empty_queue();
  statistics_task_apply_queue->empty_queue();

end:
  if (need_lock) {
    mysql_mutex_unlock(&LOCK_stats_manager);
  }
  return false;
}

/*
  Cleans up manager's resources. Called at server shutdown.

  SYNOPSIS
    Statistics_manager::deinit()

  NOTES
    This function is not synchronized.
*/

void Statistics_manager::deinit() {
  DBUG_TRACE;
  mysql_mutex_lock(&LOCK_stats_manager);

  if (state == UNINITIALIZED)
  {
    mysql_mutex_unlock(&LOCK_stats_manager);
    return;
  }

  if (state != INITIALIZED)
    Statistics_manager::stop(false);

  mysql_cond_destroy(&COND_manager_state);

  delete statistics_task_pending_queue;
  statistics_task_pending_queue = nullptr; /* safety */
  delete statistics_task_running_queue;
  statistics_task_running_queue = nullptr; /* safety */
  delete statistics_task_apply_queue;
  statistics_task_apply_queue = nullptr;

  mysql_mutex_unlock(&LOCK_stats_manager);
  return;
}

void Statistics_manager::get_all_tasks_for_display(THD *thd) {
  mem_root_deque<Item *> field_list(thd->mem_root);
  Protocol *protocol= thd->get_protocol();
  DBUG_ENTER("mysqld_list_stats_tasks");

  field_list.push_back(new Item_empty_string("TABLE_SCHEMA", NAME_CHAR_LEN));
  field_list.push_back(new Item_empty_string("TABLE_NAME", NAME_CHAR_LEN));
  field_list.push_back(new Item_empty_string("COLUMN_NAME", NAME_CHAR_LEN));
  field_list.push_back(new Item_return_int("CREATED_TIME", 21, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_return_int("EXECUTE_TIME", 21, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_return_int("LAST_EXECUTE_TIME", 21, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_return_int("ATTEMPTS", 4, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_empty_string("Status", NAME_CHAR_LEN));

  if (thd->send_result_metadata(field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    DBUG_VOID_RETURN;

  std::vector<Statistics_task_element*> ret;
  const char *pending_str= "ON_PENDING";
  const char *doing_str= "ON_DOING";
  const char *finished_str= "FINISHED";
  const char *failed_str= "FAILED";
  mysql_mutex_lock(&LOCK_stats_manager);
  statistics_task_pending_queue->get_all_tasks_for_display(ret);
  statistics_task_running_queue->get_all_tasks_for_display(ret);
  statistics_task_apply_queue->get_all_tasks_for_display(ret);

  for (size_t i= 0; i< ret.size(); i++)
  {
    for (auto it = ret[i]->m_columns.begin();
        it != ret[i]->m_columns.end(); ++it) {
      protocol->start_row();

      protocol->store(ret[i]->m_dbname, system_charset_info);
      protocol->store(ret[i]->m_tablename, system_charset_info);
      protocol->store((*it).c_str(), system_charset_info);
      protocol->store((longlong) ret[i]->m_create_at);
      protocol->store((longlong) ret[i]->m_execute_at);
      protocol->store((longlong) ret[i]->m_last_executed_at);
      protocol->store((longlong) ret[i]->m_execution_count);
      if (ret[i]->m_status == 0)
        protocol->store(pending_str, system_charset_info);
      else if (ret[i]->m_status == 1)
        protocol->store(doing_str, system_charset_info);
      else if (ret[i]->m_status == 2)
        protocol->store(finished_str, system_charset_info);
      else
        protocol->store(failed_str, system_charset_info);

      if (protocol->end_row())
        break; /* purecov: inspected */
    }
  }
  mysql_mutex_unlock(&LOCK_stats_manager);

  my_eof(thd);
  DBUG_VOID_RETURN;
}

int Statistics_manager::stats_task_fill_i_s(THD* thd, TABLE *table) {
  DBUG_ENTER("mysqld_list_stats_tasks");
  const char *pending_str= "ON_PENDING";
  const char *doing_str= "ON_DOING";
  const char *finished_str= "FINISHED";
  const char *failed_str= "FAILED";

  std::vector<Statistics_task_element*> ret;
  mysql_mutex_lock(&LOCK_stats_manager);
  statistics_task_pending_queue->get_all_tasks_for_display(ret);
  statistics_task_running_queue->get_all_tasks_for_display(ret);
  statistics_task_apply_queue->get_all_tasks_for_display(ret);

  for (size_t i= 0; i< ret.size(); i++)
  {
    for (auto it = ret[i]->m_columns.begin();
        it != ret[i]->m_columns.end(); ++it) {
      table->field[0]->store(ret[i]->m_dbname, strlen(ret[i]->m_dbname), system_charset_info);
      table->field[1]->store(ret[i]->m_tablename, strlen(ret[i]->m_tablename), system_charset_info);
      table->field[2]->store((*it).c_str(), (*it).length(), system_charset_info);
      table->field[3]->store((longlong) ret[i]->m_create_at);
      table->field[4]->store((longlong) ret[i]->m_execute_at);
      table->field[5]->store((longlong) ret[i]->m_last_executed_at);
      table->field[6]->store((longlong) ret[i]->m_execution_count);
      if (ret[i]->m_status == 0)
        table->field[7]->store(pending_str, strlen(pending_str), system_charset_info);
      else if (ret[i]->m_status == 1)
        table->field[7]->store(doing_str, strlen(doing_str), system_charset_info);
      else if (ret[i]->m_status == 2)
        table->field[7]->store(finished_str, strlen(finished_str), system_charset_info);
      else
        table->field[7]->store(failed_str, strlen(failed_str), system_charset_info);

      if (schema_table_store_record(thd, table)) {
        mysql_mutex_unlock(&LOCK_stats_manager);
        DBUG_RETURN(1);
      }
    }
  }

  mysql_mutex_unlock(&LOCK_stats_manager);
  DBUG_RETURN(0);
}

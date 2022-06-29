/* Copyright (c) 2009, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PX_WORKERPOOL_INCLUDED
#define PX_WORKERPOOL_INCLUDED

#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>

#include "sql/sql_lex.h" // JOIN
#include "sql/sql_class.h" // THD
#include "sql/parallel_execution/px_exchange_info.h" // PX_exchange_info
#include "sql/parallel_execution/px_dfo.h" // PX_dfo
#include "my_base.h" // mutex lock
#include "mysql/psi/mysql_thread.h" // mysql_mutex_t
#include "my_psi_config.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/components/services/bits/mysql_cond_bits.h"
#include "mysql/components/services/bits/mysql_mutex_bits.h"
#include "mysql/components/services/bits/psi_cond_bits.h"
#include "mysql/components/services/bits/psi_mutex_bits.h"
#include "mysql/components/services/bits/psi_stage_bits.h"

class PX_reader;
class Worker_exec_ctx;

typedef Prealloced_array<THD *, 60> THD_array;
typedef void *(*worker_func)(void *);

typedef struct cond_with_lock_t {
  mysql_cond_t COND_worker_signal; // condition for worker start.
  mysql_mutex_t LOCK_worker_signal; // mutex for worker start.
  PSI_cond_key key_COND_worker_signal; // PSI key for condition.
  PSI_mutex_key key_LOCK_worker_signal; // PSI key for mutex.
  int flag_COND_signal; // flag for condition to wait.
} cond_with_lock_t; // use for synchronize of down instrument flows.

/**
  Thread arguments between coordinator and workers, two levels:
  1. SQL parse, prepare, optimize and execute.
  2. Task execute in single dfo pair scheduling.
  Also, worker_thread_arg maintained synchronization between them.
*/
typedef struct worker_thread_arg {
  worker_func           *thread_func; // thread func pointer.
  void                  **thread_func_arg;
  /* Thread function arguments for all workers. */
  THD                   *worker_thd; // THD thread attach.
  int                   task_id; // task id of executor.
  PX_exchange_info      *exchange_info; // exchange ctx.
  PX_reader             *scan_ctx; // scan_ctx.

  /* Sychronzation arguments between coordinator and workers. */
  int                   *num_workers; // tasks cnt.

  cond_with_lock_t      sem_worker_signal; // sychonize the tasks.
  cond_with_lock_t      sem_task_signal; // sychonize the tasks.
  std::atomic<int>      *num_query_done; // finished query cnt.
  std::atomic<int>      *num_workers_done; // finished workers cnt.
  cond_with_lock_t      *sem_workers_done; // sem all workers done.
  cond_with_lock_t      *sem_tasks_done; // sem all tasks done.
  cond_with_lock_t      *sem_query_done; // sem all workers' query done.
  int                   *finished; // every workers finished.
  MY_BITMAP             ***bitmap; // bitmap of the whole SQL execution.
  MY_BITMAP             **bitmap_map; // map of the bitmap.
  mysql_mutex_t         *mutex_task; // control tasks barrier.
  PX_exchange_context   **px_exchange_context; // px exchange context.
  /* Used for check equivalence between coordinator and workers. */
  AccessPath            *coordinator_root_access_path; // coordinator's plan
  JOIN                  *coordinator_join; // coordinator's JOIN
  bool                  is_equivalent_plan; // equivalent to the coordinator's plan
} worker_thread_arg;

typedef struct worker_pool_t {
  pthread_t             *threads;
  worker_thread_arg     *thread_args;
  worker_func           thread_func; // thread func pointer.
  void                  **args;

  int                   num_workers; // workers SQL PX execution need.
  std::atomic<int>      num_query_done; // finished query cnt.
  std::atomic<int>      num_workers_done; // finished workers cnt.
  cond_with_lock_t      sem_workers_done; // sem all workers done.
  cond_with_lock_t      sem_tasks_done; // sem all tasks done.
  cond_with_lock_t      sem_query_done; // sem all query done.
  int                   finished; // every workers finished.
  MY_BITMAP             **bitmap; // bitmap of the task execution.
  MY_BITMAP             *bitmap_map; // most 64 right deep tree.
  mysql_mutex_t         mutex_task; // control tasks barrier.
  PX_exchange_context   *px_exchange_context; // px exchange context.

  MY_BITMAP             threads_bitmap; // threads bitmap one query use.
} worker_pool_t;

// Create worker pool to start worker executions, there are two levels
// of synchronization. The outside is query barrier, which exists in the
// pointer between optimization and execution, inside is task barrier,
// which barrie at every workers' task finish.
worker_pool_t* create_worker_threads(int num_threads, const THD_array &list);
// Release the worker threads created.
void release_worker_threads(worker_pool_t *worker_pool);
// Init the px condition with lock.
void px_condition_init(cond_with_lock_t *condition_with_lock);
// Set the threads arguments when start every worker group.
void set_threads_args(worker_pool_t *worker_pool, worker_func thread_func,
                      void **args);
// allocate threads for group id, set bitmap of workers.
bool allocate_threads(worker_pool_t *worker_pool, Worker_exec_ctx *ctx);


// Start outside steps for all workers: parse and optimize.
bool begin_query(worker_pool_t *worker_pool);
// Outside parse and optimize barrier for all workers.
bool wait_workers_generate_plan(worker_pool_t *worker_pool, THD *thd);
// wait for signal for begin query.
void wait_for_begin_query(worker_thread_arg *arg);
// Signal for opening barrier after worker finish parse and optimize.
bool optimize_finsh_signal(worker_thread_arg* arg, bool error = false);
// Signal for finish the query, set the query finish flag.
void finish_query(worker_thread_arg* arg);


// Start inside task execution in idx threads' group.
bool begin_task(worker_pool_t *worker_pool, Worker_exec_ctx *ctx);
// Inside task execution barrier in idx threads' group.
bool wait_workers_finish_task(worker_pool_t *worker_pool, Worker_exec_ctx *ctx);
// Signal for start task begin and wait.
void wait_for_begin_task(worker_thread_arg *arg);
// Signal for opening barrier after worker finish task.
void finish_task(worker_thread_arg *arg);


void schedule_over(worker_pool_t *worker_pool, THD *thd);
void schedule_end(worker_pool_t *worker_pool);
void destroy_cond_for_worker(cond_with_lock_t *cond);

#endif // PX_WORKERPOOL_INCLUDED

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

class PX_reader;
class Worker_exec_ctx;

typedef void *(*worker_func)(void *);

enum worker_task_type { PARSE_AND_PREPARE = 0, SLICE_TASK };

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
  std::string           query_string; // query string of coordinator.

  /* Sychronzation arguments between coordinator and workers. */
  worker_task_type      type; // 1. Parse and prepare; 2. Task.
  int                   *num_workers; // tasks cnt.
  pthread_semphore_t    sem_outer; // sychonize the main.
  pthread_semphore_t    sem_inner; // sychonize the tasks.
  std::atomic<int>      *num_workers_done; // finished workers cnt.
  pthread_semphore_t    *sem_workers_done; // sem all workers done.
  pthread_semphore_t    *sem_tasks_done; // sem all tasks done.
  int                   *finished; // every workers finished.
  MY_BITMAP             ***bitmap; // bitmap of the whole SQL execution.
  uint64_t              *bitmap_map; // map of the bitmap.
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

  worker_task_type      type; // 1. Parse and prepare; 2. Task.
  int                   num_workers; // workers SQL PX execution need.
  std::atomic<int>      num_workers_done; // finished workers cnt.
  pthread_semphore_t    sem_workers_done; // sem all workers done.
  pthread_semphore_t    sem_tasks_done; // sem all tasks done.
  int                   finished; // every workers finished.
  MY_BITMAP             **bitmap; // bitmap of the task execution.
  uint64_t              bitmap_map; // most 64 right deep tree.
  mysql_mutex_t         mutex_task; // control tasks barrier.
  PX_exchange_context   *px_exchange_context; // px exchange context.

  MY_BITMAP             threads_bitmap; // threads bitmap one query use.
} worker_pool_t;

// Create worker pool to start worker executions, there are two levels
// of synchronization. The outside is query barrier, which exists in the
// pointer between optimization and execution, inside is task barrier,
// which barrie at every workers' task finish.
worker_pool_t* create_worker_threads(int num_threads);
// Set the threads arguments when start every worker group.
void set_threads_args(worker_pool_t *worker_pool, worker_func thread_func,
                      void **args, int num_workers);
// allocate threads for group id, set bitmap of workers.
bool allocate_threads(worker_pool_t *worker_pool, Worker_exec_ctx *ctx);

// Start outside steps for all workers: parse and optimize.
bool query_execute_start(worker_pool_t *worker_pool);
// Outside parse and optimize barrier for all workers.
bool query_execute_barrier(worker_pool_t *worker_pool);
// Signal for opening barrier after worker finish parse and optimize.
bool optimize_finsh_signal(worker_thread_arg* arg);
// Handle with error before optimize for workers.
void handle_optimize_finish_error(worker_thread_arg* arg);

// Start inside task execution in idx threads' group.
bool task_execute_start(worker_pool_t *worker_pool, Worker_exec_ctx *ctx);
// Inside task execution barrier in idx threads' group.
bool task_execute_barrier(worker_pool_t *worker_pool, Worker_exec_ctx *ctx);
// Signal for opening barrier after worker finish task.
void task_finish_signal(worker_thread_arg *arg);

bool worker_pool_destroy(worker_pool_t *worker_pool);
void worker_pool_cleanup(worker_pool_t *worker_pool);

#endif // PX_WORKERPOOL_INCLUDED

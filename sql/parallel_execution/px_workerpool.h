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

#include "sql/sql_lex.h"
#include "sql/sql_class.h"
#include "sql/parallel_execution/px_exchange_info.h"

class PX_reader;

typedef void *(*worker_func)(void *);

/* Atomic operation, add 1 to n and return old value */
static inline unsigned int fetch_and_inc(unsigned int* n)
{
  unsigned int oldval;

  __asm__ __volatile__(
    "movl $1, %0 \n"
    "lock xaddl	%0, (%1) \n"
      : "=a" (oldval) : "b" (n));

  return oldval;
}

enum worker_task_type { PARSE_AND_PREPARE = 0, SLICE_TASK };

/**
  Thread arguments between coordinator and workers, There are two main funcs:
  1. Parse SQL, prepare and optimization.
  2. Multiple tasks which get from coordinator.
  Also, synchronization should be maintained by this worker_thread_arg.
*/
typedef struct worker_thread_arg {
  THD                   *worker_thd; // THD thread attach.
  int                   task_id; // task id of executor.
  worker_task_type      type; // 1. Parse and prepare; 2. Task.
  PX_exchange_info      *exchange_info; // exchange ctx.
  PX_reader             *scan_ctx; // scan_ctx.
  std::string           query_string; // query string of coordinator.

  int                   *num_workers; // tasks cnt.
  sem_t                 sem_parse_optimize_run; // sychonize the main.
  sem_t                 sem_task_run; // sychonize the workers.
  unsigned              *num_workers_done; // finished workers cnt.
  sem_t                 *sem_workers_done; // sem all workers done.
  sem_t                 *sem_tasks_done; // tasks done in loop.
  int                   *finished; // every workers finished.

  worker_func           *thread_func; // thread func pointer.
  void                  **thread_func_arg;

  AccessPath            *coordinator_root_access_path; // coordinator's plan
  JOIN                  *coordinator_join; // coordinator's JOIN
  bool                  is_equivalent_plan; // equivalent to the coordinator's plan
} worker_thread_arg;

typedef struct worker_pool_t {
  int                   num_threads; // tasks cnt.
  int                   num_workers;
  unsigned              num_workers_done; // finished workers cnt.
  sem_t                 sem_workers_done; // sem all workers done.
  sem_t                 sem_tasks_done; // tasks done in loop.
  int                   finished; // every workers finished.
  worker_func           thread_func; // thread func pointer.
  worker_task_type      type; // 1. Parse and prepare; 2. Task.
  void                  **args;
  pthread_t             *threads;
  worker_thread_arg     *thread_args;
} worker_pool_t;

/**
  Create multiple worker pool to start worker executions, there are two levels
  of synchronization. The outside is query barrier, which exists in the pointer
  between optimization and execution, the inside is task barrier, which barrie
  at every workers' task finishs.
*/
worker_pool_t* worker_pool_create(int num_threads);
int worker_pool_set(worker_pool_t *worker_pool, worker_func thread_func,
                     void **args, int num_workers);

// Start point of outside parse and optimize steps for all workers.
int worker_pool_begin_query(worker_pool_t *worker_pool);
// Waiting point of outside parse and optiize steps for all workers.
int worker_pool_wait_query(worker_pool_t *worker_pool);
// Ready to enter loop of PX_worker after optimization and before execution.
int worker_pool_optimize_end(worker_thread_arg* arg);
// Ready to enter synchrozation point when meet with error.
int worker_pool_execute_error(worker_thread_arg* arg);

// Start point of inside sub task run for all workers, control start of tasks.
int worker_pool_begin_task(worker_pool_t *worker_pool);
// Waiting point of inside sub task run for all workers, control end of tasks.
int worker_pool_wait_task(worker_pool_t *worker_pool);

int worker_pool_destroy(worker_pool_t *worker_pool);
int worker_pool_cleanup(worker_pool_t *worker_pool);

#endif // PX_WORKERPOOL_INCLUDED

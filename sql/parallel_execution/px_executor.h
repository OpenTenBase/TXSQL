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

#ifndef PX_EXECUTOR_INCLUDED
#define PX_EXECUTOR_INCLUDED

#include "sql/join_optimizer/access_path.h"  // AccessPath
#include "sql/parallel_execution/px_dfo.h"  // Dfo_mgr
#include "sql/parallel_execution/px_workerpool.h"  //worker_pool_t
#include "sql/iterators/row_iterator.h"  // RowIterator
#include "sql/sql_class.h"  // THD
#include "sql/query_result.h"  // Query_result
#include "mysql/psi/mysql_thread.h"  // mysql_mutex_t
#include "sql/parallel_execution/px_exchange_info.h"  // PX_exchange_info
#include "sql/iterators/row_iterator.h"  // RowIterator
#include "sql/parallel_execution/px_receiver.h"  // PX_receiver
#include "sql/parallel_execution/px_sender.h"  // PX_Sender

#include <unordered_map>

class JOIN;
class QEP_TAB;

/**
  PX_task is the description class for a task. It mainly contains the
  execution plan subtree in this task(the start is scan iterator or
  exchange receiver, and end is the root iterator or exchange sender).
*/
class PX_task {
 public:
  PX_task(RowIterator *itr): sub_iterator(itr) {}

  bool run(THD *thd);
  bool run_root(THD *thd);

  RowIterator *root_iterator() const { return sub_iterator; }

 private:
  RowIterator *sub_iterator{nullptr}; // physical plan of sub task.
};

/**
  Worker execute context keep the threads group of dfo, describe the
  scheduling context information.
*/
class Worker_exec_ctx {
 public:
  Worker_exec_ctx(Dfo *dfo, uint size): m_dfo(dfo)
  {
    bitmap_init(&bitmap, nullptr, size);
  }
  virtual ~Worker_exec_ctx() { bitmap_free(&bitmap); }
  void restore_parent_exec_ctx(const Worker_exec_ctx& parent)
  {
    m_dfo = parent.dfo();
    bitmap_copy(&bitmap, &parent.bitmap);
  }
  Dfo *dfo() const { return m_dfo; }

  void set_group_id(int id) { task_group_id = id; }
  int group_id() const { return task_group_id; }

  MY_BITMAP bitmap; // workers' bitmap of current task.
 private:
  int task_group_id{0}; // task group id.
  Dfo *m_dfo; // dfo of the worker execute.
};

/**
  Base class for parallel thread context.

  A parallel thread context provides access to the topology of all parallel
  executors for a given statement, parallel information for the current thread,
  and lower-level thread context.

  There are two kinds of parallel threads, the coordinator and the workers. Each
  has the same execution plan, DFO sequence and task list, as a result the
  coordinator can ask workers to run a task by its id rather than a deep copy.
*/
class PX_executor {
 public:
  PX_executor(Dfo_mgr *dfo_mgr, THD *thd)
    : m_dfo_mgr(dfo_mgr), m_thd(thd) {}
  virtual ~PX_executor() {}

  PX_executor *coordinator() {
    if (m_thd->m_is_worker) {
      assert(!m_thd->px_coordinator->m_is_worker);
      return m_thd->px_coordinator->px_executor;
    }
    return this;
  }
  query_id_t query_id() const {
    return (m_thd->m_is_worker ? m_thd->px_coordinator : m_thd)->query_id;
  }
  uint thread_id() const { return m_thd->thread_id(); }

  virtual bool prepare_task_for_dfo() { return false; }
  virtual void notify_all_workers(THD::killed_state state_to_set) {}

 protected:
  THD *thd() const { return m_thd; }
  Dfo_mgr *dfo_mgr() const { return m_dfo_mgr; }

 public:
  // Task hash table in coordinator or worker.
  std::unordered_map<int64_t, PX_task*> m_tasks_hash;

 protected:
  Dfo_mgr *m_dfo_mgr;
  THD *m_thd;
};

/**
  PX_worker is derived class of PX_executor as execution worker. It is
  embedded in the process of mysql_execute_command. The loop waits for
  the coordinator to assign tasks to it and then executes these tasks,
  it will only work stupidly.
*/
class PX_worker : public PX_executor {
 public:
  PX_worker(Dfo_mgr *dfo_mgr, THD *thd) : PX_executor(dfo_mgr, thd) {}
  ~PX_worker() {}

  /**
    In worker's SQL execution process in worker thread, worker goes to
    loop and waits for the distribution of tasks. When it receives an
    instruction to do the n-th task, differ from the classic
    architecture is that the coordinator passes the task id to worker,
    not copy of the subplan.

    After worker completes assigned task, coordinator will be responsible
    for synchronization between coordinator and workers.
  */
  virtual void loop();

  /**
    After getting DFO tree, the DFO tree is completely consistent on
    the coordinator and workers. All tasks are then pushed into array
    before the worker executes a task, prepare for execution.
  */
  virtual bool prepare_task_for_dfo() override;
};

/**
  PX_coordinator is derived class of PX_executor. PX_coordinator provides
  a schedule function that can be extended, for example, we support the
  one-to-one scheduling strategy, or full pipeline scheduling with better
  performance in the future.

  Based on these, we will design a strategy for choosing schedulers later.

  Currently, we only implement PX_parallel_coordinator.
*/
class PX_coordinator : public PX_executor {
 public:
  PX_coordinator(Dfo_mgr *dfo_mgr, THD *thd) :PX_executor(dfo_mgr, thd) {}
  ~PX_coordinator() {}

  /**
    Schedule tasks to each worker, responsible for synchronization. This
    is extendable interface which can implement more scheduling strategy.

    @param worker_pool worker pool.
    @return true when error, false when success.
  */
  virtual bool schedule(worker_pool_t *worker_pool);

  /**
    Inner schedule method, which control dispatch tasks in scheduler.

    @param worker_pool worker pool
    @param th_arg_array thread arguments array
    @return true when error, false when success.
  */
  virtual bool schedule_dfo_pair_inner(worker_pool_t *worker_pool,
                 worker_thread_arg **th_arg_array) { return false; }

  /**
    Create multiple worker threads to rebuild query execution, by parse,
    prepare, optimize and execute.

    @param worker_pool worker pool.
    @param th_arg_array thread argument array.
    @return true when error, false when success.
  */
  virtual bool create_worker_context(worker_pool_t *&worker_pool,
                 worker_thread_arg **&th_arg_array);

  /**
    At last, after all tasks are scheduled except root dfo, root task
    will be executed, currently, we set root task run in coordinator.

    @return true when error, false when success.
  */
  virtual bool run_root_dfo_task();

  /**
    Notify SIGNAL to each worker, set task finished when been killed.

    @param state_to_set signal.
  */
  virtual void notify_all_workers(THD::killed_state state_to_set) override;

  /**
    Check equivalence between coordinator and workers.

    @param worker_pool worker pool.
    @return false if equal, true if unequal.
  */
  bool check_equivalence(worker_pool_t *worker_pool);

 protected:
  PX_exchange_info *exchange_info{nullptr}; // current exchange info.
  typedef Prealloced_array<THD *, 60> THD_array;
  THD_array thd_list{4}; // THD array for all workers.
};

/**
  This coordinator schedule all tasks by dfo pair. PX_COORDINATOR is
  needed to gather data from lower iterator tree.
*/
class PX_parallel_coordinator : public PX_coordinator {
 public:
  PX_parallel_coordinator(Dfo_mgr *dfo_mgr, THD *thd)
    : PX_coordinator(dfo_mgr, thd) {}
  ~PX_parallel_coordinator() {}

  /**
    Schedule <child, parent> pair, there are more than two dfos in
    process, send task instructions(task id) to workers.

    @param worker_pool worker pool.
    @param th_arg_array worker thread arguments.
  */
  virtual bool schedule_dfo_pair_inner(worker_pool_t *worker_pool,
                 worker_thread_arg **th_arg_array) override;

 private:
  /**
    Set exchange info when scheduling dfo pair.

    @param num_workers number of workers.
    @param args thread func arguments.
    @param child child execute ctx.
    @param parent parent execute ctx.
    @param last_parent last parent execute ctx.
  */
  void attach_exchange_info(int num_workers, worker_thread_arg **args,
         Worker_exec_ctx *last_parent,
         Worker_exec_ctx *child, Worker_exec_ctx *parent);

  /**
    Prepare for scheduling the single dfo, set the arguments.
  
    @param num_workers number of workers.
    @param args thread func arguments.
    @param exec_ctx execute ctx.
  */
  void prepare_schedule_single_dfo(int num_workers,
         worker_thread_arg **args, Worker_exec_ctx *exec_ctx);

};

#endif  // PX_EXECUTOR_INCLUDED

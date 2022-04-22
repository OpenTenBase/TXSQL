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
  PX_task is the description class for a task. It mainly contains the execution
  plan subtree in this task(the start is the scan iterator or exchange receiver,
  and the end is the root iterator or exchange sender). For some special tasks,
  like the leaf node task contains QEP_TAB for dynamic segmentation, and middle
  node has exchange execution context.
*/
class PX_task {
 public:
  PX_task()
    : sub_iterator(nullptr),
      ex_sender(nullptr),
      ex_receiver(),
      query_result(nullptr),
      fields(nullptr),
      join(nullptr),
      task_id(-1) {}
  PX_task(RowIterator *itr, RowIterator *sender, RowIterator *receiver,
    Query_result *qr, mem_root_deque<Item*> *f, JOIN *j)
    : sub_iterator(itr),
      ex_sender(sender),
      ex_receiver(receiver),
      query_result(qr),
      fields(f),
      join(j),
      task_id(-1) {}

  bool run(THD *thd);
  bool run_root(THD *thd);

  void set_task_id(int id) { task_id = id; }
  void set_exchange_info(PX_exchange_info *info) { exchange_info = info; }
  void attach_exchange_info(RowIterator *iterator);
  bool choose_parallel_table_scan();

 public:
  PX_table_descriptor *px_table_descriptor{nullptr};

  PX_exchange_info *exchange_info{nullptr};  // exchange execute context.
  RowIterator *sub_iterator;  // physical plan of sub task.
  RowIterator *ex_sender, *ex_receiver;  // sender and receiver of sub task.

 private:
  /**
    Query result of sub task. Interface of Query_result_mq is join and m_handle.
    We extract the fields array from join->tmp_fields_array, and send the data
    by m_handle, put data into MQ, currently used by root task.
  */
  Query_result *query_result;
  mem_root_deque<Item *> *fields;  // item array of the MQ in and out.
  JOIN *join;  // JOIN structure of this task which is subset of whole plan.
  int task_id;  // task id info.
};

/**
  Base class for coordinator and worker, in the design, the coordinator and the
  worker have the same execution plan, the same DFO sequence and the same task
  array. So the coordinator can direct workers to do some tasks.
*/
class PX_executor {
 public:
  PX_executor(Dfo_mgr *dfo_mgr, THD *thd)
    : m_tasks_hash(),
      m_dfo_mgr(dfo_mgr),
      m_thd(thd) {}
  virtual ~PX_executor() {}

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
  PX_worker is derived class of PX_executor as execution worker. It is embedded
  in the process of mysql_execute_command. The loop waits for the coordinator to
  assign tasks to it and then executes these tasks, it will only work stupidly.
*/
class PX_worker : public PX_executor {
 public:
  PX_worker(Dfo_mgr *dfo_mgr, THD *thd) : PX_executor(dfo_mgr, thd) {}
  ~PX_worker() {}

  /**
    In a SQL execution process in worker thread, each worker goes to loop and
    waits for the distribution of tasks. When it receives an instruction to do
    the n-th task, differ from the classic architecture is that the coordinator
    passes the task id to the worker, not copy of the subplan.

    After worker completes the assigned task, coordinator will be responsible
    for synchronization between coordinator and workers.
  */
  virtual void loop();

  /**
    After getting the DFO tree, the DFO tree is completely consistent on the
    coordinator and the worker. All tasks are then pushed into a queue before
    the worker executes a task, prepare for execution.
  */
  virtual bool prepare_task_for_dfo() override;
};

/**
  PX_coordinator is derived class of PX_coordinator as coordinator. PX_coordinator
  provides a schedule function that can be extended, for example, we support simple
  one-to-one scheduling strategy, and will also support embedded scheduling with
  union operators, or full pipeline scheduling with better performance.

  Based on these, we will design a strategy for choosing schedulers later(TODO).

  Currently, we only implement PX_sequential_coordinator which support one-to-one.
*/
class PX_coordinator : public PX_executor {
 public:
  PX_coordinator(Dfo_mgr *dfo_mgr, THD *thd) :
    PX_executor(dfo_mgr, thd) {}
  ~PX_coordinator() {}

  /**
    Schedule tasks to each worker, responsible for the synchronization. This is
    a extendable interface which can implement more scheduling strategy.

    @param worker_pool worker pool.
  */
  virtual bool schedule(worker_pool_t *worker_pool) { return false; }

  /**
    Create multiple worker threads to rebuild query execution, by re-parsing,
    re-optimizing and execute.

    @param worker_pool worker pool.
    @param th_arg_array thread argument array.
  */
  virtual bool create_worker_context(worker_pool_t *&worker_pool,
                                       worker_thread_arg **&th_arg_array);

  /**
    Notify SIGNAL to each worker, set task finished when been killed.

    @param state_to_set signal.
  */
  virtual void notify_all_workers(THD::killed_state state_to_set) override;

 protected:
  typedef Prealloced_array<THD *, 60> THD_array;
  THD_array thd_list{4};
};

/**
  Sequential scheduler schedules all tasks one-to-one and assigns them to each
  worker for execution. In the process, exchange receiver contains a temporary
  table that can cache the full amount of data.
*/
class PX_sequential_coordinator : public PX_coordinator {
 public:
  PX_sequential_coordinator(Dfo_mgr *dfo_mgr, THD *thd) :
    PX_coordinator(dfo_mgr, thd) {}
  ~PX_sequential_coordinator() {}

  /**
    One-to-one scheduling strategy

    @param worker_pool worker pool.
  */
  virtual bool schedule(worker_pool_t *worker_pool) override;

  /**
    Schedule <child, parent> pair, there are only two dfos in scheduling process
    every time, send task instructions(task id) to workers.

    @param worker_pool worker pool.
    @param th_arg_array worker thread arguments.
  */
  virtual bool schedule_dfo_pair_inner(worker_pool_t *worker_pool,
                 worker_thread_arg **th_arg_array);

  /**
    At last, after all tasks whose parent task is not root dfo, root task will
    be executed, currently, we set root task always to run in coordinator.
  */
  bool run_root_dfo_task();

  /**
    Prepare tasks for parent and child dfo, ready to schedule.

    @param dfos only contains two dfos, child and parent.
  */
  virtual bool prepare_task_execution_for_dfo(const std::vector<Dfo*> dfos);

  /**
    Check equivalence between coordinator and workers.

    @param worker_pool worker pool.
    @return false if equal, true if unequal.
  */
  bool check_equivalence(worker_pool_t *worker_pool);

 private:
  PX_exchange_info *exchange_info{nullptr};
};

#endif  // PX_EXECUTOR_INCLUDED
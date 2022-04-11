#include "px_executor.h"
#include "sql/sql_lex.h"  // LEX
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_tmp_table.h"  // create_tmp_table
#include "sql/sql_executor.h"  // QEP_TAB
#include "sql/sql_parse.h"  // mysql_reset_thd_for_next_command
#include "sql/sql_db.h"  // mysql_change_db
#include "sql/iterators/row_iterator.h"  // RowIterator
#include "sql/protocol.h" // Protocol
#include "sql/log.h" // For debug to be deleted.
#include "sql/iterators/basic_row_iterators.h"  // TableScanIterator
#include "sql/parallel_execution/px.h"

extern bool px_partition(uint dop, void *&scan_ctx, TABLE *table, PX_SCAN_TYPE type,
                         uint keyno, TABLE_REF *ref, bool reverse_scan = false);

/**
  Attach exchange exec context to exchange receiver and exchange sender of this
  PX_task by traverse.
*/
void PX_task::attach_exchange_info(RowIterator *iterator)
{
  if (iterator->type() == RowIterator::PHY_PX_SEND)
    static_cast<PX_sender*>(iterator)->set_exchange_info(exchange_info);
  if (iterator->type() == RowIterator::PHY_PX_RECEIVE)
    static_cast<PX_receiver*>(iterator)->set_exchange_info(exchange_info);
  for (unsigned i = 0; i < iterator->m_children.size(); ++i)
    attach_exchange_info(iterator->m_children.at(i));
}

/**
  Currently we just pick the leftmost table that can be scanned parallelly. More
  elegantly, we'll choose which table to read in parallel based on the code model.
*/
bool PX_task::choose_parallel_table_scan()
{
  bool ret = false;
  PX_table_descriptor *px_descriptor = nullptr;
  RowIterator *child = sub_iterator;
  // Currently support PHY_TABLE_SCAN/PHY_INDEX_SCAN/PHY_INDEX_RANGE_SCAN/PHY_REF
  // parallelly scan by PX_reader, choose the left most child of the iterator tree.
  while(nullptr != child) {
    if (child->type() == RowIterator::PHY_INDEX_RANGE_SCAN ||
        child->type() == RowIterator::PHY_TABLE_SCAN ||
        child->type() == RowIterator::PHY_INDEX_SCAN ||
        child->type() == RowIterator::PHY_REF) {
      TableRowIterator *scan = static_cast<TableRowIterator *>(child);
      scan->set_parallel_scan();
      assert(!px_descriptor);
      px_descriptor = scan->get_table_descriptor();
      if (!px_descriptor) {
        return true;
      }
      break;
    } else {
      if (0 == child->m_children.size()) {
        child = nullptr;
      } else {
        child = child->m_children.at(0);
      } 
    }
  }
  px_table_descriptor = px_descriptor;
  return ret;
}

/**
  Main work flow in each worker, in mysql_execute_command function, a loop has
  been embeded in execute_inner point. After execution over in worker, cleanup
  keep original. Main steps are following:
   1. Parse SQL string, prepare it and optimize.
   2. Get all tasks from coordinator's instructions and execute every task.
   3. Cleanup in each thread.
  TODO: the query which has semicolon should be considered.
*/
static void *execute_inner_in_worker(void *args)
{
  int error = 0;
  worker_thread_arg *worker_info = (worker_thread_arg *)args;
  THD *thd = worker_info->worker_thd;
  thd->thread_stack = (char *)&thd;
  thd->mem_root = new MEM_ROOT();
  // Restore the THD evironment for current_thd.
  current_thd = thd;
  THR_MALLOC = &(thd->mem_root);
  Parser_state *parser_state = worker_info->parse_state;
  parser_state->init(thd, thd->query().str, thd->query().length);

  sql_print_information("begin to init the THD.");
  // Reset THD for next command.
  mysql_reset_thd_for_next_command(thd);
  lex_start(thd);
  thd->m_parser_state = parser_state;
  thd->m_parser_state = nullptr;

  // TODO: handle situation const char *found_semicolon = nullptr;
  bool err = thd->get_stmt_da()->is_error();

  if (!err) err = parse_sql(thd, parser_state, nullptr);

  sql_print_information("parse sql error: %d", err);

  if (!err) {
    if (!thd->is_error()) {
      error = mysql_execute_command(thd, true);
      sql_print_information("after mysql execute command return %d", error);
    }
  }

  if (error) {
    thd->worker_arg->error = true;
    worker_pool_optimize_end(thd->worker_arg);//TODO: check whether exit.
  }

  // LEX cleanup for the local thread execution.
  thd->lex->destroy();
  thd->end_statement();
  thd->cleanup_after_query();
  thd->mem_root->Clear();

  // mysql_trx_list assert to 0 at last if no release resources.
  thd->release_resources();

  return nullptr;
}

/**
  The worker task init.
*/
static void *init_task_handler(void *arg) {
  worker_thread_arg *worker_info = (worker_thread_arg *)arg;
  int task_id = worker_info->task_id;
  THD *thd = worker_info->worker_thd;
  PX_executor *px_executor = thd->px_executor;
  PX_task *task = px_executor->m_tasks_hash[task_id];
  task->set_exchange_info(worker_info->exchange_info);
  task->attach_exchange_info(task->sub_iterator);
  static_cast<PX_sender*>(task->ex_sender)->init();
  return nullptr;
}

/**
  The worker task init.
*/
// static void *close_task_handler(void *arg) {
//   worker_thread_arg *worker_info = (worker_thread_arg *)arg;
//   int task_id = worker_info->task_id;
//   THD *thd = worker_info->worker_thd;
//   PX_executor *px_executor = thd->px_executor;
//   PX_task *task = px_executor->m_tasks_hash[task_id];
//   // task->set_exchange_info(worker_info->exchange_info);
//   // task->attach_exchange_info(task->sub_iterator);
//   static_cast<PX_sender*>(task->ex_sender)->end();
//   return nullptr;
// }

/**
  The worker task execution function, get the task id assigned by the coordinator
  and execute it immediately. After the execution, detach it from exec context.
*/
static void *execute_task_in_worker(void *arg)
{
  worker_thread_arg *worker_info = (worker_thread_arg *)arg;
  int task_id = worker_info->task_id;
  // Get the PX_worker pointer from THD.
  THD *thd = worker_info->worker_thd;
  thd->px_scan_ctx = worker_info->scan_ctx;
  PX_executor *px_executor = thd->px_executor;
  sql_print_information("px task count is %d in executor. %p",
    px_executor->m_tasks_hash.size(), px_executor);
  PX_task *task = px_executor->m_tasks_hash[task_id];
  task->run(thd);
  sql_print_information("finish task and detach receiver [%d]",
    thd->worker_id);
  // Detach receiver proc from sender MQ handler.
  // PX_mq_handle **senders = task->exchange_info->get_sender_mqh();
  // PX_proc *receiver_proc = senders[thd->worker_id-1]->get_receiver();
  // senders[thd->worker_id-1]->mq_detach(receiver_proc);
  task->exchange_info->detach_sender(thd->worker_id);
  static_cast<PX_sender *>(task->ex_sender)->end();
  return nullptr;
}

/**
  When common task executing, read data from the child to the MQ, or read the
  data from the MQ to a temporary table.

  Differ from root task running, no need to send result to client, either send
  result set metadata.
*/
bool PX_task::run(THD *thd)
{
  // do the init of worker before the execute for every task.
  assert(px_table_descriptor && px_table_descriptor->table());

  if (px_table_descriptor) {
    TABLE *table = px_table_descriptor->table();
    assert(thd->px_scan_ctx);
    int error = table->file->px_worker_init(thd->px_scan_ctx);
    if (error) {
      table->file->print_error(error, MYF(0));
      return true;
    }
  }

  sql_print_information("======>run the task which has itr: %s",
    sub_iterator->str().c_str());
  if (sub_iterator->Init()) return true;

  for (;;) {
    int error = sub_iterator->Read();

    if (error > 0 || thd->is_error()) // TODO: killed in worker.
      return true;
    else if (error < 0)
      break;
    else if (thd->killed) {
      thd->send_kill_message();
      return true;
    }  
  }

  return false;
}

/**
  In last PX_task run as root task, we directly send the data to client, before
  that, result metadata also need to be sent. Lastly send eof as the task finish.

  This task is executed in coordinator thread, the original thread which receive
  the original query and start execution.
*/
bool PX_task::run_root(THD *thd)
{
  // As coordinator execute last root task, send result to client.
  if (query_result->start_execution(thd))
    return true;

  // Send result set metadata of each field in fields array.
  if (query_result->send_result_set_metadata(
        thd, *fields, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  thd->get_stmt_da()->reset_diagnostics_area();

  if (sub_iterator->Init()) return true;

  for (;;) {
    int error = sub_iterator->Read();

    if (error > 0 || thd->is_error())
      return true;
    else if (error < 0)
      break;
    else if (thd->killed) {
      assert(thd->px_executor);
      thd->px_executor->do_all_for_workers(THD::KILL_QUERY);
      thd->send_kill_message();
      return true;
    }

    if (query_result->send_data(thd, *fields))
      return true;
  }

  return query_result->send_eof(thd);
}

void PX_coordinator::create_mq_channel_for_task_pair()
{

}

void PX_coordinator::do_all_for_workers(THD::killed_state state_to_set)
{
  Prealloced_array<THD *, 60>::iterator iter = thd_list.begin();
  for(; iter != thd_list.end(); ++iter) {
    mysql_mutex_lock(&(*iter)->LOCK_thd_data);
    (*iter)->awake(state_to_set);
    mysql_mutex_unlock(&(*iter)->LOCK_thd_data);
  }
}

/**
  Convert from dfo tree to task pair array and task array by subsequent traversal.
  This function will be invoked in coordinator and workers to get the same tasks
  pair array and task array, schedule them one by one:

  We get the task tree and task pair array and task array after this function.
*/
bool PX_worker::prepare_task_for_dfo()
{
  // Create tmp table and allocate ref items slice. Create px tasks into
  // PX_scheduler and ready to schedule. (TODO)
  for (uint i = 0; i < m_dfo_mgr->m_dfos.size(); ++i) {
    Dfo *dfo = m_dfo_mgr->m_dfos.at(i);
    dfo->m_task_id = dfo->m_dfo_id;
    RowIterator *iter = dfo->m_root_iterator;
    JOIN *join = dfo->m_join;
    PX_task *task = new (m_thd->mem_root) PX_task(iter, dfo->m_sender, dfo->m_receiver,
      nullptr, nullptr, join);
    task->set_task_id(dfo->m_dfo_id);
    m_tasks_hash.insert(std::pair<int64_t, PX_task*>(dfo->m_dfo_id, task));
    sql_print_information("attach root iterator: %s in thd: %p task id: [%d]",
      iter->str().c_str(), m_thd, dfo->m_dfo_id);
    if (dfo->is_dfo_has_scan() && task->choose_parallel_table_scan())
      return true;
  }
  return false;
}

/**
  Worker loop is in execute_inner_in_worker, it receive all tasks' instructions
  which received from coordinator. Every task start with `sem_wait` sem_task_run
  semaphore, run the rask and arrive the barrier to start the next one.

  After executing all received tasks, coordinator set finish, destroy semaphore
  before exit the function.
*/
void PX_worker::loop()
{
  worker_func thread_func;
  int num_workers_done = 0;
  worker_thread_arg *thread_arg = thd()->worker_arg;
  void *thread_func_arg;

  while (true) {
    sem_wait (&thread_arg->sem_task_run);
    if (*thread_arg->finished)
      break;

    thread_func = *(thread_arg->thread_func);
    thread_func_arg = *(thread_arg->thread_func_arg);

    // All tasks' handled here.
    (*thread_func)(thread_func_arg);

    num_workers_done = fetch_and_inc(thread_arg->num_workers_done) + 1;
    if (num_workers_done == *thread_arg->num_workers) {
      // All workers have done.
      sem_post(thread_arg->sem_workers_done);
    }
  }

  sem_destroy (&thread_arg->sem_task_run);
  sql_print_information("worker's executing is over.");
}

/**
  Execute by using PX, schedule all tasks one-by-one in coordinator.

  We create other dop_parallel_degree threads as workers to attach with new THD.
  Original THD is leader which will send worker execution info to new threads,
  and control the synchronization process, workers get partial execution info
  and start their own tasks.

  After generating all partial plans' iterators, execute iterator query in all
  threads, finally leader send the last result data.
*/
bool PX_sequential_coordinator::schedule(worker_pool_t *worker_pool)
{
  bool ret = false;
  worker_func thread_func;
  int num_threads = worker_pool->num_threads;
  std::string worker_query(thd()->query().str, thd()->query().length);
  worker_thread_arg **th_arg_array = (worker_thread_arg**)
    malloc(sizeof(worker_thread_arg *) * num_threads);

  for (int i = 0; i < num_threads; ++i)
    th_arg_array[i] = (worker_thread_arg*) malloc(sizeof(worker_thread_arg));

  // TODO: bind the task to CPU core, for more stable exection.
  // Parse and optimize the SQL query, generate the physical tasks for query.
  for (int i = 0; i < num_threads; ++i) {
    THD *worker_new_thd = new THD();
    // TODO: add variables of THD copy.
    worker_new_thd->variables.optimizer_switch = thd()->variables.optimizer_switch;
    mysql_change_db(worker_new_thd, thd()->db(), false);
    // THD to thd list for state setting.
    thd_list.push_back(worker_new_thd);
    // Set the thread_id of the THD by Global_THD_Manager, in temp table
    // creatation, thread_id is needed to name a temp file in disk.
    worker_new_thd->set_new_thread_id();
    worker_new_thd->m_is_worker = true;
    worker_new_thd->worker_id = i;

    std::string sub_query = worker_query;
    worker_new_thd->set_query(sub_query.c_str(), sub_query.length());
    th_arg_array[i]->worker_thd = worker_new_thd;
    sql_print_information("create new thd: %s in %d-th thread", sub_query.c_str(), i);

    Parser_state *parse_state = new Parser_state();
    th_arg_array[i]->parse_state = parse_state;

    // In each worker THD, synchronization is needed to keep up with master.
    worker_new_thd->worker_arg = &worker_pool->thread_args[i];

    Query_expression *unit = thd()->lex->unit;
    if (unit != nullptr) {
      worker_new_thd->worker_arg->coordinator_root_access_path = unit->root_access_path();
    } else {
      worker_new_thd->worker_arg->coordinator_root_access_path = nullptr;
    }
    worker_new_thd->worker_arg->coordinator_join =
        unit->is_union() ? nullptr : unit->first_query_block()->join;
  }

  thread_func = execute_inner_in_worker;
  worker_pool_set(worker_pool, thread_func, (void **)th_arg_array, num_threads);
  /**
    Start multiple threads to run a query, a barrier is needed to synchronize
    all workers' parse and optimize phase. Before execute each worker's task,
    some checks are needed, like execution plan consistency between master and
    worker.
                                    / worker parse | optimize \
                                   /                           \
    coordinator parse | optimize  --- worker parse | optimize -- dispatch tasks.
                                   \                           /
                                    \ worker parse | optimize /
  */ 
  worker_pool_begin_query(worker_pool);
  // Waiting for all worker has done prepare work, adjust semaphore and go on.
  worker_pool_wait_query(worker_pool);
  // consistency check between master and workers.
  if (cdb_plan_equivalence_comparison_enabled) {
    for (int i = 1; i < num_threads; ++i) {
      if (!worker_pool->thread_args[i].is_equivalent_plan) {
        sql_print_error("%d-th/%d thread(%d) generated an unequal plan", i, num_threads, th_arg_array[i]->worker_thd->thread_id());
        // TODO: fallback to execute in single thread
        goto end_workers;
      }
    }
  }

  if (check_error(worker_pool, "Parse and Optimize")) return true;

  /**
    Dispatch tasks to all workers, we dispatch all task pairs(tp) to all workers
    by using the tasks which convert from dfos. like
                        / tp1 \           / tp2 \                  / tpn \
                       /       \         /       \                /       \
    dispatch tasks  ---   tp1  barrier --   tp2  barrier ......---   tpn  barrier 
                       \       /         \       /                \       /
                        \ tp1 /           \ tp2 /                  \ tpn /

  */
  sql_print_information("ready to dispatch every tasks to executor.");
  // TODO: More complex task scheduling.
  ret = schedule_dfo_pair_inner(worker_pool, th_arg_array);
  sql_print_information("finish all tasks in coordinator thread.");

end_workers:
  // Every worker jump out of loop(), 
  worker_pool_destroy(worker_pool);

  /** 
    Every worker exist from THD thread, after these step, only one thread remain
    to cleanup query execution in coordinator.
            / tpn \           / worker exit \
           /       \         /               \
    ... ---   tpn  barrier -->  worker exit -- coordinator cleamup --> finish
           \       /         \               /
            \ tpn /           \ worker exit /
  */
  worker_pool_begin_query(worker_pool);
  worker_pool_wait_query(worker_pool);

  sql_print_information("finish cleanup after all task thread.");
  worker_pool_cleanup(worker_pool);

  exchange_info->destroy_release();
  Prealloced_array<THD *, 60>::iterator iter = thd_list.begin();
   for(; iter != thd_list.end(); ++iter)
     delete (*iter);

  return ret;
}

/**
  In the sequential scheduler, the DFO tree is traversed subsequently, two DFOs
  are executed each time, they are converted into task pairs, and then executed.
  After the execution, the child task is set to be finished, and loop continues
  until the root task is executed.

  As shown as below, the pair <task1, task3> is scheduled firstly, since task3
  has two children, and then <task2, task3> is scheduled.

          root dfo           root task              schedule:
            |                    |                  <task1, task3>
           dfo5                task5                <task2, task3>
          /    \     ---->     /   \       --->     <task3, task5>
         dfo3  dfo4         task3  task4            <task4, task5>
        /   \                /  \                   <task5, root task>
    dfo1     dfo2         task1 task2               

  Every schedule pair is one dfo and its parent dfo. child execute and send data
  to MQ, and parent receive data from MQ and saved in temp table cache or send
  to client.
*/
bool PX_sequential_coordinator::schedule_dfo_pair_inner(worker_pool_t *worker_pool,
                       worker_thread_arg **th_arg_array)
{
  bool ret = false;

  std::vector<Dfo*> ready_dfos;
  while(true) {
    if (dfo_mgr()->get_ready_dfos(ready_dfos)) {
      ret = true;
      break;
    } else if (ready_dfos.size() == 0) {
      // No dfos to schedule any more.
      break;
    } else if (ready_dfos.size() != 2) {
      ret = true;
      assert(0);// TODO: error situation.
    } else {
      // Get dfo pair every time and schedule dfo pair <child, parent>.
      // 0 is child,1 is parent, as follows:
      //
      //      parent  <-- 1
      //     /
      //  child  <-- 0
      Dfo *child = ready_dfos[0];
      Dfo *parent = ready_dfos[1];
      int child_task_id = child->m_dfo_id;
      int parent_task_id = parent->m_dfo_id;
      // TODO: Cores assignment of chil and parent.
      int parent_num_threads = worker_pool->num_threads / 2;
      int child_num_threads = worker_pool->num_threads - parent_num_threads;

      // TODO: The following code will be inserted into PX_Receiver::init.
      exchange_info = new (thd()->mem_root) PX_exchange_info(thd(), 
        PX_GATHER_EXCHANGE, /*exchange_type=*/
        PX_MQ_CHANNEL, /*channel_type=*/
        thd()->variables.cdb_parallel_degree, /*senders=*/
        1, /*receivers=*/
        PX_COMPACT_ROW, /*exchange_format=*/
        false/*need_materialize=*/);

      exchange_info->init();

      // Set the arguments of child tasks.
      for (int i = 0; i < child_num_threads; ++i)
        th_arg_array[i]->task_id = child_task_id;
      // Set the arguments of parent tasks.
      for (int i = 0; i < parent_num_threads; ++i)
        th_arg_array[i]->task_id = parent_task_id;
      // Set exchange info to all children tasks and parent tasks.
      for (int i = 0; i < worker_pool->num_threads; ++i)
        th_arg_array[i]->exchange_info = exchange_info;

      if (prepare_task_execution_for_dfo(ready_dfos)) {
        ret = true;
        break;
      }

      void *px_reader = nullptr;
      PX_table_descriptor *descriptor = m_tasks_hash[child_task_id]->px_table_descriptor;
      assert(descriptor);
      bool px_partition(uint dop, void *&scan_ctx, TABLE *table, PX_SCAN_TYPE type,
                         uint keyno, TABLE_REF *ref, bool reverse_scan = false);
      px_partition(worker_pool->num_threads, px_reader, descriptor->table(),
                   descriptor->type(), descriptor->keyno(),
                   descriptor->ref(), descriptor->reverse_scan());
      assert(px_reader != nullptr);
      thd()->px_scan_ctx = px_reader;
      for (int i = 0; i < worker_pool->num_threads; ++i) {
        th_arg_array[i]->task_id = child_task_id;
        th_arg_array[i]->exchange_info = exchange_info;
        th_arg_array[i]->scan_ctx = static_cast<PX_reader*>(px_reader);
      }

      // Prepare for Sender and Receiver init work.
      worker_func thread_func = init_task_handler;
      worker_pool_set(worker_pool, thread_func, (void **)th_arg_array,
                      worker_pool->num_threads);
      worker_pool_begin_task(worker_pool);
      worker_pool_wait_task(worker_pool);
      PX_receiver *r = static_cast<PX_receiver *>(parent->m_receiver);
      r->set_exchange_info(exchange_info);
      r->init();

      // Dispatch every tasks in executor to all workers, task run.
      thread_func = execute_task_in_worker;
      worker_pool_set(worker_pool, thread_func, (void **)th_arg_array,
                      worker_pool->num_threads);

      // Enter into inner worker's loop() until all tasks have been scheduled, we
      // use one-by-one task pair scheduling strategy for better control of CPU cores.
      worker_pool_begin_task(worker_pool);

      // After all tasks except root task have been finished, start to schedule root
      // task and send result to client, Currently only two-stages needed to schedule.
      sql_print_information("run the root task for the scheduler.");
      ret = run_root_dfo_task();
      sql_print_information("run the root task for the scheduler over.");

      worker_pool_wait_task(worker_pool);

      exchange_info->detach_receiver(0);
      sql_print_information("detach receiver for the exchange.");

      // // Close for Sender and Receiver work.
      // thread_func = close_task_handler;
      // worker_pool_set(worker_pool, thread_func, (void **)th_arg_array,
      //                 worker_pool->num_threads);
      // worker_pool_begin_task(worker_pool);
      // worker_pool_wait_task(worker_pool);
      // r = static_cast<PX_receiver *>(parent->m_receiver);
      // r->end();
      // exchange_info->clean();
      // TODO: add clean of exchange info.

      // Set the child dfo finished, currently only one stage supported.[TODO]
      child->set_dfo_finished();
      break;
    }
  }

  return ret;
}

/** 
  Run root task lastly, send result metadata and data to client, this executed
  in coordinator thread, which get the original query from client.
*/
bool PX_sequential_coordinator::run_root_dfo_task()
{ 
  bool ret = false;
  Dfo *root_dfo = dfo_mgr()->m_root_dfo;
  RowIterator *iterator = root_dfo->m_root_iterator;
  RowIterator *receiver = root_dfo->m_receiver;
  Query_result *result = thd()->lex->unit->query_result();
  mem_root_deque<Item *> *fields = thd()->lex->unit->get_field_list();
  JOIN *join = root_dfo->m_join;
  PX_task *task = new (thd()->mem_root)
                  PX_task(iterator, nullptr, receiver, result, fields, join);
  task->set_exchange_info(exchange_info);
  task->attach_exchange_info(task->sub_iterator);
  // Init of task moved to init of task.
  ret = task->run_root(thd());
  sql_print_information("execute root task finish and detach the sender");
  return ret;
}

/**
  Analyze the two DFOs scheduled each time, and finally convert them into task
  pairs, and set the properties of each task, such as which table the task will
  scan in parallel. More complex properties will be considerred later.
*/
bool PX_sequential_coordinator::prepare_task_execution_for_dfo(
  const std::vector<Dfo*> dfos)
{
  PX_task *task = nullptr;
  JOIN *join = nullptr;
  RowIterator *iter = nullptr;
  RowIterator *sender = nullptr;
  RowIterator *receiver = nullptr;
  Query_result *result = nullptr;
  mem_root_deque<Item *> *fields = nullptr;
  // Traverse dfos to executable tasks.
  for (unsigned i = 0; i < dfos.size(); ++i) {
    Dfo *dfo = dfos[i];
    if (!dfo->m_is_root_dfo) {
      join = dfo->m_join;
      iter = dfo->m_root_iterator;
      sender = dfo->m_sender;
      receiver = dfo->m_receiver;
    } else {
      join = dfo->m_join;
      iter = dfo->m_root_iterator;
      sender = dfo->m_sender;
      receiver = dfo->m_receiver;
      result = m_thd->lex->unit->query_result();
      fields = m_thd->lex->unit->get_field_list();
    }

    task = new (thd()->mem_root) PX_task(iter, sender, receiver, result, fields, join);
    // Choose one TABLE to split for parallelly execution, currently choose the
    // first TABLE in this dfo to parallelly scan.(TODO)
    if (dfo->is_dfo_has_scan() && task->choose_parallel_table_scan())
      return true;
    task->set_task_id(dfo->m_dfo_id);
    m_tasks_hash.insert(std::pair<int64_t, PX_task*>(dfo->m_dfo_id, task));
  }

  return false;
}

bool PX_sequential_coordinator::check_error(worker_pool_t *worker_pool, 
  const char *err_msg) 
{
  // Check error in Coordinator and send error.
  for (int i = 0; i < worker_pool->num_threads; ++i) {
    if (worker_pool->thread_args[i].error) {
      // TODO: add the error famework for px execution.
      sql_print_warning("worker error px.");
      my_error(ER_PX_WORKER_ERROR, MYF(0), err_msg);
      return true;
    }
  }
  return false;
}

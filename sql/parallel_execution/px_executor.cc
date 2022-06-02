#include "px_executor.h"
#include "sql/sql_lex.h"  // LEX
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_tmp_table.h"  // create_tmp_table
#include "sql/sql_executor.h"  // QEP_TAB
#include "sql/sql_parse.h"  // mysql_reset_thd_for_next_command
#include "sql/sql_db.h"  // mysql_change_db
#include "sql/iterators/row_iterator.h"  // RowIterator
#include "sql/protocol.h" // Protocol
#include "sql/pfs_batch_mode.h"  // PFSBatchMode
#include "sql/log.h" // For debug to be deleted.
#include "sql/iterators/basic_row_iterators.h"  // TableScanIterator
#include "sql/parallel_execution/px.h"
#include "px_optimizer_context.h" // post_init_worker_thd

extern bool px_partition(uint dop, void *&scan_ctx, TABLE *table, PX_SCAN_TYPE type,
                         uint keyno, TABLE_REF *ref, bool reverse_scan, uint &partitions);

/**
  Main work flow in each worker, in mysql_execute_command function, a loop has
  been embeded in execute_inner point. After execution over in worker, cleanup
  keep original. Main steps are following:
   1. Parse SQL string, prepare it and optimize.
   2. Get all tasks from coordinator's instructions and execute every task.
   3. Cleanup in each thread.
  the query which has semicolon should be forbidden.
*/
static void *rebuild_query_execution(void *args)
{
  int error = 1;
  worker_thread_arg *worker_info = (worker_thread_arg *)args;
  THD *thd = worker_info->worker_thd;
  thd->thread_stack = (char *)&thd;
  thd->mem_root = new MEM_ROOT();

  // Restore the THD evironment for current_thd.
  current_thd = thd;
  THR_MALLOC = &(thd->mem_root);
  Parser_state *parser_state = new (thd->mem_root) Parser_state();
  if (nullptr == parser_state) return nullptr;
  if (parser_state->init(thd, thd->query().str, thd->query().length))
    return nullptr;

  // Reset THD for next command.
  mysql_reset_thd_for_next_command(thd);
  lex_start(thd);

  thd->m_parser_state = nullptr;
  // TODO: handle situation const char *found_semicolon = nullptr;
  bool err = thd->get_stmt_da()->is_error();

  if (!err) err = parse_sql(thd, parser_state, nullptr);
  sql_print_information("rebuild_query_execution:%d parse return[%d]", __LINE__, err);

  if (!err && !thd->is_error()) error = mysql_execute_command(thd, true);
  sql_print_information("rebuild_query_execution:%d execute return[%d] state[%d]", 
    __LINE__, error, thd->px_worker_state);

  if (error && thd->px_worker_state <= PX_WORKER_PARSE_OPTIMIZE)
    handle_optimize_finish_error(thd->worker_arg);

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
  The worker task execution function, get the task id assigned by the
  coordinator and execute it immediately. After the execution, deatach
  it from exec context.
*/
static void *execute_task_in_worker(void *arg)
{
  worker_thread_arg *worker_info = (worker_thread_arg *)arg;
  // Get the PX_worker pointer from THD.
  THD *thd = worker_info->worker_thd;
  thd->px_scan_ctx = worker_info->scan_ctx;
  PX_task *task = thd->px_executor->m_tasks_hash[worker_info->task_id];
  task->run(thd);
  // Detach receiver proc from sender MQ handler.
  worker_info->exchange_info->detach_sender(thd->task_executor_id);
  static_cast<PX_sender *>(task->root_iterator())->end();
  return nullptr;
}

/**
  When common task executing, read data from the child to the MQ,
  or read the data from the MQ to a temporary table.
  Differ from root task running, no need to send result to client,
  either send result set metadata.
*/
bool PX_task::run(THD *thd)
{
  sql_print_information("PX_task::%s:%d task to run in THD[%d]",
    __FUNCTION__, __LINE__, thd->worker_id);
  if (sub_iterator->Init()) return true;

  PFSBatchMode pfs_batch_mode(sub_iterator);

  for (;;) {
    int error = sub_iterator->Read();

    if (DBUG_EVALUATE_IF("test_error_before_sending_data_worker", true, false))
      my_error(ER_DA_OOM, MYF(0));

    if (error > 0 || thd->is_error())
      return true;
    else if (error < 0)
      break;
    else if (thd->killed)
      return true;
  }

  return false;
}

/**
  In last PX_task run as root task, we directly send the data to client,
  before that, result metadata also need to be sent. Lastly send eof as
  the task finish.
  This task is executed in coordinator thread, the original thread which
  receive the original query and start execution.
*/
bool PX_task::run_root(THD *thd)
{
  sql_print_information("PX_task::%s:%d run root task.",
    __FUNCTION__, __LINE__);

  if (DBUG_EVALUATE_IF("test_error_before_sending_data_coor", true, false))
    my_error(ER_DA_OOM, MYF(0));

  // Check other workers' execution, need fallback if error ocurred.
  if (thd->check_px_error()) {
    sql_print_warning("PX_sequential_coordinator::%s:%d error[%d] occurred",
      __FUNCTION__, __LINE__, thd->px_errno);
    thd->need_fallback = true;
    return true;
  }

  Query_expression *unit = thd->lex->unit;
  mem_root_deque<Item *> *fields = unit->get_field_list();
  Query_result *query_result = unit->query_result();

  // As coordinator execute last root task, send result to client.
  if (query_result->start_execution(thd))
    return true;

  // Send result set metadata of each field in fields array.
  if (query_result->send_result_set_metadata(
        thd, *fields, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  thd->get_stmt_da()->reset_diagnostics_area();

  // Before this point, we can fallback before something goes wrong.
  if (sub_iterator->Init()) return true;

  PFSBatchMode pfs_batch_mode(sub_iterator);

  for (;;) {
    int error = sub_iterator->Read();

    if (DBUG_EVALUATE_IF("test_error_in_sending_data", true, false))
      my_error(ER_DA_OOM, MYF(0));

    if (error > 0 || thd->is_error())
      return true;
    else if (error < 0)
      break;
    else if (thd->killed)
    {
      thd->send_kill_message();
      return true;
    }

    if (query_result->send_data(thd, *fields))
      return true;
  }

  return query_result->send_eof(thd);
}

bool PX_coordinator::create_worker_context(worker_pool_t *&worker_pool,
       worker_thread_arg **&th_arg_array)
{
  // TODO: bind the task to CPU core, for more stable exection.
  // Parse and optimize the SQL query, generate the physical tasks for query.
  for (int i = 0; i < worker_pool->num_workers; ++i) {
    THD *worker_new_thd = new THD();
    if (nullptr == worker_new_thd) return true;

    worker_new_thd->m_is_worker = true;
    // copy variables of coordinator THD for worker
    post_init_worker_thd(thd(), worker_new_thd);

    mysql_change_db(worker_new_thd, thd()->db(), false);
    // THD to thd list for state setting.
    thd_list.push_back(worker_new_thd);

    // Set the thread_id of the THD by Global_THD_Manager, in temp table
    // creatation, thread_id is needed to name a temp file in disk.
    worker_new_thd->set_new_thread_id();
    worker_new_thd->worker_id = i;
    worker_new_thd->px_coordinator = thd();
    worker_new_thd->px_exchange_context = worker_pool->px_exchange_context;
    // Set the query to worker THD.
    worker_new_thd->set_query(thd()->query().str, thd()->query().length);
    // In each worker THD, synchronization is needed to keep up with master.
    worker_new_thd->worker_arg = &worker_pool->thread_args[i];
    worker_new_thd->worker_arg->worker_thd = worker_new_thd;
    worker_new_thd->worker_pool = worker_pool;

    th_arg_array[i] = (worker_thread_arg*) malloc(sizeof(worker_thread_arg));
    // Set the arguments of thread.
    th_arg_array[i]->worker_thd = worker_new_thd;

    Query_expression *unit = thd()->lex->unit;
    worker_new_thd->worker_arg->coordinator_root_access_path =
      nullptr != unit ? unit->root_access_path() : nullptr;
    worker_new_thd->worker_arg->coordinator_join =
      unit->is_union() ? nullptr : unit->first_query_block()->join;
  }
  return false;
}

/**
  Run root task at last, send result metadata and data to client, this
  executed in coordinator thread, which get original query from client.
*/
bool PX_coordinator::run_root_dfo_task()
{
  Dfo *root_dfo = dfo_mgr()->root_dfo();
  PX_task *task = new (thd()->mem_root) PX_task(root_dfo->root_iterator());
  if (nullptr == task) return true;
  return task->run_root(thd());;
}

void PX_coordinator::notify_all_workers(THD::killed_state state_to_set)
{
  /*
  if (thd()->px_scan_ctx) {
    PX_reader *px_reader = static_cast<PX_reader*>(thd()->px_scan_ctx);
    px_reader->set_task_finished();
  }
  */
  Prealloced_array<THD *, 60>::iterator iter = thd_list.begin();
  for(; iter != thd_list.end(); ++iter) {
    mysql_mutex_lock(&(*iter)->LOCK_thd_data);
    (*iter)->awake(state_to_set);
    mysql_mutex_unlock(&(*iter)->LOCK_thd_data);
  }
}

/**
  Execute by using PX, schedule all tasks one-by-one in coordinator.

  We create other dop_parallel_degree threads as workers to attach
  with new THD. Original THD is leader which will send worker execution
  info to new threads, and control the synchronization process, workers
  get partial execution info and start their own tasks.

  After generating all partial plans' iterators, execute iterator query
  in all threads, finally coordinator send the last result data.
*/
bool PX_coordinator::schedule(worker_pool_t *worker_pool)
{
  bool ret = false;
  worker_func thread_func;
  int num_workers = worker_pool->num_workers;
  worker_thread_arg **th_arg_array = (worker_thread_arg**)
    malloc(sizeof(worker_thread_arg *) * num_workers);

  // Set the px exchange context for worker pool.
  worker_pool->px_exchange_context = m_thd->px_exchange_context;

  // Create multiple threads and rebuild query execution flow.
  if (create_worker_context(worker_pool, th_arg_array))
    return true;

  thread_func = rebuild_query_execution;
  set_threads_args(worker_pool, thread_func, (void **)th_arg_array, num_workers);
  /**
    Start multiple threads to run a query, a barrier is needed to
    synchronize all workers' parse and optimize phase. Before execute
    each worker's tasks, some checks are needed, like execution plan
    consistency between master and worker.
                        / worker parse | optimize \
                       /                           \
    parse | optimize  --- worker parse | optimize -- dispatch tasks.
                       \                           /
                        \ worker parse | optimize /
  */
  if (query_execute_start(worker_pool)) goto end_workers;
  // Waiting workers done prepare work, adjust semaphore and go on.
  if (query_execute_barrier(worker_pool)) goto end_workers;

  // Back to serial execution if error occurred before sending data.
  if (thd()->check_px_error()) {
    sql_print_warning("PX_sequential_coordinator::%s:%d error[%d] occurred",
      __FUNCTION__, __LINE__, thd()->px_errno);
    thd()->get_stmt_da()->reset_diagnostics_area();
    thd()->need_fallback = true;
    goto end_workers;
  }

  // consistency check between master and workers.
  if (!check_equivalence(worker_pool))
    goto end_workers;

  sql_print_information("PX_sequential_coordinator::%s:%d schedule DFO.",
    __FUNCTION__, __LINE__);
  /**
    Dispatch tasks to all workers, we dispatch all task pairs(tp) to
    all workers by using the tasks which convert from dfos. like
                /tp1\           /tp2\               /tpn\
               /     \         /     \             /     \
    dispatch---  tp1 barrier --  tp2 barrier ...---  tpn barrier 
               \     /         \     /             \     /
                \tp1/           \tp2/               \tpn/

  */
  DEBUG_SYNC_C("execute_in_parallel_before_scheduling");
  // set the thread func before we schedule dfo array.
  thread_func = execute_task_in_worker;
  set_threads_args(worker_pool, thread_func, (void **)th_arg_array, num_workers);

  // Currently we only support one stage scheduling.
  ret = schedule_dfo_pair_inner(worker_pool, th_arg_array);

end_workers:
  // Every worker jump out of loop()
  if (worker_pool_destroy(worker_pool)) goto clean_workers;

  /** 
    Every worker exist from THD thread, after these step, only one
    thread remain to cleanup query execution in coordinator.
        /tpn\          /worker exit\
       /     \        /             \
    ---  tpn barrier--> worker exit--coordinator cleamup-->finish
       \     /        \             /
        \tpn/          \worker exit/
  */
  if (query_execute_start(worker_pool)) goto clean_workers;
  if (query_execute_barrier(worker_pool)) goto clean_workers;

clean_workers:
  sql_print_information("PX_sequential_coordinator::%s:%d Cleanup worker pool.",
    __FUNCTION__, __LINE__);
  worker_pool_cleanup(worker_pool);

  Prealloced_array<THD *, 60>::iterator iter = thd_list.begin();
  for(; iter != thd_list.end(); ++iter)
    delete (*iter);

  return ret;
}

/**
  Check equivalence between coordinator thread and worker thread. we set
  need_fallback and fallback to serial execution if not equal.
*/
bool PX_coordinator::check_equivalence(worker_pool_t *worker_pool)
{
  for (int i = 1; i < worker_pool->num_workers; ++i) {
    if (!worker_pool->thread_args[i].is_equivalent_plan) {
      thd()->need_fallback = true;
      // TODO delete in release version
      sql_print_warning("%d-th/%d thread generated an unequal plan",
                        i, worker_pool->num_workers);
      assert(false);
      return false;
    }
  }
  return true;
}

/**
  Convert from dfo tree to task array by post-order traversal. This
  function will be invoked both in coordinator and workers to get
  same tasks array, schedule them one pair once.
  We get the task tree and task array after this function.
*/
bool PX_worker::prepare_task_for_dfo()
{
  for (uint i = 0; i < m_dfo_mgr->m_normalized_dfo_tree.size(); ++i) {
    Dfo *dfo = m_dfo_mgr->m_normalized_dfo_tree.at(i);
    PX_task *task = new (m_thd->mem_root) PX_task(dfo->root_iterator());
    if (nullptr == task) return true;
    m_tasks_hash.insert(std::pair<int64_t, PX_task*>(dfo->dfo_id(), task));
    sql_print_information("PX_worker::%s:%d prepare task[%d] of iterator[%s] r[%d]"
      "parent dfo [%d]", __FUNCTION__, __LINE__, dfo->dfo_id(),
      dfo->root_iterator()->str().c_str(), dfo->is_root_dfo(),
      dfo->is_root_dfo() ? -1 : dfo->parent_dfo()->dfo_id());
  }
  return false;
}

/**
  Worker loop is in rebuild_query_execution, it receive all tasks'
  instructions which received from coordinator. Every task start with
  `sem_wait` sem_inner semaphore, run the rask and arrive the barrier
  then start the next one.
  After executing all received tasks, coordinator set finish, destroy
  semaphore before exit the function.
*/
void PX_worker::loop()
{
  worker_func thread_func;
  void *thread_func_arg = nullptr;
  worker_thread_arg *thread_arg = thd()->worker_arg;

  while (true) {
    sem_wait (&thread_arg->sem_inner);
    if (*thread_arg->finished)
      break;

    thread_func = *(thread_arg->thread_func);
    thread_func_arg = *(thread_arg->thread_func_arg);

    // All tasks' handled here.
    (*thread_func)(thread_func_arg);

    // Lock here.
    task_finish_signal(thread_arg);
  }
  sem_destroy (&thread_arg->sem_inner);
}

/**
  In the parallel scheduler, the DFO tree is traversed by post-order,
  two DFOs are executed each time, they are converted into task pairs,
  and then executed. After the execution, the child task is set to be
  finished, and loop continues until the root task is executed.

  If DFO tree has only two DFOs. Start scheduling child dfo and then
  root dfo, when finished, send result to client.
  As shown as below, <task1, root task> is scheduled in whole process.

    root dfo           root task              schedule:
      |                    |                  <task1, root task>
      dfo1                task1

  If DFO tree has more than two DFOs. We schedule whole DFO tree by
  post-order traversal.
  As shown as below, <task1, task3> is scheduled firstly, since task3
  has two children, and then <task2, task3> is scheduled.

        root dfo           root task              schedule:
          |                    |                  <task1, task3>
          dfo5                task5                <task2, task3>
        /    \     ---->     /   \       --->     <task3, task5>
        dfo3  dfo4         task3  task4            <task4, task5>
      /   \                /  \                   <task5, root task>
  dfo1     dfo2         task1 task2

  Every schedule pair is one dfo and its parent dfo. child execute and
  send data to MQ, and parent receive data from MQ and saved in temp
  table or send to client.

  Steps: like // dfo array: 1 2 3 4 5 root
      while get ready dfos: (check switch to child from parent)
        init the exchange for <dfo_child, dfo_parent>
        schedule dfo_child to threads
        schedule dfo_parent to threads
        wait for dfo_child to finish and synchronize
        next loop
*/
bool PX_parallel_coordinator::schedule_dfo_pair_inner(worker_pool_t *worker_pool,
      worker_thread_arg **th_arg_array)
{
  bool ret = false;
  int num_workers = worker_pool->num_workers;
  Worker_exec_ctx last_parent_desc(nullptr, num_workers);

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
      Worker_exec_ctx child_desc(child, num_workers);
      Worker_exec_ctx parent_desc(parent, num_workers);
      // Get exchange info from exchange info manager instance.
      exchange_info = thd()->px_exchange_context->get(child->dfo_id());

      if (thd()->killed) return true; // return if query be killed.
      if (thd()->need_fallback) return false; // Fallback to serial execution.
      sql_print_information("PX_parallel_coordinator::%s:%d Child dfo[%d], "
        "parent dfo[%d], is root[%d]", __FUNCTION__, __LINE__,
        child->dfo_id(), parent->dfo_id(), parent->is_root_dfo());

      // Attach exchange info for child, parent or last parent dfo.
      attach_exchange_info(num_workers, th_arg_array, &last_parent_desc,
                           &child_desc, &parent_desc);

      DEBUG_SYNC_C("execute_in_parallel_scheduling");

      // Prepare for parent dfo execution threads.
      if (parent->is_dfo_active() || parent->is_root_dfo()) {
        /* Do nothing when parent dfo not finish.*/
      } else {
        // Start to schedule parent dfo, allocate the threads' array bitmap,
        // set the threads' bitmap, get the group id.
        if (allocate_threads(worker_pool, &parent_desc)) return true;
        // Set the arguments of parent tasks, set the dfo active at last.
        prepare_schedule_single_dfo(num_workers, th_arg_array, &parent_desc);
        // Enter into inner worker's loop() to run task of one stage,
        // we use one-by-one task pair scheduling strategy for better control
        // of CPU cores.
        if (task_execute_start(worker_pool, &parent_desc)) return true;
        parent->set_dfo_active();
      }
      if (!exchange_info->is_top_exchange()) exchange_info->schedule_wait();
      if (thd()->killed) return true; // be killed.

      // Prepare for child dfo execution threads.
      if (!child->is_dfo_active()) {
        // Start to schedule child dfo, allocate the threads' array bitmap,
        // set the threads' bitmap, get the group id.
        if (allocate_threads(worker_pool, &child_desc)) return true;
        // Set the arguments of child tasks, set the dfo active at last.
        prepare_schedule_single_dfo(num_workers, th_arg_array, &child_desc);
        // Post for start of the tasks on every workers.
        if (task_execute_start(worker_pool, &child_desc)) return true;
      }
      if (exchange_info->is_top_exchange()) exchange_info->schedule_wait();

      sql_print_information("PX_parallel_coordinator::%s:%d Child map[%d], "
        "Parent map[%d]", __FUNCTION__, __LINE__,
        *child_desc.bitmap.bitmap, *parent_desc.bitmap.bitmap);

      DEBUG_SYNC_C("execute_in_parallel_scheduling_before_root");
      if (parent->is_root_dfo()) run_root_dfo_task();

      if (task_execute_barrier(worker_pool, &child_desc)) return true;

      if (thd()->need_fallback) return false; // Fallback to serial execution.
      if (thd()->killed) return true; // been killed.
      if (!thd()->need_fallback && thd()->check_px_error()) {
        // Throw error 'ER_PX_EXECUTE_ERROR' when sending data.
        thd()->get_stmt_da()->reset_diagnostics_area();
        my_error(ER_PX_EXECUTE_ERROR, MYF(0), thd()->px_errno);
        return true;
      }

      DEBUG_SYNC_C("execute_in_parallel_scheduling_end");
      // TODO: add clean of exchange info. exchange_info->clean();
      if (exchange_info) exchange_info->release_in_stage_over();

      // Set the child dfo finished, currently only one stage supported.
      child->set_dfo_finished(true);
      last_parent_desc.restore_parent_exec_ctx(parent_desc);
    }
  }

  return ret;
}

/**
  Exchange info should be attached to every tasks when schedule the dfo pair.
  1. Child dfo must be finished when schedule one dfo pair.
  2. Parent dfo may be not finished when schedule one dfo pair, change parent's
     exchange info when schedule next dfo pair.
  3. Parent dfo may switch to child dfo, change the exchange info.
*/
void PX_parallel_coordinator::attach_exchange_info(int num_workers,
       worker_thread_arg **args, Worker_exec_ctx *last_parent,
       Worker_exec_ctx *child, Worker_exec_ctx *parent)
{
  // Set the parent dfo's new exchange info.
  if (parent->dfo()->is_dfo_active() || parent->dfo()->is_root_dfo())
    for (int i = 0; i < num_workers; ++i)
      if (bitmap_is_set(&parent->bitmap, i))
        args[i]->exchange_info = exchange_info;
  // Set the dfo's new exchange info when switch last parent to child.
  if (child->dfo() == last_parent->dfo()) {
    for (int i = 0; i < num_workers; ++i)
      if (bitmap_is_set(&last_parent->bitmap, i))
        args[i]->exchange_info = exchange_info;
  } else if (last_parent->dfo() && last_parent->dfo()->is_dfo_active()) {
    bitmap_copy(&parent->bitmap, &last_parent->bitmap);
  } else { /*do nothing.*/ }
}

void PX_parallel_coordinator::prepare_schedule_single_dfo(int num_workers,
       worker_thread_arg **args, Worker_exec_ctx *exec_ctx)
{
  Dfo *dfo = exec_ctx->dfo();
  // Set the arguments of tasks for the threads in exec_ctx.
  for (int i = 0, task_executor_id = 0; i < num_workers; ++i) {
    if (bitmap_is_set(&exec_ctx->bitmap, i)) {
      args[i]->task_id = dfo->dfo_id();
      args[i]->exchange_info = exchange_info;
      args[i]->worker_thd->task_executor_id = task_executor_id++;
      args[i]->worker_thd->thread_group_id = exec_ctx->group_id();
      if (dfo->is_leaf_dfo()) // set px scan ctx for leaf dfo.
        args[i]->scan_ctx = static_cast<PX_reader*>(dfo->m_px_scan_ctx);
    }
  }
}

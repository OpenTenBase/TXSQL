#include "px_executor.h"

#include "scope_guard.h"
#include "sql/sql_lex.h"  // LEX
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_tmp_table.h"  // create_tmp_table
#include "sql/sql_executor.h"  // QEP_TAB
#include "sql/sql_parse.h"  // mysql_reset_thd_for_next_command
#include "sql/sql_db.h"  // mysql_change_db
#include "sql/iterators/row_iterator.h"  // RowIterator
#include "sql/opt_trace.h" // Opt_trace_
#include "sql/protocol.h" // Protocol
#include "sql/pfs_batch_mode.h"  // PFSBatchMode
#include "sql/log.h" // For debug to be deleted.
#include "sql/iterators/basic_row_iterators.h"  // TableScanIterator
#include "sql/parallel_execution/px.h"
#if defined(HAVE_OPT_CTX)
#include "sql/parallel_execution/opt_interface.h"  // OPT_CTX
#endif
#include "sql/parallel_execution/px_interface.h"  // PX_ROLE_COORDINATOR
#include "sql/join_optimizer/access_path.h" // Access_path
#include "sql/join_optimizer/explain_access_path.h" // CheckPlanEquivalence
#include "sql/parallel_execution/px_resource_mgr.h" // PX_resource_manager
#include "sql/parallel_execution/px_plan_slice.h" // PX_plan_slice
#include "sql/parallel_execution/px_explain.h"  // WalkAccessPathsForExplain
#include "sql/mysqld_thd_manager.h"
#include "sql/psi_memory_key.h"

PSI_stage_info stage_running_task = {0, "Task runing", 0, PSI_DOCUMENT_ME};

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

  Parser_state *parser_state = new (thd->mem_root) Parser_state();
  if (nullptr == parser_state) return nullptr;
  if (parser_state->init(thd, thd->query().str, thd->query().length))
    return nullptr;

  lex_start(thd);
  thd->m_parser_state = nullptr;
  // handle found_semicolon situation forbidden.
  bool err = thd->get_stmt_da()->is_error();

  DBUG_EXECUTE_IF("enter_worker_sleep", sleep(2););

  if (!err) err = parse_sql(thd, parser_state, nullptr);
  if (err) {
    PX_PRINT_ERROR(
        "parse_sql() got error %d on worker %d", err, thd->worker_id);
  }

  if (!err && !thd->is_error()) error = mysql_execute_command(thd, true);
  PX_PRINT_ERROR("mysql_execute_command() got error %d state %d",
      error, thd->px_worker_state);

  if (error && thd->px_worker_state <= PX_WORKER_PARSE_OPTIMIZE)
    optimize_finsh_signal(thd->worker_arg, true);

  // LEX cleanup for the local thread execution.
  thd->lex->destroy();
  thd->end_statement();
  thd->cleanup_after_query();

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
  PX_task *task = PX_EXECUTOR(thd)->m_tasks_hash[worker_info->task_id];
  PX_PRINT_INFO("worker %d run task %d", thd->worker_id, worker_info->task_id);
  task->run(thd);
  return nullptr;
}

static bool check_plan_equivalence(THD *thd, AccessPath *plan, JOIN *join);

#ifndef DBUG_OFF
static void debug_print_iterator(const char *prefix, RowIterator *iterator,
                                 Dfo_mgr *dfo_mgr = nullptr, int indent = 0);
static void debug_print_dfo(const char *prefix, Dfo *dfo, int indent);
#endif

/**
  Init execution environment.

  Note that exchange info and resources are supposed to be determined in previous
  cost-based parallel optimization phase. Cost-based optimization is future work,
  so they are part of this function for now.
 */
bool px_execute_init(THD *thd, RowIterator *root_iterator, AccessPath *root_path,
                     JOIN *root_join, int64_t &requested_cores) {
  Dfo_mgr *dfo_mgr = nullptr;
  PX_executor *executor = nullptr;

  assert(thd->use_px);

  // Create the interface object to the parallel world.
  if (PX_ROLE_COORDINATOR(thd)) {
    executor = new (thd->mem_root) PX_parallel_coordinator(thd);
  } else {
    executor = new (thd->mem_root) PX_worker(thd);
  }
  if (!executor) goto create_executor_failed;
  PX_EXECUTOR(thd) = executor;

  /*
    Create the context for exchange instances which are shared among all
    parallel threads.
   */
  if (PX_ROLE_COORDINATOR(thd)) {
    thd->px_exchange_context = new (thd->mem_root) PX_exchange_context();
    if (!thd->px_exchange_context) goto create_exchange_context_failed;
  } else {
    thd->px_exchange_context =
        PX_EXECUTOR(thd)->coordinator()->thd()->px_exchange_context;
    assert(thd->px_exchange_context);
  }

  /*
    Verify for a worker that its plan is same to that of the coordinator,
    which is assumed by all successive operations.
   */
  if (PX_ROLE_WORKER(thd)) {
    PX_PRINT_INFO("worker %d start", thd->worker_id);
    if (!check_plan_equivalence(thd, root_path, root_join)) {
      PX_PRINT_ERROR("worker %d check plan found different plan",
                     thd->worker_id);
      my_error(ER_CDB_OPTIMIZATION_CONTEXT_INCONSISTENT, MYF(0),
          (thd)->thread_id(), "unequal plan");
      goto check_eq_failed;
    }
    PX_PRINT_INFO("worker %d check plan OK", thd->worker_id);
  }

  /*
    Determine plan fragments and their parallelism.

    Note that the dfo tee is private to each thread, analyzing is done on the
    coordinator and the result is inherited by all workers through the shared
    exchange instances.
   */
#ifndef DBUG_OFF
  if (PX_ROLE_COORDINATOR(thd)) {
    debug_print_iterator("iterator(raw): ", root_iterator);
  }
#endif
  dfo_mgr = executor->dfo_mgr();
  if (dfo_mgr->do_split(nullptr, root_iterator, dfo_mgr->m_root_dfo) ||
      dfo_mgr->analyze_resource_allocation(&requested_cores) ) {
    goto dfo_prepared_failed;
  }
#ifndef DBUG_OFF
  if (PX_ROLE_COORDINATOR(thd)) {
    debug_print_iterator("iterator(analyzed): ", root_iterator, dfo_mgr);
  }
#endif

  return false;

 dfo_prepared_failed:
  if (PX_ROLE_COORDINATOR(thd)) {
    destroy(thd->px_exchange_context);
    if (thd->lex->is_explain()) destroy(executor);
  }

 check_eq_failed: // only in workers.
  return true;

 create_exchange_context_failed:
  destroy(executor);

 create_executor_failed:
  my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);

  return true;
}

bool px_explain_init(THD *thd, AccessPath *root_path, JOIN *root_join) {
  PX_plan_slice *slice = nullptr;
  uint exchange_count = 0;

  assert(thd->use_px);

  // Traverse accesspath-tree to set exchange info to Exchange AccessPath
  // TODO move to px_optimizer() and make DFO by slice.
  if (!thd->lex->explain_format->is_tree()) {
    slice = new (thd->mem_root) PX_plan_slice();
    if (!slice) goto create_plan_slice_failed;
  }
  if (WalkAccessPathsForExplain(thd, root_path, thd->px_exchange_context,
      exchange_count, root_join, slice)) goto walk_path_failed;
  assert(exchange_count ==
         PX_EXECUTOR(thd)->dfo_mgr()->m_normalized_dfo_tree.size());
  if (slice && slice->tables() && root_join) {
    root_join->px_plan_slices.emplace_back(slice);
  } else {
    /*
      append is not responding to any qeury block and it won't be
      explained in traditional format.
    */
    if (slice) destroy(slice);
  }

  return false;

walk_path_failed:
  destroy(slice);

create_plan_slice_failed:
  return true;
}

bool px_execute_in_coordinator(THD *thd, int64_t requested_cores) {
  /*
    Add join_execution step to the trace. (#974)

    Note that it is a step in the execution phase, which means certain contents,
    e.g. filesort information, might be missing from the final trace result
    because they are on workers.

    The select number is ommited purposely, because it does not deserve changing
    the interface function to pass it as an argument.

    Tracing workers is to be supported in the future.
   */
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_exec(trace, "join_execution");
  Opt_trace_array trace_steps(trace, "steps");

  bool res = false;
  PX_coordinator *coordinator = down_cast<PX_coordinator *>(PX_EXECUTOR(thd));
  Dfo_mgr *dfo_mgr = coordinator->dfo_mgr();
  assert(PX_ROLE_COORDINATOR(thd) && coordinator && requested_cores > 0);
  worker_pool_t *worker_pool = nullptr;

  // Create exchange channels.
  if (thd->px_exchange_context->init()) {
    goto err;
  }

  if (!PX_resource_manager::get_instance()->acquire(requested_cores)) {
    PX_PRINT_WARN("failed to acquire %ld cores", requested_cores);
    my_error(ER_PX_OUT_OF_THREADS, MYF(0), (int)requested_cores);
    thd->need_fallback = true;
    goto lack;
  }

  PX_PRINT_INFO("acquired %ld cores", requested_cores);

  DEBUG_SYNC_C("execute_in_parallel_before_kill");
  if (DBUG_EVALUATE_IF("execute_in_parallel_before_error", true, false))
    my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
  if (DBUG_EVALUATE_IF("execute_in_parallel_before_fallback", true, false)) {
    my_error(ER_PX_OUT_OF_THREADS, MYF(0), (int)requested_cores);
    thd->need_fallback = true;
  }
  DBUG_EXECUTE_IF("execute_in_parallel_before_sleep", sleep(2););

#ifndef DBUG_OFF
  debug_print_dfo("dfo tree: ", dfo_mgr->root_dfo(), 0);
#endif

  mysql_mutex_lock(&thd->LOCK_thd_data);
  for (int i = 0; i < requested_cores; ++i) {
    THD* worker_thd = new THD();

#if defined(HAVE_OPT_CTX)
    assert(OPT_CTX_ENABLED(thd));
    OPT_CTX(worker_thd).set_ctx(OPT_CTX_REPLAY, OPT_CTX(thd).opt_ctx());
    OPT_CTX(worker_thd).init_thd();
#endif

    worker_thd->px_coordinator = thd;
    worker_thd->set_is_killable(true);
    worker_thd->m_is_worker = true;
    coordinator->thd_list.push_back(worker_thd);
  }
  mysql_mutex_unlock(&thd->LOCK_thd_data);

  if (thd->is_error() || thd->killed) goto err_finish;

  worker_pool = create_worker_threads(requested_cores, coordinator->thd_list);
  if (!worker_pool) {
    my_error(ER_PX_CREATE_THREAD_ERROR, MYF(0));
    thd->need_fallback = true;
    PX_PRINT_ERROR("unable to create threads");
    goto err_finish;
  }
  thd->worker_pool = worker_pool;

  PX_PRINT_INFO("start scheduling");
  if (coordinator->schedule(worker_pool)) res = true;

  release_worker_threads(worker_pool);

  if (!res) {
    mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_executed);
    txsql_parallel_stmt_executed++;
    mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_executed);
  }

  goto finish;

lack:
  thd->px_exchange_context->clean();

err:
  destroy(thd->px_exchange_context);
  destroy(thd->px_executor);
  return true;

err_finish:
  if (thd->killed) {
    mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_error);
    txsql_parallel_stmt_error++;
    mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_error);
  }
  res = true;

finish:
  PX_resource_manager::get_instance()->release(requested_cores);
  thd->px_exchange_context->clean();
  destroy(thd->px_exchange_context);
  PX_PRINT_INFO("release %ld cores", requested_cores);

  DBUG_EXECUTE_IF("px_simulate_worker_timeout_kill", {
    sleep(5); // Pause coordinator long to get timeout KILL.
    if (thd->killed) res = true;
  });

  mysql_mutex_lock(&thd->LOCK_thd_data);
  for (auto &itr : coordinator->thd_list) {
    if (itr->px_executor) destroy(itr->px_executor);
    delete (itr);
  }
  coordinator->thd_list.clear();
  mysql_mutex_unlock(&thd->LOCK_thd_data);

  destroy(thd->px_executor);
  return res;
}

bool px_execute_in_worker(THD *thd) {
  PX_worker *worker = down_cast<PX_worker *>(PX_EXECUTOR(thd));
  assert(PX_ROLE_WORKER(thd) && worker);

  /*
    Set up so that the worker understands task commands from the coordinator.
    Imagine the command arguements are correspondent on each parallel thread.
   */
  if (worker->prepare_task_for_dfo()) goto err;

  // Wait other workers finish their parse and optimize work.
  if (optimize_finsh_signal(thd->worker_arg)) goto err;

  // Start the task command loop.
  thd->px_worker_state = PX_WORKER_EXECUTE;

  if (DBUG_EVALUATE_IF("schedule_before_schedule_error", true, false))
    my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);

  if (thd->killed || thd->is_error()) goto err;

  worker->loop();
  PX_PRINT_INFO("worker %d leave task loop", thd->worker_id);

  return thd->is_error();

err:
  return true;
}

/**
  Check plan equivalence on worker. The result is saved in the shared space.

  @param thd  worker thd
  @param plan worker plan
  @param join optimizer state for plan root

  @return true if equal, false otherwise
 */
static bool check_plan_equivalence(THD *thd, AccessPath *plan, JOIN *join) {
  int base_level = 0;
  thd->m_equivalence_check_phase = true;
  assert(PX_ROLE_WORKER(thd));

  thd->worker_arg->is_equivalent_plan =
      CheckPlanEquivalence(base_level,
                           thd->worker_arg->coordinator_root_access_path,
                           thd->worker_arg->coordinator_join,
                           plan,
                           join,
                           /*is_root_of_join=*/!join,
                           /*sub_tree_of_exchange=*/false);
  thd->m_equivalence_check_phase = false;

  return thd->worker_arg->is_equivalent_plan;
}

#ifndef DBUG_OFF
static void debug_print_iterator(const char *prefix, RowIterator *iterator,
                                 Dfo_mgr *dfo_mgr, int indent) {
  String space;
  for (int i = 0; i < indent; i++) {
    space.append(' ');
  }

  Dfo *dfo = nullptr;
  if (dfo_mgr) {
    for (auto &it : dfo_mgr->m_normalized_dfo_tree) {
      if (iterator == it->root_iterator()) {
        dfo = it;
        break;
      }
    }
    if (iterator == dfo_mgr->root_dfo()->root_iterator()) {
      dfo = dfo_mgr->root_dfo();
    }
  }
  if (dfo) {
    TableRowIterator *scan = analyze_parallel_table(dfo);
    if (scan) {
      PX_PRINT_INFO(
          "%s%s%s (dfo %ld dop %ld pscan)", prefix, space.c_ptr_safe(),
          iterator->str().c_str(), dfo->dfo_id(), dfo->dop());
    } else {
      PX_PRINT_INFO(
          "%s%s%s (dfo %ld dop %ld)", prefix, space.c_ptr_safe(),
          iterator->str().c_str(), dfo->dfo_id(), dfo->dop());
    }
  } else {
    PX_PRINT_INFO("%s%s%s", prefix, space.c_ptr_safe(),
        iterator->str().c_str());
  }

  for (unsigned i = 0; i < iterator->m_children.size(); ++i)
    debug_print_iterator(prefix, iterator->m_children[i], dfo_mgr, indent + 2);
}

static void debug_print_dfo(const char *prefix, Dfo *dfo, int indent) {
  String space;
  for (int i = 0; i < indent; i++) {
    space.append(' ');
  }
  PX_PRINT_INFO("%s%s%s (dop %ld)", prefix, space.c_ptr_safe(),
                dfo->root_iterator()->str().c_str(), dfo->dop());
  for (unsigned i = 0; i < dfo->m_child_dfos.size(); ++i)
    debug_print_dfo(prefix, dfo->m_child_dfos.at(i), indent + 2);
}
#endif

/**
  When common task executing, read data from the child to the MQ,
  or read the data from the MQ to a temporary table.
  Differ from root task running, no need to send result to client,
  either send result set metadata.
*/
bool px_run_task(THD *thd, RowIterator *sub_iterator) {
  auto exchange_detach = create_scope_guard([thd] {
    /*
      Explicitly call the End function to release resources
      and complete detach
    */
    if (thd->px_sender) {
      thd->px_sender->End();
      thd->px_sender = nullptr;
    }
    if (thd->px_receiver) {
      thd->px_receiver->End();
      thd->px_receiver = nullptr;
    }
  });

  if (sub_iterator->Init()) return true;

  DBUG_EXECUTE_IF("enter_worker_sleep", sleep(10););

  Query_expression *unit = thd->lex->unit;
  auto reset_join_counter = create_scope_guard([thd, unit] {
    for (Query_block *sl = unit->first_query_block(); sl; sl = sl->next_query_block()) {
      JOIN *join = sl->join;
      thd->inc_examined_row_count(join->examined_rows);
      // THD may be reused, clear examined_rows in case of double counting.
      join->examined_rows = 0;
    }
    if (unit->fake_query_block != nullptr) {
      thd->inc_examined_row_count(unit->fake_query_block->join->examined_rows);
      // THD may be reused, clear examined_rows in case of double counting.
      unit->fake_query_block->join->examined_rows = 0;
    }
  });

  PFSBatchMode pfs_batch_mode(sub_iterator);

  for (;;) {
    int error = sub_iterator->Read();

    if (DBUG_EVALUATE_IF("schedule_one_loop_error_worker", true, false))
      my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);

    if (error > 0 || thd->is_error())
      return true;
    else if (error < 0)
      break;
    else if (thd->killed) {
      return true;
    }
  }

  return false;
}

void PX_task::run(THD *thd) {
  px_run_task(thd, sub_iterator);
}

/**
  In last PX_task run as root task, we directly send the data to client,
  before that, result metadata also need to be sent. Lastly send eof as
  the task finish.
  This task is executed in coordinator thread, the original thread which
  receive the original query and start execution.
*/
bool px_run_root(THD *thd, RowIterator *sub_iterator) {
  PX_PRINT_INFO("run root task");

  if (DBUG_EVALUATE_IF("run_root_before_sending_data_error", true, false))
    my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);

  // Check other workers' execution, need fallback if error ocurred.
  if (thd->check_px_error()) {
    PX_PRINT_ERROR("detected error %d", thd->px_errno);
    if (!thd->is_error()) // if coordinator no error.
      thd->collect_px_stmt_da_for_error();
    thd->need_fallback = true;
    return true;
  }

  Query_expression *unit = thd->lex->unit;
  // see send_records_ptr in SELECT_LEX_UNIT::ExecuteIteratorQuery()
  ha_rows *send_records_ptr;
  if (unit->fake_query_block != nullptr) {
    send_records_ptr = &unit->fake_query_block->join->send_records;
  } else if (unit->is_simple()) {
    send_records_ptr = &unit->first_query_block()->join->send_records;
  } else {
    send_records_ptr = &unit->send_records;
  }
  *send_records_ptr = 0;

  mem_root_deque<Item *> *fields = unit->get_field_list();
  Query_result *query_result = unit->query_result();

  // As coordinator execute last root task, send result to client.
  if (query_result->start_execution(thd))
    return true;

  // Send result set metadata of each field in fields array.
  if (query_result->send_result_set_metadata(
        thd, *fields, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  auto exchange_detach = create_scope_guard([thd] {
    assert(!thd->px_sender);
    /*
      Explicitly call the End function to release resources
      and complete detach
    */
    if (thd->px_receiver) {
      thd->px_receiver->End();
      thd->px_receiver = nullptr;
    }
  });

  // Before this point, we can fallback before something goes wrong.
  if (sub_iterator->Init()) return true;

  {
    auto join_cleanup = create_scope_guard([thd, unit] {
      for (Query_block *sl =unit->first_query_block(); sl; sl = sl->next_query_block()) {
        JOIN *join = sl->join;
        thd->inc_examined_row_count(join->examined_rows);
      }
      if (unit->fake_query_block != nullptr) {
        thd->inc_examined_row_count(unit->fake_query_block->join->examined_rows);
      }
      if (PX_ROLE_COORDINATOR(thd)) {
        assert(thd->worker_pool);
        for (int i = 0; i < thd->worker_pool->num_workers; ++i) {
          THD *worker_thd = thd->worker_pool->thread_args[i].worker_thd;
          thd->inc_examined_row_count(worker_thd->get_examined_row_count());
        }
      }
    });

    PFSBatchMode pfs_batch_mode(sub_iterator);

    for (;;) {
      int error = sub_iterator->Read();

      DEBUG_SYNC_C("run_root_sending_data_kill");
      if (DBUG_EVALUATE_IF("run_root_in_sending_data_error", true, false))
        my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
      DBUG_EXECUTE_IF("run_root_in_sending_data_sleep", sleep(2););

      if (error > 0 || thd->is_error())
        return true;
      else if (error < 0)
        break;
      else if (thd->killed)
      {
        thd->send_kill_message();
        return true;
      }

      ++*send_records_ptr;

      if (query_result->send_data(thd, *fields))
        return true;
    }

    // NOTE: join_cleanup must be done before we send EOF, so that we get the
    // row counts right.
  }

  thd->current_found_rows = *send_records_ptr;

  thd->collect_px_stmt_da_for_warnings();

  DBUG_EXECUTE_IF("px_simulate_worker_timeout_kill", {
    // Pass over to the other px_simulate_worker_timeout_kill.
    return false;
  });

  if (thd->check_px_error() && !thd->is_error()) // if coordinator no error.
    thd->collect_px_stmt_da_for_error();

  return query_result->send_eof(thd);
}

bool PX_task::run_root(THD *thd) {
  return px_run_root(thd, sub_iterator);
}

bool PX_coordinator::create_worker_context(worker_pool_t *&worker_pool,
       worker_thread_arg **&th_arg_array)
{
  // TODO: bind the task to CPU core, for more stable exection.
  // Parse and optimize the SQL query, generate the physical tasks for query.
  int i = 0;
  for (; i < worker_pool->num_workers; ++i) {
    THD *worker_new_thd = worker_pool->thread_args[i].worker_thd;
    assert(worker_new_thd);

    worker_new_thd->set_db(thd()->db());
    worker_new_thd->worker_id = i;
    worker_new_thd->px_exchange_context = worker_pool->px_exchange_context;
    // Set the query to worker THD.
    worker_new_thd->set_query(thd()->query().str, thd()->query().length);
    // In each worker THD, synchronization is needed to keep up with master.
    worker_new_thd->worker_arg = &worker_pool->thread_args[i];
    worker_new_thd->worker_arg->worker_thd = worker_new_thd;
    worker_new_thd->worker_pool = worker_pool;

    th_arg_array[i] = (worker_thread_arg*) malloc(sizeof(worker_thread_arg));
    if (nullptr == th_arg_array[i])
      goto fail_of_malloc;

    // Set the arguments of thread.
    th_arg_array[i]->worker_thd = worker_new_thd;

    Query_expression *unit = thd()->lex->unit;
    worker_new_thd->worker_arg->coordinator_root_access_path =
      nullptr != unit ? unit->root_access_path() : nullptr;
    worker_new_thd->worker_arg->coordinator_join =
      unit->is_union() ? nullptr : unit->first_query_block()->join;
  }

  return false;

 fail_of_malloc:
  --i;
  while (i >= 0) {
    free(th_arg_array[i]);
    --i;
  }
  return true;
}

/**
  Run root task at last, send result metadata and data to client, this
  executed in coordinator thread, which get original query from client.
*/
bool PX_coordinator::run_root_dfo_task()
{
  bool ret = false;
  Dfo *root_dfo = dfo_mgr()->root_dfo();
  PX_task *task = new (thd()->mem_root) PX_task(root_dfo->root_iterator());
  if (nullptr == task) {
    my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
    return true;
  }
  ret = task->run_root(thd());
  destroy(task); // destruct the root task in coordinator.
  return ret;
}

void PX_coordinator::notify_all_workers(THD::killed_state state_to_set)
{
  mysql_mutex_assert_owner(&thd()->LOCK_thd_data);
  for (auto &worker_thd : thd_list) {
    mysql_mutex_lock(&worker_thd->LOCK_thd_data);
    assert(worker_thd->m_is_worker);
    worker_thd->awake(state_to_set);
    mysql_mutex_unlock(&worker_thd->LOCK_thd_data);
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

  PX_PRINT_INFO("px query start...");
  int num_workers = worker_pool->num_workers;
  worker_thread_arg **th_arg_array = (worker_thread_arg**)
    malloc(sizeof(worker_thread_arg *) * num_workers);
  if ((ret = (nullptr == th_arg_array))) goto oom;

  worker_pool->px_exchange_context = m_thd->px_exchange_context;
  if ((ret = create_worker_context(worker_pool, th_arg_array)))
    goto create_context_failed;

  /**
    Create multiple threads to run a query, a barrier is needed to
    synchronize all workers' parse and optimize phase. Before executing
    each worker's tasks, some checks are needed, like execution plan
    consistency between master and worker.
                        / worker parse | optimize \
                       /                           \
    parse | optimize  --- worker parse | optimize -- dispatch tasks.
                       \                           /
                        \ worker parse | optimize /
  */
  DEBUG_SYNC_C("schedule_before_kill");
  // error may be occurred here, coordinator error, worker right.
  if (DBUG_EVALUATE_IF("schedule_before_fallback", true, false))
    my_error(ER_PX_OUT_OF_THREADS, MYF(0), 0);
  DBUG_EXECUTE_IF("schedule_before_sleep", sleep(2););
  if (thd()->killed) goto clean_workers;

  set_threads_args(worker_pool, rebuild_query_execution, (void **)th_arg_array);

  if (begin_query(worker_pool)) goto clean_workers;
  // Waiting workers done prepare work, adjust semaphore and go on.
  if (wait_workers_generate_plan(worker_pool, thd())) goto clean_workers;

  DEBUG_SYNC_C("schedule_before_schedule_kill");
  DBUG_EXECUTE_IF("schedule_before_schedule_sleep", sleep(2););
  if (thd()->killed) goto unlock_clean_workers;
  if (thd()->check_px_error()) {
    if (!thd()->is_error()) // if coordinator no error.
      thd()->collect_px_stmt_da_for_error();
    goto fallback_unlock_clean_workers;
  }
  if (!check_equivalence(worker_pool)) goto fallback_unlock_clean_workers;

  PX_PRINT_INFO("prepare for run task command");
  /**
    Dispatch tasks to all workers, we dispatch all task pairs(tp) to
    all workers by using the tasks which convert from dfos. like
                /tp1\           /tp2\               /tpn\
               /     \         /     \             /     \
    dispatch---  tp1 barrier --  tp2 barrier ...---  tpn barrier 
               \     /         \     /             \     /
                \tp1/           \tp2/               \tpn/

  */
  set_threads_args(worker_pool, execute_task_in_worker, (void **)th_arg_array);

  if (schedule_dfo_pair_inner(worker_pool, th_arg_array)) ret = true;

  DEBUG_SYNC_C("schedule_after_schedule_kill");
  DBUG_EXECUTE_IF("schedule_after_schedule_sleep", sleep(2););

  goto unlock_clean_workers;

 fallback_unlock_clean_workers:
  thd()->need_fallback = true;

 unlock_clean_workers:
  schedule_over(worker_pool, thd()); // Every worker jump out of loop()

 clean_workers:
  PX_PRINT_INFO("clean up worker pool");

  schedule_end(worker_pool);

  if (thd()->need_fallback) ret = true;

  for(int i = 0; i < num_workers; ++i)
    free (th_arg_array[i]);

 create_context_failed:
  free (th_arg_array);

  PX_PRINT_INFO("px query finish...");

 oom:
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
      PX_PRINT_ERROR("worker %d of %d got an unequal plan",
                     i, worker_pool->num_workers);
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
  for (uint i = 0; i < m_dfo_mgr.m_normalized_dfo_tree.size(); ++i) {
    Dfo *dfo = m_dfo_mgr.m_normalized_dfo_tree.at(i);
    PX_task *task = new (m_thd->mem_root) PX_task(dfo->root_iterator());
    if (nullptr == task) {
      my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
      return true;
    }
    m_tasks_hash.insert(std::pair<int64_t, PX_task*>(dfo->dfo_id(), task));
  }
  return false;
}

/**
  Worker loop is in rebuild_query_execution, it receive all tasks'
  instructions which received from coordinator. Every task start with
  `sem_wait` sem_worker_signal semaphore, run the rask and arrive the barrier
  then start the next one.
  After executing all received tasks, coordinator set finish, destroy
  semaphore before exit the function.
*/
void PX_worker::loop()
{
  worker_func thread_func;
  void *thread_func_arg = nullptr;
  worker_thread_arg *thread_arg = thd()->worker_arg;

  PX_PRINT_INFO("worker %d enter task loop", thd()->worker_id);
  while (true) {
    wait_for_begin_task (thread_arg);
    if (thread_arg->worker_thd->killed)
      break;

    if (*thread_arg->finished)
      break;

    thread_arg->worker_thd->set_time();

    THD_STAGE_INFO(thread_arg->worker_thd, stage_running_task);

    thread_func = *(thread_arg->thread_func);
    thread_func_arg = *(thread_arg->thread_func_arg);

    // All tasks' handled here.
    (*thread_func)(thread_func_arg);

    DBUG_EXECUTE_IF("schedule_one_loop_sleep_worker", sleep(2););
    // Lock here.
    finish_task(thread_arg);
  }
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
  assert(worker_pool);
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
      assert(child && parent);
      Worker_exec_ctx child_desc(child, num_workers);
      Worker_exec_ctx parent_desc(parent, num_workers);
      PX_PRINT_INFO("COMMAND run task (%ld,%ld)", child->dfo_id(),
        child->is_root_dfo() ? -1 : parent->dfo_id());

      // Get exchange info from exchange info manager instance.
      exchange_info = thd()->px_exchange_context->get(child->dfo_id());
      assert(exchange_info);
      // Attach exchange info for child, parent or last parent dfo.
      attach_exchange_info(num_workers, th_arg_array, &last_parent_desc,
                           &child_desc, &parent_desc);

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
        if (begin_task(worker_pool, &parent_desc)) return true;
        parent->set_dfo_active();
      }

      // Prepare for child dfo execution threads.
      if (!child->is_dfo_active()) {
        // Start to schedule child dfo, allocate the threads' array bitmap,
        // set the threads' bitmap, get the group id.
        if (allocate_threads(worker_pool, &child_desc)) return true;
        // Set the arguments of child tasks, set the dfo active at last.
        prepare_schedule_single_dfo(num_workers, th_arg_array, &child_desc);
        // Post for start of the tasks on every workers.
        if (begin_task(worker_pool, &child_desc)) return true;
      }

      PX_PRINT_INFO("Child map[%d] Parent map[%d]",
          *child_desc.bitmap.bitmap, *parent_desc.bitmap.bitmap);

      if (parent->is_root_dfo()) run_root_dfo_task();

      if (wait_workers_finish_task(worker_pool, &child_desc)) return true;

      DEBUG_SYNC_C("schedule_one_loop_kill");
      DBUG_EXECUTE_IF("schedule_one_loop_sleep", sleep(2););

      if (thd()->killed) return true; // been killed.
      if (thd()->need_fallback) return true; // Fallback to serial execution.
      if (thd()->check_px_error()) { // error and return.
        mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_error);
        txsql_parallel_stmt_error++;
        mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_error);
        return true;
      }

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
  for (int i = 0; i < num_workers; ++i) {
    if (bitmap_is_set(&exec_ctx->bitmap, i)) {
      args[i]->task_id = dfo->dfo_id();
      args[i]->exchange_info = exchange_info;
      args[i]->worker_thd->thread_group_id = exec_ctx->group_id();
      args[i]->worker_thd->worker_arg->task_id = dfo->dfo_id();
      if (dfo->is_leaf_dfo()) // set px scan ctx for leaf dfo.
        args[i]->scan_ctx = static_cast<PX_reader*>(dfo->m_px_scan_ctx);
    }
  }
}

/**
  Fallback to serial execution if check need fallback and restart statement.
  The reasons why query needs to fallback to serial execution has:
  1. unequal query plan (whatever the cause);
  2. error occured before sending data;

  @param thd            the session
  @param parser_state   the parser state
*/
void fallback_to_serial_execution(THD *thd, Parser_state *parser_state, 
                                  const char *query_string,
                                  size_t query_length) {
  PX_PRINT_INFO("fall back to serial execution.");

  mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_fallback);
  txsql_parallel_stmt_fallback++;
  mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_fallback);

  // Tell performance schema that the statement is restarted.
  MYSQL_END_STATEMENT(thd->m_statement_psi, thd->get_stmt_da());
  thd->m_statement_psi = MYSQL_START_STATEMENT(&thd->m_statement_state,
      com_statement_info[thd->get_command()].m_key,
      thd->db().str, thd->db().length, thd->charset(), nullptr);
  THD_STAGE_INFO(thd, stage_starting_fallback);

  // Reset the statement digest state.
  thd->m_digest = &thd->m_digest_state;
  thd->m_digest->reset(thd->m_token_array, max_digest_length);

  // Reset the parser state.
  thd->set_query(query_string, query_length);
  parser_state->reset(thd->query().str, thd->query().length);

  // Disable the general log. The query was written to the general log in the
  // first attempt to execute it. No need to write it twice.
  const uint64_t saved_option_bits = thd->variables.option_bits;
  thd->variables.option_bits |= OPTION_LOG_OFF;

  dispatch_sql_command(thd, parser_state);

  // Restore the original option bits.
  thd->variables.option_bits = saved_option_bits;
}

#include "sql/parallel_execution/px_optimizer.h"

#include "mem_root_deque.h"
#include "sql/join_optimizer/access_path.h"
#include "sql/join_optimizer/walk_access_paths.h" // WalkAccessPathPolicy
#include "sql/parallel_execution/px_access_path.h"
#include "sql/parallel_execution/px_executor.h"  // PX_executor
#include "sql/parallel_execution/px_interface.h" // PX_ROLE_COORDINATOR
#include "sql/parallel_execution/px.h"           // PX_PRINT_INFO
#include "sql/sql_class.h"
#include "sql/sql_union.h"
#include "sql/sql_optimizer.h"

/// RAII class to automate saving/restoring of current_select()
class Change_cur_select {
 public:
  Change_cur_select(THD *thd_arg)
      : thd(thd_arg), saved_select(thd->lex->current_query_block()) {}
  void restore() { thd->lex->set_current_query_block(saved_select); }
  ~Change_cur_select() { restore(); }

 private:
  THD *thd;
  Query_block *saved_select;
};

// Check whether the parallel table is consistent with the table specified by
// the parallel hint
void check_parallel_table_hint(const THD *thd, bool do_parallel) {
  for (Query_block *select = thd->lex->all_query_blocks_list; select;
       select = select->next_select_in_list()) {
    Opt_hints_qb *qb_hint = select->opt_hints_qb;
    if (!qb_hint || !qb_hint->is_specified(PARALLEL_TABLE_HINT_ENUM)) {
      continue;
    }

    JOIN *join = select->join;
    for (QEP_TAB *tab = join->qep_tab; tab && tab->table_ref; tab++) {
      Opt_hints_table *table_hint = tab->table_ref->opt_hints_table;
      if (!table_hint || !table_hint->is_set_parallel_hint())
        continue;

      bool do_parallel_scan_hint = table_hint->do_parallel_scan();
      if (!tab->m_parallel_scan && do_parallel_scan_hint) {
        if (do_parallel) {
          push_warning(current_thd, Sql_condition::SL_WARNING,
                       ER_WARN_UNSUPPORTED_PARALLEL,
                       "parallel hint has no effect, not the chosen table.");
        } else {
          push_warning(current_thd, Sql_condition::SL_WARNING,
                       ER_WARN_UNSUPPORTED_PARALLEL,
                       "parallel hint has no effect, statement cannot be "
                       "executed in parallel.");
        }
      }
    }
  }
}

/**
 * Set the degree hint effective when table hint is effective, if these two
 * hints are actually part of one parallel hint.
 */
void set_parallel_degree_hint(const THD *thd, TABLE_LIST *tbl) {
  Opt_hints_global *global_hints = thd->lex->opt_hints_global;
  if (hint_table_state(thd, tbl, PARALLEL_TABLE_DEGREE_HINT_ENUM, 0)) {
    assert(global_hints->is_specified(PARALLEL_HINT_ENUM));
    global_hints->parallel_hint->set_effective_hint(true);
  }
}

/**
 * @return degree hint if found, UINT_MAX32 otherwise
 */
ulong get_parallel_degree_hint(const THD *thd) {
  ulong degree_hint = UINT_MAX32;
  Opt_hints_global *global_hints = thd->lex->opt_hints_global;
  if (global_hints && global_hints->is_specified(PARALLEL_HINT_ENUM) &&
      global_hints->parallel_hint->is_effective_hint()) {
    PT_hint_parallel *parallel_hint = global_hints->parallel_hint;
    assert(parallel_hint);
    degree_hint = parallel_hint->get_degree();
  }
  return degree_hint;
}

/**
 * Compatibility check of execution plans and parallelization optimization and
 * transformation of plans that pass the check.
 *
 * @return true for error.
 */
bool px_optimize(THD *thd, JOIN *join, AccessPath *root) {
  // Check compatibility for parallel
  if (thd->need_fallback ||
      !thd->lex->pass_px_check ||
      !thd->lex->check_px_execution()) {
    thd->lex->pass_px_check = false;
    return false;
  }

  // Initialize for PX_PRINT_ macros.
  PX_EXECUTOR(thd) = nullptr;

  // Check compatibility and get exchange points.
  std::vector<px_access_path::Split_Position> split_positions_all;
  std::vector<AccessPath *> mat_access_path;  // vector for MATERIALIZE and APPEND
                                         // that cross query block
  uint ref_slice = REF_SLICE_SAVED_BASE;
  bool exchange_safe = false;
  bool is_stream = false;
  AccessPath *max_px_subpath = nullptr;  // Not used
  (void)px_access_path::WalkAccessPathsForCompat(
      thd, root, nullptr, join, /*parallel_scan=*/true, /*root=*/true,
      /*root_all=*/true, ref_slice, max_px_subpath, is_stream, &mat_access_path,
      &split_positions_all, exchange_safe);

  size_t exchange_count =
      px_access_path::count_exchange_in_split_pos(&split_positions_all);
  if (split_positions_all.empty() || !exchange_count) {
    thd->lex->pass_px_check = false;
    return false;
  }

  // There must be parallel table node in split_positions_all.
  assert(exchange_count < split_positions_all.size());

  // Optimize
  std::vector<px_access_path::Split_Position> split_positions;
  std::list<QEP_TAB *> parallel_tab;  // Save all tabs to be parallel scanned.
  try {
    if (px_access_path::FindExchangeInjectPosition(thd, &split_positions_all,
                                                   &split_positions,
                                                   &parallel_tab)) {
      assert(0);
      thd->lex->pass_px_check = false;
      return true;
    }
  } catch (std::bad_alloc &) {
    thd->lex->pass_px_check = false;
    my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0),
             "finding possible exchange positions", "px_optimize()");
    return true;
  }
  if (!split_positions.size()) {
    thd->lex->pass_px_check = false;
    return false;
  }

#ifndef DBUG_OFF
  PrintSplitPostion("Split all", &split_positions_all);
  PrintSplitPostion("Split", &split_positions);
#endif

  // Generate parallalized plan.
  if (px_generate_plan(thd, &split_positions, &mat_access_path)) return true;

  for (const QEP_TAB *tab : parallel_tab) {
    tab->set_parallel_scan(true);
  }

  // Got a parallel plan. Tell the parallel executor to take over execution.
#ifndef DBUG_OFF
  if (PX_ROLE_COORDINATOR(thd)) {
    PX_PRINT_INFO("USE PX: %s", thd->query().str);
  }
#endif
  thd->use_px = true;

  return false;
}

static void FixAccessPathForUnit(AccessPath *target_path, JOIN *join, bool is_union);

static void FixAccessPathForUnion(AccessPath *target_path);

/**
  Generate the parallel execution plan with given exchange descriptions.

  An exchange description provides necessary information to inject a pair of
  exchange nodes (sender and receiver) at a certain position in the access path
  tree, and to split special operations like aggregation and order into a pair
  of operations (local and final) enclosing the exchange nodes.

  With these modifications, a serial plan thus becomes a parallel one.

  @param split_positions Exchange descriptions
  @param mat_access_paths Special access paths that need to be fixed

  @return false on success, true on error.
*/
bool px_generate_plan(THD *thd,
    std::vector<px_access_path::Split_Position> *split_positions,
    std::vector<AccessPath *> *mat_access_paths) {
  Change_cur_select save_select(thd);

  // Travel split_positions to do split accesspath and inject exchange.
  for (px_access_path::Split_Position &sp : *split_positions) {
    AccessPath *target_path = sp.m_target;
    JOIN *join = sp.m_join;
    Query_expression *target_unit = join->query_expression();
    assert(target_path && join && join->is_optimized());

    // Inject exchange for union.
    if (sp.m_split_union) {
      AccessPath *root_path = nullptr;
      if (target_path->type == AccessPath::MATERIALIZE) {
        TABLE *table = target_path->materialize().param->table;
        root_path =
            CreateExchangeAccessPathForUnion(thd, target_path, table);
      } else if (target_path->type == AccessPath::APPEND) {
        root_path =
            CreateExchangeAccessPathForUnion(thd, target_path,
              join->query_expression()->get_union_result()->table, true);
      }
      if (root_path == nullptr) return true;
      join->query_expression()->set_root_access_path(root_path);
      thd->lex->m_exchange_number++;
      continue;
    }

    thd->lex->set_current_query_block(join->query_block);

    // Generate plan for parallel execution according to split_position.
    if (join->px_generate_plan(&sp)) {
      return true;
    }

    /**
      If fake_query_block of unit is not null, root_access_path of unit should
      be set to fake_query_block's root_access_path. So, if this is fake_query_block,
      root_access_path of unit need to be updated.
    */
    if (join->query_expression()->fake_query_block &&
        join->query_expression()->fake_query_block->join == join) {
      join->query_expression()->set_root_access_path(join->root_access_path());
      continue;
    }

    /*
      If this unit is simple, it's root_access_path is set to root_access_path of
      first_select.
    */
    if (target_unit->is_simple()) {
      target_unit->set_root_access_path(join->root_access_path());
      continue;
    }

    /**
      Sometimes, fake_query_block of unit is not null even if this unit is not union.
      For example, case in main.parser :
          (SELECT 1 FROM t1 ORDER BY 1) LIMIT 1;
      This case may create a stream accesspath for fake_query_block, if a exchange is
      inject at the bottom of it, we need to fix the connection of stream and exchange.
    */
    if (!target_unit->is_union() && target_unit->fake_query_block) {
      FixAccessPathForUnit(target_unit->root_access_path(), join, false);
    }
  }

  /*
    Fix connections for unions.

    A union is represented by MATERIALIZE (UNION) or APPEND (UNION ALL). The
    root access path of a union branch may be changed by injecting a pair of
    exchange operators between the branch and the union, so that the branch can
    be run in parallel. In this case, the connection should be fixed.
  */
  if (mat_access_paths->size() != 0) {
    for (AccessPath *target_path : *mat_access_paths) {
      if (target_path) {
        FixAccessPathForUnit(target_path, nullptr, true);
      }
    }
  }

  return false;
}

/**
  Generate partial parallel plan for the partial serial plan in a query block.

  Any aggregate operation should be split into a pair of aggregates, one
  processes partial inputs on each worker and gets partial results, and the
  other combines the partial results to final result.

  Currently we stick to JOIN because we have not built a uniform input model
  for data references yet, which may cross access paths. It is future work.

  @param split_positions Exchange descriptions

  @return false on success, true on error.
*/
bool JOIN::px_generate_plan(px_access_path::Split_Position *split_position) {
  assert(is_optimized() && !is_px_generated());
  AccessPath *root_path = root_access_path();

  // PHASE-1: Rebuild the aggr operator if necessary.
  if (split_position->m_split_agg) {
    root_path = WalkAccessPathsForAggregationRebuild(thd, this, root_path, false);

    // Whether the AGG is successfully Rebuilt.
    if (!root_path || ref_items[REF_SLICE_FINAL_AGGREGATE].is_null()) {
      assert(false); // For test
      return true;
    }
  }

  // PHASE-2: Inject the exchange operators.
  int old_exchange_num = thd->lex->m_exchange_number;
  if (exchange_temp_table == nullptr) {
    exchange_temp_table =
        new (thd->mem_root) mem_root_deque<TABLE *>(thd->mem_root);
    exchange_temp_table_param =
        new (thd->mem_root) mem_root_deque<Temp_table_param *>(thd->mem_root);
    if (!exchange_temp_table || !exchange_temp_table_param) {
      assert(false);
      return true;
    }
  }
  bool new_child = false;
  bool split_sort = split_position->m_split_sort;
  assert(!split_position->m_tables);
  split_position->m_tables =
      new (thd->mem_root) mem_root_deque<TABLE *>(thd->mem_root);
  if (!split_position->m_tables) return true;
  GetExchangeTables(split_position);
  assert(split_position->m_tables->size());
  AccessPath *exchange = WalkAccessPathsForExchange(
        thd, this, split_position, root_path, split_position->m_target,
        /*curr_exchange=*/0, /*new_child=*/new_child, /*cur_slice*/-1,
        false, /*in_join=*/false);

  if (thd->is_error() || thd->lex->m_exchange_number == old_exchange_num) {
    assert(false); // For test
    return true;
  }

  if (exchange) {
    if (split_sort ? exchange->px_receiver_merge().use_temp_table
                   : exchange->px_receiver().use_temp_table) {
      int ref_slice = split_sort ? exchange->px_receiver_merge().ref_slice
                                 : exchange->px_receiver().ref_slice;
      fields = &tmp_fields[ref_slice];
    }
    if (new_child)
      root_path = exchange;
  }
  m_root_access_path = root_path;

  set_px_generated();

  return false;
}

/**
  Fix connections for unit.

  For parallel execution, parallel optimization will be done after serial
  optimization of all query blocks. So, branch query block of unit may
  be changed, it's necessary to substitute the old branch qb by parallel-
  optimized qb.

  @param target_path accesspath need to be fixed.
  @param join
  @param is_union fix for MATERIALIZE/APPEND or not.
*/
static void FixAccessPathForUnit(AccessPath *target_path, JOIN *join, bool is_union) {
  if (is_union) {
    FixAccessPathForUnion(target_path);
    return ;
  }

  AccessPath *path = nullptr;
  const auto scan_functor = [&path](AccessPath *sub_path, const JOIN *) {
    switch(sub_path->type) {
      case AccessPath::STREAM: {
        path = sub_path;
        return true;
      }
      default:
        return false;
    }
  };
  WalkAccessPaths(target_path, /*join=*/nullptr,
                  WalkAccessPathPolicy::ENTIRE_TREE, scan_functor);
  if (path == nullptr) return ;
  path->stream().child = join->root_access_path();
  /*
  if (path->stream().copy_fields_and_items_in_materialize) {
    TABLE *dst_table = path->stream().table;
    if (join->tmp_table_param.items_to_copy) {
      join->tmp_table_param.items_to_copy = nullptr;
      ConvertItemsToCopy(*join->fields, dst_table->visible_field_ptr(),
                         &join->tmp_table_param);
    }
  }
  */
}

/**
  Fix connections for unions.

  A union is represented by MATERIALIZE (UNION) or APPEND (UNION ALL). The
  root access path of a union branch may be changed by injecting a pair of
  exchange operators between the branch and the union, so that the branch can
  be run in parallel. In this case, the connection should be fixed.

  One exception is that MATERIALIZE created in QB to materialize results also
  need to be fixed.

  @param target_path MATERIALIZE or APPEND accesspath
*/
static void FixAccessPathForUnion(AccessPath *target_path) {
  switch (target_path->type) {
    case AccessPath::MATERIALIZE: {
      MaterializePathParameters *param = target_path->materialize().param;
      TABLE *dst_table = param->table;
      for (MaterializePathParameters::QueryBlock &query_block :
           param->query_blocks) {
        JOIN *join = query_block.join;
        AccessPath *qb_root_path = join->root_access_path();
        if (!join->is_px_generated()) continue;
        if (query_block.subquery_path != qb_root_path) {
          query_block.subquery_path = qb_root_path;
        }
        // Fix items_to_copy
        if (join->tmp_table_param.items_to_copy) {
          join->tmp_table_param.items_to_copy = nullptr;
          ConvertItemsToCopy(*join->fields, dst_table->visible_field_ptr(),
                             &join->tmp_table_param);
        }
      }
      break;
    }
    case AccessPath::APPEND: {
      for (AppendPathParameters app_param :
           *target_path->append().children) {
        AccessPath *child = app_param.path;
        if (child->type == AccessPath::MATERIALIZE) {
          continue;
        }
        AccessPath *stream_path = app_param.path;
        JOIN *join = app_param.join;
        AccessPath *qb_root_path = join->root_access_path();
        if (!join->is_px_generated()) continue;
        if (stream_path->stream().child != qb_root_path) {
          stream_path->stream().child = qb_root_path;
        }
        TABLE *dst_table = stream_path->stream().table;
        // Fix items_to_copy
        if (join->tmp_table_param.items_to_copy) {
          join->tmp_table_param.items_to_copy = nullptr;
          ConvertItemsToCopy(*join->fields, dst_table->visible_field_ptr(),
                              &join->tmp_table_param);
        }
      }
      break;
    }
    default:
      assert(false);
  }
}

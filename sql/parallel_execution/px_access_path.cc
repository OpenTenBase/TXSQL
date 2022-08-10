#include "sql/parallel_execution/px_access_path.h"

#include "sql/filesort.h"
#include "sql/join_optimizer/access_path.h"
#include "sql/join_optimizer/walk_access_paths.h"
#include "sql/parallel_execution/px_executor.h"  // PX_PRINT_INFO
#include "sql/parallel_execution/px_item.h"
#include "sql/parallel_execution/px_optimizer.h"  // set_parallel_degree_hint()
#include "sql/table_function.h"
#include "sql/range_optimizer/path_helpers.h"

/**
 * @return true if exchange safe, false otherwise.
 */
static bool compat_for_table_fields(TABLE *table) {
  assert(table);
  bool ret = true;
  for (Field **pfield = table->field; *pfield != nullptr; ++pfield) {
    Field *field = *pfield;
    if (bitmap_is_set(table->read_set, field->field_index())) {
      if (check_xchg_unsafe_field(field)) {
        ret = false;
        break;
      }
    }
  }
  return ret;
}

/**
  Check the fields of parallel table is unsafe or not.
  @return true if paralel scan safe, false otherwise.
*/
static bool compat_for_parallel_table_fields(TABLE *table) {
  assert(table);
  for (Field **pfield = table->field; *pfield != nullptr; ++pfield) {
    Field *field = *pfield;
    if (bitmap_is_set(table->read_set, field->field_index()) &&
        check_parallel_scan_unsafe_field(field)) {
      return false;
    }
  }
  return true;
}

/**
 * @return true if the table can be chosen as the parallelized table, false
 * otherwise.
 */
static bool compat_for_parallel_table(const THD *thd, TABLE *tb) {
  TABLE_LIST *tbl = tb->pos_in_table_list;
  assert(tb && tbl);

  // Parallel hint.
  if (tbl->opt_hints_table &&
      tbl->opt_hints_table->not_do_parallel_scan()) {
    return false;
  }

  /*
    Check the table type. These tables can't be chosen as
    the parallelized table:
    1) temporary table
    2) non-InnoDB table
    3) partition table
    4) non-User table
    5) fulltext match search
    6) view or derived table
  */

  if (tb->s->tmp_table != NO_TMP_TABLE ||
      tb->file->ht->db_type != DB_TYPE_INNODB || tb->part_info ||
      tb->s->table_category != TABLE_CATEGORY_USER ||
      tbl->lock_descriptor().type > TL_READ_DEFAULT ||
      tbl->is_fulltext_searched() || tbl->is_view_or_derived()) {
    return false;
  }
  /*
    Check records in table.
    If it is less than cdb_min_parallel_table_rows, refuse to do parallel
    execution.
  */
  if (tb->file->stats.records <
      thd->variables.txsql_parallel_table_record_threshold) {
    return false;
  }

  if (!compat_for_parallel_table_fields(tb)) {
    return false;
  }

  // The dop hint is effective within the whole statement. Set the degree hint
  // effective when table hint is effective, if these two hints are actually
  // part of one parallel hint.
  set_parallel_degree_hint(thd, tbl);
  return true;
}

/**
 * @return true if the equivalence check is satisfied, false otherwise
 */
static bool group_field_eq_sort_list(List<Cached_item> group_fields,
                                     ORDER *sort_order) {
  // Note: The group and order columns are in reverse order. see
  // alloc_group_fields()
  List<Item> group_fields_reverse;
  for (auto first = group_fields.begin(); first != group_fields.end();
       ++first) {
    group_fields_reverse.push_front(first->get_item());
  }
  auto first = group_fields_reverse.begin();
  ORDER *second = sort_order;

  for (; first != group_fields_reverse.end() && second;
       ++first, second = second->next) {
    if (first->eq(*second->item, true)) {
      continue;
    } else {
      return false;
    }
  }
  if (first != group_fields_reverse.end() || second) return false;
  return true;
}

/**
 * Whether MaterializeIterator are deduplicating, whether through a hash field
 * or a regular unique index.
 * @see MaterializeIterator::doing_deduplication()
 */
static bool materialize_doing_deduplication(AccessPath *path, TABLE *table) {
  assert(path->type == AccessPath::MATERIALIZE);
  if (table->hash_field) return true;

  if (table->key_info != nullptr) {
    for (size_t i = 0; i < table->s->keys; ++i) {
      if ((table->key_info[i].flags & HA_NOSAME) != 0) {
        return true;
      }
    }
  }
  return false;
}

/**
 * Get the largest parallelizable subtree of the entire plan, as well as all
 * possible exchange insertion points, by combining depth-first preorder
 * postorder traversal.
 *
 * Use preorder traversal to get some current state information. Get the largest
 * parallelizable subtree using post-order traversal at all nodes by judging
 * whether the subtree, access path type and operations of each node are safe to
 * parallelize.
 *
 * MATERIALIZE is an exception. Its traversal order is more complicated. @see
 * case AccessPath::MATERIALIZE.
 *
 * In addition to the parallel_safe, there is also a parameter called
 * exchange_safe indicating whether the exchange can be inserted on the node. It
 * only related to whether the fields to be sent of current node is supported
 * for cross-thread exchange, @see check_xchg_safe_field_type(). When the the
 * child node is NOT exchange_safe, it does NOT mean that the parent node is
 * also not exchange_safe. Because the type of fields may change as the
 * calculation progresses. And not all item results need to be sent, such as
 * filter conditions. So it is necessary to distinguish between parallel_safe
 * and exchange_safe.
 *
 * @param thd
 * @param path    current access path
 * @param parent  parent access path
 * @param cur_join
 * @param parallel_scan  If false, only judge whether it is safe to parallelize,
 *                       not consider inserting exchange. This may affect the
 *                       judgment of whether safe to parallel.
 * @param root        If true, the access path is the root of a join.
 * @param root_all    If true, the access path is the root part of the entire
 *                    plan, safe for union to parallelize.
 *                    @see MoveCompositeIteratorsFromTablePath().
 * @param[out] ref_slice  current ref_slice used by exchange. The ref_slice of
 *                        the current node is determined by the current node or
 *                        child nodes.
 *                        @todo not used in exchange inject actually
 * @param[out] max_px_subpath   the largest parallelizable subtree of the entire
 *                              plan
 * @param[out] is_stream        whether the subtree is a streaming plan. It
 *                              determines whether the limit operator will
 *                              truncate parallel scans.
 * @param[out] mat_access_path  vector for MATERIALIZE and APPEND that cross
 *                              query block
 * @param[out] split_positions  all possible exchange insertion points
 * @param[out] exchange_safe    whether it is safe to insert exchange.
 *
 * @return true if safe to parallelize, false otherwise.
 */
bool px_access_path::WalkAccessPathsForCompat(
    THD *thd, AccessPath *path, AccessPath *parent, JOIN *cur_join,
    bool parallel_scan, bool root, bool root_all, uint &ref_slice,
    AccessPath *&max_px_subpath, bool &is_stream,
    std::vector<AccessPath *> *const mat_access_path,
    std::vector<Split_Position> *const split_positions, bool &exchange_safe) {
  bool parallel_safe = false;
  switch (path->type) {
    case AccessPath::TABLE_SCAN: {
      const auto &param = path->table_scan();
      exchange_safe = compat_for_table_fields(param.table);
      is_stream = true;
      ref_slice = REF_SLICE_SAVED_BASE;
      
      if (!parallel_scan) {
        parallel_safe = true;
      } else if (compat_for_parallel_table(thd, param.table)) {
        parallel_safe = true;
        //split_positions->emplace_back(cur_join, param.qep_tab);
        if (exchange_safe) {
          max_px_subpath = path;
          split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                        false, false, false, nullptr, nullptr);
        }
      } else {
        parallel_safe = false;
      }
      break;
    }
    case AccessPath::INDEX_SCAN: {
      const auto &param = path->index_scan();
      exchange_safe = compat_for_table_fields(param.table);
      is_stream = true;
      ref_slice = REF_SLICE_SAVED_BASE;
      if (!parallel_scan) {
        parallel_safe = true;
      } else if (compat_for_parallel_table(thd, param.table)) {
        parallel_safe = true;
        //split_positions->emplace_back(cur_join, param.qep_tab);
        if (exchange_safe) {
          max_px_subpath = path;
          split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                        false, param.use_order, false, nullptr,
                                        nullptr);
        }
      } else {
        parallel_safe = false;
      }
      break;
    }
    case AccessPath::REF: {
      const auto &param = path->ref();
      exchange_safe = compat_for_table_fields(param.table);
      is_stream = true;
      ref_slice = REF_SLICE_SAVED_BASE;
      if (!parallel_scan) {
        parallel_safe = true;
      } else if (compat_for_parallel_table(thd, param.table)) {
        parallel_safe = true;
        //split_positions->emplace_back(cur_join, param.qep_tab);
        if (exchange_safe) {
          max_px_subpath = path;
          split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                        false, param.use_order, false, nullptr,
                                        nullptr);
        }
      } else {
        parallel_safe = false;
      }
      break;
    }
    case AccessPath::INDEX_RANGE_SCAN: {
      const auto &param = path->index_range_scan();
      TABLE *table = param.used_key_part[0].field->table;
      exchange_safe = compat_for_table_fields(table);
      is_stream = true;
      ref_slice = REF_SLICE_SAVED_BASE;
      if (!parallel_scan) {
        parallel_safe = true;
      } else if (compat_for_parallel_table(thd, table)) {
        parallel_safe = true;
        //split_positions->emplace_back(cur_join, param.qep_tab);
        if (exchange_safe) {
          max_px_subpath = path;
          // @TODO
          bool use_order =  true;
          split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                        false, use_order, false, nullptr,
                                        nullptr);
        }
      } else {
        parallel_safe = false;
      }
      break;
    }
    case AccessPath::REF_OR_NULL: {
      is_stream = true;
      exchange_safe = compat_for_table_fields(path->ref_or_null().table);
      ref_slice = REF_SLICE_SAVED_BASE;
      parallel_safe = !parallel_scan;
      break;
    }
    case AccessPath::EQ_REF: {
      is_stream = true;
      exchange_safe = compat_for_table_fields(path->eq_ref().table);
      ref_slice = REF_SLICE_SAVED_BASE;
      parallel_safe = !parallel_scan;
      break;
    }
    case AccessPath::CONST_TABLE: {
      is_stream = true;
      exchange_safe = compat_for_table_fields(path->const_table().table);
      ref_slice = REF_SLICE_SAVED_BASE;
      parallel_safe = !parallel_scan;
      break;
    }
    case AccessPath::PUSHED_JOIN_REF:
    case AccessPath::FULL_TEXT_SEARCH:
    case AccessPath::MRR:
    case AccessPath::FOLLOW_TAIL:
    case AccessPath::DYNAMIC_INDEX_RANGE_SCAN:
    case AccessPath::TABLE_VALUE_CONSTRUCTOR:
    case AccessPath::FAKE_SINGLE_ROW:
    case AccessPath::ZERO_ROWS:
    case AccessPath::ZERO_ROWS_AGGREGATED:
    case AccessPath::MATERIALIZED_TABLE_FUNCTION:
    case AccessPath::UNQUALIFIED_COUNT: {
      // No children.
      parallel_safe = false;
      break;
    }
    case AccessPath::NESTED_LOOP_JOIN: {
      bool exchange_safe_l = false, exchange_safe_r = false;
      bool is_stream_l = false, is_stream_r = false;
      uint child_ref_slice_l = REF_SLICE_SAVED_BASE,
           child_ref_slice_r = REF_SLICE_SAVED_BASE;
      // Only the first non-const table in join can be parallelize table now, if
      // the outer table is const table, the inner table can be parallelize
      // table as its join condition is const.
      bool left_const =
          (path->nested_loop_join().outer->type == AccessPath::FAKE_SINGLE_ROW);
      bool child_parallel_safe = false;
      if (!left_const) {
        child_parallel_safe =
            (WalkAccessPathsForCompat(thd, path->nested_loop_join().outer, path,
                                      cur_join, parallel_scan, false, false,
                                      child_ref_slice_r, max_px_subpath,
                                      is_stream_r, mat_access_path,
                                      split_positions, exchange_safe_r) &&
             WalkAccessPathsForCompat(
                 thd, path->nested_loop_join().inner, path, cur_join,
                 /*parallel_scan=*/false, /*root=*/false, /*root_all=*/false,
                 child_ref_slice_l, max_px_subpath, is_stream_l,
                 mat_access_path, split_positions, exchange_safe_l));
        is_stream = is_stream_r;  // Only concern the branch to parallelize scan
      } else if (path->nested_loop_join().join_type == JoinType::INNER) {
        // Only when the join type is Inner Join, the inner table can be select
        // as the parallel table. If the join type is Left Join or Anti Join,
        // the inner table cannot be a parallel table, because when the inner
        // table scans the slice without corresponding data and the
        // corresponding data is in another slice, a join record will also be
        // generated. Nested semi join is rarely used so ignore it for now.
        // TODO: If the join type is not Inner Join, although the Nested join
        // itself cannot be parallelized, the inner branch can still be
        // parallelized. But exchange injection not support this condition
        // currently.
        exchange_safe_r = true;
        is_stream_r = true;
        child_parallel_safe = WalkAccessPathsForCompat(
            thd, path->nested_loop_join().inner, path, cur_join, parallel_scan,
            /*root=*/false, /*root_all=*/false, child_ref_slice_l,
            max_px_subpath, is_stream_l, mat_access_path, split_positions,
            exchange_safe_l);
        is_stream = is_stream_l;  // Only concern the branch to parallelize scan
      }
      if (child_parallel_safe) {
        parallel_safe = true;
        ref_slice = REF_SLICE_SAVED_BASE;
        if (parallel_scan) {
          exchange_safe = exchange_safe_r && exchange_safe_l;
          if (exchange_safe) {
            max_px_subpath = path;
            split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                          false, false, false, nullptr,
                                          nullptr);
          }
        }
      } else {
        parallel_safe = false;
      }
      break;
    }
    case AccessPath::NESTED_LOOP_SEMIJOIN_WITH_DUPLICATE_REMOVAL: {
      parallel_safe = false;
      break;
    }
    case AccessPath::BKA_JOIN: {
      // BKA_JOIN is not supported in parallel query.
      parallel_safe = false;
      break;
    }
    case AccessPath::HASH_JOIN: {
      // HASH_JOIN is not supported in parallel query.
      parallel_safe = WalkAccessPathsForCompat(
          thd, path->hash_join().inner, path, cur_join, parallel_scan, false,
          false, ref_slice, max_px_subpath, is_stream, mat_access_path,
          split_positions, exchange_safe);
      if (parallel_scan) {
        parallel_safe = false;
      }
      is_stream = false;
      break;
    }
    case AccessPath::FILTER: {
      if (WalkAccessPathsForCompat(thd, path->filter().child, path, cur_join,
                                   parallel_scan, false, root_all, ref_slice,
                                   max_px_subpath, is_stream, mat_access_path,
                                   split_positions, exchange_safe) &&
          !check_px_unsafe_cond(cur_join, path->filter().condition,
                                ref_slice)) {
        parallel_safe = true;
        if (parallel_scan && exchange_safe) {
          max_px_subpath = path;
          split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                        false, false, false, nullptr, nullptr);
        }
      } else {
        parallel_safe = false;
      }
      break;
    }
    case AccessPath::SORT: {
      if (WalkAccessPathsForCompat(thd, path->sort().child, path, cur_join,
                                   parallel_scan, false, root_all, ref_slice,
                                   max_px_subpath, is_stream, mat_access_path,
                                   split_positions, exchange_safe) &&
          !check_px_unsafe_order(cur_join, path->sort().filesort->m_order,
                                 ref_slice)) {
        parallel_safe = true;
        if (parallel_scan && exchange_safe) {
          max_px_subpath = path;
          split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                        false, true, false,
                                        path->sort().filesort, nullptr);
        }
      } else {
        parallel_safe = false;
      }
      is_stream = false;
      break;
    }
    case AccessPath::AGGREGATE: {
      // If there is an aggregate function but no GROUP BY, the WINDOWING access
      // path will not be used and the window function will be calculated in the
      // projector. Rebuild AGGREGATE does not support this case.
      if (parallel_scan && !cur_join->m_windows.is_empty()) {
        parallel_safe = false;
        break;
      }
      const auto &param = path->aggregate();
      if (WalkAccessPathsForCompat(thd, param.child, path, cur_join,
                                   parallel_scan, false, false, ref_slice,
                                   max_px_subpath, is_stream, mat_access_path,
                                   split_positions, exchange_safe) &&
          !param.rollup && !check_px_unsafe_sum_funcs(cur_join) &&
          !check_px_unsafe_group(cur_join->group_fields)) {
        parallel_safe = true;
        if (parallel_scan) {
          exchange_safe =
              (!check_xchg_unsafe_sum_funcs(cur_join));
          if (exchange_safe) {
            max_px_subpath = path;
            split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                          true, false, false, nullptr, nullptr);
          }
        }
      } else {
        parallel_safe = false;
      }
      break;
    }
    case AccessPath::TEMPTABLE_AGGREGATE: {
      const auto &param = path->temptable_aggregate();
      if (WalkAccessPathsForCompat(thd, param.subquery_path, path, cur_join,
                                   parallel_scan, false, false, ref_slice,
                                   max_px_subpath, is_stream, mat_access_path,
                                   split_positions, exchange_safe) &&
          !check_px_unsafe_sum_funcs(cur_join) &&
          !check_px_unsafe_temp_param(param.temp_table_param) &&
          !check_px_unsafe_order(cur_join, param.table->group, ref_slice)) {
        parallel_safe = true;
        if (param.ref_slice != -1) ref_slice = param.ref_slice;
        if (parallel_scan) {
          exchange_safe = compat_for_table_fields(param.table);
          if (exchange_safe) {
            max_px_subpath = path;
            split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                          true, false, false, nullptr, nullptr);
          }
        }
      } else {
        parallel_safe = false;
      }
      is_stream = false;
      break;
    }
    case AccessPath::LIMIT_OFFSET: {
      WalkAccessPathsForCompat(thd, path->limit_offset().child, path, cur_join,
                               parallel_scan, false, root_all, ref_slice,
                               max_px_subpath, is_stream, mat_access_path,
                               split_positions, exchange_safe);
      // The limit operator may truncate parallel scans.
      if (parallel_scan && is_stream) {
        split_positions->clear();
      }
      parallel_safe = !parallel_scan;
      break;
    }
    case AccessPath::STREAM: {
      const auto &param = path->stream();
      // Change cur_join
      // TODO: View
      TABLE_LIST *tl = param.table->pos_in_table_list;
      if (tl && tl->is_view()) {
        parallel_safe = false;
        break;
      }
      bool child_exchange_safe = false;
      bool child_root = (cur_join != param.join);
      if (root_all && (param.join == cur_join)) {
        root_all = true;
      }
      if (WalkAccessPathsForCompat(
              thd, param.child, path, param.join, parallel_scan, child_root,
              root_all, ref_slice, max_px_subpath, is_stream, mat_access_path,
              split_positions, child_exchange_safe) &&
          !check_px_unsafe_temp_param(param.temp_table_param)) {
        parallel_safe = true;
        // ref_slice should be switched. It is unreasonable that ref_slice
        // needs to be inferred, in the latest community code, ref_slice is
        // recorded directly in the variable.
        /*
        if (param.copy_fields_and_items_in_materialize) {
          if (ref_slice == REF_SLICE_SAVED_BASE) {
            ref_slice = REF_SLICE_TMP1;
          } else if (ref_slice == REF_SLICE_TMP1) {
            ref_slice = REF_SLICE_TMP2;
          } else {
            assert(false);
          }
        }
        */
        if (parallel_scan) {
          exchange_safe = compat_for_table_fields(param.table);
          if (exchange_safe) {
             max_px_subpath = path;
          }
        }

      } else {
        parallel_safe = false;
      }
      break;
    }
    case AccessPath::MATERIALIZE: {
      const auto &param = path->materialize();

      TABLE_LIST *tl = param.param->table->pos_in_table_list;
      if ((tl && tl->is_view()) || param.param->cte) {
        parallel_safe = false;
        break;
      }

      bool is_child_stream = true;
      // Check table path firstly.
      parallel_safe = WalkAccessPathsForCompat(
          thd, param.table_path, path, cur_join, /*parallel_scan=*/false, false,
          false, ref_slice, max_px_subpath, is_child_stream, mat_access_path,
          split_positions, exchange_safe);
      // Parallel scheduling of the union plan requires at least one exchange
      // insertion point both on the union and on the branch of the union, and
      // the union operator must be the root of the entire plan tree.
      if (param.param->query_blocks.size() > 1 &&  // 1. UNION
          root_all &&  // 2. Whether the operator is the root node.
          parallel_safe &&
          exchange_safe) {  // 3. One exchange insertion point on the union
        bool child_exchange_safe = false;
        // Note: MATERIALIZE operator for union is executed serially by one
        // worker thread, so the operator itself is always parallel_safe.
        parallel_safe = true;
        for (auto qb : param.param->query_blocks) {
          uint child_ref_slice = REF_SLICE_SAVED_BASE;
          (void)WalkAccessPathsForCompat(thd, qb.subquery_path, path, qb.join,
                                         parallel_scan, /*root=*/true,
                                         /*root_all=*/false, child_ref_slice,
                                         max_px_subpath, is_child_stream,
                                         mat_access_path, split_positions,
                                         child_exchange_safe);
        }
        // Limit table size may truncate parallel scans.
        if (parallel_scan && is_child_stream &&
            param.param->limit_rows != HA_POS_ERROR) {
          split_positions->clear();
          parallel_safe = false;
        }
        if (param.param->ref_slice != -1) {
          ref_slice = param.param->ref_slice;
        } else {
          ref_slice = REF_SLICE_SAVED_BASE;
        }

        if (parallel_scan && parallel_safe && exchange_safe &&
            count_exchange_in_split_pos(split_positions)) {
          max_px_subpath = path;
          split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                        false, false, true, nullptr, nullptr);
          mat_access_path->push_back(path);
        } else {
          parallel_safe = false;
          if (split_positions && split_positions->size()) {
            split_positions->clear();
          }
        }
      } else if (param.param->query_blocks.size() == 1) {
        // Derived table or deduplication in query block.
        bool is_derived = (tl && tl->is_derived());
        if (root_all && is_derived) {
          root_all = false;
        }

        // Compatibility check for children Query Blocks.
        size_t split_size_pre = 0;
        if (split_positions) split_size_pre = split_positions->size();
        parallel_safe = true;
        bool child_exchange_safe = false;
        for (auto qb : param.param->query_blocks) {
          if (is_derived && !qb.join) {
            // Join is null in derived table means it is a union. See
            // GetAccessPathForDerivedTable()
            parallel_safe = false;
            break;
          }
          parallel_safe &=
              (WalkAccessPathsForCompat(
                   thd, qb.subquery_path, path, qb.join, parallel_scan,
                   /*root=*/is_derived, root_all, ref_slice, max_px_subpath,
                   is_child_stream, mat_access_path, split_positions,
                   child_exchange_safe) &&
               !check_px_unsafe_temp_param(qb.temp_table_param));
          if (!parallel_safe) break;
        }
        // Limit table size may truncate parallel scans.
        if (parallel_scan && is_child_stream &&
            param.param->limit_rows != HA_POS_ERROR) {
          split_positions->clear();
          parallel_safe = false;
        }
        // Compatibility check on the operator itself.
        if (parallel_safe && parallel_scan) {
          parallel_safe = !(is_derived || materialize_doing_deduplication(
                                              path, param.param->table));
        }
        if (param.param->ref_slice != -1) {
          ref_slice = param.param->ref_slice;
        } else {
          ref_slice = REF_SLICE_SAVED_BASE;
        }

        if (parallel_safe && parallel_scan) {
          if (exchange_safe) {
            max_px_subpath = path;
            split_positions->emplace_back(cur_join, path, parent, ref_slice,
                                          false, false, false, nullptr,
                                          nullptr);
          }
        }

        size_t split_size_post = 0;
        if (split_positions) split_size_post = split_positions->size();
        if (parallel_scan && is_derived && split_size_post > split_size_pre) {
          mat_access_path->push_back(path);
        }
      } else {
        parallel_safe = false;
      }
      is_stream = false;
      break;
    }
    case AccessPath::MATERIALIZE_INFORMATION_SCHEMA_TABLE: {
      parallel_safe = false;
      break;
    }
    case AccessPath::APPEND: {
      // Parallel scheduling of the union plan requires at least one exchange
      // insertion point both on the union and on the branch of the union, and
      // the union operator must be the root of the entire plan tree.
      if (parallel_scan &&
          (!root_all ||
           cur_join->query_expression()
               ->m_union_needs_tmp_table)) {  // needs_tmp_table means Both
                                              // UNION and UNION_ALL are used in
                                              // the statement
        parallel_safe = false;
        break;
      }
      const auto &param = path->append();
      exchange_safe = true;
      parallel_safe = true;
      bool is_child_stream =
          false;  // Not used. Even if the child is not streaming, treat it as
                  // streaming, avoiding the effects of multiple branches.
      for (AppendPathParameters child : *param.children) {
        (void)WalkAccessPathsForCompat(
            thd, child.path, path, child.join, parallel_scan, false, false,
            ref_slice, max_px_subpath, is_child_stream, mat_access_path,
            split_positions, exchange_safe);
        // If exchange can not be inserted here, Parallel scheduling does not
        // support union statements.
        if (!exchange_safe) {
          parallel_safe = false;
          split_positions->clear();
          break;
        }
      }
      ref_slice = REF_SLICE_SAVED_BASE;
      // Note: APPEND operator for union is executed serially by one worker
      // thread, so it is always parallel_safe.
      if (parallel_scan && root_all && parallel_safe && exchange_safe &&
          count_exchange_in_split_pos(split_positions)) {
        max_px_subpath = path;
        split_positions->emplace_back(cur_join, path, parent, ref_slice, false,
                                      false, true, nullptr, nullptr);
        mat_access_path->push_back(path);
      } else {
        parallel_safe = false;
        if (split_positions && split_positions->size()) {
          split_positions->clear();
        }
      }
      is_stream = true;
      break;
    }
    case AccessPath::WINDOW: {
      // Fix connection to windowing is NOT supported now.
      if (!parallel_scan) {
        parallel_safe = WalkAccessPathsForCompat(
            thd, path->window().child, path, cur_join, parallel_scan, false,
            false, ref_slice, max_px_subpath, is_stream, mat_access_path,
            split_positions, exchange_safe);
      }
      is_stream = false;
      if (path->window().ref_slice != -1)
        ref_slice = path->window().ref_slice;
      break;
    }
    case AccessPath::WEEDOUT: {
      // Get rowid from handler when read records to deduplicate, no value is
      // obtained during parallel query execution.
      if (!parallel_scan) {
        parallel_safe = WalkAccessPathsForCompat(
            thd, path->weedout().child, path, cur_join, parallel_scan, false,
            false, ref_slice, max_px_subpath, is_stream, mat_access_path,
            split_positions, exchange_safe);
      }
      is_stream = false;
      break;
    }
    case AccessPath::REMOVE_DUPLICATES: {
      // Deduplicate by ordered input (usually using index)
      if (!parallel_scan) {
        parallel_safe = WalkAccessPathsForCompat(
            thd, path->remove_duplicates().child, path, cur_join, parallel_scan,
            false, false, ref_slice, max_px_subpath, is_stream, mat_access_path,
            split_positions, exchange_safe);
      }
      break;
    }
    case AccessPath::ALTERNATIVE: {
      parallel_safe = false;
      break;
    }
    case AccessPath::CACHE_INVALIDATOR: {
      parallel_safe = false;
      break;
    }
    default:
      break;
  }

  // Compatibility check for projector of join.
  if (root && parallel_safe && cur_join) {
    parallel_safe = !check_px_unsafe_projector(cur_join);
  }

  if (!thd->lex->pass_px_check && split_positions && split_positions->size()) {
    parallel_safe = false;
    split_positions->clear();
  }

  return parallel_safe;
}

/**
 * Choose the best insertion points from all possible insertion points.
 * Currently the insertion points is chosen as close to the root node as
 * possible based on rules.
 *
 * @param thd
 * @param split_positions_in
 * @param[out] split_positions
 * @return true
 * @return false
 */
bool px_access_path::FindExchangeInjectPosition(
    THD *thd, std::vector<Split_Position> *const split_positions_in,
    std::vector<Split_Position> *const split_positions,
    std::list<QEP_TAB *> *const parallel_tab) {
  JOIN *cur_join = nullptr;
  QEP_TAB *cur_parallel_tab = nullptr;
  Split_Position *cur_split_pos = nullptr;

  for (Split_Position &split_pos : *split_positions_in) {
    // New query block or union
    if (!cur_join || cur_join != split_pos.m_join || split_pos.m_split_union) {
      if (cur_split_pos) {
        split_positions->push_back(*cur_split_pos);
        if (cur_parallel_tab) {
          parallel_tab->push_back(cur_parallel_tab);
        }
      }
      if (split_pos.m_parallel_tab) {
        // Special node only contains parallel_tab.
        cur_parallel_tab = split_pos.m_parallel_tab;
        cur_split_pos = nullptr;
      } else {
        cur_parallel_tab = nullptr;
        cur_split_pos = &split_pos;
      }
      cur_join = split_pos.m_join;
      continue;
    }

    // There is at most one parallel tab in a query block.
    assert(!split_pos.m_parallel_tab);

    if (!cur_split_pos) {
      assert(cur_join == split_pos.m_join);
      cur_split_pos = &split_pos;
      continue;
    }

    // Split aggregation
    if (split_pos.m_split_agg &&
        split_pos.m_target->type == AccessPath::AGGREGATE) {
      if (cur_split_pos->m_split_sort &&
          (!cur_split_pos->m_filesort ||  // order by index
           group_field_eq_sort_list(      // order by filesort
               cur_join->group_fields, cur_split_pos->m_filesort->m_order))) {
        // If there is no filesort, index is used. Index ordering is used only
        // if the index supports ordering of all group columns. See
        // JOIN::test_skip_sort() and test_if_skip_sort_order().

        // Split sort & agg.
        split_pos.m_split_sort = true;
        split_pos.m_filesort = cur_split_pos->m_filesort;
        cur_split_pos = &split_pos;
        continue;
      }
    }

    if (!cur_split_pos->m_split_agg && !cur_split_pos->m_split_sort) {
      cur_split_pos = &split_pos;
      continue;
    }
  }
  if (cur_split_pos) {
    split_positions->push_back(*cur_split_pos);
    if (cur_parallel_tab) {
      parallel_tab->push_back(cur_parallel_tab);
    }
  }

  return false;
}

size_t px_access_path::count_exchange_in_split_pos(
    std::vector<Split_Position> *const split_positions) {
  size_t count = 0;
  for (const Split_Position &split_pos : *split_positions) {
    if (!split_pos.m_parallel_tab) { ++count; }
  }
  return count;
}

#ifndef DBUG_OFF
void px_access_path::PrintSplitPostion(
    const char *str, std::vector<Split_Position> *const split_positions) {
  for (Split_Position &split_pos : *split_positions) {
    if (!split_pos.m_parallel_tab) {
      PX_PRINT_INFO("%s SPLIT POSITION: [%d]", str, split_pos.m_target->type);
    }
  }
}
#endif

bool AccessPath::operator==(const AccessPath &other) const {
  if (type != other.type) {
    return false;
  }
  switch (type) {
    case TABLE_SCAN: {
      // equivalence check: same TABLE_SHARE
      return EquivalenceCheckHelper::eq_table_share(
          u.table_scan.table, other.table_scan().table);
      break;
    }
    case INDEX_SCAN: {
      // equivalence check: same INDEX.idx and reverse
      if (u.index_scan.idx != other.index_scan().idx ||
          u.index_scan.use_order != other.index_scan().use_order ||
          u.index_scan.reverse != other.index_scan().reverse) {
        return false;
      }
      return EquivalenceCheckHelper::eq_table_share_without_icp(
          u.index_scan.table, other.index_scan().table);
      break;
    }
    case REF: {
      // equivalence check: same TABLE_SHARE, ref, and idx_cond
      if (!EquivalenceCheckHelper::eq_table_share(u.ref.table,
              other.ref().table) ||
          !EquivalenceCheckHelper::eq_table_ref(u.ref.ref, other.ref().ref) ||
          !EquivalenceCheckHelper::eq_item(u.ref.table->file->pushed_idx_cond,
              other.ref().table->file->pushed_idx_cond)) {
        return false;
      }
      if (u.ref.use_order != other.ref().use_order ||
          u.ref.reverse != other.ref().reverse) {
        return false;
      }
      break;
    }
    case REF_OR_NULL: {
      // equivalence check: same TABLE_SHARE, ref, and idx_cond
      if (!EquivalenceCheckHelper::eq_table_share(u.ref_or_null.table,
              other.ref_or_null().table) ||
          !EquivalenceCheckHelper::eq_table_ref(u.ref_or_null.ref,
              other.ref_or_null().ref) ||
          !EquivalenceCheckHelper::eq_item(u.ref_or_null.table->file->pushed_idx_cond,
              other.ref_or_null().table->file->pushed_idx_cond)) {
        return false;
      }
      if (u.ref_or_null.use_order != other.ref_or_null().use_order) {
        return false;
      }
      break;
    }
    case EQ_REF: {
      // equivalence check: same TABLE_SHARE, ref, and idx_cond
      if (!EquivalenceCheckHelper::eq_table_share(u.eq_ref.table,
              other.eq_ref().table) ||
          !EquivalenceCheckHelper::eq_table_ref(u.eq_ref.ref,
              other.eq_ref().ref) ||
          !EquivalenceCheckHelper::eq_item(u.eq_ref.table->file->pushed_idx_cond,
              other.eq_ref().table->file->pushed_idx_cond)) {
        return false;
      }
      if (u.eq_ref.use_order != other.eq_ref().use_order) {
        return false;
      }
      break;
    }
    case PUSHED_JOIN_REF: {
      // equivalence check: same TABLE_SHARE and ref
      if (!EquivalenceCheckHelper::eq_table_share_without_icp(
              u.pushed_join_ref.table,
              other.pushed_join_ref().table) ||
          !EquivalenceCheckHelper::eq_table_ref(u.pushed_join_ref.ref,
              other.pushed_join_ref().ref)) {
        return false;
      }
      if (u.pushed_join_ref.use_order != other.pushed_join_ref().use_order ||
          u.pushed_join_ref.is_unique != other.pushed_join_ref().is_unique) {
        return false;
      }
      break;
    }
    case FULL_TEXT_SEARCH: {
      // equivalence check: same TABLE_SHARE and ref
      if (!EquivalenceCheckHelper::eq_table_share_without_icp(
              u.full_text_search.table,
              other.full_text_search().table) ||
          !EquivalenceCheckHelper::eq_table_ref(u.full_text_search.ref,
              other.full_text_search().ref)) {
        return false;
      }
      if (u.full_text_search.use_order != other.full_text_search().use_order) {
        return false;
      }
      break;
    }
    case CONST_TABLE: {
      // TODO: check item? (plan cache)
      // equivalence check: same TABLE_SHARE and ref
      if (!EquivalenceCheckHelper::eq_table_share_without_icp(
              u.const_table.table,
              other.const_table().table) ||
          !EquivalenceCheckHelper::eq_table_ref(u.const_table.ref,
              other.const_table().ref)) {
        return false;
      }
      TABLE *table = u.const_table.table;
      assert(table->file->pushed_cond == nullptr);
      TABLE *coordinator_table = other.const_table().table;
      assert(coordinator_table->file->pushed_cond == nullptr);
      break;
    }
    case MRR: {
      // equivalence check: same TABLE_SHARE, ref and idx_cond
      if (!EquivalenceCheckHelper::eq_table_share(
              u.mrr.table,
              other.mrr().table) ||
          !EquivalenceCheckHelper::eq_table_ref(u.mrr.ref,
              other.mrr().ref) ||
          !EquivalenceCheckHelper::eq_item(u.mrr.table->file->pushed_idx_cond,
              other.mrr().table->file->pushed_idx_cond)) {
        return false;
      }
      if (u.mrr.mrr_flags != other.mrr().mrr_flags ||
          u.mrr.keep_current_rowid != other.mrr().keep_current_rowid) {
        return false;
      }
      break;
    }
    case FOLLOW_TAIL: {
      // equivalence check: same TABLE_SHARE
      if (!EquivalenceCheckHelper::eq_table_share(
              u.follow_tail.table,
              other.follow_tail().table)) {
        return false;
      }
      break;
    }
    case INDEX_RANGE_SCAN: {
      // equivalence check: same TABLE_SHARE, quick, idx_cond and QUICK_SELECT_I
      if (!EquivalenceCheckHelper::eq_table_share(
              u.index_range_scan.used_key_part[0].field->table,
              other.index_range_scan().used_key_part[0].field->table) ||
          !EquivalenceCheckHelper::eq_item(
              u.index_range_scan.used_key_part[0].field->table->file->pushed_idx_cond,
              other.index_range_scan().used_key_part[0].field->table->file->pushed_idx_cond)) {
        return false;
      }
      break;
    }
    case DYNAMIC_INDEX_RANGE_SCAN: {
      // equivalence check: same TABLE_SHARE and idx_cond
      if (!EquivalenceCheckHelper::eq_table_share(
              u.dynamic_index_range_scan.table,
              other.dynamic_index_range_scan().table) ||
          !EquivalenceCheckHelper::eq_item(u.dynamic_index_range_scan.table->file->pushed_idx_cond,
              other.dynamic_index_range_scan().table->file->pushed_idx_cond)) {
        return false;
      }
      break;
    }
    case TABLE_VALUE_CONSTRUCTOR: {
      return true;
    }
    case FAKE_SINGLE_ROW: {
      return true;
    }
    case ZERO_ROWS: {
      // TODO: worker_children.push_back(
      // equivalence check: same cause
      if (strlen(u.zero_rows.cause) != strlen(other.zero_rows().cause) ||
          strncmp(u.zero_rows.cause, other.zero_rows().cause,
                  strlen(u.zero_rows.cause)) != 0) {
        return false;
      }
      break;
    }
    case ZERO_ROWS_AGGREGATED: {
      // equivalence check: same cause
      if (strlen(u.zero_rows_aggregated.cause) !=
            strlen(other.zero_rows_aggregated().cause) ||
          strncmp(u.zero_rows_aggregated.cause, other.zero_rows_aggregated().cause,
                  strlen(u.zero_rows_aggregated.cause)) != 0) {
        return false;
      }
      break;
    }
    case MATERIALIZED_TABLE_FUNCTION: {
      // equivalence check: same TABLE_SHARE and Table_function
      if (!EquivalenceCheckHelper::eq_table_share(
              u.materialized_table_function.table,
              other.materialized_table_function().table) ||
          !EquivalenceCheckHelper::eq_table_share(
              u.materialized_table_function.table_function->get_table(),
              other.materialized_table_function().table_function->get_table())) {
        return false;
      }
      break;
    }
    case NESTED_LOOP_JOIN: {
      // equivalence check: same join_type
      if (u.nested_loop_join.join_type != other.nested_loop_join().join_type ||
          u.nested_loop_join.pfs_batch_mode != other.nested_loop_join().pfs_batch_mode) {
        return false;
      }
      break;
    }
    case NESTED_LOOP_SEMIJOIN_WITH_DUPLICATE_REMOVAL: {
      // equivalence check: same TABLE_SHARE and key.name
      if (!EquivalenceCheckHelper::eq_table_share(
              u.nested_loop_semijoin_with_duplicate_removal.table,
              other.nested_loop_semijoin_with_duplicate_removal().table) ||
          !EquivalenceCheckHelper::eq_table_share(
              u.nested_loop_semijoin_with_duplicate_removal.key->table,
              other.nested_loop_semijoin_with_duplicate_removal().key->table)) {
        return false;
      }
      if (u.nested_loop_semijoin_with_duplicate_removal.key_len !=
              other.nested_loop_semijoin_with_duplicate_removal().key_len ||
          strlen(u.nested_loop_semijoin_with_duplicate_removal.key->name) !=
              strlen(other.nested_loop_semijoin_with_duplicate_removal().key->name) ||
          strcmp(u.nested_loop_semijoin_with_duplicate_removal.key->name,
              other.nested_loop_semijoin_with_duplicate_removal().key->name) != 0) {
        return false;
      }
      break;
    }
    case BKA_JOIN: {
      if (u.bka_join.join_type != other.bka_join().join_type ||
          u.bka_join.rec_per_key != other.bka_join().rec_per_key ||
          u.bka_join.store_rowids != other.bka_join().store_rowids ||
          u.bka_join.tables_to_get_rowid_for != other.bka_join().tables_to_get_rowid_for ||
          u.bka_join.mrr_length_per_rec != other.bka_join().mrr_length_per_rec) {
        return false;
      }
      break;
    }
    case HASH_JOIN: {
      // equivalence check: same JoinPredicate
      //const JoinPredicate *join_predicate = u.hash_join.join_predicate;
      //const JoinPredicate *other_join_predicate = other.hash_join().join_predicate;
      if (/*!join_predicate->eq(other_join_predicate) ||*/
          u.hash_join.store_rowids != other.hash_join().store_rowids ||
          u.hash_join.tables_to_get_rowid_for != other.hash_join().tables_to_get_rowid_for ||
          u.hash_join.allow_spill_to_disk != other.hash_join().allow_spill_to_disk) {
        return false;
      }
      break;
    }
    case FILTER: {
      // equivalence check: cond
      if (!EquivalenceCheckHelper::eq_item(u.filter.condition,
              other.filter().condition)) {
        return false;
      }
      break;
    }
    case SORT: {
      // equivalence check: same Filesort
      if (u.sort.tables_to_get_rowid_for != other.sort().tables_to_get_rowid_for ||
          u.sort.filesort->using_addon_fields() != other.sort().filesort->using_addon_fields() ||
          u.sort.filesort->m_remove_duplicates != other.sort().filesort->m_remove_duplicates ||
          u.sort.filesort->limit != other.sort().filesort->limit ||
          u.sort.filesort->sort_order_length() != other.sort().filesort->sort_order_length()) {
        return false;
      }
      for (unsigned i = 0; i < u.sort.filesort->sort_order_length();
          ++i) {
        const st_sort_field *order = &u.sort.filesort->sortorder[i];
        const st_sort_field *other_order = &other.sort().filesort->sortorder[i];
        if (order->reverse != other_order->reverse ||
            !order->item->eq(other_order->item, true)) {
          return false;
        }
      }
      break;
    }
    case AGGREGATE: {
      // equivalence check: AccessPath
      if (u.aggregate.rollup != other.aggregate().rollup ||
          u.aggregate.px_agg_type != other.aggregate().px_agg_type) {
        return false;
      }
      break;
    }
    case TEMPTABLE_AGGREGATE: {
      // equivalence check: AccessPath
      // Note the table_name/path of temptable (u.temptable_aggregate.table)
      // is different from coordinate, thus the equivalence check need rely
      // on sub-path in worker_table_explain.children.
      if (u.temptable_aggregate.px_agg_type != other.temptable_aggregate().px_agg_type ||
          u.temptable_aggregate.ref_slice != other.temptable_aggregate().ref_slice) {
        return false;
      }
      break;
    }
    case LIMIT_OFFSET: {
      if (u.limit_offset.offset != other.limit_offset().offset ||
          u.limit_offset.reject_multiple_rows != other.limit_offset().reject_multiple_rows ||
          u.limit_offset.count_all_rows != other.limit_offset().count_all_rows ||
          u.limit_offset.limit != other.limit_offset().limit) {
        return false;
      }
      if (u.limit_offset.send_records_override != nullptr) {
        if (other.limit_offset().send_records_override == nullptr) {
          return false;
        }
        return true;
      }
      if (other.limit_offset().send_records_override != nullptr) {
        return false;
      }
      break;
    }
    case REMOVE_DUPLICATES: {
      // equivalence check: same TABLE_SHARE and key.name
      /*
      if (u.remove_duplicates.loosescan_key_len !=
              other.remove_duplicates().loosescan_key_len ||
          strlen(u.remove_duplicates.key->name) !=
              strlen(other.remove_duplicates().key->name) ||
          strcmp(u.remove_duplicates.key->name,
              other.remove_duplicates().key->name) != 0) {
        return false;
      }
      */
      break;
    }
    case ALTERNATIVE: {
      // equivalence check: same TABLE_SHARE, ref, and cond_guards(field)
      if (!EquivalenceCheckHelper::eq_table_share(
              u.alternative.table_scan_path->table_scan().table,
              other.alternative().table_scan_path->table_scan().table)) {
        if (!u.alternative.table_scan_path->table_scan().table->s ||
            !other.alternative().table_scan_path->table_scan().table->s) {
          return false;
        }
        if (!EquivalenceCheckHelper::eq_lex_string(
              &(u.alternative.table_scan_path->table_scan().table->s->db),
              &(other.alternative().table_scan_path->table_scan().table->s->db)) ||
            !EquivalenceCheckHelper::eq_lex_string(
              &(u.alternative.table_scan_path->table_scan().table->s->table_name),
              &(other.alternative().table_scan_path->table_scan().table->s->table_name))) {
          return false;
        }
      }
      if (!EquivalenceCheckHelper::eq_table_ref(u.alternative.used_ref,
            other.alternative().used_ref)) {
        return false;
      }
      break;
    }
    case MATERIALIZE: {
      // equivalence check: check call MaterializePathParameters ExplainMaterializeAccessPath
      if (!u.materialize.param->eq(other.materialize().param)) {
        return false;
      }
      break;
    }
    case APPEND: {
      break;
    }
    case WINDOW: {
      break;
    }
    case UNQUALIFIED_COUNT: {
      break;
    }
    case WEEDOUT: {
      // equivalence check: SJ_TMP_TABLE
      if (u.weedout.tables_to_get_rowid_for != other.weedout().tables_to_get_rowid_for) {
        return false;
      }
      SJ_TMP_TABLE *sj = u.weedout.weedout_table;
      SJ_TMP_TABLE *other_sj = other.weedout().weedout_table;
      if (sj->tabs_end == sj->tabs + 1) {
        if (other_sj->tabs_end != other_sj->tabs + 1 ||
            !EquivalenceCheckHelper::eq_table_share(
                sj->tabs->qep_tab->table(),
                other_sj->tabs->qep_tab->table())) {
          return false;
        }
      } else {
        SJ_TMP_TABLE_TAB *other_tab = other_sj->tabs;
        for (SJ_TMP_TABLE_TAB *tab = sj->tabs; tab != sj->tabs_end; ++tab, other_tab++) {
          if (other_tab == nullptr ||
              !EquivalenceCheckHelper::eq_table_share(
                  tab->qep_tab->table(),
                  other_tab->qep_tab->table())) {
            return false;
          }
        }
        if (other_tab != other_sj->tabs_end) {
          return false;
        }
      }
      break;
    }
    case CACHE_INVALIDATOR: {
      if (strlen(u.cache_invalidator.name) !=
              strlen(other.cache_invalidator().name) ||
          strcmp(u.cache_invalidator.name,
              other.cache_invalidator().name) != 0) {
        return false;
      }
      break;
    }
    case PX_RECEIVER_MERGE:
    case PX_RECEIVE:
    case PX_SEND: {
      return true;
    }
    case STREAM: {
      if (/*u.stream.copy_fields_and_items_in_materialize !=
              other.stream().copy_fields_and_items_in_materialize ||*/
          u.stream.provide_rowid !=
              other.stream().provide_rowid ||
          !EquivalenceCheckHelper::eq_table_share(
                  u.stream.table,
                  other.stream().table)) {
        return false;
      }

      Temp_table_param * temp_table_param = u.stream.temp_table_param;
      Temp_table_param * other_temp_table_param = other.stream().temp_table_param; 
      if (temp_table_param) {
        if (!temp_table_param->eq(other_temp_table_param)) {
          return false;
        }
        return true;
      }
      if (other_temp_table_param) {
        return false;
      }
      return true;
    }
    case MATERIALIZE_INFORMATION_SCHEMA_TABLE: {
      if (strlen(u.materialize_information_schema_table.table_list->table->alias) !=
              strlen(other.materialize_information_schema_table().table_list->table->alias) ||
          strcmp(u.materialize_information_schema_table.table_list->table->alias,
              other.materialize_information_schema_table().table_list->table->alias) != 0) {
        return false;
      }
      if (!EquivalenceCheckHelper::eq_item(u.materialize_information_schema_table.condition,
              other.materialize_information_schema_table().condition)) {
        return false;
      }
      return true;
    }
    default:
      return false; // not supported
      break;
  }
  return true;
}

void GetExchangeTables(px_access_path::Split_Position *split_pos) {
  assert(split_pos && split_pos->m_tables);
  auto find_tables = [split_pos](AccessPath *subpath, const JOIN *) {
    switch (subpath->type) {
      case AccessPath::TABLE_SCAN:
        split_pos->m_tables->push_back(subpath->table_scan().table);
        return false;
      case AccessPath::INDEX_SCAN:
        split_pos->m_tables->push_back(subpath->index_scan().table);
        return false;
      case AccessPath::INDEX_RANGE_SCAN:
        split_pos->m_tables->push_back(subpath->index_range_scan().used_key_part[0].field->table);
        return false;
      case AccessPath::REF:
        split_pos->m_tables->push_back(subpath->ref().table);
        return false;
      case AccessPath::FILTER:
        return false;
      case AccessPath::NESTED_LOOP_JOIN:
        return false;
      case AccessPath::REF_OR_NULL:
        split_pos->m_tables->push_back(subpath->ref_or_null().table);
        return false;
      case AccessPath::EQ_REF:
        split_pos->m_tables->push_back(subpath->eq_ref().table);
        return false;
      case AccessPath::PUSHED_JOIN_REF:
        return false;
      case AccessPath::CONST_TABLE:
        split_pos->m_tables->push_back(subpath->const_table().table);
        return false;
      case AccessPath::STREAM:
        split_pos->m_tables->push_back(subpath->stream().table);
        return true;
      case AccessPath::MATERIALIZE:
        split_pos->m_tables->push_back(subpath->materialize().param->table);
        return true;
      case AccessPath::LIMIT_OFFSET:
        return false;
      case AccessPath::SORT:
        return false;
      case AccessPath::AGGREGATE: {
        TABLE *table = split_pos->m_join->aggr_tmp_table;
        assert(table);
        split_pos->m_tables->push_back(table);
        return true;
      }
      case AccessPath::TEMPTABLE_AGGREGATE:
        split_pos->m_tables->push_back(subpath->temptable_aggregate().table);
        return true;
      case AccessPath::WEEDOUT:
        return false;
      case AccessPath::REMOVE_DUPLICATES:
        return false;
      default:
        return true;
    }
  };

  WalkAccessPaths(split_pos->m_target, /*join=*/nullptr,
                  WalkAccessPathPolicy::STOP_AT_MATERIALIZATION, find_tables);
}

static AccessPath *CreateExchangeAccessPath(
    THD *thd, JOIN *join, AccessPath *const path,
    mem_root_deque<Item *> &tmp_table_fields,
    px_access_path::Split_Position *split_pos, bool save_sum_fields,
    uint curr_exchange, bool alloc_group_field);

static AccessPath *CreateExchangeAccessPathUseTables(
    THD *thd, JOIN *join, AccessPath *const path,
    mem_root_deque<TABLE *> *tables, int ref_slice,
    px_access_path::Split_Position *split_pos);

static void FixAccessPathForExchange(AccessPath *const path,
                                     AccessPath *receiver, JOIN *join,
                                     bool do_merge_sort);

static void GetExchangeParam(ReceiverParam *receiver_param, AccessPath *receiver,
                             bool do_merge_sort);

/**
 * Walk through access path to inject exchange.
 * (1) Create exchange up to this path using CreateExchangeAccessPath(), the new
 *     exchange access path will be the parent of this path.
 * (2) Walk through the child of this path. If the child is injected exchange,
 *     set `exchange_following` as child (see `new_child`). If the following
 *     exchange has influence to this path, call FixAccessPathForExchange() to
 *     fix the item.
 *
 * @param[out] new_child  Set to true if exchange was inject successfully 
 *                        ahead of this path, old parent should set child to the
 *                        exchange.
 * 
 * @param[in]  sort_parent True if parent access path is sort, this may cause
 *                         some problem, exchange behind sort must be
 *                         materialized(TODO)
 * 
 * @return exchange which has influence to the parent of this path or nullptr
 */

/**
 * Traverse access path to inject exchange.
 * (1) Create exchange up to target_path using CreateExchangeAccessPath(), the
 * new exchange access path will be the parent of this path.
 *
 * (2) If the child is injected exchange, call FixAccessPathForExchange() to
 * fix the data flow from exchange to this access path. If the exchange has
 * influence to the parent this path, return the exchange to parent.
 *
 * @param thd
 * @param join
 * @param path          current exchange
 * @param target_path   access path to be injected exchange
 * @param curr_exchange index for current exchange
 * @param new_child     true if exchange injected below
 * @param cur_slice     current ref_slice in join
 * @param alloc_group_field  need to refine group by list for aggregate.
 * @param in_join       exchange injected below join can only use field mode.
 * @return exchange if injected, nullptr otherwise.
 */
AccessPath *WalkAccessPathsForExchange(THD *thd, JOIN *join,
                                       px_access_path::Split_Position *split_pos,
                                       AccessPath *const path,
                                       AccessPath *const target_path,
                                       uint curr_exchange, bool &new_child,
                                       int cur_slice, bool alloc_group_field,
                                       bool in_join) {
  assert(target_path);

  bool return_follow = false;

  // Invalidate the previous rule and reset the insert rule
  bool inject_here = (target_path == path);

  if(curr_exchange >= MAX_EXCHANGE_NUM) return nullptr;

  bool use_tmp_table = false;  // if true, create temporary table.
  TABLE *table = nullptr;

  List_item *fields = nullptr;
  bool save_sum_func = false;

  AccessPath *child = nullptr;
  int child_slice = -1;
  bool alloc_child_group_field = false;

  bool child_in_join = in_join;

  // If receiver don`t use temporary table, it should set ref slice for child.
  int output_slice = -1;

  switch (path->type) {
    case AccessPath::TABLE_SCAN:
    case AccessPath::INDEX_SCAN:
    case AccessPath::INDEX_RANGE_SCAN:
    case AccessPath::REF: {
      if (!in_join &&
          DBUG_EVALUATE_IF("exchange_inject_use_item", true, false)) {
        use_tmp_table = true;
        int ref_slice = join->ref_items[REF_SLICE_SAVED_BASE].is_null()
                            ? 0
                            : REF_SLICE_SAVED_BASE;
        fields = ref_slice ? &join->tmp_fields[ref_slice] : join->fields;
      } else {
        use_tmp_table = false;
      }

      switch(path->type) {
        case AccessPath::TABLE_SCAN: {
          table = path->table_scan().table;
          break;
        }
        case AccessPath::INDEX_SCAN: {
          table = path->index_scan().table;
          break;
        }
        case AccessPath::INDEX_RANGE_SCAN: {
          const auto &param = path->index_range_scan();
          table = param.used_key_part[0].field->table;
          break;
        }
        case AccessPath::REF: {
          table = path->ref().table;
          break;
        }
        default:
          assert(false);
      }
      break;
    }
    case AccessPath::NESTED_LOOP_JOIN: {
      use_tmp_table = false;

      if (!in_join &&
          DBUG_EVALUATE_IF("exchange_inject_use_item", true, false)) {
        use_tmp_table = true;
        fields = join->ref_items[REF_SLICE_SAVED_BASE].is_null()
                    ? join->fields
                    : &join->tmp_fields[REF_SLICE_SAVED_BASE];
      }

      // Only the first non-const table in join can be parallelize table now, if
      // the outer table is const table, the inner table can be parallelize
      // table as its join condition is const.
      bool left_const =
          (path->nested_loop_join().outer->type == AccessPath::FAKE_SINGLE_ROW);
      if (!left_const) {
        child = path->nested_loop_join().outer;
      } else {
        child = path->nested_loop_join().inner;
      }
      child_slice = REF_SLICE_SAVED_BASE;
      child_in_join = true;
      break;
    }
    case AccessPath::HASH_JOIN: {
      assert(!inject_here);

      child = path->hash_join().inner;
      child_slice = REF_SLICE_SAVED_BASE;
      child_in_join = true;
      break;
    }
    case AccessPath::FILTER: {
      child = path->filter().child;
      if (cur_slice == -1) {
        /*
        cur_slice = join->ref_items[REF_SLICE_TMP1].is_null()
                        ? join->ref_items[REF_SLICE_ORDERED_GROUP_BY].is_null()
                              ? REF_SLICE_SAVED_BASE
                              : REF_SLICE_ORDERED_GROUP_BY
                        : REF_SLICE_TMP1;
        */
        cur_slice = join->ref_items[REF_SLICE_TMP1].is_null()
                        ? REF_SLICE_SAVED_BASE : REF_SLICE_TMP1;
      }
      if (split_pos->m_tables) use_tmp_table = false;

      uint ref_slice = cur_slice;
      if (!in_join &&
          DBUG_EVALUATE_IF("exchange_inject_use_item", true, false)) {
        use_tmp_table = true;
      }

      if (use_tmp_table) {
        if (ref_slice == 0) {
          fields = join->fields;
        } else {
          fields = &join->tmp_fields[ref_slice];
          /*
          if (ref_slice == REF_SLICE_ORDERED_GROUP_BY) {
            save_sum_func = true;
          }
          */
        }
      }

      child_slice = ref_slice;
      break;
    }
    case AccessPath::SORT: {
      child = path->sort().child;
      Filesort *filesort = path->sort().filesort;
      if (filesort->tables.size() == 1) {
        use_tmp_table = false;
        table = filesort->tables[0];
      }

      if (cur_slice == -1) {
        cur_slice = join->ref_items[REF_SLICE_TMP2].is_null()
                        ? join->ref_items[REF_SLICE_TMP1].is_null()
                              ? join->ref_items[REF_SLICE_SAVED_BASE].is_null()
                                    ? 0
                                    : REF_SLICE_SAVED_BASE
                              : REF_SLICE_TMP1
                        : REF_SLICE_TMP2;
      }

      if (use_tmp_table) {
        fields = cur_slice ? &join->tmp_fields[cur_slice] : join->fields;
      }

      output_slice = cur_slice ? cur_slice : -1;
      child_slice = cur_slice;
      return_follow = true;

      break;
    }
    case AccessPath::AGGREGATE: {
      // Tmp table has been created in the stage of split
      use_tmp_table = false;
      table = join->aggr_tmp_table;
      //output_slice = path->aggregate().output_slice;

      child = path->aggregate().child;
      /*
      if (output_slice == REF_SLICE_ORDERED_GROUP_BY) {
        if (!join->ref_items[REF_SLICE_TMP2].is_null()) {
          child_slice = REF_SLICE_TMP2;
        } else if (!join->ref_items[REF_SLICE_TMP1].is_null()) {
          child_slice = REF_SLICE_TMP1;
        } else {
          child_slice = REF_SLICE_SAVED_BASE;
        }
      } else*/if (output_slice == REF_SLICE_TMP2) {
        child_slice = REF_SLICE_TMP1;
      } else if (output_slice == REF_SLICE_TMP1) {
        child_slice = REF_SLICE_SAVED_BASE;
      } else if (output_slice != REF_SLICE_FINAL_AGGREGATE) {
        assert(0);
      }

      break;
    }
    case AccessPath::TEMPTABLE_AGGREGATE: {
      // There is no item to be calculated, just use the tmp table of
      // TEMPTABLE_AGGREGATE
      use_tmp_table = false;
      table = path->temptable_aggregate().table;
      output_slice = path->temptable_aggregate().ref_slice;

      child = path->temptable_aggregate().subquery_path;
      if (output_slice == REF_SLICE_TMP2) {
        child_slice = REF_SLICE_TMP1;
      } else if (output_slice == REF_SLICE_TMP1) {
        child_slice = REF_SLICE_SAVED_BASE;
      } else if (output_slice != REF_SLICE_FINAL_AGGREGATE) {
        assert(0);
      }

      break;
    }
    case AccessPath::LIMIT_OFFSET: {
      child = path->limit_offset().child;
      return_follow = true;
      break;
    }
    case AccessPath::STREAM: {

      child = path->stream().child;
      child_slice = REF_SLICE_SAVED_BASE;

      break;
    }
    case AccessPath::MATERIALIZE: {
      const auto &param = path->materialize();
      if (param.param->query_blocks.size() != 1 ||
          param.param->query_blocks[0].join != join) {
        return nullptr;
      }

      child = param.param->query_blocks[0].subquery_path;
      if (param.param->ref_slice == REF_SLICE_TMP2) {
        child_slice = REF_SLICE_TMP1;
      } else if (param.param->ref_slice == REF_SLICE_TMP1) {
        child_slice = REF_SLICE_SAVED_BASE;
      }
      break;
    }
    case AccessPath::WEEDOUT: {
      assert(!inject_here);
      child = path->weedout().child;
      break;
    }

    default:
      return nullptr;
      // assert(false);
  }

  AccessPath *exchange = nullptr;

  const bool stop_walk =
      (inject_here || !child);  // We only need to inject one exchange.

  if (inject_here) {
      mem_root_deque<TABLE *> *&tables = split_pos->m_tables;

    if (use_tmp_table) {
      if (in_join) return nullptr;
      assert(fields != nullptr);
      exchange =
          CreateExchangeAccessPath(thd, join, path, *fields, split_pos,
                                   save_sum_func, curr_exchange,
                                   alloc_group_field);
      /*
        Tmp table created by exchange might use some type of fields that are not
        supported by Message Queue. In this case try to create exchange without
        new tmp table.
      */
      if (!exchange && (table || tables)) {
        use_tmp_table = false;
      }
    }
    if (!use_tmp_table) {
      if (!tables) {
        assert(table);
        tables = new (thd->mem_root) mem_root_deque<TABLE *>(thd->mem_root);
        if (!tables) return nullptr;
        tables->push_back(table);
      }
      assert(tables->size());
      // TODO: ref slice number saved in split position was tested to always be
      // equal to output_slice, possibly using split_pos in the future.
      // assert(output_slice <= 0 ||
      //             uint(output_slice) == split_pos->m_ref_slice);

      exchange = CreateExchangeAccessPathUseTables(
          thd, join, path, tables, output_slice,
          split_pos);
    }
    if (exchange) {
      thd->lex->m_exchange_number++;
      new_child = true;
    }
  }

  if (stop_walk) return exchange;

  AccessPath *exchange_following = nullptr;

  bool have_new_child = false;
  assert(child != nullptr);
  int next_exchange_slice = inject_here ? curr_exchange + 1 : curr_exchange;
  exchange_following = WalkAccessPathsForExchange(
      thd, join, split_pos, child, target_path, next_exchange_slice,
      have_new_child, child_slice, alloc_child_group_field, child_in_join);
  if (exchange_following) {
    FixAccessPathForExchange(path, exchange_following, join,
        split_pos->m_split_sort);
    if (have_new_child) {
      ConnectAccessPathWithChildExchange(path, exchange_following);
    }
  }

  return (return_follow ? exchange_following : exchange);
}

static void dbug_print_table(TABLE *table, const char *str, uint slice = -1);

static void rebuild_ref_item(THD *thd, JOIN *join, uint exchange_ref_slice);

/**
 * First create a temporary table for sender with tmp_table_fields, then
 * create a temporary table for receiver with item point at sender table.
 * 
 * The following access path should be fixed for the right item point at new
 * temporary table, we do that in FixAccessPathForExchange().
 *
 * @param thd 
 * @param join 
 * @param path 
 * @param tmp_table_fields 
 * @param save_sum_fields 
 * @param curr_exchange   The ref_slice that the items of exchange saved in.
 * @return The receiver if success, nullptr otherwise.
 */
static AccessPath *CreateExchangeAccessPath(
    THD *thd, JOIN *join, AccessPath *const path,
    mem_root_deque<Item *> &tmp_table_fields,
    px_access_path::Split_Position *split_pos,
    bool save_sum_fields, uint curr_exchange, bool alloc_group_field) {
  if (join->exchange_tmp_fields == nullptr) {
    join->exchange_tmp_fields =
        (*THR_MALLOC)
            ->ArrayAlloc<mem_root_deque<Item *>>(MAX_EXCHANGE_NUM, *THR_MALLOC);
    if (join->exchange_tmp_fields == nullptr) return nullptr;
  }
  AccessPath *sender = nullptr, *receiver = nullptr;
  TABLE *table = nullptr, *table2 = nullptr;
  mem_root_deque<TABLE *> *tables_s = nullptr, *tables_r = nullptr;


  /*
    Save the original result fields of items in case of falling back.
  */
  std::vector<Field *> result_fields;
  result_fields.reserve(tmp_table_fields.size());
  for (Item *item : tmp_table_fields) {
    result_fields.push_back(item->get_tmp_table_field());
  }

  Temp_table_param *temp_table_param = nullptr, *temp_table_param2 = nullptr;

  mem_root_deque<Item *> tmp_field(thd->mem_root);
  List_item *curr_fields = &tmp_table_fields;

  const uint ref_slice = REF_SLICE_EXCHANGE_1 + curr_exchange;

  const char *table_info_sender = "sender";
  const char *table_info_recv = "receiver";
  char *table_alias_sender = nullptr;
  char *table_alias_recv = nullptr;

  /*
    1. Create sender
  */
  temp_table_param =
      new (thd->mem_root) Temp_table_param(join->tmp_table_param);
  temp_table_param->skip_create_table = true;
  temp_table_param->copy_fields.clear();
  //temp_table_param->grouped_expressions.clear();
  temp_table_param->items_to_copy = nullptr;

  // TODO: Not always need to count.
  count_field_types(join->query_block, temp_table_param, *curr_fields, false,
                    save_sum_fields);
  temp_table_param->hidden_field_count = CountHiddenFields(*curr_fields);

  table_alias_sender =
      new (thd->mem_root) char[strlen(table_info_sender) + 5 + 1];
  sprintf(table_alias_sender, "<%s_%d>", table_info_sender, ref_slice);
  table = create_tmp_table(
      thd, temp_table_param, *curr_fields, /*group=*/nullptr,
      /*distinct=*/false, save_sum_fields, join->query_block->active_options(),
      HA_POS_ERROR, table_alias_sender);
  // The new field may not be compatible with message queues
  if (!table || !compat_for_table_fields(table)) goto inject_err;

  tables_s = new (thd->mem_root) mem_root_deque<TABLE *>(thd->mem_root);
  if (!tables_s) goto inject_err;
  tables_s->push_back(table);

  if (temp_table_param->items_to_copy &&
      temp_table_param->items_to_copy->size()) {
    Func_ptr_array *func_ptr = temp_table_param->items_to_copy;
    uint end = func_ptr->size();
    for (uint i = 0; i < end; i++) {
      Func_ptr &func = func_ptr->at(i);
      func.set_override_result_field(func.func()->get_result_field());
    }
  }
  DBUG_EXECUTE_IF("exchange_inject_print",
                  dbug_print_table(table, "PX_Sender", ref_slice););
  join->exchange_temp_table->push_back(table);
  join->exchange_temp_table_param->push_back(temp_table_param);
  sender = NewPXSendAccessPath(thd, path, tables_s, curr_fields,
                               temp_table_param, true, true);

  if (join->ref_items[REF_SLICE_SAVED_BASE].is_null()) {
    if (join->alloc_ref_item_slice(thd, REF_SLICE_SAVED_BASE)) goto inject_err;

    join->copy_ref_item_slice(REF_SLICE_SAVED_BASE, REF_SLICE_ACTIVE);
    join->tmp_fields[REF_SLICE_SAVED_BASE] = tmp_table_fields;
  }
  if (join->alloc_ref_item_slice(thd, ref_slice)) goto inject_err;

  if (save_sum_fields) {
    if (change_to_use_tmp_fields(curr_fields, thd, join->ref_items[ref_slice],
                                 &tmp_field, join->query_block->m_added_non_hidden_fields))
      goto inject_err;
  } else {
    if (change_to_use_tmp_fields_except_sums(curr_fields, thd, join->query_block,
                                             join->ref_items[ref_slice],
                                             &tmp_field, join->query_block->m_added_non_hidden_fields))
      goto inject_err;
  }

  // Saved for cleanup items.
  join->exchange_tmp_fields[curr_exchange] = tmp_field;

  /*
    2. Create receiver
  */
  curr_fields = &tmp_field;
  temp_table_param2 =
      new (thd->mem_root) Temp_table_param(join->tmp_table_param);
  temp_table_param2->skip_create_table = true;
  temp_table_param2->copy_fields.clear();
  //temp_table_param2->grouped_expressions.clear();
  temp_table_param2->items_to_copy = nullptr;

  // TODO: Not always need to count.
  count_field_types(join->query_block, temp_table_param2, *curr_fields, false,
                    false);
  temp_table_param2->hidden_field_count = CountHiddenFields(*curr_fields);

  table_alias_recv =
      new (thd->mem_root) char[strlen(table_info_recv) + 5 + 1];
  sprintf(table_alias_recv, "<%s_%d>", table_info_recv, ref_slice);
  table2 = create_tmp_table(
      thd, temp_table_param2, *curr_fields, /*group=*/nullptr,
      /*distinct=*/false, /*save_sum_funcs=*/false,
      join->query_block->active_options(), HA_POS_ERROR, table_alias_recv);
  if (!table2) goto inject_err;

  if (temp_table_param2->items_to_copy &&
      temp_table_param2->items_to_copy->size()) {
    Func_ptr_array *func_ptr = temp_table_param2->items_to_copy;
    uint end = func_ptr->size();
    for (uint i = 0; i < end; i++) {
      Func_ptr &func = func_ptr->at(i);
      func.set_override_result_field(func.func()->get_result_field());
    }
  }
  DBUG_EXECUTE_IF("exchange_inject_print",
                  dbug_print_table(table, "PX_Gather", ref_slice););
  join->exchange_temp_table->push_back(table2);
  join->exchange_temp_table_param->push_back(temp_table_param2);

  tables_r = new (thd->mem_root) mem_root_deque<TABLE *>(thd->mem_root);
  if (!tables_r) goto inject_err;
  tables_r->push_back(table2);

  if (split_pos->m_split_sort) {
    Filesort *curr_file_sort = split_pos->m_filesort;
    Filesort *final_file_sort = new (thd->mem_root) Filesort(
        thd, {table2}, curr_file_sort->keep_buffers, curr_file_sort->m_order,
        curr_file_sort->limit, curr_file_sort->m_remove_duplicates,
        curr_file_sort->m_force_sort_rowids,
        false);  // TODO reset sort_before_group
    receiver = NewPXReceiverMergeAccessPath(thd, sender, final_file_sort,
                                            tables_r, ref_slice, true);
  } else {
    
    receiver =
        NewPXReceiveAccessPath(thd, sender, tables_r, ref_slice, true);
  }

  /*
    ref_items[ref_slice] is covered, because we don`t need to
    store ref_item of exchange_send.
  */
  if (save_sum_fields) {
    if (change_to_use_tmp_fields(curr_fields, thd, join->ref_items[ref_slice],
                                 &join->tmp_fields[ref_slice], join->query_block->m_added_non_hidden_fields))
      goto inject_err;
  } else {
    if (change_to_use_tmp_fields_except_sums(curr_fields, thd, join->query_block,
                                             join->ref_items[ref_slice],
                                             &join->tmp_fields[ref_slice], join->query_block->m_added_non_hidden_fields))
      goto inject_err;
  }

  if (path->type == AccessPath::AGGREGATE ||
      path->type == AccessPath::TEMPTABLE_AGGREGATE)
    rebuild_ref_item(thd, join, ref_slice);

  if (alloc_group_field && !join->group_list.empty()) {
    join->set_ref_item_slice(ref_slice);
    ORDER *group = join->group_list.order;
    join->group_fields.clear();
    for (; group; group = group->next) {
      Cached_item *tmp = new_Cached_item(join->thd, *group->item);
      if (!tmp || join->group_fields.push_front(tmp)) goto inject_err;
    }
    join->set_ref_item_slice(REF_SLICE_SAVED_BASE);
  }

  return receiver;

inject_err:
  if (temp_table_param) {
    destroy(temp_table_param);
    temp_table_param = nullptr;
  }
  if (table) {
    close_tmp_table(table);
    free_tmp_table(table);
    uint idx = 0;
    for (Item *item : tmp_table_fields) {
      item->set_result_field(result_fields[idx++]);
    }
  }
  if (sender) {
    sender->px_send().tables = nullptr;
  }
  if (temp_table_param2) {
    destroy(temp_table_param2);
    temp_table_param2 = nullptr;
  }
  if (table2) {
    close_tmp_table(table2);
    free_tmp_table(table);
  }
  if (receiver) {
    split_pos->m_split_sort ? receiver->px_receiver_merge().tables = nullptr
                            : receiver->px_receiver().tables = nullptr;
  }
  return nullptr;
}

static AccessPath *CreateExchangeAccessPathUseTables(
    THD *thd, JOIN *join, AccessPath *const path,
    mem_root_deque<TABLE *> *tables, int ref_slice,
    px_access_path::Split_Position *split_pos) {
  assert(tables && tables->size());
  AccessPath *sender = NewPXSendAccessPath(thd, path, tables, nullptr,
                                           nullptr, false, false);

  AccessPath *receiver = nullptr;

  if (split_pos->m_split_sort) {
    assert(tables->size() == 1);
    TABLE *table = tables->front();

    Filesort *final_file_sort = nullptr;
    if (split_pos->m_filesort) {
      Filesort *curr_file_sort = split_pos->m_filesort;
      final_file_sort = new (thd->mem_root)
          Filesort(thd, {table}, curr_file_sort->keep_buffers,
                   curr_file_sort->m_order, curr_file_sort->limit,
                   curr_file_sort->m_remove_duplicates,
                   curr_file_sort->m_force_sort_rowids,
                   false);  // TODO reset sort_before_group
      // TODO: returns a result of type bool
      if (!final_file_sort) return nullptr;
    } else {
      // When using the AGGREGATE operator for grouping, merge sort must be used
      // to ensure that the input of the final agg is in order.
      assert(
          (join->saved_group_list && !join->saved_group_list->empty()) ||
          (join->saved_order && !join->saved_order->empty()));
      // If there is no filesort here, index is used. Index ordering is used
      // only if the index supports ordering of all group or order columns.
      // GROUP BY is executed before ORDER BY. See JOIN::test_skip_sort() and
      // test_if_skip_sort_order().
      ORDER *order =
          (join->saved_group_list && !join->saved_group_list->empty())
              ? join->saved_group_list->order
              : join->saved_order->order;
      final_file_sort = new (thd->mem_root) Filesort(
          thd, {table}, false, order, HA_POS_ERROR, false, false, false);
      if (!final_file_sort) return nullptr;
    }
    assert(final_file_sort);
    receiver = NewPXReceiverMergeAccessPath(thd, sender, final_file_sort,
        tables, ref_slice, false);
  } else {
    receiver =
        NewPXReceiveAccessPath(thd, sender, tables, ref_slice, false);
  }
  return receiver;
}

AccessPath *CreateExchangeAccessPathForUnion(THD *thd, AccessPath *const path,
                                              TABLE *table, bool is_append) {
  assert(table);

  mem_root_deque<TABLE *> *tables =
      new (thd->mem_root) mem_root_deque<TABLE *>(thd->mem_root);
  if (!tables) return nullptr;
  tables->push_back(table);

  if (is_append) {
    AccessPath *table_path =
        NewTableScanAccessPath(thd, table,
                               /*count_examined_rows=*/false);
    AccessPath *sender = NewPXSendAccessPath(thd, path, tables, nullptr,
                                             nullptr, false, false, table_path);
    AccessPath *receiver =
        NewPXReceiveAccessPath(thd, sender, tables, -1, false);
    return receiver;
  } else {
    AccessPath *sender = NewPXSendAccessPath(thd, path, tables, nullptr,
                                             nullptr, false, false, nullptr);
    AccessPath *receiver =
        NewPXReceiveAccessPath(thd, sender, tables, -1, false);
    return receiver;
  }
}

/**
 * Item_sum_avg would be split as sum and count, which will change the
 * number of items in ref_items so as to cause item_ref or group_list get wrong
 * item from ref_items. This function will recover sum and count with raw avg.
 */
static void rebuild_ref_item(THD *thd, JOIN *join, uint exchange_ref_slice) {
  const List_item &exchange_fields = join->tmp_fields[exchange_ref_slice];
  uint fields_count = join->saved_base_fields->size();
  uint ref_items_count = join->ref_items[REF_SLICE_SAVED_BASE].size();
  uint num_hidden_fields = CountHiddenFields(*join->saved_base_fields);
  Item **ref_item = thd->mem_root->ArrayAlloc<Item *>(ref_items_count);
  Ref_item_array tmp_ref_items = Ref_item_array(ref_item, ref_items_count);
  int i = 0, j = 0;
  for (Item *item : *join->saved_base_fields) {
    if (item->type() == Item::SUM_FUNC_ITEM) {
      Item_sum *item_sum = down_cast<Item_sum *>(item);
      if (item_sum->sum_func() == Item_sum::AVG_FUNC) {
        tmp_ref_items[(item->hidden ? fields_count - i -1
                                    : i - num_hidden_fields)] = item_sum;
        ++++j;
      } else {
        tmp_ref_items[(item->hidden ? fields_count - i -1
                                    : i - num_hidden_fields)] = exchange_fields[j++];
      }
    } else {
      tmp_ref_items[(item->hidden ? fields_count - i -1
                                  : i - num_hidden_fields)] = exchange_fields[j++];
    }
    ++i;
  }

  join->ref_items[exchange_ref_slice] = tmp_ref_items;
}

static void FixTmpTableParamForFinalAgg(Temp_table_param *param,
                                                 Temp_table_param *param_sender,
                                                 TABLE *exchange_table,
                                                 bool use_hash);

static void FixTmpTableParam(JOIN *join, Temp_table_param *temp_table_param,
                             uint exchange_slice, TABLE *exchange_table,
                             uint before_slice, uint curr_slice);

static void GetExchangeParam(ReceiverParam *receiver_param, AccessPath *receiver,
                             bool do_merge_sort) {
  assert(receiver_param != nullptr);
  if (do_merge_sort) {
    const auto &px_merge_receive = receiver->px_receiver_merge();
    receiver_param->ref_slice = px_merge_receive.ref_slice;
    receiver_param->table = px_merge_receive.tables->front();
    receiver_param->child = px_merge_receive.child;
    receiver_param->use_temp_table = px_merge_receive.use_temp_table;
  } else {
    const auto &px_receive = receiver->px_receiver();
    receiver_param->ref_slice = px_receive.ref_slice;
    receiver_param->table = px_receive.tables->front();
    receiver_param->child = px_receive.child;
    receiver_param->use_temp_table = px_receive.use_temp_table;
  }
}

static void FixAccessPathForExchange(AccessPath *const path,
                                     AccessPath *receiver, JOIN *join,
                                     bool do_merge_sort) {
  assert(receiver != nullptr);
  
  ReceiverParam exchange_param = {-1, nullptr, nullptr, false};
  GetExchangeParam(&exchange_param, receiver, do_merge_sort);

  // If exchange don`t create temporary table, there`s no need to fix it.
  if (!exchange_param.use_temp_table) return ;

  switch (path->type) {
    case AccessPath::NESTED_LOOP_JOIN: {
      break;
    }
    case AccessPath::HASH_JOIN: {
      assert(false);
      break;
    }
    case AccessPath::FILTER: {
      /*
        For having_cond, there`s no need to fix because it use item_ref. For
        where_cond, it can`t be here.
      */
      break;
    }
    case AccessPath::SORT: {
      FixSortAccessPath(join, path,
                        exchange_param.table, exchange_param.ref_slice);
      break;
    }
    case AccessPath::AGGREGATE:
    case AccessPath::TEMPTABLE_AGGREGATE: {
      /*
        (1) Reset join::sum_funcs.
          (1.1) Recompute sum_funcs with all Item_sum objects point at
                temporary table of gather.
          (1.2) Reset result_fields of sum_funcs to point at temporary table
                used by temp_table_aggregate.
        (2) Reset temp_table_param->copy_fields
        TODO: Save these objects in case of rollback.
      */

      Temp_table_param *temp_table_param = nullptr;
      bool is_final_agg = false;
      bool use_hash = false;
      switch (path->type) {
        case AccessPath::AGGREGATE: {
          //temp_table_param = path->aggregate().temp_table_param;
          is_final_agg = (path->aggregate().px_agg_type == AggType::PX_FINAL_AGG);
          break;
        }
        case AccessPath::TEMPTABLE_AGGREGATE: {
          temp_table_param = path->temptable_aggregate().temp_table_param;
          is_final_agg = (path->temptable_aggregate().px_agg_type == AggType::PX_FINAL_AGG);
          use_hash = (path->temptable_aggregate().table->hash_field != nullptr);
          break;
        }
        default:
          assert(false);
      }

      assert(exchange_param.ref_slice != -1);

      List_item *exchange_fields = &join->tmp_fields[exchange_param.ref_slice];

      if (is_final_agg) {
        // step 1
        if (temp_table_param->sum_func_count) {
          // Set args of final aggregate to items in exchange.
          Item_sum **sum_funcs = join->final_aggr_sum_funcs, *sum_func;
          while ((sum_func = *(sum_funcs++))) {
            for (uint i = 0; i < sum_func->argument_count(); ++i) {
              Item_sum::Sumfunctype sum_type = sum_func->sum_func();
              Item *arg = sum_func->get_arg(i);
              if (((sum_type == Item_sum::MIN_FUNC ||
                    sum_type == Item_sum::MAX_FUNC) &&
                   arg->basic_const_item()))
                continue;
              assert(arg->type() == Item::FIELD_ITEM);
              uint field_index = ((Item_field *)arg)->field->field_index();
              if (use_hash) --field_index;
              Field *res_field =
                  exchange_param.table->field[field_index];
              assert(res_field != nullptr);
              arg = sum_func->set_arg(join->thd, i,
                                      new (join->thd->mem_root)
                                          Item_field(res_field));
              if (!arg) return;
            }
            if (sum_func->sum_func() == Item_sum::GROUP_CONCAT_FUNC) {
              // Temp_table_param of Item_func_group_concat should be reset.
              Item_func_group_concat *item = (Item_func_group_concat *)sum_func;
              if (item->reset(join->thd)) return ;
            }
          }
        }

        // step 2
        Temp_table_param *param_sender =
            exchange_param.child->px_send().temp_table_param;
        switch (path->type) {
          case AccessPath::AGGREGATE: {
            // reset final_group_feilds
            if (join->final_group_feilds.size()) {
              assert(join->simple_group);
              join->set_ref_item_slice(exchange_param.ref_slice);
              ORDER *group = join->group_list.order;
              join->final_group_feilds.clear();
              for (; group; group = group->next) {
                Cached_item *tmp = new_Cached_item(join->thd, *group->item);
                if (!tmp || join->final_group_feilds.push_front(tmp)) return ;
              }
              join->set_ref_item_slice(REF_SLICE_SAVED_BASE);
            }
            FixTmpTableParamForFinalAgg(
                temp_table_param, param_sender, exchange_param.table, use_hash);
            break;
          }
          case AccessPath::TEMPTABLE_AGGREGATE: {
            FixTmpTableParamForFinalAgg(
                temp_table_param, param_sender, exchange_param.table, use_hash);
            break;
          }
          default:
            assert(false);
        }
      } else {
        // step 1
        if (temp_table_param->sum_func_count) {
          mem_root_deque<Field *> result_fields(join->thd->mem_root);
          mem_root_deque<Item_sum *> resolve_sum_funcs(join->thd->mem_root);
          Item_sum **sum_funcs = join->sum_funcs, *sum_func;
          while ((sum_func = *(sum_funcs++))) {
            result_fields.push_back(sum_func->get_result_field());
            if (sum_func->sum_func() == Item_sum::AVG_DISTINCT_FUNC ||
                sum_func->sum_func() == Item_sum::AVG_FUNC) {
              resolve_sum_funcs.push_back(sum_func);
            }
          }

          join->make_sum_func_list(*exchange_fields, /*before_group_by=*/false,
                                  /*recompute=*/true);

          sum_funcs = join->sum_funcs;
          for (Field *result_field : result_fields) {
            sum_func = *(sum_funcs++);
            assert(sum_func != nullptr);
            if (sum_func->sum_func() == Item_sum::AVG_DISTINCT_FUNC ||
                sum_func->sum_func() == Item_sum::AVG_FUNC) {
              Item_sum_avg *avg = (Item_sum_avg *)sum_func;
              const Item_sum_avg *raw_avg =
                  (Item_sum_avg *)resolve_sum_funcs.front();
              avg->f_precision = raw_avg->f_precision;
              avg->f_scale = raw_avg->f_scale;
              avg->dec_bin_size = raw_avg->dec_bin_size;
              resolve_sum_funcs.pop_front();
            }
            // sum_func->resolve_type(join->thd);
            sum_func->set_result_field(result_field);
            if (sum_func->sum_func() == Item_sum::GROUP_CONCAT_FUNC) {
              // Temp_table_param of type Item_func_group_concat should be reset.
              Item_func_group_concat *item = (Item_func_group_concat *)sum_func;
              if (item->reset(join->thd)) return ;
            }
          }

          if (path->type == AccessPath::AGGREGATE) {
            const bool need_distinct =
                !(join->qep_tab && join->qep_tab[0].range_scan() &&
                 is_agg_loose_index_scan(join->qep_tab[0].range_scan()));
            if (prepare_sum_aggregators(sum_funcs, need_distinct)) return ;
            if (setup_sum_funcs(join->thd, join->sum_funcs)) return ;

            sum_funcs = join->sum_funcs;
            // Sum funcs in join->fields should be reset
            for (Item *&item : *join->fields) {
              if (item->type() == Item::SUM_FUNC_ITEM && !item->const_item() &&
                  down_cast<Item_sum *>(item)->aggr_query_block ==
                      join->query_block) {
                sum_func = *(sum_funcs++);
                assert(sum_func);
                item = sum_func;
              }
            }
          }
        }

        int agg_ref_slice = -1;
        switch (path->type) {
          case AccessPath::AGGREGATE: {
            //agg_ref_slice = path->aggregate().output_slice;
            break;
          }
          case AccessPath::TEMPTABLE_AGGREGATE: {
            agg_ref_slice = path->temptable_aggregate().ref_slice;
            break;
          }
          default:
            assert(false);
        }

        FixTmpTableParam(join, temp_table_param, exchange_param.ref_slice,
                        exchange_param.table, REF_SLICE_SAVED_BASE,
                        agg_ref_slice);
      }
      break;
    }
    case AccessPath::LIMIT_OFFSET: {
      break;
    }
    case AccessPath::STREAM: {
      //if (!path->stream().copy_fields_and_items_in_materialize) break;
      auto tmp_table_param = path->stream().temp_table_param;
      FixTmpTableParam(join, tmp_table_param, exchange_param.ref_slice,
                       exchange_param.table, REF_SLICE_SAVED_BASE,
                       REF_SLICE_TMP1);

      break;
    }
    case AccessPath::MATERIALIZE: {
      const auto &param = path->materialize().param;
      auto tmp_table_param = param->query_blocks[0].temp_table_param;
      if (!tmp_table_param) break;
      int output_slice = param->ref_slice;
      int input_slice = -1;
      assert(output_slice != -1);
      if (output_slice == REF_SLICE_TMP2) {
        input_slice = REF_SLICE_TMP1;
      } else {
        input_slice = REF_SLICE_SAVED_BASE;
      }
      FixTmpTableParam(join, tmp_table_param, exchange_param.ref_slice,
                       exchange_param.table, input_slice, output_slice);
      break;
    }
    case AccessPath::WEEDOUT: {
      assert(false);
      break;
    }
    default:
      assert(false);
  }
}

void ConnectAccessPathWithChildExchange(AccessPath *const path,
                                        AccessPath *receiver) {
  switch (path->type) {
    case AccessPath::NESTED_LOOP_JOIN:
      path->nested_loop_join().outer = receiver;
      break;
    case AccessPath::HASH_JOIN: {
      path->hash_join().inner = receiver;
      break;
    }
    case AccessPath::FILTER:
      path->filter().child = receiver;
      break;
    case AccessPath::SORT:
      path->sort().child = receiver;
      break;
    case AccessPath::AGGREGATE:
      path->aggregate().child = receiver;
      break;
    case AccessPath::TEMPTABLE_AGGREGATE:
      path->temptable_aggregate().subquery_path = receiver;
      break;
    case AccessPath::LIMIT_OFFSET:
      path->limit_offset().child = receiver;
      break;
    case AccessPath::STREAM:
      path->stream().child = receiver;
      break;
    case AccessPath::MATERIALIZE: {
      path->materialize().param->query_blocks[0].subquery_path = receiver;
      break;
    }
    case AccessPath::WEEDOUT: {
      path->weedout().child = receiver;
      break;
    }
    default:
      assert(false);
  }
}

static void FixTmpTableParamForFinalAgg(Temp_table_param *param,
                                                 Temp_table_param *param_sender,
                                                 TABLE *exchange_table,
                                                 bool use_hash) {
  Mem_root_vector<Copy_field> *copy_fields = &param->copy_fields;
  Mem_root_vector<Copy_field> *copy_fields_sender = &param_sender->copy_fields;
  auto copy_field_sender = copy_fields_sender->begin();
  for (auto &copy_field : *copy_fields) {
    Field *from = copy_field.from_field();
    Field *sender_from = copy_field_sender->from_field();
    while (from->table != sender_from->table ||
           from->field_index() != sender_from->field_index()) {
      ++copy_field_sender;
      assert(copy_field_sender != copy_fields_sender->end());
      sender_from = copy_field_sender->from_field();
    }
    uint16 field_index = (copy_field_sender++)->to_field()->field_index();
    copy_field.set_from_field(exchange_table->field[field_index]);
  }
}

/**
 * @note Item_func may be calculated by exchange in advance.
 * @note Field used by aggregate
 */
static void FixTmpTableParam(JOIN *join, Temp_table_param *temp_table_param,
                             uint exchange_slice, TABLE *exchange_table,
                             uint before_slice, uint curr_slice) {
  List_item &before_fields = join->tmp_fields[before_slice];
  List_item &curr_fields = join->tmp_fields[curr_slice];
  List_item &exchange_fields = join->tmp_fields[exchange_slice];

  // REF_SLICE_ORDERED_GROUP_BY was created by setup_copy_fields() which don`t
  // skip const items.
  bool copy_all = true; //(curr_slice == REF_SLICE_ORDERED_GROUP_BY);

  uint16 field_index = 0;
  long num_hidden_fields = CountHiddenFields(before_fields);

  Mem_root_vector<Copy_field> *copy_fields = &temp_table_param->copy_fields;
  Func_ptr_array *items_to_copy = temp_table_param->items_to_copy;
  auto copy_field_it = copy_fields->begin();
  Mem_root_vector<Copy_field> new_copy_fields(
          Mem_root_allocator<Copy_field>(join->thd->mem_root));
  if (items_to_copy && items_to_copy->size()) {
    new_copy_fields.reserve(items_to_copy->size());
  }
  for (uint item_index = 0; item_index < before_fields.size(); ++item_index) {
    Item *before_item = before_fields[item_index];
    Item *exchange_item = exchange_fields[item_index];

    Item::Type type = before_item->type();
    // Const items would be skipped for copy
    if ((!copy_all || type != Item::FIELD_ITEM) &&
        before_item->const_item() && num_hidden_fields <= 0) {
      // Const items in ref slice would become not const in recevier, during the
      // creation of temporary tables
      if (!exchange_item->const_item()) ++field_index;
      continue;
    }

    switch(type) {
      case Item::SUM_FUNC_ITEM: {
        Item_sum *sum_item = down_cast<Item_sum *>(before_item);
        for (uint i = 0; i < sum_item->argument_count(); i++) {
          Item *arg = sum_item->get_arg(i);
          if (!arg->const_item()) field_index++;
        }
        break;
      }
      case Item::FIELD_ITEM: {
        Field *from_field = exchange_table->field[field_index++];
        assert(from_field != nullptr);
        if (copy_field_it == copy_fields->end() ||
            (from_field->type() == MYSQL_TYPE_BLOB &&
             copy_field_it->from_field()->type() != MYSQL_TYPE_BLOB))
          break;
        (copy_field_it++)->set_from_field(from_field);
        break;
      }
      case Item::FUNC_ITEM:
      case Item::COND_ITEM:
      case Item::SUBSELECT_ITEM:
      case Item::CACHE_ITEM:
      case Item::REF_ITEM: {
        // item_func may be calculated by exchange in advance.
        if (exchange_item->type() == Item::FIELD_ITEM) {
          Field *from_field = exchange_table->field[field_index++];
          assert(from_field != nullptr);
          Item *curr_item = curr_fields[item_index];
          Field *to_field = nullptr;
          switch (curr_item->type()) {
            case Item::FIELD_ITEM:
              to_field = ((Item_field *)curr_item)->field;
              break;
            /*
            case Item::COPY_STR_ITEM: { // TODO
              Item *real_item = ((Item_copy *)curr_item)->get_item();
              switch (real_item->type()) {
                case Item::FIELD_ITEM:
                  to_field = ((Item_field*)real_item)->field;
                  break;
                case Item::FUNC_ITEM:
                case Item::SUBSELECT_ITEM:
                  to_field = real_item->get_result_field();
                  break;
                default:
                  assert(false);
              }
              break;
            }
            */
            default:
              assert(false);
          }
          assert(to_field != nullptr);
          // TODO: Need to know in which condition the item would be added in
          // copy_fields.
          if (copy_field_it != copy_fields->end() &&
              to_field == copy_field_it->to_field()) {
            (copy_field_it++)->set_from_field(from_field);
          } else {
            new_copy_fields.emplace_back(to_field, from_field);
            // Fields of REF_SLICE_ORDERED_GROUP_BY was maked specially.
            /*
            if (curr_slice == REF_SLICE_ORDERED_GROUP_BY)
              new_copy_fields.back().reset();
            */
          }
        }
        break;
      }
      case Item::STRING_ITEM:
      case Item::NULL_ITEM: {
        if (exchange_item->type() == type) {
          ++field_index;
        }
        break;
      }
      default:
        break;
    }

    num_hidden_fields--;
  }

  if (new_copy_fields.size()) {
    if (items_to_copy) items_to_copy->clear();
    copy_fields->insert(copy_fields->end(), new_copy_fields.begin(),
                        new_copy_fields.end());
  }
}

/**
 * @brief Fix filesort for sort accesspath
 * 
 * @todo If insert exchange as child of sort accesspath, the source of exchange
 * inserted ahead of sort will be changed as the same as sort accesspath. So we
 * need to create parent exchange after when child exchange has been created.
 * Now we just don`t create child exchange of sort.
 */
void FixSortAccessPath(JOIN *join, AccessPath *path, TABLE *const new_table,
                       int ref_slice) {
  auto &new_filesort = path->sort().filesort;
  new_filesort->tables = std::move(
      Mem_root_array<TABLE *>({new_table}));

  // Redefine sort_order
  if (join->order.order) {
    join->set_ref_item_slice(ref_slice);
    new_filesort->make_sortorder(join->order.order, false);
    join->set_ref_item_slice(REF_SLICE_SAVED_BASE);
  }
  // Redefine addon fields for filesort
  if (new_filesort->m_sort_param.using_addon_fields()) {
    new_filesort->m_sort_param.addon_fields = nullptr;
    new_filesort->m_sort_param.m_addon_fields_status =
        Addon_fields_status::unknown_status;
    new_filesort->using_addon_fields();
  }
}

#ifndef DBUG_OFF
/*
  Print a text, SQL-like record representation into dbug trace, `error.log`
  generally.

  Note: this function is a work in progress: at the moment
   - column read bitmap is ignored (can print garbage for unused columns)
   - there is no quoting
*/
static void dbug_print_table(TABLE *table, const char *str, uint slice) {
  char buff[1024];
  Field **pfield;
  String tmp(buff, sizeof(buff), &my_charset_bin);
  DBUG_LOCK_FILE;

  fprintf(DBUG_FILE, "\nrecord: %s, %d (", str, slice);
  for (pfield = table->field; *pfield; pfield++)
    fprintf(DBUG_FILE, "%s%s", (*pfield)->field_name, (pfield[1]) ? ", " : "");
  fprintf(DBUG_FILE, ")");
  fprintf(DBUG_FILE, "\n");
  DBUG_UNLOCK_FILE;
}
#endif

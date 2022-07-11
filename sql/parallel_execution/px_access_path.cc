#include "sql/parallel_execution/px_access_path.h"

#include "sql/filesort.h"
#include "sql/join_optimizer/access_path.h"
#include "sql/join_optimizer/walk_access_paths.h"
#include "sql/parallel_execution/px_executor.h"  // PX_PRINT_INFO
#include "sql/parallel_execution/px_item.h"
#include "sql/parallel_execution/px_optimizer.h"  // set_parallel_degree_hint()

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
      thd->variables.px_parallel_table_record_threshold) {
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
  auto first = group_fields.begin();
  ORDER *second = sort_order;
  for (; first != group_fields.end() && second;
       ++first, second = second->next) {
    if ((first->get_item())->eq(*second->item, true)) {
      continue;
    } else {
      return false;
    }
  }
  if (first != group_fields.end() || second) return false;
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
      } else {
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
  bool cur_join_end = false;
  QEP_TAB *cur_parallel_tab = nullptr;
  Split_Position *cur_split_pos = nullptr;

  for (Split_Position &split_pos : *split_positions_in) {
    // New query block or union
    if (!cur_join || cur_join != split_pos.m_join || split_pos.m_split_union) {
      cur_join_end = false;
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

    if (cur_join_end) continue;

    // TODO: aggregate(stream or temptable) below Materialize can NOT be split
    // now.
    if (split_pos.m_split_agg &&
        (split_pos.m_parent &&
         split_pos.m_parent->type == AccessPath::MATERIALIZE)) {
      cur_join_end = true;
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

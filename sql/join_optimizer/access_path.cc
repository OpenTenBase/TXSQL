/* Copyright (c) 2020, 2022, Oracle and/or its affiliates.

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

#include "sql/join_optimizer/access_path.h"
#include "sql/filesort.h"
#include "sql/field.h"
#include "sql/sql_tmp_table.h"
#include "sql/item_sum.h"
#include "sql/iterators/basic_row_iterators.h"
#include "sql/iterators/bka_iterator.h"
#include "sql/iterators/composite_iterators.h"
#include "sql/iterators/delete_rows_iterator.h"
#include "sql/iterators/hash_join_iterator.h"
#include "sql/iterators/ref_row_iterators.h"
#include "sql/iterators/sorting_iterator.h"
#include "sql/iterators/timing_iterator.h"
#include "sql/iterators/window_iterators.h"
#include "sql/join_optimizer/bit_utils.h"
#include "sql/join_optimizer/cost_model.h"
#include "sql/join_optimizer/estimate_selectivity.h"
#include "sql/join_optimizer/relational_expression.h"
#include "sql/join_optimizer/walk_access_paths.h"
#include "sql/range_optimizer/geometry_index_range_scan.h"
#include "sql/range_optimizer/group_index_skip_scan.h"
#include "sql/range_optimizer/group_index_skip_scan_plan.h"
#include "sql/range_optimizer/index_merge.h"
#include "sql/range_optimizer/index_range_scan.h"
#include "sql/range_optimizer/index_skip_scan.h"
#include "sql/range_optimizer/index_skip_scan_plan.h"
#include "sql/range_optimizer/range_optimizer.h"
#include "sql/range_optimizer/reverse_index_range_scan.h"
#include "sql/range_optimizer/rowid_ordered_retrieval.h"
#include "sql/sql_optimizer.h"
#include "sql/sql_update.h"
#include "sql/table.h"
#include "sql/iterators/sort_merge_join_iterator.h"
#include "sql/parallel_execution/px_item.h"
#include "sql/px_exchange.h"
#include "sql/parallel_execution/px_sender.h"
#include "sql/parallel_execution/px_receiver.h"
#include "sql/parallel_execution/px_receiver_merge.h"
#include "sql/log.h"
#include "sql/table_function.h"  // Table_function

#include <vector>

using pack_rows::TableCollection;
using std::vector;

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
      // Quick select removed
      TABLE *table = u.index_range_scan.used_key_part[0].field->table;
      TABLE *other_table = other.index_range_scan().used_key_part[0].field->table;
      if (!EquivalenceCheckHelper::eq_table_share(
              table,
              other_table) ||
          !EquivalenceCheckHelper::eq_item(
              table->file->pushed_idx_cond,
             other_table->file->pushed_idx_cond)) {
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
      if (u.hash_join.store_rowids != other.hash_join().store_rowids ||
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
          u.aggregate.is_final_aggr != other.aggregate().is_final_aggr) {
        return false;
      }
      break;
    }
    case TEMPTABLE_AGGREGATE: {
      // equivalence check: AccessPath
      // Note the table_name/path of temptable (u.temptable_aggregate.table)
      // is different from coordinate, thus the equivalence check need rely
      // on sub-path in worker_table_explain.children.
      if (u.temptable_aggregate.is_final_aggr != other.temptable_aggregate().is_final_aggr ||
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
        return (*u.limit_offset.send_records_override == *other.limit_offset().send_records_override);
      }
      if (other.limit_offset().send_records_override != nullptr) {
        return false;
      }
      break;
    }
    case REMOVE_DUPLICATES: {
      // group items need compare
      break;
    }
    case ALTERNATIVE: {
      // equivalence check: same TABLE_SHARE, ref, and cond_guards(field)
      if (!EquivalenceCheckHelper::eq_table_share(
              u.alternative.table_scan_path->table_scan().table,
              other.alternative().table_scan_path->table_scan().table) ||
          !EquivalenceCheckHelper::eq_table_ref(u.alternative.used_ref,
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
        if (other_tab != nullptr) {
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
      if (u.stream.provide_rowid !=
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
    case MATERIALIZE_INFORMATION_SCHEMA_TABLE:
    default:
      return false; // not supported
      break;
  }
  return true;
}

AccessPath *NewSortAccessPath(THD *thd, AccessPath *child, Filesort *filesort,
                              bool count_examined_rows) {
  AccessPath *path = new (thd->mem_root) AccessPath;
  path->type = AccessPath::SORT;
  path->count_examined_rows = count_examined_rows;
  path->sort().child = child;
  path->sort().filesort = filesort;

  if (filesort->using_addon_fields()) {
    path->sort().tables_to_get_rowid_for = 0;
  } else {
    if (filesort->tables.size() == 1 &&
        filesort->tables[0]->pos_in_table_list == nullptr) {
      // This can happen if we sort a single temporary table
      // which is not in the table list (e.g., one that was
      // specifically created for us). Filesort has special-casing
      // to always get the row ID in this case.
      path->sort().tables_to_get_rowid_for = 0;
    } else {
      FindTablesToGetRowidFor(path);
    }
  }

  return path;
}

AccessPath *NewDeleteRowsAccessPath(THD *thd, AccessPath *child,
                                    table_map delete_tables,
                                    table_map immediate_tables) {
  assert(IsSubset(immediate_tables, delete_tables));
  AccessPath *path = new (thd->mem_root) AccessPath;
  path->type = AccessPath::DELETE_ROWS;
  path->delete_rows().child = child;
  path->delete_rows().tables_to_delete_from = delete_tables;
  path->delete_rows().immediate_tables = immediate_tables;
  return path;
}

AccessPath *NewUpdateRowsAccessPath(THD *thd, AccessPath *child,
                                    table_map update_tables,
                                    table_map immediate_tables) {
  assert(IsSubset(immediate_tables, update_tables));
  AccessPath *path = new (thd->mem_root) AccessPath;
  path->type = AccessPath::UPDATE_ROWS;
  path->update_rows().child = child;
  path->update_rows().tables_to_update = update_tables;
  path->update_rows().immediate_tables = immediate_tables;
  return path;
}

bool compat_for_table(TABLE *table) {
  bool ret = true;
  for (Field **pfield = table->field; *pfield != nullptr; ++pfield) {
    Field *field = *pfield;
    if (nullptr == field) continue;
    if (bitmap_is_set(table->read_set, field->field_index())) {
      if (field->type() == MYSQL_TYPE_BLOB || 
          field->type() == MYSQL_TYPE_BIT || 
          field->type() == MYSQL_TYPE_JSON ||
          field->type() == MYSQL_TYPE_TINY_BLOB ||
          field->type() == MYSQL_TYPE_MEDIUM_BLOB ||
          field->type() == MYSQL_TYPE_LONG_BLOB ||
          field->type() == MYSQL_TYPE_GEOMETRY) {
        ret = false;
        break;
      }
    }
  }
  return ret;
}

bool WalkAccessPathsForCompat(AccessPath *path, bool check) {
  // TODO: more check
  bool parallel_safe = false;
  switch (path->type) {
    case AccessPath::TABLE_SCAN:
    case AccessPath::INDEX_SCAN:
    case AccessPath::REF:
    case AccessPath::REF_OR_NULL:
    case AccessPath::INDEX_RANGE_SCAN: {
      parallel_safe = true;
      break;
    }
    case AccessPath::EQ_REF:
    case AccessPath::PUSHED_JOIN_REF: {
      parallel_safe = true;
      break;
    }
    case AccessPath::FULL_TEXT_SEARCH: {
      // No children.
      parallel_safe = false;
      break;
    }
    case AccessPath::CONST_TABLE: {
      parallel_safe = false;
      break;
    }
    case AccessPath::MRR:
    case AccessPath::FOLLOW_TAIL: {
      // No children.
      parallel_safe = true;
      break;
    }
    case AccessPath::INDEX_MERGE:
    case AccessPath::ROWID_INTERSECTION:
    case AccessPath::ROWID_UNION:
    case AccessPath::INDEX_SKIP_SCAN:
    case AccessPath::GROUP_INDEX_SKIP_SCAN:
    case AccessPath::DYNAMIC_INDEX_RANGE_SCAN:
    case AccessPath::TABLE_VALUE_CONSTRUCTOR: {
      // No children.
      parallel_safe = true;
      break;
    }
    case AccessPath::FAKE_SINGLE_ROW: {
      // No children.
      parallel_safe = false;
      break;
    }
    case AccessPath::ZERO_ROWS: {
      parallel_safe = false;
      break;
    }
    case AccessPath::ZERO_ROWS_AGGREGATED:
    case AccessPath::MATERIALIZED_TABLE_FUNCTION:
    case AccessPath::UNQUALIFIED_COUNT: {
      // No children.
      parallel_safe = true;
      break;
    }
    case AccessPath::NESTED_LOOP_JOIN: {
      bool o_compat = WalkAccessPathsForCompat(path->nested_loop_join().outer, true);
      bool i_compat = WalkAccessPathsForCompat(path->nested_loop_join().inner, false);
      parallel_safe = o_compat && i_compat;
      break;
    }
    case AccessPath::NESTED_LOOP_SEMIJOIN_WITH_DUPLICATE_REMOVAL: {
      bool o_compat = WalkAccessPathsForCompat(
        path->nested_loop_semijoin_with_duplicate_removal().outer, true);
      bool i_compat = WalkAccessPathsForCompat(
        path->nested_loop_semijoin_with_duplicate_removal().inner, false);
      parallel_safe = o_compat && i_compat;
      break;
    }
    case AccessPath::BKA_JOIN: {
      bool o_compat = WalkAccessPathsForCompat(path->bka_join().outer, true);
      bool i_compat = WalkAccessPathsForCompat(path->bka_join().inner, false);
      parallel_safe = o_compat && i_compat;
      break;
    }
    case AccessPath::HASH_JOIN: {
      bool o_compat = WalkAccessPathsForCompat(path->hash_join().outer, true);
      bool i_compat = WalkAccessPathsForCompat(path->hash_join().inner, false);
      parallel_safe = o_compat && i_compat;
      // hash join is not supported in parallel query.
      parallel_safe = false;
      break;
    }
    case AccessPath::FILTER: {
      parallel_safe = WalkAccessPathsForCompat(path->filter().child);
      if (parallel_safe) {
        Item *condition = path->filter().condition;
        if (condition->walk(&Item::check_compat_for_parallel,
                      enum_walk::POSTFIX, nullptr))
          parallel_safe = false;
      }
      break;
    }
    case AccessPath::SORT: {
      parallel_safe = WalkAccessPathsForCompat(path->sort().child);
      break;
    }
    case AccessPath::AGGREGATE: {
      parallel_safe = WalkAccessPathsForCompat(path->aggregate().child);
      break;
    }
    case AccessPath::TEMPTABLE_AGGREGATE: {
      bool subquery_compat = WalkAccessPathsForCompat(
        path->temptable_aggregate().subquery_path);
      // bool table_compat = WalkAccessPathsForCompat(
      //   path->temptable_aggregate().table_path);
      parallel_safe = subquery_compat;
      break;
    }
    case AccessPath::LIMIT_OFFSET: {
      parallel_safe = WalkAccessPathsForCompat(path->limit_offset().child);
      if (parallel_safe) {
        // only support limit after filesort.
        parallel_safe = path->limit_offset().child->type == AccessPath::SORT;
      }
      break;
    }
    case AccessPath::STREAM: {
      parallel_safe = WalkAccessPathsForCompat(path->stream().child);
      break;
    }
    case AccessPath::MATERIALIZE: {
      parallel_safe = WalkAccessPathsForCompat(path->materialize().table_path);
      // Check for each query block.
      break;
    }
    case AccessPath::MATERIALIZE_INFORMATION_SCHEMA_TABLE: {
      parallel_safe = false;
      break;
    }
    case AccessPath::APPEND: {
      // check for each children
      parallel_safe = true;
      break;
    }
    case AccessPath::WINDOW: {
      parallel_safe = WalkAccessPathsForCompat(path->window().child);
      break;
    }
    case AccessPath::WEEDOUT: {
      parallel_safe = WalkAccessPathsForCompat(path->weedout().child);
      break;
    }
    case AccessPath::REMOVE_DUPLICATES: {
      parallel_safe = WalkAccessPathsForCompat(path->remove_duplicates().child);
      break;
    }
    case AccessPath::ALTERNATIVE: {
      parallel_safe = WalkAccessPathsForCompat(path->alternative().child);
      break;
    }
    case AccessPath::CACHE_INVALIDATOR: {
      parallel_safe = WalkAccessPathsForCompat(path->cache_invalidator().child);
      break;
    }
    default:
      break;
  }
  if (!parallel_safe)
    sql_print_information("WalkAccessPathsForCompat:%d NO Compat type[%d]",
      __LINE__, path->type);
  return parallel_safe;
}

static AccessPath *FindSingleAccessPathOfType(AccessPath *path,
                                              AccessPath::Type type) {
  AccessPath *found_path = nullptr;

  auto func = [type, &found_path](AccessPath *subpath, const JOIN *) {
#ifdef NDEBUG
    constexpr bool fast_exit = true;
#else
    constexpr bool fast_exit = false;
#endif
    if (subpath->type == type) {
      assert(found_path == nullptr);
      found_path = subpath;
      // If not in debug mode, stop as soon as we find the first one.
      if (fast_exit) {
        return true;
      }
    }
    return false;
  };
  // Our users generally want to stop at STREAM or MATERIALIZE nodes,
  // since they are table-oriented and those nodes have their own tables.
  WalkAccessPaths(path, /*join=*/nullptr,
                  WalkAccessPathPolicy::STOP_AT_MATERIALIZATION, func);
  return found_path;
}

static RowIterator *FindSingleIteratorOfType(AccessPath *path,
                                             AccessPath::Type type) {
  AccessPath *found_path = FindSingleAccessPathOfType(path, type);
  if (found_path == nullptr) {
    return nullptr;
  } else {
    return found_path->iterator->real_iterator();
  }
}

TABLE *GetBasicTable(const AccessPath *path) {
  switch (path->type) {
    // Basic access paths (those with no children, at least nominally).
    case AccessPath::TABLE_SCAN:
      return path->table_scan().table;
    case AccessPath::INDEX_SCAN:
      return path->index_scan().table;
    case AccessPath::REF:
      return path->ref().table;
    case AccessPath::REF_OR_NULL:
      return path->ref_or_null().table;
    case AccessPath::EQ_REF:
      return path->eq_ref().table;
    case AccessPath::PUSHED_JOIN_REF:
      return path->pushed_join_ref().table;
    case AccessPath::FULL_TEXT_SEARCH:
      return path->full_text_search().table;
    case AccessPath::CONST_TABLE:
      return path->const_table().table;
    case AccessPath::MRR:
      return path->mrr().table;
    case AccessPath::FOLLOW_TAIL:
      return path->follow_tail().table;
    case AccessPath::INDEX_RANGE_SCAN:
      return path->index_range_scan().used_key_part[0].field->table;
    case AccessPath::DYNAMIC_INDEX_RANGE_SCAN:
      return path->dynamic_index_range_scan().table;

    case AccessPath::INDEX_MERGE:
      return path->index_merge().table;

    // Basic access paths that don't correspond to a specific table.
    case AccessPath::TABLE_VALUE_CONSTRUCTOR:
    case AccessPath::FAKE_SINGLE_ROW:
    case AccessPath::ZERO_ROWS:
    case AccessPath::ZERO_ROWS_AGGREGATED:
    case AccessPath::MATERIALIZED_TABLE_FUNCTION:
    case AccessPath::UNQUALIFIED_COUNT:

    // Note, some other AccessPaths may use its own temporary (derived) table.
    // We intentionally do not return such TABLEs.
    default:
      return nullptr;
  }
}

table_map GetUsedTableMap(const AccessPath *path, bool include_pruned_tables) {
  table_map tmap = 0;
  WalkTablesUnderAccessPath(
      const_cast<AccessPath *>(path),
      [&tmap](TABLE *table) {
        if (table->pos_in_table_list == nullptr) {
          // Materialization within a JOIN (e.g., for sorting). The table won't
          // have a map, so the caller will need to find the table manually.
          tmap |= RAND_TABLE_BIT;
        } else {
          tmap |= table->pos_in_table_list->map();
        }
        return false;
      },
      include_pruned_tables);
  return tmap;
}

static Prealloced_array<TABLE *, 4> GetUsedTables(AccessPath *child,
                                                  bool include_pruned_tables) {
  Prealloced_array<TABLE *, 4> tables{PSI_NOT_INSTRUMENTED};
  WalkTablesUnderAccessPath(
      child,
      [&tables](TABLE *table) {
        tables.push_back(table);
        return false;
      },
      include_pruned_tables);
  return tables;
}

Mem_root_array<TABLE *> CollectTables(THD *thd, AccessPath *root_path) {
  Mem_root_array<TABLE *> tables(thd->mem_root);
  WalkTablesUnderAccessPath(
      root_path, [&tables](TABLE *table) { return tables.push_back(table); },
      /*include_pruned_tables=*/true);
  return tables;
}

// Mirrors QEP_TAB::pfs_batch_update(), with one addition:
// If there is more than one table, batch mode will be handled by the join
// iterators on the probe side, so joins will return false.
bool ShouldEnableBatchMode(AccessPath *path) {
  switch (path->type) {
    case AccessPath::TABLE_SCAN:
    case AccessPath::INDEX_SCAN:
    case AccessPath::REF:
    case AccessPath::REF_OR_NULL:
    case AccessPath::PUSHED_JOIN_REF:
    case AccessPath::FULL_TEXT_SEARCH:
    case AccessPath::DYNAMIC_INDEX_RANGE_SCAN:
      return true;
    case AccessPath::FILTER:
      if (path->filter().condition->has_subquery()) {
        return false;
      } else {
        return ShouldEnableBatchMode(path->filter().child);
      }
    case AccessPath::SORT:
      return ShouldEnableBatchMode(path->sort().child);
    case AccessPath::EQ_REF:
    case AccessPath::CONST_TABLE:
      // These can read only one row per scan, so batch mode will never be a
      // win (fall through).
    default:
      // All others, in particular joins.
      return false;
  }
}

bool FinalizeMaterializedSubqueries(THD *thd, JOIN *join, AccessPath *path) {
  if (path->type != AccessPath::FILTER ||
      !path->filter().materialize_subqueries) {
    return false;
  }
  return WalkItem(
      path->filter().condition, enum_walk::POSTFIX, [thd, join](Item *item) {
        if (!IsItemInSubSelect(item)) {
          return false;
        }
        Item_in_subselect *item_subs = down_cast<Item_in_subselect *>(item);
        Query_block *subquery_block = item_subs->unit->first_query_block();
        if (!item_subs->subquery_allows_materialization(thd, subquery_block,
                                                        join->query_block)) {
          return false;
        }
        if (item_subs->finalize_materialization_transform(
                thd, subquery_block->join)) {
          return true;
        }
        item_subs->create_iterators(thd);
        return false;
      });
}

namespace {

struct IteratorToBeCreated {
  AccessPath *path;
  JOIN *join;
  bool eligible_for_batch_mode;
  unique_ptr_destroy_only<RowIterator> *destination;
  Bounds_checked_array<unique_ptr_destroy_only<RowIterator>> children;

  void AllocChildren(MEM_ROOT *mem_root, int num_children) {
    children =
        Bounds_checked_array<unique_ptr_destroy_only<RowIterator>>::Alloc(
            mem_root, num_children);
  }
};

void SetupJobsForChildren(MEM_ROOT *mem_root, AccessPath *child, JOIN *join,
                          bool eligible_for_batch_mode,
                          IteratorToBeCreated *job,
                          Mem_root_array<IteratorToBeCreated> *todo) {
  // Make jobs for the child, and we'll return to this job later.
  job->AllocChildren(mem_root, 1);
  todo->push_back(*job);
  todo->push_back(
      {child, join, eligible_for_batch_mode, &job->children[0], {}});
}

void SetupJobsForChildren(MEM_ROOT *mem_root, AccessPath *outer,
                          AccessPath *inner, JOIN *join,
                          bool inner_eligible_for_batch_mode,
                          IteratorToBeCreated *job,
                          Mem_root_array<IteratorToBeCreated> *todo) {
  // Make jobs for the children, and we'll return to this job later.
  // Note that we push the inner before the outer job, so that we get
  // left created before right (invalidators in materialization access paths,
  // used in the old join optimizer, depend on this).
  job->AllocChildren(mem_root, 2);
  todo->push_back(*job);
  todo->push_back(
      {inner, join, inner_eligible_for_batch_mode, &job->children[1], {}});
  todo->push_back({outer, join, false, &job->children[0], {}});
}

}  // namespace

unique_ptr_destroy_only<RowIterator> CreateIteratorFromAccessPath(
    THD *thd, MEM_ROOT *mem_root, AccessPath *top_path, JOIN *top_join,
    bool top_eligible_for_batch_mode) {
  unique_ptr_destroy_only<RowIterator> ret;
  Mem_root_array<IteratorToBeCreated> todo(mem_root);
  todo.push_back({top_path, top_join, top_eligible_for_batch_mode, &ret, {}});

  // The access path trees can be pretty deep, and the stack frames can be big
  // on certain compilers/setups, so instead of explicit recursion, we push jobs
  // onto a MEM_ROOT-backed stack. This uses a little more RAM (the MEM_ROOT
  // typically lives to the end of the query), but reduces the stack usage
  // greatly.
  //
  // The general rule is that if an iterator requires any children, it will push
  // jobs for their access paths at the end of the stack and then re-push
  // itself. When the children are instantiated and we get back to the original
  // iterator, we'll actually instantiate it. (We distinguish between the two
  // cases on basis of whether job.children has been allocated or not; the child
  // iterator's destination will point into this array. The child list needs
  // to be allocated in a way that doesn't move around if the TODO job list
  // is reallocated, which we do by means of allocating it directly on the
  // MEM_ROOT.)
  while (!todo.empty()) {
    IteratorToBeCreated job = todo.back();
    todo.pop_back();

    AccessPath *path = job.path;
    JOIN *join = job.join;
    bool eligible_for_batch_mode = job.eligible_for_batch_mode;

    if (job.join != nullptr) {
      assert(!job.join->needs_finalize);
    }

    unique_ptr_destroy_only<RowIterator> iterator;

    ha_rows *examined_rows = nullptr;
    if (path->count_examined_rows && join != nullptr) {
      examined_rows = &join->examined_rows;
    }

    switch (path->type) {
      case AccessPath::TABLE_SCAN: {
        const auto &param = path->table_scan();
        iterator = NewIterator<TableScanIterator>(
            thd, mem_root, param.table, path->num_output_rows, examined_rows);
        break;
      }
      case AccessPath::INDEX_SCAN: {
        const auto &param = path->index_scan();
        if (param.reverse) {
          iterator = NewIterator<IndexScanIterator<true>>(
              thd, mem_root, param.table, param.idx, param.use_order,
              path->num_output_rows, examined_rows, true);
        } else {
          iterator = NewIterator<IndexScanIterator<false>>(
              thd, mem_root, param.table, param.idx, param.use_order,
              path->num_output_rows, examined_rows, false);
        }
        break;
      }
      case AccessPath::REF: {
        const auto &param = path->ref();
        if (param.reverse) {
          iterator = NewIterator<RefIterator<true>>(
              thd, mem_root, param.table, param.ref, param.use_order,
              path->num_output_rows, examined_rows, true);
        } else {
          iterator = NewIterator<RefIterator<false>>(
              thd, mem_root, param.table, param.ref, param.use_order,
              path->num_output_rows, examined_rows, false);
        }
        break;
      }
      case AccessPath::REF_OR_NULL: {
        const auto &param = path->ref_or_null();
        iterator = NewIterator<RefOrNullIterator>(
            thd, mem_root, param.table, param.ref, param.use_order,
            path->num_output_rows, examined_rows);
        break;
      }
      case AccessPath::EQ_REF: {
        const auto &param = path->eq_ref();
        iterator =
            NewIterator<EQRefIterator>(thd, mem_root, param.table, param.ref,
                                       param.use_order, examined_rows);
        break;
      }
      case AccessPath::PUSHED_JOIN_REF: {
        const auto &param = path->pushed_join_ref();
        iterator = NewIterator<PushedJoinRefIterator>(
            thd, mem_root, param.table, param.ref, param.use_order,
            param.is_unique, examined_rows);
        break;
      }
      case AccessPath::FULL_TEXT_SEARCH: {
        const auto &param = path->full_text_search();
        iterator = NewIterator<FullTextSearchIterator>(
            thd, mem_root, param.table, param.ref, param.ft_func,
            param.use_order, param.use_limit, examined_rows);
        break;
      }
      case AccessPath::CONST_TABLE: {
        const auto &param = path->const_table();
        iterator = NewIterator<ConstIterator>(thd, mem_root, param.table,
                                              param.ref, examined_rows);
        break;
      }
      case AccessPath::MRR: {
        const auto &param = path->mrr();
        const auto &bka_param = param.bka_path->bka_join();
        iterator = NewIterator<MultiRangeRowIterator>(
            thd, mem_root, param.table, param.ref, param.mrr_flags,
            bka_param.join_type,
            GetUsedTables(bka_param.outer, /*include_pruned_tables=*/true),
            bka_param.store_rowids, bka_param.tables_to_get_rowid_for);
        break;
      }
      case AccessPath::FOLLOW_TAIL: {
        const auto &param = path->follow_tail();
        iterator = NewIterator<FollowTailIterator>(
            thd, mem_root, param.table, path->num_output_rows, examined_rows);
        break;
      }
      case AccessPath::INDEX_RANGE_SCAN: {
        const auto &param = path->index_range_scan();
        TABLE *table = param.used_key_part[0].field->table;
        if (param.geometry) {
          iterator = NewIterator<GeometryIndexRangeScanIterator>(
              thd, mem_root, table, examined_rows, path->num_output_rows,
              param.index, param.need_rows_in_rowid_order, param.reuse_handler,
              mem_root, param.mrr_flags, param.mrr_buf_size,
              Bounds_checked_array{param.ranges, param.num_ranges});
        } else if (param.reverse) {
          iterator = NewIterator<ReverseIndexRangeScanIterator>(
              thd, mem_root, table, examined_rows, path->num_output_rows,
              param.index, mem_root, param.mrr_flags,
              Bounds_checked_array{param.ranges, param.num_ranges},
              param.using_extended_key_parts);
        } else {
          iterator = NewIterator<IndexRangeScanIterator>(
              thd, mem_root, table, examined_rows, path->num_output_rows,
              param.index, param.need_rows_in_rowid_order, param.reuse_handler,
              mem_root, param.mrr_flags, param.mrr_buf_size,
              Bounds_checked_array{param.ranges, param.num_ranges});
        }
        break;
      }
      case AccessPath::INDEX_MERGE: {
        const auto &param = path->index_merge();
        unique_ptr_destroy_only<RowIterator> pk_quick_select;
        if (job.children.is_null()) {
          job.AllocChildren(mem_root, param.children->size());
          todo.push_back(job);
          for (size_t child_idx = 0; child_idx < param.children->size();
               ++child_idx) {
            todo.push_back({(*param.children)[child_idx],
                            join,
                            /*eligible_for_batch_mode=*/false,
                            &job.children[child_idx],
                            {}});
          }
          continue;
        }
        Mem_root_array<unique_ptr_destroy_only<RowIterator>> children(mem_root);
        children.reserve(param.children->size());
        for (size_t child_idx = 0; child_idx < param.children->size();
             ++child_idx) {
          AccessPath *range_scan = (*param.children)[child_idx];
          if (param.allow_clustered_primary_key_scan &&
              param.table->file->primary_key_is_clustered() &&
              range_scan->index_range_scan().index ==
                  param.table->s->primary_key) {
            assert(pk_quick_select == nullptr);
            pk_quick_select = std::move(job.children[child_idx]);
          } else {
            children.push_back(std::move(job.children[child_idx]));
          }
        }

        iterator = NewIterator<IndexMergeIterator>(
            thd, mem_root, mem_root, param.table, std::move(pk_quick_select),
            std::move(children));
        break;
      }
      case AccessPath::ROWID_INTERSECTION: {
        const auto &param = path->rowid_intersection();
        if (job.children.is_null()) {
          job.AllocChildren(mem_root, param.children->size() +
                                          (param.cpk_child != nullptr ? 1 : 0));
          todo.push_back(job);
          for (size_t child_idx = 0; child_idx < param.children->size();
               ++child_idx) {
            todo.push_back({(*param.children)[child_idx],
                            join,
                            /*eligible_for_batch_mode=*/false,
                            &job.children[child_idx],
                            {}});
          }
          if (param.cpk_child != nullptr) {
            todo.push_back({param.cpk_child,
                            join,
                            /*eligible_for_batch_mode=*/false,
                            &job.children[param.children->size()],
                            {}});
          }
          continue;
        }

        // TODO(sgunders): Consider just sending in the array here,
        // changing types in the constructor.
        Mem_root_array<unique_ptr_destroy_only<RowIterator>> children(mem_root);
        children.reserve(param.children->size());
        for (size_t child_idx = 0; child_idx < param.children->size();
             ++child_idx) {
          children.push_back(move(job.children[child_idx]));
        }

        unique_ptr_destroy_only<RowIterator> cpk_child;
        if (param.cpk_child != nullptr) {
          cpk_child = move(job.children[param.children->size()]);
        }
        iterator = NewIterator<RowIDIntersectionIterator>(
            thd, mem_root, mem_root, param.table, param.retrieve_full_rows,
            param.need_rows_in_rowid_order, std::move(children),
            std::move(cpk_child));
        break;
      }
      case AccessPath::ROWID_UNION: {
        const auto &param = path->rowid_union();
        if (job.children.is_null()) {
          job.AllocChildren(mem_root, param.children->size());
          todo.push_back(job);
          for (size_t child_idx = 0; child_idx < param.children->size();
               ++child_idx) {
            todo.push_back({(*param.children)[child_idx],
                            join,
                            /*eligible_for_batch_mode=*/false,
                            &job.children[child_idx],
                            {}});
          }
          continue;
        }
        // TODO(sgunders): Consider just sending in the array here,
        // changing types in the constructor.
        Mem_root_array<unique_ptr_destroy_only<RowIterator>> children(mem_root);
        children.reserve(param.children->size());
        for (unique_ptr_destroy_only<RowIterator> &child : job.children) {
          children.push_back(move(child));
        }
        iterator = NewIterator<RowIDUnionIterator>(
            thd, mem_root, mem_root, param.table, std::move(children));
        break;
      }
      case AccessPath::INDEX_SKIP_SCAN: {
        const IndexSkipScanParameters *param = path->index_skip_scan().param;
        iterator = NewIterator<IndexSkipScanIterator>(
            thd, mem_root, path->index_skip_scan().table, param->index_info,
            path->index_skip_scan().index, param->eq_prefix_len,
            param->eq_prefix_key_parts, param->eq_prefixes,
            path->index_skip_scan().num_used_key_parts, mem_root,
            param->has_aggregate_function, param->min_range_key,
            param->max_range_key, param->min_search_key, param->max_search_key,
            param->range_cond_flag, param->range_key_len);
        break;
      }
      case AccessPath::GROUP_INDEX_SKIP_SCAN: {
        const GroupIndexSkipScanParameters *param =
            path->group_index_skip_scan().param;
        iterator = NewIterator<GroupIndexSkipScanIterator>(
            thd, mem_root, path->group_index_skip_scan().table,
            &param->min_functions, &param->max_functions,
            param->have_agg_distinct, param->min_max_arg_part,
            param->group_prefix_len, param->group_key_parts,
            param->real_key_parts, param->max_used_key_length,
            param->index_info, path->group_index_skip_scan().index,
            param->key_infix_len, mem_root, param->is_index_scan,
            &param->prefix_ranges, &param->key_infix_ranges,
            &param->min_max_ranges);
        break;
      }
      case AccessPath::DYNAMIC_INDEX_RANGE_SCAN: {
        const auto &param = path->dynamic_index_range_scan();
        iterator = NewIterator<DynamicRangeIterator>(
            thd, mem_root, param.table, param.qep_tab, examined_rows);
        break;
      }
      case AccessPath::TABLE_SAMPLE:
        iterator = NewIterator<TableSampleIterator>(
            thd, mem_root, path->table_sample().table, 
            path->table_sample().qep_tab, path->num_output_rows,
            examined_rows);
        break;
      case AccessPath::TABLE_VALUE_CONSTRUCTOR: {
        assert(join != nullptr);
        Query_block *query_block = join->query_block;
        iterator = NewIterator<TableValueConstructorIterator>(
            thd, mem_root, examined_rows, *query_block->row_value_list,
            query_block->join->fields);
        break;
      }
      case AccessPath::FAKE_SINGLE_ROW:
        iterator =
            NewIterator<FakeSingleRowIterator>(thd, mem_root, examined_rows);
        break;
      case AccessPath::ZERO_ROWS: {
        iterator = NewIterator<ZeroRowsIterator>(thd, mem_root,
                                                 CollectTables(thd, path));
        break;
      }
      case AccessPath::ZERO_ROWS_AGGREGATED:
        iterator = NewIterator<ZeroRowsAggregatedIterator>(thd, mem_root, join,
                                                           examined_rows);
        break;
      case AccessPath::MATERIALIZED_TABLE_FUNCTION: {
        const auto &param = path->materialized_table_function();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.table_path, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = NewIterator<MaterializedTableFunctionIterator>(
            thd, mem_root, param.table_function, param.table,
            move(job.children[0]));
        break;
      }
      case AccessPath::UNQUALIFIED_COUNT:
        iterator = NewIterator<UnqualifiedCountIterator>(thd, mem_root, join);
        break;
      case AccessPath::NESTED_LOOP_JOIN: {
        const auto &param = path->nested_loop_join();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.outer, param.inner, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }

        iterator = NewIterator<NestedLoopIterator>(
            thd, mem_root, move(job.children[0]), move(job.children[1]),
            param.join_type, param.pfs_batch_mode);
        iterator->adjust_children();
        break;
      }
      case AccessPath::NESTED_LOOP_SEMIJOIN_WITH_DUPLICATE_REMOVAL: {
        const auto &param = path->nested_loop_semijoin_with_duplicate_removal();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.outer, param.inner, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = NewIterator<NestedLoopSemiJoinWithDuplicateRemovalIterator>(
            thd, mem_root, move(job.children[0]), move(job.children[1]),
            param.table, param.key, param.key_len);
        break;
      }
      case AccessPath::BKA_JOIN: {
        const auto &param = path->bka_join();
        AccessPath *mrr_path =
            FindSingleAccessPathOfType(param.inner, AccessPath::MRR);
        if (job.children.is_null()) {
          mrr_path->mrr().bka_path = path;
          SetupJobsForChildren(mem_root, param.outer, param.inner, join,
                               /*inner_eligible_for_batch_mode=*/false, &job,
                               &todo);
          continue;
        }

        MultiRangeRowIterator *mrr_iterator =
            down_cast<MultiRangeRowIterator *>(
                mrr_path->iterator->real_iterator());
        iterator = NewIterator<BKAIterator>(
            thd, mem_root, move(job.children[0]),
            GetUsedTables(param.outer, /*include_pruned_tables=*/true),
            move(job.children[1]), thd->variables.join_buff_size,
            param.mrr_length_per_rec, param.rec_per_key, param.store_rowids,
            param.tables_to_get_rowid_for, mrr_iterator, param.join_type);
        iterator->adjust_children();
        break;
      }
      case AccessPath::HASH_JOIN: {
        const auto &param = path->hash_join();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.outer, param.inner, join,
                               /*inner_eligible_for_batch_mode=*/true, &job,
                               &todo);
          continue;
        }
        const JoinPredicate *join_predicate = param.join_predicate;
        vector<HashJoinCondition> conditions;
        for (Item_func_eq *cond : join_predicate->expr->equijoin_conditions) {
          conditions.emplace_back(HashJoinCondition(cond, thd->mem_root));
        }
        const bool probe_input_batch_mode =
            eligible_for_batch_mode && ShouldEnableBatchMode(param.outer);
        double estimated_build_rows = param.inner->num_output_rows;
        if (param.inner->num_output_rows < 0.0) {
          // Not all access paths may propagate their costs properly.
          // Choose a fairly safe estimate (it's better to be too large
          // than too small).
          estimated_build_rows = 1048576.0;
        }
        JoinType join_type{JoinType::INNER};
        switch (join_predicate->expr->type) {
          case RelationalExpression::INNER_JOIN:
          case RelationalExpression::STRAIGHT_INNER_JOIN:
            join_type = JoinType::INNER;
            break;
          case RelationalExpression::LEFT_JOIN:
            join_type = JoinType::OUTER;
            break;
          case RelationalExpression::ANTIJOIN:
            join_type = JoinType::ANTI;
            break;
          case RelationalExpression::SEMIJOIN:
            join_type =
                param.rewrite_semi_to_inner ? JoinType::INNER : JoinType::SEMI;
            break;
          case RelationalExpression::TABLE:
          default:
            assert(false);
        }
        // See if we can allow the hash table to keep its contents across Init()
        // calls.
        //
        // The old optimizer will sometimes push join conditions referring
        // to outer tables (in the same query block) down in under the hash
        // operation, so without analysis of each filter and join condition, we
        // cannot say for sure, and thus have to turn it off. But the hypergraph
        // optimizer sets parameter_tables properly, so we're safe if we just
        // check that.
        //
        // Regardless of optimizer, we can push outer references down in under
        // the hash, but join->hash_table_generation will increase whenever we
        // need to recompute the query block (in JOIN::clear_hash_tables()).
        //
        // TODO(sgunders): The old optimizer had a concept of _when_ to clear
        // derived tables (invalidators), and this is somehow similar. If it
        // becomes a performance issue, consider reintroducing them.
        //
        // TODO(sgunders): Should this perhaps be set as a flag on the access
        // path instead of being computed here? We do make the same checks in
        // the cost model, so perhaps it should set the flag as well.
        uint64_t *hash_table_generation =
            (thd->lex->using_hypergraph_optimizer &&
             path->parameter_tables == 0)
                ? &join->hash_table_generation
                : nullptr;

        iterator = NewIterator<HashJoinIterator>(
            thd, mem_root, move(job.children[1]),
            GetUsedTables(param.inner, /*include_pruned_tables=*/true),
            estimated_build_rows, move(job.children[0]),
            GetUsedTables(param.outer, /*include_pruned_tables=*/true),
            param.store_rowids, param.tables_to_get_rowid_for,
            thd->variables.join_buff_size, move(conditions),
            param.allow_spill_to_disk, join_type,
            join_predicate->expr->join_conditions, probe_input_batch_mode,
            hash_table_generation);
        iterator->adjust_children();
        break;
      }
      case AccessPath::SORT_MERGE_JOIN: {
        const JoinPredicate *join_predicate = path->sort_merge_join().join_predicate;
        unique_ptr_destroy_only<RowIterator> outer = CreateIteratorFromAccessPath(
            thd, path->sort_merge_join().outer, join, eligible_for_batch_mode);
        unique_ptr_destroy_only<RowIterator> inner = CreateIteratorFromAccessPath(
            thd, path->sort_merge_join().inner, join, /*eligible_for_batch_mode=*/true);
        vector<HashJoinCondition> conditions;
        for (Item_func_eq *cond : join_predicate->expr->equijoin_conditions)
          conditions.emplace_back(HashJoinCondition(cond, thd->mem_root));

        JoinType join_type{JoinType::INNER};
        switch (join_predicate->expr->type) {
          case RelationalExpression::INNER_JOIN:
          case RelationalExpression::STRAIGHT_INNER_JOIN:
            join_type = JoinType::INNER;
            break;
          case RelationalExpression::LEFT_JOIN:
            join_type = JoinType::OUTER;
            break;
          case RelationalExpression::ANTIJOIN:
            join_type = JoinType::ANTI;
            break;
          case RelationalExpression::SEMIJOIN:
            join_type =
              path->sort_merge_join().rewrite_semi_to_inner ?
                JoinType::INNER : JoinType::SEMI;
            break;
          case RelationalExpression::TABLE:
          default:
            assert(false);
        }

        const bool probe_input_batch_mode =
            eligible_for_batch_mode &&
            ShouldEnableBatchMode(path->sort_merge_join().inner);
        iterator = NewIterator<SortMergeJoinIterator>(
            thd, thd->mem_root, move(outer), 
            GetUsedTables(path->sort_merge_join().outer, true),
            move(inner),
            GetUsedTables(path->sort_merge_join().inner, true),
            path->sort_merge_join().store_rowids,
            path->sort_merge_join().tables_to_get_rowid_for,
            thd->variables.merge_join_buff_size, move(conditions),
            thd->variables.merge_join_buff_size > 0 ? true : false,
            join_type, join,
            join_predicate->expr->join_conditions,
            probe_input_batch_mode);
        break;
      }
      case AccessPath::FILTER: {
        const auto &param = path->filter();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        if (FinalizeMaterializedSubqueries(thd, join, path)) {
          return nullptr;
        }
        iterator = NewIterator<FilterIterator>(
            thd, mem_root, move(job.children[0]), param.condition);
        iterator->adjust_children();
        break;
      }
      case AccessPath::SORT: {
        const auto &param = path->sort();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        ha_rows num_rows_estimate = param.child->num_output_rows < 0.0
                                        ? HA_POS_ERROR
                                        : lrint(param.child->num_output_rows);
        Filesort *filesort = param.filesort;
        iterator = NewIterator<SortingIterator>(
            thd, mem_root, filesort, move(job.children[0]), num_rows_estimate,
            param.tables_to_get_rowid_for, examined_rows);
        iterator->adjust_children();
        if (filesort->m_remove_duplicates) {
          filesort->tables[0]->duplicate_removal_iterator =
              down_cast<SortingIterator *>(iterator->real_iterator());
        } else {
          filesort->tables[0]->sorting_iterator =
              down_cast<SortingIterator *>(iterator->real_iterator());
        }
        break;
      }
      case AccessPath::AGGREGATE: {
        const auto &param = path->aggregate();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        Prealloced_array<TABLE *, 4> tables =
            GetUsedTables(param.child, /*include_pruned_tables=*/true);
        iterator = NewIterator<AggregateIterator>(
            thd, mem_root, move(job.children[0]), join,
            TableCollection(tables, /*store_rowids=*/false,
                            /*tables_to_get_rowid_for=*/0),
            param.rollup, param.is_final_aggr);
        iterator->adjust_children();
        break;
      }
      case AccessPath::TEMPTABLE_AGGREGATE: {
        const auto &param = path->temptable_aggregate();
        if (job.children.is_null()) {
          job.AllocChildren(mem_root, 2);
          todo.push_back(job);
          todo.push_back({param.subquery_path,
                          join,
                          /*eligible_for_batch_mode=*/true,
                          &job.children[0],
                          {}});
          todo.push_back({param.table_path,
                          join,
                          eligible_for_batch_mode,
                          &job.children[1],
                          {}});
          continue;
        }

        iterator = unique_ptr_destroy_only<RowIterator>(
            temptable_aggregate_iterator::CreateIterator(
                thd, move(job.children[0]), param.temp_table_param, param.table,
                move(job.children[1]), join, param.ref_slice, param.is_final_aggr));
        iterator->adjust_children();
        break;
      }
      case AccessPath::LIMIT_OFFSET: {
        const auto &param = path->limit_offset();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        ha_rows *send_records = nullptr;
        if (param.send_records_override != nullptr) {
          send_records = param.send_records_override;
        } else if (join != nullptr) {
          send_records = &join->send_records;
        }
        iterator = NewIterator<LimitOffsetIterator>(
            thd, mem_root, move(job.children[0]), param.limit, param.offset,
            param.count_all_rows, param.reject_multiple_rows, send_records);
        iterator->adjust_children();
        break;
      }
      case AccessPath::STREAM: {
        const auto &param = path->stream();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, param.join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = NewIterator<StreamingIterator>(
            thd, mem_root, move(job.children[0]), param.temp_table_param,
            param.table, param.provide_rowid, param.join, param.ref_slice);
        iterator->adjust_children();
        break;
      }
      case AccessPath::MATERIALIZE: {
        // The table access path should be a single iterator, not a tree.
        // (ALTERNATIVE counts as a single iterator in this regard.)
        assert(
            path->materialize().table_path->type == AccessPath::TABLE_SCAN ||
            path->materialize().table_path->type == AccessPath::REF ||
            path->materialize().table_path->type == AccessPath::REF_OR_NULL ||
            path->materialize().table_path->type == AccessPath::EQ_REF ||
            path->materialize().table_path->type == AccessPath::ALTERNATIVE ||
            path->materialize().table_path->type == AccessPath::CONST_TABLE ||
            path->materialize().table_path->type == AccessPath::INDEX_SCAN ||
            path->materialize().table_path->type ==
                AccessPath::INDEX_RANGE_SCAN);

        MaterializePathParameters *param = path->materialize().param;
        if (job.children.is_null()) {
          job.AllocChildren(mem_root, param->query_blocks.size() + 1);
          todo.push_back(job);
          todo.push_back({path->materialize().table_path,
                          join,
                          eligible_for_batch_mode,
                          &job.children[0],
                          {}});
          for (size_t i = 0; i < param->query_blocks.size(); ++i) {
            const MaterializePathParameters::QueryBlock &from =
                param->query_blocks[i];
            todo.push_back({from.subquery_path,
                            from.join,
                            /*eligible_for_batch_mode=*/true,
                            &job.children[i + 1],
                            {}});
          }
          continue;
        }
        unique_ptr_destroy_only<RowIterator> table_iterator =
            move(job.children[0]);
        Mem_root_array<materialize_iterator::QueryBlock> query_blocks(
            thd->mem_root, param->query_blocks.size());
        for (size_t i = 0; i < param->query_blocks.size(); ++i) {
          const MaterializePathParameters::QueryBlock &from =
              param->query_blocks[i];
          materialize_iterator::QueryBlock &to = query_blocks[i];
          to.subquery_iterator = move(job.children[i + 1]);
          to.select_number = from.select_number;
          to.join = from.join;
          to.disable_deduplication_by_hash_field =
              from.disable_deduplication_by_hash_field;
          to.copy_items = from.copy_items;
          to.temp_table_param = from.temp_table_param;
          to.is_recursive_reference = from.is_recursive_reference;

          if (to.is_recursive_reference) {
            // Find the recursive reference to ourselves; there should be
            // exactly one, as per the standard.
            RowIterator *recursive_reader = FindSingleIteratorOfType(
                from.subquery_path, AccessPath::FOLLOW_TAIL);
            if (recursive_reader == nullptr) {
              // The recursive reference was optimized away, e.g. due to an
              // impossible WHERE condition, so we're not a recursive
              // reference after all.
              to.is_recursive_reference = false;
            } else {
              to.recursive_reader =
                  down_cast<FollowTailIterator *>(recursive_reader);
            }
          }
        }
        JOIN *subjoin = param->ref_slice == -1 ? nullptr : query_blocks[0].join;

        iterator = unique_ptr_destroy_only<RowIterator>(
            materialize_iterator::CreateIterator(thd, std::move(query_blocks),
                                                 param, move(table_iterator),
                                                 subjoin));
        iterator->adjust_children();
        break;
      }
      case AccessPath::MATERIALIZE_INFORMATION_SCHEMA_TABLE: {
        const auto &param = path->materialize_information_schema_table();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.table_path, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = NewIterator<MaterializeInformationSchemaTableIterator>(
            thd, mem_root, move(job.children[0]), param.table_list,
            param.condition);
        iterator->adjust_children();
        break;
      }
      case AccessPath::APPEND: {
        const auto &param = path->append();
        if (job.children.is_null()) {
          job.AllocChildren(mem_root, param.children->size());
          todo.push_back(job);
          for (size_t child_idx = 0; child_idx < param.children->size();
               ++child_idx) {
            const AppendPathParameters &child_param =
                (*param.children)[child_idx];
            todo.push_back({child_param.path,
                            child_param.join,
                            /*eligible_for_batch_mode=*/true,
                            &job.children[child_idx],
                            {}});
          }
          continue;
        }
        // TODO(sgunders): Consider just sending in the array here,
        // changing types in the constructor.
        vector<unique_ptr_destroy_only<RowIterator>> children;
        children.reserve(param.children->size());
        for (unique_ptr_destroy_only<RowIterator> &child : job.children) {
          children.push_back(move(child));
        }
        iterator = NewIterator<AppendIterator>(thd, mem_root, move(children));
        iterator->adjust_children();
        break;
      }
      case AccessPath::WINDOW: {
        const auto &param = path->window();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        if (param.needs_buffering) {
          iterator = NewIterator<BufferingWindowIterator>(
              thd, mem_root, move(job.children[0]), param.temp_table_param,
              join, param.ref_slice);
        } else {
          iterator = NewIterator<WindowIterator>(
              thd, mem_root, move(job.children[0]), param.temp_table_param,
              join, param.ref_slice);
        }
        break;
      }
      case AccessPath::WEEDOUT: {
        const auto &param = path->weedout();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = NewIterator<WeedoutIterator>(
            thd, mem_root, move(job.children[0]), param.weedout_table,
            param.tables_to_get_rowid_for);
        iterator->adjust_children();
        break;
      }
      case AccessPath::REMOVE_DUPLICATES: {
        const auto &param = path->remove_duplicates();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = NewIterator<RemoveDuplicatesIterator>(
            thd, mem_root, move(job.children[0]), join, param.group_items,
            param.group_items_size);
        iterator->adjust_children();
        break;
      }
      case AccessPath::REMOVE_DUPLICATES_ON_INDEX: {
        const auto &param = path->remove_duplicates_on_index();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = NewIterator<RemoveDuplicatesOnIndexIterator>(
            thd, mem_root, move(job.children[0]), param.table, param.key,
            param.loosescan_key_len);
        break;
      }
      case AccessPath::ALTERNATIVE: {
        const auto &param = path->alternative();
        if (job.children.is_null()) {
          job.AllocChildren(mem_root, 2);
          todo.push_back(job);
          todo.push_back({param.child,
                          join,
                          eligible_for_batch_mode,
                          &job.children[0],
                          {}});
          todo.push_back({param.table_scan_path,
                          join,
                          eligible_for_batch_mode,
                          &job.children[1],
                          {}});
          continue;
        }
        iterator = NewIterator<AlternativeIterator>(
            thd, mem_root, param.table_scan_path->table_scan().table,
            move(job.children[0]), move(job.children[1]), param.used_ref);
        break;
      }
      case AccessPath::CACHE_INVALIDATOR: {
        const auto &param = path->cache_invalidator();
        if (job.children.is_null()) {
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = NewIterator<CacheInvalidatorIterator>(
            thd, mem_root, move(job.children[0]), param.name);
        break;
      }
      case AccessPath::DELETE_ROWS: {
        const auto &param = path->delete_rows();
        if (job.children.is_null()) {
          // Setting up tables for delete must be done before the child
          // iterators are created, as some of the child iterators need to see
          // the final read set when they are constructed, so doing it in
          // DeleteRowsIterator's constructor or Init() is too late.
          SetUpTablesForDelete(thd, join);
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = NewIterator<DeleteRowsIterator>(
            thd, mem_root, move(job.children[0]), join,
            param.tables_to_delete_from, param.immediate_tables);
        break;
      }
      case AccessPath::UPDATE_ROWS: {
        const auto &param = path->update_rows();
        if (job.children.is_null()) {
          // Do the final setup for UPDATE before the child iterators are
          // created.
          if (FinalizeOptimizationForUpdate(join)) {
            return nullptr;
          }
          SetupJobsForChildren(mem_root, param.child, join,
                               eligible_for_batch_mode, &job, &todo);
          continue;
        }
        iterator = CreateUpdateRowsIterator(thd, mem_root, join,
                                            std::move(job.children[0]));
        break;
      }
      case AccessPath::PX_RECEIVE: {
        unique_ptr_destroy_only<RowIterator> child = CreateIteratorFromAccessPath(
            thd, mem_root, path->px_receiver().child, join, eligible_for_batch_mode);
        iterator = NewIterator<PX_receiver>(thd, mem_root, 0, nullptr, join, move(child),
            path->px_receiver().table, path->px_receiver().ref_slice);
        iterator->adjust_children();
        break;
      }
      case AccessPath::PX_SEND: {
        if (path->px_send().table_path) {
          unique_ptr_destroy_only<RowIterator> table_iterator =
              CreateIteratorFromAccessPath(thd, path->px_send().table_path,
                                        join, eligible_for_batch_mode);
          unique_ptr_destroy_only<RowIterator> child = CreateIteratorFromAccessPath(
              thd, mem_root, path->px_send().child, join, eligible_for_batch_mode);

          iterator = NewIterator<PX_sender>(
              thd, mem_root, 0, nullptr, move(child), path->px_send().table,
              path->px_send().fields, nullptr, path->px_send().temp_table_param,
              move(table_iterator));
          iterator->adjust_children();
        } else {
          unique_ptr_destroy_only<RowIterator> child = CreateIteratorFromAccessPath(
              thd, mem_root, path->px_send().child, join, eligible_for_batch_mode);
          iterator = NewIterator<PX_sender>(
              thd, mem_root, 0, nullptr, move(child), path->px_send().table,
              path->px_send().fields, nullptr, path->px_send().temp_table_param,
              nullptr);
          iterator->adjust_children();
        }
        break;
      }
      case AccessPath::PX_RECEIVER_MERGE: {
        unique_ptr_destroy_only<RowIterator> child = CreateIteratorFromAccessPath(
            thd, mem_root, path->px_receiver_merge().child, join, eligible_for_batch_mode);
        iterator = NewIterator<PX_receiver_merge>(thd, mem_root, 0, nullptr, path->px_receiver_merge().table,
            path->px_receiver_merge().filesort, move(child), join, path->px_receiver_merge().ref_slice);
        iterator->adjust_children();
        break;
      }
    }

    if (iterator == nullptr) {
      return nullptr;
    }

    path->iterator = iterator.get();
    *job.destination = std::move(iterator);
  }
  return ret;
}

void FindTablesToGetRowidFor(AccessPath *path) {
  table_map handled_by_others = 0;

  auto add_tables_handled_by_others = [path, &handled_by_others](
                                          AccessPath *subpath, const JOIN *) {
    if (path == subpath) return false;  // Skip ourselves.
    switch (subpath->type) {
      case AccessPath::HASH_JOIN:
        handled_by_others |=
            GetUsedTableMap(subpath, /*include_pruned_tables=*/true);
        FindTablesToGetRowidFor(subpath);
        return true;  // Don't double-traverse.
      case AccessPath::SORT_MERGE_JOIN:
        handled_by_others |= GetUsedTableMap(subpath, /*include_pruned_tables=*/true);
        FindTablesToGetRowidFor(subpath);
        return true;  // Don't double-traverse.
      case AccessPath::BKA_JOIN:
        handled_by_others |= GetUsedTableMap(subpath->bka_join().outer,
                                             /*include_pruned_tables=*/true);
        FindTablesToGetRowidFor(subpath);
        return true;  // Don't double-traverse.
      case AccessPath::STREAM: {
        subpath->stream().provide_rowid = true;
        TABLE *table = subpath->stream().table;
        if (table->pos_in_table_list == nullptr) {
          // Don't need to set anything; see comment on the similar
          // test in NewSortAccessPath().
        } else {
          handled_by_others |= table->pos_in_table_list->map();
        }
        // Doesn't really matter, we don't cross query blocks anyway.
        return true;
      }
      default:
        return false;
    }
  };

  // We stop at MATERIALIZE and STREAM (they supply row IDs for us without
  // having to ask the tables below).
  switch (path->type) {
    case AccessPath::HASH_JOIN:
      WalkAccessPaths(path, /*join=*/nullptr,
                      WalkAccessPathPolicy::STOP_AT_MATERIALIZATION,
                      add_tables_handled_by_others);
      path->hash_join().store_rowids = true;
      path->hash_join().tables_to_get_rowid_for =
          GetUsedTableMap(path, /*include_pruned_tables=*/true) &
          ~handled_by_others;
      break;
    case AccessPath::SORT_MERGE_JOIN:
      WalkAccessPaths(path, /*join=*/nullptr,
                      WalkAccessPathPolicy::STOP_AT_MATERIALIZATION,
                      add_tables_handled_by_others);
      path->sort_merge_join().store_rowids = true;
      path->sort_merge_join().tables_to_get_rowid_for =
          GetUsedTableMap(path, /*include_pruned_tables=*/true) &
          ~handled_by_others;
      break;
    case AccessPath::BKA_JOIN:
      WalkAccessPaths(path->bka_join().outer, /*join=*/nullptr,
                      WalkAccessPathPolicy::STOP_AT_MATERIALIZATION,
                      add_tables_handled_by_others);
      path->bka_join().store_rowids = true;
      path->bka_join().tables_to_get_rowid_for =
          GetUsedTableMap(path->bka_join().outer,
                          /*include_pruned_tables=*/true) &
          ~handled_by_others;
      break;
    case AccessPath::WEEDOUT:
      WalkAccessPaths(path, /*join=*/nullptr,
                      WalkAccessPathPolicy::STOP_AT_MATERIALIZATION,
                      add_tables_handled_by_others);
      path->weedout().tables_to_get_rowid_for =
          GetUsedTableMap(path, /*include_pruned_tables=*/true) &
          ~handled_by_others;
      break;
    case AccessPath::SORT:
      WalkAccessPaths(path, /*join=*/nullptr,
                      WalkAccessPathPolicy::STOP_AT_MATERIALIZATION,
                      add_tables_handled_by_others);
      path->sort().tables_to_get_rowid_for =
          GetUsedTableMap(path, /*include_pruned_tables=*/true) &
          ~handled_by_others;
      break;
    default:
      my_abort();
  }
}

static Item *ConditionFromFilterPredicates(
    const Mem_root_array<Predicate> &predicates, OverflowBitset mask,
    int num_where_predicates) {
  List<Item> items;
  for (int pred_idx : BitsSetIn(mask)) {
    if (pred_idx >= num_where_predicates) break;
    items.push_back(predicates[pred_idx].condition);
  }
  return CreateConjunction(&items);
}

void ExpandSingleFilterAccessPath(THD *thd, AccessPath *path, const JOIN *join,
                                  const Mem_root_array<Predicate> &predicates,
                                  unsigned num_where_predicates) {
  // Expand join filters for nested loop joins.
  if (path->type == AccessPath::NESTED_LOOP_JOIN &&
      !path->nested_loop_join().already_expanded_predicates &&
      !(path->nested_loop_join().equijoin_predicates.empty() &&
        path->nested_loop_join()
            .join_predicate->expr->join_conditions.empty()) &&
      path->nested_loop_join().inner->type != AccessPath::ZERO_ROWS) {
    AccessPath *right_path = path->nested_loop_join().inner;
    const RelationalExpression *expr =
        path->nested_loop_join().join_predicate->expr;

    // While we're collecting the join conditions, calculate cost and output
    // rows (purely for display purposes). Note that this mirrors the
    // calculation we are doing in CostingReceiver::ProposeNestedLoopJoin();
    // we don't have space in the AccessPath to store it there.
    double filter_cost = right_path->cost;
    double filter_rows = right_path->num_output_rows;

    List<Item> items;
    for (size_t filter_idx :
         BitsSetIn(path->nested_loop_join().equijoin_predicates)) {
      Item *condition = expr->equijoin_conditions[filter_idx];
      items.push_back(condition);
      filter_cost +=
          EstimateFilterCost(thd, filter_rows, condition, join->query_block)
              .cost_if_not_materialized;
      filter_rows *= EstimateSelectivity(thd, condition, /*trace=*/nullptr);
    }
    for (Item *condition : expr->join_conditions) {
      items.push_back(condition);
      filter_cost +=
          EstimateFilterCost(thd, filter_rows, condition, join->query_block)
              .cost_if_not_materialized;
      filter_rows *= EstimateSelectivity(thd, condition, /*trace=*/nullptr);
    }
    assert(!items.is_empty());

    AccessPath *filter_path = new (thd->mem_root) AccessPath;
    filter_path->type = AccessPath::FILTER;
    filter_path->filter().child = right_path;

    // We don't bother trying to materialize subqueries in join conditions,
    // since they should be very rare.
    filter_path->filter().materialize_subqueries = false;

    CopyBasicProperties(*right_path, filter_path);
    filter_path->filter().condition = CreateConjunction(&items);
    filter_path->cost = filter_cost;
    filter_path->num_output_rows = filter_rows;

    path->nested_loop_join().inner = filter_path;

    // Since multiple root paths may have their filters expanded,
    // and the same nested loop may be a subpath in several
    // of them, we need to make sure we don't add the join predicates
    // more than once, so mark them as done here.
    path->nested_loop_join().already_expanded_predicates = true;
  }

  // Expand filters _after_ the access path (these are much more common).
  Item *condition = ConditionFromFilterPredicates(
      predicates, path->filter_predicates, num_where_predicates);
  if (condition == nullptr) {
    return;
  }
  AccessPath *new_path = new (thd->mem_root) AccessPath(*path);
  new_path->filter_predicates.Clear();
  new_path->num_output_rows = path->num_output_rows_before_filter;
  new_path->cost = path->cost_before_filter;

  // We don't really know how much of init_cost comes from the filter,
  // but we need to heed the invariant that cost >= init_cost
  // also for the new (non-filter) path we're creating, even if it's
  // just for display. Heuristically allocate as much as possible to
  // the filter.
  double filter_only_cost = path->cost - path->cost_before_filter;
  new_path->init_cost = std::max(new_path->init_cost - filter_only_cost, 0.0);
  new_path->init_once_cost =
      std::max(new_path->init_once_cost - filter_only_cost, 0.0);
  assert(new_path->cost >= new_path->init_cost);
  assert(new_path->init_cost >= new_path->init_once_cost);

  path->type = AccessPath::FILTER;
  path->filter().condition = condition;
  path->filter().child = new_path;
  path->filter().materialize_subqueries = false;

  // Clear filter_predicates, but keep applied_sargable_join_predicates.
  MutableOverflowBitset applied_sargable_join_predicates =
      path->applied_sargable_join_predicates().Clone(thd->mem_root);
  applied_sargable_join_predicates.ClearBits(0, num_where_predicates);
  path->filter_predicates = std::move(applied_sargable_join_predicates);
}

void ExpandFilterAccessPaths(THD *thd, AccessPath *path_arg, const JOIN *join,
                             const Mem_root_array<Predicate> &predicates,
                             unsigned num_where_predicates) {
  WalkAccessPaths(path_arg, join, WalkAccessPathPolicy::ENTIRE_QUERY_BLOCK,
                  [thd, &predicates, num_where_predicates](
                      AccessPath *path, const JOIN *sub_join) {
                    ExpandSingleFilterAccessPath(
                        thd, path, sub_join, predicates, num_where_predicates);
                    return false;
                  });
}

table_map GetHashJoinTables(AccessPath *path) {
  table_map tables = 0;
  WalkAccessPaths(
      path, /*join=*/nullptr, WalkAccessPathPolicy::STOP_AT_MATERIALIZATION,
      [&tables](AccessPath *subpath, const JOIN *) {
        if (subpath->type == AccessPath::HASH_JOIN) {
          tables |= GetUsedTableMap(subpath, /*include_pruned_tables=*/true);
          return true;
        }
        return false;
      });
  return tables;
}

/**
  Walk through accespath to inject final_aggregate.
  This recursive will be stopped when we find Aggregate or there
  is no aggregate in accesspath.
  If find Aggregate or TemptableAggregate:
  [1] Rebuild Aggregate/TemptableAggregate(function: Rebuild__AccessPath).
    a. rebuild items in fields, sum_funcs, tmp_fields, ref_items.
    b. create tmp_table to store results
  [2] Build final Aggregate/TemptableAggregate(function: BuildFinal__AccessPath).
    a. build items for final aggregate, include final_aggr_sum_funcs,
       fields, tmp_fields, ref_items.
    b. create tmp_table for final aggregate if needed.
    c. create new accesspath.

  @param thd
  @param join
  @param path
  @return new accesspath
*/
AccessPath *WalkAccessPathsForAggregationRebuild(THD *thd, JOIN *join,
                                                AccessPath *const path) {
  AccessPath *newFinalAggrPath = nullptr;
  AccessPath *child = nullptr;
  AccessPath *newChild = nullptr;
  bool do_inject = false;
  switch (path->type) {
    case AccessPath::AGGREGATE: {
      do_inject = true;
      uint avg_count = 0;
      uint curr_slice = 0;
      if (!(join->implicit_grouping || join->group_optimized_away) &&
          !thd->lex->using_hypergraph_optimizer) {
        curr_slice = join->get_ref_item_slice();
      }

      // check all sum_funcs are support
      if (check_sum_func_support(thd, join)) {
        do_inject = false;
        break;
      }

      // [1] rebuild Aggregate
      if (RebuildAggregateAccessPath(thd, join, path, curr_slice, &avg_count)) {
        assert(false);
      }
      // [2] create final Aggregate accesspath
      newFinalAggrPath = BuildFinalAggregateAccessPath(thd, join, path, curr_slice,
                                                       avg_count);
      if (newFinalAggrPath == nullptr) {
        assert(false);
      }
      break;
    }
    case AccessPath::TEMPTABLE_AGGREGATE: {
      do_inject = true;
      uint avg_count = 0;
      uint curr_slice = path->temptable_aggregate().ref_slice;

      // check all sum_funcs are support
      if (check_sum_func_support(thd, join)) {
        do_inject = false;
        break;
      }
      
      // [1] rebuild temptableAggregate
      if (RebuildTempAggregateAccessPath(thd, join, path, curr_slice, &avg_count)) {
        assert(false);
      }
      // save current slice ref_items, it will be used in exchange inject.
      if(join->alloc_ref_item_slice(thd, REF_SLICE_SAVED_TMP1)) assert(false);
      join->copy_ref_item_slice(REF_SLICE_SAVED_TMP1, curr_slice);
      join->tmp_fields[REF_SLICE_SAVED_TMP1] = join->tmp_fields[curr_slice];
      // [2] create final temptableAggregate accesspath
      newFinalAggrPath = BuildFinalTempAggregateAccessPath(thd, join, path, curr_slice,
                                                           avg_count);
      if (newFinalAggrPath == nullptr) {
        assert(false);
      }
      break;
    }
    case AccessPath::FILTER: {
      child = path->filter().child;
      newChild = WalkAccessPathsForAggregationRebuild(thd, join, child);
      path->filter().child = newChild;
      break;
    }
    case AccessPath::LIMIT_OFFSET : {
      child = path->limit_offset().child;
      newChild = WalkAccessPathsForAggregationRebuild(thd, join, child);
      path->limit_offset().child = newChild;
      break;
    }
    case AccessPath::SORT : {
      child = path->sort().child;
      newChild = WalkAccessPathsForAggregationRebuild(thd, join, child);
      path->sort().child = newChild;
      FixSortAccessPathForAggrInject(thd, join, path, REF_SLICE_FINAL_AGGREGATE);
      break;
    }
    default:
      break;
  }

  return do_inject ? newFinalAggrPath : path;
}

/**
  Restore ref_items, keep number and position of ref_item no change.
  
  For example:
    select avg(col1), avg(col2)/sum(col3) from t;
  
  JOIN::saved_base_fields
  |------hidden-------|   |-------visible-------|
  +---------+---------+ # +---------+-----------+
  |sum(col3)|avg(col2)| # |avg(col1)|func_div1()|
  +---------+---------+ # +---------+-----------+
  
  [1] If it's not final aggr
  JOIN::fields:
  |------------hidden-------------|   |-------------visible-------------|
  +---------+---------+-----------+ # +---------+-----------+-----------+
  |sum(col3)|sum(col2)|count(col2)| # |sum(col1)|count(col1)|func_div1()|
  +---|-----+---------+-----------+ # +---------+-----------+------|----+
      |                                                            |
       \---------------->--------------\                           |
                    /-------------------\----<---------------------/
  JOIN::ref_items:  |                    \
  +---------+-------|---+ # +---------+---|-----+
  |avg(col1)|func_div1()| # |avg(col2)|sum(col3)|
  +---------+-----------+ # +---------+---------+
  |-------visible-------|   |------hidden-------|

  [2] If it's final aggr
  JOIN::fields:
  |------------------------------hidden-----------------------------|   |-------visible---------|
  +---------+---------+-----------+-----------+---------+-----------+ # +-----------+-----------+
  |sum(col3)|sum(col2)|count(col2)|func_div2()|sum(col1)|count(col1)| # |func_div3()|func_div1()|
  +---|-----+---------+-----------+-----|-----+---------+-----------+ # +-----------+-----------+
      |                                 v                               \_ _ _ _ _ _ _ _ _ _ _ _/     
       \           /--------------------|----------------<--------------------------/
        \----------|----->--------------|------\
                   |                    |      |
  JOIN::ref_items: |                    /      | 
  /-----------------------\            |       v 
  +-----------+-----------+ # +-----------+---------+
  |func_div3()|func_div1()| # |func_div2()|sum(col3)| 
  +-----------+-----------+ # +-----------+---------+
  |--------visible--------|   |-------hidden--------|

  @param thd
  @param join
  @param curr_slice current slice of ref_items.
*/
void RebuildCurrentRefItems(THD *thd, JOIN *join, uint curr_slice, bool is_final_aggr) {
  Ref_item_array &curr_ref_items = join->ref_items[curr_slice];
  uint fields_count = join->saved_base_fields->size();
  uint num_hidden_fields = CountHiddenFields(*join->saved_base_fields);
  uint ref_items_count = join->ref_items[0].size();
  Item **ref_item = thd->mem_root->ArrayAlloc<Item *>(ref_items_count);
  Ref_item_array tmp_ref_items = Ref_item_array(ref_item, ref_items_count);

  if (is_final_aggr) {
    uint i = 0, position = 0;
    for (;i < num_hidden_fields; ++i) {
      Item *item = (*join->saved_base_fields)[i];
      if (item->type() == Item::SUM_FUNC_ITEM) {
        Item_sum *item_sum = down_cast<Item_sum *>(item);
        if (item_sum->sum_func() == Item_sum::AVG_FUNC) {
          position += 2;
        } 
      }
      tmp_ref_items[fields_count - i - 1] = (*join->fields)[position];
      ++position;
    }
    for (Item *item : VisibleFields(*join->fields)) {
      tmp_ref_items[i - num_hidden_fields] = item;
      ++i;
    }
  } else {
    uint i = 0, j = 0;
    for (Item *item : *join->saved_base_fields) {
      if (item->type() == Item::SUM_FUNC_ITEM) {
        Item_sum *item_sum = down_cast<Item_sum *>(item);
        if (item_sum->sum_func() == Item_sum::AVG_FUNC) {
          tmp_ref_items[(item->hidden ? fields_count - i -1
                                      : i - num_hidden_fields)] = item_sum;
          ++j;
        } else {
          tmp_ref_items[(item->hidden ? fields_count - i -1
                                      : i - num_hidden_fields)] = (*join->fields)[j];
        }
      } else {
        tmp_ref_items[(item->hidden ? fields_count - i -1
                                    : i - num_hidden_fields)] = (*join->fields)[j];
      }
      ++i;
      ++j;
    }
  }

  curr_ref_items = tmp_ref_items;
}

/**
  Rebuild Aggregate.
  [1] rebuild aggregation functions in fields and tmp_fields.
  [2] restore tmp_table_param and calculate properties for param.
  [3] count of sum_funcs may change after rebuild aggregation functions,
      so we need re-alloc memory for sum_funcs.
  [4] re-create tmp_table for Aggregate.
  [5] call RebuildCurrentRefItems to rebuild ref_items for curr_slice
      after change_to_use_tmp_fields.

  @param thd
  @param join
  @param path
  @param curr_slice current slice in ref_items.

  @return false if successful, true if fail.
*/
bool RebuildAggregateAccessPath(THD *thd, JOIN *join, AccessPath *const path,
                                uint curr_slice, uint *avg_count) {

  List_item *curr_fields = nullptr;
  TABLE *tmp_table = nullptr;
  mem_root_deque<Item *> tmp_field(thd->mem_root);
  join->aggr_tmp_table_param = new (thd->mem_root) Temp_table_param(join->tmp_table_param);
  join->aggr_tmp_table_param->copy_fields.clear();
  //join->aggr_tmp_table_param->grouped_expressions.clear();

  // rebuild sum_funcs for Aggregate.
  if (join->check_and_rebuild_sum_funcs(thd, REF_SLICE_SAVED_BASE, avg_count)) {
    goto rebuild_err;
  }
  curr_fields = &join->tmp_fields[REF_SLICE_SAVED_BASE];

  count_field_types(join->query_block, join->aggr_tmp_table_param,
                    *curr_fields, /*reset_with_sum_func=*/false,
                    /*save_sum_fields=*/true);
  
  // re-alloc sum_funcs
  if (join->alloc_func_list_with_param(join->aggr_tmp_table_param,
        &join->sum_funcs)) goto rebuild_err;

  join->aggr_tmp_table_param->hidden_field_count =
      CountHiddenFields(*curr_fields);
  
  // create tmp table for first aggregate to save result.
  tmp_table = create_tmp_table(
      thd, join->aggr_tmp_table_param, *curr_fields, /*group=*/nullptr,
      /*distinct=*/false, /*save_sum_fields=*/true, join->query_block->active_options(),
      HA_POS_ERROR, "<temp>");
  if (!tmp_table) goto rebuild_err;
  join->aggr_tmp_table = tmp_table;

  // rebuild sum_func_list
  if (join->make_sum_func_list(*curr_fields, /*before_group_by=*/true, /*recompute=*/true))
   goto rebuild_err;

  if (change_to_use_tmp_fields(curr_fields, thd, join->ref_items[curr_slice],
                               &tmp_field, join->query_block->m_added_non_hidden_fields))
    goto rebuild_err;
  join->tmp_fields[curr_slice] = tmp_field;
  
  // reset join::fields to curr_slice items.
  join->fields = &join->tmp_fields[curr_slice];
  
  if (*avg_count) RebuildCurrentRefItems(thd, join, curr_slice, /*is_final_aggr=*/false);

  join->make_sum_func_const_list(*curr_fields);

  // reset aggregate accesspath param
  //path->aggregate().temp_table_param = join->aggr_tmp_table_param;

  return false;

rebuild_err:
  if (join->aggr_tmp_table_param) {
    destroy(join->aggr_tmp_table_param);
    join->aggr_tmp_table_param = nullptr;
  }
  if (tmp_table) {
    close_tmp_table(tmp_table);
    free_tmp_table(tmp_table);
  }
  return true;
}

/**
  Rebuild TemptableAggregate.
  [1] rebuild aggregation functions in fields and tmp_fields.
  [2] restore tmp_table_param and calculate properties for param.
  [3] count of sum_funcs may change after rebuild aggregation functions,
      so we need re-alloc memory for sum_funcs.
  [4] re-create tmp_table for TemptableAggregate.
  [5] call RebuildCurrentRefItems to rebuild ref_items for curr_slice
      after change_to_use_tmp_fields.

  @param thd
  @param join
  @param path
  @param curr_slice current slice in ref_items.

  @return false if successful, true if fail.
*/
bool RebuildTempAggregateAccessPath(THD *thd, JOIN *join, AccessPath *const path,
                                    uint curr_slice, uint *avg_count) {
  AccessPath *table_path = nullptr;
  List_item *curr_fields = nullptr;
  uint curr_tmp_table = join->primary_tables;
  QEP_TAB *tab = &join->qep_tab[curr_tmp_table];
  mem_root_deque<Item *> tmp_field(thd->mem_root);
  ORDER_with_src *tmp_group;
  
  // rebuild sum_funcs for temptableAggregate.
  if (join->check_and_rebuild_sum_funcs(thd, REF_SLICE_SAVED_BASE, avg_count)) {
    goto rebuild_err;
  }
  // if there is no Item_sum_avg, don't need rebuild temptable.
  if (!(*avg_count)) return false;

  curr_fields = &join->tmp_fields[REF_SLICE_SAVED_BASE];

  if (!join->simple_group && !(test_flags & TEST_NO_KEY_GROUP) && !join->with_json_agg) {
    tmp_group = new (thd->mem_root) ORDER_with_src(join->query_block->group_list.first,ESC_GROUP_BY);
  }

  join->set_ref_item_slice(REF_SLICE_SAVED_BASE);

  join->tmp_table_param.pq_copy_from(path->temptable_aggregate().temp_table_param);

  count_field_types(join->query_block, &join->tmp_table_param, *curr_fields,
                    /*reset_with_sum_func=*/false, /*save_sum_fields=*/true);

  // re-alloc sum_funcs
  if (join->alloc_func_list()) goto rebuild_err;
  
  // cleanup temp_table before rebuild a new temp_table for tab
  if (tab->table()) {
    close_tmp_table(tab->table());
    free_tmp_table(tab->table());
    tab->set_table(nullptr);
  }

  if (join->create_intermediate_table(tab, *curr_fields, *tmp_group,
                                      !(*tmp_group).empty() && join->simple_group)) {
    goto rebuild_err;
  }

  if (change_to_use_tmp_fields(curr_fields, thd, join->ref_items[curr_slice],
                               &tmp_field, join->query_block->m_added_non_hidden_fields)) {
    goto rebuild_err;
  }
  join->tmp_fields[curr_slice] = tmp_field;
  join->fields = &join->tmp_fields[curr_slice];

  // keep ref_items[curr_slice] no change
  RebuildCurrentRefItems(thd, join, curr_slice, /*is_final_aggr=*/false);
                          
  // re-create table path
  table_path = create_table_access_path(thd, tab->table(), tab->range_scan(),
                                        tab->table_ref, tab->position(),
                                        /*count_examined_rows=*/false);
  if (!table_path) goto rebuild_err;

  // set path properties
  path->temptable_aggregate().table = tab->table();
  path->temptable_aggregate().table_path = table_path;
  path->temptable_aggregate().temp_table_param = tab->tmp_table_param;
  return false;

rebuild_err:
  return true;
}

/**
  Create order for group list.
  We need build new group list for Final TempTableAggregate
  through saved group list, if this aggregate with group
  by.
  
  @param thd
  @param order the first order in group_list

  @return order object.
*/
ORDER *CreateOrderForGroupList(THD *thd, ORDER *order) {
  ORDER *new_order = new (thd->mem_root) ORDER;
  new_order->item = order->item;
  new_order->item_initial = *(order->item);
  new_order->field_in_tmp_table = nullptr;
  new_order->in_field_list = order->in_field_list;
  new_order->used_alias = order->used_alias;
  new_order->is_position = order->is_position;
  new_order->is_explicit = order->is_explicit;

  if (order->next != nullptr) {
    new_order->next = CreateOrderForGroupList(thd, order->next);
    if (!new_order->next) assert(false);
  }

  if (!new_order) assert(false);
  return new_order;
}

/**
  Fix Item_func_div for average.
  Items may change after change_to_use_temp_fields, need
  to specify items for Item_func_div.

  For example:
    select avg(col1), avg(col2)/sum(col3) from t;

  |------hidden-------|   |-------visible-------|
  +---------+---------+ # +---------+-----------+
  |sum(col3)|avg(col2)| # |avg(col1)|func_div1()|  saved_base_fields
  +---------+---------+ # +---------+-----------+

  After we replace (check_and_rebuild_sum_funcs) avg with sum/count
  |------------hidden-------------|   |-------------visible-------------|
  +---------+---------+-----------+ # +---------+-----------+-----------+
  |sum(col3)|sum(col2)|count(col2)| # |sum(col1)|count(col1)|func_div1()|
  +---------+---------+-----------+ # +---------+-----------+-----------+

  After we insert (rebuild_final_sum_funcs) func_div for avg and set sum/count hidden
  |------------------------------hidden-----------------------------|   |-------visible---------|
  +---------+---------+-----------+-----------+---------+-----------+ # +-----------+-----------+
  |sum(col3)|sum(col2)|count(col2)|func_div2()|sum(col1)|count(col1)| # |func_div3()|func_div1()|
  +---------+---------+-----------+-----------+---------+-----------+ # +-----------+-----------+

  Then, we should reset func_div's args.
  |------------------------------hidden-----------------------------|   |-------visible---------|
  +---------+---------+-----------+-----------+---------+-----------+ # +-----------+-----------+
  |sum(col3)|sum(col2)|count(col2)|func_div2()|sum(col1)|count(col1)| # |func_div3()|func_div1()|
  +---------+---------+-----------+-----------+---------+-----------+ # +-----------+-----------+
        \        \          /                      \           /
         \        \        /                        \         /
          \       func_div2                          func_div3
           \          /
            \        /
            func_div1

  @param thd
  @param join
  @param avg_num number of average

  @return false if successful, true if fail.
*/
bool FixFuncDivForAvg(THD *thd, JOIN *join, uint avg_count) {
  uint saved_base_hidden = CountHiddenFields(*join->saved_base_fields);
  uint saved_base_size = join->saved_base_fields->size();

  uint avg_hidden_i = 0;
  for (uint i = 0; i < saved_base_hidden; ++i) {
    Item *item = (*join->saved_base_fields)[i];
    if (item->type() == Item::SUM_FUNC_ITEM) {
      Item_sum *item_sum = down_cast<Item_sum *>(item);
      if (item_sum->sum_func() == Item_sum::AVG_FUNC) {
        uint avg_positin = i + (++avg_hidden_i) * 2;
        Item *now_item = (*join->fields)[avg_positin];
        if (now_item->type() != Item::FUNC_ITEM) {
          return true;
        }
        // set args
        Item_func *func = down_cast<Item_func *>(now_item);
        func->set_arg_resolve(thd, 0, (*join->fields)[avg_positin - 2]);
        func->set_arg_resolve(thd, 1, (*join->fields)[avg_positin - 1]);
      }
    }
  }

  uint avg_i = 0;
  for (uint i = saved_base_hidden; i < saved_base_size; i++) {
    Item *item = (*join->saved_base_fields)[i];
    if (item->type() == Item::SUM_FUNC_ITEM) {
      Item_sum *item_sum = down_cast<Item_sum *>(item);
      if (item_sum->sum_func() == Item_sum::AVG_FUNC) {
        Item *now_item = (*join->fields)[i + 2 * avg_count];
        if (now_item->type() != Item::FUNC_ITEM) {
          return true;
        }
        // set args
        Item_func *func = down_cast<Item_func *>(now_item);
        uint avg_position = saved_base_hidden + avg_hidden_i * 2 + avg_i * 2;
        func->set_arg_resolve(thd, 0, (*join->fields)[avg_position]);
        func->set_arg_resolve(thd, 1, (*join->fields)[avg_position + 1]);
        avg_i++;
      }
    }
  }

  return false;
}

/**
  Build final TemptableAggregate.
  [1] create tmp_table_param for final TemptableAggregate, and copy properties
      from temp_table_param of TemptableAggregate.
  [2] alloc memory for final_aggr_sum_funcs.
  [3] rebuild aggregation functions for final TemptableAggregate.
  [4] re-calculate properties for final_aggr_tmp_table_param.
  [5] create group list for final TemptableAggregate.
  [6] create tmp_table and items for final TemptableAggregate.
  [7] fix Item_func_div for average function.

  @param thd
  @param join
  @param path
  @param curr_slice current slice in ref_items.
  @param avg_num number of average funccs.

  @return false if successful, true if fail.
*/
AccessPath *BuildFinalTempAggregateAccessPath(THD *thd, JOIN *join, AccessPath *const path,
                                          uint curr_slice, uint avg_count) {
  AccessPath *newPath = nullptr;
  AccessPath *table_path = nullptr;
  TABLE *tmp_table = nullptr;
  mem_root_deque<Item *> tmp_field(thd->mem_root);
  List_item *curr_fields = nullptr;
  ORDER *final_order = nullptr;
  ORDER *old_order = nullptr;

  // create tmp_table_param for final aggregate.
  Temp_table_param *final_tmp_table_param = new (thd->mem_root) Temp_table_param(join->tmp_table_param);
  // copy from tmp_table_param.
  final_tmp_table_param->copy_fields.clear();
  final_tmp_table_param->pq_copy_from(path->temptable_aggregate().temp_table_param);
  join->final_aggr_tmp_table_param = final_tmp_table_param;

  if (join->alloc_ref_item_slice(thd, REF_SLICE_FINAL_AGGREGATE)) goto build_err;

  // alloc new sum_func for final_aggregate
  if (join->alloc_func_list_with_param(join->final_aggr_tmp_table_param, &join->final_aggr_sum_funcs)) goto build_err;

  // rebuild final_sum_funcs
  if (join->rebuild_final_sum_funcs(thd, curr_slice, avg_count)) goto build_err;

  curr_fields = &join->tmp_fields[curr_slice];

  count_field_types(join->query_block, join->final_aggr_tmp_table_param, *curr_fields,
                    /*reset_with_sum_func=*/false, /*save_sum_fields=*/true);
  
  // create new group_list for temptable.
  join->set_ref_item_slice(curr_slice);
  old_order = path->temptable_aggregate().table->group;
  if (old_order) final_order = CreateOrderForGroupList(thd, old_order);

  join->final_aggr_tmp_table_param->hidden_field_count =
      CountHiddenFields(*curr_fields);

  // use curr_slice to build temp table for final aggregate.
  tmp_table = create_tmp_table(
      thd, join->final_aggr_tmp_table_param, *curr_fields, final_order,
      /*distinct=*/false, /*save_sum_fields=*/true, join->query_block->active_options(),
      /*rows_limit=*/HA_POS_ERROR, "<temp>");
  if (!tmp_table) goto build_err;
  join->final_tmpaggr_tmp_table = tmp_table;

  if (final_tmp_table_param->items_to_copy &&
      final_tmp_table_param->items_to_copy->size()) {
    Func_ptr_array *func_ptr = final_tmp_table_param->items_to_copy;
    uint end = func_ptr->size();
    for (uint i = 0; i < end; i++) {
      Func_ptr &func = func_ptr->at(i);
      func.set_override_result_field(func.func()->get_result_field());
    }
  }

  if (change_to_use_tmp_fields(curr_fields, thd, join->ref_items[REF_SLICE_FINAL_AGGREGATE],
                               &tmp_field, join->query_block->m_added_non_hidden_fields))
    goto build_err;

  join->tmp_fields[REF_SLICE_FINAL_AGGREGATE] = tmp_field;

  // reset join::fields to curr_slice items.
  join->fields = &join->tmp_fields[REF_SLICE_FINAL_AGGREGATE];

  if (avg_count) {
    // fix Item_func_div's args, it should be item_field
    if (FixFuncDivForAvg(thd, join, avg_count)) goto build_err;
    // rebuild ref_items[REF_SLICE_FINAL_AGGREGATE]
    RebuildCurrentRefItems(thd, join, REF_SLICE_FINAL_AGGREGATE, /*is_final_aggr=*/true);
  }
  
  join->set_ref_item_slice(REF_SLICE_SAVED_BASE);

  // create accesspath for final aggregate.
  table_path = NewTableScanAccessPath(thd, tmp_table, /*count_examined_rows=*/false);
  newPath = NewTemptableAggregateAccessPath(thd, path, join->final_aggr_tmp_table_param,
                                            tmp_table, table_path, REF_SLICE_FINAL_AGGREGATE,
                                            /*is_final_aggr=*/true);

  return newPath;

build_err:
  if (final_tmp_table_param) {
    destroy(final_tmp_table_param);
    final_tmp_table_param = nullptr;
  }
  if (tmp_table) {
    close_tmp_table(tmp_table);
    free_tmp_table(tmp_table);
  }
  return nullptr;
}

/**
  Build Aggregate.
  [1] create tmp_table_param for final Aggregate, and copy properties
      from temp_table_param of Aggregate.
  [2] alloc memory for final_aggr_sum_funcs.
  [3] rebuild aggregation functions for final Aggregate.
  [4] create group fields for final Aggregate.
  [5] create items for final Aggregate.
  [6] fix Item_func_div for average function.

  @param thd
  @param join
  @param path
  @param curr_slice current slice in ref_items.
  @param avg_num number of average funccs.

  @return false if successful, true if fail.
*/
AccessPath *BuildFinalAggregateAccessPath(THD *thd, JOIN *join, AccessPath *const path,
                                          uint curr_slice, uint avg_count) {
  AccessPath *newPath = nullptr;

  // use JOIN::tmp_table_param for final aggregate.
  //join->tmp_table_param.pq_copy_from(path->aggregate().temp_table_param);
  join->tmp_table_param.copy_fields.clear();
  join->final_aggr_tmp_table_param = &join->tmp_table_param;

  if (join->alloc_ref_item_slice(thd, REF_SLICE_FINAL_AGGREGATE)) goto build_err;

  // alloc new sum_func for final_aggregate
  if (join->alloc_func_list_with_param(join->aggr_tmp_table_param, &join->final_aggr_sum_funcs)) goto build_err;

  // use aggregate's item to rebuild sum_func of final_aggregate and reset join::fields
  if (join->rebuild_final_sum_funcs(thd, curr_slice, avg_count)) goto build_err;

  count_field_types(join->query_block, join->final_aggr_tmp_table_param, *join->fields,
                    /*reset_with_sum_func=*/false, /*save_sum_fields=*/false);

  // reset group_feilds
  if (!join->group_list.empty()) {
    join->set_ref_item_slice(curr_slice);
    ORDER *group = join->group_list.order;
    join->final_group_feilds.clear();
    for (; group; group = group->next) {
      Cached_item *tmp = new_Cached_item(join->thd, *group->item);
      if (!tmp || join->final_group_feilds.push_front(tmp)) goto build_err;
    }
    join->set_ref_item_slice(REF_SLICE_SAVED_BASE);
  }

  /*
  setup_copy_fields(*join->fields, thd, join->final_aggr_tmp_table_param,
                    join->ref_items[REF_SLICE_FINAL_AGGREGATE],
                    &join->tmp_fields[REF_SLICE_FINAL_AGGREGATE]);
  */
  join->fields = &join->tmp_fields[REF_SLICE_FINAL_AGGREGATE];

  if (join->sum_funcs_const) {
    for (size_t idx = 0; idx < join->sum_funcs_const->size(); ++idx) {
      Item *item;
      size_t field_idx, ref_idx;
      std::tie(item, field_idx, ref_idx) = join->sum_funcs_const->at(idx);
      join->tmp_fields[REF_SLICE_FINAL_AGGREGATE][field_idx + (item->hidden ? 0 : avg_count * 2)] = item;
      join->ref_items[REF_SLICE_FINAL_AGGREGATE][ref_idx + (item->hidden ? avg_count * 2 : 0)] = item;
    }
  }

  if (avg_count) {
    if (FixFuncDivForAvg(thd, join, avg_count)) goto build_err;
    // rebuild ref_items[REF_SLICE_FINAL_AGGREGATE]
    RebuildCurrentRefItems(thd, join, REF_SLICE_FINAL_AGGREGATE, /*is_final_aggr=*/true);
  }

  // create final aggregate AccessPath.
  newPath = NewAggregateAccessPath(thd, path,
                                   path->aggregate().rollup, /*is_final_aggr=*/true);

  return newPath;

build_err:
  if (join->final_aggr_tmp_table_param) {
    destroy(join->final_aggr_tmp_table_param);
    join->final_aggr_tmp_table_param = nullptr;
  }
  return nullptr;
}

void FixSortAccessPathForAggrInject(THD *thd, JOIN *join, AccessPath *path, int ref_slice) {
  AccessPath *newChild = path->sort().child;
  bool do_fixsort = false;
  auto &new_filesort = path->sort().filesort;
  switch (newChild->type) {
    case AccessPath::TEMPTABLE_AGGREGATE : {
      if (newChild->temptable_aggregate().is_final_aggr) {
        new_filesort->tables = std::move(Mem_root_array<TABLE *>(
          {newChild->temptable_aggregate().table}));
        do_fixsort = true;
      }
      break;
    }
    case AccessPath::FILTER : {
      AccessPath *filter_child = newChild->filter().child;
      if (filter_child->type == AccessPath::TEMPTABLE_AGGREGATE &&
          filter_child->temptable_aggregate().is_final_aggr) {
        new_filesort->tables = std::move(Mem_root_array<TABLE *>(
          {filter_child->temptable_aggregate().table}));
        do_fixsort = true;
      }
      break;
    }
    case AccessPath::SORT : {
      AccessPath *sort_child = newChild->sort().child;
      if (sort_child->type == AccessPath::TEMPTABLE_AGGREGATE &&
          sort_child->temptable_aggregate().is_final_aggr) {
        new_filesort->tables = std::move(Mem_root_array<TABLE *>(
          {sort_child->temptable_aggregate().table}));
        do_fixsort = true;
      }
    }
    default :
      break; 
  }

  if (do_fixsort) {
    // Redefine sort_order
    join->set_ref_item_slice(ref_slice);
    if (new_filesort->m_remove_duplicates) {
      bool all_order_fields_used;
      ORDER *desired_order = join->order.order;
      if (desired_order == nullptr &&
          join->qep_tab[0].filesort_pushed_order != nullptr) {
        desired_order = join->qep_tab[0].filesort_pushed_order;
      }
      ORDER *order = create_order_from_distinct(
        thd, join->ref_items[ref_slice], desired_order, &join->query_block->fields,
        /*skip_aggregates=*/false, /*convert_bit_fields_to_long=*/false,
        &all_order_fields_used);
      new_filesort->make_sortorder(order, false);
    } else {
      new_filesort->make_sortorder(join->order.order, false);
    }
    join->set_ref_item_slice(REF_SLICE_SAVED_BASE);
    
    // Redefine addon fields for filesort    
    if (new_filesort->m_sort_param.using_addon_fields()) {
      new_filesort->m_sort_param.addon_fields = nullptr;
      new_filesort->m_sort_param.m_addon_fields_status =
          Addon_fields_status::unknown_status;
      new_filesort->using_addon_fields();
    }
  }
}

/**
 * @return true for the equation is satisfied, false otherwise
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
 * @param path  Raw access path
 * @param target_path Where to inject exchange.
 * @return true if not inject, false otherwise.
 */
bool FindExchangeInjectPosition(THD *thd, JOIN *join, AccessPath *const path,
                                AccessPath *&target_path,  SplitPosition *split_pos) {
  assert(target_path == nullptr);
  // WalkAccessPaths would not stop when the callback return true if the parent
  // node has another branch.
  bool stop_walk = false;
  auto find_exchange_inject_position = [thd, join, &target_path, split_pos,
                                        &stop_walk](AccessPath *subpath,
                                                    const JOIN *) {
    if (stop_walk) return true;
    switch (subpath->type) {
      // Iterators that could be injected exchange.
      case AccessPath::TABLE_SCAN:
      case AccessPath::INDEX_SCAN:
      case AccessPath::INDEX_RANGE_SCAN:
      case AccessPath::REF:
      case AccessPath::NESTED_LOOP_JOIN:
      case AccessPath::FILTER:
        if (target_path == nullptr) {
          target_path = subpath;
        }
        return false;

      // Iterators that should not be injected exchange but can appear in the
      // plan tree.
      case AccessPath::REF_OR_NULL:
      case AccessPath::EQ_REF:
      case AccessPath::PUSHED_JOIN_REF:
      case AccessPath::CONST_TABLE:
      case AccessPath::STREAM:
        return false;

      // Iterators that cannot be parallized directly.
      case AccessPath::LIMIT_OFFSET:
        return false;
      case AccessPath::SORT:
        if (split_pos->type == SplitPosition::SPLIT_AGG &&
            target_path->type == AccessPath::AGGREGATE &&
            group_field_eq_sort_list(join->group_fields,
                                     subpath->sort().filesort->m_order)) {
          split_pos->type = SplitPosition::SPLIT_SORT_AGG;
          split_pos->filesort = subpath->sort().filesort;
        } else {
          target_path = subpath;
          split_pos->type = SplitPosition::SPLIT_SORT;
        }
        split_pos->split_sort = true;
        return false;
      case AccessPath::AGGREGATE:
      case AccessPath::TEMPTABLE_AGGREGATE:
        if (check_sum_func_support(thd, join)) {
          target_path = nullptr;
        } else {
          target_path = subpath;
          split_pos->type = SplitPosition::SPLIT_AGG;
          split_pos->split_sort = false;
        }
        return false;

      // Iterators that are not supported.
      default:  // TODO: list all types of Accesspath
        target_path = nullptr;
        stop_walk = true;
        return true;
    }
  };

  WalkAccessPaths(path, /*join=*/join,
                  WalkAccessPathPolicy::ENTIRE_QUERY_BLOCK,
                  find_exchange_inject_position);

  return (target_path == nullptr);
}

static AccessPath *CreateExchangeAccessPath(
    THD *thd, JOIN *join, AccessPath *const path,
    mem_root_deque<Item *> &tmp_table_fields, bool save_sum_fields,
    uint curr_exchange, bool alloc_group_field, bool do_receiver_merge);

static AccessPath *CreateExchangeAccessPathUseTable(THD *thd, JOIN *join,
                                                    AccessPath *const path,
                                                    TABLE *table,
                                                    int ref_slice,
                                                    bool do_receiver_merge);

static void FixAccessPathForExchange(AccessPath *const path,
                                     AccessPath *receiver, JOIN *join,
                                     bool do_merge_sort);

static void GetExchangeParam(ReceiverParam *receiver_param, AccessPath *receiver,
                             bool do_merge_sort);

static void ConnectAccessPathWithChildExchange(AccessPath *const path,
                                               AccessPath *receiver);

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
AccessPath *WalkAccessPathsForExchange(THD *thd, JOIN *join,
                                       AccessPath *const path,
                                       AccessPath *const target_path,
                                       uint curr_exchange, bool &new_child,
                                       int cur_slice, bool alloc_group_field) {
  assert(target_path);

  bool return_follow = false;

  // Invalidate the previous rule and reset the insert rule
  bool inject_here = (target_path == path);

  if(curr_exchange >= MAX_EXCHANGE_NUM) return nullptr;

  bool use_tmp_table = true;  // if true, create temporary table.
  TABLE *table = nullptr;

  List_item *fields = nullptr;
  bool save_sum_func = false;

  AccessPath *child = nullptr;
  int child_slice = -1;
  bool alloc_child_group_field = false;

  // If receiver don`t use temporary table, it should set ref slice for child.
  int output_slice = -1;

  switch (path->type) {
    case AccessPath::TABLE_SCAN:
    case AccessPath::INDEX_SCAN:
    case AccessPath::INDEX_RANGE_SCAN:
    case AccessPath::REF: {
      use_tmp_table = false;
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
          table = path->index_range_scan().used_key_part[0].field->table;
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
      fields = join->ref_items[REF_SLICE_SAVED_BASE].is_null()
                  ? join->fields
                  : &join->tmp_fields[REF_SLICE_SAVED_BASE];

      // Only the first table can be parallelized now
      child = path->nested_loop_join().outer;
      child_slice = REF_SLICE_SAVED_BASE;
      break;
    }
    case AccessPath::FILTER: {
      child = path->filter().child;

      if (cur_slice == -1) {
        cur_slice = join->ref_items[REF_SLICE_TMP1].is_null()
                        ? REF_SLICE_SAVED_BASE
                        : REF_SLICE_TMP1;
      }

      uint ref_slice = 0;
      if (cur_slice == REF_SLICE_SAVED_BASE) {
        if (child->type == AccessPath::TABLE_SCAN) {
          use_tmp_table = false;
          table = child->table_scan().table;
        } else if(child->type == AccessPath::INDEX_SCAN) {
          use_tmp_table = false;
          table = child->index_scan().table;
        } else if (child->type == AccessPath::REF) {
          use_tmp_table = false;
          table = child->ref().table;
        } else {
          ref_slice = join->ref_items[REF_SLICE_SAVED_BASE].is_null()
                          ? 0
                          : REF_SLICE_SAVED_BASE;
        }
      } else {
        ref_slice = cur_slice;
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

      child_slice = cur_slice;
      return_follow = true;

      break;
    }
    case AccessPath::AGGREGATE: {
      use_tmp_table = false;
      table = join->aggr_tmp_table;
      //output_slice = path->aggregate().output_slice;

      child = path->aggregate().child;
      child_slice = REF_SLICE_SAVED_BASE;

      break;
    }
    case AccessPath::TEMPTABLE_AGGREGATE: {
      use_tmp_table = false;
      table = path->temptable_aggregate().table;
      output_slice = path->temptable_aggregate().ref_slice;

      child = path->temptable_aggregate().subquery_path;
      child_slice = REF_SLICE_SAVED_BASE;

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

    default:
      return nullptr;
      // assert(false);
  }

  AccessPath *exchange = nullptr;

  const bool stop_walk =
      (inject_here || !child);  // We only need to inject one exchange.
  //if ((path->type != AccessPath::AGGREGATE &&
  //     path->type != AccessPath::TEMPTABLE_AGGREGATE)) {
  //  inject_here = false;
  //}

  if (inject_here) {
    if (use_tmp_table) {
      assert(fields != nullptr);
      exchange =
          CreateExchangeAccessPath(thd, join, path, *fields, save_sum_func,
                                   curr_exchange, alloc_group_field,
                                   join->split_position.split_sort);
      /*
        Tmp table created by exchange might use some type of fields that are not
        supported by Message Queue. In this case try to create exchange without
        new tmp table.
      */
      if (!exchange && table) {
        exchange = CreateExchangeAccessPathUseTable(
            thd, join, path, table, output_slice,
            join->split_position.split_sort);
      }
    } else {
      exchange = CreateExchangeAccessPathUseTable(thd, join, path, table,
                                                  output_slice,
                                                  join->split_position.split_sort);
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
      thd, join, child, target_path, next_exchange_slice, have_new_child,
      child_slice, alloc_child_group_field);
  if (exchange_following) {
    FixAccessPathForExchange(path, exchange_following, join,
        join->split_position.split_sort);
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
    mem_root_deque<Item *> &tmp_table_fields, bool save_sum_fields,
    uint curr_exchange, bool alloc_group_field, bool do_receiver_merge) {
  if (join->exchange_tmp_fields == nullptr) {
    join->exchange_tmp_fields =
        (*THR_MALLOC)
            ->ArrayAlloc<mem_root_deque<Item *>>(MAX_EXCHANGE_NUM, *THR_MALLOC);
    if (join->exchange_tmp_fields == nullptr) return nullptr;
  }
  AccessPath *sender = nullptr, *receiver = nullptr;
  TABLE *table = nullptr, *table2 = nullptr;


  /*
    Save the original result fields of items in case of falling back.
  */
  vector<Field *> result_fields;
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
  // grouped_expressions removed from Temp_table_param
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
  if (!table || !compat_for_table(table)) goto inject_err;

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
  sender = NewPXSendAccessPath(thd, path, table, nullptr, curr_fields,
                               temp_table_param, true);

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
  // grouped_expressions was removed from Temp_table_param
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

  if (do_receiver_merge) {
    Filesort *curr_file_sort = path->sort().filesort;
    Filesort *final_file_sort = new (thd->mem_root) Filesort(thd, {table2},
        curr_file_sort->keep_buffers, curr_file_sort->m_order, curr_file_sort->limit,
        curr_file_sort->m_remove_duplicates,
        curr_file_sort->m_force_sort_rowids, false); //TODO reset sort_before_group
    receiver = NewPXReceiverMergeAccessPath(thd, sender, final_file_sort,
        table2, ref_slice, true);
  } else {
    receiver = NewPXReceiveAccessPath(thd, sender, table2, ref_slice, true);
  }

  /*
    ref_items[ref_slice] is covered, because we don`t need to
    store ref_item of exchange_send.
  */
  if (save_sum_fields) {
    if (change_to_use_tmp_fields(curr_fields, thd, join->ref_items[ref_slice],
                                 &join->tmp_fields[ref_slice],
                                 join->query_block->m_added_non_hidden_fields))
      goto inject_err;
  } else {
    if (change_to_use_tmp_fields_except_sums(curr_fields, thd, join->query_block,
                                             join->ref_items[ref_slice],
                                             &join->tmp_fields[ref_slice],
                                             join->query_block->m_added_non_hidden_fields))
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
    sender->px_send().table = nullptr;
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
    do_receiver_merge ? receiver->px_receiver_merge().table = nullptr
                      : receiver->px_receiver().table = nullptr;
  }
  return nullptr;
}

static AccessPath *CreateExchangeAccessPathUseTable(THD *thd, JOIN *join,
                                                    AccessPath *const path,
                                                    TABLE *table,
                                                    int ref_slice,
                                                    bool do_receiver_merge) {
  // uchar *record = new (thd->mem_root) uchar[EXCHANGE_BUFFER_SIZE];
  // if (record == nullptr) return nullptr;

  assert(table);
  AccessPath *sender = NewPXSendAccessPath(thd, path, table, nullptr,
      join->fields, nullptr, false);
  AccessPath *receiver = nullptr;
  if (do_receiver_merge) {
    Filesort *final_file_sort = nullptr;
    if (path->type == AccessPath::SORT){
      Filesort *curr_file_sort = path->sort().filesort;
      final_file_sort = new (thd->mem_root) Filesort(thd,
          Mem_root_array<TABLE *>(thd->mem_root, curr_file_sort->tables),
          curr_file_sort->keep_buffers,
          curr_file_sort->m_order, curr_file_sort->limit,
          curr_file_sort->m_remove_duplicates,
          curr_file_sort->m_force_sort_rowids, false); //TODO reset sort_before_group
    } else {
      Filesort *curr_file_sort = join->split_position.filesort;
      // Split sort and agg
      assert(ref_slice > 0);
      join->set_ref_item_slice(ref_slice);
      final_file_sort = new (thd->mem_root) Filesort(thd,
          {table}, curr_file_sort->keep_buffers,
          curr_file_sort->m_order, curr_file_sort->limit,
          curr_file_sort->m_remove_duplicates,
          curr_file_sort->m_force_sort_rowids, false); //TODO reset sort_before_group
      join->set_ref_item_slice(REF_SLICE_SAVED_BASE);
    }
    assert(final_file_sort);
    receiver = NewPXReceiverMergeAccessPath(thd, sender, final_file_sort,
        table, ref_slice, false);
  } else {
    receiver = NewPXReceiveAccessPath(thd, sender, table, ref_slice, false);
  }
  return receiver;
}

AccessPath *CreateExchangeAccessPathForUnion(THD *thd, AccessPath *const path,
                                              TABLE *table, bool is_append) {
  assert(table);

  if (is_append) {
    AccessPath *table_path =
        NewTableScanAccessPath(thd, table, /*count_examined_rows=*/false);
    AccessPath *sender =
        NewPXSendAccessPath(thd, path, table, nullptr, nullptr, nullptr, false,
                            table_path);
    AccessPath *receiver = NewPXReceiveAccessPath(thd, sender, table, -1, false);
    return receiver;
  } else {
    AccessPath *sender =
        NewPXSendAccessPath(thd, path, table, nullptr, nullptr, nullptr, false,
                            nullptr);
    AccessPath *receiver = NewPXReceiveAccessPath(thd, sender, table, -1, false);
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
    receiver_param->table = px_merge_receive.table;
    receiver_param->child = px_merge_receive.child;
    receiver_param->use_temp_table = px_merge_receive.use_temp_table;
  } else {
    const auto &px_receive = receiver->px_receiver();
    receiver_param->ref_slice = px_receive.ref_slice;
    receiver_param->table = px_receive.table;
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
          is_final_agg = path->aggregate().is_final_aggr;
          break;
        }
        case AccessPath::TEMPTABLE_AGGREGATE: {
          temp_table_param = path->temptable_aggregate().temp_table_param;
          is_final_agg = path->temptable_aggregate().is_final_aggr;
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
                  join->qep_tab[0].range_scan()->type == AccessPath::GROUP_INDEX_SKIP_SCAN);
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
    default:
      assert(false);
  }
}

static void ConnectAccessPathWithChildExchange(AccessPath *const path,
                                               AccessPath *receiver) {
  switch (path->type) {
    case AccessPath::NESTED_LOOP_JOIN:
      path->nested_loop_join().outer = receiver;
      break;
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
  bool copy_all = true;
  //bool copy_all = (curr_slice == REF_SLICE_ORDERED_GROUP_BY);

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
            //if (curr_slice == REF_SLICE_ORDERED_GROUP_BY)
            //  new_copy_fields.back().reset();
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
  new_filesort->tables = std::move(Mem_root_array<TABLE *>({new_table}));

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

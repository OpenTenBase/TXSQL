#include "px_split_path.h"

#include "sql/filesort.h"
#include "sql/item_sum.h"
#include "sql/join_optimizer/access_path.h"
#include "sql/join_optimizer/walk_access_paths.h"
#include "sql/sql_optimizer.h"
#include "sql/sql_tmp_table.h"

static AccessPath *SplitAggAccessPath(THD *thd, JOIN *join,
                                      AccessPath *target_path,
                                      bool is_stream_agg);

static bool RebuildLocalAggregateAccessPath(THD *thd, JOIN *join,
                                            AccessPath *const path,
                                            uint curr_slice, uint avg_count);

static bool RebuildLocalTempAggregateAccessPath(THD *thd, JOIN *join,
                                                AccessPath *const path,
                                                uint curr_slice,
                                                uint avg_count);

static AccessPath *BuildFinalAggregateAccessPath(THD *thd, JOIN *join,
                                                 AccessPath *const path,
                                                 uint curr_slice,
                                                 uint avg_count,
                                                 bool stream_agg);

static AccessPath *BuildFinalTempAggregateAccessPath(THD *thd, JOIN *join,
                                                     AccessPath *const path,
                                                     uint curr_slice,
                                                     uint avg_count);

static TABLE *CreateTmpTableForAgg(THD *thd, JOIN *join, Temp_table_param *param,
                                   QEP_TAB *const tab, mem_root_deque<Item *> *tmp_table_fields,
                                   ORDER *group, bool save_sum_fields, bool is_final,
                                   bool is_tmp_agg);

static bool AllocSumFuncList(THD *thd, JOIN *join, Temp_table_param *param,
                             ORDER_with_src *order, bool is_final);

static bool FixMaterializeAccessPath(THD *thd, JOIN *join, AccessPath *const path,
                                     uint curr_slice);

static void SetAggregationForFunc(THD *thd, JOIN *join);

/**
  Walk through accespath to do split aggregate accesspath.
  This recursive will be stopped when we find Aggregate or there
  is no aggregate in accesspath.

  @param thd
  @param join
  @param path
  @return new accesspath
*/
AccessPath *WalkAccessPathsForAggregationSplit(THD *thd, JOIN *join,
                                               AccessPath *const path,
                                               bool stream_agg) {
  AccessPath *new_final_agg_path = nullptr; // accesspath of final agg
  AccessPath *child = nullptr; // child accesspath of this path
  AccessPath *new_child = nullptr;
  bool do_split = false; // whether split agg here
  switch (path->type) {
    case AccessPath::AGGREGATE:
    case AccessPath::TEMPTABLE_AGGREGATE: {
      do_split = true;
      new_final_agg_path = SplitAggAccessPath(thd, join, path, stream_agg);
      if (new_final_agg_path == nullptr) goto err;
      break;
    }
    case AccessPath::FILTER: {
      child = path->filter().child;
      new_child = WalkAccessPathsForAggregationSplit(thd, join, child, stream_agg);
      if (!new_child) goto err;
      path->filter().child = new_child;
      break;
    }
    case AccessPath::LIMIT_OFFSET : {
      child = path->limit_offset().child;
      new_child = WalkAccessPathsForAggregationSplit(thd, join, child, stream_agg);
      if (!new_child) goto err;
      path->limit_offset().child = new_child;
      break;
    }
    case AccessPath::SORT : {
      child = path->sort().child;
      new_child = WalkAccessPathsForAggregationSplit(thd, join, child, stream_agg);
      if (!new_child) goto err;
      path->sort().child = new_child;
      if (FixSortAccessPathForAggrInject(thd, join, path, REF_SLICE_FINAL_AGGREGATE)) {
        goto err;
      }
      break;
    }
    case AccessPath::STREAM : {
      child = path->stream().child;
      // create final temp_table_param
      join->final_tmp_table_param = new (thd->mem_root) Temp_table_param();
      if (join->final_tmp_table_param == nullptr) goto err;
      join->final_tmp_table_param->pq_copy_from(path->stream().temp_table_param);
      new_child = WalkAccessPathsForAggregationSplit(thd, join, child, true);
      if (!new_child) goto err;
      path->stream().child = new_child;
      path->stream().temp_table_param = join->final_tmp_table_param;
      path->stream().table = join->final_tmpaggr_tmp_table;
      // reset final_tmpaggr_tmp_table to nullptr, or it will be clear
      // twice in JOIN::destroy.
      join->final_tmpaggr_tmp_table = nullptr;
      break;
    }
    case AccessPath::MATERIALIZE : {
      MaterializePathParameters *param = path->materialize().param;
      /*
        In this case, materialize accesspath is created in query block
        to store temporary results for sorting or other operations, so
        it only has one query_block. In addition to that child accesspath
        of this materialize must be Aggregate/TemptableAggregate. Fallback
        to serial execution for cases that do not meet the conditions.
      */
      if (param->query_blocks.size() != 1 ||
          param->query_blocks[0].join != join) {
        goto err;
      }
      child = param->query_blocks[0].subquery_path;
      if (child->type != AccessPath::AGGREGATE &&
          child->type != AccessPath::TEMPTABLE_AGGREGATE) {
        goto err;
      }
      if (param->ref_slice == REF_SLICE_TMP1) {
        Temp_table_param *table_param = param->query_blocks[0].temp_table_param;
        AccessPath *table_path = path->materialize().table_path;
        join->final_tmp_table_param = new (thd->mem_root) Temp_table_param();
        if (join->final_tmp_table_param == nullptr) goto err;
        join->final_tmp_table_param->pq_copy_from(table_param);

        new_child = WalkAccessPathsForAggregationSplit(thd, join, child, true);
        if (!new_child) goto err;
        param->ref_slice = REF_SLICE_FINAL_AGGREGATE;
        param->table = join->final_tmpaggr_tmp_table;
        table_path->table_scan().table = join->final_tmpaggr_tmp_table;
        join->final_tmpaggr_tmp_table = nullptr;
      } else if (param->ref_slice == REF_SLICE_TMP2) {
        new_child = WalkAccessPathsForAggregationSplit(thd, join, child, stream_agg);
        if (!new_child) goto err;
        if (FixMaterializeAccessPath(thd, join, path, REF_SLICE_FINAL_AGGREGATE)) {
          goto err;
        }
      }
      param->query_blocks[0].subquery_path = new_child;
      break;
    }
    default:
      break;
  }

  return do_split ? new_final_agg_path : path;

err:
  assert(false);
  return nullptr;
}

/**
  For Aggregate or TemptableAggregate, Rebuilt of sum_funcs is only
  done in the presence of AVG. So we need check whether there are
  AVGs in sum_funcs.

  @param thd
  @param join
  @param avg_count thd count of Item_sum_avg

  @return false to success, true to error.
*/
static bool CheckForRebuildAgg(THD *thd, JOIN *join, uint *avg_count) {
  for (Item_sum **sum_item = join->sum_funcs; *sum_item != nullptr; ++sum_item) {
    const Item_sum::Sumfunctype sum_type = (*sum_item)->sum_func();
    if (sum_type == Item_sum::AVG_FUNC) {
      ++(*avg_count);
    }
  }

  /*
    In addition, ref_items of REF_SLICE_SAVED_BASE may be changed when
    rebuild_sum_funcs, it is necessary to save a copy of ref_items for
    later use.
  */
  join->saved_base_fields = new (thd->mem_root) mem_root_deque<Item *>(thd->mem_root);
  if (!join->saved_base_fields) return true;
  if (join->transform_ref_items_to_fields(join->saved_base_fields, REF_SLICE_SAVED_BASE)) {
    return true;
  }
  join->tmp_fields[REF_SLICE_SAVED_BASE] = *join->saved_base_fields;

  return false;
}

/**
  Split aggregate accesspath into local agg and final agg, it means that a new
  aggregate accesspath will be inject after the current aggregate accesspath as
  final aggreate.
  Two steps included in this function:
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
  @param target_path AggregateAccessPath or TemptableAggregateAccessPath.
  @param is_stream_agg whether there is a stream after aggregate.

  @return nullptr means error.
*/
static AccessPath *SplitAggAccessPath(THD *thd, JOIN *join, AccessPath *target_path,
                                      bool is_stream_agg) {
  AccessPath *new_final_agg_path = nullptr;
  uint avg_count = 0;
  uint curr_slice = 0;

  if (CheckForRebuildAgg(thd, join, &avg_count)) goto err;

  if (target_path->type == AccessPath::AGGREGATE) {
    //curr_slice = target_path->aggregate().output_slice;

    // [1] rebuild Aggregate
    if (RebuildLocalAggregateAccessPath(thd, join, target_path, curr_slice, avg_count)) {
      goto err;
    }

    // save current slice ref_items, it will be used in exchange inject.
    if (join->alloc_ref_item_slice(thd, REF_SLICE_SAVED_ORDERED_GROUP_BY)) {
      goto err;
    }
    join->copy_ref_item_slice(REF_SLICE_SAVED_ORDERED_GROUP_BY, curr_slice);
    join->tmp_fields[REF_SLICE_SAVED_ORDERED_GROUP_BY] = join->tmp_fields[curr_slice];

    // [2] create final Aggregate accesspath
    new_final_agg_path = BuildFinalAggregateAccessPath(thd, join, target_path, curr_slice,
                                                       avg_count, is_stream_agg);
    if (new_final_agg_path == nullptr) goto err;
  } else {
    assert(target_path->type == AccessPath::TEMPTABLE_AGGREGATE);
    curr_slice = target_path->temptable_aggregate().ref_slice;

    SetAggregationForFunc(thd, join);

    // [1] rebuild temptableAggregate
    if (RebuildLocalTempAggregateAccessPath(thd, join, target_path, curr_slice, avg_count)) {
      goto err;
    }

    // save current slice ref_items, it will be used in exchange inject.
    if(join->alloc_ref_item_slice(thd, REF_SLICE_SAVED_TMP1)) goto err;
    join->copy_ref_item_slice(REF_SLICE_SAVED_TMP1, curr_slice);
    join->tmp_fields[REF_SLICE_SAVED_TMP1] = join->tmp_fields[curr_slice];

    // [2] create final temptableAggregate accesspath
    new_final_agg_path =
        BuildFinalTempAggregateAccessPath(thd, join, target_path, curr_slice, avg_count);
    if (new_final_agg_path == nullptr) goto err;
  }

  return new_final_agg_path;

err:
  assert(false);
  return nullptr;
}

/**
  Reset the PROP_AGGREGATION For Func object.
  PROP_AGGREGATION affects func to store the result in Item_field,
  but this property maybe reset in function count_field_types. It's
  neccessary to reset it by PROP_SAVED_AGGREGATION.

  @param thd 
  @param join 
*/
static void SetAggregationForFunc(THD *thd, JOIN *join) {
  uint item_count = join->fields->size();
  for (uint i = 0; i < item_count; ++i) {
    Item *item = join->ref_items[REF_SLICE_SAVED_BASE][i];
    if (item->has_saved_aggregation()) {
      item->reset_saved_aggregation();
      item->set_aggregation();
    }
  }
}

/**
  Fix connection bettween materialize and aggregate.

  @param thd 
  @param join 
  @param path 
  @param curr_slice 
  @return true for error, false for success.
 */
static bool FixMaterializeAccessPath(THD *thd, JOIN *join,
                                     AccessPath *const path,
                                     uint curr_slice) {
  uint curr_tmp_table = join->primary_tables + 1;
  List_item *curr_fields = &join->tmp_fields[curr_slice];
  mem_root_deque<Item *> tmp_field(thd->mem_root);
  QEP_TAB *tab = &join->qep_tab[curr_tmp_table];
  TABLE *tmp_table = nullptr;
  AccessPath *table_path = nullptr;
  bool distinct_arg = false;
  uint avg_count = 0;

  //tab->tmp_table_param->grouped_expressions.clear();
  tab->tmp_table_param->copy_fields.clear();
  tab->tmp_table_param->items_to_copy = nullptr;
  tab->tmp_table_param->skip_create_table = true;
  count_field_types(join->query_block, tab->tmp_table_param, *curr_fields,
                    /*reset_with_sum_func=*/true, /*save_sum_fields=*/false);
  tab->tmp_table_param->hidden_field_count = CountHiddenFields(*curr_fields);

  if (tab->table()) {
    distinct_arg = tab->table()->s->is_distinct;
    close_tmp_table(tab->table());
    free_tmp_table(tab->table());
    tab->set_table(nullptr);
  }
  
  join->set_ref_item_slice(curr_slice);
  tmp_table = create_tmp_table(thd, tab->tmp_table_param, *curr_fields,
                               nullptr, distinct_arg, true,
                               join->query_block->active_options(), HA_POS_ERROR, "<temporary>");
  if (!tmp_table) return true;
  tab->set_table(tmp_table);
  //tab->set_temporary_table_deduplicates(distinct_arg);

  if (change_to_use_tmp_fields(curr_fields, thd, join->ref_items[REF_SLICE_TMP2],
                               &tmp_field, join->query_block->m_added_non_hidden_fields)) {
    goto err;
  }

  join->tmp_fields[REF_SLICE_TMP2] = tmp_field;
  join->fields = &join->tmp_fields[REF_SLICE_TMP2];

  // Fix func_div for avg
  for (Item *item : *join->saved_base_fields) {
    if (item->type() == Item::SUM_FUNC_ITEM && !item->const_item()) {
      Item_sum *sum_item = down_cast<Item_sum*>(item);
      if (sum_item->sum_func() == Item_sum::AVG_FUNC) {
        avg_count++;
      }
    }
  }
  if (avg_count) {
    RebuildCurrentRefItems(thd, join, REF_SLICE_TMP2, true);
  }

  join->set_ref_item_slice(REF_SLICE_SAVED_BASE);

  table_path = path->materialize().table_path;
  table_path->table_scan().table = tmp_table;
  path->materialize().param->table = tmp_table;

  return false;

err:
  if (tmp_table) {
    close_tmp_table(tmp_table);
    free_tmp_table(tmp_table);
    tab->set_table(nullptr);
    tmp_table = nullptr;
  }
  return true;
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
static bool RebuildLocalAggregateAccessPath(THD *thd, JOIN *join, AccessPath *const path,
                                            uint curr_slice, uint avg_count) {

  List_item *curr_fields = nullptr;
  TABLE *tmp_table = nullptr;
  mem_root_deque<Item *> tmp_field(thd->mem_root);
  join->local_tmp_table_param =
      new (thd->mem_root) Temp_table_param(join->tmp_table_param);
  if (!join->local_tmp_table_param) return true;
  join->local_tmp_table_param->copy_fields.clear();
  //join->local_tmp_table_param->grouped_expressions.clear();

  // rebuild sum_funcs for Aggregate.
  if (avg_count && join->rebuild_sum_funcs(thd, REF_SLICE_SAVED_BASE)) {
    return true;
  }
  curr_fields = &join->tmp_fields[REF_SLICE_SAVED_BASE];

  // Item in fields has been changed, it is necessary to reset
  // variables in temptable param.
  count_field_types(join->query_block, join->local_tmp_table_param,
                    *curr_fields, /*reset_with_sum_func=*/false,
                    /*save_sum_fields=*/true);
  join->local_tmp_table_param->hidden_field_count =
      CountHiddenFields(*curr_fields);

  // re-alloc sum_funcs
  if (AllocSumFuncList(thd, join, join->local_tmp_table_param, join->saved_order, false)){
    return true;
  }

  tmp_table = CreateTmpTableForAgg(thd, join, join->local_tmp_table_param, /*tab=*/nullptr,
                                   curr_fields, nullptr, /*save_sum_fields=*/true, false, false);
  if (tmp_table == nullptr) return true;

  if (change_to_use_tmp_fields(curr_fields, thd, join->ref_items[curr_slice],
                               &tmp_field, join->query_block->m_added_non_hidden_fields))
    return true;
  join->tmp_fields[curr_slice] = tmp_field;
  
  // reset join::fields to curr_slice items.
  join->fields = &join->tmp_fields[curr_slice];
  
  if (avg_count) RebuildCurrentRefItems(thd, join, curr_slice, /*is_final_aggr=*/false);

  join->aggr_tmp_table = tmp_table;
  // reset aggregate accesspath param
  //path->aggregate().temp_table_param = join->local_tmp_table_param;
  path->aggregate().px_agg_type = AggType::PX_LOCAL_AGG;

  return false;
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
static bool RebuildLocalTempAggregateAccessPath(THD *thd, JOIN *join, AccessPath *const path,
                                                uint curr_slice, uint avg_count) {
  AccessPath *table_path = nullptr;
  List_item *curr_fields = nullptr;
  uint curr_tmp_table = join->primary_tables;
  QEP_TAB *tab = &join->qep_tab[curr_tmp_table];
  mem_root_deque<Item *> tmp_field(thd->mem_root);
  ORDER *local_group = nullptr;

  // if there is no Item_sum_avg, don't need rebuild temptable.
  if (avg_count == 0) return false;

  // rebuild sum_funcs for temptableAggregate.
  if (join->rebuild_sum_funcs(thd, REF_SLICE_SAVED_BASE)) {
    return true;
  }

  curr_fields = &join->tmp_fields[REF_SLICE_SAVED_BASE];

  local_group = path->temptable_aggregate().table->group;

  join->set_ref_item_slice(REF_SLICE_SAVED_BASE);

  count_field_types(join->query_block, tab->tmp_table_param, *curr_fields,
                    /*reset_with_sum_func=*/false, /*save_sum_fields=*/true);

  // re-alloc sum_funcs
  if (AllocSumFuncList(thd, join, tab->tmp_table_param, join->saved_order, false)){
    return true;
  }

  bool save_sum_fields = (local_group != nullptr && join->simple_group);
  TABLE *tmp_table = CreateTmpTableForAgg(thd, join, tab->tmp_table_param, tab,
                                          curr_fields, local_group, save_sum_fields, false, true);
  if (tmp_table == nullptr) return true;

  if (change_to_use_tmp_fields(curr_fields, thd, join->ref_items[curr_slice],
                               &tmp_field, join->query_block->m_added_non_hidden_fields)) {
    return true;
  }
  join->tmp_fields[curr_slice] = tmp_field;
  join->fields = &join->tmp_fields[curr_slice];

  // keep ref_items[curr_slice] no change
  RebuildCurrentRefItems(thd, join, curr_slice, /*is_final_aggr=*/false);

  // re-create table path
  table_path = create_table_access_path(thd, nullptr, tab->range_scan(), tab->table_ref,
                                        tab->position(), /*count_examined_rows=*/false);
  if (!table_path) return true;

  // set path properties
  path->temptable_aggregate().table = tab->table();
  path->temptable_aggregate().table_path = table_path;
  path->temptable_aggregate().temp_table_param = tab->tmp_table_param;
  path->temptable_aggregate().px_agg_type = AggType::PX_LOCAL_AGG;
  return false;
}

static ORDER *CreateOrderForGroupList(THD *thd, ORDER *order);

static bool FixFuncDivForAvg(THD *thd, JOIN *join, uint avg_count);

/**
  Build final TemptableAggregate.
  [1] create tmp_table_param for final TemptableAggregate, and copy properties
      from temp_table_param of TemptableAggregate.
  [2] alloc memory for final_aggr_sum_funcs.
  [3] rebuild aggregation functions for final TemptableAggregate.
  [4] re-calculate properties for final_tmp_table_param.
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
static AccessPath *BuildFinalTempAggregateAccessPath(THD *thd, JOIN *join,
                                                     AccessPath *const path,
                                                     uint curr_slice, uint avg_count) {
  AccessPath *newPath = nullptr;
  AccessPath *table_path = nullptr;
  TABLE *tmp_table = nullptr;
  mem_root_deque<Item *> tmp_field(thd->mem_root);
  List_item *curr_fields = nullptr;
  ORDER *final_order = nullptr;
  ORDER *old_order = nullptr;

  // create tmp_table_param for final aggregate.
  Temp_table_param *final_tmp_table_param =
      new (thd->mem_root) Temp_table_param(*path->temptable_aggregate().temp_table_param);
  if (final_tmp_table_param == nullptr) return nullptr;
  join->final_tmp_table_param = final_tmp_table_param;

  if (join->alloc_ref_item_slice(thd, REF_SLICE_FINAL_AGGREGATE)) return nullptr;

  // alloc new sum_func for final_aggregate
  if (AllocSumFuncList(thd, join, join->final_tmp_table_param, join->saved_order, true)) return nullptr;

  // rebuild final_sum_funcs
  if (join->rebuild_final_sum_funcs(thd, curr_slice, avg_count)) return nullptr;

  // reset ref_items of REF_SLICE_TMP1
  RebuildCurrentRefItems(thd, join, curr_slice, /*is_final_aggr=*/true);

  curr_fields = &join->tmp_fields[curr_slice];

  count_field_types(join->query_block, join->final_tmp_table_param, *curr_fields,
                    /*reset_with_sum_func=*/false, /*save_sum_fields=*/true);

  // create new group_list for temptable.
  join->set_ref_item_slice(curr_slice);
  old_order = path->temptable_aggregate().table->group;
  if (old_order) {
    final_order = CreateOrderForGroupList(thd, old_order);
    if (final_order == nullptr) {
      return nullptr;
    }
  }

  join->final_tmp_table_param->hidden_field_count =
      CountHiddenFields(*curr_fields);

  bool save_sum_fields = (final_order != nullptr && join->simple_group);
  // use curr_slice to build temp table for final aggregate.
  tmp_table = CreateTmpTableForAgg(thd, join, join->final_tmp_table_param, nullptr,
                                   curr_fields, final_order, save_sum_fields, true, true);
  if (!tmp_table) return nullptr;
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
    return nullptr;

  join->tmp_fields[REF_SLICE_FINAL_AGGREGATE] = tmp_field;

  // reset join::fields to curr_slice items.
  join->fields = &join->tmp_fields[REF_SLICE_FINAL_AGGREGATE];

  if (avg_count) {
    // fix Item_func_div's args, it should be item_field
    if (FixFuncDivForAvg(thd, join, avg_count)) return nullptr;
    // rebuild ref_items[REF_SLICE_FINAL_AGGREGATE]
    RebuildCurrentRefItems(thd, join, REF_SLICE_FINAL_AGGREGATE, /*is_final_aggr=*/true);
  }

  join->set_ref_item_slice(REF_SLICE_SAVED_BASE);

  // create accesspath for final aggregate.
  table_path = NewTableScanAccessPath(thd, tmp_table, /*count_examined_rows=*/false);
  newPath = NewTemptableAggregateAccessPath(thd, path, join->final_tmp_table_param,
                                            tmp_table, table_path, REF_SLICE_FINAL_AGGREGATE,
                                            /*px_agg_type=*/AggType::PX_FINAL_AGG);

  return newPath;
}

static void FixForConstSumfuncs(JOIN *join, uint avg_count, uint curr_slice);

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
static AccessPath *BuildFinalAggregateAccessPath(THD *thd, JOIN *join, AccessPath *const path,
                                                 uint curr_slice, uint avg_count, bool stream_agg) {
  AccessPath *newPath = nullptr;
  ORDER *final_order = nullptr;
  TABLE *tmp_table = nullptr;
  List_item *curr_fields = nullptr;
  QEP_TAB *tab = nullptr;
  mem_root_deque<Item *> tmp_field(thd->mem_root);

  if (!stream_agg) {
    // use JOIN::tmp_table_param for final aggregate.
    //join->tmp_table_param.pq_copy_from(path->aggregate().temp_table_param);
    join->tmp_table_param.copy_fields.clear();
    join->final_tmp_table_param = &join->tmp_table_param;
  }

  if (join->alloc_ref_item_slice(thd, REF_SLICE_FINAL_AGGREGATE)) goto build_err;

  // alloc new sum_func for final_aggregate
  if (AllocSumFuncList(thd, join, join->local_tmp_table_param, join->saved_order, true)) goto build_err;

  // use aggregate's item to rebuild sum_func of final_aggregate and reset join::fields
  if (join->rebuild_final_sum_funcs(thd, curr_slice, avg_count)) goto build_err;

  // reset ref_items of REF_SLICE_ORDERED_GROUP_BY
  RebuildCurrentRefItems(thd, join, curr_slice, /*is_final_aggr=*/true);

  curr_fields = &join->tmp_fields[curr_slice];

  count_field_types(join->query_block, join->final_tmp_table_param, *join->fields,
                    /*reset_with_sum_func=*/false, /*save_sum_fields=*/false);

  // reset group_feilds and group_list(if needed).
  if (join->saved_group_list != nullptr && !join->saved_group_list->empty()) {
    join->set_ref_item_slice(curr_slice);
    ORDER *group = join->saved_group_list->order;
    // reset fields
    join->final_group_feilds.clear();
    for (; group; group = group->next) {
      Cached_item *tmp = new_Cached_item(join->thd, *group->item);
      if (!tmp || join->final_group_feilds.push_front(tmp)) goto build_err;
    }
    // reset group_list
    if (stream_agg && group) {
      final_order = CreateOrderForGroupList(thd, group);
      if (final_order == nullptr) {
        assert(false);
        goto build_err;
      }
    }
    join->set_ref_item_slice(REF_SLICE_SAVED_BASE);
  }

  join->final_tmp_table_param->hidden_field_count =
      CountHiddenFields(*curr_fields);
  
  if (stream_agg) {
    // If this is streaming aggregate, we need to create tmp_table.
    join->set_ref_item_slice(curr_slice);

    uint curr_tmp_table = join->primary_tables;
    tab = &join->qep_tab[curr_tmp_table];
    if (tab->table()) {
      close_tmp_table(tab->table());
      free_tmp_table(tab->table());
      tab->set_table(nullptr);
    }
    tab->tmp_table_param = join->final_tmp_table_param;

    tmp_table = create_tmp_table(
      thd, join->final_tmp_table_param, *curr_fields, final_order,
      /*distinct=*/false, /*save_sum_fields=*/true, join->query_block->active_options(),
      /*rows_limit=*/HA_POS_ERROR, "<temporary>");
    if (!tmp_table) goto build_err;
    join->final_tmpaggr_tmp_table = tmp_table;
    tab->set_table(tmp_table);

    if (join->final_tmp_table_param->items_to_copy &&
        join->final_tmp_table_param->items_to_copy->size()) {
      Func_ptr_array *func_ptr = join->final_tmp_table_param->items_to_copy;
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
    join->set_ref_item_slice(REF_SLICE_SAVED_BASE);
  } else {
    /*
    setup_copy_fields(*join->fields, thd, join->final_tmp_table_param,
                      join->ref_items[REF_SLICE_FINAL_AGGREGATE],
                      &join->tmp_fields[REF_SLICE_FINAL_AGGREGATE]);
    */
    FixForConstSumfuncs(join, avg_count, REF_SLICE_FINAL_AGGREGATE);
  }

  join->fields = &join->tmp_fields[REF_SLICE_FINAL_AGGREGATE];

  if (avg_count) {
    if (FixFuncDivForAvg(thd, join, avg_count)) goto build_err;
    // rebuild ref_items[REF_SLICE_FINAL_AGGREGATE]
    RebuildCurrentRefItems(thd, join, REF_SLICE_FINAL_AGGREGATE, /*is_final_aggr=*/true);
  }

  // create final aggregate AccessPath.
  newPath = NewAggregateAccessPath(thd, path,
                                   path->aggregate().rollup,
                                   /*px_agg_type=*/AggType::PX_FINAL_AGG);

  return newPath;

build_err:
  if (join->final_tmp_table_param) {
    destroy(join->final_tmp_table_param);
    join->final_tmp_table_param = nullptr;
  }
  if (tmp_table) {
    close_tmp_table(tmp_table);
    free_tmp_table(tmp_table);
    join->final_tmpaggr_tmp_table = nullptr;
    if (tab && tab->table()) {
      tab->set_table(nullptr);
    }
  }
  return nullptr;
}

/**
  Create a Tmp Table For Aggregate/TemptableAggregate.

  Because of unsupport of window function, tmp_rows_limit will always be
  HA_POS_ERROR and distinct_arg is only dependent of select_distinct and
  group_list. Sum_funcs for local aggregate will be created after creating
  temporary table, and it dosen't affect make_sum_func_list whether it's
  parameter before_group_by is true or false because that we don't support
  rollup now.

  @param thd 
  @param join 
  @param param 
  @param tab 
  @param tmp_table_fields 
  @param group 
  @param save_sum_fields 
  @param is_final Whether create tmp table for final agg.
  @param is_tmp_agg Whether create tmp table for temp_agg.
  @return temp table.
*/
static TABLE *CreateTmpTableForAgg(THD *thd, JOIN *join, Temp_table_param *param,
                                   QEP_TAB *const tab, mem_root_deque<Item *> *tmp_table_fields,
                                   ORDER *group, bool save_sum_fields, bool is_final, bool is_tmp_agg) {
  DBUG_TRACE;

  // In this instance, with_sum_func in query_block will always be true.
  ha_rows tmp_rows_limit = HA_POS_ERROR;
  // Not support window function now.
  bool distinct_arg =
      join->select_distinct &&
      // GROUP BY is absent or has been done in a previous step
      join->saved_group_list->empty();

  Item_sum **agg_sum_funcs = is_final ? join->final_aggr_sum_funcs
                                      : join->sum_funcs;
  // Clear old tmp_table.
  if (tab && tab->table()) {
    close_tmp_table(tab->table());
    free_tmp_table(tab->table());
    tab->set_table(nullptr);
  }
  param->copy_fields.clear();
  //param->grouped_expressions.clear();
  param->items_to_copy = nullptr;
  param->skip_create_table = true;

  TABLE *table = create_tmp_table(thd, param, *tmp_table_fields, group, distinct_arg,
                                  save_sum_fields, join->query_block->active_options(),
                                  tmp_rows_limit, "<temporary>");
  if (!table) return nullptr;

  if (tab) {
    tab->set_table(table);
  }

  // Not support distinct sum funcs yet.
  const bool need_distinct = false;
  // Not support rollup, so it dosn't matter that whether before_group_by is true or false.
  if (!is_final &&
      join->make_sum_func_list(*tmp_table_fields, /*before_group_by=*/true, /*recompute=*/true)){
    goto err;
  }
  if (prepare_sum_aggregators(agg_sum_funcs, need_distinct)) {
    goto err;
  }
  if (setup_sum_funcs(thd, agg_sum_funcs)) goto err;

  return table;

err:
  if (table != nullptr) {
    close_tmp_table(table);
    free_tmp_table(table);
    if (tab) {
      tab->set_table(nullptr);
    }
  }
  return nullptr;
}

/**
  Alloc for local sum_funcs or final sum_funcs.
  See JOIN::alloc_func_list

  @param join 
  @param param 
  @param is_final 
  @return false for success, true for error.
 */
static bool AllocSumFuncList(THD *thd, JOIN *join, Temp_table_param *param,
                             ORDER_with_src *order, bool is_final) {
  uint func_count, group_parts;
  DBUG_TRACE;

  func_count = param->sum_func_count;
  group_parts = join->send_group_parts;

  if (join->select_distinct) {
    group_parts += CountVisibleFields(*join->fields);
    if (order && !order->empty()) {
      ORDER *ord;
      for (ord = order->order; ord; ord = ord->next) group_parts++;
    }
  }

  (is_final ? join->final_aggr_sum_funcs : join->sum_funcs) =
      (Item_sum **)thd->mem_calloc(sizeof(Item_sum **) * (func_count + 1) +
                                   sizeof(Item_sum ***) * (group_parts + 1));
  return (is_final ? (join->final_aggr_sum_funcs == nullptr)
                   : (join->sum_funcs == nullptr));
}

/**
  Const sumfuncs item should not be calculated in parallel operations.
  The values of these kind items could be obtained prior to the stage
  of execution.

  @param join 
  @param avg_count 
  @param curr_slice 
*/
static void FixForConstSumfuncs(JOIN *join, uint avg_count, uint curr_slice) {
  uint saved_base_size = join->saved_base_fields->size();
  uint befor_avg_count = 0;
  for (uint i = 0; i < saved_base_size; ++i) {
    Item *saved_item = (*join->saved_base_fields)[i];
    if (saved_item->type() == Item::SUM_FUNC_ITEM) {
      Item_sum *item_sum = down_cast<Item_sum *>(saved_item);
      if (item_sum->sum_func() == Item_sum::AVG_FUNC) {
        befor_avg_count += 1;
      } else if (saved_item->const_item()) {
        join->tmp_fields[curr_slice][saved_item->hidden ? (befor_avg_count * 2 + i)
                                                        : (avg_count * 2 + i)] = saved_item;
      }
    }
  }
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
static ORDER *CreateOrderForGroupList(THD *thd, ORDER *order) {
  ORDER *new_order = new (thd->mem_root) ORDER;
  if (new_order == nullptr) {
    assert(false);
    return nullptr;
  }
  new_order->item = order->item;
  new_order->item_initial = *(order->item);
  new_order->field_in_tmp_table = nullptr;
  new_order->in_field_list = order->in_field_list;
  new_order->used_alias = order->used_alias;
  new_order->is_position = order->is_position;
  new_order->is_explicit = order->is_explicit;

  if (order->next != nullptr) {
    new_order->next = CreateOrderForGroupList(thd, order->next);
    if (!new_order->next) {
      assert(false);
      return nullptr;
    }
  }

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

  After we replace (rebuild_sum_funcs) avg with sum/count
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
static bool FixFuncDivForAvg(THD *thd, JOIN *join, uint avg_count) {
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

bool FixSortAccessPathForAggrInject(THD *thd, JOIN *join, AccessPath *path, int ref_slice) {
  bool do_fixsort = false;
  auto &new_filesort = path->sort().filesort;
  TABLE *table_for_sort = nullptr;

  const auto scan_functor = [&table_for_sort, &do_fixsort, &ref_slice](AccessPath *sub_path, const JOIN *) {
    switch(sub_path->type) {
      case AccessPath::STREAM: {
        table_for_sort = sub_path->stream().table;
        do_fixsort = true;
        return true;
      }
      case AccessPath::TEMPTABLE_AGGREGATE: {
        table_for_sort = sub_path->temptable_aggregate().table;
        do_fixsort = true;
        return true;
      }
      case AccessPath::MATERIALIZE: {
        MaterializePathParameters *param = sub_path->materialize().param;
        AccessPath *mat_child = param->query_blocks[0].subquery_path;
        table_for_sort = param->table;
        if (mat_child->type == AccessPath::TEMPTABLE_AGGREGATE) {
          ref_slice = REF_SLICE_TMP2;
        }
        do_fixsort = true;
        return true;
      }
      case AccessPath::AGGREGATE: {
        return true;
      }
      default:
        return false;
    }
  };

  WalkAccessPaths(path, /*join=*/nullptr,
                  WalkAccessPathPolicy::ENTIRE_TREE, scan_functor);
  if (!do_fixsort) return false;
  if (!table_for_sort) {
    return true;
  }
  new_filesort->tables =
      std::move(Mem_root_array<TABLE *>({table_for_sort}));

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
      mem_root_deque<Item *> *select_list =
          (join->rollup_state == JOIN::RollupState::NONE) ? &join->query_block->fields : join->fields;
      ORDER *order = create_order_from_distinct(
        thd, join->ref_items[ref_slice],
        desired_order, select_list,
        /*skip_aggregates=*/false, /*convert_bit_fields_to_long=*/false,
        &all_order_fields_used);
      if (!order) return true;
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
  return false; 
}

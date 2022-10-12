#include "px_explain.h"

#include "sql/join_optimizer/access_path.h"
#include "sql/sql_optimizer.h"
#include "sql/sql_select.h"
#include "sql/sql_optimizer.h"
#include "sql/range_optimizer/range_optimizer.h"
#include "sql/parallel_execution/px_plan_slice.h" // PX_plan_slice

extern QEP_TAB *get_matched_tab(JOIN *join, TABLE *table);

static void reset_position(POSITION *pos, double exchange_rows) {
  pos->rows_fetched = exchange_rows;
  pos->read_cost = 0;
  pos->filter_effect = 1.0;
  pos->prefix_rowcount = 0;
  pos->prefix_cost = 0;
  pos->table = nullptr;
  pos->key = nullptr;
  pos->use_join_buffer = false;
}

/**
  Make a fake QEP_TAB to represent given exchange operator, either sender or receiver.

  @return qep_tab success, nullptr otherwise.
*/
static QEP_TAB *make_exchange_tab(THD *thd, AccessPath *path, JOIN *join,
    std::string &table_name, double exchange_rows) {
  assert(thd && path && join && !table_name.empty());
  assert(path->type == AccessPath::PX_SEND ||
         path->type == AccessPath::PX_RECEIVE ||
         path->type == AccessPath::PX_RECEIVER_MERGE);
  
  TABLE *table = nullptr;
  TABLE_SHARE *share = nullptr;
  QEP_TAB *exchange_qep_tab = nullptr;
  QEP_shared *qep_shared = nullptr;
  POSITION *position = nullptr;
  TABLE_LIST *tbl = nullptr;
  TABLE_REF *ref = nullptr;

  exchange_qep_tab = new (thd->mem_root) QEP_TAB;
  qep_shared = new (thd->mem_root) QEP_shared;
  position = new (thd->mem_root) POSITION;
  tbl = new (thd->mem_root) TABLE_LIST;
  ref = new (thd->mem_root) TABLE_REF;
  table = new (thd->mem_root) TABLE;
  share = new (thd->mem_root) TABLE_SHARE;
  if (!exchange_qep_tab || !qep_shared || !position || !tbl || !ref || !table || !share) goto oom;

  tbl->alias = strmake_root(thd->mem_root, table_name.c_str(), strlen(table_name.c_str()));
  share->table_name.str = tbl->alias;
  share->table_name.length = strlen(tbl->alias);
  table->s = share;
  table->pos_in_table_list = tbl;
  tbl->query_block = join->query_block;
  tbl->table_name = strmake_root(thd->mem_root, table->s->table_name.str, table->s->table_name.length);
  tbl->table_name_length = table->s->table_name.length;
  tbl->table = table;
  reset_position(position, exchange_rows);
  qep_shared->set_join(join);
  exchange_qep_tab->set_qs(qep_shared);
  exchange_qep_tab->set_table(table);
  exchange_qep_tab->set_position(position);
  exchange_qep_tab->set_type(JT_ALL);
  exchange_qep_tab->ref() = *ref;
  exchange_qep_tab->table_ref = tbl;
  exchange_qep_tab->filesort = nullptr;
  return exchange_qep_tab;

oom:
  my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
  if (exchange_qep_tab) destroy(exchange_qep_tab);
  if (qep_shared) destroy(qep_shared);
  if (position) destroy(position);
  if (tbl) destroy(tbl);
  if (ref) destroy(ref);
  if (table) destroy(table);
  if (share) destroy(share);

  return nullptr;
}

/**
  Traversing the access path tree in pre-order.
  If find a DFO-pair(child dfo and parent dfo), find the
  PX_exchange_info from exchange_context and set to Exchange
  AccessPath.

  @param coordinator thd
  @param path current access path
  @param exchange_context PX_exchange_info context
  @param join current join
  @param plan_slice current plan slice

  @return false success, true otherwise.
*/
bool WalkAccessPathsForExplain(THD *thd, AccessPath *path, PX_exchange_context *exchange_context,
                               uint &exchange_count, JOIN *join, PX_plan_slice *plan_slice) {
  assert(path && exchange_context);

  switch (path->type) {
    case AccessPath::PX_RECEIVE: {
      assert(path->px_receiver().child->type == AccessPath::PX_SEND);
      // Traversing the access path tree in pre-order to set exchange_info
      PX_exchange_info *exchange_info = exchange_context->get(++exchange_count);
      assert(exchange_info);
      path->px_receiver().exchange_info = exchange_info;
      path->px_receiver().child->px_send().exchange_info = exchange_info;
      if (join && !join->px_encounter_exchange) join->px_encounter_exchange = true;

      if (WalkAccessPathsForExplain(thd, path->px_receiver().child,
          exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }

      /*
        Some plan slices just respond to SELECT_LEX_UNIT, can't be attached
        to any query block. this plan slices won't be explained in format=traditional. 
      */
      if (!thd->lex->explain_format->is_tree() && join) {
        // create a fake QEP_TAB for exchange operator
        QEP_TAB *exchange_tab = nullptr;
        std::string table_name = exchange_info->exchange_table_name(PX_RECEIVER);
        /*
          The cost of PX_RECEIVE/PX_RECEIVER_MERGE is the same as the child AccessPath,
          Becuase the child is PX_SEND always.
        */
        if (!(exchange_tab = make_exchange_tab(thd, path, join, table_name,
            path->px_receiver().child->num_output_rows))) return true;
        exchange_tab->exchange_type = QEP_TAB::Exchange_receiver;
        exchange_tab->px_exchange_id = exchange_info->exchange_id();
        path->num_output_rows = path->px_receiver().child->num_output_rows;
        plan_slice->add_tab(exchange_tab);
      }
      break;
    }
    case AccessPath::PX_RECEIVER_MERGE: {
      assert(path->px_receiver_merge().child->type == AccessPath::PX_SEND);
      // Traversing the access path tree in pre-order to set exchange_info
      PX_exchange_info *exchange_info = exchange_context->get(++exchange_count);
      assert(exchange_info);
      path->px_receiver_merge().exchange_info = exchange_info;
      path->px_receiver_merge().child->px_send().exchange_info = exchange_info;
      if (join && !join->px_encounter_exchange) join->px_encounter_exchange = true;

      if (WalkAccessPathsForExplain(thd, path->px_receiver_merge().child,
          exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      /*
        Some plan slices just respond to SELECT_LEX_UNIT, can't attached
        to any query block. this plan slices won't explained in format=traditional. 
      */
      if (!thd->lex->explain_format->is_tree() && join) {
        // create a fake QEP_TAB for exchange operator!
        QEP_TAB *exchange_tab = nullptr;
        std::string table_name = exchange_info->exchange_table_name(PX_RECEIVER_MERGE);
        if (!(exchange_tab = make_exchange_tab(thd, path, join, table_name,
            path->px_receiver_merge().child->num_output_rows))) return true;
        exchange_tab->exchange_type = QEP_TAB::Exchange_receiver_merge;
        exchange_tab->px_exchange_id = exchange_info->exchange_id();
        path->num_output_rows = path->px_receiver_merge().child->num_output_rows;
        plan_slice->add_tab(exchange_tab);
      }
      break;
    }
    case AccessPath::PX_SEND: {
      PX_plan_slice *slice = nullptr;
      PX_exchange_info *exchange_info = path->px_send().exchange_info;
      assert(exchange_info);
      if (!thd->lex->explain_format->is_tree()) {
        // PX_SEND is the root accesspath of a plan slice.
        slice = new (thd->mem_root) PX_plan_slice();
        if (!slice) {
          my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
          return true;
        }
      }

      if (WalkAccessPathsForExplain(thd, path->px_send().child,
          exchange_context, exchange_count, join, slice)) {
        return true;
      }

      /*
        Some plan slices just respond to SELECT_LEX_UNIT, can't attached
        to any query block. this plan slices won't explained in format=traditional. 
      */
      if (!thd->lex->explain_format->is_tree() && join) {
        assert(slice && !slice->attached());
        QEP_TAB *exchange_tab = nullptr;
        std::string table_name = exchange_info->exchange_table_name(PX_SENDER);
        /*
          PX_SEND will create a QEP_TAB representation, and the cost calculation rules
          of its POSITION are as follows:
          Rule 1: its rows_fetched is equal to its child AccessPath's num_output_rows
                  if the num_output_rows is not unknown(-1), otherwise 0
          Rule 2: filter_effect is always equal to 1.0
        */
        double exchange_rows = path->px_send().child->num_output_rows == -1 ?
            0 : path->px_send().child->num_output_rows;
        if (!(exchange_tab = make_exchange_tab(thd, path, join, table_name, exchange_rows))) return true;
        exchange_tab->exchange_type = QEP_TAB::Exchange_sender;
        exchange_tab->px_exchange_id = exchange_info->exchange_id();
        path->num_output_rows = exchange_rows;
        slice->add_tab(exchange_tab);
        slice->set_attach();
        join->px_plan_slices.emplace_back(slice);
      } else {
        if (slice) destroy(slice);
      }
      break;
    }
    case AccessPath::TABLE_SCAN: {
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          QEP_TAB *tab = get_matched_tab(join, path->table_scan().table);
          if (tab) plan_slice->add_tab(tab);
        }
      }
      break;
    }
    case AccessPath::INDEX_SCAN: {
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          QEP_TAB *tab = get_matched_tab(join, path->index_scan().table);
          if (tab) plan_slice->add_tab(tab);
        }
      }
      break;
    }
    case AccessPath::REF: {
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          QEP_TAB *tab = get_matched_tab(join, path->ref().table);
          if (tab) plan_slice->add_tab(tab);
        }
      }
      break;
    }
    case AccessPath::REF_OR_NULL: {
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          QEP_TAB *tab = get_matched_tab(join, path->ref_or_null().table);
          if (tab) plan_slice->add_tab(tab);
        }
      }
      break;
    }
    case AccessPath::FOLLOW_TAIL: {
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          QEP_TAB *tab = get_matched_tab(join, path->follow_tail().table);
          if (tab) plan_slice->add_tab(tab);
        }
      }
      break;
    }
    case AccessPath::INDEX_RANGE_SCAN: {
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          TABLE *table = path->index_range_scan().used_key_part[0].field->table;
          QEP_TAB *tab = get_matched_tab(join, table);
          if (tab) plan_slice->add_tab(tab);
        }
      }
      break;
    }
    case AccessPath::DYNAMIC_INDEX_RANGE_SCAN: {
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          QEP_TAB *tab = path->dynamic_index_range_scan().qep_tab;
          if (tab) plan_slice->add_tab(tab);
        }
      }
      break;
    }
    case AccessPath::TABLE_SAMPLE: {
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          // QEP_TAB *tab = get_matched_tab(join, path->table_sample().table);
          // if (tab) plan_slice->add_tab(tab);
        }
      }
      break;
    }
    case AccessPath::EQ_REF: {
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          QEP_TAB *tab = get_matched_tab(join, path->eq_ref().table);
          if (tab) plan_slice->add_tab(tab);
        }
      }
    }
    case AccessPath::INDEX_MERGE: {
      break;
    }
    case AccessPath::ROWID_INTERSECTION: {
      break;
    }
    case AccessPath::ROWID_UNION: {
      break;
    }
    case AccessPath::INDEX_SKIP_SCAN: {
      break;
    }
    case AccessPath::GROUP_INDEX_SKIP_SCAN: {
      break;
    }
    case AccessPath::REMOVE_DUPLICATES_ON_INDEX: {
      break;
    }
    case AccessPath::DELETE_ROWS: {
      break;
    }
    case AccessPath::UPDATE_ROWS: {
      break;
    }
    case AccessPath::PUSHED_JOIN_REF:
    case AccessPath::FULL_TEXT_SEARCH:
    case AccessPath::CONST_TABLE:
    case AccessPath::MRR:
    case AccessPath::MATERIALIZED_TABLE_FUNCTION:
    case AccessPath::UNQUALIFIED_COUNT:
    case AccessPath::TABLE_VALUE_CONSTRUCTOR:
    case AccessPath::FAKE_SINGLE_ROW:
    case AccessPath::ZERO_ROWS:
    case AccessPath::ZERO_ROWS_AGGREGATED: {
      // no children and no QEP_TAB
      break;
    }
    case AccessPath::NESTED_LOOP_JOIN: {
      if (WalkAccessPathsForExplain(thd, path->nested_loop_join().outer,
                                    exchange_context, exchange_count, join, plan_slice) ||
          WalkAccessPathsForExplain(thd, path->nested_loop_join().inner,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::HASH_JOIN: {
      // keep the same order with adjust_children.
      if (WalkAccessPathsForExplain(thd, path->hash_join().inner,
                                    exchange_context, exchange_count, join, plan_slice) ||
          WalkAccessPathsForExplain(thd, path->hash_join().outer,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::SORT_MERGE_JOIN: {
      if (WalkAccessPathsForExplain(thd, path->sort_merge_join().inner,
                                    exchange_context, exchange_count, join, plan_slice) ||
          WalkAccessPathsForExplain(thd, path->sort_merge_join().outer,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::BKA_JOIN: {
      if (WalkAccessPathsForExplain(thd, path->bka_join().inner,
                                    exchange_context, exchange_count, join, plan_slice) ||
          WalkAccessPathsForExplain(thd, path->bka_join().outer,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::NESTED_LOOP_SEMIJOIN_WITH_DUPLICATE_REMOVAL: {
      if (WalkAccessPathsForExplain(thd, path->nested_loop_semijoin_with_duplicate_removal().inner,
                                    exchange_context, exchange_count, join, plan_slice) ||
          WalkAccessPathsForExplain(thd, path->nested_loop_semijoin_with_duplicate_removal().outer,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::FILTER: {
      if (WalkAccessPathsForExplain(thd, path->filter().child,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::SORT: {
      if (WalkAccessPathsForExplain(thd, path->sort().child,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      if (!thd->lex->explain_format->is_tree() && path->sort().filesort) {
        if (join && join->px_encounter_exchange) {
          plan_slice->get_explain_flags()->set(ESC_ORDER_BY, ESP_EXISTS);
          plan_slice->get_explain_flags()->set(ESC_ORDER_BY, ESP_USING_FILESORT);
        }
      }
      break;
    }
    case AccessPath::AGGREGATE: {
      if (WalkAccessPathsForExplain(thd, path->aggregate().child,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::TEMPTABLE_AGGREGATE: {
      if (WalkAccessPathsForExplain(thd, path->temptable_aggregate().subquery_path,
                                    exchange_context, exchange_count, join, plan_slice) ||
          WalkAccessPathsForExplain(thd, path->temptable_aggregate().table_path,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      if (!thd->lex->explain_format->is_tree()) {
        if (join && join->px_encounter_exchange) {
          plan_slice->get_explain_flags()->set(ESC_GROUP_BY, ESP_EXISTS);
          plan_slice->get_explain_flags()->set(ESC_GROUP_BY, ESP_USING_TMPTABLE);
        }
      }
      break;
    }
    case AccessPath::LIMIT_OFFSET: {
      if (WalkAccessPathsForExplain(thd, path->limit_offset().child,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::STREAM: {
      if (WalkAccessPathsForExplain(thd, path->stream().child,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::MATERIALIZE: {
      MaterializePathParameters *param = path->materialize().param;
      for (auto qb : param->query_blocks) {
        // change the current join.
        if (WalkAccessPathsForExplain(thd, qb.subquery_path,
                                      exchange_context, exchange_count, qb.join, plan_slice)) {
          return true;
        }

        /*
          If the query block has ended but the plan slice has not ended(the end is PX_SEND),
          create a plan slice according to current plan slice and attach it to the current 
          query block.
        */
        assert(thd->lex->explain_format->is_tree() || plan_slice);
        if (!thd->lex->explain_format->is_tree() && (qb.join && qb.join->px_encounter_exchange) &&
            !plan_slice->attached() && plan_slice->tables()) {
          assert(qb.join);
          PX_plan_slice *slice = new (thd->mem_root) PX_plan_slice();
          if (!slice) {
            my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
            return true;
          }
          plan_slice->copy_to(*slice);
          slice->set_attach();
          qb.join->px_plan_slices.emplace_back(slice);
          plan_slice->reset();
        }
      }
      if (WalkAccessPathsForExplain(thd, path->materialize().table_path,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::MATERIALIZE_INFORMATION_SCHEMA_TABLE: {
      if (WalkAccessPathsForExplain(thd, path->materialize_information_schema_table().table_path,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::APPEND: {
      const auto &param = path->append();
      for (AppendPathParameters child : *param.children) {
        // change the current join
        if (WalkAccessPathsForExplain(thd, child.path,
                                      exchange_context, exchange_count, child.join, plan_slice)) {
          return true;
        }
        /*
          If the query block has ended but the plan slice has not ended(the end is PX_SEND),
          create a plan slice according to current plan slice and attach it to the current 
          query block.
        */
        assert(thd->lex->explain_format->is_tree() || plan_slice);
        if (!thd->lex->explain_format->is_tree() && (child.join && child.join->px_encounter_exchange) && 
            !plan_slice->attached() && plan_slice->tables()) {
          assert(child.join);
          PX_plan_slice *slice = new (thd->mem_root) PX_plan_slice();
          if (!slice) {
            my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
            return true;
          }
          plan_slice->copy_to(*slice);
          slice->set_attach();
          child.join->px_plan_slices.emplace_back(slice);
          plan_slice->reset();
        }
      }
      break;
    }
    case AccessPath::WINDOW: {
      if (WalkAccessPathsForExplain(thd, path->window().child,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::WEEDOUT: {
      if (WalkAccessPathsForExplain(thd, path->weedout().child,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::REMOVE_DUPLICATES: {
      if (WalkAccessPathsForExplain(thd, path->remove_duplicates().child,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::ALTERNATIVE: {
      if (WalkAccessPathsForExplain(thd, path->alternative().child,
                                    exchange_context, exchange_count, join, plan_slice) ||
          WalkAccessPathsForExplain(thd, path->alternative().table_scan_path,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
    case AccessPath::CACHE_INVALIDATOR: {
      if (WalkAccessPathsForExplain(thd, path->cache_invalidator().child,
                                    exchange_context, exchange_count, join, plan_slice)) {
        return true;
      }
      break;
    }
  }

  return false;
}


#include "sql/sql_count_conversion.h"
#include "sql/item.h"
#include "sql/item_sum.h"
#include "sql/parallel_execution/px_interface.h"
#include "sql/opt_trace.h"  // Opt_trace_object
#include "sql/opt_trace_context.h"

void inc_txsql_implicit_convert_to_count_zero() {
  current_thd->status_var.txsql_implicit_convert_to_count_zero++;
}

void inc_txsql_total_count_function() {
  current_thd->status_var.txsql_total_count_function++;
}

bool can_convert_to_zero(Item *item) {
  return (item && item->type() == Item::FIELD_ITEM && !item->is_nullable());
}

bool count_col_conversion(THD *thd, Item_sum_count *item_count) {
  inc_txsql_total_count_function();
  Opt_trace_context *const trace = &thd->opt_trace;

  if (can_convert_to_zero(item_count->get_arg(0))) {
      Field *field = ((Item_field *)item_count->get_arg(0))->field;
    TABLE *table = field->table;
    table->release_field(field->field_index());
    item_count->set_arg(thd, 0, new (thd->mem_root) Item_int(int32{0}, 1));
    item_count->is_coverted_count_zero = true;
    inc_txsql_implicit_convert_to_count_zero();
    Opt_trace_object trace_cnt_conversion(trace);
    trace_cnt_conversion.add("using count conversion", true);
  }

  return false;
}

bool count_col_conversion(THD *thd, const mem_root_deque<Item *> &fields) {
  DBUG_TRACE;

  assert(thd && thd->variables.txsql_count_conversion_enabled);
  for (Item *item : fields) {
    if (item->type() == Item::SUM_FUNC_ITEM &&  !item->m_is_window_function) {
      if (item->is_outer_reference()) {
        continue;
      }

      /*
        MySQL will convert count(*) to count(0), and 0 has two meanings:the field
        value is not considered and it will not be NULL.Therefore, count(*) counts
        all values that contain NULL. Count(col), will add col to read_set, that is,
        need to copy col to TABLE::record[0], and will not count the null value in
        col. However, if col can ensure that it is not NULL, for example, there is
        a not null restriction in the table definition, and col does not belong to
        the inner table of left join. Then count(col) is equivalent to count(0).
      */
      Item_sum *item_sum = down_cast<Item_sum *>(item);
      if (item_sum->real_sum_func() == Item_sum::COUNT_FUNC &&
          count_col_conversion(thd, (Item_sum_count *)item_sum)) {
        return true;
      }
    }
    if (thd->is_error()) return true;
  }
  return false;
}
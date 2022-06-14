#include "sql/parallel_execution/px_optimizer.h"

#include "sql/join_optimizer/access_path.h"
#include "sql/parallel_execution/px_access_path.h"
#include "sql/sql_class.h"

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

  if (!split_positions_all.size()) {
    thd->lex->pass_px_check = false;
    return false;
  }

  // Optimize
  std::vector<px_access_path::Split_Position> split_positions;
  try {
    if (px_access_path::FindExchangeInjectPosition(thd, &split_positions_all,
                                                   &split_positions)) {
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
  return false;
}

/**
  Parallel optimize for this query expression.
  Travel all query blocks to split aggregate/sort/.. and inject exchange if it
  has passed the compatibility check.
*/
bool px_generate_plan(
    THD *thd, std::vector<px_access_path::Split_Position> *split_positions,
    std::vector<AccessPath *> *mat_access_path) {
  return false;
}

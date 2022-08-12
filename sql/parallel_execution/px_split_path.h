#ifndef PX_SPLIT_PATH
#define PX_SPLIT_PATH

#include <sys/types.h>

class THD;
class JOIN;
class AccessPath;

AccessPath *WalkAccessPathsForAggregationSplit(THD *thd, JOIN *join,
                                               AccessPath *const path,
                                               bool stream_agg);

void RebuildCurrentRefItems(THD *thd, JOIN *join, uint curr_slice, bool is_final_aggr);

bool FixSortAccessPathForAggrInject(THD *thd, JOIN *join, AccessPath *path, int ref_slice);

bool FixSubqueryInProjection(THD *thd, JOIN *join);

#endif

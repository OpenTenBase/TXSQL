#ifndef PX_EXPLAIN_INCLUDED
#define PX_EXPLAIN_INCLUDED

#include <sys/types.h>

class THD;
class AccessPath;
class PX_exchange_context;
class JOIN;
class PX_plan_slice;

bool WalkAccessPathsForExplain(THD *thd, AccessPath *path,
                               PX_exchange_context *exchange_context,
                               uint &exchange_count, JOIN *join,
                               PX_plan_slice *plan_slice);

#endif
#ifndef PX_OPTIMIER_INCLUDED
#define PX_OPTIMIER_INCLUDED

#include <vector>

#include "my_inttypes.h"  // ulong

class AccessPath;
class JOIN;
class SELECT_LEX_UNIT;
class THD;
struct TABLE_LIST;
namespace px_access_path {
class Split_Position;
};

void check_parallel_table_hint(const THD *thd, bool do_parallel);

void set_parallel_degree_hint(const THD *thd, TABLE_LIST *tbl);

ulong get_parallel_degree_hint(const THD *thd, bool should_effect);

bool px_optimize(THD *thd, JOIN *join, AccessPath *root);

bool px_generate_plan(
    THD *thd, std::vector<px_access_path::Split_Position> *split_positions,
    std::vector<AccessPath *> *mat_access_path);

#endif

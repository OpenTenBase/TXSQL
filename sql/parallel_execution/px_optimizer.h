#ifndef PX_OPTIMIER_INCLUDED
#define PX_OPTIMIER_INCLUDED

#include <vector>

class AccessPath;
class JOIN;
class SELECT_LEX_UNIT;
class THD;
namespace px_access_path {
class Split_Position;
};

bool px_optimize(THD *thd, JOIN *join, AccessPath *root);

bool px_generate_plan(
    THD *thd, std::vector<px_access_path::Split_Position> *split_positions,
    std::vector<AccessPath *> *mat_access_path);

#endif

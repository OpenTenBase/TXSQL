#ifndef PX_ACCESS_PATH_INCLUDED
#define PX_ACCESS_PATH_INCLUDED

#include <stdint.h>
#include <sys/types.h>
#include <vector>


class Filesort;
class JOIN;
class THD;
struct AccessPath;
struct TABLE;

namespace px_access_path {
struct Split_Position {
  JOIN *m_join;
  AccessPath *m_target;
  AccessPath *m_parent;
  uint m_ref_slice;
  bool m_split_agg;
  bool m_split_sort;
  bool m_split_union;
  Filesort *m_filesort;
  std::vector<TABLE *> *m_tables;
  Split_Position()
      : m_join(nullptr),
        m_target(nullptr),
        m_parent(nullptr),
        m_ref_slice(0),
        m_split_agg(false),
        m_split_sort(false),
        m_split_union(false),
        m_filesort(nullptr),
        m_tables(nullptr) {}

  Split_Position(JOIN *join, AccessPath *target, AccessPath *parent,
                 uint ref_slice, bool split_agg, bool split_sort,
                 bool split_union, Filesort *filesort,
                 std::vector<TABLE *> *tables)
      : m_join(join),
        m_target(target),
        m_parent(parent),
        m_ref_slice(ref_slice),
        m_split_agg(split_agg),
        m_split_sort(split_sort),
        m_split_union(split_union),
        m_filesort(filesort),
        m_tables(tables) {}
};

bool WalkAccessPathsForCompat(
    THD *thd, AccessPath *path, AccessPath *parent, JOIN *cur_join,
    bool parallel_scan, bool root, bool root_all, uint &ref_slice,
    AccessPath *&max_px_subpath, bool &is_stream,
    std::vector<AccessPath *> *const mat_access_path,
    std::vector<Split_Position> *const split_positions, bool &exchange_safe);

bool FindExchangeInjectPosition(THD *thd,
    std::vector<Split_Position> *const split_positions_in,
    std::vector<Split_Position> *const split_positions);

#ifndef DBUG_OFF
void PrintSplitPostion(const char *str,
                       std::vector<Split_Position> *const split_positions);
#endif
}  // namespace px_access_path

#endif

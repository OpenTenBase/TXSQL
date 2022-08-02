#ifndef PX_ACCESS_PATH_INCLUDED
#define PX_ACCESS_PATH_INCLUDED

#include <stdint.h>
#include <sys/types.h>
#include <list>
#include <vector>
#include "lex_string.h"
#include "sql/item.h"
#include "sql/table.h"
#include "sql/sql_opt_exec_shared.h"

class Filesort;
class JOIN;
class THD;
struct AccessPath;
struct TABLE;
class QEP_TAB;

template <class T>
class mem_root_deque;

constexpr uint MAX_EXCHANGE_NUM =
    3;  // The maximum number of exchanges in a query block

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
  mem_root_deque<TABLE *> *m_tables;
  QEP_TAB *m_parallel_tab;  // Tab to be parallel scanned, held by a special
                            // node in the array for Split_Position. This node
                            // would only contain the m_parallel_tab.
  Split_Position()
      : m_join(nullptr),
        m_target(nullptr),
        m_parent(nullptr),
        m_ref_slice(0),
        m_split_agg(false),
        m_split_sort(false),
        m_split_union(false),
        m_filesort(nullptr),
        m_tables(nullptr),
        m_parallel_tab(nullptr) {}

  Split_Position(JOIN *join, AccessPath *target, AccessPath *parent,
                 uint ref_slice, bool split_agg, bool split_sort,
                 bool split_union, Filesort *filesort,
                 mem_root_deque<TABLE *> *tables)
      : m_join(join),
        m_target(target),
        m_parent(parent),
        m_ref_slice(ref_slice),
        m_split_agg(split_agg),
        m_split_sort(split_sort),
        m_split_union(split_union),
        m_filesort(filesort),
        m_tables(tables),
        m_parallel_tab(nullptr) {}

  Split_Position(JOIN *join, QEP_TAB *parallel_tab)
      : m_join(join),
        m_target(nullptr),
        m_parent(nullptr),
        m_ref_slice(0),
        m_split_agg(false),
        m_split_sort(false),
        m_split_union(false),
        m_filesort(nullptr),
        m_tables(nullptr),
        m_parallel_tab(parallel_tab) {}
};

bool WalkAccessPathsForCompat(
    THD *thd, AccessPath *path, AccessPath *parent, JOIN *cur_join,
    bool parallel_scan, bool root, bool root_all, uint &ref_slice,
    AccessPath *&max_px_subpath, bool &is_stream,
    std::vector<AccessPath *> *const mat_access_path,
    std::vector<Split_Position> *const split_positions, bool &exchange_safe);

bool FindExchangeInjectPosition(THD *thd,
    std::vector<Split_Position> *const split_positions_in,
    std::vector<Split_Position> *const split_positions,
    std::list<QEP_TAB *> *const parallel_tab);

// return the number of exchange points in all split positions
size_t count_exchange_in_split_pos(
    std::vector<Split_Position> *const split_positions);

#ifndef DBUG_OFF
void PrintSplitPostion(const char *str,
                       std::vector<Split_Position> *const split_positions);
#endif
}  // namespace px_access_path

struct ReceiverParam {
  int ref_slice;
  TABLE *table;
  AccessPath *child;
  bool use_temp_table;
};

class EquivalenceCheckHelper {
public:
  static inline bool eq_table_share (const TABLE *a, const TABLE *b) {
    if (a == nullptr || b == nullptr) {
      if (a == nullptr && b == nullptr) {
        return true;
      }
      return false;
    }
    if (a->s->table_category == TABLE_CATEGORY_TEMPORARY &&
        b->s->table_category == TABLE_CATEGORY_TEMPORARY) {
      return true;
    }
    return (a->s == b->s);
  }

  static inline bool eq_lex_string (const LEX_CSTRING *a, const LEX_CSTRING *b) {
    if (a == nullptr || b == nullptr) {
      if (a == nullptr && b == nullptr) {
        return true;
      }
      return false;
    }
    if (a->length != b->length || strncmp(a->str, b->str, a->length) != 0) {
      return false;
    }
    return true;
  }

  static inline bool eq_table_share_without_icp (const TABLE *a, const TABLE *b) {
    if (a == nullptr || b == nullptr) {
      return false;
    }
    assert(a->file->pushed_idx_cond == nullptr);
    assert(b->file->pushed_idx_cond == nullptr);
    if (a->s->table_category == TABLE_CATEGORY_TEMPORARY &&
        b->s->table_category == TABLE_CATEGORY_TEMPORARY) {
      return true;
    }
    return (a->s == b->s);
  }

  static inline bool eq_table_ref (const TABLE_REF *a, const TABLE_REF *b) {
    if (a == nullptr || b == nullptr) {
      if (a == nullptr && b == nullptr) {
        return true;
      }
      return false;
    }
    if (a->key_parts != b->key_parts ||
        a->key_length != b->key_length) {
      return false;
    }
    for (unsigned key_part_idx = 0; key_part_idx < a->key_parts;
        ++key_part_idx) {
      bool *cond_guard_a = a->cond_guards[key_part_idx];
      bool *cond_guard_b = b->cond_guards[key_part_idx];
      if (cond_guard_a == nullptr || cond_guard_b == nullptr) {
        if (cond_guard_a == nullptr && cond_guard_b == nullptr) {
          continue;
        }
        return false;
      }
      if (*cond_guard_a != *cond_guard_b) {
        return false;
      }
    }
    return true;
  }

  static inline bool eq_item (const Item *a, const Item *b) {
    if (a != nullptr) {
      if (b == nullptr) {
        return false;
      }
      // item equivalence check (recursive call eq_item)
      return a->eq(b, true);
    }
    if (b != nullptr) {
      return false;
    }
    return true;
  }
};

void FixSortAccessPath(JOIN *join, AccessPath *path, TABLE *const new_table,
                       int ref_slice);

AccessPath *WalkAccessPathsForExchange(
    THD *thd, JOIN *join, px_access_path::Split_Position *split_pos,
    AccessPath *const path, AccessPath *const target_path, uint curr_exchange,
    bool &new_child, int cur_slice, bool alloc_group_field, bool in_join);

AccessPath *CreateExchangeAccessPathForUnion(THD *thd, AccessPath *const path,
                                             TABLE *table,
                                             bool is_append = false);

void GetExchangeTables(px_access_path::Split_Position *split_pos);

void ConnectAccessPathWithChildExchange(AccessPath *const path,
                                        AccessPath *receiver);
#endif

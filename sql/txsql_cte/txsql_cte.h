
#ifndef TXSQL_CTE_H
#define TXSQL_CTE_H

#include <iostream>
#include <unordered_map>
#include "sql/mem_root_array.h"
#include "include/my_alloc.h"
#include "sql/txsql_hash/txsql_hash.h"

class THD;
struct TABLE_LIST;
struct TABLE;

namespace txsql {

enum class CTE_Type : uint8_t {
  INVALID = 0,
  VIEW
};

/**
  This structure represents an encapsulation of a structural object - such as a subplan,
  view object, etc. After a structure is encapsulated as CTE_node, it is used as the
  basic unit to detect whether it is repeated.
  Note: Any CTE_node needs to have the Equals interface and the hash interface to achieve
  duplicate detection.
*/
class CTE_node {
 public:
    CTE_node(CTE_Type type) : type(type) {}
    virtual ~CTE_node() {}
	  CTE_Type GetCteType() const { return type; }
    CTE_Type type{CTE_Type::INVALID};
 public:
  virtual bool IsView() const = 0;
  /**
    Creates a hash value of this CTE_node.

    It is important that if two CTE_node are identical
    (i.e. CTE_node::Equals() returns true), that their
    hash value is identical as well.
  */
  virtual txsql::hash_t Hash() const = 0;
  virtual bool Equals(const CTE_node *other) const;
  static bool Equals(const CTE_node *left, const CTE_node *right) {
		if (left == right) { return true;}
		if (!left || !right) { return false; }
		return left->Equals(right);
	}
	bool operator==(const CTE_node &rhs) { return this->Equals(&rhs); }
};

class CTE_view : public CTE_node {
  public:
    CTE_view(const TABLE_LIST *view);
    ~CTE_view() {}

  public:
    virtual bool IsView() const override { return true; }
    virtual txsql::hash_t Hash() const override;
    virtual bool Equals(const CTE_node *other) const override;
    const TABLE_LIST *getView() { return m_view; }
  private:
    // The target table of the current node 
    const TABLE_LIST *m_view{nullptr};
};

/**
  The CTE_view_expr structure represents a set of CTEs converted from subplan.
  Each repeated CTE_view will correspond to a TABLE_LIST (representing target
  table). A set of repeated subplans is maintained using tmp_tables, where
  tmp_tables[0] represents the producer - which will perform complete
  evaluation and materialization, and tmp_tables subsequently represents the
  consumer, which reads the results directly from the materialized TABLE.
*/
struct CTE_view_expr {
  CTE_view_expr(MEM_ROOT *mem_root);
  bool is_cte_consumer(TABLE_LIST *table) const;
  TABLE *clone_tmp_table(THD *thd, TABLE_LIST *tl);
  void remove_table(TABLE_LIST *tr);

  uint count;
  Mem_root_array<TABLE_LIST *> tmp_tables;
};

struct CteHashFunction {
	uint64_t operator()(const CTE_node *const &node) const {
		return (uint64_t)node->Hash();
	}
};

struct CteEqualFunction {
	bool operator()(const CTE_node *const &a, const CTE_node *const &b) const {
		return a->Equals(b);
	}
};

template <typename T>
using cte_map_t = std::unordered_map<CTE_node *, T *, CteHashFunction, CteEqualFunction>;

template <typename T>
void cte_clear(cte_map_t<T> *m) {
  for (auto it = m->begin(); it != m->end(); ++it) {
    destroy(it->first);
    destroy(it->second);
  }
  m->clear();
}

}

#endif
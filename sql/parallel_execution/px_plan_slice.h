#ifndef PX_PLAN_SLICE_INCLUDED
#define PX_PLAN_SLICE_INCLUDED

#include "sql/sql_executor.h" // QEP_TAB
#include "sql/opt_explain_format.h"
#include <vector>

class QEP_TAB;
class Explain_format_flags;

class PX_plan_slice {
 public:
  PX_plan_slice() {}
  ~PX_plan_slice() { clean(); }
  
  void copy_to(PX_plan_slice &slice) {
    slice.get_explain_flags()->set(*get_explain_flags());
    for (auto tab : m_tabs) {
      slice.add_tab(tab);
    }
  }

  void reset() {
    m_tabs.clear();
    m_tables = false;
    Explain_format_flags tmp_flag;
    m_explain_flags.set(tmp_flag);
    m_attached = false;
  }

  uint tables() { return m_tables; }
  void set_tables(uint tables) { m_tables = tables; }
  Explain_format_flags *get_explain_flags() { return &m_explain_flags; }
  bool attached() { return m_attached; }
  void set_attach() { m_attached = true; }

  void add_tab(QEP_TAB *tab) {
    m_tabs.push_back(tab);
    ++m_tables;
  }

  QEP_TAB *get_tab(uint tabnum) {
    assert(tabnum < m_tables);
    return m_tabs[tabnum];
  }

 private:
  void clean() {
    for (auto qep : m_tabs) {
      if (qep->exchange_type != QEP_TAB::Exchange_none) destroy(qep);
    }
    m_tabs.clear();
    m_tables = 0;
    m_attached = false;
  }

 private:
  uint m_tables{0};
  std::vector<QEP_TAB *> m_tabs;
  Explain_format_flags m_explain_flags;
  // Indicates the plan slice has attached to a query block.
  bool m_attached{false};
};

#endif  // PX_PLAN_SLICE_INCLUDED
/**
  @file sql/parallel_execution/opt_dbug.h

  Interactive DBUG session state.
*/

#ifndef OPT_DBUG_INCLUDED
#define OPT_DBUG_INCLUDED

#include <vector>
#include "my_dbug.h"
#include "mysql/components/services/bits/psi_memory_bits.h"  // PSI_memory_key

#ifndef DBUG_OFF
/**
  Interactive DBUG session state, as a list of "SET session debug = val".
  Note that "SET SESSION debug = default" clears the list.

  Recorded to be replayed on any target thread so that it has the same
  effective DBUG setting stack. See Sys_var_dbug.

  The DBUG session is designed to be owned by the recorder and referenced by
  all replayers. The recorder should never update the dbug session when it is
  referenced by any replayer.
 */
class Opt_dbug_session {
 public:
  Opt_dbug_session(PSI_memory_key psi_memory_key)
      : m_psi_memory_key(psi_memory_key) {}

  ~Opt_dbug_session() {
    for (auto &val : m_dbug_vals) {
      my_free(const_cast<char *>(val));
    }
    m_dbug_vals.clear();
  }

  void dbug_set(const char *val) {
    DBUG_TRACE;
    DBUG_PRINT("info", ("dbug_set %s", val));
    const char *v = my_strdup(m_psi_memory_key, val,
                              MYF(MY_WME));
    m_dbug_vals.push_back(v);
  }

  void dbug_pop() {
    DBUG_TRACE;
    DBUG_PRINT("info", ("dbug_pop"));
    for (auto &v : m_dbug_vals) {
      my_free(const_cast<char *>(v));
    }
    m_dbug_vals.clear();
  }

  bool dbug_init_thd(THD *thd) {
    // Must be called with current thd, because DBUG stack is thread local.
    assert(current_thd == thd);
    DBUG_TRACE;
    for (auto &val : m_dbug_vals) {
      DBUG_SET(val);
    }
    // Separated for full dbug stack.
    for (auto &val : m_dbug_vals) {
      DBUG_PRINT("info", ("DBUG_SET %s", val));
    }
    return false;
  }

 private:
  std::vector<const char*> m_dbug_vals;
  PSI_memory_key m_psi_memory_key;
};
#endif

#endif  // OPT_DBUG_INCLUDED

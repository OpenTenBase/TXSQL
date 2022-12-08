/*  Copyright (c) 2021, Tencent and/or its affiliates.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2.0,
    as published by the Free Software Foundation.

    This program is also distributed with certain software (including
    but not limited to OpenSSL) that is licensed under separate terms,
    as designated in a particular file or component or in included license
    documentation.  The authors of MySQL hereby grant you an additional
    permission to link the program and your derivative works with the
    separately licensed software that they have included with MySQL.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License, version 2.0, for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */
#ifndef STATEMENT_OUTLINE_OUTLINE_INCLUDED
#define STATEMENT_OUTLINE_OUTLINE_INCLUDED

#include "my_config.h"

#include <functional>
#include <memory>
#include <string>

#include "map_helpers.h"
#include "sql/statement_outline/rule.h"
#include "sql/auth/partitioned_rwlock.h"

/**
  @file sql/statement_outline/outline.h

  Facade for the statement outline. Provides the Outline class which performs
  all of the functionality.
*/

namespace std {
/// Define a hash function object for DigestSchemaHashKey used by
/// Outline::m_rules
template <>
struct hash<statement_outline::DigestSchemaHashKey> {
  std::size_t operator()(
      const statement_outline::DigestSchemaHashKey &k) const {
    return k.hash_value();
  }
};
}

namespace statement_outline {
class Persisted_rule;
class Outline;
extern PSI_rwlock_key psi_key_rwlock_outline_rules;

/**
  Each operation of statement outline are performed by a worker thread, This
  class is the base class of worker thread argument. The driver routine will
  cast the argument to this real derived class. @sa routines:
  do_*(Outline_worker_oper_info *).
*/
struct Outline_worker_oper_info {
  enum class Oper_type {
    Refresh,     /// Reload rules from the system table.
    AddRule,     /// Add a outline rule.
    DeleteRule,  /// Delete a outline rule.
    ToggleRule   /// Toggle a outline rule' enabled or not.
  };
  Oper_type oper_type;
  Outline *me;
  THD *session_thd;  /// Worker thread use a standalone thread descriptor.
  /// The error message to return to caller thread if worker thread raise error.
  std::string error_message;
  Outline_worker_oper_info(Oper_type op)
      : oper_type(op) {}
};

/**
  Implementation of statement outline after query parse. The public interface
  consists of 5 operations:

  - refresh(), which loads the rules from the disk table
  - apply_to_query(), which apply outline rules to a query if applicable
  - add_rule_from_query(), which add rules to memory and flush to disk table
    given a query with hints
  - delete_rule(), delete a rule from memory and disk table.
  - enable_rule(), disable or enable a rule.
*/
class Outline {
 public:
  Outline(uint nparts);
  ~Outline();

  /**
    Attempts to apply outline rules to thd's current query.
  */
  void apply_to_query(THD *thd, const uchar *key);
  /**
    Add a outline rule to digest hash map (m_digests) and save it to disk table.

    @param database  set it to current database when worker thread parse the
                     query.
    @param query     The function extract optimizer hints from this query.
  */
  longlong add_rule_from_query(const char *database, const char *query);
  /// Delete a rule from cache and disk table given a rule_id.
  bool delete_rule(THD *thd, longlong rule_id);
  /// Enable (when @param enable is true) or disable a rule from cache and disk
  /// table given a rule_id.
  bool enable_rule(THD *thd, longlong rule_id, bool enable);
  /// Empty the hashtable and reload all rules from disk table.
  bool refresh(THD *thd);

  /**
    Implementation of the loading procedure. The server doesn't handle
    different sessions in the same thread, so we load the rules into the hash
    table in this function, intended to be run in a new thread. The main
    thread will do join().

    @param oper_info  The thread argument of worker thread. It is class
                      Outline_worker_refresh_info which derived from
                      Outline_worker_oper_info
  */
  void do_refresh(Outline_worker_oper_info *oper_info);
  /**
    Implementation of the rule adding procedure. @sa do_refresh()
  */
  void do_add_rule(Outline_worker_oper_info *oper_info);
  /**
    Implementation of the rule deleting procedure. @sa do_refresh()
  */
  void do_delete_rule(Outline_worker_oper_info *oper_info);

  /**
    Implementation of the rule enable or disable procedure. @sa do_refresh()
  */
  void do_toggle_rule(Outline_worker_oper_info *oper_info);
  void invoke_auxiliary_worker(Outline_worker_oper_info *oper_info);
  void foreach_rule(THD *thd, const std::function<void(Rule *)> &accessor);
 private:
  uint m_partitions;
  using rule_multimap =
      malloc_unordered_multimap<DigestSchemaHashKey, std::shared_ptr<Rule>>;
  std::vector<rule_multimap> m_rule_partitions;
  Partitioned_rwlock m_lock;
  inline rule_multimap &current_rule_partition(THD *thd);

  /// Find the rule in the m_digests given rule_id, the complexity is O(n)
  inline rule_multimap::iterator find_rule_by_id(rule_multimap &rule_map,
                                          longlong rule_id);
  /// Loads the rule retrieved from the disk table.  Called by auxiliary thread.
  bool load_rule(THD *thd, Persisted_rule *diskrule, longlong &rule_id,
                 std::string &errmsg);
  /// flush the rule to the disk table, called by auxiliary thread.
  bool save_rule(THD *thd, Rule *rule, std::string &error_message);
};

// Following declares for functions that are implemented in outline_main.cc they
// are just wrappers members of Outline and called by proc.cc.

/// Load outline rules from disk table adn it is called when system startup
bool refresh_rules_table(THD *thd);
longlong add_rule_from_query(THD *thd, const char *database, const char *query);
bool delete_rule(THD *thd, longlong rule_id);
bool enable_rule(THD *thd, longlong rule_id, bool enable);
/// Iterate all Rule objects and call @param accessor for each one, Note the
/// @param accessor is only allowed to read because the function just hold read
/// lock.
void foreach_rule(THD *thd, const std::function<void(Rule *)> &accessor);
}

#endif /* STATEMENT_OUTLINE_OUTLINE_INCLUDED */

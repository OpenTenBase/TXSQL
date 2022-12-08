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

#include "sql/statement_outline/outline.h"
#include <mysql/components/services/log_builtins.h>
#include <mysql/service_parser.h>  // mysql_parser_start_thread
#include "nullable.h"
#include "sql/sql_class.h"
#include "sql/statement_outline/parser.h"

/**
  @file sql/statement_outline/outline.cc
  Implementation of the Outline class's member functions.
*/

using Mysql::Nullable;
using std::string;

namespace statement_outline {
/// Out system log if there are some errors when loading rules from disk table
/// when system startup.
static void log_error(const char *msg) {
  LogErr(ERROR_LEVEL, ER_STATEMENT_OUTLINE_LOAD_ERROR, msg);
}

static bool check_cursor(const Cursor &c, THD *thd, string &error_message,
                         bool write_log) {
  if (c.table_is_malformed()) {
    error_message = "Wrong column count or names in rules table when accessing rules.";
    if (write_log) log_error(error_message.c_str());
    return true;
  }

  // ha_init_rnd() could return an error.
  if (c.had_serious_error()) {
    char errmsg[1024];
    assert(thd->is_error());
    snprintf(errmsg, sizeof(errmsg), "Got error \"%s\" from storage engine.",
             thd->get_stmt_da()->message_text());
    error_message = errmsg;
    if (write_log) log_error(errmsg);
    return true;
  }

  return false;
}

/**
  Create a auxiliary session to modify outline rule table. @sa
  mysql_parser_open_session().
*/
class Session {
 public:
  Session()
      : m_current_session(mysql_parser_open_session()) {}

  THD *thd() { return m_current_session; }

 private:
  /// The thd will be released inside of auxiliary thread.
  THD *m_current_session;
};

constexpr int max_warning_invalid_rules = 10000;
using List_invalid_rule = std::vector<std::pair<longlong, std::string>>;

struct Outline_worker_refresh_info : Outline_worker_oper_info {
  bool is_background;
  /// Save invalid rules to output warnings.
  List_invalid_rule invalid_rules;
  /// The warning message after all rules read.
  string warning_message;
  Outline_worker_refresh_info( bool bckgrd)
      : Outline_worker_oper_info(Oper_type::Refresh),
        is_background(bckgrd) {}
  void push_invalid_rule(longlong rule_id, string warning) {
    if (invalid_rules.size() < max_warning_invalid_rules)
      invalid_rules.emplace_back(rule_id, warning);
  }
};

struct Outline_worker_add_info : Outline_worker_oper_info {
  const char *database;
  const char *query;
  longlong added_rule_id;
  Outline_worker_add_info(const char *db, const char *q)
      : Outline_worker_oper_info(Oper_type::AddRule), database(db), query(q) {}
};

struct Outline_worker_delete_info : Outline_worker_oper_info {
  Rule *rule;
  Outline_worker_delete_info(Rule *r)
      : Outline_worker_oper_info(Oper_type::DeleteRule), rule(r) {}
};

struct Outline_worker_toggle_info : Outline_worker_oper_info {
  Rule *rule;
  bool enabled;
  Outline_worker_toggle_info(Rule *r, bool ena)
      : Outline_worker_oper_info(Oper_type::ToggleRule), rule(r), enabled(ena) {}
};

extern "C" void *outline_worker_callback(void *p_args) {
  Outline_worker_oper_info *info =
      pointer_cast<Outline_worker_oper_info *>(p_args);
  switch (info->oper_type) {
    case Outline_worker_oper_info::Oper_type::Refresh:
      (info->me->do_refresh)(info);
      break;
    case Outline_worker_oper_info::Oper_type::AddRule:
      (info->me->do_add_rule)(info);
      break;
    case Outline_worker_oper_info::Oper_type::DeleteRule:
      (info->me->do_delete_rule)(info);
      break;
    case Outline_worker_oper_info::Oper_type::ToggleRule:
      (info->me->do_toggle_rule)(info);
      break;
    default:
      assert(0);
  }
  return nullptr;
}

Outline::Outline(uint nparts) : m_partitions(nparts) {
  for (uint i = 0; i < nparts; i++)
    m_rule_partitions.emplace_back(psi_key_memory_statement_outline);
  m_lock.init(nparts
#ifdef HAVE_PSI_INTERFACE
              ,
              psi_key_rwlock_outline_rules
#endif
  );
}

Outline::~Outline() {
  m_lock.destroy();
}

/// Copy error message from auxiliary thread diagnostics area.
static std::string get_error_message_from(THD *thd) {
  assert(thd->is_error());
 return thd->get_stmt_da()->message_text();
}

bool Outline::load_rule(THD *thd, Persisted_rule *diskrule, longlong &rule_id,
                        std::string &errmsg) {
  std::shared_ptr<Rule> memrule_ptr(new (std::nothrow) Rule);
  Rule *memrule = memrule_ptr.get();

  if (!memrule) {
    errmsg = "Out of memory";
    return true;
  }
  Rule::Load_status load_status = memrule->load(thd, diskrule);

  switch (load_status) {
    case Rule::OK:
      for (auto &rule_map : m_rule_partitions)
        rule_map.emplace(memrule->digest_and_schema(), memrule_ptr);
      return false;
      break;
    case Rule::INVALID_DIGEST:
      rule_id = memrule->id();
      errmsg = "Invalid statement digest.";
      break;
    case Rule::HINTS_PARSE_ERROR:
      rule_id = memrule->id();
      errmsg = "Invalid optimizer hint in outline column.";
      break;
  }

  return true;
}

bool Outline::save_rule(THD *thd, Rule *rule, std::string &error_message) {
  DBUG_TRACE;
  Cursor c(thd, Cursor::Mode::Write);

  if (check_cursor(c, thd, error_message, false)) return true;

  DBUG_EXECUTE_IF("stmtol.simulate_write_se_error",
                  {DBUG_SET("+d,simulate_storage_engine_out_of_memory");});

  Persisted_rule diskrule;
  if (rule->save(&diskrule)) return true;
  bool res = diskrule.write_to(&c);
  if (!res) rule->load_id(&diskrule);

  return res;
}

/// Launch auxiliary worker and wait for it to finish its work.
void Outline::invoke_auxiliary_worker(Outline_worker_oper_info *oper_info) {
  int res;
  my_thread_handle handle;
  Session session;
  oper_info->me = this;
  oper_info->session_thd = session.thd();
  if ((res = mysql_parser_create_thread(session.thd(), outline_worker_callback,
                                        oper_info, &handle)) != 0) {
    oper_info->error_message =
        "Fail to start outline auxiliary worker: errno = " +
        std::to_string(res);
    return;
  }

  mysql_parser_join_thread(&handle);
}

void Outline::do_refresh(Outline_worker_oper_info *oper_info) {
  assert(oper_info->oper_type == Outline_worker_oper_info::Oper_type::Refresh);
  Outline_worker_refresh_info *info =
      pointer_cast<Outline_worker_refresh_info *>(oper_info);
  THD *session_thd = info->session_thd;
  bool is_background = info->is_background;
  bool saw_rule_error = false;

  DBUG_TRACE;

  DBUG_EXECUTE_IF("stmtol.simulate_init_read_se_error",
                  {DBUG_SET("+d,ha_rnd_init_fail");});

  Cursor c(session_thd, Cursor::Mode::Read);
  if (check_cursor(c, session_thd, info->error_message, is_background)) return;

  Partitioned_rwlock_write_guard guard(&m_lock);
  for (auto &rule_map : m_rule_partitions)
    rule_map.clear();

  DBUG_EXECUTE_IF("stmtol.simulate_read_se_error",
                  {DBUG_SET("+d,ha_rnd_next_deadlock");});

  for (; c != Cursor::end(); ++c) {
    Persisted_rule diskrule(&c);

    if (!diskrule.id.has_value()) {
      info->push_invalid_rule(-1, "Rule id is NULL.");
    } else if (!diskrule.digest.has_value()) {
      info->push_invalid_rule(diskrule.id.value(),
                                  "Pattern digest is NULL.");
      saw_rule_error = true;
    } else if (!diskrule.outline.has_value()) {
      info->push_invalid_rule(diskrule.id.value(), "Outline is NULL.");
      saw_rule_error = true;
    } else {
      longlong rule_id;
      string rule_errmsg;
      if (load_rule(session_thd, &diskrule, rule_id, rule_errmsg)) {
        info->push_invalid_rule(rule_id, std::move(rule_errmsg));
        saw_rule_error = true;
      }
    }
  }

  if (c.had_serious_error()) {
    char errmsg[1024];
    assert(session_thd->is_error());
    snprintf(errmsg, sizeof(errmsg), "Got error \"%s\" from storage engine.",
             session_thd->get_stmt_da()->message_text());
    info->warning_message = errmsg;
    if (is_background) log_error(errmsg);
  } else if (saw_rule_error) {
    info->warning_message = "Some rules failed to load.";
    if (is_background) log_error(info->warning_message.c_str());
  }
}

bool Outline::refresh(THD *thd) {
  bool is_background = (thd == nullptr);
  Outline_worker_refresh_info info(is_background);
  invoke_auxiliary_worker(&info);

  bool is_error = info.error_message.size() > 0;
  // For background calling, we have logged error in do_refresh()
  if (is_background) return is_error;

  if (is_error) {
    assert(info.invalid_rules.size() == 0);
    my_error(ER_STATEMENT_OUTLINE_REFRESH_ERROR, MYF(0),
             info.error_message.c_str());
    return true;
  }
  // Print warnings for invalid rules
  for (auto &it : info.invalid_rules)
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_STATEMENT_OUTLINE_INVALID_RULE,
                        ER_THD(thd, ER_STATEMENT_OUTLINE_INVALID_RULE),
                        it.first, it.second.c_str());

  // Print other warnings for messages that worker thread reports.
  if (info.warning_message.size() > 0)
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_STATEMENT_OUTLINE_INVALID_RULE,
                        ER_THD(thd, ER_STATEMENT_OUTLINE_REFRESH_ERROR),
                        info.warning_message.c_str());
  return false;
}

void Outline::do_add_rule(Outline_worker_oper_info *oper_info) {
  assert(oper_info->oper_type == Outline_worker_oper_info::Oper_type::AddRule);
  Outline_worker_add_info *info =
      pointer_cast<Outline_worker_add_info *>(oper_info);
  THD *thd = info->session_thd;
  const char *database = info->database;
  const char *query = info->query;
  LEX_CSTRING db_const =  NULL_CSTR;
  LEX_CSTRING query_str;
  if (database) lex_cstring_set(&db_const, database);
  lex_cstring_set(&query_str, query);
  thd->set_db(db_const);

  if (parse_query(thd, query_str)) {
    info->error_message = get_error_message_from(thd);
    return;
  }

  std::shared_ptr<Rule> memrule_ptr(new (std::nothrow) Rule);
  Rule *memrule = memrule_ptr.get();
  if (!memrule) {
    info->error_message = "Out of memory";
    return;
  }

  memrule->set_digest_and_schema_from(thd);
  if (!parser_visit_hints(thd,
                          [memrule](uint select_number, const char *hints_str) {
                            memrule->add_new_hint(select_number, hints_str);
                          }))
  {
    info->error_message = "No hint or invalid hint is given.";
    return;
  }
  memrule->set_enabled(true);
  if (save_rule(thd, memrule, info->error_message)) {
    info->error_message = get_error_message_from(thd);
    return;
  }

  info->added_rule_id = memrule->id();

  Partitioned_rwlock_write_guard guard(&m_lock);
  for (auto &rule_map : m_rule_partitions)
    rule_map.emplace(memrule->digest_and_schema(), memrule_ptr);
}

longlong Outline::add_rule_from_query(const char *database,
                                  const char *query) {
  Outline_worker_add_info info(database, query);
  invoke_auxiliary_worker(&info);

  if (info.error_message.size() > 0) {
    my_error(ER_STATEMENT_OUTLINE_ADD_RULE_ERROR, MYF(0), info.error_message.c_str());
    return -1;
  }
  return info.added_rule_id;
}

Outline::rule_multimap &Outline::current_rule_partition(THD *thd) {
  return m_rule_partitions[thd->thread_id() % m_partitions];
}

Outline::rule_multimap::iterator Outline::find_rule_by_id(
    rule_multimap &rule_map, longlong rule_id) {
  for (auto it = rule_map.begin(); it != rule_map.end(); ++it) {
    if (it->second->id() == rule_id) return it;
  }

  return rule_map.end();
}

void Outline::do_delete_rule(Outline_worker_oper_info *oper_info) {
  assert(oper_info->oper_type ==
         Outline_worker_oper_info::Oper_type::DeleteRule);
  Outline_worker_delete_info *info =
      pointer_cast<Outline_worker_delete_info *>(oper_info);
  THD *thd = info->session_thd;
  Rule *rule = info->rule;

  Cursor c(thd, Cursor::Mode::Delete);
  if (check_cursor(c, thd, oper_info->error_message, false)) return;

  DBUG_EXECUTE_IF("stmtol.simulate_delete_se_error",
                  {DBUG_SET("+d,inject_error_ha_delete_row");});

  Persisted_rule diskrule;
  rule->save_id(&diskrule);
  if (diskrule.delete_from(&c))
    info->error_message = get_error_message_from(thd);
}

bool Outline::delete_rule(THD *thd, longlong rule_id) {
  Partitioned_rwlock_write_guard guard(&m_lock);
  auto &first_rule_map = m_rule_partitions[0];
  auto it = find_rule_by_id(first_rule_map, rule_id);

  if (it == first_rule_map.end()) {
    char errmsg[128];
    snprintf(errmsg, sizeof(errmsg), "Rule %lld is not found.", rule_id);
    push_warning_printf(
        thd, Sql_condition::SL_WARNING, ER_STATEMENT_OUTLINE_DELETE_RULE_ERROR,
        ER_THD(thd, ER_STATEMENT_OUTLINE_DELETE_RULE_ERROR), errmsg);
    return false;
  }

  Outline_worker_delete_info info(it->second.get());
  invoke_auxiliary_worker(&info);

  if (info.error_message.size() > 0) {
    my_error(ER_STATEMENT_OUTLINE_DELETE_RULE_ERROR, MYF(0),
             info.error_message.c_str());
    return true;
  }
  first_rule_map.erase(it);
  for (size_t i = 1; i < m_rule_partitions.size(); i++) {
    auto &rule_map = m_rule_partitions[i];
    auto it_delete = find_rule_by_id(rule_map, rule_id);
    assert(it_delete != rule_map.end());
    rule_map.erase(it_delete);
  }

  return false;
}

void Outline::do_toggle_rule(Outline_worker_oper_info *oper_info) {
  assert(oper_info->oper_type ==
         Outline_worker_oper_info::Oper_type::ToggleRule);
  Outline_worker_toggle_info *info =
      pointer_cast<Outline_worker_toggle_info *>(oper_info);
  THD *thd = info->session_thd;
  Rule *rule = info->rule;

  Cursor c(thd, Cursor::Mode::Update);
  if (check_cursor(c, thd, oper_info->error_message, false)) return;

  Persisted_rule diskrule;
  rule->save_id(&diskrule);
  // Toggle enabled
  diskrule.is_enabled = info->enabled;

  DBUG_EXECUTE_IF("stmtol.simulate_update_se_error",
                  {DBUG_SET("+d,handler_crashed_table_on_usage");});

  if (diskrule.update(&c))
    info->error_message = get_error_message_from(thd);
}

bool Outline::enable_rule(THD *thd, longlong rule_id, bool enable) {
  // Because we just set a atomic variable in Rule object, we read lock should
  // be enough.
  Partitioned_rwlock_read_guard guard(&m_lock, thd->thread_id());
  rule_multimap &rule_map = current_rule_partition(thd);
  auto it = find_rule_by_id(rule_map, rule_id);
  char errmsg[128];

  if (it == rule_map.end()) {
    snprintf(errmsg, sizeof(errmsg), "Rule %lld is not found.", rule_id);
    push_warning_printf(
        thd, Sql_condition::SL_WARNING, ER_STATEMENT_OUTLINE_ENABLE_RULE_ERROR,
        ER_THD(thd, ER_STATEMENT_OUTLINE_ENABLE_RULE_ERROR), errmsg);
    return false;
  }

  Rule *rule = it->second.get();

  if (rule->is_enabled() == enable) {
    snprintf(errmsg, sizeof(errmsg), "Rule %lld is already %s.", rule_id,
             enable ? "enabled" : "disabled");
    push_warning_printf(
        thd, Sql_condition::SL_WARNING, ER_STATEMENT_OUTLINE_ENABLE_RULE_ERROR,
        ER_THD(thd, ER_STATEMENT_OUTLINE_ENABLE_RULE_ERROR), errmsg);

    return false;
  }

  Outline_worker_toggle_info info(rule, enable);
  invoke_auxiliary_worker(&info);

  if (info.error_message.size() > 0) {
    my_error(ER_STATEMENT_OUTLINE_ENABLE_RULE_ERROR, MYF(0),
             info.error_message.c_str());
    return true;
  }

  rule->set_enabled(enable);

  return false;
}
void Outline::foreach_rule(THD *thd, const std::function<void(Rule *)> &accessor) {
  Partitioned_rwlock_read_guard guard(&m_lock, thd->thread_id());
  auto &curr_rule_map = current_rule_partition(thd);
  for (auto &it : curr_rule_map) accessor(it.second.get());
}

void Outline::apply_to_query(THD *thd, const uchar *key) {
  LEX_CSTRING db = thd->db();
  Partitioned_rwlock_read_guard guard(&m_lock, thd->thread_id());
  rule_multimap &cur_rule_map = current_rule_partition(thd);
  auto it_range = cur_rule_map.equal_range(DigestSchemaHashKey(key, db.str, db.length));
  for (auto it = it_range.first; it != it_range.second; ++it) {
    Rule *rule = it->second.get();
    rule->apply(thd);
  }
}
}  // namespace statement_outline

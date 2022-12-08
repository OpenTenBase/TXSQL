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

#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_rwlock.h"
#include "sql/rpl_reload_cache.h"
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"
#include "sql/statement_outline/outline.h"
#include "sql/statement_outline/parser.h"
#include "sql/statement_outline/statement_outline.h"

namespace statement_outline {
class Outline_reload : public im::Reload {
public:
  Outline_reload() {}

  virtual ~Outline_reload() {}

  virtual void execute(THD *thd) override { reload_outline_rules(); }
  virtual im::Reload_type type() const override { return im::RELOAD_STATEMENT_OUTLINE; }
};

/* Statement outline rules table key def */
LEX_CSTRING OUTLINE_RULE_TABLE_KEY = {
    STRING_WITH_LEN("mysql\0statement_outline_rules\0")};

static Outline *outline;

/// Enabled.
bool sys_var_enabled;
uint sys_var_partitions;

PSI_memory_key psi_key_memory_statement_outline;
PSI_rwlock_key psi_key_rwlock_outline_rules;

#ifdef HAVE_PSI_INTERFACE
static PSI_memory_info all_outline_memorys[] = {
    {&psi_key_memory_statement_outline, "statement_outline", 0, 0,
     PSI_DOCUMENT_ME}};

static PSI_rwlock_info all_outline_rwlocks[] = {
    {&psi_key_rwlock_outline_rules, "statement_outline_rules", 0, 0,
     PSI_DOCUMENT_ME}};

static void init_outline_psi_keys() {
  const char *category = "sql";
  int mem_count = static_cast<int>(array_elements(all_outline_memorys));
  mysql_memory_register(category, all_outline_memorys, mem_count);
  int rwlock_count = static_cast<int>(array_elements(all_outline_rwlocks));
  mysql_rwlock_register(category, all_outline_rwlocks, rwlock_count);
}
#endif

bool refresh_rules_table(THD *thd) { return outline->refresh(thd); }

/**
  Register the outline reload entry when reboot outline module.
*/
static void register_reload_entry() {
  Outline_reload *entry = new Outline_reload();
  if (entry)
    im::register_reload_entry(OUTLINE_RULE_TABLE_KEY.str,
                          OUTLINE_RULE_TABLE_KEY.length, entry);
}

/**
  Remove outline reload entry when shutdown outline module.
*/
static void remove_reload_entry() {
  im::remove_reload_entry(OUTLINE_RULE_TABLE_KEY.str,
                      OUTLINE_RULE_TABLE_KEY.length);
}
bool init_statement_outline() {
#ifdef HAVE_PSI_INTERFACE
  init_outline_psi_keys();
#endif
  if (!(outline = new (std::nothrow) Outline(sys_var_partitions))) return true;

  register_reload_entry();

  return false;
}

void destroy_statement_outline() {
  remove_reload_entry();
  delete outline;
}

longlong add_rule_from_query(THD *thd, const char *database,
                             const char *query) {
  if (check_readonly(thd, true)) return -1;
  return outline->add_rule_from_query(database, query);
}

bool delete_rule(THD *thd, longlong rule_id) {
  if (check_readonly(thd, true)) return true;
  return outline->delete_rule(thd, rule_id);
}

bool enable_rule(THD *thd, longlong rule_id, bool enable) {
  if (check_readonly(thd, true)) return true;
  return outline->enable_rule(thd, rule_id, enable);
}

void foreach_rule(THD *thd, const std::function<void(Rule *)> &accessor) {
  outline->foreach_rule(thd, accessor);
}

void reload_outline_rules() { outline->refresh(nullptr); }

/**
  Entry point to the Statement outline, The server calls this function after
  each parsed query The function extracts the digest of the query. If the digest
  and current db matches an existing outline rule, it is executed. @param
  digest_computed is true indicts the current statement digest has been computed
  by pfs instrument.
*/
void apply_outline_rules(THD* thd, bool digest_computed) {
  // thd->m_digest could be nullptr if digest computing explicitly turned off by
  // setting maximum digest length to zero or dispatch_sql_command() is called
  // by functions other that dispatch_command() e.g. process_iterator()
  if (!sys_var_enabled || !thd->variables.statement_outline_enable_apply ||
      !thd->m_digest || !is_explainable_query(thd->lex->sql_command))
    return;

  uchar digest_buf[DIGEST_HASH_SIZE];

  uchar *digest =
      compute_digest_skip_explain(thd, digest_buf, !digest_computed);
  outline->apply_to_query(thd, digest);
}
}

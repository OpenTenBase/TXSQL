/* Copyright (c) 2021, Tencent and/or its affiliates.

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
#include "sql/statement_outline/proc.h"
#include "sql/protocol.h"
#include "sql/statement_outline/outline.h"

namespace statement_outline {
const char *proc_qname = "DBMS_ADMIN.";

Rules_flush::Rules_flush(PSI_memory_key key) : Proc(key) {
    m_result_type = Result_type::RESULT_OK;
}

im::Proc *Rules_flush::instance() {
  static im::Proc *proc = new Rules_flush(psi_key_memory_statement_outline);
  return proc;
}

Sql_cmd *Rules_flush::evoke_cmd(THD *thd, mem_root_deque<Item *> *list) const {
  return new (thd->mem_root) Sql_cmd_type(thd, list, this);
}

Rules_flush::~Rules_flush() {}

///  Refresh statement outline cache, read rules from from disk table.
bool Sql_cmd_rules_flush::pc_execute(THD *thd) {
  return refresh_rules_table(thd);
}

im::Proc *Rule_add::instance() {
  static im::Proc *proc = new Rule_add(psi_key_memory_statement_outline);

  return proc;
}

Sql_cmd *Rule_add::evoke_cmd(THD *thd, mem_root_deque<Item *> *list) const {
  return new (thd->mem_root) Sql_cmd_type(thd, list, this);
}

/// Add a rule to statement outline cache, write the rule to disk table.
bool Sql_cmd_rule_add::pc_execute(THD *thd) {
  char buff[1024];
  String str(buff, sizeof(buff), system_charset_info);
    /* schema */
  String *res = (*m_items)[0]->val_str(&str);
  LEX_CSTRING db =
      to_lex_cstring(strmake_root(thd->mem_root, res->ptr(), res->length()));
  res = (*m_items)[1]->val_str(&str);
  LEX_CSTRING query =
      to_lex_cstring(strmake_root(thd->mem_root, res->ptr(), res->length()));

  added_rule_id = add_rule_from_query(thd, db.str, query.str);
  return added_rule_id < 0;
}

void Sql_cmd_rule_add::send_result(THD *thd, bool error) {
  Protocol *protocol = thd->get_protocol();
  if (error) {
    assert(thd->is_error());
    return;
  }
  if (m_proc->send_result_metadata(thd)) return;
  protocol->start_row();
  // Column ID
  protocol->store(added_rule_id);
  protocol->end_row();
  my_eof(thd);
}

im::Proc *Rule_delete::instance() {
  static im::Proc *proc = new Rule_delete(psi_key_memory_statement_outline);

  return proc;
}

Sql_cmd *Rule_delete::evoke_cmd(THD *thd, mem_root_deque<Item *> *list) const {
  return new (thd->mem_root) Sql_cmd_type(thd, list, this);
}

/// Delete a rule from cache and remove it from disk table.
bool Sql_cmd_rule_delete::pc_execute(THD *thd) {

  longlong rule_id = (*m_items)[0]->val_int();
  return delete_rule(thd, rule_id);
}

im::Proc *Rule_enable::instance() {
  static im::Proc *proc = new Rule_enable(psi_key_memory_statement_outline);

  return proc;
}

Sql_cmd *Rule_enable::evoke_cmd(THD *thd, mem_root_deque<Item *> *list) const {
  return new (thd->mem_root) Sql_cmd_type(thd, list, this);
}

/// Delete a rule from cache and remove it from disk table.
bool Sql_cmd_rule_enable::pc_execute(THD *thd) {

  longlong rule_id = (*m_items)[0]->val_int();
  longlong enable_it = (*m_items)[1]->val_int();
  return enable_rule(thd, rule_id, enable_it == 1);
}

im::Proc *Rules_show::instance() {
  static im::Proc *proc = new Rules_show(psi_key_memory_statement_outline);

  return proc;
}

Sql_cmd *Rules_show::evoke_cmd(THD *thd, mem_root_deque<Item *> *list) const {
  return new (thd->mem_root) Sql_cmd_type(thd, list, this);
}

/// Output the statement outline rules.
void Sql_cmd_rules_show::send_result(THD *thd, bool error) {
  Protocol *protocol = thd->get_protocol();
  if (error) {
    assert(thd->is_error());
    return;
  }
  if (m_proc->send_result_metadata(thd)) return;
  foreach_rule(thd, [protocol](Rule *rule) {
    protocol->start_row();
    // Column ID
    protocol->store(rule->id());
    // Column SCHEMA
    outline_string schema = rule->schema();
    protocol->store_string(schema.c_str(), schema.size(), system_charset_info);
    // Column DIGEST
    std::string digest = rule->digest_literal();
    protocol->store_string(digest.c_str(), digest.size(), system_charset_info);
    // Column ENABLED
    std::string enabled = rule->is_enabled() ? "YES" : "NO";
    protocol->store_string(enabled.c_str(), enabled.size(), system_charset_info);
    // Column HITS
    protocol->store(rule->hits());
    // Column HINT
    std::string hints = rule->hints();
    protocol->store_string(hints.c_str(), hints.size(), system_charset_info);
    // Column DIGEST_TEXT
    outline_string &digest_text = rule->digest_text();
    protocol->store_string(digest_text.c_str(), digest_text.size(), system_charset_info);
    // Column HAS_APPLY_FAILURE
    protocol->store((longlong)rule->has_apply_failure());
    protocol->end_row();
  });
  my_eof(thd);
}
}

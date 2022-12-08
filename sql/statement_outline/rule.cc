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

#include "sql/statement_outline/rule.h"
#include <assert.h>
#include <stddef.h>
#include <sstream>
#include <string>
#include <vector>
#include "mysqld_error.h"
#include "sql/derror.h"
#include "sql/sql_class.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/statement_outline/parser.h"

using std::string;
using std::vector;

/**
  @file sql/statement_outline/rule.cc

  Implementation of statement outline rule execution.
*/
namespace statement_outline {
static outline_string get_current_query_normalized(THD *thd) {
  String normalized_query = thd->normalized_query();

  return outline_string(normalized_query.ptr(), normalized_query.length());
}

bool DigestAndSchema::load(THD* thd) {
  uchar *digest MY_ATTRIBUTE((unused)) =
      compute_digest_skip_explain(thd, m_digest_buf, true);
  assert(digest == m_digest_buf);
  LEX_CSTRING db = thd->db();
  if (db.str) m_schema = db.str;
  return false;
}

bool DigestAndSchema::load(const std::string &literal,
                               const outline_string &schema) {
  assert(literal.size() >= 64);
  uchar *to = m_digest_buf;
  const char *from = literal.c_str();
  for (uint i = 0; i < sizeof(m_digest_buf); i++) {
    unsigned int v;
    if (sscanf(from, "%02x", &v) == EOF)
      return true;
    *to++ = v;
    from += 2;
  }
  m_schema = schema;
  return false;
}

std::string DigestAndSchema::digest_to_string() const {
  const size_t string_size = DIGEST_HASH_SIZE * 2;
  char digest_str[string_size + sizeof('\0')];
  for (int i = 0; i < DIGEST_HASH_SIZE; ++i)
    snprintf(digest_str + i * 2, string_size, "%02x", m_digest_buf[i]);
  return digest_str;
}

// Initialize class static variables.
const char *OptimizerHints::key_name_of_select_number = "select_number";
const char *OptimizerHints::key_name_of_hint = "hint";

OptimizerHints::OptimizerHints() : m_hints(psi_key_memory_statement_outline) {}

/**
  Load the optimizer hints from table field.
*/
bool OptimizerHints::load(THD *thd, Json_wrapper hints) {
  if (hints.type() != enum_json_type::J_ARRAY) return true;

  for (uint i = 0; i < hints.length(); i++) {
    Json_wrapper select_number =
        hints[i].lookup({key_name_of_select_number, strlen(key_name_of_select_number)});
    Json_wrapper hint = hints[i].lookup({key_name_of_hint, strlen(key_name_of_hint)});
    if (select_number.type() != enum_json_type::J_INT ||
        hint.type() != enum_json_type::J_STRING)
      return true;
    outline_string hint_str =
        outline_string(hint.get_data(), hint.get_data_length());
    LEX_CSTRING str{hint_str.c_str(), hint_str.size()};
    if (!parse_optimizer_hint(thd, str, nullptr)) return true;
    m_hints.emplace(select_number.get_int(), std::move(hint_str));
  }

  return false;
}

bool OptimizerHints::save(Persisted_rule *diskrule) {
  std::unique_ptr<Json_array> hints_array(new (std::nothrow) Json_array);
  if (!hints_array) return true;
  for (auto it = m_hints.begin(); it != m_hints.end(); it++) {
    std::unique_ptr<Json_object> rule_one_select(new (std::nothrow)
                                                     Json_object());
    if (!rule_one_select) return true;
    if (rule_one_select->add_alias(key_name_of_select_number,
                                   new (std::nothrow) Json_int(it->first)) ||
        rule_one_select->add_alias(key_name_of_hint,
                                   new (std::nothrow)
                                       Json_string(it->second.c_str())))
      return true;
    if (hints_array->append_alias(std::move(rule_one_select))) return true;
  }

  diskrule->outline = Json_wrapper(std::move(hints_array));
  return false;
}

void OptimizerHints::add_new_hint(uint select_number, const char *hint) {
  m_hints.emplace(select_number, hint);
}

LEX_CSTRING OptimizerHints::find_hint(uint select_number) {
  auto it = m_hints.find(select_number);
  if (it == m_hints.end()) return {nullptr, 0};
  return {it->second.c_str(), it->second.size()};
}

std::string OptimizerHints::to_string() const {
  std::stringstream ss;
  ss << '[';
  for (auto &it : m_hints) {
    ss << "{\"" << key_name_of_hint << "\": \"" << it.second << "\", ";
    ss << "\"" << key_name_of_select_number << "\": " << it.first << "}, ";
  }
  // Erase last ', '
  string s = ss.str();
  s[s.size() - 2] = ']';
  s.resize(s.size() - 1);
  return s;
}

Rule::Load_status Rule::load(THD *thd, const Persisted_rule *diskrule) {
  m_id = diskrule->id.value();
  m_is_enabled = diskrule->is_enabled;
  outline_string schema_name;
  if (diskrule->schema.has_value())
    schema_name = diskrule->schema.value();
  const string digest_literal = diskrule->digest.value();

  if (digest_literal.size() < 64 ||
      m_digest_and_schema.load(digest_literal, schema_name))
    return INVALID_DIGEST;

  if (diskrule->digest_text.has_value())
    m_digest_text = diskrule->digest_text.value();
  if (m_hints.load(thd, diskrule->outline.value())) return HINTS_PARSE_ERROR;
  return OK;
}

bool Rule::save(Persisted_rule *diskrule) {
  diskrule->digest = m_digest_and_schema.digest_to_string();
  auto schema = m_digest_and_schema.schema();
  if (schema.size() > 0)
    diskrule->schema = schema;
  diskrule->is_enabled = m_is_enabled;
  if (m_digest_text.size() > 0) diskrule->digest_text = m_digest_text;
  if (m_hints.save(diskrule)) return true;

  return false;
}

void Rule::set_digest_and_schema_from(THD *thd) {
  m_digest_text = get_current_query_normalized(thd);
  m_digest_and_schema.load(thd);
}

std::string Rule::digest_literal() const { return m_digest_and_schema.digest_to_string(); }

bool Rule::apply(THD *thd) {
  DBUG_TRACE;
  // This rule is disabled
  if (!m_is_enabled) return false;

  bool warn_fail = thd->variables.statement_outline_apply_verbose;
  m_hits.fetch_add(1, std::memory_order_relaxed);
  std::vector<std::string> hint_apply_warnings;

  // We may allocate some warning messages where can raise an OOM.
  try {
    if (apply_optimizer_hints(
            thd,
            [this](uint select_number) {
              return m_hints.find_hint(select_number);
            },
            warn_fail ? &hint_apply_warnings : nullptr) &&
        !m_has_apply_failure)
      m_has_apply_failure = true;
  } catch (std::bad_alloc &) {
    // Nothing can do here if only allocation of some warning messages cause
    // OOM.
  }

  if (warn_fail && hint_apply_warnings.size() > 0) {
    /*
      Do not clear the condition list when starting execution as it now contains
      not the results of the previous executions, but a non-zero number of
      errors/warnings thrown during parsing!
    */
    thd->lex->keep_diagnostics = DA_KEEP_PARSE_ERROR;
    for (auto &msg : hint_apply_warnings)
      push_warning_printf(thd, Sql_condition::SL_WARNING,
                          ER_STATEMENT_OUTLINE_APPLY_FAILED,
                          ER_THD(thd, ER_STATEMENT_OUTLINE_APPLY_FAILED), m_id,
                          msg.c_str());
  }
  return false;
}
}

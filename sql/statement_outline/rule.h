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
#ifndef STATEMENT_OUTLINE_RULE_INCLUDED
#define STATEMENT_OUTLINE_RULE_INCLUDED

#include "my_config.h"

#include <atomic>
#include <string>
#include <vector>

#include "map_helpers.h"
#include "sql/sql_digest.h"
#include "sql/statement_outline/persisted_rule.h"

namespace statement_outline {
extern PSI_memory_key psi_key_memory_statement_outline;
/**
  Save a digest of statement and schema name that the statement is running in.
*/
class DigestAndSchema {
  /// digest buffer
  uchar m_digest_buf[DIGEST_HASH_SIZE];
  /// Save schema name
  outline_string m_schema;

 public:
  outline_string &schema() { return m_schema; }

  /**
    Load schema and digest, @param literal is a printable digest hex literal
    string.
  */
  bool load(const std::string &literal, const outline_string &schema);
  /**
    Compute digest from THD' digest storge and get schema from THD::db()
  */
  bool load(THD *thd);
  /// Convert digest in m_buf to a printable hex string
  std::string digest_to_string() const;
  /// This class caccesses m_buf to caclulate hash value.
  friend class DigestSchemaHashKey;
};

/**
  Hash map key used for map between DigestAndSchema and Rule object. We only
  need pointers to digest and schema because the content is already saved in
  Rule object.
*/
class DigestSchemaHashKey {
  /// Pointer to a digest buffer to compute digest temporarily.
  const uchar *m_digest_buf;
  /// Pointer to schema raw string buffer
  const char *m_schema;
  /// string length of schema name
  uint m_schema_len;
 public:
  bool operator==(const DigestSchemaHashKey &other) const {
    if (memcmp(m_digest_buf, other.m_digest_buf, DIGEST_HASH_SIZE) != 0) return false;
    if (m_schema_len != other.m_schema_len) return false;
    // Both object are empty.
    if (m_schema_len == 0) return true;
    return memcmp(m_schema, other.m_schema, m_schema_len) == 0;
  }
  /**
    Construct a temporary object search digest and schema in hash map.
  */
  DigestSchemaHashKey(const uchar *pbuf, const char *schema, uint schema_len)
      : m_digest_buf(pbuf), m_schema(schema), m_schema_len(schema_len) {}
  DigestSchemaHashKey(DigestAndSchema *digest)
      : DigestSchemaHashKey(digest->m_digest_buf, digest->m_schema.c_str(),
                            digest->m_schema.size()) {}
  std::size_t hash_value() const {
    std::size_t val = std::hash<std::string>()(
        std::string(pointer_cast<const char *>(m_digest_buf), DIGEST_HASH_SIZE));
    // Don't combine schema if it's empty to let connections without using db
    // work.
    if (m_schema_len > 0)
      val ^= std::hash<std::string>()(std::string(m_schema, m_schema_len));
    return val;
  }
};

/**
  Saves hints of a rule which contains a map of query block number and its
  hints.
 */
class OptimizerHints {
 public:
  /// Key names in JSON.
  static const char *key_name_of_select_number;
  static const char *key_name_of_hint;

  OptimizerHints();
  /// Loads hints from a JSON object to hash map.
  bool load(THD *thd, Json_wrapper hint);
  /// Save the object to Persisted_rule, then save it to disk table.
  bool save(Persisted_rule *diskrule);
  /// Add @param hint to this object with query block number is @param
  /// select_number
  void add_new_hint(uint select_number, const char *hint);
  /// Search a hint in hash map given query block number
  LEX_CSTRING find_hint(uint select_number);
  /// Print hints in JSON text format.
  std::string to_string() const;
 private:
  /// The map of query block number and optimizer hint
  malloc_unordered_map<uint, outline_string> m_hints;

};

/**
  Internal representation of a statement outline rule.  A statement outline rule
  consists of a digest, a schema name and an OptimizeHints object.
*/
class Rule {
 public:
  /// Load result from a rule table row.
  enum Load_status {
    OK,
    INVALID_DIGEST,  /// A invalid digest, e.g. less that 32 bytes.
    HINTS_PARSE_ERROR /// Hint parse error, return by HINT PARSER.
  };
  /// Allocate memory on psi_key_memory_statement_outline
  void *operator new(size_t size, const std::nothrow_t &) noexcept {
    /*
      Call my_malloc() with the MY_WME flag to make sure that it will
      write an error message if the memory could not be allocated.
    */
    return my_malloc(psi_key_memory_statement_outline, size, MYF(MY_WME));
  }
  void operator delete(void *ptr) noexcept { my_free(ptr); }

  /// Used by DigestSchemaHashKey to insert an entry to hash map.
  DigestAndSchema *digest_and_schema() { return &m_digest_and_schema; }
  /// Loads and parses the rule and hints. The digest_text string is deep
  /// copied.
  Load_status load(THD *thd, const Persisted_rule *diskrule);

  void load_id(const Persisted_rule *diskrule) { m_id = diskrule->id.value(); }
  void save_id(Persisted_rule *diskrule) const { diskrule->id = m_id; }

  bool save(Persisted_rule *diskrule);
  /// Apply this rule to current statement.
  bool apply(THD *thd);

  longlong id() const { return m_id; }
  bool is_enabled() const { return m_is_enabled; }
  void set_enabled(bool enable) {
    m_is_enabled.store(enable, std::memory_order_relaxed);
  }

  /// Load digest and schema from THD
  void set_digest_and_schema_from(THD *thd);

  void add_new_hint(uint select_number, const char *hint) {
    m_hints.add_new_hint(select_number, hint);
  }
  std::string digest_literal() const;
  longlong hits() const { return m_hits; }
  bool has_apply_failure() { return m_has_apply_failure; }
  std::string hints() const { return m_hints.to_string(); }
  outline_string &digest_text() { return m_digest_text; }
  outline_string &schema() { return m_digest_and_schema.schema(); }

  private:
  /// The rule id, it is saved as primary key in disk table.
  longlong m_id;
  /// The rule is enabled?
  std::atomic<bool> m_is_enabled;
  /// The digest and schema name obtained from the query
  DigestAndSchema m_digest_and_schema;
  /// The normalized query of the digest.
  outline_string m_digest_text;
  OptimizerHints m_hints;

  /// Matched times of the rule, included apply failure, it will be clear after
  /// a load.
  std::atomic<longlong> m_hits{0};
  /// Indicate there is a apply failure of this rule.
  std::atomic<bool> m_has_apply_failure{false};
};
}

#endif /* STATEMENT_OUTLINE_RULE_INCLUDED */

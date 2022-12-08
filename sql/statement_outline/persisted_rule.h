#ifndef STATEMENT_OUTLINE_PERSISTED_RULE_INCLUDED
#define STATEMENT_OUTLINE_PERSISTED_RULE_INCLUDED
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
#include <memory>
#include <string>
#include "my_config.h"
#include "nullable.h"
#include "sql-common/json_dom.h"
#include "sql/stateless_allocator.h"
#include "sql/statement_outline/rule_table.h"
#include "sql_string.h"

namespace statement_outline {
extern PSI_memory_key psi_key_memory_statement_outline;
/// Allocate memory for std::string with specific spi memory key.
struct Statement_outline_psi_key_alloc {
  void *operator()(size_t s) {
    return my_malloc(psi_key_memory_statement_outline, s,
                     MYF(MY_WME | ME_FATALERROR));
  }
};

template <class T>
using Statement_outline_psi_key_allocator =
    Stateless_allocator<T, Statement_outline_psi_key_alloc>;

template <template <class T> class Allocator>
using default_string =
    std::basic_string<char, std::char_traits<char>, Allocator<char>>;

/**
  A std::basic_string which allocated memory on memory which psi key is
  psi_key_memory_statement_outline, we use this save digest_text so that we
  statistics memory for whole outline.
 */
typedef default_string<Statement_outline_psi_key_allocator> outline_string;

/**
  @file persisted_rule.h

  The facilities for easily manipulating nullable values from a Cursor.
*/

/// A rule as persisted on disk.
class Persisted_rule {
 public:
  Mysql::Nullable<longlong> id;
  /// Schema name.
  Mysql::Nullable<outline_string> schema;
  /// The digest.
  Mysql::Nullable<std::string> digest;

  /// Outline.
  Mysql::Nullable<Json_wrapper> outline;

  /// True if the rule is enabled.
  bool is_enabled;

  /// The digest_text could be null
  Mysql::Nullable<outline_string> digest_text;

  /**
    Constructs a Persisted_rule object that copies all data into the current
    heap.
  */
  Persisted_rule(Cursor *c) {
    copy_and_set(&id, c, c->id_column());
    copy_and_set(&schema, c, c->schema_column());
    copy_and_set(&digest, c, c->digest_column());
    copy_and_set(&outline, c, c->outline_column());
    String val_buf;
    const String *val = c->fetch_string(c->enabled_column(), &val_buf);
    const char *is_enabled_c = val->ptr();
    is_enabled = is_enabled_c != nullptr && is_enabled_c[0] == 'Y';
    copy_and_set(&digest_text, c, c->digest_text_column());
  }
  Persisted_rule() {}

  bool write_to(Cursor *c) {
    c->make_writeable();
    set_if_present(c, c->schema_column(), schema);
    set_if_present(c, c->digest_column(), digest);
    set_if_present(c, c->digest_text_column(), digest_text);
    set_if_present(c, c->enabled_column(),
                   is_enabled ? Mysql::Nullable<std::string>("YES")
                              : Mysql::Nullable<std::string>("NO"));
    c->set(c->outline_column(), &outline.value());
    longlong auto_inc_id;
    int error = c->write(&auto_inc_id);
    id = auto_inc_id;
    return error;
  }

  bool update(Cursor *c) {
    assert(id.has_value());
    c->set(c->id_column(), id.value());
    if (c->search() != 0) return true;
    c->make_updatable();
    // We only update enabled field.
    set_if_present(c, c->enabled_column(),
                   is_enabled ? Mysql::Nullable<std::string>("YES")
                   : Mysql::Nullable<std::string>("NO"));
    return c->update();
  }

  bool delete_from(Cursor *c) {
    assert(id.has_value());
    c->set(c->id_column(), id.value());
    if (c->search() != 0) return true;
    return c->delete_();
  }

 private:
  void copy_and_set(Mysql::Nullable<longlong> *property, Cursor *c,
                    int colno) {
    bool is_null;
    longlong value = c->fetch_integer(colno, is_null);
    if (!is_null) *property = value;
  }

  /**
    Reads from a Cursor and writes to a property of string (type
    Nullable<string> or Nullable<outline_string>) after forcing a copy of the
    string buffer. The function calls a member function in Cursor.
  */
  template <typename T>
  void copy_and_set(Mysql::Nullable<T> *property, Cursor *c,
                    int colno) {
    String val_buf;
    const String *val = c->fetch_string(colno, &val_buf);
    if (val != nullptr) {
      T tmp;
      tmp.assign(val->ptr(), val->length());
      *property = tmp;
    }
  }

  void copy_and_set(Mysql::Nullable<Json_wrapper> *property, Cursor *c,
                    int colno) {
    *property = c->fetch_json(colno);
  }

  /// Writes a string value to the cursor's column if it exists.
  template <typename T>
  void set_if_present(Cursor *cursor, Cursor::column_id column,
                      Mysql::Nullable<T> value) {
    if (column == Cursor::ILLEGAL_COLUMN_ID) return;
    if (!value.has_value()) {
      cursor->set(column, nullptr, 0);
      return;
    }
    const T &s = value.value();
    cursor->set(column, s.c_str(), s.length());
  }
};
}

#endif  // STATEMENT_OUTLINE_PERSISTED_RULE_INCLUDED

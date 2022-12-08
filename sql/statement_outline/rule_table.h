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
#ifndef STATEMENT_OUTLINE_RULE_TABLE_INCLUDED
#define STATEMENT_OUTLINE_RULE_TABLE_INCLUDED
#include <stdlib.h>
#include <string>
#include "my_dbug.h"
#include "sql-common/json_dom.h"

/**
  @file sql/statement_outline/rule_table.h

  The source provides access to the statement outline rules table
*/

class THD;
struct TABLE_LIST;
class Field;

namespace statement_outline {
/**
  Writable cursor that allows reading, updating, deleting and inserting of rows
  in a persistent table.
*/
class Cursor {
 public:
  typedef int column_id;
  /**
    Access mode of current cursor.
  */
  enum class Mode {
    Read, Update, Write, Delete
  };

  static const column_id ILLEGAL_COLUMN_ID = -1;

  /**
    Creates a cursor to an already-opened table.
  */
  Cursor(THD *thd, Mode mode);

  /// Creates a past-the-end cursor.
  Cursor() : m_thd(nullptr), m_table_list(nullptr), m_is_finished(true) {}

  Cursor(const Cursor &) = default;

  column_id id_column() const { return m_id_column; }
  column_id schema_column() const { return m_schema_column; }
  column_id digest_column() const { return m_digest_column; }
  column_id outline_column() const { return m_outline_column; }
  column_id enabled_column() const { return m_enabled_column; }
  column_id digest_text_column() const {
    return m_digest_text_column;
  }

  /**
    True if the table does not contain columns named 'id', 'digest', 'outline',
    'enabled' and 'digest_text'. In this case the cursor is equal to any
    past-the-end Cursor.
  */
  bool table_is_malformed()  const { return m_table_is_malformed; }

  /**
    Fetches the value of the column with the given number as a integer(longlong).
  */
  longlong fetch_integer(int fieldno, bool &is_null);

  /**
    Fetches the value of the column with the given number as a String.
  */
  const String *fetch_string(int fieldno, String *value_buf);

  /**
     Fetches the value of the column with the given number as a JSON.
  */
  Json_wrapper fetch_json(int fieldno);
  /**
    Equality operator. The only cursors that are equal are past-the-end
    cursors.
  */
  bool operator==(const Cursor &other) {
    return (m_is_finished == other.m_is_finished);
  }

  /**
    Inequality operator. All cursors are considered different except
    past-the-end cursors.
  */
  bool operator!=(const Cursor &other) { return !(*this == other); }

  /**
    Advances this Cursor. Read errors are kept, and had_serious_read_error()
    will tell if there was an unexpected error (e.g. not EOF) while reading.
  */
  Cursor &operator++() {
    if (!m_is_finished) read();
    return *this;
  }

  /// Prepares the write buffer for updating the current row.
  void make_updatable();
  /// Prepares the write buffer for inserting the current row.
  void make_writeable();

  /**
    Sets the value of column colno to a string value.

    @param colno The column number.
    @param str The string.
    @param length The string's length.
  */
  void set(int colno, const char *str, size_t length);

  /**
    Sets the value of column colno to a json object.
  */
  void set(int colno, const Json_wrapper *value);
  /**
    Sets the value of column colno to a longlong value.
  */
  void set(int colno, longlong value);

  /// Updates the row in the write buffer to the table at the current row.
  int update();
  /// Search and read the row according to keys, call it before update or
  /// delete;
  int search();
  /// Write a new row to the table return the auto increment id column to @param
  /// auto_inc_id
  int write(longlong *auto_inc_id);
  /// Delete current row of cursor. The function requires the first column is
  /// the primary key.
  int delete_();
  /// True if there was an unexpected error while reading, e.g. other than EOF.
  bool had_serious_error() const;

  /// Closes the table scan if initiated and commits the transaction.
  ~Cursor();

  /**
    A past-the-end Cursor. All past-the-end cursors are considered equal
    when compared with operator ==.
  */
  static Cursor end();
 private:
  int field_index(const char *field_name);

  /// Indexes of all columns.
  int m_id_column;
  int m_schema_column;
  int m_digest_column;
  int m_outline_column;
  int m_enabled_column;
  int m_digest_text_column;

  THD *m_thd;
  TABLE_LIST *m_table_list;

  bool m_is_finished;
  bool m_table_is_malformed;
  int m_last_return_status;

  int read();
};
}  // namespace statement_outline

#endif /* STATEMENT_OUTLINE_RULE_TABLE_INCLUDED */

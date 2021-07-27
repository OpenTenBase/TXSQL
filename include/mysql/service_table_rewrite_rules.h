#ifndef SERVICE_TABLE_REWRITE_RULES_INCLUDED
#define SERVICE_TABLE_REWRITE_RULES_INCLUDED

#include <string>

#include "my_dbug.h"

#ifndef MYSQL_ABI_CHECK
#include <stdlib.h>
#endif

/**
  @file include/mysql/service_rules_table.h

  Plugin service that provides access to the rewrite rules table that is used
  by the Rewriter plugin. No other use intended.
*/

class THD;
struct TABLE_LIST;
class Field;

namespace table_rewrite_rules_service {

/**
  There must be one function of this kind in order for the symbols in the
  server's dynamic library to be visible to plugins.
*/
int dummy_function_to_ensure_we_are_linked_into_the_server();

/**
  Frees a const char pointer allocated in the server's dynamic library using
  new[].
*/
void free_string(const char *str);

/**
  Writable cursor that allows reading and updating of rows in a persistent
  table.
*/
class Cursor {
 public:
  typedef int column_id;

  static const column_id ILLEGAL_COLUMN_ID = -1;

  /**
    Creates a cursor to an already-opened table. The constructor is kept
    explicit because of implicit conversions from void*.
  */
  explicit Cursor(THD *thd);

  /// Creates a past-the-end cursor.
  Cursor() : m_thd(nullptr), m_table_list(nullptr), m_is_finished(true) {}

  Cursor(const Cursor &) = default;

  column_id db_column() const { return m_db_column; }
  column_id table_name_column() const { return m_table_name_column; }
  column_id table_name_new_column() const { return m_table_name_new_column; }

  const char* db() { return fetch_string(m_db_column); }
  const char* table_name() { return fetch_string(m_table_name_column); }
  const char* table_name_new() { return fetch_string(m_table_name_new_column); }

  /**
    True if the table does not contain columns named 'pattern', 'replacement',
    'enabled' and 'message'. In this case the cursor is equal to any
    past-the-end Cursor.
  */
  bool table_is_malformed() { return m_table_is_malformed; }

  /**
    Fetches the value of the column with the given number as a C string.

    This interface is meant for crossing dynamic library boundaries, hence the
    use of C-style const char*. The function casts a column value to a C
    string and returns a copy, allocated in the callee's DL. The pointer
    must be freed using free_string().

    @param fieldno One of PATTERN_COLUMN, REPLACEMENT_COLUMN, ENABLED_COLUMN
    or MESSAGE_COLUMN.
  */
  const char *fetch_string(int fieldno);

  /**
    Advances this Cursor. Read errors are kept, and had_serious_read_error()
    will tell if there was an unexpected error (e.g. not EOF) while reading.
  */
  bool advance() {
    if (!m_is_finished) {
      read();
    }
    return !m_is_finished;
  }

  /// True if there was an unexpected error while reading, e.g. other than EOF.
  bool had_serious_read_error() const;

  /// Closes the table scan if initiated and commits the transaction.
  ~Cursor();

 private:
  int field_index(const char *field_name);

  int m_db_column;
  int m_table_name_column;
  int m_table_name_new_column;

  THD *m_thd;
  TABLE_LIST *m_table_list;

  bool m_is_finished;
  bool m_table_is_malformed;
  int m_last_read_status;

  int read();
};

}  // namespace rules_table_service

#endif  // SERVICE_RULES_TABLE_INCLUDED

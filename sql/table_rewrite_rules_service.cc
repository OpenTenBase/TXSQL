#include <string.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "mysql/service_table_rewrite_rules.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/sql_base.h"
#include "sql/table.h"
#include "sql/transaction.h"
#include "sql_string.h"
#include "thr_lock.h"

class THD;

namespace table_rewrite_rules_service {

int MY_ATTRIBUTE((visibility("default")))
    dummy_function_to_ensure_we_are_linked_into_the_server() {
  return 1;
}

const char *rule_db_name = "query_rewrite";
const char *rule_table_name = "table_rewrite_rules";

int Cursor::read() {
  TABLE *table = m_table_list->table;
  m_last_read_status = table->file->ha_rnd_next(table->record[0]);
  if (m_last_read_status != 0) m_is_finished = true;
  return m_last_read_status;
}

void free_string(const char *str) { delete[] str; }

static void add_column(MY_BITMAP *map, Cursor::column_id column) {
  if (column != Cursor::ILLEGAL_COLUMN_ID) bitmap_set_bit(map, column);
}

Cursor::Cursor(THD *mysql_thd)
    : m_thd(mysql_thd),
      m_table_list(nullptr),
      m_is_finished(true),
      m_table_is_malformed(true) {
  m_table_list = new TABLE_LIST(rule_db_name, strlen(rule_db_name), rule_table_name,
                                strlen(rule_table_name), "alias", TL_READ_DEFAULT);
  if (m_table_list == nullptr) return;  // Error

  m_table_list->updating = false;

  if (open_and_lock_tables(m_thd, m_table_list,
                           MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY))
    return;  // Error

  TABLE *table = m_table_list->table;
  if (table == nullptr) return;  // Error

  m_db_column = field_index("db");
  m_table_name_column = field_index("table_name");
  m_table_name_new_column = field_index("table_name_new");

  if (m_db_column == ILLEGAL_COLUMN_ID ||
      m_table_name_column == ILLEGAL_COLUMN_ID ||
      m_table_name_new_column == ILLEGAL_COLUMN_ID) {
    trans_rollback_stmt(m_thd);
    close_thread_tables(m_thd);

    /*
      We are never going to use this table, we might as well get rid of the
      reference to it.
    */
    delete m_table_list;
    m_table_list = nullptr;
    m_table_is_malformed = true;
    return;  // Error
  } else
    m_table_is_malformed = false;

  add_column(table->read_set, db_column());
  add_column(table->read_set, table_name_column());
  add_column(table->read_set, table_name_new_column());

  if (m_table_list->table->file->ha_rnd_init(true) != 0) return;  // Error

  // No error occurred, set this to false.
  m_is_finished = false;
}

const char *Cursor::fetch_string(int fieldno) {
  Field **fields = m_table_list->table->field;
  Field *field = fields[fieldno];
  if (field->is_null()) return nullptr;
  String value_buf;
  String *value = field->val_str(&value_buf);
  size_t length = value->length();
  char *res = new char[length + 1];
  strncpy(res, value->ptr(), length);
  res[length] = '\0';
  return res;
}

int Cursor::field_index(const char *field_name) {
  TABLE *table = m_table_list->table;
  for (uint i = 0; i < table->s->fields; ++i)
    if (strcmp(table->field[i]->field_name, field_name) == 0) return i;
  return -1;
}

bool Cursor::had_serious_read_error() const {
  return m_last_read_status != 0 && m_last_read_status != HA_ERR_END_OF_FILE;
}

Cursor::~Cursor() {
  if (m_table_list != nullptr && m_table_list->table != nullptr)
    m_table_list->table->file->ha_rnd_end();
  delete m_table_list;
}

}

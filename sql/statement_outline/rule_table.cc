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

/**
  @file sql/statement_outline/rule_table.cc

  Implementation of the interfaces for accessing the statement outline rules
  table.
*/

#include <string.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "sql/sql_class.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/sql_base.h"
#include "sql/statement_outline/rule_table.h"
#include "sql/table.h"
#include "sql/transaction.h"
#include "sql_string.h"
#include "thr_lock.h"

class THD;

namespace statement_outline {
const char *db_name = "mysql";
const char *table_name = "statement_outline_rules";

int Cursor::read() {
  TABLE *table = m_table_list->table;
  if ((m_last_return_status = table->file->ha_rnd_next(table->record[0])) != 0) {
    if (m_last_return_status != HA_ERR_END_OF_FILE)
      table->file->print_error(m_last_return_status, MYF(0));
    m_is_finished = true;
  }

  return m_last_return_status;
}

static void add_column(MY_BITMAP *map, Cursor::column_id column) {
  if (column != Cursor::ILLEGAL_COLUMN_ID) bitmap_set_bit(map, column);
}

Cursor::Cursor(THD *mysql_thd, Mode mode)
    : m_thd(mysql_thd),
      m_table_list(nullptr),
      m_is_finished(true),
      m_table_is_malformed(true),
      m_last_return_status(0) {
  if (!(m_table_list = new (m_thd->mem_root)
            TABLE_LIST(db_name, strlen(db_name), table_name, strlen(table_name),
                       "alias", mode == Mode::Read ? TL_READ : TL_WRITE))) {
    m_last_return_status = HA_ERR_OUT_OF_MEM;
    return;  // Error
  }

  if (mode != Mode::Read) m_table_list->updating = true;

  if (open_and_lock_tables(m_thd, m_table_list,
                           MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY) || !m_table_list->table) {
    // Just assume it is a OOM, let caller know there is a serious error.
    m_last_return_status = HA_ERR_OUT_OF_MEM;
    return;  // Error
  }

  TABLE *table = m_table_list->table;

  m_id_column = field_index("id");
  m_schema_column = field_index("schema_name");
  m_digest_column = field_index("digest");
  m_outline_column = field_index("outline");
  m_enabled_column = field_index("enabled");
  // The "digest_text" column is not required for the Cursor to work.
  m_digest_text_column = field_index("digest_text");

  if (m_id_column == ILLEGAL_COLUMN_ID ||
      m_schema_column == ILLEGAL_COLUMN_ID ||
      m_digest_column == ILLEGAL_COLUMN_ID ||
      m_outline_column == ILLEGAL_COLUMN_ID ||
      m_enabled_column == ILLEGAL_COLUMN_ID ||
      m_digest_text_column == ILLEGAL_COLUMN_ID) {
    trans_rollback_stmt(m_thd);
    close_thread_tables(m_thd);

    /*
      We are never going to use this table, we might as well get rid of the
      reference to it.
    */
    m_table_list = nullptr;

    m_table_is_malformed = true;
    return;  // Error
  } else
    m_table_is_malformed = false;
  // ID column is always used
  add_column(table->read_set, id_column());
  if (mode != Mode::Read) add_column(table->write_set, id_column());
  switch (mode) {
    case Mode::Read:
      add_column(table->read_set, schema_column());
      add_column(table->read_set, digest_column());
      add_column(table->read_set, outline_column());
      add_column(table->read_set, enabled_column());
      add_column(table->read_set, digest_text_column());
      break;
    case Mode::Write:
      add_column(table->write_set, schema_column());
      add_column(table->write_set, digest_column());
      add_column(table->write_set, outline_column());
      add_column(table->write_set, enabled_column());
      add_column(table->write_set, digest_text_column());
      break;
    case Mode::Update:
      add_column(table->write_set, enabled_column());
      break;
    case Mode::Delete:
      break;
    default:
      assert(false);
  }

  if (mode == Mode::Read &&
      (m_last_return_status = m_table_list->table->file->ha_rnd_init(true)) != 0) {
    m_table_list->table->file->print_error(m_last_return_status, MYF(0));
    return;  // Error
  }

  // No error occurred, set this to false.
  m_is_finished = false;
  if (mode == Mode::Read) read();
}

longlong Cursor::fetch_integer(int fieldno, bool &is_null) {
  Field **fields = m_table_list->table->field;
  Field *field = fields[fieldno];
  is_null = field->is_null();
  if (is_null) return 0;
  return field->val_int();
}

const String *Cursor::fetch_string(int fieldno, String *value_buf) {
  Field **fields = m_table_list->table->field;
  Field *field = fields[fieldno];
  if (field->is_null()) return nullptr;
  return field->val_str(value_buf);
}

Json_wrapper Cursor::fetch_json(int fieldno) {
  Field **fields = m_table_list->table->field;
  Field *field = fields[fieldno];
  if (field->is_null()) return Json_wrapper();
  Json_wrapper value;
  down_cast<Field_json *>(field)->val_json(&value);
  return value;
}

int Cursor::field_index(const char *field_name) {
  TABLE *table = m_table_list->table;
  for (uint i = 0; i < table->s->fields; ++i)
    if (strcmp(table->field[i]->field_name, field_name) == 0) return i;
  return -1;
}

void Cursor::make_updatable() {
  TABLE *table = m_table_list->table;
  memcpy(table->record[1], table->record[0], table->s->rec_buff_length);
}
void Cursor::make_writeable() {
  TABLE *table = m_table_list->table;
  restore_record(table, s->default_values);
}

void Cursor::set(int colno, const char *str, size_t length) {
  TABLE *table = m_table_list->table;
  Field *field = table->field[colno];

  const CHARSET_INFO *charset = &my_charset_utf8_unicode_ci;
  if (str == nullptr)
    field->set_null(0);
  else {
    field->store(str, length, charset);
    field->set_notnull(0);
  }
}

void Cursor::set(int colno, longlong value) {
  TABLE *table = m_table_list->table;
  Field *field = table->field[colno];
  field->store(value, false);
  field->set_notnull(0);
}

void Cursor::set(int colno, const Json_wrapper *value) {
  TABLE *table = m_table_list->table;
  Field *field = table->field[colno];
  if (!value)
    field->set_null(0);
  else {
    down_cast<Field_json *>(field)->store_json(value);
    field->set_notnull(0);
  }
}

int Cursor::write(longlong *auto_inc_id) {
  TABLE *table = m_table_list->table;
  table->use_all_columns();
  // Let SE update the auto increment value
  table->next_number_field = table->found_next_number_field;
  if ((m_last_return_status = table->file->ha_write_row(table->record[0])) != 0)
    table->file->print_error(m_last_return_status, MYF(0));
  else
    *auto_inc_id = table->next_number_field->val_int();
  table->file->ha_release_auto_increment();
  table->next_number_field = nullptr;
  return m_last_return_status;
}

int Cursor::search() {
  TABLE *table = m_table_list->table;
  uchar user_key[MAX_KEY_LENGTH];
  KEY *key_info = table->key_info;
  key_copy(user_key, table->record[0], key_info, key_info->key_length);
  if ((m_last_return_status = table->file->ha_index_read_idx_map(
           table->record[0], 0, user_key, HA_WHOLE_KEY, HA_READ_KEY_EXACT)) !=
      0) {
    table->file->print_error(m_last_return_status, MYF(0));
    return m_last_return_status;
  }

  return m_last_return_status;
}

int Cursor::update() {
  TABLE *table = m_table_list->table;
  if ((m_last_return_status =
           table->file->ha_update_row(table->record[1], table->record[0])) != 0)
    table->file->print_error(m_last_return_status, MYF(0));

  return m_last_return_status;
}

int Cursor::delete_() {
  TABLE *table = m_table_list->table;

  store_record(table, record[1]);
  if ((m_last_return_status = table->file->ha_delete_row(table->record[1])) !=
      0)
    table->file->print_error(m_last_return_status, MYF(0));
  return m_last_return_status;
}

bool Cursor::had_serious_error() const {
  return m_last_return_status != 0 && m_last_return_status != HA_ERR_END_OF_FILE;
}

Cursor::~Cursor() {
  // Don't know call ha_rnd_init() or not, so just call ha_index_or_rnd_end();
  if (m_table_list != nullptr && m_table_list->table != nullptr)
    m_table_list->table->file->ha_index_or_rnd_end();
  if (m_thd) {
    trans_commit_stmt(m_thd);
    trans_commit_implicit(m_thd);
    close_mysql_tables(m_thd);
  }
}

Cursor Cursor::end() { return Cursor(); }
}  // namespace statement_outline

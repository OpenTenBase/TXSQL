/*****************************************************************************
Copyright (c) 2020, 2022, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file dict/dict0inst.cc
Instant DDL interface implementation

Created 2020-04-24 by Mayank Prasad */

#include "dict0inst.h"
#include "dict0dd.h"
#include "dict0dict.h"
#include "ha_innodb.h"

static void populate_to_be_instant_columns_low(
    const Alter_inplace_info *ha_alter_info, const TABLE *old_table,
    const TABLE *new_table, Columns &cols_to_add, Columns &cols_to_drop,
    bool &cols_to_modify);

template <typename Table>
bool Instant_ddl_impl<Table>::is_instant_add_drop_possible(
    const Alter_inplace_info *ha_alter_info, const TABLE *table,
    const TABLE *altered_table, const dict_table_t *dict_table,
    bool &is_add_error) {
  Columns cols_to_add;
  Columns cols_to_drop;
  bool cols_to_modify;
  populate_to_be_instant_columns_low(ha_alter_info, table, altered_table,
                                     cols_to_add, cols_to_drop, cols_to_modify);

  auto check_instant_add_possible = [&cols_to_add, &dict_table]() -> bool {
    if (cols_to_add.empty()) {
      return true;
    }

    const dict_index_t *index = dict_table->first_index();

    /* Get maximum permissible size on page */
    size_t page_rec_max;
    size_t page_ptr_max;
    get_permissible_max_size(dict_table, index, page_rec_max, page_ptr_max);

    /* Get the maximum size of a valid rec in current table */
    size_t current_max_size;
    bool res = dict_index_validate_max_rec_size(
        dict_table, index, true, page_rec_max, page_ptr_max, current_max_size);

    if (res) {
      /* Table is already in a state where possible row size can go beyond
      permissible size limit. Don't allow INSTANT ADD */
      return false;
    }

    for (auto field : cols_to_add) {
      if (innobase_is_v_fld(field)) {
        continue;
      }

      /* Get the maximum possible size needed for this field */
      size_t field_max_size = 0;
      {
        ulint col_len;
        ulint mtype;
        ulint prtype;
        get_field_types(nullptr, dict_table, field, col_len, mtype, prtype);

        /* Create a dummy dict_cot_t and dict_field_t just to calculate size */
        dict_col_t dummy_col;
        dummy_col.mtype = (unsigned int)mtype;
        dummy_col.prtype = (unsigned int)prtype;
        dummy_col.len = (unsigned int)col_len;
        ulint mbminlen;
        ulint mbmaxlen;
        dtype_get_mblen(mtype, prtype, &mbminlen, &mbmaxlen);
        dummy_col.set_mbminmaxlen(mbminlen, mbmaxlen);
        dict_field_t dummy_field;
        dummy_field.col = &dummy_col;
        get_field_max_size(dict_table, index, &dummy_field, field_max_size);
      }

      current_max_size += field_max_size;
      if (current_max_size > page_rec_max) {
        /* Don't allow INSTANT ADD */
        return false;
      }
    }
    return true;
  };
  is_add_error = false;
  if (!check_instant_add_possible()) {
    is_add_error = true;
    return false;
  }
  auto check_instant_drop_possible = [&cols_to_drop, &dict_table]() -> bool {
    if (cols_to_drop.empty()) {
      return true;
    }
    for (auto field : cols_to_drop) {
      auto col = dict_table->get_col_by_name(field->field_name);
      if (col && col->is_instant_modified()) {
        return false;
      }
    }
    return true;
  };
  if (!check_instant_drop_possible()) {
    return false;
  }

  return true;
}

template bool Instant_ddl_impl<dd::Table>::is_instant_add_drop_possible(
    const Alter_inplace_info *ha_alter_info, const TABLE *table,
    const TABLE *altered_table, const dict_table_t *dict_table,
    bool &is_add_error);

template bool Instant_ddl_impl<dd::Partition>::is_instant_add_drop_possible(
    const Alter_inplace_info *ha_alter_info, const TABLE *table,
    const TABLE *altered_table, const dict_table_t *dict_table,
    bool &is_add_error);

template <typename Table>
void Instant_ddl_impl<Table>::commit_instant_add_col_low() {
  ut_ad(!m_dict_table->is_temporary());

  ut_d(bool is_add_error = false;);
  ut_ad(is_instant_add_drop_possible(m_ha_alter_info, m_old_table,
                                     m_altered_table, m_dict_table,
                                     is_add_error));

  /* To remember old default values if exist */
  dd_copy_table_columns(m_ha_alter_info, m_new_dd_tab->table(),
                        m_old_dd_tab->table(), m_dict_table);

  /* Then add all new default values */
  dd_add_instant_columns(&m_old_dd_tab->table(), &m_new_dd_tab->table(),
                         m_dict_table, m_cols_to_add);

  /* Keep the metadata for newly added virtual columns if exist */
  dd_update_v_cols(&m_new_dd_tab->table(), m_dict_table->id);
}

template void Instant_ddl_impl<dd::Table>::commit_instant_add_col_low();
template void Instant_ddl_impl<dd::Partition>::commit_instant_add_col_low();

template <>
void Instant_ddl_impl<dd::Table>::commit_instant_add_col() {
  commit_instant_add_col_low();
}

template <>
void Instant_ddl_impl<dd::Partition>::commit_instant_add_col() {
  if (dd_part_is_first(m_new_dd_tab)) {
    commit_instant_add_col_low();
  }
}

template <typename Table>
void Instant_ddl_impl<Table>::commit_instant_drop_col_low() {
  ut_ad(!m_dict_table->is_temporary());

  /* Copy columns metadata */
  dd_copy_table_columns(m_ha_alter_info, m_new_dd_tab->table(),
                        m_old_dd_tab->table(), m_dict_table);

  /* Update metadata of columns to be dropped */
  dd_drop_instant_columns(&m_old_dd_tab->table(), &m_new_dd_tab->table(),
                          m_dict_table, m_cols_to_drop
#ifdef UNIV_DEBUG
                          ,
                          m_cols_to_add, m_ha_alter_info
#endif
  );
}

template void Instant_ddl_impl<dd::Table>::commit_instant_drop_col_low();
template void Instant_ddl_impl<dd::Partition>::commit_instant_drop_col_low();

template <>
void Instant_ddl_impl<dd::Table>::commit_instant_drop_col() {
  commit_instant_drop_col_low();
}

template <>
void Instant_ddl_impl<dd::Partition>::commit_instant_drop_col() {
  if (dd_part_is_first(m_new_dd_tab)) {
    commit_instant_drop_col_low();
  }
}

/* Changes from txsql start. */

template void Instant_ddl_impl<dd::Table>::commit_instant_modify_col_low();
template void Instant_ddl_impl<dd::Partition>::commit_instant_modify_col_low();

template <>
void Instant_ddl_impl<dd::Partition>::commit_instant_modify_column() {
  if (dd_part_is_first(m_new_dd_tab)) {
    commit_instant_modify_col_low();
  }
}

template <>
void Instant_ddl_impl<dd::Table>::commit_instant_modify_column() {
  commit_instant_modify_col_low();
}

/* Changes from txsql end. */

template <typename Table>
void Instant_ddl_impl<Table>::commit_instant_ddl() {
  Instant_Type type =
      static_cast<Instant_Type>(m_ha_alter_info->handler_trivial_ctx);

  switch (type) {
    case Instant_Type::INSTANT_NO_CHANGE:
      dd_commit_inplace_no_change(false);
      break;
    case Instant_Type::INSTANT_COLUMN_RENAME:
      dd_commit_inplace_no_change(false);

      if (!dd_table_is_partitioned(m_new_dd_tab->table()) ||
          dd_part_is_first(reinterpret_cast<dd::Partition *>(m_new_dd_tab))) {
        dd_update_v_cols(&m_new_dd_tab->table(), m_dict_table->id);
      }

      row_mysql_lock_data_dictionary(m_trx, UT_LOCATION_HERE);
      innobase_discard_table(m_thd, m_dict_table);
      row_mysql_unlock_data_dictionary(m_trx);
      break;
    case Instant_Type::INSTANT_VIRTUAL_ONLY:
      if (dd_find_column(&m_old_dd_tab->table(), FTS_DOC_ID_COL_NAME) &&
          !dd_find_column(&m_new_dd_tab->table(), FTS_DOC_ID_COL_NAME)) {
        dd::Column *col = dd_add_hidden_column(
            &m_new_dd_tab->table(), FTS_DOC_ID_COL_NAME, FTS_DOC_ID_LEN,
            dd::enum_column_types::LONGLONG);
        dd_set_hidden_unique_index(m_new_dd_tab->table().add_index(),
                                   FTS_DOC_ID_INDEX_NAME, col);
      }
      dd_commit_inplace_no_change(true);

      if (!dd_table_is_partitioned(m_new_dd_tab->table()) ||
          dd_part_is_first(reinterpret_cast<dd::Partition *>(m_new_dd_tab))) {
        dd_update_v_cols(&m_new_dd_tab->table(), m_dict_table->id);
      }

      row_mysql_lock_data_dictionary(m_trx, UT_LOCATION_HERE);
      innobase_discard_table(m_thd, m_dict_table);
      row_mysql_unlock_data_dictionary(m_trx);
      break;
    case Instant_Type::INSTANT_ADD_DROP_COLUMN:
    case Instant_Type::INSTANT_MODIFY_COLUMN:
      dd_copy_private(*m_new_dd_tab, *m_old_dd_tab);

      /* Fetch the columns which are to be added or dropped */
      populate_to_be_instant_columns();

      ut_ad(!m_cols_to_add.empty() || !m_cols_to_drop.empty() ||
            m_cols_to_modify);

      if (!m_cols_to_drop.empty()) {
        /* INSTANT DROP */
        commit_instant_drop_col();
      }

      if (!m_cols_to_add.empty()) {
        /* INSTANT ADD */
        commit_instant_add_col();
      }

      if (m_cols_to_modify) {
        /* We don't support modify column and add/drop column in one
        statement. */
        ut_a(m_cols_to_add.empty() && m_cols_to_drop.empty());
        commit_instant_modify_column();
      }

      /* Update the current row version in dictionary cache */
      m_dict_table->current_row_version++;

      ut_ad(dd_table_has_instant_cols(m_new_dd_tab->table()));

      row_mysql_lock_data_dictionary(m_trx, UT_LOCATION_HERE);
      innobase_discard_table(m_thd, m_dict_table);
      row_mysql_unlock_data_dictionary(m_trx);
      break;
    case Instant_Type::INSTANT_IMPOSSIBLE:
    default:
      ut_ad(0);
  }

  if (m_autoinc != nullptr) {
    ut_ad(m_altered_table->found_next_number_field != nullptr);
    if (!dd_table_is_partitioned(m_new_dd_tab->table()) ||
        dd_part_is_first(reinterpret_cast<dd::Partition *>(m_new_dd_tab))) {
      dd_set_autoinc(m_new_dd_tab->table().se_private_data(), *m_autoinc);
    }
  }
}

template void Instant_ddl_impl<dd::Table>::commit_instant_ddl();
template void Instant_ddl_impl<dd::Partition>::commit_instant_ddl();

static void populate_to_be_instant_columns_low(
    const Alter_inplace_info *ha_alter_info, const TABLE *old_table,
    const TABLE *altered_table, Columns &cols_to_add, Columns &cols_to_drop,
    bool &cols_to_modify) {
  /* Collect all renamed columns */
  using renamed_fields_t = std::pair<std::string, std::string>;
  std::vector<renamed_fields_t> renamed_fields;
  for (size_t i = 0; i < old_table->s->fields; i++) {
    const char *field_name = old_table->field[i]->field_name;
    std::string new_name;
    if (is_renamed(ha_alter_info, field_name, new_name)) {
      ut_a(!new_name.empty());
      renamed_fields.push_back(std::pair(field_name, new_name));
    }
  }

  for (size_t i = 0; i < old_table->s->fields; i++) {
    Field *old_table_field = old_table->field[i];
    const char *old_field_name = old_table_field->field_name;

    /* Skip virtual column from old table */
    if (innobase_is_v_fld(old_table_field)) {
      continue;
    }

    /* Skip if this column is being renamed */
    auto it = std::find_if(
        renamed_fields.begin(), renamed_fields.end(),
        [&old_field_name](renamed_fields_t &element) {
          return (strcmp(element.first.c_str(), old_field_name) == 0);
        });
    if (it != renamed_fields.end()) {
      continue;
    }

    /* Look for this column in new table */
    bool found = false;
    for (size_t j = 0; j < altered_table->s->fields; j++) {
      Field *new_table_field = altered_table->field[j];

      /* Skip virtual column from altered table */
      if (innobase_is_v_fld(new_table_field)) {
        continue;
      }

      const char *new_field_name = new_table_field->field_name;

      if (strcmp(old_field_name, new_field_name) == 0) {
        /* This column is present in both the tables. Stop iteration. */

        /* Check if this column is in drop list of alter_info. */
        if (is_dropped(ha_alter_info, old_field_name)) {
          /* This column is being dropped */
          cols_to_drop.push_back(old_table_field);

          /* But this column is present in new table as well which is possible
          if a new column with same name is being added or an existing column
          is being renamed to this name. */
          auto it = std::find_if(
              renamed_fields.begin(), renamed_fields.end(),
              [&new_field_name](renamed_fields_t &element) {
                return (strcmp(element.second.c_str(), new_field_name) == 0);
              });
          if (it == renamed_fields.end()) {
            /* Not renamed, so must be being added */
            cols_to_add.push_back(new_table_field);
          }
        } else if (is_modified(old_table_field, new_table_field)) {
          cols_to_modify = true;
        }

        found = true;
        break;
      }
    }

    /* Could not find this column in new table. So it is being dropped. */
    if (!found) {
      cols_to_drop.push_back(old_table_field);
    }
  } /* For */

  for (size_t i = 0; i < altered_table->s->fields; i++) {
    Field *new_table_field = altered_table->field[i];

    /* Skip virtual column from altered table */
    if (innobase_is_v_fld(new_table_field)) {
      continue;
    }

    const char *new_field_name = new_table_field->field_name;
    /* Skip if it is renamed field */
    auto it = std::find_if(
        renamed_fields.begin(), renamed_fields.end(),
        [&new_field_name](renamed_fields_t &element) {
          return (strcmp(element.second.c_str(), new_field_name) == 0);
        });
    if (it != renamed_fields.end()) {
      continue;
    }

    /* Look for this column in old table */
    bool found = false;
    for (size_t j = 0; j < old_table->s->fields; j++) {
      Field *old_table_field = old_table->field[j];

      /* Skip virtual column from old table */
      if (innobase_is_v_fld(old_table_field)) {
        continue;
      }

      const char *old_field_name = old_table_field->field_name;

      /* This column is present in both the tables. Stop iteration. */
      if (strcmp(old_field_name, new_field_name) == 0) {
        /* If the old column is renamed, then this new column is being added */
        auto it = std::find_if(
            renamed_fields.begin(), renamed_fields.end(),
            [&old_field_name](renamed_fields_t &element) {
              return (strcmp(element.first.c_str(), old_field_name) == 0);
            });
        if (it != renamed_fields.end()) {
          cols_to_add.push_back(new_table_field);
        }

        found = true;
        break;
      }
    }

    /* Could not find this column in old table. So it is being added. */
    if (!found) {
      cols_to_add.push_back(new_table_field);
    }
  } /* FOR */

  renamed_fields.clear();
}

template <typename Table>
void Instant_ddl_impl<Table>::populate_to_be_instant_columns() {
  populate_to_be_instant_columns_low(m_ha_alter_info, m_old_table,
                                     m_altered_table, m_cols_to_add,
                                     m_cols_to_drop, m_cols_to_modify);
}

template <typename Table>
void Instant_ddl_impl<Table>::dd_commit_inplace_no_change(bool ignore_fts) {
  if (dd_table_has_instant_drop_cols(m_old_dd_tab->table())) {
    /* Copy dropped columns from old table to new table */
    copy_dropped_columns(&m_old_dd_tab->table(), &m_new_dd_tab->table(),
                         UINT32_UNDEFINED);
  }

  if (!ignore_fts) {
    dd_add_fts_doc_id_index(m_new_dd_tab->table(), m_old_dd_tab->table());
  }

  dd_copy_private(*m_new_dd_tab, *m_old_dd_tab);

  if (!dd_table_is_partitioned(m_new_dd_tab->table()) ||
      dd_part_is_first(reinterpret_cast<dd::Partition *>(m_new_dd_tab))) {
    dd_copy_table(m_ha_alter_info, m_new_dd_tab->table(),
                  m_old_dd_tab->table());
  }
}

/* Changes from txsql start */

template <typename Table>
void Instant_ddl_impl<Table>::commit_instant_modify_col_low() {
  ut_ad(!m_dict_table->is_temporary());

  /* To remember old default values if exist */
  dd_copy_table_columns(m_ha_alter_info, m_new_dd_tab->table(),
                        m_old_dd_tab->table(), m_dict_table);

  if (dd_table_has_instant_drop_cols(m_old_dd_tab->table())) {
    /* Copy metadata of already dropped columns */
    copy_dropped_columns(&m_old_dd_tab->table(), &m_new_dd_tab->table(),
                         m_dict_table->current_row_version);
  }

  /* Set old types and length of new columns. */
  dd_modify_instant_columns(m_old_table, m_altered_table,
                            &m_new_dd_tab->table(), m_dict_table);
}

template <typename Table>
bool Instant_ddl_impl<Table>::is_instant_modify_possible(
    const Alter_inplace_info *ha_alter_info, const TABLE *old_table,
    const TABLE *altered_table, dict_table_t *table) {
  const Create_field *new_field;
  ulint i = 0;
  dict_col_t *col = NULL;
  ut_ad(dict_table_is_comp(table));

  std::map<unsigned int, dict_col_t> new_columns;
  auto save_new_column_info =
      [](const Field *new_field, const dict_col_t *old_col,
         std::map<unsigned int, dict_col_t> &new_columns) {
        /* Get the mtype and prtype of the old field. Keep this same
        with the code in dd_fill_dict_table(), except FTS check */
        ulint prtype = 0;
        ulint col_len = new_field->pack_length();
        ulint nulls_allowed;
        ulint unsigned_type;
        ulint binary_type;
        ulint long_true_varchar;
        ulint charset_no;
        ulint mbminlen;
        ulint mbmaxlen;
        ulint mtype =
            get_innobase_type_from_mysql_type(&unsigned_type, new_field);
        nulls_allowed = new_field->is_nullable() ? 0 : DATA_NOT_NULL;
        binary_type = new_field->binary() ? DATA_BINARY_TYPE : 0;
        charset_no = 0;
        if (dtype_is_string_type(mtype)) {
          charset_no = static_cast<ulint>(new_field->charset()->number);
        }
        /* see dd_add_instant_columns */
        long_true_varchar = 0;
        if (new_field->type() == MYSQL_TYPE_VARCHAR) {
          col_len -= new_field->get_length_bytes();
          if (new_field->get_length_bytes() == 2) {
            long_true_varchar = DATA_LONG_TRUE_VARCHAR;
          }
        }
        prtype = dtype_form_prtype((ulint)new_field->type() | nulls_allowed |
                                       unsigned_type | binary_type |
                                       long_true_varchar,
                                   charset_no);
        dtype_get_mblen(mtype, prtype, &mbminlen, &mbmaxlen);
        dict_col_t new_col = *old_col;
        new_col.mtype = mtype;
        new_col.prtype = prtype;
        new_col.len = col_len;
        new_col.set_mbminmaxlen(mbminlen, mbmaxlen);
        unsigned int col_ind = new_col.ind;
        new_columns.insert({col_ind, new_col});
      };
  List_iterator_fast<Create_field> cf_it(
      ha_alter_info->alter_info->create_list);
  while ((new_field = (cf_it++)) != NULL) {
    const Field *field = new_field->field;
    ulint is_virtual;
    ulint n_vcols = 0;
    ulint old_i;
    i++;
    is_virtual = (innobase_is_v_fld(field)) ? DATA_VIRTUAL : 0;
    if (is_virtual) {
      n_vcols++;
    }
    /* Skip unchanged columns. */
    if (!new_field->change) {
      continue;
    }
    for (old_i = 0; old_table->field[old_i]; old_i++) {
      const Field *n_field = old_table->field[old_i];
      if (field == n_field) {
        break;
      }
    }
    if (is_virtual) {
      return (false);
    }
    field = altered_table->field[i - 1];
    /* check if modified one column more the one time */
    col = table->get_col(i - 1 - n_vcols);
    ut_ad(!col->is_instant_modified());
    ut_ad(!col->is_instant_added());
    ut_ad(!col->is_instant_dropped());
    ut_ad(!table->is_upgraded_instant());
    save_new_column_info(field, col, new_columns);
  }

  auto check_clust_index_size =
      [](std::map<unsigned int, dict_col_t> &new_columns,
         dict_table_t *table) -> bool {
    dict_index_t *old_clust = table->first_index();
    ut_a(old_clust->is_clustered());
    ut_a(old_clust->type != DICT_FTS);
    dict_index_t *new_index =
        dict_mem_index_create(table->name.m_name, old_clust->name, table->space,
                              old_clust->type, old_clust->n_fields);
    ut_a(new_index != nullptr);
    /* Copy the fields of index */
    dict_index_copy(new_index, old_clust, table, 0, old_clust->n_fields);
    new_index->n_fields = old_clust->n_fields;
    new_index->id = old_clust->id;
    new_index->n_uniq = old_clust->n_uniq;
    new_index->cached = old_clust->cached;
    /* Set row_versions to true, so 1 byte will be reserved when calculate max
    record size, see dict_index_validate_max_rec_size for more information. */
    new_index->row_versions = true;
    /* replace old column with new column info. */
    for (ulint i = 0; i < new_index->n_fields; i++) {
      auto field = new_index->get_field(i);
      if (field->col->is_virtual()) {
        continue;
      }
      auto it = new_columns.find(field->col->ind);
      if (it == new_columns.end()) {
        continue;
      }
      auto new_col = &it->second;
      field->col = new_col;
      /* see dict_index_add_col. */
      field->fixed_len = static_cast<unsigned int>(
          new_col->get_fixed_size(dict_table_is_comp(table)));
      if (field->fixed_len > DICT_MAX_FIXED_COL_LEN) {
        field->fixed_len = 0;
      }
      field->prefix_len = 0;
    }
    bool ret = true;
    /* param strict is always true for instant modify. */
    if (dict_index_too_big_for_tree(table, new_index, true)) {
      ret = false;
      sql_print_information(
          "Row size too large, instant modify column is not supported!");
    }
    dict_mem_index_free(new_index);
    return ret;
  };

  if (!check_clust_index_size(new_columns, table)) {
    return false;
  }
  return true;
}

template bool Instant_ddl_impl<dd::Table>::is_instant_modify_possible(
    const Alter_inplace_info *ha_alter_info, const TABLE *old_table,
    const TABLE *altered_table, dict_table_t *table);

template bool Instant_ddl_impl<dd::Partition>::is_instant_modify_possible(
    const Alter_inplace_info *ha_alter_info, const TABLE *old_table,
    const TABLE *altered_table, dict_table_t *table);

/* Changes from txsql end */

/* Copyright (c) 2019, 2020, Oracle and/or its affiliates.

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

#include "sql/iterators/sort_merge_join_iterator.h"

#include <sys/types.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "field_types.h"
#include "my_alloc.h"
#include "my_bit.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "scope_guard.h"
#include "sql/pack_rows.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"
#include "sql/pfs_batch_mode.h"
#include "sql/iterators/row_iterator.h"
#include "sql/sql_class.h"
#include "sql/sql_executor.h"
#include "sql/sql_optimizer.h"
#include "sql/sql_select.h"
#include "sql/table.h"
#include "sql/iterators/hash_join_buffer.h"
#include "sql/iterators/hash_join_iterator.h"

using pack_rows::TableCollection;

#define UNUSED(x) (void)(x)

SortMergeJoinIterator::SortMergeJoinIterator(
    THD *thd,
    unique_ptr_destroy_only<RowIterator> left_input,
    const Prealloced_array<TABLE *, 4> left_input_tables,
    unique_ptr_destroy_only<RowIterator> right_input,
    const Prealloced_array<TABLE *, 4> right_input_tables,
    bool store_rowids,
    table_map tables_to_get_rowid_for, size_t max_memory_available,
    const std::vector<HashJoinCondition> &join_conditions,
    bool allow_spill_to_disk, JoinType join_type, const JOIN *join,
    const Mem_root_array<Item *> &extra_conditions, bool right_input_batch_mode)
    : RowIterator(thd),
      m_left_input(move(left_input)),
      m_right_input(move(right_input)),
      m_right_input_tables(right_input_tables, store_rowids,
                           tables_to_get_rowid_for),
      m_left_input_tables(left_input_tables, store_rowids,
                           tables_to_get_rowid_for),
      m_tables_to_get_rowid_for(tables_to_get_rowid_for),
      m_join_conditions(PSI_NOT_INSTRUMENTED, join_conditions.data(),
                        join_conditions.data() + join_conditions.size()),
      m_right_input_batch_mode(right_input_batch_mode),
      m_allow_spill_to_disk(allow_spill_to_disk),
      m_join_type(join_type) {
  assert(m_left_input != nullptr);
  assert(m_right_input != nullptr);
  // In-memory saving file size
  UNUSED(max_memory_available);

  // If there are multiple extra conditions, merge them into a single AND-ed
  // condition, so evaluation of the item is a bit easier.
  List<Item> items;
  if(extra_conditions.size() > 0) {
    for (Item *cond : extra_conditions) {
      items.push_back(cond);
    }
    m_extra_condition = new Item_cond_and(items);
    m_extra_condition->quick_fix_field();
    m_extra_condition->update_used_tables();
    m_extra_condition->apply_is_true();
  }

  // judge the join order with the item order of join conditions
  if (join_conditions.size() > 0) {
    if (!m_join_conditions[0].left_uses_any_table(
        m_left_input_tables.tables_bitmap())) {
      m_swap_cmp_order = -1;
    }
  }
}


bool SortMergeJoinIterator::InitWritingToEqualRowSavingFile() {
  m_write_to_equal_row_saving = true;
  return m_equal_row_saving_write_file.Init(m_left_input_tables,
                                            m_join_type == JoinType::OUTER);
}

bool SortMergeJoinIterator::InitReadingFromEqualRowSavingFile() {
  m_equal_row_saving_read_file = std::move(m_equal_row_saving_write_file);
  m_equal_row_saving_read_file_current_row = 0;
  left_cache_need_output = true;
  if (InitWritingToEqualRowSavingFile()) {
    return true;
  }
  return m_equal_row_saving_read_file.Rewind();
}

// Mark that blobs should be copied for each table that contains at least one
// geometry column.
static void MarkCopyBlobsIfTableContainsGeometry(
    const pack_rows::TableCollection &table_collection) {
  for (const pack_rows::Table &table : table_collection.tables()) {
    for (const pack_rows::Column &col : table.columns) {
      if (col.field_type == MYSQL_TYPE_GEOMETRY) {
        table.table->copy_blobs = true;
        break;
      }
    }
  }
}

bool SortMergeJoinIterator::InitRightIterator() {
  PrepareForRequestRowId(m_right_input_tables.tables(),
                            m_tables_to_get_rowid_for);
  if (m_right_input->Init()) {
    return true;
  }

  if (m_right_input_batch_mode) {
    m_right_input->StartPSIBatchMode();
  }

  m_right_iterator_has_more_rows = true;
  m_state = State::READING_ROW_FROM_RIGHT_ITERATOR;
  return false;
}

bool SortMergeJoinIterator::Init() {
  PrepareForRequestRowId(m_left_input_tables.tables(),
                            m_tables_to_get_rowid_for);
  if (m_left_input->Init()) {
    assert(thd()->is_error() ||
                thd()->killed);  // my_error should have been called.
    return true;
  }
  // to ensure the left_input always have row to be join
  PFSBatchMode batch_mode(m_left_input.get());
  int res = m_left_input->Read();
  if (res == 1) {
    assert(thd()->is_error() ||
                thd()->killed);  // my_error should have been called.
    return true;
  }

  if (res == -1) {
    m_left_iterator_has_more_rows = false;
    // If the left input was empty, the result of all the joins
    // will also be empty.
    m_state = State::END_OF_ROWS;
    return false;
  }
  RequestRowId(m_left_input_tables.tables(), m_tables_to_get_rowid_for);

  // We always start out by doing everything in memory.
  m_merge_join_type = MergeJoinType::IN_MEMORY;
  m_write_to_equal_row_saving = false;
  m_left_iterator_has_more_rows = true;
  m_right_input->EndPSIBatchModeIfStarted();
  m_row_match_flag = false;

  // Set up the buffer that is used when
  // a) moving a row between the tables' record buffers, and,
  // b) when constructing a join key from join conditions.
  size_t left_row_upper_size = 0;
  if (!m_left_input_tables.has_blob_column()) {
    left_row_upper_size =
        pack_rows::ComputeRowSizeUpperBound(m_left_input_tables);
  }

  if (m_temporary_row_and_join_key_buffer.reserve(left_row_upper_size)) {
    my_error(ER_OUTOFMEMORY, MYF(0), left_row_upper_size);
    return true;  // oom
  }

  if (last_left_row.reserve(left_row_upper_size)) {
    my_error(ER_OUTOFMEMORY, MYF(0), left_row_upper_size);
    return true;  // oom
  }

  size_t right_row_upper_size = 0;
  if (!m_right_input_tables.has_blob_column()) {
    right_row_upper_size =
        pack_rows::ComputeRowSizeUpperBound(m_right_input_tables);
  }

  if (last_right_row.reserve(right_row_upper_size)) {
    my_error(ER_OUTOFMEMORY, MYF(0), right_row_upper_size);
    return true;  // oom
  }
  // If any of the tables contains a geometry column, we must ensure that
  // the geometry data is copied to the row buffer (see
  // Field_geom::store_internal) instead of only setting the pointer to the
  // data. This is needed if the join result spills to disk; when we read a row
  // back from chunk file, row data is stored in a temporary buffer. If not told
  // otherwise, Field_geom::store_internal will only store the pointer to the
  // data, and not the data itself. The data this field points to will then
  // become invalid when the temporary buffer is used for something else.
  MarkCopyBlobsIfTableContainsGeometry(m_left_input_tables);

  return InitRightIterator();
}

bool SortMergeJoinIterator::JoinedRowPassesExtraConditions() const {
  if (m_extra_condition != nullptr) {
    return m_extra_condition->val_int() != 0;
  }
  return true;
}


longlong SortMergeJoinIterator::JoinedRowPassesEqConditions() const {
  for (unsigned int i = 0; i < m_join_conditions.size(); i++) {
    Item_func_eq *eq_condition = m_join_conditions[i].join_condition();
    longlong fal = eq_condition->val_cmp();
    if (fal != 0) {
      return fal;
    }
  }
  return 0;
}

// Read one row from the equal row saving file, and put that row
// into the record buffer of the left input table.
bool SortMergeJoinIterator::ReadRowFromEqualRowSavingFile() {
  if (m_equal_row_saving_read_file.num_rows() == 0) {
    m_state = State::READING_ROW_FROM_LEFT_ITERATOR;
    return false;
  }

  if (m_equal_row_saving_read_file_current_row >=
      m_equal_row_saving_read_file.num_rows()) {
    // We are done reading all the rows from the equal row saving file.

    if (InitReadingFromEqualRowSavingFile() ||
        ReadRowFromEqualRowSavingFile()) {
      return 1;
    }
    // go and load the next row of right input.
    m_write_to_equal_row_saving = false;
    m_state = State::READING_ROW_FROM_RIGHT_ITERATOR;

    return false;
  } else if (m_equal_row_saving_read_file.LoadRowFromChunk(
                 &m_temporary_row_and_join_key_buffer,
                 &m_row_match_flag)) {
    assert(thd()->is_error());  // my_error should have been called.
    return true;
  }

  m_equal_row_saving_read_file_current_row++;

  // A row from the saving file is ready.
  m_state = State::NEED_TO_COMPARE;
  return false;
}

bool SortMergeJoinIterator::OutputOuterJoin() {
  bool saving_row_ret = false;
  if (read_saving_row_for_outer_join) {
    saving_row_ret = ReadRowFromEqualRowSavingFile();
    if (saving_row_ret) {
      return saving_row_ret;
    } else if (m_state == State::READING_ROW_FROM_RIGHT_ITERATOR) {
      read_saving_row_for_outer_join = false;
      return false;
    }
  }

  for (;;) {
    read_saving_row_for_outer_join = false;
    if (!m_row_match_flag){
      read_saving_row_for_outer_join = true;
      if (!reset_right_row) {
        if (pack_rows::StoreFromTableBuffers(m_right_input_tables,
            &last_right_row)) {
          my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR),
          pack_rows::ComputeRowSizeUpperBound(m_right_input_tables));
          return true;
        }
        m_right_input->SetNullRowFlag(true);
        reset_right_row = true;
      }
      return 0;
    }
    saving_row_ret = ReadRowFromEqualRowSavingFile();
    if (saving_row_ret) {
      return saving_row_ret;
    } else if (m_state == State::READING_ROW_FROM_RIGHT_ITERATOR) {
      if (!m_right_iterator_has_more_rows) {
        m_state = State::END_OF_ROWS;
      }
      return false;
    }
  }
  return false;
}

int SortMergeJoinIterator::Read() {
  for (;;) {
    if (thd()->killed) {  // Aborted by user.
      thd()->send_kill_message();
      return 1;
    }

    switch (m_state) {
      case State::NEED_TO_COMPARE: {
        const longlong fal = JoinedRowPassesEqConditions();
        longlong& exchanged_fal = const_cast<longlong&>(fal);
        bool passes_extra_conditions = false;
        if (fal == 0) {
          passes_extra_conditions = JoinedRowPassesExtraConditions();
          if (thd()->is_error() || thd()->killed) {
            // Evaluation of extra conditions raised an error, so abort the join.
            return 1;
          }
        }
        else {
          //exchange item expresion
          exchanged_fal *= m_swap_cmp_order;
        }

        if (exchanged_fal == 0) {
          m_state = State::READING_ROW_FROM_LEFT_ITERATOR;
          if (m_join_type == JoinType::ANTI) {
            if (passes_extra_conditions) {
              continue;
            } else {
              m_state = State::READING_ROW_FROM_RIGHT_ITERATOR;
              continue;
            }
          } else if (m_join_type == JoinType::SEMI) {
            return 0;
          } else {
            if (!m_write_to_equal_row_saving) {
              InitWritingToEqualRowSavingFile();
            }
            if (m_equal_row_saving_write_file.WriteRowToChunk(
                  &m_temporary_row_and_join_key_buffer,
                  passes_extra_conditions || m_row_match_flag)) {
              return true;
            }

            if (left_cache_is_valid || !m_left_iterator_has_more_rows) {
              m_state = State::READING_ROW_FROM_LEFT_CACHE;
            }
            if (passes_extra_conditions) {
              return 0;
            }
          }
        } else if (exchanged_fal < 0) {
          if (left_cache_need_output) {
            left_cache_need_output = false;
            if (m_join_type == JoinType::OUTER) {
              m_state = State::OUTPUT_ROW_FROM_LEFT_CACHE;
              continue;
            }
          } else if (m_join_type == JoinType::OUTER || m_join_type == JoinType::ANTI) {
            if (!reset_right_row &&
                pack_rows::StoreFromTableBuffers(m_right_input_tables,
                &last_right_row)) {
              my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR),
                  pack_rows::ComputeRowSizeUpperBound(m_right_input_tables));
              return true;
            }
            reset_right_row = true;
            m_right_input->SetNullRowFlag(true);
            m_state = State::READING_ROW_FROM_LEFT_ITERATOR;
            return false;
          }

          m_state = State::READING_ROW_FROM_LEFT_ITERATOR;
          continue;
        } else if (exchanged_fal > 0) {
          if (m_write_to_equal_row_saving) {
            if (pack_rows::StoreFromTableBuffers(m_left_input_tables,
                &last_left_row)) {
              my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR),
                  pack_rows::ComputeRowSizeUpperBound(m_left_input_tables));
              return true;
            }
            // cache left flag: left_cache_is_valid
            left_cache_is_valid = true;

            if (InitReadingFromEqualRowSavingFile() ||
                ReadRowFromEqualRowSavingFile()) {
              return 1;
            }
          }

          m_write_to_equal_row_saving = false;
          m_state = State::READING_ROW_FROM_RIGHT_ITERATOR;
        }
        break;
      }
      case State::READING_ROW_FROM_LEFT_ITERATOR: {
        if (reset_right_row) {
          reset_right_row = false;
          pack_rows::LoadIntoTableBuffers(m_right_input_tables,
              pointer_cast<const uchar *>(last_right_row.ptr()));
        }
        if (!m_left_iterator_has_more_rows) {
          m_state = State::END_OF_ROWS;
          return -1;
        }
        m_state = State::NEED_TO_COMPARE;

        if (left_cache_is_valid) {
          left_cache_is_valid = false;
          left_cache_need_output = false;
          // cached row wirte back to table.record[0]
          pack_rows::LoadIntoTableBuffers(m_left_input_tables,
              pointer_cast<const uchar *>(last_left_row.ptr()));
          continue;
        }

        int res = m_left_input->Read();

        if (res == 1) {
          assert(thd()->is_error() || thd()->killed);
          return 1;
        }

        if (res == -1) {
          m_left_iterator_has_more_rows = false;
          if (m_write_to_equal_row_saving) {
            if (InitReadingFromEqualRowSavingFile() ||
                ReadRowFromEqualRowSavingFile()) {
              return 1;
            }
            m_write_to_equal_row_saving = false;
            m_state = State::READING_ROW_FROM_RIGHT_ITERATOR;
            continue;
          }
          m_state = State::END_OF_ROWS;
          return -1;
        }
        RequestRowId(m_left_input_tables.tables(), m_tables_to_get_rowid_for);
        break;
      }
      case State::READING_ROW_FROM_RIGHT_ITERATOR: {
        int result = -1;
        if (!m_right_iterator_has_more_rows) {
          m_state = State::END_OF_ROWS;
          return -1;
        }

        m_state = State::NEED_TO_COMPARE;
        result = m_right_input->Read();

        if (result == 1) {
          assert(thd()->is_error() ||
          thd()->killed);  // my_error should have been called.
          return 1;
        }

        if (result == -1) {
          m_right_iterator_has_more_rows = false;
          if(m_join_type == JoinType::OUTER) {
            m_right_input->SetNullRowFlag(true);
            ouput_tmp_row = true;
            m_state = State::OUTPUT_LEFT_CACHE_AND_ITERATOR;
            continue;
          } else if (m_join_type == JoinType::ANTI) {
            m_right_input->SetNullRowFlag(true);
            m_state = State::OUTPUT_ROW_FROM_LEFT_ITERATOR;
            return 0;
          }
          m_state = State::END_OF_ROWS;
          return -1;
        }

        RequestRowId(m_right_input_tables.tables(), m_tables_to_get_rowid_for);
        continue;
      }
      case State::READING_ROW_FROM_LEFT_CACHE: {
        if (ReadRowFromEqualRowSavingFile()) {
          return 1;
        }
        break;
      }
      case State::OUTPUT_ROW_FROM_LEFT_CACHE: {
        if (OutputOuterJoin()) {
          return 1;
        } else if (read_saving_row_for_outer_join) {
          m_state = State::OUTPUT_ROW_FROM_LEFT_CACHE;
          return 0;
        } else if (m_state == State::END_OF_ROWS) {
          return -1;
        }

        m_state = State::READING_ROW_FROM_LEFT_ITERATOR;

        continue;
      }
      case State::OUTPUT_ROW_FROM_LEFT_ITERATOR: {
        if (m_left_iterator_has_more_rows) {
          int res = m_left_input->Read();

          if (res == 1) {
            assert(thd()->is_error() || thd()->killed);
            return 1;
          }

          if (res == -1) {
            m_state = State::END_OF_ROWS;
            return -1;
          }
          RequestRowId(m_left_input_tables.tables(), m_tables_to_get_rowid_for);
          return 0;
        }
        m_state = State::END_OF_ROWS;
        return -1;
      }
      case State::OUTPUT_LEFT_CACHE_AND_ITERATOR: {
        if (m_write_to_equal_row_saving || left_cache_need_output) {
          if (OutputOuterJoin()) {
            return 1;
          } else if (read_saving_row_for_outer_join) {
            m_state = State::OUTPUT_LEFT_CACHE_AND_ITERATOR;
            return 0;
          } else if (m_state == State::END_OF_ROWS) {
            m_write_to_equal_row_saving = false;
            left_cache_need_output = false;
            ouput_tmp_row = false;
            m_state = State::OUTPUT_LEFT_CACHE_AND_ITERATOR;
          }
        } else if (ouput_tmp_row) {
          ouput_tmp_row = false;
          return 0;
        }

        if (left_cache_is_valid) {
          left_cache_is_valid = false;
          // cached row wirte back to table.record[0]
          pack_rows::LoadIntoTableBuffers(m_left_input_tables,
              pointer_cast<const uchar *>(last_left_row.ptr()));
          return 0;
        }

        if (m_left_iterator_has_more_rows) {
          int res = m_left_input->Read();

          if (res == 1) {
            assert(thd()->is_error() || thd()->killed);
            return 1;
          }

          if (res == -1) {
            m_state = State::END_OF_ROWS;
            return -1;
          }
          RequestRowId(m_left_input_tables.tables(), m_tables_to_get_rowid_for);
          return 0;
        }
        m_state = State::END_OF_ROWS;
        return -1;
      }
      case State::READING_ROW_FROM_LEFT_AFTER_RESET_RIGHT: {
        m_right_input->SetNullRowFlag(false);
        m_state = State::READING_ROW_FROM_LEFT_ITERATOR;
        continue;
      }
      case State::END_OF_ROWS:
        return -1;
    }
  }

  // Unreachable.
  assert(false);
  return 1;
}
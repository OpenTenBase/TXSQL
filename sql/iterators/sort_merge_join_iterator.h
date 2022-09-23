#ifndef SQL_SORT_MERGE_JOIN_ITERATOR_H_
#define SQL_SORT_MERGE_JOIN_ITERATOR_H_

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

#include <stdio.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "my_alloc.h"
#include "my_inttypes.h"
#include "sql/iterators/hash_join_buffer.h"
#include "sql/iterators/hash_join_chunk.h"
#include "sql/item_cmpfunc.h"
#include "sql/mem_root_array.h"
#include "sql/iterators/row_iterator.h"
#include "sql/table.h"
#include "sql_string.h"
#include "sql/join_type.h"

class THD;
class QEP_TAB;

using pack_rows::TableCollection;

class SortMergeJoinIterator final : public RowIterator {
 public:
  /// Construct a SortMergeJoinIterator.
  ///
  /// @param thd
  ///   the thread handle
  /// @param left_input
  ///   the iterator for the left input
  /// @param left_input_tables
  ///   a bitmap of all the tables in the left input.
  /// @param right_input
  ///   the iterator for the right input
  /// @param right_input_tables
  ///   the right input tables. Needed for the same reasons as
  ///   left_input_tables.
  /// @param store_rowids whether we need to make sure row ids are available
  ///   for all tables below us, after Read() has been called. used only if
  ///   we are below a weedout operation.
  /// @param tables_to_get_rowid_for a map of which tables we need to call
  ///   position() for ourselves. tables that are in left_input_tables
  ///   but not in this map, are expected to be handled by some other iterator.
  ///   tables that are in this map but not in left_input_tables will be
  ///   ignored.
  /// @param max_memory_available
  ///   the amount of memory available, in bytes, for this join iterator.
  ///   This can be user-controlled by setting the system variable
  ///   join_buffer_size.
  /// @param join_conditions
  ///   a list of all the join conditions between the two inputs
  /// @param allow_spill_to_disk
  ///   whether the merge join can spill to disk.
  /// @param join_type
  ///   The join type.
  /// @param join
  ///   The join we are a part of.
  /// @param extra_conditions
  ///   A list of extra conditions that the iterator will evaluate after a
  ///   equal compare is done, but before the row is returned. The
  ///   conditions are AND-ed together into a single Item.
  /// @param right_input_batch_mode
  ///   Whether we need to enable batch mode on the right input table.
  ///   Only make sense if it is a single table, and we are not on the
  ///   outer side of any nested loop join.
  SortMergeJoinIterator(THD *thd, unique_ptr_destroy_only<RowIterator> left_input,
                   const Prealloced_array<TABLE *, 4> left_input_tables,
                   unique_ptr_destroy_only<RowIterator> right_input,
                   const Prealloced_array<TABLE *, 4> right_input_tables,
                   bool store_rowids,
                   table_map tables_to_get_rowid_for,
                   size_t max_memory_available,
                   const std::vector<HashJoinCondition> &join_conditions,
                   bool allow_spill_to_disk, JoinType join_type,
                   const JOIN *join,
                   const Mem_root_array<Item *> &extra_conditions,
                   bool right_input_batch_mode);

  bool Init() override;

  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_left_input->SetNullRowFlag(is_null_row);
    m_right_input->SetNullRowFlag(is_null_row);
  }

  void EndPSIBatchModeIfStarted() override {
    m_left_input->EndPSIBatchModeIfStarted();
    m_right_input->EndPSIBatchModeIfStarted();
  }

  void UnlockRow() override {
    // Since both inputs may have been materialized to disk, we cannot unlock
    // them.
  }

 private:
  /// Read a single row from the row saving file into the tables' record
  /// buffers.
  ///
  /// @retval true in case of error
  bool ReadRowFromEqualRowSavingFile();

  /// Output a outer join row from the row saving file.
  ///
  /// @retval true in case of error
  bool OutputOuterJoin();

  /// @retval true if the last joined row passes all of the extra conditions.
  bool JoinedRowPassesExtraConditions() const;

  /// @retval true if the last joined row passes all of the extra conditions.
  longlong JoinedRowPassesEqConditions() const;

  /// Prepare to read the right iterator from the beginning, and enable batch
  /// mode if applicable. The iterator state will remain unchanged.
  ///
  /// @retval true in case of error. my_error has been called.
  bool InitRightIterator();

  /// Mark that equal row saving is enabled, and prepare the equal row saving
  /// file for writing.
  /// @see m_write_to_equal_row_saving
  ///
  /// @retval true in case of error. my_error has been called.
  bool InitWritingToEqualRowSavingFile();

  /// Mark that we should read from the equal row saving file. The equal row
  /// saving file is rewinded to the beginning.
  /// @see m_read_from_equal_row_saving
  ///
  /// @retval true in case of error. my_error has been called.
  bool InitReadingFromEqualRowSavingFile();

  enum class State {
    READING_ROW_FROM_LEFT_AFTER_RESET_RIGHT,
    READING_ROW_FROM_LEFT_ITERATOR,
    READING_ROW_FROM_RIGHT_ITERATOR,
    OUTPUT_LEFT_CACHE_AND_ITERATOR,
    OUTPUT_ROW_FROM_LEFT_ITERATOR,
    READING_ROW_FROM_LEFT_CACHE,
    OUTPUT_ROW_FROM_LEFT_CACHE,
    NEED_TO_COMPARE,
    END_OF_ROWS
  };

  State m_state;

  const unique_ptr_destroy_only<RowIterator> m_left_input;
  const unique_ptr_destroy_only<RowIterator> m_right_input;

  // These structures holds the tables and columns that are needed for the
  // join. Rows/columns that are not needed are filtered out in the constructor.
  // We need to know which tables that belong to each iterator, so that we can
  // compute the join key when needed.
  pack_rows::TableCollection m_right_input_tables;
  pack_rows::TableCollection m_left_input_tables;
  const table_map m_tables_to_get_rowid_for;

  // A list of the join conditions (all of them are equi-join conditions).
  Prealloced_array<HashJoinCondition, 4> m_join_conditions;

  // whether the order of tables(left&right table) is consistent with
  // that of join expression(items of m_join_conditions)
  int m_swap_cmp_order{1};

  // A buffer that is used when moving a row between tables' record buffers
  // and the saving file.
  String m_temporary_row_and_join_key_buffer;

  bool m_right_input_batch_mode{false};

  // Whether we are allowed to spill to disk.
  bool m_allow_spill_to_disk{true};

  // Whether we should write rows from the left input to the
  // equal row saving file.
  bool m_write_to_equal_row_saving{false};

  bool m_left_iterator_has_more_rows{true};
  bool m_right_iterator_has_more_rows{true};

  // What kind of join the iterator should execute.
  const JoinType m_join_type;

  // If not nullptr, an extra condition that the iterator will evaluate after a
  // equal merge join is done, but before the row is returned. This is
  // needed in case we have a semijoin condition that is not an equi-join
  // condition (i.e. 't1.col1 < t2.col1').
  Item *m_extra_condition{nullptr};

  // The equal row saving files where equal joined rows are written to and
  // read from.
  HashJoinChunk m_equal_row_saving_write_file;
  HashJoinChunk m_equal_row_saving_read_file;

  // Which row we currently are reading from in the saving read file.
  // Used to know whether we have reached the end of the file. How many files
  // the row saving read file contains is contained in the HashJoinChunk
  // (see m_equal_row_saving_read_file).
  ha_rows m_equal_row_saving_read_file_current_row{0};

  bool m_row_match_flag{false};

  bool read_saving_row_for_outer_join{false};

  bool left_cache_is_valid{false};

  bool left_cache_need_output{false};

  bool reset_right_row{false};

  bool ouput_tmp_row{false};

  String last_left_row;

  String last_right_row;

  // for in-memory merge join
  enum class StoreRowResult { ROW_STORED, BUFFER_FULL, FATAL_ERROR };

  enum class MergeJoinType {
    IN_MEMORY
  };

  MergeJoinType m_merge_join_type{MergeJoinType::IN_MEMORY};
};

#endif  // SQL_SORT_MERGE_JOIN_ITERATOR_H_
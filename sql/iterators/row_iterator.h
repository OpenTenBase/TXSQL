#ifndef SQL_ITERATORS_ROW_ITERATOR_H_
#define SQL_ITERATORS_ROW_ITERATOR_H_

/* Copyright (c) 2018, 2022, Oracle and/or its affiliates.

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

#include <assert.h>
#include <string>
#include <prealloced_array.h>

class Item;
class JOIN;
class PX_table_descriptor;
class THD;
struct TABLE;

/**
   Profiling data for an iterator, needed by 'EXPLAIN ANALYZE'.
   Note that an iterator may be iterated over multiple times, e.g. if it is
   the inner operand of a neste loop join. This is denoted 'loops'
   below, and the metrics in this class are aggregated values for all loops.
*/
class IteratorProfiler {
 public:
  /** Time (in ms) spent fetching the first row. (Sum for all loops.)*/
  virtual double GetFirstRowMs() const = 0;

  /** Time (in ms) spent fetching the remaining rows. (Sum for all loops.)*/
  virtual double GetLastRowMs() const = 0;

  /** The number of loops (i.e number of iterator->Init() calls.*/
  virtual uint64_t GetNumInitCalls() const = 0;

  /** The number of rows fetched. (Sum for all loops.)*/
  virtual uint64_t GetNumRows() const = 0;
  virtual ~IteratorProfiler() = default;
};

/**
  A context for reading through a single table using a chosen access method:
  index read, scan, etc, use of cache, etc.. It is mostly meant as an interface,
  but also contains some private member functions that are useful for many
  implementations, such as error handling.

  A RowIterator is a simple iterator; you initialize it, and then read one
  record at a time until Read() returns EOF. A RowIterator can read from
  other Iterators if you want to, e.g., SortingIterator, which takes in records
  from another RowIterator and sorts them.

  The abstraction is not completely tight. In particular, it still leaves some
  specifics to TABLE, such as which columns to read (the read_set). This means
  it would probably be hard as-is to e.g. sort a join of two tables.

  Use by:
@code
  unique_ptr<RowIterator> iterator(new ...);
  if (iterator->Init())
    return true;
  while (iterator->Read() == 0) {
    ...
  }
@endcode
 */
class RowIterator {
#if defined(HAVE_PX)
 public:
  enum PhysicalRowIteratorType {
    PHY_NO_TYPE = 1000,
    // Basic access paths (those with no children, at least nominally).
    PHY_TABLE_SCAN,
    PHY_INDEX_SCAN,
    PHY_REF,
    PHY_REF_OR_NULL,
    PHY_EQ_REF,
    PHY_PUSHED_JOIN_REF,
    PHY_FULL_TEXT_SEARCH,
    PHY_CONST_TABLE,
    PHY_MRR,
    PHY_FOLLOW_TAIL,
    PHY_INDEX_RANGE_SCAN,
    PHY_INDEX_REVERSE_RANGE_SCAN,
    PHY_INDEX_MERGE,
    PHY_ROWID_INTERSECTION,
    PHY_ROWID_UNION,
    PHY_INDEX_SKIP_SCAN,
    PHY_GROUP_INDEX_SKIP_SCAN,
    PHY_DYNAMIC_INDEX_RANGE_SCAN,
    PHY_TABLE_SAMPLE,

    // Basic access paths that don't correspond to a specific table.
    PHY_TABLE_VALUE_CONSTRUCTOR,
    PHY_FAKE_SINGLE_ROW,
    PHY_ZERO_ROWS,
    PHY_ZERO_ROWS_AGGREGATED,
    PHY_MATERIALIZED_TABLE_FUNCTION,
    PHY_UNQUALIFIED_COUNT,

    // Joins.
    PHY_NESTED_LOOP_JOIN,
    PHY_NESTED_LOOP_SEMIJOIN_WITH_DUPLICATE_REMOVAL,
    PHY_BKA_JOIN,
    PHY_HASH_JOIN,
    PHY_SORT_MERGE_JOIN,

    // Composite access paths.
    PHY_FILTER,
    PHY_SORT,
    PHY_AGGREGATE,
    PHY_PRECOMPUTED_AGGREGATE,
    PHY_TEMPTABLE_AGGREGATE,
    PHY_LIMIT_OFFSET,
    PHY_STREAM,
    PHY_MATERIALIZE,
    PHY_MATERIALIZE_INFORMATION_SCHEMA_TABLE,
    PHY_APPEND,
    PHY_WINDOWING,
    PHY_WEEDOUT,
    PHY_REMOVE_DUPLICATES,
    PHY_REMOVE_DUPLICATES_ON_INDEX,
    PHY_ALTERNATIVE,
    PHY_CACHE_INVALIDATOR,

    // Exchange
    PHY_PX_RECEIVE,
    PHY_PX_SEND
  }physical_row_iterator_type;
#endif /* defined(HAVE_PX) */

 public:
  // NOTE: Iterators should typically be instantiated using NewIterator,
  // in sql/iterators/timing_iterator.h.
  explicit RowIterator(THD *thd) : m_thd(thd)
#if defined(HAVE_PX)
  , m_children(4)
#endif /* defined(HAVE_PX) */
  {}
  virtual ~RowIterator() = default;

  RowIterator(const RowIterator &) = delete;

  // Default move ctor used by IndexRangeScanIterator.
  RowIterator(RowIterator &&) = default;

  /**
    Initialize or reinitialize the iterator. You must always call Init()
    before trying a Read() (but Init() does not imply Read()).

    You can call Init() multiple times; subsequent calls will rewind the
    iterator (or reposition it, depending on whether the iterator takes in
    e.g. a TABLE_REF) and allow you to read the records anew.
   */
  virtual bool Init() = 0;

  /**
    Read a single row. The row data is not actually returned from the function;
    it is put in the table's (or tables', in case of a join) record buffer, ie.,
    table->records[0].

    @retval
      0   OK
    @retval
      -1   End of records
    @retval
      1   Error
   */
  virtual int Read() = 0;

  /**
    Mark the current row buffer as containing a NULL row or not, so that if you
    read from it and the flag is true, you'll get only NULLs no matter what is
    actually in the buffer (typically some old leftover row). This is used
    for outer joins, when an iterator hasn't produced any rows and we need to
    produce a NULL-complemented row. Init() or Read() won't necessarily
    reset this flag, so if you ever set is to true, make sure to also set it
    to false when needed.

    Note that this can be called without Init() having been called first.
    For example, NestedLoopIterator can hit EOF immediately on the outer
    iterator, which means the inner iterator doesn't get an Init() call,
    but will still forward SetNullRowFlag to both inner and outer iterators.

    TODO: We shouldn't need this. See the comments on AggregateIterator for
    a bit more discussion on abstracting out a row interface.
   */
  virtual void SetNullRowFlag(bool is_null_row) = 0;

  // In certain queries, such as SELECT FOR UPDATE, UPDATE or DELETE queries,
  // reading rows will automatically take locks on them. (This means that the
  // set of locks taken will depend on whether e.g. the optimizer chose a table
  // scan or used an index, due to InnoDB's row locking scheme with “gap locks”
  // for B-trees instead of full predicate locks.)
  //
  // However, under some transaction isolation levels (READ COMMITTED or
  // less strict), it is possible to release such locks if and only if the row
  // failed a WHERE predicate, as only the returned rows are protected,
  // not _which_ rows are returned. Thus, if Read() returned a row that you did
  // not actually use, you should call UnlockRow() afterwards, which allows the
  // storage engine to release the row lock in such situations.
  //
  // TableRowIterator has a default implementation of this; other iterators
  // should usually either forward the call to their source iterator (if any)
  // or just ignore it. The right behavior depends on the iterator.
  virtual void UnlockRow() = 0;

  /** Get profiling data for this iterator (for 'EXPLAIN ANALYZE').*/
  virtual const IteratorProfiler *GetProfiler() const {
    /**
        Valid for TimingIterator, MaterializeIterator and
        TemptableAggregateIterator only.
    */
    assert(false);
    return nullptr;
  }

  /** @see TimingIterator .*/
  virtual void SetOverrideProfiler([
      [maybe_unused]] const IteratorProfiler *profiler) {
    // Valid for TimingIterator only.
    assert(false);
  }

  /**
    Start performance schema batch mode, if supported (otherwise ignored).

    PFS batch mode is a mitigation to reduce the overhead of performance schema,
    typically applied at the innermost table of the entire join. If you start
    it before scanning the table and then end it afterwards, the entire set
    of handler calls will be timed only once, as a group, and the costs will
    be distributed evenly out. This reduces timer overhead.

    If you start PFS batch mode, you must also take care to end it at the
    end of the scan, one way or the other. Do note that this is true even
    if the query ends abruptly (LIMIT is reached, or an error happens).
    The easiest workaround for this is to simply call EndPSIBatchModeIfStarted()
    on the root iterator at the end of the scan. See the PFSBatchMode class for
    a useful helper.

    The rules for starting batch and ending mode are:

      1. If you are an iterator with exactly one child (FilterIterator etc.),
         forward any StartPSIBatchMode() calls to it.
      2. If you drive an iterator (read rows from it using a for loop
         or similar), use PFSBatchMode as described above.
      3. If you have multiple children, ignore the call and do your own
         handling of batch mode as appropriate. For materialization,
         #2 would typically apply. For joins, it depends on the join type
         (e.g., NestedLoopIterator applies batch mode only when scanning
         the innermost table).

    The upshot of this is that when scanning a single table, batch mode
    will typically be activated for that table (since we call
    StartPSIBatchMode() on the root iterator, and it will trickle all the way
    down to the table iterator), but for a join, the call will be ignored
    and the join iterator will activate batch mode by itself as needed.
   */
  virtual void StartPSIBatchMode() {}

  /**
    Ends performance schema batch mode, if started. It's always safe to
    call this.

    Iterators that have children (composite iterators) must forward the
    EndPSIBatchModeIfStarted() call to every iterator they could conceivably
    have called StartPSIBatchMode() on. This ensures that after such a call
    to on the root iterator, all handlers are out of batch mode.
   */
  virtual void EndPSIBatchModeIfStarted() {}

  /**
    If this iterator is wrapping a different iterator (e.g. TimingIterator<T>)
    and you need to down_cast<> to a specific iterator type, this allows getting
    at the wrapped iterator.
   */
  virtual RowIterator *real_iterator() { return this; }
  virtual const RowIterator *real_iterator() const { return this; }

#if defined(HAVE_PX)
  // Type of the row iterator.
  virtual PhysicalRowIteratorType type() { return PHY_NO_TYPE; }
  virtual RowIterator *child(size_t i) { return m_children.at(i); }
  virtual void add_child(RowIterator *itr) { m_children.push_back(itr); }
  virtual void adjust_children() {}
  virtual std::string str() { return ""; }
  virtual bool prepare_for_parallel_query() { return false; }
#endif /* defined(HAVE_PX) */

 protected:
  THD *thd() const { return m_thd; }

 private:
  THD *const m_thd;

#if defined(HAVE_PX)
 public:
  Prealloced_array<RowIterator*, 4> m_children;
#endif /* defined(HAVE_PX) */
};

class TableRowIterator : public RowIterator {
 public:
  TableRowIterator(THD *thd, TABLE *table) : RowIterator(thd), m_table(table) {}

  void UnlockRow() override;
  void SetNullRowFlag(bool is_null_row) override;
  void StartPSIBatchMode() override;
  void EndPSIBatchModeIfStarted() override;

#if defined(HAVE_PX)
  virtual void set_parallel_scan() { m_parallel_scan = true; }
  virtual std::shared_ptr<PX_table_descriptor> get_table_descriptor() const { return nullptr; }
  virtual void set_parallel_workers(uint dop);
  virtual int px_scan_init();
#endif /* defined(HAVE_PX) */

 protected:
  int HandleError(int error);
  void PrintError(int error);
  TABLE *table() const { return m_table; }

 protected:
#if defined(HAVE_PX)
  bool m_parallel_scan{false};
#endif /* defined(HAVE_PX) */

 private:
  TABLE *const m_table;

  friend class AlternativeIterator;
};

#endif  // SQL_ITERATORS_ROW_ITERATOR_H_

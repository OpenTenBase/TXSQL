#include "row0px.h"
#include "row0row.h"
#include "row0vers.h"
#include "lock0lock.h"
#include "trx0trx.h"
#include "record_buffer.h"
#include <time.h>  // for performance

#include <sql_class.h> // THD
#include <current_thd.h> // current_thd
#include "my_dbug.h" // DBUG_PRINT

/*
  To avoid reverse dependency on server code (px.h), PX_PRINT_INFO_INNOBASE and
  PX_PRINT_DEBUG_INNOBASE are simulated. A separate header for storage engine
  might be introduced in the future.
 */
#define PX_PRINT_INFO_INNOBASE(fmt, ...) \
    DBUG_PRINT("pinfo", \
               ("px:%ld:%u:%u: " fmt, 0L, 0U, current_thd->thread_id(), \
                ##__VA_ARGS__))
#define PX_PRINT_DEBUG_INNOBASE(fmt, ...) \
    DBUG_PRINT("pdebug", \
               ("px:%ld:%u:%u: " fmt, 0L, 0U, current_thd->thread_id(), \
                ##__VA_ARGS__))

extern ICP_RESULT row_search_idx_cond_check(byte *mysql_rec,
                                            row_prebuilt_t *prebuilt,
                                            const rec_t *rec,
                                            const ulint *offsets);
extern bool sel_restore_position_for_mysql(bool *same_user_rec,
                                           ulint latch_mode,
                                           btr_pcur_t *pcur,
                                           bool moves_up,
                                          mtr_t *mtr);
extern Record_buffer *row_sel_get_record_buffer(const row_prebuilt_t *prebuilt);
extern void row_sel_dequeue_cached_row_for_mysql(byte *buf, row_prebuilt_t *prebuilt);
extern byte *row_sel_fetch_last_buf(row_prebuilt_t *prebuilt);
extern void row_sel_enqueue_cache_row_for_mysql(byte *mysql_rec,row_prebuilt_t *prebuilt);

extern void row_sel_store_row_id_to_prebuilt(
  row_prebuilt_t *prebuilt,  /*!< in/out: prebuilt */
  const rec_t *index_rec,    /*!< in: record */
  const dict_index_t *index, /*!< in: index of the record */
  const ulint *offsets);

PX_reader::PX_reader(size_t max_threads)
    : m_max_threads(max_threads), m_ctxs() {
  mutex_create(LATCH_ID_PARALLEL_READ, &m_mutex);
  m_event = os_event_create();             
}

PX_reader::~PX_reader() {
  mutex_destroy(&m_mutex);
  os_event_destroy(m_event);
}

dberr_t PX_reader::add_scan(trx_t *trx, const PX_Config &config) {
  // clang-format off

  auto scan_ctx = std::shared_ptr<PX_Scan_ctx>(
      ut::new_withkey<PX_Scan_ctx>(UT_NEW_THIS_FILE_PSI_KEY, this, m_scan_ctx_id, trx, config),
      [](PX_Scan_ctx *scan_ctx) { ut::delete_(scan_ctx); });

  // clang-format on
  if (scan_ctx.get() == nullptr) {
    ib::error(ER_IB_ERR_PX_OOM) << "Out of memory";
    return (DB_OUT_OF_MEMORY);
  }

  m_scan_ctxs.push_back(scan_ctx);
  ++m_scan_ctx_id;
  scan_ctx->index_s_lock();

  PX_Scan_ctx::Ranges ranges{};
  dberr_t err{DB_SUCCESS};
  /* Split at the root node (level == 0). */
  err = scan_ctx->partition(config.m_scan_range, ranges, 0);

  if (ranges.empty() || err != DB_SUCCESS) {
    /* Table is empty. */
    scan_ctx->index_s_unlock();
    return (err);
  }

  err = scan_ctx->create_contexts(ranges);

  if (err != DB_SUCCESS) {
    scan_ctx->index_s_unlock();
    return (err);
  }

  scan_ctx->index_s_unlock();

  return (err);
}

size_t PX_reader::calulate_split_point() {
  size_t split_point{};
  auto depth = m_scan_ctxs.front()->m_depth;
  PX_PRINT_INFO_INNOBASE("B+Tree depth is: %lu", depth);
  PX_PRINT_INFO_INNOBASE("ctx size is: %lu", m_ctxs.size());
  PX_PRINT_INFO_INNOBASE("DOP is: %lu", max_threads());

  if (m_ctxs.size() > max_threads()) {
    split_point = (m_ctxs.size() / max_threads()) * max_threads();
  }

  return split_point;
}

/**
  Do the second split in the second level of B+Tree.
  In Parallel_reader, the second partition is finished by workers. 
  Dynamic sencond partition by workers can't keep the interesting order
  of index.
*/
dberr_t PX_reader::split(ulong avg_partitions) {
  dberr_t err = DB_SUCCESS;
  size_t i = 0;
  size_t ctx_size = m_ctxs.size();
  size_t current_split_level = ++split_level;
  auto depth = m_scan_ctxs.front()->m_depth;

  /*
    In parallel query split policy, if the count of partitions already
    more than dop * avg_partitions, don't do split for next level. In 
    other words, PX_reader will split until the number of partitions
    > avg_partitions * dop or the exists partitions has only one page.
  */
  if (current_split_level >= depth || (ctx_size >= (max_threads() * avg_partitions))) {
    return err;
  }

  size_t split_point = calulate_split_point();
  PX_PRINT_INFO_INNOBASE("split_point is: %lu", split_point);
  PX_PRINT_INFO_INNOBASE("current split level is: %lu", current_split_level);

  clock_t start,end;
  start = clock();
  for (; i < ctx_size; ++i) {
    auto ctx = dequeue();

    if (i >= split_point) {
      err = ctx->split(current_split_level);
      
      if (err != DB_SUCCESS) {
        return err;
      }
    } else {
      enqueue(ctx);
    }
  }

  end = clock();
  PX_PRINT_INFO_INNOBASE("split time is: %g", double(end-start)/CLOCKS_PER_SEC);
  size_t m_n_tasks = m_ctxs.size();
  PX_PRINT_INFO_INNOBASE("total tasks is: %lu", m_n_tasks);

  
  if ((m_ctxs.size() < (max_threads() * avg_partitions))) {
    return split(avg_partitions);
  }

  return err;
}

void PX_reader::enqueue(std::shared_ptr<PX_Ctx> ctx) {
  if (!m_reverse_scan) {
    m_ctxs.push_back(ctx);
  } else {
    m_ctxs.push_front(ctx);
  }
}

std::shared_ptr<PX_Ctx> PX_reader::dequeue() {
  mutex_enter(&m_mutex);

  if (m_ctxs.empty()) {
    mutex_exit(&m_mutex);
    return (nullptr);
  }

  auto ctx = m_ctxs.front();
  m_ctxs.pop_front();

  mutex_exit(&m_mutex);
  return (ctx);
}

/**
  Dispatch task to parallel execution worker thd.

  @param[in,out] prebuilt attach a PX_Ctx to prebuilt

  @retval	errno Failure
  @retval	DB_SUCCESS Success 
*/
dberr_t PX_reader::task_dispatch(std::shared_ptr<PX_Ctx> &task) {
  dberr_t err{DB_SUCCESS};
  PX_PRINT_DEBUG_INNOBASE("==========>>>> THREAD[%lu] LEFT [%lu] task.",
   my_thread_self(), m_ctxs.size());

  /*
    With the contract that all tasks are prepared before any worker starting
    to consume, getting no more task indicates a worker has done. There is
    no need to wait for more. See also ha_innobase::px_coordinator_init().

    Note that the contract is completely different from that in Parallel_reader
    which allows odd tasks to be divided and refilled to the queue after some
    worker starts to consume.
  */
  auto ctx = dequeue();
  task = ctx;

  if (ctx == nullptr) {
    err = DB_END_OF_INDEX;
  }
  
  return err;
}

PX_Scan_ctx::PX_Scan_ctx(PX_reader *reader, size_t id, trx_t *trx,
  const PX_Config &config) : m_id(id), m_config(config), m_trx(trx), m_reader(reader) {}

PX_Scan_ctx::~PX_Scan_ctx() {}

void PX_Scan_ctx::index_s_lock() {
  if (m_s_locks.fetch_add(1, std::memory_order_acquire) == 0) {
    auto index = m_config.m_index;
    /* The latch can be unlocked by a thread that didn't originally lock it. */
    rw_lock_s_lock_gen(dict_index_get_lock(index), true, UT_LOCATION_HERE);
  }
}

void PX_Scan_ctx::index_s_unlock() {
  if (m_s_locks.fetch_sub(1, std::memory_order_acquire) == 1) {
    auto index = m_config.m_index;
    /* The latch can be unlocked by a thread that didn't originally lock it. */
    rw_lock_s_unlock_gen(dict_index_get_lock(index), true);
  }
}

buf_block_t *PX_Scan_ctx::block_get_s_latched(
  const page_id_t &page_id, mtr_t *mtr, size_t line) const {
  /* We never scan undo tablespaces. */
  ut_a(!fsp_is_undo_tablespace(page_id.space()));

  auto block =
      buf_page_get_gen(page_id, m_config.m_page_size, RW_S_LATCH, nullptr,
                       Page_fetch::SCAN, {__FILE__, line}, mtr);

  buf_block_dbg_add_level(block, SYNC_TREE_NODE);

  return (block);
}

void PX_Scan_ctx::copy_row(const rec_t *rec, Iter *iter) const {
  iter->m_offsets = rec_get_offsets(rec, m_config.m_index, nullptr, ULINT_UNDEFINED,
                                    UT_LOCATION_HERE, &iter->m_heap);

  /* Copy the row from the page to the scan iterator. The copy should use
  memory from the iterator heap because the scan iterator owns the copy. */
  auto rec_len = rec_offs_size(iter->m_offsets);
  auto copy_rec = static_cast<rec_t *>(mem_heap_alloc(iter->m_heap, rec_len));
  memcpy(copy_rec, rec, rec_len);
  iter->m_rec = copy_rec;

  auto tuple = row_rec_to_index_entry_low(iter->m_rec, m_config.m_index,
                                          iter->m_offsets, iter->m_heap);
  ut_ad(dtuple_validate(tuple));

  /* We have copied the entire record but we only need to compare the
  key columns when we check for boundary conditions. */
  const auto n_compare = dict_index_get_n_unique_in_tree(m_config.m_index);
  dtuple_set_n_fields_cmp(tuple, n_compare);
  iter->m_tuple = tuple;
}

std::shared_ptr<PX_Scan_ctx::Iter>
PX_Scan_ctx::create_persistent_cursor(
    const page_cur_t &page_cursor, mtr_t *mtr) const {
  ut_ad(index_s_own());

  std::shared_ptr<Iter> iter = std::make_shared<Iter>();
  iter->m_heap = mem_heap_create(sizeof(btr_pcur_t) + (srv_page_size / 16),
                                 UT_LOCATION_HERE);

  auto rec = page_cursor.rec;
  const bool is_infimum = page_rec_is_infimum(rec);
  if (is_infimum) {
    rec = page_rec_get_next(rec);
  }

  if (page_rec_is_supremum(rec)) {
    /* Empty page, only root page can be empty. */
    ut_a(!is_infimum ||
         page_cursor.block->page.id.page_no() == m_config.m_index->page);
    return (iter);
  }

  void *ptr = mem_heap_alloc(iter->m_heap, sizeof(btr_pcur_t));
  ::new (ptr) btr_pcur_t();
  iter->m_pcur = reinterpret_cast<btr_pcur_t *>(ptr);
  iter->m_pcur->init(m_config.m_read_level);

  /* Make a copy of the rec. */
  copy_row(rec, iter.get());
  iter->m_pcur->open_on_user_rec(page_cursor, PAGE_CUR_GE,
                                 BTR_ALREADY_S_LATCHED | BTR_SEARCH_LEAF);

  ut_ad(btr_page_get_level(buf_block_get_frame(iter->m_pcur->get_block())) ==
        m_config.m_read_level);

  iter->m_pcur->store_position(mtr);
  iter->m_pcur->set_fetch_type(Page_fetch::SCAN);

  return (iter);
}

page_no_t PX_Scan_ctx::search(const buf_block_t *block,
                              const dtuple_t *key) const {
  ut_ad(index_s_own());

  page_cur_t page_cursor;
  const auto index = m_config.m_index;

  if (key != nullptr) {
    page_cur_search(block, index, key, PAGE_CUR_LE, &page_cursor);
  } else {
    page_cur_set_before_first(block, &page_cursor);
  }

  if (page_rec_is_infimum(page_cur_get_rec(&page_cursor))) {
    page_cur_move_to_next(&page_cursor);
  }

  const auto rec = page_cur_get_rec(&page_cursor);
  mem_heap_t *heap = nullptr;
  ulint offsets_[REC_OFFS_NORMAL_SIZE];
  auto offsets = offsets_;
  rec_offs_init(offsets_);
  offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, UT_LOCATION_HERE, &heap);

  auto page_no = btr_node_ptr_get_child_page_no(rec, offsets);

  if (heap != nullptr) {
    mem_heap_free(heap);
  }

  return (page_no);
}

page_cur_t PX_Scan_ctx::start_range(
    page_no_t page_no, mtr_t *mtr, const dtuple_t *key,
    Savepoints &savepoints) const {
  ut_ad(index_s_own());

  auto index = m_config.m_index;
  page_id_t page_id(index->space, page_no);
  ulint height{};

  /* Follow the left most pointer down on each page. */
  for (;;) {
    auto savepoint = mtr->get_savepoint();
    auto block = block_get_s_latched(page_id, mtr, __LINE__);
    height = btr_page_get_level(buf_block_get_frame(block));
    savepoints.push_back({savepoint, block});

    if (height != 0 && height != m_config.m_read_level) {
      page_id.set_page_no(search(block, key));
      continue;
    }

    page_cur_t page_cursor;
    if (key != nullptr) {
      page_cur_search(block, index, key, PAGE_CUR_GE, &page_cursor);
    } else {
      page_cur_set_before_first(block, &page_cursor);
    }

    if (page_rec_is_infimum(page_cur_get_rec(&page_cursor))) {
      page_cur_move_to_next(&page_cursor);
    }

    return (page_cursor);
  }

  ut_error;
  return (page_cur_t{});
}

void PX_Scan_ctx::create_range(Ranges &ranges,
                               page_cur_t &leaf_page_cursor,
                               mtr_t *mtr) const {
  leaf_page_cursor.index = m_config.m_index;
  auto iter = create_persistent_cursor(leaf_page_cursor, mtr);

  /* Setup the previous range (next) to point to the current range. */
  if (!ranges.empty()) {
    ut_a(ranges.back().second->m_heap == nullptr);
    ranges.back().second = iter;
  }

  ranges.push_back(Range(iter, std::make_shared<Iter>()));
}

dberr_t PX_Scan_ctx::create_ranges(const PX_Scan_range &scan_range,
                                   page_no_t page_no,
                                   size_t depth,
                                   const size_t split_level,
                                   Ranges &ranges, mtr_t *mtr) {
  ut_ad(index_s_own());
  ut_a(max_threads() > 0);
  ut_a(page_no != FIL_NULL);

  /* Do a breadth first traversal of the B+Tree using recursion. We want to
  set up the scan ranges in one pass. This guarantees that the tree structure
  cannot change while we are creating the scan sub-ranges.

  Once we create the persistent cursor (Range) for a sub-tree we can release
  the latches on all blocks traversed for that sub-tree. */

  const auto index = m_config.m_index;
  page_id_t page_id(index->space, page_no);
  Savepoint savepoint({mtr->get_savepoint(), nullptr});
  auto block = block_get_s_latched(page_id, mtr, __LINE__);

  /* read_level requested should be less than the tree height. */
  ut_ad(m_config.m_read_level <
        btr_page_get_level(buf_block_get_frame(block)) + 1);

  savepoint.second = block;
  ulint offsets_[REC_OFFS_NORMAL_SIZE];
  auto offsets = offsets_;
  rec_offs_init(offsets_);
  page_cur_t page_cursor;
  page_cursor.index = index;
  auto start = scan_range.m_start;

  if (start != nullptr) {
    page_cur_search(block, index, start, PAGE_CUR_LE, &page_cursor);

    if (page_cur_is_after_last(&page_cursor)) {
      return (DB_SUCCESS);
    } else if (page_cur_is_before_first((&page_cursor))) {
      page_cur_move_to_next(&page_cursor);
    }
  } else {
    page_cur_set_before_first(block, &page_cursor);
    /* Skip the infimum record. */
    page_cur_move_to_next(&page_cursor);
  }

  page_cur_t start_page_cursor = page_cursor;
  mem_heap_t *heap{};
  const auto at_leaf = page_is_leaf(buf_block_get_frame(block));
  const auto at_level = btr_page_get_level(buf_block_get_frame(block));
  Savepoints savepoints{};

  while (!page_cur_is_after_last(&page_cursor)) {
    const auto rec = page_cur_get_rec(&page_cursor);

    ut_a(at_leaf || rec_get_node_ptr_flag(rec) ||
         !dict_table_is_comp(index->table));

    if (heap == nullptr) {
      heap = mem_heap_create(srv_page_size / 4, UT_LOCATION_HERE);
    }

    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, UT_LOCATION_HERE, &heap);
    const auto end = scan_range.m_end;

    if (end != nullptr && end->compare(rec, index, offsets) < 0) {
      break;
    }

    page_cur_t level_page_cursor;

    /* Split the tree one level below the root if read_level requested is below
    the root level. */
    if (at_level > m_config.m_read_level) {
      auto page_no = btr_node_ptr_get_child_page_no(rec, offsets);

      if (depth < split_level) {
        /* Need to create a range starting at a lower level in the tree. */
        create_ranges(scan_range, page_no, depth + 1, split_level, ranges, mtr);
        page_cur_move_to_next(&page_cursor);
        continue;
      }

      /* Find the range start in the leaf node. */
      level_page_cursor = start_range(page_no, mtr, start, savepoints);
    } else {
      /* In case of root node being the leaf node or in case we've been asked to
      read the root node (via read_level) place the cursor on the root node and
      proceed. */
      if (start != nullptr) {
        page_cur_search(block, index, start, PAGE_CUR_GE, &page_cursor);
        ut_a(!page_rec_is_infimum(page_cur_get_rec(&page_cursor)));
      } else {
        page_cur_set_before_first(block, &page_cursor);
        /* Skip the infimum record. */
        page_cur_move_to_next(&page_cursor);
        ut_a(!page_cur_is_after_last(&page_cursor));
      }

      /* Since we are already at the requested level use the current page
       * cursor. */
      memcpy(&level_page_cursor, &page_cursor, sizeof(level_page_cursor));
    }

    if (!page_rec_is_supremum(page_cur_get_rec(&level_page_cursor))) {
      create_range(ranges, level_page_cursor, mtr);
    }

    /* We've created the persistent cursor, safe to release S latches on
    the blocks that are in this range (sub-tree). */
    for (auto &savepoint : savepoints) {
      mtr->release_block_at_savepoint(savepoint.first, savepoint.second);
    }

    if (m_depth == 0 && depth == 0) {
      m_depth = savepoints.size();
    }

    savepoints.clear();

    if (at_level == m_config.m_read_level) {
      break;
    }

    start = nullptr;
    page_cur_move_to_next(&page_cursor);
  }

  /* Support split_by_row for test. */
#ifndef DBUG_OFF
  if (m_reader->m_row_split && ranges.size() == 1 && depth == split_level && at_leaf) {
    ranges.clear();
    while (!page_cur_is_after_last(&start_page_cursor)) {
      auto rec = page_cur_get_rec(&start_page_cursor);
      offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, UT_LOCATION_HERE, &heap);
      if (scan_range.m_end != nullptr && scan_range.m_end->compare(rec, index, offsets) < 0) {
        break;
      }

      create_range(ranges, start_page_cursor, mtr);
      page_cur_move_to_next(&start_page_cursor);
    }
  }
#endif
  savepoints.push_back(savepoint);

  for (auto &savepoint : savepoints) {
    mtr->release_block_at_savepoint(savepoint.first, savepoint.second);
  }

  if (heap != nullptr) {
    mem_heap_free(heap);
  }

  return (DB_SUCCESS);
}

dberr_t PX_Scan_ctx::partition(const PX_Scan_range &scan_range,
                               PX_Scan_ctx::Ranges &ranges,
                               size_t split_level) {
  ut_ad(index_s_own());

  mtr_t mtr;
  mtr.start();
  mtr.set_log_mode(MTR_LOG_NO_REDO);

  dberr_t err{DB_SUCCESS};
  err = create_ranges(scan_range, m_config.m_index->page, 0, split_level,
                      ranges, &mtr);

  /*
    In forward scan, for a PX_Ctx, contains a range [start, end).
    The start cursor must exits because the scan is from start to
    end, in other word, move from start from end. The end can be 
    set to null which indicates the range of [start, +inf).
    In create_ranges, make a Range just set the start, the end is
    set when create the next range, so the last Range's end is null.
    For this reason, set the end of last range if the end of scan_range
    exists.
  */
  if (!m_config.m_reverse_scan && err == DB_SUCCESS &&
      scan_range.m_end != nullptr && !ranges.empty()) {
    auto &iter = ranges.back().second;

    ut_a(iter->m_heap == nullptr);

    iter->m_heap = mem_heap_create(sizeof(btr_pcur_t) + (srv_page_size / 16), UT_LOCATION_HERE);
    iter->m_tuple = dtuple_copy(scan_range.m_end, iter->m_heap);

    /* Do a deep copy. */
    for (size_t i = 0; i < dtuple_get_n_fields(iter->m_tuple); ++i) {
      dfield_dup(&iter->m_tuple->fields[i], iter->m_heap);
    }
  }

  /*
    In reverse scan, for a PX_Ctx, contains a range (start, end].
    The end tuple must exists because the scan is backward, in
    other word, move from end to start.
  */
  if (m_config.m_reverse_scan && err == DB_SUCCESS && !ranges.empty()) {
    /* Restore position before use the pcur. */
    bool same_user_rec = false;
    sel_restore_position_for_mysql(&same_user_rec, BTR_SEARCH_LEAF, m_config.m_last_pcur, false, &mtr);

    auto &iter = ranges.back().second;
    auto page_cursor = m_config.m_last_pcur->get_page_cur();
    page_cursor->index = m_config.m_index;
    iter = create_persistent_cursor(*page_cursor, &mtr);

    /*
      Set the left tuple of first range to null if the 
      start of scan range is nullptr, because the start
      of a range must be a invalid tuple for the range.
    */
    if (scan_range.m_start == nullptr) {
      auto &first_iter = ranges.front().first;
      first_iter = std::make_shared<Iter>();
    }
  }

  mtr.commit();

  return (err);
}

dberr_t PX_Scan_ctx::create_context(const Range &range, bool split) {
  auto ctx_id = m_reader->m_ctx_id.fetch_add(1, std::memory_order_relaxed);
  // clang-format off
  auto ctx = std::shared_ptr<PX_Ctx>(
      ut::new_withkey<PX_Ctx>(UT_NEW_THIS_FILE_PSI_KEY, ctx_id, this, range),
      [](PX_Ctx *ctx) { ut::delete_(ctx); });

  // clang-format on
  dberr_t err{DB_SUCCESS};

  if (ctx.get() == nullptr) {
    m_reader->m_ctx_id.fetch_sub(1, std::memory_order_relaxed);
    return (DB_OUT_OF_MEMORY);
  } else {
    ctx->m_split = split;
    m_reader->enqueue(ctx);
  }

  return (err);
}

dberr_t PX_Scan_ctx::create_contexts(const Ranges &ranges) {
  for (auto range : ranges) {
    auto err = create_context(range, false);

    if (err != DB_SUCCESS) {
      return (err);
    }
  }

  return (DB_SUCCESS);
}

dberr_t PX_Scan_ctx::find_visible_record(byte *buf, const rec_t *&rec,
                                         const rec_t *&clust_rec, ulint *&offsets,
                                         ulint *&clust_offsets, mem_heap_t *&heap,
                                         mtr_t *mtr, row_prebuilt_t *prebuilt,
                                         bool &mtr_has_extra_clust_latch) {
  dberr_t err = DB_SUCCESS;
  que_thr_t *thr = nullptr;
  trx_t *trx = prebuilt->trx;
  prebuilt->px_requires_clust_rec = false;
  Row_sel_get_clust_rec_for_mysql row_sel_get_clust_rec_for_mysql;

  /* The readview must has been assigned or in the readonly mode. */
  // ut_ad(!m_trx || m_trx->read_view == nullptr || MVCC::is_view_active(m_trx->read_view));

  /*
    Check visibility with mvcc when innodb_read_only = true and get the visibility
    record.
    1) When innodb_read_only = true, there is no need to check the visibility, but
    should find the cluster record if needed.
    2) When innodb_read_only = false, for cluster index record, only check the visibility and
    find the visible version of record.
    For secondary index record, check the visibility and get the cluster
    record if needed.
  */
  if (trx->isolation_level == TRX_ISO_READ_UNCOMMITTED) {
    /* Do nothing: we let a non-locking SELECT read the
      latest version of the record */

  } else if (m_config.m_index->is_clustered()) {
    /*
      Fetch a previous version of the row if the current
      one is not visible in the snapshot; if we have a very
      high force recovery level set, we try to avoid crashes
      by skipping this lookup
    */
    ReadView *view = (ReadView *)trx_get_read_view(m_trx, m_config.m_index);
    if (!lock_clust_rec_cons_read_sees(rec, m_config.m_index, offsets, view)) {
      rec_t *old_vers = nullptr;
      err = row_vers_build_for_consistent_read(rec, mtr, m_config.m_index, &offsets,
                                               view, &heap, heap, &old_vers,
                                               nullptr, nullptr);
      if (err != DB_SUCCESS) {
        return err;
      }

      if (old_vers == nullptr) {
        /* The row did not exist yet in the read view */
        return DB_NOT_FOUND;
      }

      rec = old_vers;
    }
  } else {
    /*
      We are looking into a non-clustered index,
      and to get the right version of the record we
      have to look also into the clustered index: this
      is necessary, because we can only get the undo
      information via the clustered index record.
    */
    if (!srv_read_only_mode &&
        !lock_sec_rec_cons_read_sees(rec, m_config.m_index, trx->read_view)) {
      /*
        We should look at the clustered index. However, as this is
        a non-locking read, we can skip the clustered index lookup
        if the condition does not match the secondary index entry.
      */
      switch (row_search_idx_cond_check(buf, prebuilt, rec, offsets)) {
        case ICP_NO_MATCH:
          return DB_NOT_FOUND;
        case ICP_OUT_OF_RANGE:
          return DB_RECORD_NOT_FOUND;
        case ICP_MATCH:
          goto require_clust;
      }
    }
  }

  /*
    NOTE that at this point rec can be an old version of a clustered
    index record built for a consistent read. We cannot assume after this
    point that rec is on a buffer pool page. Functions like 
  */
  if (rec_get_deleted_flag(rec, m_config.m_is_compact)) {
    /* The record is delete-marked: we can skip it */
    return DB_NOT_FOUND;
  }

  /* Check if the record matches the index condition. */
  switch (row_search_idx_cond_check(buf, prebuilt, rec, offsets)) {
    case ICP_NO_MATCH:
      return DB_NOT_FOUND;
    case ICP_OUT_OF_RANGE:
      return DB_RECORD_NOT_FOUND;
    case ICP_MATCH:
      break;
  }

  /*
    Get the clustered index record if needed, if we did not do the
    search using the clustered index.
  */
  if (prebuilt->need_to_access_clustered && !m_config.m_index->is_clustered()) {
  require_clust:
    ut_ad(!m_config.m_index->is_clustered());
    ut_ad(rec_offs_validate(rec, m_config.m_index, offsets));
    /* It was a non-clustered index and we must fetch also the
    clustered index record */
    mtr_has_extra_clust_latch = true;
    prebuilt->px_requires_clust_rec = true;

    thr = que_fork_get_first_thr(prebuilt->sel_graph);
    /*
      The following call returns 'offsets' associated with
      'clust_rec'. Note that 'clust_rec' can be an old version
      built for a consistent read.
    */
    err = row_sel_get_clust_rec_for_mysql(prebuilt, m_config.m_index, rec,
                                          thr, &clust_rec, &clust_offsets,
                                          &heap, nullptr, mtr, nullptr);

    if (err == DB_SUCCESS && clust_rec == nullptr) {
      /* The record did not exist in the read view */
      ut_ad(prebuilt->select_lock_type == LOCK_NONE);
      return DB_NOT_FOUND;
    } else if (err == DB_SUCCESS || err == DB_SUCCESS_LOCKED_REC) {
      ut_a(clust_rec != nullptr);
      err = DB_SUCCESS;
    } else if (err == DB_SKIP_LOCKED) {
      return DB_NOT_FOUND;
    } else {
      return err;
    }

    if (rec_get_deleted_flag(clust_rec, m_config.m_is_compact)) {
      /* The record is delete marked: we can skip it */
      return DB_NOT_FOUND;
    }
  }

  return err;
}

PX_Scan_ctx::Iter::~Iter() {
  if (m_heap == nullptr) {
    return;
  }

  if (m_pcur != nullptr) {
    m_pcur->free_rec_buf();
    /* Created with placement new on the heap. */
    call_destructor(m_pcur);
  }

  mem_heap_free(m_heap);
  m_heap = nullptr;
}

PX_Ctx::~PX_Ctx() {}

dberr_t PX_Ctx::split(size_t split_level) {
  ut_ad(m_range.first->m_tuple == nullptr ||
        dtuple_validate(m_range.first->m_tuple));
  ut_ad(m_range.second->m_tuple == nullptr ||
        dtuple_validate(m_range.second->m_tuple));

  /* Setup the sub-range. */
  PX_Scan_range scan_range(m_range.first->m_tuple, m_range.second->m_tuple);

  /* S lock so that the tree structure doesn't change while we are
  figuring out the sub-trees to scan. */
  m_scan_ctx->index_s_lock();

  PX_Scan_ctx::Ranges ranges{};
  m_scan_ctx->partition(scan_range, ranges, split_level);

  if (!ranges.empty()) {
    ranges.back().second = m_range.second;
  }

  dberr_t err{DB_SUCCESS};

  /* Create the partitioned scan execution contexts. */
  for (auto &range : ranges) {
    err = m_scan_ctx->create_context(range, false);

    if (err != DB_SUCCESS) {
      break;
    }
  }

  if (err != DB_SUCCESS) {
    m_scan_ctx->set_error_state(err);
  }

  m_scan_ctx->index_s_unlock();

  return err;
}

/**
  Searches for rows in the database using a pair of persistent cursors provided
  by current ctx. The cursors defines a valid range for rows. Found record is
  saved in buf, and out-of-range gets DB_END_OF_PX_CTX.

  There are two persistent curator in a PX_Ctx, we scan from the first tuple to
  second cursor in forward scan and scan from the second cursor to first cursor
  in reverse mode.

  @param[in,out] buf  the record buffer to store a a row
  @param[in,out] prebuilt the worker scan strcuture

  @retval	errno Failure
  @retval	0 Success 
*/
dberr_t PX_Ctx::row_search_px(uchar *buf, row_prebuilt_t *prebuilt) {
  int ret{0};
  mtr_t mtr;
  auto index = m_scan_ctx->m_config.m_index;
  auto *clust_index = index->table->first_index();
  dberr_t err{DB_SUCCESS};
  dberr_t err1{DB_SUCCESS};
  const rec_t *clust_rec = nullptr;
  const rec_t *rec = nullptr;
  const rec_t *result_rec = nullptr;
  ulint *offsets = offsets_;
  byte *next_buf = nullptr;
  bool mtr_has_extra_clust_latch = false;
  bool moves_up = false;
  auto trx = prebuilt->trx;
  bool same_user_rec = false;
  bool need_to_process = false;
  moves_up = !m_scan_ctx->m_config.m_reverse_scan;
  auto &from = moves_up ? m_range.first : m_range.second;
  const auto &end_tuple = moves_up ?
      m_range.second->m_tuple : m_range.first->m_tuple;
  btr_pcur_t *pcur = from->m_pcur;

  /*-------------------------------------------------------------*/
  /* PHASE 1: Try to pop the row from the record buffer */
  const auto record_buffer = row_sel_get_record_buffer(prebuilt);

  if (prebuilt->px_first_read) {
    trx->op_info = "starting index read";

    prebuilt->n_rows_fetched = 0;
    prebuilt->n_fetch_cached = 0;
    prebuilt->fetch_cache_first = 0;
    if (record_buffer != nullptr) {
      record_buffer->reset();
    }

    if (prebuilt->sel_graph == nullptr) {
      /* Build a dummy select query graph */
      row_prebuild_sel_graph(prebuilt);
    }

    prebuilt->px_first_read = false;
  } else {
    trx->op_info = "fetching rows";

    if (UNIV_LIKELY(prebuilt->n_fetch_cached > 0)) {
      row_sel_dequeue_cached_row_for_mysql(buf, prebuilt);
      prebuilt->n_rows_fetched++;
      err = DB_SUCCESS;
      goto func_exit;
    } else if (prebuilt->m_end_range == true) {
      prebuilt->m_end_range = false; 
      err = DB_RECORD_NOT_FOUND;
      goto func_exit;
    }

    /*
      The prefetch cache is exhausted, so fetch_cache_first
      should point to the beginning of the cache.
    */
    ut_ad(prebuilt->fetch_cache_first == 0);

    if (record_buffer != nullptr && record_buffer->is_out_of_range()) {
      /*
        The previous returned row was popped from
        the fetch cache, but the end of the range was
        reached while filling the cache, so there are
        no more rows to put into the cache.
      */
      err = DB_RECORD_NOT_FOUND;
      goto func_exit;
    }

    prebuilt->n_rows_fetched++;

    if (prebuilt->n_rows_fetched > 1000000000) {
      /* Prevent wrap-over */
      prebuilt->n_rows_fetched = 500000000;
    }
  }

  /*-------------------------------------------------------------*/
  /* PHASE 2: Start a mini-transcation for a scan. */
  mtr.start();
  mtr.set_log_mode(MTR_LOG_NO_REDO);

  /*-------------------------------------------------------------*/
  /* PHASE 3: Open or restore index cursor position */
  need_to_process = sel_restore_position_for_mysql(
      &same_user_rec, BTR_SEARCH_LEAF, pcur, moves_up, &mtr);

  if (UNIV_UNLIKELY(need_to_process)) {
    /*
      there is no need to process because 
      prebuilt->row_read_type == ROW_READ_DID_SEMI_CONSISTENT
      is not supported in parallel execution now!
    */
  }
  
  /* 
    In the start scan of a PX_Ctx, because the pcur has opened
    in the first valid record, so a restore position, we enter
    rec_loop directly. Otherwise, goto next_rec.
  */
  if (start_read) {
    rec_offs_init(offsets_);
    m_heap = mem_heap_create(srv_page_size / 4, UT_LOCATION_HERE);
    start_read = false;
  } else {
    goto next_rec;
  }

rec_loop:
  prebuilt->lob_undo_reset();

  if (trx_is_interrupted(trx)) {
    pcur->store_position(&mtr);
    err = DB_INTERRUPTED;
    goto normal_return;
  }

  /*-------------------------------------------------------------*/
  /* PHASE 4: Look for matching records in a loop */

  rec = pcur->get_rec();

  if (page_rec_is_infimum(rec)) {
    /*
      The infimum record on a page cannot be in the result set,
      and neither can a record lock be placed on it: we skip such
      a record.
    */
    goto next_rec;
  }

  if (page_rec_is_supremum(rec)) {
    /*
      A page supremum record cannot be in the result set: skip
      it now that we have placed a possible lock on it
    */
    goto next_rec;
  }

  /* Calculate the 'offsets' associated with 'rec' */

  ut_ad(fil_page_index_page_check(pcur->get_page()));
  ut_ad(btr_page_get_index_id(pcur->get_page()) == index->id);

  offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, UT_LOCATION_HERE, &m_heap);

  /*
    There is no need to do the pushdown end range check in
    row_search_idx_cond_check. All the record fulfill the
    range condition until the end_tuple.
  */
  if (rec != nullptr && end_tuple != nullptr) {
    ret = end_tuple->compare(rec, index, offsets);

    if ((moves_up && ret <= 0) || (!moves_up && ret >= 0)) {
      err = DB_END_OF_PX_CTX;
      goto px_ctx_cond_failed;
    }
  }

  /*
    This is a non-locking consistent read: if necessary, fetch
    a previous version of the record
  */
  err1 = m_scan_ctx->find_visible_record(buf, rec, clust_rec, offsets,
                                         offsets, m_heap, &mtr, prebuilt, mtr_has_extra_clust_latch);
  /*
    When err1 == DB_SUCCESS, indicates we have get the visible record.
    when err1 == DB_NOT_FOUND, indicates we need skip the record.
    Otherwise, there is a lock wait or error occurs.
  */
  if (err1 == DB_NOT_FOUND) {
    err = DB_NOT_FOUND;
    goto next_rec;
  } else if (err1 != DB_SUCCESS) {
    goto lock_wait_or_error;
  }

  /* Convert record to mysql format. */
  if (prebuilt->px_requires_clust_rec) {
    result_rec = clust_rec;

    if (prebuilt->idx_cond) {
      /*
        Convert the record to MySQL format. We were
        unable to do this in row_search_idx_cond_check(),
        because the condition is on the secondary index
        and the requested column is in the clustered index.
        We convert all fields, including those that
        may have been used in ICP, because the
        secondary index may contain a column prefix
        rather than the full column. Also, as noted
        in Bug #56680, the column in the secondary
        index may be in the wrong case, and the
        authoritative case is in result_rec, the
        appropriate version of the clustered index record.
      */
      if (!row_sel_store_mysql_rec(buf, prebuilt, clust_rec, nullptr, true,
                                   clust_index, prebuilt->index, offsets,
                                   false, nullptr, prebuilt->blob_heap)) {
        goto next_rec;
      }
    }
  } else {
    result_rec = rec;
  }

  /*
    Decide whether to prefetch extra rows.
    At this point, the clustered index record is protected
    by a page latch that was acquired when pcur was positioned.
    The latch will not be released until mtr_commit(&mtr).
  */
  if (record_buffer != nullptr) {
    const auto max_rows_to_cache = record_buffer->max_records();
    /*
      We only convert from InnoDB row format to MySQL row
      format when ICP is disabled.
    */
    if (!prebuilt->idx_cond) {
      /*
        We use next_buf to track the allocation of buffers
        where we store and enqueue the buffers for our
        pre-fetch optimisation.

        If next_buf == 0 then we store the converted record
        directly into the MySQL record buffer (buf). If it is
        != 0 then we allocate a pre-fetch buffer and store the
        converted record there.

        If the conversion fails and the MySQL record buffer
        was not written to then we reset next_buf so that
        we can re-use the MySQL record buffer in the next
        iteration.
      */
      next_buf = next_buf ? row_sel_fetch_last_buf(prebuilt) : buf;

      if (!row_sel_store_mysql_rec(next_buf, prebuilt, result_rec, nullptr,
              result_rec != rec, result_rec != rec ? clust_index : index,
              prebuilt->index, offsets, false, nullptr, prebuilt->blob_heap)) {
        if (next_buf == buf) {
          ut_a(prebuilt->n_fetch_cached == 0);
          next_buf = nullptr;
        }

        /*
          Only fresh inserts may contain incomplete
          externally stored columns. Pretend that such
          records do not exist. Such records may only be
          accessed at the READ UNCOMMITTED isolation
          level or when rolling back a recovered
          transaction. Rollback happens at a lower
          level, not here.
        */
        goto next_rec;
      }

      if (next_buf != buf) {
        row_sel_enqueue_cache_row_for_mysql(next_buf, prebuilt);
      }
    } else {
      row_sel_enqueue_cache_row_for_mysql(buf, prebuilt);
    }

    if (prebuilt->n_fetch_cached < max_rows_to_cache) {
      goto next_rec;
    }

  } else {
    /*
      We cannot use a record buffer for this scan, so assert that
      we don't have one. If we have a record buffer here,
      ha_innobase::is_record_buffer_wanted() should be updated so
      that a buffer is not allocated unnecessarily.
    */
    ut_ad(record_buffer == nullptr);

    if (!prebuilt->idx_cond) {
      /* The record was not yet converted to MySQL format. */
      if (!row_sel_store_mysql_rec(buf, prebuilt, result_rec, nullptr,
                                   result_rec != rec,
                                   result_rec != rec ? clust_index : index,
                                   prebuilt->index, offsets, false,
                                   prebuilt->get_lob_undo(), prebuilt->blob_heap)) {
        goto next_rec;
      }
    }

    if (prebuilt->clust_index_was_generated) {
      row_sel_store_row_id_to_prebuilt(prebuilt, result_rec,
                                       result_rec == rec ? index : clust_index,
                                       offsets);
    }
  }

  err = DB_SUCCESS;

px_ctx_cond_failed:
  pcur->store_position(&mtr);
  goto normal_return;

next_rec:

  /*-------------------------------------------------------------*/
  /* PHASE 5: Move the cursor to the next index record */

  /*
    NOTE: For moves_up==FALSE, the mini-transaction will be
    committed and restarted every time when switching b-tree
    pages. For moves_up==TRUE in index condition pushdown, we can
    scan an entire secondary index tree within a single
    mini-transaction. As long as the prebuilt->idx_cond does not
    match, we do not need to consult the clustered index or
    return records to MySQL, and thus we can avoid repositioning
    the cursor. What prevents us from buffer-fixing all leaf pages
    within the mini-transaction is the btr_leaf_page_release()
    call in btr_pcur_move_to_next_page(). Only the leaf page where
    the cursor is positioned will remain buffer-fixed.
  */
  if (mtr_has_extra_clust_latch) {
    /*
      If we have extra cluster latch, we must commit
      mtr if we are moving to the next non-clustered
      index record, because we could break the latching
      order if we would access a different clustered
      index page right away without releasing the previous.
    */
    pcur->store_position(&mtr);
    mtr_commit(&mtr);
    mtr_has_extra_clust_latch = false;

    mtr_start(&mtr);
    
    const bool result = sel_restore_position_for_mysql(
          &same_user_rec, BTR_SEARCH_LEAF, pcur, moves_up, &mtr);
    
    if (result) {
      goto rec_loop;
    }

    ut_ad(same_user_rec);
  }

  if (moves_up) {
    bool move;
    move = pcur->move_to_next(&mtr);

    if (!move) {
    not_moved:
      pcur->store_position(&mtr);
      err = DB_END_OF_INDEX;
      goto normal_return;
    }
  } else {
    if (UNIV_UNLIKELY(!pcur->move_to_prev(&mtr))) {
      goto not_moved;
    }
  }

  goto rec_loop;

lock_wait_or_error:
  pcur->store_position(&mtr);
  prebuilt->trx->error_state = err;
  mtr_has_extra_clust_latch = false;

  goto func_exit;

normal_return:
  ut_a(mtr.is_active());
  mtr.commit();

  if (prebuilt->idx_cond != 0) {
    /*
      When ICP is active we don't write to the MySQL buffer
      directly, only to buffers that are enqueued in the pre-fetch
      queue. We need to dequeue the first buffer and copy the contents
      to the record buffer that was passed in by MySQL.
    */
    if (prebuilt->n_fetch_cached > 0) {
      row_sel_dequeue_cached_row_for_mysql(buf, prebuilt);
      err = DB_SUCCESS;
    }
  } else if (next_buf != nullptr) {
    /*
      We may or may not have enqueued some buffers to the
      pre-fetch queue, but we definitely wrote to the record
      buffer passed to use by MySQL.
    */
    err = DB_SUCCESS;
  }

func_exit:
  trx->op_info = "";

  DEBUG_SYNC_C("innodb_row_search_for_px_exit");

  prebuilt->lob_undo_reset();

  ut_a(!trx->has_search_latch);

  return err;
}







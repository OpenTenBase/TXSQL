#ifndef row0px_h
#define row0px_h

#include <mutex>
#include <condition_variable>

#include "row0sel.h"
#include "btr0cur.h"
#include "db0err.h"
#include "fil0fil.h"
#include "os0event.h"
#include "page0size.h"
#include "rem0types.h"
#include "ut0mpmcbq.h"

class PX_Scan_range;
class PX_Config;
class PX_Ctx;
class PX_Scan_ctx;
class PX_PCursor;

/** 
  The parallel reader context in storage engine used in parallel query.
 */
class PX_reader {
 public:
  using Links = std::vector<page_no_t, ut::allocator<page_no_t>>;

  /** Constructor.
  @param[in]  max_threads Maximum number of threads to use. */
  explicit PX_reader(size_t max_threads);

  /** Destructor. */
  ~PX_reader();

  /** Add scan context.
  @param[in,out]  trx         Covering transaction.
  @param[in]      config      Scan condfiguration.
  (default is 0 which is leaf level)
  @return error. */
  dberr_t add_scan(trx_t *trx, const PX_Config &config)
      MY_ATTRIBUTE((warn_unused_result));

  /** Get the error stored in the global error state.
  @return global error state. */
  dberr_t get_error_state() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_err);
  }

  /** @return true if the tree is empty, else false. */
  bool is_tree_empty() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_ctx_id.load(std::memory_order_relaxed) == 0);
  }

  /** @return the configured max threads size. */
  size_t max_threads() const MY_ATTRIBUTE((warn_unused_result)) {
    return m_max_threads;
  }

  /** @return true if in error state. */
  bool is_error_set() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_err.load(std::memory_order_relaxed) != DB_SUCCESS);
  }

  /** Set the error state.
  @param[in] err                Error state to set to. */
  void set_error_state(dberr_t err) {
    m_err.store(err, std::memory_order_relaxed);
  }

  /** Increment the m_n_completed. */
  void incr_completed() { 
    m_n_completed.fetch_add(1, std::memory_order_relaxed);
  }

  /** Set the m_task_finished. */
  void set_task_finished() {
    m_task_finished.store(true, std::memory_order_relaxed);
  }

  /** Notify all worker threads. */
  void wakeup_workers() {
    os_event_set(m_event);
  }

  /** Dispatch ctx to worker thread.
  @param[in,out]  task dispatched to worker.
  @return error. */
  dberr_t task_dispatch(std::shared_ptr<PX_Ctx> &task);

  dberr_t split();

  uint get_total_ctxs() { return m_ctxs.size(); }

  // Disable copying.
  PX_reader(const PX_reader &) = delete;
  PX_reader(const PX_reader &&) = delete;
  PX_reader &operator=(PX_reader &&) = delete;
  PX_reader &operator=(const PX_reader &) = delete;

 public:
  uint key{0};
  ReadView *snapshot{};
  bool m_reverse_scan{false};

 private:
  /** Reset error state. */
  void reset_error_state() { m_err = DB_SUCCESS; }

  /** Add an execution context to the run queue.
  @param[in] ctx                Execution context to add to the queue. */
  void enqueue(std::shared_ptr<PX_Ctx> ctx);

  /** Fetch the next job execute.
  @return job to execute or nullptr. */
  std::shared_ptr<PX_Ctx> dequeue() MY_ATTRIBUTE((warn_unused_result));

  /** @return true if job queue is empty. */
  bool is_queue_empty() const MY_ATTRIBUTE((warn_unused_result)) {
    return m_ctxs.empty();
  }

  /** @return true if tasks are still executing. */
  bool is_active() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_n_completed.load(std::memory_order_relaxed) <
            m_ctx_id.load(std::memory_order_relaxed));
  }

  size_t calulate_split_point();

 private:
  // clang-format off
  using Ctxs =
      std::list<std::shared_ptr<PX_Ctx>,
                ut::allocator<std::shared_ptr<PX_Ctx>>>;

  using Scan_ctxs =
      std::list<std::shared_ptr<PX_Scan_ctx>,
                ut::allocator<std::shared_ptr<PX_Scan_ctx>>>;

  /** Maximum number of worker threads to use. */
  size_t m_max_threads{};

  /** Contexts that must be executed. */
  Ctxs m_ctxs{};

  /** Scan contexts. */
  Scan_ctxs m_scan_ctxs{};

  /** Counter for allocating scan context IDs. */
  size_t m_scan_ctx_id{};

  /** Context ID. Monotonically increasing ID. */
  std::atomic_size_t m_ctx_id{};

  /** Total tasks executed so far. */
  std::atomic_size_t m_n_completed{};

  /** Total tasks to be executed. */
  std::atomic_size_t m_n_tasks{};

  /** The flag the query has finished. */
  std::atomic<bool> m_task_finished{false};

  /** Error during parallel read. */
  std::atomic<dberr_t> m_err{DB_SUCCESS};

  /** Mutex protecting m_ctxs. */
  mutable ib_mutex_t m_mutex;

  /** For signalling worker threads about events. */
  os_event_t m_event{};

  friend class PX_Ctx;
  friend class PX_Scan_ctx;
};

/** Specifies the range from where to start the scan and where to end it. */
struct PX_Scan_range {
  /** Default constructor. */
  PX_Scan_range() : m_start(), m_end() {}

  /** Copy constructor.
  @param[in] scan_range       Instance to copy from. */
  PX_Scan_range(const PX_Scan_range &scan_range)
        : m_start(scan_range.m_start), m_end(scan_range.m_end) {}

  /** Constructor.
  @param[in] start            Start key
  @param[in] end              End key. */
  PX_Scan_range(const dtuple_t *start, const dtuple_t *end)
    : m_start(start), m_end(end) {}

  /** Start of the scan, can be nullptr for -infinity. */
  const dtuple_t *m_start{};

  /** End of the scan, can be null for +infinity. */
  const dtuple_t *m_end{};
};

/** Scan (Scan_ctx) configuration. */
struct PX_Config {
  /** Constructor.
  @param[in] scan_range     Range to scan.
  @param[in] index          Cluster index to scan.
  @param[in] read_level     Btree level from which records need to be read.
  belongs to a partitioned table. */
  PX_Config(const PX_Scan_range &scan_range, dict_index_t *index,
            size_t read_level = 0)
    : m_scan_range(scan_range),
      m_index(index),
      m_is_compact(dict_table_is_comp(index->table)),
      m_page_size(dict_tf_to_fsp_flags(index->table->flags)),
      m_read_level(read_level) {}

  /** Copy constructor.
  @param[in] config           Instance to copy from. */
  PX_Config(const PX_Config &config)
    : m_scan_range(config.m_scan_range),
      m_index(config.m_index),
      m_is_compact(config.m_is_compact),
      m_page_size(config.m_page_size),
      m_read_level(config.m_read_level),
      m_last_pcur(config.m_last_pcur),
      m_reverse_scan(config.m_reverse_scan) {}

  /** Range to scan. */
  const PX_Scan_range m_scan_range;

  /** (Cluster) Index in table to scan. */
  dict_index_t *m_index{};

  /** Row format of table. */
  const bool m_is_compact{};

  /** Tablespace page size. */
  const page_size_t m_page_size;

  /** Btree level from which records need to be read. */
  size_t m_read_level{0};

  btr_pcur_t *m_last_pcur{nullptr};

  bool m_reverse_scan{false};
};

/** Parallel reader context. */
class PX_Scan_ctx {
 public:
  /** Constructor.
  @param[in]  reader          Parallel reader that owns this context.
  @param[in]  id              ID of this scan context.
  @param[in]  trx             Transaction covering the scan.
  @param[in]  config          Range scan config. */
  PX_Scan_ctx(PX_reader *reader, size_t id, trx_t *trx,
           const PX_Config &config);

  /** Destructor. */
  ~PX_Scan_ctx();

 private:
  /** Boundary of the range to scan. */
  struct Iter {
    /** Destructor. */
    ~Iter();

    /** Heap used to allocate m_rec, m_tuple and m_pcur. */
    mem_heap_t *m_heap{};

    /** m_rec column offsets. */
    const ulint *m_offsets{};

    /** Start scanning from this key. Raw data of the row. */
    const rec_t *m_rec{};

    /** Tuple representation inside m_rec, for two Iter instances in a range
    m_tuple will be [first->m_tuple, second->m_tuple). */
    const dtuple_t *m_tuple{};

    /** Persistent cursor.*/
    btr_pcur_t *m_pcur{};
  };

  /** mtr_t savepoint. */
  using Savepoint = std::pair<ulint, buf_block_t *>;

  /** For releasing the S latches after processing the blocks. */
  using Savepoints = std::vector<Savepoint, ut::allocator<Savepoint>>;

  /** The first cursor should read up to the second cursor [f, s). */
  using Range = std::pair<std::shared_ptr<Iter>, std::shared_ptr<Iter>>;

  using Ranges = std::vector<Range, ut::allocator<Range>>;

  /** @return the scan context ID. */
  size_t id() const MY_ATTRIBUTE((warn_unused_result)) { return (m_id); }

  /** Set the error state.
  @param[in] err                Error state to set to. */
  void set_error_state(dberr_t err) {
    m_err.store(err, std::memory_order_relaxed);
  }

  /** @return true if in error state. */
  bool is_error_set() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_err.load(std::memory_order_relaxed) != DB_SUCCESS);
  }

  /** Fetch a block from the buffer pool and acquire an S latch on it.
  @param[in]      page_id       Page ID.
  @param[in,out]  mtr           Mini-transaction covering the fetch.
  @param[in]      line          Line from where called.
  @return the block fetched from the buffer pool. */
  buf_block_t *block_get_s_latched(const page_id_t &page_id, mtr_t *mtr,
                                   size_t line) const
      MY_ATTRIBUTE((warn_unused_result));

  /** Partition the B+Tree for parallel read.
  @param[in] scan_range Range for partitioning.
  @param[in,out]  ranges        Ranges to scan.
  @param[in] split_level  Sub-range required level (0 == root).
  @return the partition scan ranges. */
  dberr_t partition(const PX_Scan_range &scan_range, Ranges &ranges,
                    size_t split_level);

  /** Find the page number of the node that contains the search key. If the
  key is null then we assume -infinity.
  @param[in]  block             Page to look in.
  @param[in] key                Key of the first record in the range.
  @return the left child page number. */
  page_no_t search(const buf_block_t *block, const dtuple_t *key) const
      MY_ATTRIBUTE((warn_unused_result));

  /** Traverse from given sub-tree page number to start of the scan range
  from the given page number.
  @param[in]      page_no       Page number of sub-tree.
  @param[in,out]  mtr           Mini-transaction.
  @param[in]      key           Key of the first record in the range.
  @param[in,out]  savepoints    Blocks S latched and accessed.
  @return the leaf node page cursor. */
  page_cur_t start_range(page_no_t page_no, mtr_t *mtr, const dtuple_t *key,
                         Savepoints &savepoints) const
      MY_ATTRIBUTE((warn_unused_result));

  /** Create and add the range to the scan ranges.
  @param[in,out]  ranges        Ranges to scan.
  @param[in,out]  leaf_page_cursor Leaf page cursor on which to create the
                                persistent cursor.
  @param[in,out]  mtr           Mini-transaction */
  void create_range(Ranges &ranges, page_cur_t &leaf_page_cursor,
                    mtr_t *mtr) const;

  /** Find the subtrees to scan in a block.
  @param[in]      scan_range    Partition based on this scan range.
  @param[in]      page_no       Page to partition at if at required level.
  @param[in]      depth         Sub-range current level.
  @param[in]      split_level   Sub-range starting level (0 == root).
  @param[in,out]  ranges        Ranges to scan.
  @param[in,out]  mtr           Mini-transaction */
  dberr_t create_ranges(const PX_Scan_range &scan_range, page_no_t page_no,
                        size_t depth, const size_t split_level, Ranges &ranges,
                        mtr_t *mtr);

  /** Build a dtuple_t from rec_t.
  @param[in]      rec           Build the dtuple from this record.
  @param[in,out]  iter          Build in this iterator. */
  void copy_row(const rec_t *rec, Iter *iter) const;

  /** Create the persistent cursor that will be used to traverse the
  partition and position on the the start row.
  @param[in]      page_cursor   Current page cursor
  @param[in]      mtr           Mini-transaction covering the read.
  @return Start iterator. */
  std::shared_ptr<Iter> create_persistent_cursor(const page_cur_t &page_cursor,
                                                 mtr_t *mtr) const
      MY_ATTRIBUTE((warn_unused_result));

  dberr_t find_visible_record(byte *buf, const rec_t *&rec,
                              const rec_t *&clust_rec, ulint *&offsets,
                              ulint *&clust_offsets, mem_heap_t *&heap,
                              mtr_t *mtr, row_prebuilt_t *prebuilt = nullptr)
      MY_ATTRIBUTE((warn_unused_result));

  /** Create an execution context for a range and add it to
  the Parallel_reader's run queue.
  @param[in] range              Range for which to create the context.
  @param[in] split              true if the sub-tree should be split further.
  @return DB_SUCCESS or error code. */
  dberr_t create_context(const Range &range, bool split)
      MY_ATTRIBUTE((warn_unused_result));

  /** Create the execution contexts based on the ranges.
  @param[in]  ranges            Ranges for which to create the contexts.
  @return DB_SUCCESS or error code. */
  dberr_t create_contexts(const Ranges &ranges)
      MY_ATTRIBUTE((warn_unused_result));

  /** @return the maximum number of threads configured. */
  size_t max_threads() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_reader->m_max_threads);
  }

  /** S lock the index. */
  void index_s_lock();

  /** S unlock the index. */
  void index_s_unlock();

  /** @return true if at least one thread owns the S latch on the index. */
  bool index_s_own() const {
    return (m_s_locks.load(std::memory_order_acquire) > 0);
  }

 private:
  /** Context ID. */
  size_t m_id{std::numeric_limits<size_t>::max()};

  /** Parallel scan configuration. */
  PX_Config m_config;

  /** Covering transaction. */
  const trx_t *m_trx{};

  /** Depth of the Btree. */
  size_t m_depth{};

  /** The parallel reader. */
  PX_reader *m_reader{};

  /** Error during parallel read. */
  mutable std::atomic<dberr_t> m_err{DB_SUCCESS};

  /** Number of threads that have S locked the index. */
  std::atomic_size_t m_s_locks{};

  friend class PX_reader;
  friend class PX_Ctx;

  PX_Scan_ctx(PX_Scan_ctx &&) = delete;
  PX_Scan_ctx(const PX_Scan_ctx &) = delete;
  PX_Scan_ctx &operator=(PX_Scan_ctx &&) = delete;
  PX_Scan_ctx &operator=(const PX_Scan_ctx &) = delete;
};

/** Parallel reader execution context. */
class PX_Ctx {
 public:
  /** Constructor.
  @param[in]    id              Thread ID.
  @param[in]    scan_ctx        Scan context.
  @param[in]    range           Range that the thread has to read. */
  PX_Ctx(size_t id, PX_Scan_ctx *scan_ctx, const PX_Scan_ctx::Range &range)
      : m_id(id), m_range(range), m_scan_ctx(scan_ctx) {}

  /** Destructor. */
  ~PX_Ctx();

 public:
  /** @return the context ID. */
  size_t id() const MY_ATTRIBUTE((warn_unused_result)) { return (m_id); }

  /** The scan ID of the scan context this belongs to. */
  size_t scan_id() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_scan_ctx->id());
  }

  /** @return the covering transaction. */
  const trx_t *trx() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_scan_ctx->m_trx);
  }

  /** @return the index being scanned. */
  const dict_index_t *index() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_scan_ctx->m_config.m_index);
  }

  /** Scan the rows of PX_Ctx with mvcc in Parallel query.
  @param[in, out] buf record[0] of the table
  @param[in] prebuilt row_prebuilt_t of worker thd
  @return error. */
  dberr_t row_search_px(uchar *buf, row_prebuilt_t *prebuilt);

 private:
  /** Split the context into sub-ranges and add them to the execution queue.
  @return DB_SUCCESS or error code. */
  dberr_t split() MY_ATTRIBUTE((warn_unused_result));

  /** @return true if in error state. */
  bool is_error_set() const MY_ATTRIBUTE((warn_unused_result)) {
    return m_scan_ctx->m_reader->is_error_set() || m_scan_ctx->is_error_set();
  }

 private:
  /** Context ID. */
  size_t m_id{std::numeric_limits<size_t>::max()};

  /** If true the split the context at the block level. */
  bool m_split{};

  /** Range to read in this context. */
  PX_Scan_ctx::Range m_range{};

  /** Scanner context. */
  PX_Scan_ctx *m_scan_ctx{};

 public:
  /** Current block. */
  const buf_block_t *m_block{};

  /** Current row. */
  const rec_t *m_rec{};

  /** Number of pages traversed by the context. */
  size_t m_n_pages{};

  /** True if m_rec is the first record in the page. */
  bool m_first_rec{true};

  ulint *m_offsets{};

  /** Start of a new range to scan. */
  bool m_start{};

  bool start_read{true};

  mem_heap_t *m_heap{};

  ulint offsets_[REC_OFFS_NORMAL_SIZE]{};

  ulint clust_offsets_[REC_OFFS_NORMAL_SIZE]{};

  friend class PX_reader;
  friend class PX_Scan_ctx;
};

#endif
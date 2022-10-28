//file is named with tlog and start trx id
//for example: tlog_111, means the tlog file is started with id 111
#ifndef trx0tlog_h
#define trx0tlog_h
#define TLOG_FILE_PREFIX "tlog_"
#define TLOG_FILE_PREFIX_LEN 5
#define TLOG_PAGE_SIZE 4096
#define TLOG_PAGE_NUM_PER_FILE 4096
#define TLOG_FILE_SIZE (TLOG_PAGE_NUM_PER_FILE * TLOG_PAGE_SIZE)

#define TLOG_PAGE_HEADER 128

#define TLOG_PAGE_IS_DIRTY 0
#define TLOG_PAGE_MIN_GTS 1
#define TLOG_PAGE_MAX_GTS (TLOG_PAGE_MIN_GTS + 8)
#define TLOG_PAGE_CHECKSUM (TLOG_PAGE_MAX_GTS + 8)

/**Structure of each record */
#define TLOG_GTS_BYTES 8
#define TLOG_GTS_VALUE 0

#define TLOG_ELEMS_PER_PAGE ((TLOG_PAGE_SIZE - TLOG_PAGE_HEADER) / TLOG_GTS_BYTES)

/* The first file is reserved, so we onlu use TLOG_PAGE_NUM_PER_FILE - 1 pages
to stoe elements */
#define TLOG_ELEMS_PER_FILE (TLOG_ELEMS_PER_PAGE * (TLOG_PAGE_NUM_PER_FILE - 1))

#include <atomic>
#include <string>

/**
Structure of tlog file:
- file name: tlog_[0, 1, 2....]
- first page: reserved for feature usage.
- from second page, we start storing gts value
- for each page, there's 128 reserved bytes for storing
  information, currently only used 25bytes
- each gts occupies 8bytes.
*/
class TLogFile {
  public:
    TLogFile(uint64_t file_num, const std::string &dir);

    ~TLogFile();

    struct Position {
      uint64_t page_no;
      uint64_t offset;
    };

    Position id_to_position(trx_id_t id) {
      ut_a(id >= m_start_id);
      ut_a(id < m_end_id);

      uint64_t delta = (id - m_start_id) * TLOG_GTS_BYTES;

      uint64_t elem_size_in_page = (TLOG_PAGE_SIZE - TLOG_PAGE_HEADER);

      ut_a(elem_size_in_page % TLOG_GTS_BYTES == 0);

      Position pos;

      pos.page_no = delta / elem_size_in_page + 1/* first page is reserved */;
      pos.offset = delta % elem_size_in_page + TLOG_PAGE_HEADER;

      ut_a(pos.page_no < TLOG_PAGE_NUM_PER_FILE);

      return pos;
    }

    /** Functions for manage buffer that stores everything
      of the tlog file. */
    void create_buffer();
    bool map_buffer();
    void close_buffer();
    void flush_buffer();

    /** Get Min/MAX gts by scanning file */
    void init_gts();

    /** Write mapping of trx_id => gts to file */
    void write_gts(trx_id_t id, uint64_t gts);

    /** Get gts value or zero */
    uint64_t get_gts(trx_id_t id);

    /** Manage tlog file */
    bool create_file();
    void delete_file();
    bool open_file(pfs_os_file_t &file);

    /** Manage reference on page */
    void inc_ref() { m_ref++; }
    void dec_ref() { m_ref--; }
    uint64_t ref() { return m_ref.load(); }

    trx_id_t start_id() const { return m_start_id; }

    trx_id_t end_id() const { return m_end_id; }

    void close_if_possible();

    void mark_deleted() { m_deleted = true; }

    bool is_deleted() { return m_deleted; }

    uint64_t max_gts() const { return m_max_gts; }
  private:
    void do_flushing(pfs_os_file_t &file, bool acquire_lock);

    void x_lock();
    void x_unlock();
    void s_lock();
    void s_unlock();

    trx_id_t m_start_id;
    trx_id_t m_end_id;

    uint64_t m_total_element_in_file;

    /** Min commit gts in file, or zero if
      this is an empty file */
    std::atomic<uint64_t> m_min_gts;

    /** Max commit gts in file, or zero if
      this is an empty file */
    std::atomic<uint64_t> m_max_gts;

    /** Use clock to avoid unnecessary flushing */
    std::atomic<uint64_t> m_modify_clock;
    std::atomic<uint64_t> m_modify_done_clock;
    std::atomic<uint64_t> m_flushed_clock;

    /* If the tlog doesn't be accessed for too
       long time, we should close the buffer for it. */
    time_t m_access_time;

    /** Read/Write reference on file. */
    std::atomic<uint64_t> m_ref;

    std::atomic<bool> m_being_flush;

    /** True if the file will be deleted */
    bool m_deleted;

    rw_lock_t *m_lock;

    /* File buffer in memory, aligned by TLOG_PAGE_SIZE. */
    byte *m_file_buffer;

    /** Not aligned buffer */
    byte *m_file_buffer_ptr;

    std::string m_path;
};

using TlogFiles = std::map<uint64_t, TLogFile*>;
class TLogManager {
  public:
    enum gts_state_t {
      ACTIVE = 1,
      PREPARED = 2,
      COMMITTED = 3,
      UNKNOWN_STATE
    };

    /** Constructor */
    TLogManager(const char *dir);

    /** Destructor */
    ~TLogManager();

    struct STATE {
      std::atomic<uint64_t> io_read_counter;
      std::atomic<uint64_t> io_write_counter;
    };

    struct STATE state;

    uint64_t trx_id_to_file_num(trx_id_t trx_id) {
      return (trx_id / TLOG_ELEMS_PER_FILE);
    }

    /** Init tlog system on startup */
    bool init();

    /** Write mapping of trx_id => gts to file */
    void write_gts(trx_id_t id, uint64_t gts);

    void set_prepared(trx_id_t id, uint64_t gts);

    enum gts_state_t trx_state(trx_id_t trx_id, uint64_t& state_gts);

    /** Log GTS along with prepare log */
    void log_prepare_gts(mtr_t *mtr, trx_id_t id, uint64_t gts);

    /** Log GTS along with commit log */
    void log_commit_gts(mtr_t *mtr, trx_id_t id, uint64_t gts) {
      log_gts(mtr, id, gts);
    }

    /** Get gts value or zero */
    uint64_t get_gts(trx_id_t id, bool &purged);

    void flush_buffers();
    /** Run after crash recovery */
    void post_recovery();

    /** Delete unused files. Done by background threads*/
    void purge(uint64_t gts);

    void purge_all();

    void close_if_possible();

    /** Shutdown all tlog files. */
    void shutdown();

    void part_xlock(uint64_t part_id);
    void part_xunlock(uint64_t part_id);
    void part_slock(uint64_t part_id);
    void part_sunlock(uint64_t part_id);

    /** Increase number of buffers in memory */
    void inc_buffer() {
      m_buffer_counter++;
    }

    /** Decrease number of buffers in memory */
    void dec_buffer() {
      m_buffer_counter--;
    }

    void set_max_committed_gts(uint64_t new_gts);

    uint64_t max_committed_gts() {
      return m_max_committed_gts.load();
    }

  private:
    /** Get tlog file by giving trx_id or create a
      new file if param create is true. */
    TLogFile *get_file(trx_id_t trx_id, bool create, bool &purged);

    /** Get gts value or zero */
    uint64_t get_gts_value(trx_id_t id, bool &purged);

    uint64_t get_part(trx_id_t start_id, trx_id_t current_id) {
      ut_a(current_id >= start_id);

      const uint64_t elem_per_page = (TLOG_PAGE_SIZE - TLOG_PAGE_HEADER) / TLOG_GTS_BYTES;

      uint64_t ret_val = ((current_id - start_id) / elem_per_page) + 1;

      ut_a(ret_val < TLOG_PAGE_NUM_PER_FILE);

      return ret_val;
    }

    /** Log GTS along with commit log */
    void log_gts(mtr_t *mtr, trx_id_t id, uint64_t gts);

    void purge_slock();
    void purge_xlock();
    void purge_sunlock();
    void purge_xunlock();
    void lock();
    void unlock();

    std::atomic<uint32_t> m_buffer_counter;
    std::atomic<uint64_t> m_max_committed_gts;

    std::string m_dir;

    TlogFiles m_files;

    rw_lock_t* m_locks[TLOG_PAGE_NUM_PER_FILE];

    rw_lock_t *m_purge_lock;

    ib_mutex_t m_mutex;

    uint64_t m_max_purged_file;

};

extern TLogManager *tlog_mgr;
#endif

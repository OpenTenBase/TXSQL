#include "trx0trx.h"
#include "trx0sys.h"
#include "trx0types.h"
#include "srv0srv.h"
#include "os0file.h"
#include "sync0rw.h"
#include "sync0sync.h"
#include "trx0tlog.h"
TLogManager *tlog_mgr = nullptr;

#define TLOG_GTS_MASK          0xFFFFFFFF0FFFFFFF
#define TLOG_GTS_FLAGS                 0xF0000000
#define TLOG_GTS_PREPARED_FLAG         0x80000000

TLogFile::TLogFile(uint64_t file_num, const std::string &dir) {
  /* Store to avoid extra calculation */
  m_total_element_in_file = TLOG_ELEMS_PER_FILE;
  m_start_id = file_num * m_total_element_in_file;
  m_end_id = m_start_id + m_total_element_in_file;
  m_min_gts = m_max_gts = 0;

  m_flushed_clock = 0;
  m_modify_clock = 0;
  m_modify_done_clock = 0;

  m_access_time = 0;
  m_ref = 0;
  m_being_flush = false;
  m_deleted = false;

  m_lock = static_cast<rw_lock_t *>(ut::malloc(sizeof(rw_lock_t)));
  rw_lock_create(tlog_file_lock_key, m_lock, SYNC_NO_ORDER_CHECK);

  m_file_buffer = nullptr;
  m_file_buffer_ptr = nullptr;

  m_path = dir + std::string(TLOG_FILE_PREFIX) + std::to_string(file_num);
}

TLogFile::~TLogFile() {

  if (m_file_buffer != nullptr) {
    flush_buffer();
    close_buffer();
  }

  rw_lock_free(m_lock);
  ut::free(m_lock);
  ut_a(m_file_buffer_ptr == nullptr);
}

void TLogFile::x_lock() {
  rw_lock_x_lock(m_lock, UT_LOCATION_HERE);
}

void TLogFile::x_unlock() {
  rw_lock_x_unlock(m_lock);
}

void TLogFile::s_lock() {
  rw_lock_s_lock(m_lock, UT_LOCATION_HERE);
}

void TLogFile::s_unlock() {
  rw_lock_s_unlock(m_lock);
}

void TLogFile::init_gts() {
  uint64_t min_gts = ULONG_MAX;
  uint64_t max_gts = 0;

  if (m_file_buffer == nullptr) {
    bool ret = map_buffer();
    ut_a(ret);
  }

  ut_a(m_file_buffer != nullptr);

  for (uint64_t i = 1; i < TLOG_PAGE_NUM_PER_FILE; i++) {
     byte *page = m_file_buffer + i * TLOG_PAGE_SIZE;
     uint64_t read_min_gts = mach_read_from_8(page + TLOG_PAGE_MIN_GTS);
     uint64_t read_max_gts = mach_read_from_8(page + TLOG_PAGE_MAX_GTS);

     if (read_min_gts > 0 && read_min_gts < min_gts) {
       min_gts = read_min_gts;
     }

     if (read_max_gts > max_gts) {
       max_gts = read_max_gts;
     }
  }

  if (min_gts == ULONG_MAX) {
    min_gts = 0;
  }

  m_min_gts = min_gts;
  m_max_gts = max_gts;

  if (max_gts > 0) {
    tlog_mgr->set_max_committed_gts(max_gts);
  }

  close_buffer();
}

bool TLogFile::create_file() {
  bool ret;
  unlink(m_path.c_str());

  pfs_os_file_t file = os_file_create(
                      innodb_log_file_key, m_path.c_str(),
                      OS_FILE_CREATE|OS_FILE_ON_ERROR_NO_EXIT, OS_FILE_NORMAL,
                      OS_LOG_FILE, srv_read_only_mode, &ret);

  if (!ret) {
    ib::error() << "Fail to create file " << m_path.c_str();
    return false;
  }

  if (ftruncate(file.m_file, TLOG_FILE_SIZE) != 0) {
    ib::error() << "Fail to set size of " << m_path.c_str() << " to " << TLOG_FILE_SIZE;
    os_file_close(file);
    /* delete it */
    unlink(m_path.c_str());

    return false;
  }

  ret = os_file_close(file);
  ut_a(ret);

  return true;
}

void TLogFile::delete_file() {
  close_buffer();
  unlink(m_path.c_str());
}

bool TLogFile::open_file(pfs_os_file_t &file) {
  bool ret;
  file = os_file_create(innodb_log_file_key, m_path.c_str(),
                        OS_FILE_OPEN, OS_FILE_AIO,
                        OS_LOG_FILE, srv_read_only_mode, &ret);
  if (!ret) {
    ib::error() << "Fail to open " << m_path;
    return false;
  }

  return true;
}

void TLogFile::create_buffer() {
  ut_a(m_file_buffer_ptr == nullptr);

  m_file_buffer_ptr = (byte *)ut::malloc(TLOG_FILE_SIZE + TLOG_PAGE_SIZE * 2);
  ut_a(m_file_buffer_ptr != nullptr);
  memset(m_file_buffer_ptr, 0, TLOG_FILE_SIZE + TLOG_PAGE_SIZE * 2);

  m_file_buffer = static_cast<byte*>(ut_align(m_file_buffer_ptr, TLOG_PAGE_SIZE));

  tlog_mgr->inc_buffer();
}

void TLogFile::close_buffer() {
  ut_a(m_being_flush.load() == false);
  if (m_file_buffer_ptr == nullptr) {
    return;
  }

  ut::free(m_file_buffer_ptr);
  m_file_buffer_ptr = nullptr;
  m_file_buffer = nullptr;

  tlog_mgr->dec_buffer();
}

void TLogFile::close_if_possible() {
  //dirty read
  if (m_file_buffer == nullptr ||
      m_ref > 0 ||
      m_being_flush.load() == true) {
    return;
  }

  if (time(NULL) - m_access_time < 5) {
    //don't free if it's accesssed in 5 seconds
    return;
  }

  x_lock();
  if (m_file_buffer == nullptr ||
      m_ref > 0 ||
      m_being_flush.load() == true) {
    x_unlock();
    return;
  }

  pfs_os_file_t file;
  if (!open_file(file)) {
    x_unlock();
    return;
  }

  do_flushing(file, false);

  os_file_close(file);
  close_buffer();

  x_unlock();
}

bool TLogFile::map_buffer() {
  x_lock();
  if (m_file_buffer == nullptr) {
    create_buffer();
  } else {
    /* Already mapped to memory */
    x_unlock();
    return true;
  }

  pfs_os_file_t file;

  if (!open_file(file)) {
    close_buffer();
    x_unlock();
    return false;
  }

  IORequest request(IORequest::READ);
  request.disable_compression();

  dberr_t err = os_file_read(request, m_path.c_str(), file, m_file_buffer, 0, TLOG_FILE_SIZE);

  if (err != DB_SUCCESS) {
    ib::error() << "Fail to read from file " << m_path;
    close_buffer();
    x_unlock();
    os_file_close(file);

    return false;
  }

  x_unlock();
  os_file_close(file);

  tlog_mgr->state.io_read_counter++;

  return true;
}

void TLogFile::do_flushing(pfs_os_file_t &file, bool acquire_lock) {
  IORequest request(IORequest::WRITE);
  request.disable_compression();

#define MAX_BUFFER_COUNT  4
  byte *write_buffer_ptr = (byte *)ut::malloc(TLOG_PAGE_SIZE * MAX_BUFFER_COUNT
                                              + TLOG_PAGE_SIZE * 2);
  byte *write_buffer = static_cast<byte*>(ut_align(write_buffer_ptr, TLOG_PAGE_SIZE));
  uint64_t start_offset = ULONG_MAX;
  uint64_t count = 0;
  bool is_dirty = false;
  for (uint64_t i = 0 ; i < TLOG_PAGE_NUM_PER_FILE; i++) {
    if (acquire_lock) {
      tlog_mgr->part_slock(i);
    }
    byte *page = m_file_buffer + i * TLOG_PAGE_SIZE;
    is_dirty = mach_read_from_1(page + TLOG_PAGE_IS_DIRTY);

    if (is_dirty) {
      /* Reset the flag */
      mach_write_to_1(page + TLOG_PAGE_IS_DIRTY, 0);
      ut_a(count < MAX_BUFFER_COUNT);
      memcpy(write_buffer + count * TLOG_PAGE_SIZE, page, TLOG_PAGE_SIZE);
      count++;
      if (start_offset == ULONG_MAX) {
        start_offset = i;
      }
    }

    if (acquire_lock) {
      tlog_mgr->part_sunlock(i);
    }

    if (count == 0) {
      continue;
    }

    if (!is_dirty /* Current page isn't dirty, flush pending buffer*/
        || count == MAX_BUFFER_COUNT /* buffer is full, flush it */
        || (i == TLOG_PAGE_NUM_PER_FILE - 1)) /* Last page */ {
      dberr_t err = os_file_write(request, m_path.c_str(),
                                  file, write_buffer,
                                  start_offset * TLOG_PAGE_SIZE,
                                  count * TLOG_PAGE_SIZE);
      ut_a(err == DB_SUCCESS);
      start_offset = ULONG_MAX;
      count = 0;
      tlog_mgr->state.io_write_counter++;
    }
  }

  ut_a(count == 0);
  ut::free(write_buffer_ptr);
  os_file_flush(file);
}

void TLogFile::flush_buffer() {
  uint64_t modify_done_clock = 0;
loop:
  x_lock();
  modify_done_clock = m_modify_done_clock.load();
  if ((modify_done_clock == m_modify_clock.load())
        && (modify_done_clock == m_flushed_clock.load())) {
    //this is a clean page, do nothing
    x_unlock();
    return;
  }

  if (m_file_buffer == nullptr) {
    /* do nothing */
    x_unlock();
    return;
  }

  if (m_being_flush.load()) {
    x_unlock();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    goto loop;
  }

  m_being_flush = true;

  pfs_os_file_t file;
  if (!open_file(file)) {
    x_unlock();
    return;
  }

  x_unlock();

  do_flushing(file, true);

  m_flushed_clock = modify_done_clock;

  m_being_flush = false;

  os_file_close(file);
}

void TLogFile::write_gts(trx_id_t id, uint64_t gts) {

  ut_a(m_ref > 0);
  ut_a(gts > 0);

  /** Confirm file is mapped to memory */
  s_lock();
  m_modify_clock++;
  if (m_file_buffer == nullptr) {
    s_unlock();
    bool ret = map_buffer();
    ut_a(ret);
  } else {
    s_unlock();
  }

  ut_a(m_file_buffer != nullptr);
  Position pos = id_to_position(id);

  /** Write GTS */
  byte *page = m_file_buffer + pos.page_no * TLOG_PAGE_SIZE;
  mach_write_to_8(page + pos.offset + TLOG_GTS_VALUE, gts);

  /** Mark the page as dirty */
  mach_write_to_1(page + TLOG_PAGE_IS_DIRTY, 1);

  if (gts & TLOG_GTS_PREPARED_FLAG) {
    /* This is a prepared gts, just store the value */
    m_modify_done_clock++;
    m_access_time = time(NULL);
    return;
  }

  gts = (gts & TLOG_GTS_MASK);

  uint64_t min_gts_on_page = mach_read_from_8(page + TLOG_PAGE_MIN_GTS);
  if (min_gts_on_page == 0 || min_gts_on_page > gts) {
    mach_write_to_8(page + TLOG_PAGE_MIN_GTS, gts);
  }

  uint64_t max_gts_on_page = mach_read_from_8(page + TLOG_PAGE_MAX_GTS);
  if (max_gts_on_page < gts) {
    mach_write_to_8(page + TLOG_PAGE_MAX_GTS, gts);
  }

  /* Update the min gts */
  uint64_t old_min = m_min_gts.load();
  while (gts < old_min && !m_min_gts.compare_exchange_weak(old_min, gts));

  if (gts < old_min) {
    return;
  }

  /* Update the max gts */
  uint64_t old_max = m_max_gts.load();
  while (gts > old_max && !m_max_gts.compare_exchange_weak(old_max, gts));

  tlog_mgr->set_max_committed_gts(gts);

  m_modify_done_clock++;

  m_access_time = time(NULL);
}

uint64_t TLogFile::get_gts(trx_id_t id) {
  ut_a(m_ref > 0);

  /** Confirm file is mapped to memory */
  s_lock();
  if (m_file_buffer == nullptr) {
    s_unlock();
    bool ret = map_buffer();
    ut_a(ret);
  } else {
    s_unlock();
  }

  ut_a(m_file_buffer != nullptr);

  Position pos = id_to_position(id);

  byte* page = m_file_buffer + pos.page_no * TLOG_PAGE_SIZE;

  ut_a(pos.page_no >= 1);

  m_access_time = time(NULL);

  return (mach_read_from_8(page + pos.offset));
}

TLogManager::TLogManager(const char* dir) {
  m_dir = std::string(dir);
  m_files.clear();
  m_buffer_counter = 0;
  m_max_committed_gts = 0;
  state.io_read_counter = 0;
  state.io_write_counter = 0;
  m_max_purged_file = 0;

  for (uint64_t i = 0; i < TLOG_PAGE_NUM_PER_FILE; i++) {
    m_locks[i] = static_cast<rw_lock_t *>(ut::malloc(sizeof(rw_lock_t)));
    rw_lock_create(tlog_page_lock_key, m_locks[i], SYNC_NO_ORDER_CHECK);
  }

  m_purge_lock = static_cast<rw_lock_t *>(ut::malloc(sizeof(rw_lock_t)));
  rw_lock_create(tlog_purge_lock_key, m_purge_lock, SYNC_NO_ORDER_CHECK);

  mutex_create(LATCH_ID_TLOG_MGR, &m_mutex);
}

void TLogManager::lock() {
  mutex_enter(&m_mutex);
}

void TLogManager::unlock() {
  mutex_exit(&m_mutex);
}

void TLogManager::shutdown() {
  purge_xlock();
  for (auto item : m_files) {
    delete item.second;
  }

  purge_xunlock();

  m_files.clear();

  ut_a(m_buffer_counter.load() == 0);
}

TLogManager::~TLogManager() {
  for (uint64_t i = 0; i < TLOG_PAGE_NUM_PER_FILE; i++) {
    rw_lock_free(m_locks[i]);
    ut::free(m_locks[i]);
  }

  rw_lock_free(m_purge_lock);
  ut::free(m_purge_lock);

  mutex_free(&m_mutex);
}

void TLogManager::set_max_committed_gts(uint64_t new_gts) {
  /* Update the max gts */
  uint64_t old_max = m_max_committed_gts.load();
  while (new_gts > old_max &&
          !m_max_committed_gts.compare_exchange_weak(old_max, new_gts));
}

void TLogManager::purge_slock() {
  rw_lock_s_lock(m_purge_lock, UT_LOCATION_HERE);
}

void TLogManager::purge_xlock() {
  rw_lock_x_lock(m_purge_lock, UT_LOCATION_HERE);
}

void TLogManager::purge_sunlock() {
  rw_lock_s_unlock(m_purge_lock);
}

void TLogManager::purge_xunlock() {
  rw_lock_x_unlock(m_purge_lock);
}

void TLogManager::part_slock(uint64_t part_id) {
  rw_lock_s_lock(m_locks[part_id], UT_LOCATION_HERE);
}

void TLogManager::part_xlock(uint64_t part_id) {
  rw_lock_x_lock(m_locks[part_id], UT_LOCATION_HERE);
}

void TLogManager::part_sunlock(uint64_t part_id) {
  rw_lock_s_unlock(m_locks[part_id]);
}

void TLogManager::part_xunlock(uint64_t part_id) {
  rw_lock_x_unlock(m_locks[part_id]);
}

bool TLogManager::init() {
  os_file_dir_t dir = opendir(m_dir.c_str());

  if (!dir) {
    return false;
  }

  dberr_t err = DB_SUCCESS;
  os_file_stat_t state;

  while (fil_file_readdir_next_file(&err, m_dir.c_str(),
        dir, &state) == 0) {
    if (state.type != OS_FILE_TYPE_FILE ||
        strncmp(state.name, TLOG_FILE_PREFIX, TLOG_FILE_PREFIX_LEN) != 0) {
      continue;
    }

    char *end_ptr = nullptr;
    uint64_t id = strtoll(state.name + TLOG_FILE_PREFIX_LEN, &end_ptr, 10);
    if (end_ptr == nullptr || *end_ptr != '\0' || id ==  ULONG_MAX) {
      ib::error() << "Detect invalid tlog file " << state.name;
      continue;
    } else {
      ib::info() << "Detect tlog file with number " << id;
    }

    if (state.size != TLOG_FILE_SIZE) {
        ib::error() << "File " << state.name
                    << "has invalid size, you can remove it manually and restart it";
        closedir(dir);
        return false;
    }

    TLogFile *new_file = new TLogFile(id, m_dir);

    m_files[id] = new_file;
  }

  closedir(dir);

  /** Validate the collected files */
  if (!m_files.empty()) {
    for (auto item: m_files) {
      item.second->init_gts();
    }
  }

  return true;
}

void TLogManager::post_recovery() {
  if (m_files.empty()) {
    uint64_t num = trx_id_to_file_num(trx_sys_get_max_trx_id());
    TLogFile *new_file = new TLogFile(num, m_dir);
    bool ret = new_file->create_file();
    ut_a(ret);
    ret = new_file->map_buffer();
    ut_a(ret);
    m_files[num] = new_file;
  } else {
    /* Only map last file into memory */
    TLogFile* file = m_files.rbegin()->second;
    bool ret = file->map_buffer();
    ut_a(ret);
  }

  if (m_max_purged_file == 0) {
    /** Get minimal transaction id from trx system, because they
    may still active and may generate tlog files. */
    trx_id_t min_trx_id = trx_sys->get_min_trx_id();
    m_max_purged_file = trx_id_to_file_num(min_trx_id);

    if (m_max_purged_file > 0) {
      /* It should be exactly the value of file number that should
      never be written by any transaction, so here it decreases 1. */
      m_max_purged_file--;
    }

    uint64_t first_file_num = m_files.begin()->first;

    if (first_file_num > 0 &&
        m_max_purged_file >= first_file_num) {
      m_max_purged_file = first_file_num - 1;
    } else if (first_file_num == 0) {
      /* It's ok to set it to zero because we also check
      first element of m_files in get_file() function while
      setting purged flag. */
      m_max_purged_file = 0;
    }
  }
}

void TLogManager::log_prepare_gts(mtr_t *mtr, trx_id_t trx_id, uint64_t gts) {
   log_gts(mtr, trx_id, gts | TLOG_GTS_PREPARED_FLAG);
}

void TLogManager::log_gts(mtr_t *mtr, trx_id_t trx_id, uint64_t gts) {
  byte *ptr = nullptr;
  if (!mlog_open(mtr, 1 + 8 + 8, ptr)) {
    ut_error;
  }

  mach_write_to_1(ptr, MLOG_TRX_MAP_GTS);
  ptr += 1;
  mach_write_to_8(ptr, trx_id);
  ptr += 8;
  mach_write_to_8(ptr, gts);
  ptr += 8;

  mlog_close(mtr, ptr);

  mtr->added_rec();
}

void TLogManager::write_gts(trx_id_t trx_id, uint64_t gts) {
  /* Serializable writing to tlog. */
  lock();

  bool purged = false;
  TLogFile *tlog = get_file(trx_id, true, purged);
  if (tlog == nullptr) {
    unlock();
    ib::error() << "Fail to write trx_Id " << trx_id << " with gts " << gts;
    return;
  }

  ut_a(tlog->ref() > 0);

  uint64_t part = get_part(tlog->start_id(), trx_id);

  part_xlock(part);
  tlog->write_gts(trx_id, gts);
  part_xunlock(part);

  tlog->dec_ref();

  unlock();
}

uint64_t TLogManager::get_gts_value(trx_id_t trx_id, bool &purged) {
  TLogFile *tlog = get_file(trx_id, false, purged);
  if (tlog == nullptr) {
    return 0;
  }

  purged = false;

  ut_a(tlog->ref() > 0);

  uint64_t part = 0;

  /* If trx_id is smaller than min_active_id, it means all transaction before
  this id has written gts to file, and we don't need part lock to protect it. */
  //FIXME: If the invoker already checked that the tranaction has been committed,
  //we don't need the part lock
  bool need_part_lock = (trx_id >= trx_sys->get_min_trx_id());

  if (need_part_lock) {
    part = get_part(tlog->start_id(), trx_id);
    part_slock(part);
  }
  uint64_t gts = tlog->get_gts(trx_id);

  if (need_part_lock) {
    part_sunlock(part);
  }

  tlog->dec_ref();

  return (gts);
}

uint64_t TLogManager::get_gts(trx_id_t trx_id, bool &purged) {
  uint64_t gts = get_gts_value(trx_id, purged);
  return (gts & TLOG_GTS_MASK);
}

enum TLogManager::gts_state_t TLogManager::trx_state(trx_id_t trx_id, uint64_t& state_gts) {
  bool purged = false;
  uint64_t gts = get_gts_value(trx_id, purged);

  state_gts = (gts & TLOG_GTS_MASK);

  if (purged) {
    /** The related gts file is already purged, so the
    transaction must have been committed */
    return (TLogManager::COMMITTED);
  }

  if (gts == 0) {
    /* hasn't write any gts, still active. */
    return (TLogManager::ACTIVE);
  }

  if (gts & TLOG_GTS_PREPARED_FLAG) {
    /* Prepared flag is set */
    return (TLogManager::PREPARED);
  }

  if ((gts & TLOG_GTS_MASK) > 0) {
    return (TLogManager::COMMITTED);
  }

  return (TLogManager::UNKNOWN_STATE);
}

void TLogManager::set_prepared(trx_id_t trx_id, uint64_t prepared_gts) {
  write_gts(trx_id, prepared_gts | TLOG_GTS_PREPARED_FLAG);
}

TLogFile *TLogManager::get_file(trx_id_t trx_id, bool create, bool &purged) {
  TLogFile *file = nullptr;
  uint64_t file_num = trx_id_to_file_num(trx_id);
  /* Init to false */
  purged = false;
  bool x_locked = false;

  purge_slock();

  auto itr = m_files.find(file_num);

  if (itr == m_files.end()) {
    if (!create) {
      if (file_num <= m_max_purged_file) {
        purged = true;
      }

      if (file_num >= m_files.begin()->first) {
        purged = false;
      }

      purge_sunlock();
      return nullptr;
    }
  } else {
    file = itr->second;
    ut_a(itr->first == file_num);
    goto got_file;
  }

  ut_a(file == nullptr);

  /* In order to prevent reading from m_files map while
  creating new entry, let's release s lock and hold x lock. */
  purge_sunlock();
  purge_xlock();
  x_locked = true;

  /* Create new file */
  file = new TLogFile(file_num, m_dir);
  file->create_file();
  ut_a(m_files.find(file_num) == m_files.end());

  m_files[file_num] = file;

got_file:
  ut_a(file != nullptr);
  file->inc_ref();

  if (x_locked) {
    purge_xunlock();
  } else {
    purge_sunlock();
  }

  return file;
}

void TLogManager::purge_all() {
  purge_xlock();

  uint64_t max_file_num = m_files.empty() ? 0
                            : m_files.rbegin()->first;

  while (!m_files.empty()) {
    auto itr = m_files.begin();
    TLogFile *file = itr->second;

    if (file->ref() > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
      continue;
    }

    file->delete_file();
    delete file;

    m_files.erase(itr);
  }

  if (max_file_num > 0) {
    m_max_purged_file = max_file_num - 1;
  }

  //reset max_committed_gts
  m_max_committed_gts = 0;

  purge_xunlock();
}

//The purge task should be run by background thread
void TLogManager::purge(uint64_t gts) {
  TlogFiles to_delete;
  to_delete.clear();

  trx_id_t min_id = trx_sys->get_min_trx_id();
  purge_xlock();
  
  //may have just executed purge all
  //ut_a(!m_files.empty());

  for (auto item : m_files) {
    TLogFile *file =  item.second;
    if (file->end_id() >= min_id
        || file == m_files.rbegin()->second) {
      break;
    }

    if (file->max_gts() < gts && file->ref() == 0) {
      file->mark_deleted();
      to_delete[item.first] = item.second;
      m_max_purged_file = item.first;
    } else {
      break;
    }
  }

  while (m_files.begin() != m_files.end() && 
         m_files.begin()->second->is_deleted()) {
    trx_id_t start_id = m_files.begin()->first;
    m_files.erase(start_id);
  }
  purge_xunlock();

  for (auto item : to_delete) {
    TLogFile *file = item.second;
    file->delete_file();
    delete file;
  }

  to_delete.clear();

  close_if_possible();
}

void TLogManager::flush_buffers() {
  purge_slock();
  for (auto item = m_files.begin(); item != m_files.end(); item++) {
    TLogFile *file = item->second;
    file->flush_buffer();
  }

  purge_sunlock();
}

void TLogManager::close_if_possible() {
  purge_slock();
  if (m_files.size() <= 2 || m_buffer_counter.load() <= 4) {
    purge_sunlock();
    return;
  }

  auto itr = m_files.rbegin();
  itr++;
  TLogFile *end_file = itr->second;

  for (auto item = m_files.begin(); item != m_files.end(); item++) {
     TLogFile *file = item->second;
     if (file == end_file) {
      break;
     }

     file->close_if_possible();
  }
 purge_sunlock();
}

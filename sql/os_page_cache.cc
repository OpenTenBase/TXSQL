#include "os_page_cache.h"
#include <sql/malloc_allocator.h>
#include <my_sys.h>
#include <boost/functional/hash.hpp>
#include <list>
#include <unordered_set>
#include "log.h"  // sql_print_information
#include "my_systime.h"

struct page_cache_cleaning_t {
  std::string filename_full;
  File fd;
  my_off_t start_off;
  my_off_t len;
  page_cache_cleaning_t(const char *filename_full, my_off_t start_off,
                        my_off_t len)
      : filename_full(filename_full), start_off(start_off), len(len) {
    fd = -1;
  }
  page_cache_cleaning_t(File fd, my_off_t start_off, my_off_t len)
      : fd(fd), start_off(start_off), len(len) {}
  bool operator==(const page_cache_cleaning_t &other) const {
    if (filename_full == other.filename_full && start_off == other.start_off &&
        len == other.len) {
      return true;
    }
    return false;
  }
};

namespace std {
template <>
struct hash<page_cache_cleaning_t> {
  std::size_t operator()(const page_cache_cleaning_t &c) const {
    std::size_t result = 0;
    if (!c.filename_full.empty()) {
      boost::hash_combine(result, c.filename_full);
    }
    boost::hash_combine(result, c.fd);
    boost::hash_combine(result, c.start_off);
    boost::hash_combine(result, c.start_off + c.len);
    return result;
  }
};
}  // namespace std

mysql_mutex_t LOCK_page_cache_cleaning;
mysql_cond_t COND_page_cache_cleaning;
typedef std::list<page_cache_cleaning_t,
                  Malloc_allocator<page_cache_cleaning_t>>
    cleaning_task_list_t;
typedef std::unordered_set<page_cache_cleaning_t,
                           std::hash<page_cache_cleaning_t>,
                           std::equal_to<page_cache_cleaning_t>,
                           Malloc_allocator<page_cache_cleaning_t>>
    cleaning_task_hashmap_t;
cleaning_task_list_t page_cache_clean_list(
    (Malloc_allocator<page_cache_cleaning_t>(key_memory_page_cache_clean)));
cleaning_task_hashmap_t page_cache_clean_hashmap(
    (Malloc_allocator<page_cache_cleaning_t>(key_memory_page_cache_clean)));

void page_cache_post_cleaning_work(const char *filename_full,
                                   my_off_t start_off, my_off_t len) {
#ifdef POSIX_FADV_DONTNEED
  page_cache_cleaning_t pc =
      page_cache_cleaning_t(filename_full, start_off, len);
  mysql_mutex_lock(&LOCK_page_cache_cleaning);
  if (page_cache_clean_hashmap.find(pc) == page_cache_clean_hashmap.end()) {
    page_cache_clean_hashmap.insert(pc);
    page_cache_clean_list.push_back(std::move(pc));
  }
  // Wake up the cleaning worker thread
  mysql_cond_signal(&COND_page_cache_cleaning);
  mysql_mutex_unlock(&LOCK_page_cache_cleaning);
#endif /* POSIX_FADV_DONTNEED */
}

void page_cache_post_cleaning_work(File fd, my_off_t start_off, my_off_t len) {
#ifdef POSIX_FADV_DONTNEED
  page_cache_cleaning_t pc = page_cache_cleaning_t(fd, start_off, len);
  mysql_mutex_lock(&LOCK_page_cache_cleaning);
  /* Nothing will repeatedly pull redo logs. Just care about binlogs here */
  page_cache_clean_list.push_back(std::move(pc));
  // Wake up the cleaning worker thread
  mysql_cond_signal(&COND_page_cache_cleaning);
  mysql_mutex_unlock(&LOCK_page_cache_cleaning);
#endif /* POSIX_FADV_DONTNEED */
}

extern MYSQL_PLUGIN_IMPORT bool volatile abort_page_cache_cleaner;

void page_cache_cleanup() {
  mysql_mutex_lock(&LOCK_page_cache_cleaning);
  while (!page_cache_clean_list.empty()) {
    page_cache_clean_list.pop_front();
  }
  mysql_mutex_unlock(&LOCK_page_cache_cleaning);
}

void page_cache_process_cleaning_work() {
  struct timespec abstime;
  int error = 0;
  mysql_mutex_lock(&LOCK_page_cache_cleaning);
  if (page_cache_clean_list.empty()) {
    set_timespec(&abstime, 1);
    while ((!error || error == EINTR) && !abort_page_cache_cleaner)
      error = mysql_cond_timedwait(&COND_page_cache_cleaning,
                                   &LOCK_page_cache_cleaning, &abstime);
    mysql_mutex_unlock(&LOCK_page_cache_cleaning);
    return;
  }
  page_cache_cleaning_t clean = page_cache_clean_list.front();
  page_cache_clean_list.pop_front();
  mysql_mutex_unlock(&LOCK_page_cache_cleaning);
  File fd = clean.fd;
  bool opened = false;
  if (fd == -1) {
    fd = ::open(clean.filename_full.c_str(), O_RDONLY);
    opened = true;
  }
#ifdef POSIX_FADV_DONTNEED
  if (fd != -1) {
    // Since this is merely an advice to the operating system,
    // we don't care if the fadvise succeeded or not.
    ::posix_fadvise(fd, clean.start_off, clean.len, POSIX_FADV_DONTNEED);
  }

  DBUG_EXECUTE_IF("page_cache_cleaning_test", {
    if (!clean.filename_full.empty())
      sql_print_information(
          "POSIX_FADV_DONTNEED on file %s, range[%llu, %llu), length %llu",
          clean.filename_full.c_str(), clean.start_off,
          clean.start_off + clean.len, clean.len);
    else
      sql_print_information(
          "POSIX_FADV_DONTNEED on file fd %d, range[%llu, %llu), length %llu",
          fd, clean.start_off, clean.start_off + clean.len, clean.len);
  });

#endif /* POSIX_FADV_DONTNEED */
  if (opened) {
    ::close(fd);
  }
}

void page_cache_wake_up_worker() {
  mysql_mutex_lock(&LOCK_page_cache_cleaning);
  mysql_cond_signal(&COND_page_cache_cleaning);
  mysql_mutex_unlock(&LOCK_page_cache_cleaning);
}
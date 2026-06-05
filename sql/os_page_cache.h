#ifndef OS_PAGE_CACHE_INCLUDED
#define OS_PAGE_CACHE_INCLUDED
#include "include/mysql/psi/mysql_cond.h"
#include "include/mysql/psi/mysql_mutex.h"
#include "my_thread.h"
#include "mysql/psi/mysql_file.h"

extern mysql_mutex_t LOCK_page_cache_cleaning;
extern mysql_cond_t COND_page_cache_cleaning;
extern PSI_memory_key key_memory_page_cache_clean;

extern void page_cache_post_cleaning_work(const char *filename_full,
                                          my_off_t start_off, my_off_t len);

extern void page_cache_post_cleaning_work(File fd, my_off_t start_off,
                                          my_off_t len);

extern void page_cache_process_cleaning_work();

extern void page_cache_wake_up_worker();

extern void page_cache_cleanup();
#endif /* OS_PAGE_CACHE_INCLUDED */

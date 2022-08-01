#ifndef PX_INCLUDED
#define PX_INCLUDED

/**
  @file sql/parallel_execution/px.h

  PX internal interface.
 */

// PSI_*_key
#include "mysql/components/services/bits/mysql_cond_bits.h"
#include "mysql/components/services/bits/psi_memory_bits.h"
#include "mysql/components/services/bits/psi_mutex_bits.h"
#include "mysql/components/services/bits/psi_stage_bits.h"
#include "mysql/components/services/bits/psi_thread_bits.h"

// mysql_*_t
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"

#include <atomic> // fetch_add

/**
  Get parallel thread context, PX_executor, for given THD pointer.

  A parallel thread context provides access to the topology of all parallel
  executors for a given statement as well as lower-level thread context.

  For sequential execution, the context is nullptr.
 */
#define PX_EXECUTOR(thd) (thd)->px_executor

#ifndef DBUG_OFF

/*
  These macros provide convenient access to information for debugging purpose.
  They work only when THD::px_executor is set.
 */

#define PX_QUERY_ID \
    (PX_EXECUTOR(current_thd) ? PX_EXECUTOR(current_thd)->query_id() : 0)

#define PX_COORDINATOR_ID \
    (PX_EXECUTOR(current_thd) ? \
         PX_EXECUTOR(current_thd)->coordinator()->thread_id() : 0)

#define PX_WORKER_ID \
    (PX_EXECUTOR(current_thd) ? PX_EXECUTOR(current_thd)->thread_id() : 0)

#define PX_TEXT(fmt, ...) \
    ("px:%ld:%u:%u: " fmt, PX_QUERY_ID, PX_COORDINATOR_ID, PX_WORKER_ID, \
     ##__VA_ARGS__)

/*
  These macros are built on the DBUG package (dbug.cc), thus under the control
  of the dbug sysvar:

    set global debug='+d,pinfo,perror:N';

  Note that parallel workers do not have full support for DBUG. It is a hack to
  set the global variable. See also Sys_var_dbug.
 */
#define PX_PRINT_ERROR(fmt, ...) DBUG_PRINT("perror", PX_TEXT(fmt, ##__VA_ARGS__))
#define PX_PRINT_WARN(fmt, ...) DBUG_PRINT("pwarn", PX_TEXT(fmt, ##__VA_ARGS__))
#define PX_PRINT_INFO(fmt, ...) DBUG_PRINT("pinfo", PX_TEXT(fmt, ##__VA_ARGS__))
#define PX_PRINT_DEBUG(fmt, ...) DBUG_PRINT("pdebug", PX_TEXT(fmt, ##__VA_ARGS__))

#else

#define PX_PRINT_ERROR(fmt, ...)
#define PX_PRINT_WARN(fmt, ...)
#define PX_PRINT_INFO(fmt, ...)
#define PX_PRINT_DEBUG(fmt, ...)

#endif // DBUG_OFF

class THD;
struct TABLE;
struct TABLE_REF;
class QEP_TAB;
class Item;
template <class Element_type>
class mem_root_deque;
struct MEM_ROOT;

extern PSI_mutex_key key_px_thd_lock;
extern PSI_cond_key key_px_thd_cond;

extern PSI_mutex_key key_px_mq_lock;
extern PSI_memory_key key_px_mq_memory;

extern ulong txsql_parallel_threads_currently_used;
extern ulong txsql_parallel_stmt_executed;
extern ulong txsql_parallel_stmt_fallback;
extern ulong txsql_parallel_stmt_error;
extern ulong txsql_parallel_stmt_thread_refused;
extern ulong txsql_parallel_stmt_hint_executed;
extern ulong txsql_parallel_stmt_memory_refused;

extern mysql_mutex_t LOCK_allocate_resource;
extern mysql_mutex_t LOCK_inc_txsql_parallel_stmt_executed;
extern mysql_mutex_t LOCK_inc_txsql_parallel_stmt_fallback;
extern mysql_mutex_t LOCK_inc_txsql_parallel_stmt_error;
extern mysql_mutex_t LOCK_inc_txsql_parallel_stmt_thread_refused;
extern mysql_mutex_t LOCK_inc_txsql_parallel_stmt_hint_executed;
extern mysql_mutex_t LOCK_inc_txsql_parallel_stmt_memory_refused;

extern uint rehash_for_px(mem_root_deque<Item *> *key);

typedef void *(*malloc_func_t)(size_t size);
typedef void (*free_func_t)(void *);

typedef size_t Size;

typedef mysql_mutex_t spin_lock_t;

#define SpinLockInit(key, lock) mysql_mutex_init(key, lock, MY_MUTEX_INIT_FAST)
#define SpinLockAcquire(lock) mysql_mutex_lock(lock)
#define SpinLockRelease(lock) mysql_mutex_unlock(lock)
#define SpinLockFree(lock) mysql_mutex_destroy(lock)

/* Notify given thread. */
#define SetLatch(proc) (proc)->notify()

/* Wait for notification. */
#define WaitLatch(proc, timeout) \
  (proc)->wait((timeout), nullptr, __func__, __FILE__, __LINE__)

/* Reset event state. No need for cond var. */
#define ResetLatch(proc)

class PX_mem_allocator {
 public:
  PX_mem_allocator() {}
  virtual ~PX_mem_allocator() {}
  virtual MEM_ROOT *get_mem_allocator() = 0;
};

/**
  The status of a backend thread
*/
enum PX_handle_status { NOT_YET_STARTED = 0, STARTED, KILLED };

/**
  This class represents a counterpart handle of the message queue handle.
*/
class PX_worker_handle {
 public:
  PX_worker_handle(uint id) : m_id(id) {}
  virtual ~PX_worker_handle() {}

  // FIXME: the worker thread may have switched to the next task.
  virtual PX_handle_status check_worker_status() = 0;

  uint id() const { return m_id; }

 private:
  uint m_id;
};

/**
  Each backend thread of message queue has a PX_proc object.
*/
class PX_proc {
 public:
  PX_proc(THD *thd);
  ~PX_proc();

  bool is_killed() const;

  /// Notify this thread.
  void notify();

  /// Wait for notification.
  void wait(ulong timeout, const PSI_stage_info *stage, const char *src_func,
            const char *src_file, uint src_line);

  PX_handle_status check_status();
 private:
  /// The backend thread
  THD *m_thd;
  mysql_mutex_t m_thd_lock;
  mysql_cond_t m_thd_cond;
  bool m_notified;
};

enum PX_SCAN_TYPE {
  PX_TABLE_SCAN,
  PX_INDEX_SCAN,
  PX_RANGE_SCAN,
  PX_REF_SCAN,
  PX_DEPEND_REF_SCAN,
  PX_INVALID_SCAN
};
class PX_table_descriptor {
 public:
  PX_table_descriptor(TABLE *table, PX_SCAN_TYPE type,
                      uint keyno, TABLE_REF *ref,
                      bool reverse_scan) :
      m_table(table),
      m_type(type),
      m_keyno(keyno),
      m_ref(ref),
      m_reverse_scan(reverse_scan) {}

  TABLE *table() { return m_table; }
  PX_SCAN_TYPE type() { return m_type; }
  uint keyno() { return m_keyno; }
  TABLE_REF *ref() { return m_ref; }
  bool reverse_scan() { return m_reverse_scan; }

 private:
  TABLE *m_table{nullptr};
  PX_SCAN_TYPE m_type{PX_INVALID_SCAN};
  uint m_keyno{UINT_MAX};
  TABLE_REF *m_ref{nullptr};
  bool m_reverse_scan{false};
};

#endif

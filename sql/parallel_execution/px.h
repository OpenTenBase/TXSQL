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

class THD;

extern PSI_mutex_key key_px_thd_lock;
extern PSI_cond_key key_px_thd_cond;

extern PSI_mutex_key key_px_mq_lock;
extern PSI_memory_key key_px_mq_memory;

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

/**
  The status of a backend thread
*/
enum PX_handle_status { NOT_YET_STARTED = 0, STARTED, KILLED };

/**
  This class represents a counterpart handle of the message queue handle.
*/
class PX_worker_handle {
 public:
  PX_worker_handle(THD *thd) : m_thd(thd) {}
  ~PX_worker_handle() {}

  // FIXME: the worker thread may have switched to the next task.
  virtual PX_handle_status check_worker_status() = 0;

  THD *thd() { return m_thd; }

 private:
  /// The thread of the counterpart of the message queue handle
  THD *m_thd;
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

 private:
  /// The backend thread
  THD *m_thd;
  mysql_mutex_t m_thd_lock;
  mysql_cond_t m_thd_cond;
  bool m_notified;
};

#endif

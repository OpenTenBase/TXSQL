/**
  @file sql/parallel_execution/px.cc

  PX implemenation.
*/

#include "px.h"

#include "my_psi_config.h"           // HAVE_PSI_INTERFACE
#include "mysql/psi/mysql_cond.h"    // mysql_cond_register
#include "mysql/psi/mysql_memory.h"  // mysql_memory_register
#include "mysql/psi/mysql_mutex.h"   // mysql_mutex_register
#include "mysql/psi/mysql_thread.h"  // mysql_thread_register
#include "template_utils.h"          // array_elements
#include "sql/table.h"               // TABLE
#include "sql/sql_opt_exec_shared.h" // TABLE_REF

#include "sql/sql_class.h"  // THD

PSI_mutex_key key_px_thd_lock;
PSI_cond_key key_px_thd_cond;

PSI_mutex_key key_px_mq_lock;
PSI_memory_key key_px_mq_memory;

/// The maximum number of parallel workers to use for parallel execution.
unsigned long px_max_parallel_threads;
/// Force fallback in execution phase. It is for testing purpose.
bool px_fallback_in_execution = false;

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_info all_px_mutexes[] = {
    {&key_px_thd_lock, "PX::LOCK_thd", 0, 0, PSI_DOCUMENT_ME},
    {&key_px_mq_lock, "PX::LOCK_mq", 0, 0, PSI_DOCUMENT_ME},
};

static PSI_cond_info all_px_conds[] = {
    {&key_px_thd_cond, "PX::COND_thd", 0, 0, PSI_DOCUMENT_ME},
};

#if 0
static PSI_thread_info all_px_threads[] = {
};
#endif

static PSI_memory_info all_px_memory[] = {
    {&key_px_mq_memory, "PX::mq", 0, 0, PSI_DOCUMENT_ME},
};

void px_init_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_px_mutexes));
  mysql_mutex_register(category, all_px_mutexes, count);

  count = static_cast<int>(array_elements(all_px_conds));
  mysql_cond_register(category, all_px_conds, count);

  count = static_cast<int>(array_elements(all_px_memory));
  mysql_memory_register(category, all_px_memory, count);

#if 0
  count = static_cast<int>(array_elements(all_px_threads));
  mysql_thread_register(category, all_px_threads, count);
#endif
}

#else
void px_init_psi_keys(void) {}
#endif /* HAVE_PSI_INTERFACE */

PX_proc::PX_proc(THD *thd) : m_thd(thd), m_notified() {
  mysql_mutex_init(key_px_thd_lock, &m_thd_lock, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_px_thd_cond, &m_thd_cond);
}

PX_proc::~PX_proc() {
  mysql_mutex_destroy(&m_thd_lock);
  mysql_cond_destroy(&m_thd_cond);
}

bool PX_proc::is_killed() const { return m_thd && m_thd->is_killed(); }

void PX_proc::notify() {
  assert(m_thd);
  mysql_mutex_lock(&m_thd_lock);
  if (!m_notified) {
    mysql_cond_signal(&m_thd_cond);
    m_notified = true;
  }
  mysql_mutex_unlock(&m_thd_lock);
}

void PX_proc::wait(ulong timeout, const PSI_stage_info *stage,
                   const char *src_func, const char *src_file, uint src_line) {
  assert(m_thd);
  PSI_stage_info old_stage;

  bool actual_wait = false;

  struct timespec wait_timeout;
  set_timespec(&wait_timeout, timeout);

  mysql_mutex_lock(&m_thd_lock);

  if (!m_notified) {
    actual_wait = true;
    m_thd->enter_cond(&m_thd_cond, &m_thd_lock, stage, &old_stage, src_func,
                      src_file, src_line);

    // Don't care about timeout.
    mysql_cond_timedwait(&m_thd_cond, &m_thd_lock, &wait_timeout);
  }

  m_notified = false;

  mysql_mutex_unlock(&m_thd_lock);

  if (actual_wait) {
    m_thd->exit_cond(stage, src_func, src_file, src_line);
  }
}

PX_handle_status PX_proc::check_status() {
  return is_killed() ? KILLED : STARTED;
}

bool px_partition(uint dop, void *&scan_ctx, TABLE *table, PX_SCAN_TYPE type,
                  uint keyno, TABLE_REF *ref, bool reverse_scan, uint &partitions) {
  assert(table);
  int error = 0;
  table->file->px_scan_type = type;
  if (ref) {
    assert(type == PX_REF_SCAN);
    table->file->px_ref_key.key = ref->key_buff;
    table->file->px_ref_key.keypart_map = make_prev_keypart_map(ref->key_parts);
    table->file->px_ref_key.length = ref->key_length;
    table->file->px_ref_key.flag = HA_READ_KEY_OR_NEXT;
  }

  error = table->file->ha_px_coordinator_init(dop, keyno, scan_ctx, partitions, reverse_scan);
  if (error) {
    table->file->print_error(error, MYF(0));
  }

  return error;
}

/**
  @file sql/parallel_execution/px.cc

  PX implemenation.
*/

#include "px.h"

#include <mysql/components/services/log_builtins.h>

#include "my_psi_config.h"           // HAVE_PSI_INTERFACE
#include "mysql/psi/mysql_cond.h"    // mysql_cond_register
#include "mysql/psi/mysql_memory.h"  // mysql_memory_register
#include "mysql/psi/mysql_mutex.h"   // mysql_mutex_register
#include "mysql/psi/mysql_thread.h"  // mysql_thread_register
#include "template_utils.h"          // array_elements
#include "sql/table.h"               // TABLE
#include "sql/sql_opt_exec_shared.h" // TABLE_REF
#include "sql/sql_optimizer.h"       // JOIN

#include "sql/sql_class.h"  // THD

#include "sql/parallel_execution/px_resource_mgr.h" // PX_resource_manager

PSI_mutex_key key_px_thd_lock;
PSI_cond_key key_px_thd_cond;

PSI_mutex_key key_px_mq_lock;
PSI_memory_key key_px_mq_memory;

PSI_mutex_key key_px_worker_lock;
PSI_cond_key key_px_worker_cond;

static PSI_mutex_key key_LOCK_allocate_resource;
static PSI_mutex_key key_LOCK_inc_txsql_parallel_stmt_executed;
static PSI_mutex_key key_LOCK_inc_txsql_parallel_stmt_fallback;
static PSI_mutex_key key_LOCK_inc_txsql_parallel_stmt_error;
static PSI_mutex_key key_LOCK_inc_px_threads_refused;
static PSI_mutex_key key_LOCK_inc_px_hint_stmt_executed;

/**
  The lock which used by parallel execution.
*/
mysql_mutex_t LOCK_allocate_resource;
mysql_mutex_t LOCK_inc_txsql_parallel_stmt_executed;
mysql_mutex_t LOCK_inc_txsql_parallel_stmt_fallback;
mysql_mutex_t LOCK_inc_txsql_parallel_stmt_error;
mysql_mutex_t LOCK_inc_txsql_parallel_stmt_thread_refused;
mysql_mutex_t LOCK_inc_txsql_parallel_stmt_hint_executed;

/**
  Current statements of parallel execution.
*/
ulong txsql_parallel_threads_currently_used = 0;
ulong txsql_parallel_stmt_executed = 0;
ulong txsql_parallel_stmt_fallback = 0;
ulong txsql_parallel_stmt_error = 0;
ulong txsql_parallel_stmt_thread_refused = 0;
ulong txsql_parallel_stmt_hint_executed = 0;

/// The maximum number of parallel workers to use for parallel execution.
unsigned long txsql_max_parallel_worker_threads;
/// Force fallback in execution phase. It is for testing purpose.
bool txsql_parallel_fallback_in_execution = false;
/// Forcibly disable parallel execution on the server.
bool txsql_parallel_execution_enabled = true;

static void px_init_psi_keys(void);

static bool px_initialized = false;

/**
  Initialize the parallel execution.

  Called at server startup, say to initialize mutexes and condition variables.
*/
bool px_init(void) {
  assert(!px_initialized);
  px_initialized = true;

#ifdef HAVE_PSI_INTERFACE
  px_init_psi_keys();
#endif

  mysql_mutex_init(key_LOCK_allocate_resource, &LOCK_allocate_resource,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_inc_txsql_parallel_stmt_executed, &LOCK_inc_txsql_parallel_stmt_executed,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_inc_txsql_parallel_stmt_fallback, &LOCK_inc_txsql_parallel_stmt_fallback,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_inc_txsql_parallel_stmt_error, &LOCK_inc_txsql_parallel_stmt_error,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_inc_px_threads_refused, &LOCK_inc_txsql_parallel_stmt_thread_refused,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_inc_px_hint_stmt_executed, &LOCK_inc_txsql_parallel_stmt_hint_executed,
                   MY_MUTEX_INIT_FAST);

  if (PX_resource_manager::init_instance()) {
    LogErr(ERROR_LEVEL, ER_PX_THREAD_HANDLING_OOM);
    return true;
  }

  return false;
}

/**
  Release resources of parallel execution.

  Called at server shutdown. Destroys mutexes and condition variables.
 */
void px_destroy(void) {
  // Avoid uninitialized state on error shutdown
  if (!px_initialized) return;

  PX_resource_manager::destroy_instance();

  mysql_mutex_destroy(&LOCK_allocate_resource);
  mysql_mutex_destroy(&LOCK_inc_txsql_parallel_stmt_executed);
  mysql_mutex_destroy(&LOCK_inc_txsql_parallel_stmt_fallback);
  mysql_mutex_destroy(&LOCK_inc_txsql_parallel_stmt_error);
  mysql_mutex_destroy(&LOCK_inc_txsql_parallel_stmt_thread_refused);
  mysql_mutex_destroy(&LOCK_inc_txsql_parallel_stmt_hint_executed);
}

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_info all_px_mutexes[] = {
    {&key_px_thd_lock, "PX::LOCK_thd", 0, 0, PSI_DOCUMENT_ME},
    {&key_px_mq_lock, "PX::LOCK_mq", 0, 0, PSI_DOCUMENT_ME},
    {&key_LOCK_allocate_resource, "LOCK_allocate_resource", PSI_FLAG_SINGLETON,
     0, PSI_DOCUMENT_ME},
    {&key_LOCK_inc_txsql_parallel_stmt_executed, "LOCK_inc_txsql_parallel_stmt_executed",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_LOCK_inc_txsql_parallel_stmt_fallback, "LOCK_inc_txsql_parallel_stmt_fallback",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_LOCK_inc_txsql_parallel_stmt_error, "LOCK_inc_txsql_parallel_stmt_error", PSI_FLAG_SINGLETON,
     0, PSI_DOCUMENT_ME},
    {&key_LOCK_inc_px_threads_refused, "LOCK_inc_txsql_parallel_stmt_thread_refused",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_LOCK_inc_px_hint_stmt_executed, "LOCK_inc_txsql_parallel_stmt_hint_executed",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_px_worker_lock, "cond_with_lock_t::LOCK_worker", 0, 0, PSI_DOCUMENT_ME}
};

static PSI_cond_info all_px_conds[] = {
    {&key_px_thd_cond, "PX::COND_thd", 0, 0, PSI_DOCUMENT_ME},
    {&key_px_worker_cond, "cond_with_lock_t::COND_worker", 0, 0, PSI_DOCUMENT_ME}
};

#if 0
static PSI_thread_info all_px_threads[] = {
};
#endif

static PSI_memory_info all_px_memory[] = {
    {&key_px_mq_memory, "PX::mq", 0, 0, PSI_DOCUMENT_ME},
};

static void px_init_psi_keys(void) {
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
static void px_init_psi_keys(void) {}
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

int show_txsql_parallel_stmt_executed(THD *, SHOW_VAR *var, char *buff) {
  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (long)txsql_parallel_stmt_executed;
  return 0;
}

int show_txsql_parallel_stmt_fallback(THD *, SHOW_VAR *var, char *buff) {
  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (long)txsql_parallel_stmt_fallback;
  return 0;
}

int show_txsql_parallel_stmt_error(THD *, SHOW_VAR *var, char *buff) {
  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (long)txsql_parallel_stmt_error;
  return 0;
}

int show_txsql_parallel_threads_currently_used(THD *, SHOW_VAR *var, char *buff) {
  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (long)txsql_parallel_threads_currently_used;
  return 0;
}

int show_txsql_parallel_stmt_thread_refused(THD *, SHOW_VAR *var, char *buff) {
  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (long)txsql_parallel_stmt_thread_refused;
  return 0;
}

int show_txsql_parallel_stmt_hint_executed(THD *, SHOW_VAR *var, char *buff) {
  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (long)txsql_parallel_stmt_hint_executed;
  return 0;
}

void reset_txsql_parallel_stmt_executed()
{
  mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_executed);
  txsql_parallel_stmt_executed = 0;
  mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_executed);
}

void reset_txsql_parallel_stmt_fallback()
{
  mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_fallback);
  txsql_parallel_stmt_fallback = 0;
  mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_fallback);
}

void reset_txsql_parallel_stmt_error()
{
  mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_error);
  txsql_parallel_stmt_error = 0;
  mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_error);
}

void reset_txsql_parallel_stmt_thread_refused()
{
  mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_thread_refused);
  txsql_parallel_stmt_thread_refused = 0;
  mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_thread_refused);
}

void reset_txsql_parallel_stmt_hint_executed()
{
  mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_hint_executed);
  txsql_parallel_stmt_hint_executed = 0;
  mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_hint_executed);
}

/**
  Check if there is a QEP_TAB matching TABLE in the Query block.

  @return qep_tab exists, nullptr otherwise.
*/
QEP_TAB *get_matched_tab(JOIN *join, TABLE *table) {
  assert(join && table);
  for (uint i = 0; i < join->tables; ++i) {
    if (join->qep_tab[i].table() == table) {
      return &join->qep_tab[i];
    }
  }
  return nullptr;
}
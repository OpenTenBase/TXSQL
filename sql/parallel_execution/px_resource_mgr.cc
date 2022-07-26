#include "sql/parallel_execution/px_resource_mgr.h"
#include "sql/parallel_execution/px_interface.h"
#include "sql/parallel_execution/px.h"
#include "mysql/psi/mysql_thread.h"  // mysql_mutex_init

PX_resource_manager *PX_resource_manager::m_instance = nullptr;

bool PX_resource_manager::acquire(int64_t cpu_cores)
{
  bool enough = false;
  mysql_mutex_lock(&LOCK_allocate_resource);
  if (txsql_parallel_threads_currently_used + cpu_cores <= txsql_max_parallel_worker_threads) {
    txsql_parallel_threads_currently_used += cpu_cores;
    enough = true;
  } else {
    mysql_mutex_lock(&LOCK_inc_txsql_parallel_stmt_thread_refused);
    txsql_parallel_stmt_thread_refused++;
    mysql_mutex_unlock(&LOCK_inc_txsql_parallel_stmt_thread_refused);
  }
  mysql_mutex_unlock(&LOCK_allocate_resource);
  return enough;
}

void PX_resource_manager::release(int64_t cpu_cores)
{
  mysql_mutex_lock(&LOCK_allocate_resource);
  txsql_parallel_threads_currently_used -= cpu_cores;
  mysql_mutex_unlock(&LOCK_allocate_resource);
}

bool PX_resource_manager::init_instance()
{
  m_instance = new (std::nothrow) PX_resource_manager();
  if (m_instance == nullptr) {
    delete m_instance;
    return true;
  }
  return false;
}

void PX_resource_manager::destroy_instance() {
  if (m_instance != nullptr) {
    delete m_instance;
    m_instance = nullptr;
  }
}

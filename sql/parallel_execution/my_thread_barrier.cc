#include "my_thread_barrier.h"

#include "mutex_lock.h"
#include "my_inttypes.h"
#include "mysql/psi/mysql_cond.h"
#include "thr_mutex.h"

my_thread_barrier::my_thread_barrier(int n)
{
  m_cnt=n;
  bool fail1 = (pthread_mutex_init(&m_mutex,0)==0);
  assert(fail1);
  bool fail2 = (pthread_cond_init(&m_cond,0)==0);
  assert(fail2);
  m_counter=0;
}

my_thread_barrier::~my_thread_barrier()
{
  pthread_mutex_destroy(&m_mutex);
  pthread_cond_destroy(&m_cond);
}

void my_thread_barrier::arrive()
{
  pthread_mutex_lock(&m_mutex);
  m_counter++;
  if(m_counter < m_cnt)
    pthread_cond_wait(&m_cond,&m_mutex);
  else
    pthread_cond_broadcast(&m_cond);
  pthread_mutex_unlock(&m_mutex);
}
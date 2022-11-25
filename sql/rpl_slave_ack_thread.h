/*
 * rpl_slave_ack_thread.h
 *
 *  Created on: 2021/4/22
 *      Author: harlylei
 */

#ifndef SQL_RPL_SLAVE_ACK_THREAD_H_
#define SQL_RPL_SLAVE_ACK_THREAD_H_

#include <thread>

#include "sql/lockfree_mwsr_queue_with_notify.h"
#include "sql_class.h"
#include "log.h"

class rpl_slave_ack_thread {
public:
rpl_slave_ack_thread() :
  m_queue(4096, m_exit) {

  }

  ~rpl_slave_ack_thread() {
    stop();
  }

  void stop() {
    if (!m_exit.load()) {
      m_exit.store(true);
      m_queue.notify_all();
      if (m_thread.joinable()) {
        m_thread.join();
      }
    }
  }

  void start_thread() {
    std::thread t(&rpl_slave_ack_thread::run, this);
    m_thread = move(t);
  }

  bool push(const char *binlog, my_off_t pos) {
    Thd_Trans_binlog_info info;
    info.set(binlog, pos);
    return m_queue.push(info);
  }

  bool queue_is_empty_and_thread_is_waiting() {
    return m_queue.queue_is_empty_and_thread_is_waiting();
  }

  inline void set_fail_next_check(bool fail) {
    m_next_check_fail.store(fail);
  }

  inline bool is_fail_when_check() const{
    return m_next_check_fail.load();
  }

#if !defined(DBUG_OFF)
  void debug_stop_check();
#endif

private:
  std::thread m_thread;
  std::atomic_bool m_exit { false };
  std::atomic_bool m_next_check_fail { false };//whether fail when next check
  lockfree_mwsr_queue_with_notify<Thd_Trans_binlog_info> m_queue;

  void run();
  void flush_and_ack(const Thd_Trans_binlog_info &binlog_info);
};

#endif /* SQL_RPL_SLAVE_ACK_THREAD_H_ */

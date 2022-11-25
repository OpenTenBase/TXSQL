/*
 * lockfree_queue_with_notify.h
 *
 *  Created on: 2021/4/22
 *      Author: harlylei
 */

#ifndef SQL_LOCKFREE_MWSR_QUEUE_WITH_NOTIFY_H_
#define SQL_LOCKFREE_MWSR_QUEUE_WITH_NOTIFY_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/lockfree/queue.hpp>

#include "my_compiler.h"
#include "log.h"

/*
 * multile write thread
 * single read thread
 * with nodify queue
 */
template<typename T>
class lockfree_mwsr_queue_with_notify {

public:

lockfree_mwsr_queue_with_notify(size_t size, std::atomic_bool &can_exit) :
  m_exit(can_exit), m_queue(size) {
  }

  inline bool push(const T &t) {
    bool ret = m_queue.push(t);
    if (unlikely(!ret)) {
      return false;
    }
    //sleep 3; //rara case,when check m_waiting is true ,but m_queue may be empty(by consummer thread consumer)

    if (unlikely(m_waiting.load())) {

      std::unique_lock<std::mutex> lck(m_mutex);
      if (!m_queue.empty()) { //m_queue has some nodes,notify once
        m_waiting.store(false);
        m_cond.notify_one();
      }
    }

    return ret;
  }

  inline bool pop(T &t) {
    T tmp;
    bool ret = m_queue.pop(tmp); //It's invasive,so we use tmp,when pop success,we copy to t
    if (ret) {
      t = tmp;
    }
    return ret;
  }

  inline bool loop_pop(T &t, int msec_per) {
    bool ret = pop(t);
    if (ret) {
      return true;
    }
    return wait_pop(t, msec_per);
  }

  void notify_all() {
    std::unique_lock<std::mutex> lck(m_mutex);
    m_cond.notify_all();
  }

  //check the queue is empty and this thread is not working
  //when the ret is true, the check thread can do something safely(dont' conflict with queue thread)
  bool queue_is_empty_and_thread_is_waiting() {

    bool ret = m_queue.empty();
    if (!ret) { //queue is not empty,so return false
      return false;
    }
    std::unique_lock<std::mutex> lck(m_mutex); //lock
    ret = m_queue.empty();//double check
    if (!ret) { //queue is not empty,so return false
      return false;
    }
    //now queue is empty,check thread
    return m_waiting.load();
  }

private:
  std::mutex m_mutex;
  std::condition_variable m_cond;

  std::atomic_bool m_waiting { false }; //whether the consumer thread is waiting.
  std::atomic_bool &m_exit;

  boost::lockfree::queue<T, boost::lockfree::fixed_sized<true>> m_queue;

  inline bool wait_pop(T &t, int msec_per) {
    assert(!m_waiting.load());
    if (msec_per <= 0) {
      msec_per = 3;
    }

    bool ret = false;

    //store m_waiting is must before check pop
    //it is better to store which before lock mutex,in this case,push thread may can lock before ,Reduce the probability of the push thread switching
    m_waiting.store(true);
    std::unique_lock<std::mutex> lck(m_mutex);
    do {
      ret = pop(t);
      if (ret) {
        break;
      }
      auto const timeout = std::chrono::system_clock::now()
          + std::chrono::milliseconds(msec_per);
      m_cond.wait_until(lck, timeout);

    } while (!m_exit.load());

    m_waiting.store(false);

    return ret;
  }
};

#endif /* SQL_LOCKFREE_MWSR_QUEUE_WITH_NOTIFY_H_ */

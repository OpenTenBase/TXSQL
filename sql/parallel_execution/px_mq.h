#ifndef PX_MQ_INCLUDED
#define PX_MQ_INCLUDED

/**
  @file sql/parallel_execution/px_mq.h

  A single producer single consumer message queue.
*/

#include "px.h"

/// Scatter input buffer, resembling writev(2) system call interface.
struct PX_iovec {
  const char *data;
  Size len;
};

/**
  Possible results of a send or receive operation.
*/
enum PX_mq_result {
  /// Sent or received a message
  PX_MQ_SUCCESS,
  /// Not completed; retry later
  PX_MQ_WOULD_BLOCK,
  /// Other thread has detached queue
  PX_MQ_DETACHED,
  /// Error return for my_error()
  PX_MQ_ERROR,
  /// Received kill signal
  PX_MQ_INTERRUPTED
};

class PX_mq {
 public:
  PX_mq(size_t ring_size, char *ring_buffer);
  ~PX_mq();

  void set_receiver(PX_proc *proc);
  void set_sender(PX_proc *proc);

  PX_proc *get_receiver();
  PX_proc *get_sender();

 private:
  friend class PX_mq_handle;

  /// Notify counterparty that we're detaching from message queue
  void detach(PX_proc *me);

  spin_lock_t m_mutex;
  PX_proc *m_receiver;
  PX_proc *m_sender;
  uint64 m_bytes_read;
  uint64 m_bytes_written;
  Size m_ring_size;
  bool m_detached;
  char *m_ring_buffer;
};

class PX_mq_handle {
 public:
  PX_mq_handle(PX_mq *queue, PX_proc *me, malloc_func_t malloc_func,
               free_func_t free_func)
      : m_queue(queue),
        m_me(me),
        m_malloc_func(malloc_func),
        m_free_func(free_func) {}

  ~PX_mq_handle() {}

  void set_worker_handle(PX_worker_handle *handle) {
    assert(m_handle == nullptr);
    m_handle = handle;
  }

  /// Detach from the message queue
  void detach();

  PX_mq *get_queue() { return m_queue; }

  /// Send message to a shared message queue.
  PX_mq_result send(Size nbytes, const void *datap, bool nowait);
  /// Send message to a shared message queue.
  PX_mq_result sendv(PX_iovec *iov, int iovcnt, bool nowait);
  /// Receive a message from a shared message queue.
  PX_mq_result receive(Size *nbytesp, void **datap, bool nowait);

 private:
  /// Write bytes into a shared message queue
  PX_mq_result send_bytes(Size nbytes, const void *data, bool nowait,
                          Size *bytes_written);
  /// Receive bytes_needed from message queue
  PX_mq_result receive_bytes(Size bytes_needed, bool nowait, Size *nbytesp,
                             void **datap);
  /// Increment the number of bytes read in atomic mode
  void atomic_inc_bytes_read(int n);
  /// Increment the number of bytes written in atomic mode
  void atomic_inc_bytes_written(int n);
  /// Check the counterpart status
  bool mq_counterparty_gone(PX_worker_handle *handle);
  /// Waiting for its counterpart to attach to the message queue
  PX_mq_result mq_wait_internal(PX_proc **ptr, PX_worker_handle *handle);

  PX_mq *m_queue;
  PX_worker_handle *m_handle{nullptr};
  char *m_buffer{nullptr};
  Size m_buflen{0};
  Size m_consume_pending{0};
  Size m_partial_bytes{0};
  Size m_expected_bytes{0};
  bool m_length_word_complete{false};
  bool m_counterparty_attached{false};

  PX_proc *m_me;

  malloc_func_t m_malloc_func;
  free_func_t m_free_func;
};

#endif

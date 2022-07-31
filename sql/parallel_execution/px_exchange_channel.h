
#ifndef PX_EXCHANGE_CHANNEL_INCLUDED
#define PX_EXCHANGE_CHANNEL_INCLUDED

#include "mem_root_deque.h"
#include "my_base.h"
#include "px.h"
#include "px_mq.h"

class Item;
class PX_mq;
class PX_worker_handle;
class PX_proc;
class PX_mq_handle;
class THD;
struct PX_iovec;

enum PX_channel_type {
  PX_INVALID_CHANNEL = 0,
  PX_MQ_CHANNEL
};

enum PX_io_error {
  PX_IO_OK = 0,
  PX_IO_EOF = -1,
  PX_IO_WOULD_BLOCK = -2,
  PX_IO_ERROR= 1
};

class PX_exchange_handle;

/**
  Abstract class for the data channel between a producer and a consumer.

  The implementation is dependent on the placement of the communication endpoints.
  For example, message queue might be used when both endpoints are in the
  same process.
*/
class PX_exchange_channel {
 public:
  PX_exchange_channel(uint id) : m_id(id) {}
  virtual ~PX_exchange_channel() {}

  virtual PX_channel_type type() = 0;

  virtual void set_sender(PX_proc *sender) = 0;

  virtual void set_receiver(PX_proc *receiver) = 0;

  virtual bool attach(PX_proc *me, PX_exchange_handle *&handle) = 0;

  uint id() const { return m_id; }

 private:
  uint m_id;
};

/**
  Abstract class for one side of a data channel.
 */
class PX_exchange_handle {
 public:
  PX_exchange_handle(uint channel_id) : m_channel_id(channel_id) {}
  virtual ~PX_exchange_handle() {}
  virtual PX_io_error send(PX_iovec *iov, int iovcnt, bool nowait) = 0;
  virtual PX_io_error receive(void **datap, Size *len, bool nowait) = 0;
  virtual void set_worker_handle(PX_worker_handle *worker_handle) = 0;
  virtual void detach() = 0;
  uint channel_id() const { return m_channel_id; }

  void set_detached() { m_detached = true; }
  bool get_detached() { return m_detached; }
 private:
  uint m_channel_id;
  bool m_detached{false};
};

/**
  Data channel backed by message queue (PX_mq).
*/
class PX_mq_channel : public PX_exchange_channel {
 public:
  PX_mq_channel(uint channel_no, char *ring_buffer, Size size);
  ~PX_mq_channel();

  PX_channel_type type() override { return PX_MQ_CHANNEL; }

  void set_sender(PX_proc *sender) override;

  void set_receiver(PX_proc *receiver) override;

  bool attach(PX_proc *me, PX_exchange_handle *&handle) override;

 private:
  // The message queue
  PX_mq m_mq;
};

#endif

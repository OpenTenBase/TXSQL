
#ifndef PX_EXCHANGE_CHANNEL_INCLUDED
#define PX_EXCHANGE_CHANNEL_INCLUDED

#include "mem_root_deque.h"
#include "my_base.h"
#include "px.h"

class Item;
class PX_mq;
class PX_worker_handle;
class PX_proc;
class PX_mq_handle;
class THD;
struct PX_iovec;

/**
  The struct represents the compact data of a field.
  For null field and basic_const_item,set the
  m_need_send = false, indicates that we won't
  send it to message queue. For var-length field,
  only send the compact data.
*/
struct PX_field_data {
  uchar *m_ptr{nullptr};
  uint32 m_len{0};
  bool m_need_send{true};
};

enum PX_channel_type {
  PX_INVALID_CHANNEL = 0,
  PX_MQ_CHANNEL
};

/**
  The class represents a exchange channel between
  a exchange sender and a exchange reciever.
  A channel can be realized by message queue and
  other methods for different system.
*/
class PX_exchange_channel {
 public:
  PX_exchange_channel(uint channel_no) : m_channel_no(channel_no) {}
  virtual ~PX_exchange_channel() {}

  /**
    Initial the chennel.

    @retval false Success
    @retval true  Error
  */
 	virtual bool init() = 0;

  virtual void clean() = 0;

  /**
    Send a mysql row data.

    @retval false Success
    @retval true  Error
  */
  virtual int send_row(PX_iovec *iov, int iovcnt, bool nowait) = 0;
  /**
    Recieve data from the channel.

    @retval false Success
    @retval true  Error
  */
  virtual int receive(void **datap, Size *len, bool nowait) = 0;

  virtual bool init_sender() = 0;

  virtual bool init_receiver() = 0;

  virtual void detach_sender() = 0;

  virtual void detach_receiver() = 0;

  virtual bool attach_sender() = 0;

  virtual bool attach_receiver() = 0;

  virtual void receiver_wait() = 0;

	virtual PX_channel_type type() { return PX_INVALID_CHANNEL; }

  virtual uint get_channel_no() { return m_channel_no; }

 private:
  // The no of the channel in the dfo
  uint m_channel_no;
};

/**
  PX_mq_channel represents a PX_exchange_channel realized
  by message queue.
*/
class PX_mq_channel : public PX_exchange_channel {
 public:
	PX_mq_channel(uint channel_no, Size size, THD *coordinator);
  ~PX_mq_channel();

  PX_channel_type type() override { return PX_MQ_CHANNEL; }
  
  /**
    Initial the message queue. Create a message queue in
    address.

    @retval false Success
    @retval true  Error
  */
	bool init() override;

  void clean() override;
  
  /**
     Send a mysql row to message queue.
  */
  int send_row(PX_iovec *iov, int iovcnt, bool nowait) override;
  
  /*
    Receive data from message queue.
    The parameter nowait indicates whether the receiver
    will be blocked when the message queue is empty.

    
  */
  int receive(void **datap, Size *len, bool nowait) override;

  /**
    Create the PX_mq_handle used in exchange sender backend.

    @retval false Success
    @retval true  Error    
  */
  bool init_sender() override;

  /**
    Create the PX_mq_handle used in exchange receiver backend.

    @retval false Success
    @retval true  Error    
  */
  bool init_receiver() override;

  void detach_sender() override;

  void detach_receiver() override;

  bool attach_sender() override;

  bool attach_receiver() override;

  void receiver_wait() override;

  void register_sender_handle(PX_worker_handle *handle);
  void register_receiver_handle(PX_worker_handle *handle);
  void register_sender_event(PX_proc *event);
  void register_reciever_event(PX_proc *event);

 private:
  // The message queue
  PX_mq *m_mq{nullptr};
  Size m_size{0};

  THD *m_coordinator_thd{nullptr};
  // Synchronize the sender and receiver through PX_proc
  PX_proc *m_sender_event{nullptr};
  PX_proc *m_receiver_event{nullptr};
  // Check the countpart status through PX_worker_handle
  PX_worker_handle *m_sender_handle{nullptr};
  PX_worker_handle *m_receiver_handle{nullptr};
  // Send or receive data through PX_mq_handle
  PX_mq_handle *m_sender{nullptr};
  PX_mq_handle *m_receiver{nullptr};
};

#endif
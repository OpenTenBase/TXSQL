#include "include/my_dbug.h"
#include "sql/sql_class.h"
#include "sql/mysqld.h"
#include "px_exchange_info.h"
#include "px_mq.h"
#include "px.h"
#include "sql/log.h"

// The default size of data queue is 1MB
#define RING_SIZE 1048576

class PX_worker_handle_impl : public PX_worker_handle {
 public:
  PX_worker_handle_impl(THD *thd) : PX_worker_handle(thd) {}
  PX_handle_status check_worker_status() override {
    // FIXME: return a reasonable status.
    return STARTED;
  }
};

PX_exchange_info::PX_exchange_info(THD *thd, PX_exchange_type exchange_type,
    PX_channel_type type, uint senders, uint receivers, PX_exchange_format format, bool need_materialize)
    : m_coordinator_thd(thd),
      m_channel_type(type),
      m_senders(senders),
      m_receivers(receivers),
      m_type(exchange_type),
      m_format(format),
      m_need_materialize(need_materialize) {
  mysql_mutex_init(key_LOCK_Exchange_Info_Channel, &m_lock, MY_MUTEX_INIT_FAST);
}

/**
  Create the exchange channels between two dfos, each sender and a receiver
  will connect to a channel.

  @return true if fail, false if success.
*/
bool PX_exchange_info::init() {
  assert(m_senders && m_receivers);
  uint nchannels = m_senders * m_receivers;
  m_channels.reserve(nchannels);

  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = nullptr;

    switch (m_channel_type) {
      case PX_MQ_CHANNEL: {
        channel = new (m_coordinator_thd->mem_root)
            PX_mq_channel(i, RING_SIZE, m_coordinator_thd);

        if (!channel) {
          my_error(ER_OUTOFMEMORY, MYF(0));
          sql_print_error("init Parallel execution exchange info error!");
          return true;
        }

        break;
      }
      default: {
        // not support yes!
        assert(0);
        return true;
        break;
      }
    }

    channel->init();
    m_channels.push_back(channel);
  }

  return false;
}

/**
  Register for a exchange receiver thread. 

  @return true if fail, false if success.
*/
bool PX_exchange_info::register_receiver(THD *receiver, uint receiver_no) {
  assert(receiver_no < m_receivers);
  PX_proc *event = nullptr;
  PX_worker_handle *handle = nullptr;

  switch (m_channel_type) {
   case PX_MQ_CHANNEL: {
     event = new (m_coordinator_thd->mem_root) PX_proc(receiver);

     if (!event) {
       my_error(ER_OUTOFMEMORY, MYF(0));
       sql_print_error("fail to register exchange receiver in parallel execution!");
       return true;
     }

     handle = new (m_coordinator_thd->mem_root) PX_worker_handle_impl(receiver);

     if (!handle) {
       my_error(ER_OUTOFMEMORY, MYF(0));
       sql_print_error("fail to register exchange receiver in parallel execution!");
       return true;
     }
     
     break;
    }
   default: {
     // not support yet!
     assert(0);
     return true;
     break;
   }
  }

  /*
    Find all the channel the receiver will create connection.
    Then register the receiver to these channels.
  */
  uint nchannels = m_senders;

  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(i, receiver_no));

    switch (m_channel_type) {
     case PX_MQ_CHANNEL: {
       PX_mq_channel *mq_channel = down_cast<PX_mq_channel *>(channel);
       mq_channel->register_receiver_handle(handle);
       mq_channel->register_reciever_event(event);
       break;
     }
     default: {
       // not support yet!
       assert(0);
       return true;
       break;
     }
    }
  }

  return false;
}

/**
  Register for a exchange sender thread. 

  @return true if fail, false if success.
*/
bool PX_exchange_info::register_sender(THD *sender, uint sender_no) {
  assert(sender_no < m_senders);
  PX_proc *event = nullptr;
  PX_worker_handle *handle = nullptr;

  lock();
  switch (m_channel_type) {
   case PX_MQ_CHANNEL: {
     event = new (m_coordinator_thd->mem_root) PX_proc(sender);

     if (!event) {
       my_error(ER_OUTOFMEMORY, MYF(0));
       sql_print_error("fail to register exchange sender in parallel execution!");
       return true;
     }

     handle = new (m_coordinator_thd->mem_root) PX_worker_handle_impl(sender);

     if (!handle) {
       my_error(ER_OUTOFMEMORY, MYF(0));
       sql_print_error("fail to register exchange sender in parallel execution!");
       return true;
     }
     
     break;
   }
   default: {
     // not support yet!
     assert(0);
     return true;
     break;
   }
  }

  uint nchannels = m_receivers;

  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(sender_no, i));

    switch (m_channel_type) {
     case PX_MQ_CHANNEL: {
       PX_mq_channel *mq_channel = down_cast<PX_mq_channel *>(channel);
       mq_channel->register_sender_handle(handle);
       mq_channel->register_sender_event(event);
       break;
     }
     default: {
       // not support yet!
       assert(0);
       return true;
       break;
     }
    }
  }
  unlock();

  return false;
}

/**
  Clean the exchange info.
  @return true if fail, false if success.
*/
void PX_exchange_info::clean() {
  uint nchannels = m_channels.size();

  for (uint i = 0; i < nchannels; ++i) {
    m_channels[i]->clean();
    destroy(m_channels[i]);
  }
}

bool PX_exchange_info::init_receiver(uint receiver_no) {
  // Find all the channels the receiver will connect to.
  uint nchannels = m_senders;

  lock();
  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(i, receiver_no));

    if (channel->init_receiver()) {
      sql_print_error("fail to init exchange receiver in parallel execution!");
      return true;
    }
  }
  unlock();

  return false;
}

bool PX_exchange_info::init_sender(uint sender_no) {
  // Find all the channels the sender will connect to.
  uint nchannels = m_receivers;

  lock();
  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(sender_no, i));

    if (channel->init_sender()) {
      sql_print_error("fail to init exchange sender in parallel execution!");
      return true;
    }
  }
  unlock();

  return false;
}

bool PX_exchange_info::attach_sender(uint sender_no) {
  // Find all the channels the sender will connect to.
  uint nchannels = m_receivers;

  lock();
  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(sender_no, i));
    if (channel->attach_sender()) {
      return true;
    }
  }
  unlock();

  return false;
}

bool PX_exchange_info::attach_receiver(uint receiver_no) {
  // Find all the channels the receiver will connect to.
  uint nchannels = m_senders;

  lock();
  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(i, receiver_no));
    if (channel->attach_receiver()) {
      return true;
    }
  }
  unlock();

  return false;
}

void PX_exchange_info::detach_sender(uint sender_no) {
  // Find all the channels the sender will detach.
  uint nchannels = m_receivers;

  lock();
  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(sender_no, i));
    channel->detach_sender();
  }
  unlock();
}

void PX_exchange_info::detach_receiver(uint receiver_no) {
  // Find all the channels the receiver will connect to.
  uint nchannels = m_senders;

  lock();
  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(i, receiver_no));
    channel->detach_receiver();
  }
  unlock();
}

void PX_exchange_info::receiver_wait(uint receiver_no) {
  // Find the channel of (0, receiver_no).
  PX_exchange_channel *channel = get_channel(find_channel_no(0, receiver_no));
  channel->receiver_wait();
}

void PX_exchange_info::destroy_release() 
{
  mysql_mutex_destroy(&m_lock);
}

void PX_exchange_info::get_sender_channel(uint sender_no, std::vector<PX_exchange_channel *> &channels) {
  // Find all the channels the sender will sender data to.
  uint nchannels = m_receivers;
  channels.reserve(nchannels);

  lock();
  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(sender_no, i));
    channels.push_back(channel);
  }
  unlock();
}

void PX_exchange_info::get_receiver_channel(uint receiver_no, std::vector<PX_exchange_channel *> &channels) {
  // Find all the channels the receiver will recevie data from.
  uint nchannels = m_senders;
  channels.reserve(nchannels);

  lock();
  for (uint i = 0; i < nchannels; ++i) {
    PX_exchange_channel *channel = get_channel(find_channel_no(i, receiver_no));
    channels.push_back(channel);
  }
  unlock();
}

PX_exchange_channel *PX_exchange_info::get_channel(uint channel_no) {
  uint nchannel = m_channels.size();
  assert(nchannel > 0 && nchannel > channel_no);

  return m_channels[channel_no];
}

uint PX_exchange_info::find_channel_no(uint sender_no, uint receiver_no) {
  assert(sender_no < m_senders && receiver_no < m_receivers);

  return sender_no * m_receivers + receiver_no;
}


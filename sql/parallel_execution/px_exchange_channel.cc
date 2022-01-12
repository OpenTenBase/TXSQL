#include<memory>
#include "sql/sql_class.h"
#include "my_sys.h"
#include "my_dbug.h"
#include "mysqld_error.h"
#include "px_exchange_channel.h"
#include "px_mq.h"
#include "sql/log.h"

PX_mq_channel::PX_mq_channel(uint channel_no, Size size, THD *coordiantor)
    : PX_exchange_channel(channel_no),
      m_size(size),
      m_coordinator_thd(coordiantor) {}

PX_mq_channel::~PX_mq_channel() {}

bool PX_mq_channel::init() {
  assert(m_size);
  assert(m_coordinator_thd);

  m_mq = new (m_coordinator_thd->mem_root) PX_mq(m_size, malloc, free);

  if (m_mq == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(0), (uint32)m_size);
    return true;
  }

  if (m_mq->init()) {
    return true;
  }

  return false;
}

int PX_mq_channel::send_row(PX_iovec *iov, int iovcnt, bool nowait) {
  return m_sender->sendv(iov, iovcnt, nowait);
}

int PX_mq_channel::receive(void **datap, Size *len, bool nowait) {
  return m_receiver->receive(len, datap, nowait);
}

void PX_mq_channel::register_sender_handle(PX_worker_handle *handle) {
  m_sender_handle = handle;
}

void PX_mq_channel::register_receiver_handle(PX_worker_handle *handle) {
  m_receiver_handle = handle;
}

void PX_mq_channel::register_sender_event(PX_proc *event) {
  assert(event);
  m_sender_event = event;
  m_mq->set_sender(m_sender_event);
}

void PX_mq_channel::register_reciever_event(PX_proc *event) {
  assert(event);
  m_receiver_event = event;
  m_mq->set_receiver(m_receiver_event);
}

bool PX_mq_channel::init_sender() {
  // We must register sender event before call init_sender.
  assert(m_coordinator_thd);
  assert(m_sender_event && m_sender_handle);
  assert(!m_sender);

  m_sender = new (m_coordinator_thd->mem_root) PX_mq_handle(m_mq, m_sender_event, malloc, free);

  if (m_sender == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(0), (uint32)m_size);
    return true;
  }

  return 0;
}

bool PX_mq_channel::init_receiver() {
  // We must register receiver event before call init_receiver.
  assert(m_coordinator_thd);
  assert(m_receiver_event && m_receiver_handle);
  assert(!m_receiver);

  m_receiver = new (m_coordinator_thd->mem_root) PX_mq_handle(m_mq, m_receiver_event, malloc, free);

  if (m_receiver == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(0), (uint32)m_size);
    return true;
  }

  return 0;
}

bool PX_mq_channel::attach_sender() {
  // The attach of sender must after the register of receiver.
  if (!m_receiver_handle) {
    sql_print_error("Attach sender must after register of receiver!");
    return true;
  }

  m_sender->set_worker_handle(m_receiver_handle);
  return false;
}

bool PX_mq_channel::attach_receiver() {
  // The attach of receiver must after the register of sender.
  if (!m_sender_handle) {
    sql_print_error("Attach receiver must after register of sender!");
    return true;
  }

  m_receiver->set_worker_handle(m_sender_handle);
  return false;
}

void PX_mq_channel::detach_sender() {
  assert(m_sender);

  m_sender->detach();
}

void PX_mq_channel::detach_receiver() {
  assert(m_receiver);

  m_receiver->detach();
}

void PX_mq_channel::receiver_wait() {
  WaitLatch(m_receiver_event, /*timeout=*/100);
  ResetLatch(m_receiver_event);
}

void PX_mq_channel::clean() {
  if (m_mq) {
    destroy(m_mq);
  }

  if (m_sender_event) {
    destroy(m_sender_event);
  }

  if (m_receiver_event) {
    destroy(m_receiver_event);
  }

  if (m_sender_handle) {
    destroy(m_sender_handle);
  }

  if (m_receiver_handle) {
    destroy(m_receiver_handle);
  }

  if (m_sender) {
    destroy(m_sender);
  }

  if (m_receiver) {
    destroy(m_receiver);
  }
}




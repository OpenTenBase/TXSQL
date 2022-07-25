#include<memory>
#include "sql/sql_class.h"
#include "my_sys.h"
#include "my_dbug.h"
#include "mysqld_error.h"
#include "px_exchange_channel.h"
#include "px_mq.h"
#include "sql/log.h"

#include "px_executor.h"
#include "px.h"

PX_mq_channel::PX_mq_channel(uint channel_no, char *ring_buffer, Size size)
    : PX_exchange_channel(channel_no), m_mq(ring_buffer, size) {}

PX_mq_channel::~PX_mq_channel() {}

void PX_mq_channel::set_sender(PX_proc *me) {
  assert(me);
  m_mq.set_sender(me);
}

void PX_mq_channel::set_receiver(PX_proc *me) {
  assert(me);
  m_mq.set_receiver(me);
}

#ifndef DBUG_OFF
static const char *cstr(PX_mq_result res) {
  const char *s = "???";
  switch (res) {
    case PX_MQ_SUCCESS:
      s = "PX_MQ_SUCCESS";
      break;
    case PX_MQ_WOULD_BLOCK:
      s = "PX_MQ_WOULD_BLOCK";
      break;
    case PX_MQ_DETACHED:
      s = "PX_MQ_DETACHED";
      break;
    case PX_MQ_ERROR:
      s = "PX_MQ_ERROR";
      break;
    case PX_MQ_INTERRUPTED:
      s = "PX_MQ_INTERRUPTED";
      break;
    default:
      assert(0);
      break;
  }
  return s;
}
#endif

typedef void *(*malloc_func_t)(size_t size);
typedef void (*free_func_t)(void *);

static void *px_malloc(size_t size) {
  // TODO psi
  return my_malloc(PSI_INSTRUMENT_ME, size, MYF(MY_WME));
}

static void px_free(void *ptr) {
  my_free(ptr);
}

class PX_exchange_handle_mq : public PX_exchange_handle {
 public:
  PX_exchange_handle_mq(uint channel_id, PX_mq *mq, PX_proc *me)
      : PX_exchange_handle(channel_id), m_handle(mq, me, px_malloc, px_free) {}

  PX_io_error send(PX_iovec *iov, int iovcnt, bool nowait) override {
    PX_mq_result result = m_handle.sendv(iov, iovcnt, nowait);

#ifndef DBUG_OFF
    uint n = 0;
    char buf[513];
    char *p = buf, *end = buf + 512;
    for (int i = 0; i < iovcnt; i++) {
      n += iov[i].len;
      for (Size j = 0; j < iov[i].len && p < end; j++, p += 2) {
        sprintf(p, "%02x", *((char*)iov[i].data + j));
      }
    }
    if (p > end) {
      *end = '\0';
    } else {
      *p = '\0';
    }
    if (n > 256) {
      *(end-1) = '.';
      *(end-2) = '.';
    }
    PX_PRINT_DEBUG("send to channel %u %s %u bytes in %d iov", channel_id(), cstr(result), n, iovcnt);
    PX_PRINT_DEBUG("%s", buf);
#endif

    return convert_mq_result_to_error(result);
  }

  PX_io_error receive(void **datap, Size *len, bool nowait) override {
    PX_mq_result result = m_handle.receive(len, datap, nowait);

#ifndef DBUG_OFF
    char buf[513];
    char *p = buf, *end = buf + 512;
    if (result == PX_MQ_SUCCESS) {
      for (Size i = 0; i < *len && p < end; i++, p += 2) {
        sprintf(p, "%02x", *((char*)(*datap) + i));
      }
      if (p > end) {
        *end = '\0';
      } else {
        *p = '\0';
      }
      if (*len > 256) {
        *(end-1) = '.';
        *(end-2) = '.';
      }
    }
    PX_PRINT_DEBUG("receive from channel %u %s %lu bytes", channel_id(), cstr(result), *len);
    if (result == PX_MQ_SUCCESS) {
      PX_PRINT_DEBUG("%s", buf);
    }
#endif

    return convert_mq_result_to_error(result);
  }

  void set_worker_handle(PX_worker_handle *worker_handle) override {
    m_handle.set_worker_handle(worker_handle);
    PX_PRINT_INFO("set peer %u", worker_handle->id());
  }
  void detach() override {
    PX_PRINT_INFO("detach from channel %u", channel_id());
    m_handle.detach();
    // Allocated by PX_mq_channel::attach().
    destroy(this);
    px_free(this);
  }

 private:
  PX_io_error convert_mq_result_to_error(PX_mq_result result) const {
    PX_io_error res;
    switch (result) {
      case PX_MQ_SUCCESS:
        res = PX_IO_OK;
        break;
      case PX_MQ_WOULD_BLOCK:
        res = PX_IO_WOULD_BLOCK;
        break;
      case PX_MQ_ERROR:
      case PX_MQ_INTERRUPTED:
        res = PX_IO_ERROR;
        break;
      case PX_MQ_DETACHED:
        res = PX_IO_EOF;
        break;
      default:
        assert(0);
        res = PX_IO_ERROR;
        break;
    }
    return res;
  }

  PX_mq_handle m_handle;
};

bool PX_mq_channel::attach(PX_proc *me, PX_exchange_handle *&handle) {
  // Attach sender to channel after regiter of sender.
  PX_PRINT_INFO("attach to channel %u", id());

  // Will be deallocated by PX_exchange_handle_mq::detach().
  handle = (PX_exchange_handle_mq*) px_malloc(sizeof(PX_exchange_handle_mq));
  DBUG_EXECUTE_IF("px_sender_attach_error", {
    if (handle) px_free(handle);
    handle = nullptr;
  });

  DBUG_EXECUTE_IF("px_receiver_attach_error", {
    if (handle) px_free(handle);
    handle = nullptr;
  });

  if (handle == nullptr) {
    my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "PX_mq_channel::attach()");
    PX_PRINT_ERROR("attach to channel %u failed", id());
    return true;
  }
  new (handle) PX_exchange_handle_mq(id(), &m_mq, me);

  return false;
}

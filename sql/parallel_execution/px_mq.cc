/**
  @file sql/parallel_execution/px_mq.cc

  The message queue implemtation.

  @see shm_mq.c
*/

#include "px_mq.h"

#include "my_sys.h"        // my_error
#include "mysqld_error.h"  // ER_*

#include "px_atomic.h"

#define MQH_INITIAL_BUFSIZE ((Size)8192)
#define MaxAllocSize ((Size)0x3fffffff) /* 1 gigabyte - 1 */
#define MAXIMUM_ALIGNOF 8

#define TYPEALIGN(ALIGNVAL, LEN) \
  (((uintptr_t)(LEN) + ((ALIGNVAL)-1)) & ~((uintptr_t)((ALIGNVAL)-1)))
#define MAXALIGN(LEN) TYPEALIGN(MAXIMUM_ALIGNOF, (LEN))

#define TYPEALIGN_DOWN(ALIGNVAL, LEN) \
  (((uintptr_t)(LEN)) & ~((uintptr_t)((ALIGNVAL)-1)))
#define MAXALIGN_DOWN(LEN) TYPEALIGN_DOWN(MAXIMUM_ALIGNOF, (LEN))

#define CHECK_FOR_INTERRUPTS(me)                     \
  do {                                               \
    if ((me)->is_killed()) return PX_MQ_INTERRUPTED; \
  } while (0);

/* If the size isn't MAXALIGN'd, just discard the odd bytes. */
PX_mq::PX_mq(size_t ring_size, char *ring_buffer)
    : m_receiver(),
      m_sender(),
      m_bytes_read(),
      m_bytes_written(),
      m_ring_size(MAXALIGN_DOWN(ring_size)),
      m_detached(),
      m_ring_buffer(ring_buffer) {
  /* Minimum queue size is enough for header and at least one chunk of data. */
  assert(m_ring_size >= MAXIMUM_ALIGNOF);
  SpinLockInit(key_px_mq_lock, &m_mutex);
}

PX_mq::~PX_mq() { SpinLockFree(&m_mutex); }

void PX_mq::set_receiver(PX_proc *proc) {
  PX_proc *sender;

  SpinLockAcquire(&m_mutex);
  assert(m_receiver == nullptr);
  m_receiver = proc;
  sender = m_sender;
  SpinLockRelease(&m_mutex);

  /* Notify sender that receiver has attach. */
  if (sender != nullptr) {
    SetLatch(sender);
  }
}

void PX_mq::set_sender(PX_proc *proc) {
  PX_proc *receiver;

  SpinLockAcquire(&m_mutex);
  assert(m_sender == nullptr);
  m_sender = proc;
  receiver = m_receiver;
  SpinLockRelease(&m_mutex);

  /* Notify the receiver that the sender has attach. */
  if (receiver) {
    SetLatch(receiver);
  }
}

PX_proc *PX_mq::get_receiver() {
  PX_proc *receiver;

  SpinLockAcquire(&m_mutex);
  receiver = m_receiver;
  SpinLockRelease(&m_mutex);

  return receiver;
}

PX_proc *PX_mq::get_sender() {
  PX_proc *sender;

  SpinLockAcquire(&m_mutex);
  sender = m_sender;
  SpinLockRelease(&m_mutex);

  return sender;
}

PX_mq_result PX_mq_handle::send(Size nbytes, const void *data, bool nowait) {
  PX_iovec iov;

  iov.data = static_cast<const char *>(data);
  iov.len = nbytes;

  return sendv(&iov, 1, nowait);
}

PX_mq_result PX_mq_handle::sendv(PX_iovec *iov, int iovcnt, bool nowait) {
  PX_mq_result res;
  PX_proc *receiver;
  Size nbytes = 0;
  Size bytes_written;
  int i;
  int which_iov = 0;
  Size offset;

  assert(m_queue->m_sender == m_me);

  /* Compute total size of write. */
  for (i = 0; i < iovcnt; ++i) nbytes += iov[i].len;

  /* Prevent writing messages overwhelming the receiver. */
  if (nbytes > MaxAllocSize) {
    my_error(ER_DATA_OUT_OF_RANGE, MYF(0), "nbytes", "(MQ::send)");
    return PX_MQ_ERROR;
  }

  /* Try to write, or finish writing, the length word into the message queue. */
  while (!m_length_word_complete) {
    assert(m_partial_bytes < sizeof(Size));
    res =
        send_bytes(sizeof(Size) - m_partial_bytes,
                   ((char *)&nbytes) + m_partial_bytes, nowait, &bytes_written);

    if (res == PX_MQ_DETACHED) {
      /* Reset state in case caller tries to send another message. */
      m_partial_bytes = 0;
      m_length_word_complete = false;
      return res;
    }

    m_partial_bytes += bytes_written;

    if (m_partial_bytes >= sizeof(Size)) {
      assert(m_partial_bytes == sizeof(Size));
      m_partial_bytes = 0;
      m_length_word_complete = true;
    }

    if (res != PX_MQ_SUCCESS) {
      return res;
    }

    /* Length word can't be split unless bigger than required alignment. */
    assert(m_length_word_complete || sizeof(Size) > MAXIMUM_ALIGNOF);
  }

  /* Write the actual data bytes into the buffer. */
  assert(m_partial_bytes <= nbytes);
  offset = m_partial_bytes;
  do {
    Size chunksize;

    /* Figure out which bytes need to be sent next. */
    if (offset >= iov[which_iov].len) {
      offset -= iov[which_iov].len;
      ++which_iov;
      if (which_iov >= iovcnt) break;
      continue;
    }

    /*
      We want to avoid copying the data if at all possible, but every
      chunk of bytes we write into the queue has to be MAXALIGN'd, except
      the last.  Thus, if a chunk other than the last one ends on a
      non-MAXALIGN'd boundary, we have to combine the tail end of its
      data with data from one or more following chunks until we either
      reach the last chunk or accumulate a number of bytes which is
      MAXALIGN'd.
     */
    if (which_iov + 1 < iovcnt &&
        offset + MAXIMUM_ALIGNOF > iov[which_iov].len) {
      char tmpbuf[MAXIMUM_ALIGNOF];
      int j = 0;

      for (;;) {
        if (offset < iov[which_iov].len) {
          tmpbuf[j] = iov[which_iov].data[offset];
          j++;
          offset++;
          if (j == MAXIMUM_ALIGNOF) break;
        } else {
          offset -= iov[which_iov].len;
          which_iov++;
          if (which_iov >= iovcnt) break;
        }
      }

      res = send_bytes(j, tmpbuf, nowait, &bytes_written);

      if (res == PX_MQ_DETACHED) {
        /* Reset state in case caller tries to send another message. */
        m_partial_bytes = 0;
        m_length_word_complete = false;
        return res;
      }

      m_partial_bytes += bytes_written;
      if (res != PX_MQ_SUCCESS) return res;

      continue;
    }

    /*
      If this is the last chunk, we can write all the data, even if it
      isn't a multiple of MAXIMUM_ALIGNOF.  Otherwise, we need to
      MAXALIGN_DOWN the write size.
     */
    chunksize = iov[which_iov].len - offset;
    if (which_iov + 1 < iovcnt) chunksize = MAXALIGN_DOWN(chunksize);
    res = send_bytes(chunksize, &iov[which_iov].data[offset], nowait,
                     &bytes_written);

    if (res == PX_MQ_DETACHED) {
      /* Reset state in case caller tries to send another message. */
      m_length_word_complete = false;
      m_partial_bytes = 0;
      return res;
    }

    m_partial_bytes += bytes_written;
    offset += bytes_written;
    if (res != PX_MQ_SUCCESS) return res;
  } while (m_partial_bytes < nbytes);

  /* Reset for next message. */
  m_partial_bytes = 0;
  m_length_word_complete = false;

  /* If queue has been detached, let caller know. */
  if (m_queue->m_detached) {
    return PX_MQ_DETACHED;
  }

  /*
    If the counterparty is known to have attached, we can read m_receiver
    without acquiring the spinlock and assume it isn't NULL. Otherwise,
    more caution is needed.
  */
  if (m_counterparty_attached) {
    receiver = m_queue->m_receiver;
  } else {
    SpinLockAcquire(&m_queue->m_mutex);
    receiver = m_queue->m_receiver;
    SpinLockRelease(&m_queue->m_mutex);
    if (receiver == NULL) {
      return PX_MQ_SUCCESS;
    }
    m_counterparty_attached = true;
  }

  /* Notify the receiver of the newly-written data, and return. */
  SetLatch(receiver);
  return PX_MQ_SUCCESS;
}

PX_mq_result PX_mq_handle::receive(Size *nbytesp, void **datap, bool nowait) {
  PX_mq_result res;
  Size rb = 0;
  Size nbytes;
  void *rawdata;

  assert(m_queue->m_receiver == m_me);

  /* We can't receive data until the sender has attached. */
  if (!m_counterparty_attached) {
    if (nowait) {
      auto counterparty_gone = mq_counterparty_gone(m_handle);

      if (m_queue->get_sender() == nullptr) {
        return counterparty_gone ? PX_MQ_DETACHED : PX_MQ_WOULD_BLOCK;
      }
    } else {
      res = mq_wait_internal(&m_queue->m_sender, m_handle);
      switch (res) {
        case PX_MQ_DETACHED:
          if (m_queue->m_sender == nullptr) {
            m_queue->m_detached = true;
            return res;
          }
          /*
            Even though the sender is detached, the queue might have
            something to be consumed.
           */
          break;
        case PX_MQ_INTERRUPTED:
          return res;
        case PX_MQ_SUCCESS:
          break;
        default:
          assert(0);
          break;
      }
    }

    m_counterparty_attached = true;
  }

  /*
    If we've consumed an amount of data greater than 1/4th of the ring
    size, mark it consumed in shared memory.  We try to avoid doing this
    unnecessarily when only a small amount of data has been consumed,
    because SetLatch() is fairly expensive and we don't want to do it too
    often.
  */
  if (m_consume_pending > m_queue->m_ring_size / 4) {
    atomic_inc_bytes_read(m_consume_pending);
    m_consume_pending = 0;
  }

  /* Try to read, or finish reading, the length word from the buffer. */
  while (!m_length_word_complete) {
    /* Try to receive the message length word. */
    assert(m_partial_bytes < sizeof(Size));
    res = receive_bytes(sizeof(Size) - m_partial_bytes, nowait, &rb, &rawdata);

    if (res != PX_MQ_SUCCESS) {
      return res;
    }

    /*
      Hopefully, we'll receive the entire message length word at once.
      But if sizeof(Size) > MAXIMUM_ALIGNOF, then it might be split over
      multiple reads.
    */
    if (m_partial_bytes == 0 && rb >= sizeof(Size)) {
      Size needed;

      nbytes = *(Size *)rawdata;

      /* If we've already got the whole message, we're done. */
      needed = MAXALIGN(sizeof(Size)) + MAXALIGN(nbytes);
      if (rb >= needed) {
        m_consume_pending += needed;
        *nbytesp = nbytes;
        *datap = ((char *)rawdata) + MAXALIGN(sizeof(Size));
        return PX_MQ_SUCCESS;
      }

      /*
        We don't have the whole message, but we at least have the whole
        length word.
      */
      m_expected_bytes = nbytes;
      m_length_word_complete = true;
      m_consume_pending += MAXALIGN(sizeof(Size));
      rb -= MAXALIGN(sizeof(Size));
    } else {
      Size lengthbytes = 0;

      /* Can't be split unless bigger than required alignment. */
      assert(sizeof(Size) > MAXIMUM_ALIGNOF);

      /* Message word is split; need buffer to reassemble. */
      if (m_buffer == nullptr) {
        m_buffer = (char *)(*m_malloc_func)(MQH_INITIAL_BUFSIZE);
        m_buflen = MQH_INITIAL_BUFSIZE;
      }
      if (m_buffer == nullptr) {
        my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "(MQ::receive)");
        return PX_MQ_ERROR;
      }
      assert(m_buflen >= sizeof(Size));

      /* Copy partial length word; remember to consume it. */
      if (m_partial_bytes + rb > sizeof(Size)) {
        lengthbytes = sizeof(Size) - m_partial_bytes;
      } else {
        lengthbytes = rb;
      }

      memcpy(&m_buffer[m_partial_bytes], rawdata, lengthbytes);
      m_partial_bytes += lengthbytes;
      m_consume_pending += MAXALIGN(lengthbytes);
      rb -= lengthbytes;

      /* If we now have the whole word, we're ready to read payload. */
      if (m_partial_bytes >= sizeof(Size)) {
        assert(m_partial_bytes == sizeof(Size));
        m_expected_bytes = *(Size *)m_buffer;
        m_length_word_complete = true;
        m_partial_bytes = 0;
      }
    }
  }

  nbytes = m_expected_bytes;

  /*
    Should be disallowed on the sending side already, but better check and
    error out on the receiver side as well rather than trying to read a
    prohibitively large message.
   */
  if (nbytes > MaxAllocSize) {
    my_error(ER_DATA_OUT_OF_RANGE, MYF(0), "nbytes", "(MQ::receive)");
    return PX_MQ_ERROR;
  }

  if (m_partial_bytes == 0) {
    /*
      Try to obtain the whole message in a single chunk.  If this works, we need
      not copy the data and can return a pointer directly into shared memory.
    */
    res = receive_bytes(nbytes, nowait, &rb, &rawdata);

    if (res != PX_MQ_SUCCESS) {
      return res;
    }

    /* If we've already got the whole message, we're done. */
    if (rb >= nbytes) {
      m_length_word_complete = false;
      m_consume_pending += MAXALIGN(nbytes);
      *nbytesp = nbytes;
      *datap = rawdata;
      return PX_MQ_SUCCESS;
    }

    /*
      The message has wrapped the buffer.  We'll need to copy it in order to
      return it to the client in one chunk.  First, make sure we have a large
      enough buffer available.
    */
    if (m_buflen < nbytes) {
      Size newbuflen = std::max(m_buflen, MQH_INITIAL_BUFSIZE);
      while (newbuflen < nbytes) {
        newbuflen *= 2;
      }
      newbuflen = std::min(newbuflen, MaxAllocSize);

      if (m_buffer != nullptr) {
        (*m_free_func)(m_buffer);
        m_buffer = nullptr;
        m_buflen = 0;
      }

      m_buffer = (char *)(*m_malloc_func)(newbuflen);
      m_buflen = newbuflen;

      if (m_buffer == nullptr) {
        my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "(MQ::receive)");
        return PX_MQ_ERROR;
      }
    }
  }

  /* Loop until we've copied the entire message. */
  for (;;) {
    Size still_needed;

    /* Copy as much as we can. */
    assert(m_partial_bytes + rb <= nbytes);
    memcpy(&m_buffer[m_partial_bytes], rawdata, rb);
    m_partial_bytes += rb;

    /*
      Update count of bytes that can be consumed, accounting for
      alignment padding.  Note that this will never actually insert any
      padding except at the end of a message, because the buffer size is
      a multiple of MAXIMUM_ALIGNOF, and each read and write is as well.
     */
    assert(m_partial_bytes == nbytes || rb == MAXALIGN(rb));
    m_consume_pending += MAXALIGN(rb);

    /* If we got all the data, exit the loop. */
    if (m_partial_bytes >= nbytes) {
      break;
    }

    /* Wait for some more data. */
    still_needed = nbytes - m_partial_bytes;
    res = receive_bytes(still_needed, nowait, &rb, &rawdata);

    if (res != PX_MQ_SUCCESS) {
      return res;
    }

    if (rb > still_needed) {
      rb = still_needed;
    }
  }

  /* Return the complete message, and reset for next message. */
  m_length_word_complete = false;
  m_partial_bytes = 0;
  *nbytesp = nbytes;
  *datap = m_buffer;

  return PX_MQ_SUCCESS;
}

void PX_mq_handle::detach() {
  /* Notify counterparty that we're outta here. */
  m_queue->detach(m_me);

  /* Cancel on_dsm_detach callback, if any. */

  /* Release local memory associated with handle. */
  if (m_buffer) {
    (*m_free_func)(m_buffer);
    m_buffer = nullptr;
    m_buflen = 0;
  }
}

void PX_mq::detach(PX_proc *me) {
  PX_proc *victim;

  SpinLockAcquire(&m_mutex);

  if (m_sender == me) {
    victim = m_receiver;
  } else {
    assert(m_receiver == me);
    victim = m_sender;
  }

  m_detached = true;
  SpinLockRelease(&m_mutex);

  if (victim != NULL) {
    SetLatch(victim);
  }
}

PX_mq_result PX_mq_handle::send_bytes(Size nbytes, const void *data,
                                      bool nowait, Size *bytes_written) {
  PX_mq_result res;
  Size used;
  Size ringsize = m_queue->m_ring_size;
  Size sent = 0, available;

  while (sent < nbytes) {
    uint64 rb;
    uint64 wb;

    /* Compute number of ring buffer bytes used and available. */
    rb = px_atomic_read_u64(&m_queue->m_bytes_read);
    wb = px_atomic_read_u64(&m_queue->m_bytes_written);
    assert(wb >= rb);
    used = wb - rb;
    assert(used <= ringsize);
    available = std::min(ringsize - used, nbytes - sent);

    /*
      Bail out if the queue has been detached.  Note that we would be in
      trouble if the compiler decided to cache the value of
      mq->mq_detached in a register or on the stack across loop
      iterations.  It probably shouldn't do that anyway since we'll
      always return, call an external function that performs a system
      call, or reach a memory barrier at some point later in the loop,
      but just to be sure, insert a compiler barrier here.
    */
    px_compiler_barrier();

    if (m_queue->m_detached) {
      *bytes_written = sent;
      return PX_MQ_DETACHED;
    }

    if (available == 0 && !m_counterparty_attached) {
      /*
        The queue is full, so if the receiver isn't yet known to be
        attached, we must wait for that to happen.
       */
      if (nowait) {
        if (mq_counterparty_gone(m_handle)) {
          *bytes_written = sent;
          return PX_MQ_DETACHED;
        }
        if (m_queue->get_receiver() == nullptr) {
          *bytes_written = sent;
          return PX_MQ_WOULD_BLOCK;
        }
      } else {
        res = mq_wait_internal(&m_queue->m_receiver, m_handle);
        switch (res) {
          case PX_MQ_DETACHED:
            m_queue->m_detached = true;
            *bytes_written = sent;
            return res;
          case PX_MQ_INTERRUPTED:
            *bytes_written = sent;
            return res;
          case PX_MQ_SUCCESS:
            break;
          default:
            assert(0);
            break;
        }
      }

      m_counterparty_attached = true;
      /*
        The receiver may have read some data after attaching, so we
        must not wait without rechecking the queue state.
      */
    } else if (available == 0) {
      /*
        Since mq->mqh_counterparty_attached is known to be true at this
        point, mq_receiver has been set, and it can't change once set.
        Therefore, we can read it without acquiring the spinlock.
       */
      assert(m_counterparty_attached);
      /* Notify receiver to read data. */
      SetLatch(m_queue->m_receiver);

      /* Skip manipulation of our latch if nowait = true. */
      if (nowait) {
        *bytes_written = sent;
        return PX_MQ_WOULD_BLOCK;
      }

      /*
        Wait for our latch to be set.  It might already be set  for some
        unrelated reason, but that'll just result in one extra trip through
        the loop.  It's worth it to avoid resetting the latch at top of loop,
        because setting an already-set latch is much cheaper than setting
        one that has been reset.
      */
      WaitLatch(m_queue->m_sender, /*timeout=*/100);
      /* Reset the latch so we don't spin. */
      ResetLatch(m_queue->m_sender);

      /* An interrupt may have occurred while we were waiting. */
      CHECK_FOR_INTERRUPTS(m_me);
    } else {
      Size offset;
      Size sendnow;

      offset = wb % (uint64)ringsize;
      sendnow = std::min(available, ringsize - offset);

      /*
        Write as much data as we can via a single memcpy(). Make sure
        these writes happen after the read of mq_bytes_read, above.
        This barrier pairs with the one in shm_mq_inc_bytes_read.
        (Since we're separating the read of mq_bytes_read from a
        subsequent write to mq_ring, we need a full barrier here.)
       */
      px_memory_barrier();
      memcpy(&m_queue->m_ring_buffer[offset], ((const char *)(data)) + sent,
             sendnow);
      sent += sendnow;

      /*
        Update count of bytes written, with alignment padding.  Note
        that this will never actually insert any padding except at the
        end of a run of bytes, because the buffer size is a multiple of
        MAXIMUM_ALIGNOF, and each read is as well.
       */
      assert(sent == nbytes || sendnow == MAXALIGN(sendnow));
      atomic_inc_bytes_written(MAXALIGN(sendnow));
      /*
        For efficiency, we don't set the reader's latch here. We'll do
        that only when the buffer fills up or after writing an entire
        message.
      */
    }
  }

  assert(sent == nbytes);
  *bytes_written = sent;

  return PX_MQ_SUCCESS;
}

PX_mq_result PX_mq_handle::receive_bytes(Size bytes_needed, bool nowait,
                                         Size *nbytesp, void **datap) {
  uint64 used;
  uint64 written;
  Size ringsize = m_queue->m_ring_size;

  for (;;) {
    Size offset;
    uint64 read;

    /* Get bytes written, so we can compute what's available to read. */
    written = px_atomic_read_u64(&m_queue->m_bytes_written);

    /*
      Get bytes read.  Include bytes we could consume but have not yet
      consumed.
     */
    read = px_atomic_read_u64(&m_queue->m_bytes_read) + m_consume_pending;

    assert(written >= read);
    used = written - read;
    assert(used <= ringsize);
    offset = read % (uint64)ringsize;

    /*
      If there are enough data to read, there are two scenarios:
      1) The message has not been splited, we can just return the pointer
      without memcpy.
      2) The messsage has been splited, so we need two memcpy to get the
      whole message,

      If there are not enough data to read, but buffer has wrapped:
      we can just get the pointer of first part.
    */

    /* If we have enough data or buffer has wrapped, we're done. */
    if (used >= bytes_needed || offset + used >= ringsize) {
      *nbytesp = std::min(used, (uint64)(ringsize - offset));
      *datap = &m_queue->m_ring_buffer[offset];

      /*
        Separate the read of m_bytes_written, above, from caller's attempt to
        read the data itself.  Pairs with the barrier in
        shm_mq_inc_bytes_written.
      */
      px_read_barrier();
      return PX_MQ_SUCCESS;
    }

    /*
      Fall out before waiting if the queue has been detached.

      Note that we don't check for this until *after* considering
      whether the data already available is enough, since the receiver can
      finish receiving a message stored in the buffer even after the sender has
      detached.
    */
    if (m_queue->m_detached) {
      /*
        If the writer advanced m_bytes_written and then set
        m_detached, we might not have read the final value of
        m_bytes_written above.  Insert a read barrier and then check
        again if m_bytes_written has advanced.
      */
      px_read_barrier();
      if (written != px_atomic_read_u64(&m_queue->m_bytes_written)) {
        continue;
      }

      return PX_MQ_DETACHED;
    }

    /*
      We didn't get enough data to satisfy the request, so mark any data
      previously-consumed as read to make more buffer space.
    */
    if (m_consume_pending > 0) {
      atomic_inc_bytes_read(m_consume_pending);
      m_consume_pending = 0;
    }

    /* Skip manipulation of our latch if nowait = true. */
    if (nowait) {
      return PX_MQ_WOULD_BLOCK;
    }

    /*
      Wait for our latch to be set.  It might already be set for some
      unrelated reason, but that'll just result in one extra trip through
      the loop.  It's worth it to avoid resetting the latch at top of
      loop, because setting an already-set latch is much cheaper than
      setting one that has been reset.
    */
    WaitLatch(m_queue->m_receiver, /*timeout=*/100);
    /* Reset the latch so we don't spin. */
    ResetLatch(m_queue->m_receiver);
    /* An interrupt may have occurred while we were waiting. */
    CHECK_FOR_INTERRUPTS(m_me);
  }

  return PX_MQ_DETACHED;
}

bool PX_mq_handle::mq_counterparty_gone(PX_worker_handle *handle) {
  /* If the queue has been detached, counterparty is definitely gone. */
  if (m_queue->m_detached) {
    return true;
  }

  /* If there's a handle, check worker status. */
  if (handle != nullptr) {
    /* Check for unexpected worker death. */
    PX_handle_status status = handle->check_worker_status();

    if (status != NOT_YET_STARTED && status != STARTED) {
      /* Mark it detached, just to make it official. */
      m_queue->m_detached = true;
      return true;
    }
  }

  /* Counterparty is not definitively gone. */
  return false;
}

PX_mq_result PX_mq_handle::mq_wait_internal(PX_proc **ptr,
                                            PX_worker_handle *handle) {
  PX_mq_result result = PX_MQ_DETACHED;

  for (;;) {
    PX_handle_status status;

    /* Acquire the lock just long enough to check the pointer. */
    SpinLockAcquire(&m_queue->m_mutex);
    if (*ptr != nullptr) {
      /* Counterparty attached. */
      result = PX_MQ_SUCCESS;
    }
    SpinLockRelease(&m_queue->m_mutex);

    /* Fail if detached; else succeed if initialized. */
    if (m_queue->m_detached) {
      result = PX_MQ_DETACHED;
      break;
    }

    if (result == PX_MQ_SUCCESS) {
      break;
    }

    if (handle != NULL) {
      /* Check for unexpected worker death. */
      status = m_handle->check_worker_status();

      if (status != NOT_YET_STARTED && status != STARTED) {
        result = PX_MQ_DETACHED;
        break;
      }
    }

    /* Wait to be signaled. */
    WaitLatch(m_me, /*timeout=*/100);

    /* Reset the latch so we don't spin. */
    ResetLatch(m_me);

    /* An interrupt may have occurred while we were waiting. */
    CHECK_FOR_INTERRUPTS(m_me);
  }

  return result;
}

void PX_mq_handle::atomic_inc_bytes_read(int n) {
  /*
    Separate prior reads of m_ring_buffer from the increment of
    m_bytes_read which follows.  This pairs with the full barrier in
    send_bytes(). We only need a read barrier here because the
    increment of m_bytes_read is actually a read followed by a dependent
    write.
  */
  px_read_barrier();

  /*
    There's no need to use pg_atomic_fetch_add_u64 here, because nobody
    else can be changing this value.  This method should be cheaper.
   */
  px_atomic_write_u64(&m_queue->m_bytes_read,
                      px_atomic_read_u64(&m_queue->m_bytes_read) + n);

  /*
    We shouldn't have any bytes to read without a sender, so we can read
    mq_sender here without a lock.  Once it's initialized, it can't change.
  */
  assert(m_queue->m_sender != NULL);
  SetLatch(m_queue->m_sender);
}

void PX_mq_handle::atomic_inc_bytes_written(int n) {
  /*
    Separate prior reads of m_ring_buffer from the write of m_bytes_written
    which we're about to do.  Pairs with the read barrier found in
    receive_bytes.
  */
  px_write_barrier();

  /*
    There's no need to use pg_atomic_fetch_add_u64 here, because nobody
    else can be changing this value.  This method avoids taking the bus
    lock unnecessarily.
   */
  px_atomic_write_u64(&m_queue->m_bytes_written,
                      px_atomic_read_u64(&m_queue->m_bytes_written) + n);
}

#include "include/my_dbug.h"
#include "sql/sql_class.h"
#include "sql/mysqld.h"
#include "px_exchange_info.h"
#include "px_mq.h"
#include "px.h"
#include "px_executor.h"
#include "sql/log.h"

/**
  Compute consumer id by key.
*/
uint rehash_for_px(mem_root_deque<Item *> *key) {
  // TODO: implment the function.
  assert(0);
  return 0;
}

PX_exchange_info::PX_exchange_info(THD *thd, PX_exchange_type exchange_type,
    PX_channel_type type, uint senders, uint receivers, PX_exchange_format format,
    bool need_materialize, reshuffle_func_t reshuffle_func)
    : m_coordinator_thd(thd),
      m_exchange_buffer_size(thd->variables.px_exchange_buffer_size),
      m_channel_type(type),
      m_format(format),
      m_type(exchange_type),
      reshuffle_func(reshuffle_func),
      m_senders(senders),
      m_receivers(receivers),
      m_channels(Malloc_allocator<PSI_memory_key>(PSI_INSTRUMENT_ME)),
      m_proc_handles(Malloc_allocator<PSI_memory_key>(PSI_INSTRUMENT_ME)),
      m_live_handles(Malloc_allocator<PSI_memory_key>(PSI_INSTRUMENT_ME)) {
  assert(exchange_type == PX_GATHER_EXCHANGE);
}

void PX_exchange_info::set_dop(uint senders, uint receivers) {
  m_senders = senders;
  m_receivers = receivers;
}

/**
  Initialize registries.

  Note that all the registries are preallocated before concurrent access, thus
  no further lock is required. Concurrent access to any entry itself is not a
  problem of the registry.

  @return true if fail, false if success.
*/
bool PX_exchange_info::init() {
  assert(m_senders && m_receivers && !m_inited);
#ifndef DBUG_OFF
  m_inited = true;
#endif

  PX_PRINT_INFO("exchange %u init with %u senders and %u receivers",
                m_exchange_id, m_senders, m_receivers);

  /*
    There will be:

      - one channel per each pair of producer and receiver.
      - one live handle per each processor, to detect liveness.
      - one process slot per each processor, to wake up.

    The channels and live handles are allocated and polulated, while the process
    slots are just allocated leaving each processor to register independently.

    All these contexts are maintained in coordinator memory root because its
    lifetime properly include that of all workers.

    See also peer_slot().
   */

  try {
    m_channels.resize(num_channels());
    m_live_handles.resize(num_procs());
    m_proc_handles.resize(num_procs());

    for (uint i = 0; i < num_channels(); i++) {
      PX_exchange_channel *channel = create_channel(i, m_channel_type);
      if (!channel) goto err;
      m_channels.at(i) = channel;
    }

    for (uint i = 0; i < num_procs(); i++) {
      PX_worker_handle *live_handle = new (m_coordinator_thd->mem_root)
          PX_worker_handle_impl(this, i);
      if (!live_handle) goto oom;
      m_live_handles.at(i) = live_handle;
    }

    return false;
  } catch (...) {
    goto oom;
  }

oom:
  my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "PX_exchange_info::init()");

err:
  return true;
}

/**
  Create a channel of given type.

  @return the created channel or nullptr on failure.
*/
PX_exchange_channel *PX_exchange_info::create_channel(
    uint channel_id, PX_channel_type channel_type) {
  PX_exchange_channel *channel = nullptr;
  switch (m_channel_type) {
    case PX_MQ_CHANNEL: {
      const Size ring_size = m_exchange_buffer_size;
      char *ring_buffer = (char *)m_coordinator_thd->mem_root->Alloc(ring_size);
      channel = new (m_coordinator_thd->mem_root)
          PX_mq_channel(channel_id, ring_buffer, ring_size);
      if (!ring_buffer || !channel) {
        my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "PX_exchange_info::create_channel()");
      }
      break;
    }
    default: {
      assert(0);
      break;
    }
  }

  return channel;
}

/**
  Register a proc to the exchange and get its id.

  A sender's id distinguishes it among all senders.
  A receiver's id distinguishes it among all receivers.

  The function is supposed to be invoked by the processor. Since the registry
  is preallocated, it is safe to be concurrently accessed without locking.

  @param[in]  me         The proc to register
  @param[in]  as_sender  The proc is a sender
  @param[out] id         The allocated id

  @return true if fail, false if success.
*/
bool PX_exchange_info::register_proc(PX_proc *me, bool as_sender, uint &id) {
  assert(me && m_inited);

  if (as_sender) {
    const uint new_id = m_sender_id.fetch_add(1);
    assert(new_id < m_senders);

    uint p = slot(new_id, as_sender);
    PX_PRINT_INFO("register proc %u as sender %u to in exchange %u", p, new_id,
                  m_exchange_id);

    // Register proc handle so that live handle can check its status
    m_proc_handles.at(p) = me;

    // Set the proc handle to connected channels.
    for (uint i = new_id * m_receivers, j = 0; j < m_receivers; i++, j++) {
      m_channels.at(i)->set_sender(me);
    }

    id = new_id;
  } else {
    const uint new_id = m_receiver_id.fetch_add(1);
    assert(new_id < m_receivers);

    const uint p = slot(new_id, as_sender);
    PX_PRINT_INFO("register proc %u as receiver %u in exchange %u", p, new_id,
                  m_exchange_id);

    // Register proc handle so that live handle can check its status
    m_proc_handles.at(p) = me;

    // Set the proc handle to connected channels.
    for (uint i = new_id; i < num_channels(); i += m_receivers) {
      m_channels.at(i)->set_receiver(me);
    }

    id = new_id;
  }

  return false;
}

bool PX_exchange_info::deregister_proc(bool as_sender, uint id) {
  // TODO interacts with live handle.
  return false;
}

void PX_exchange_info::clean() {
  for (auto &channel : m_channels) {
    destroy(channel);
  }
  for (auto &handle: m_live_handles) {
    destroy(handle);
  }
}

/**
  Attach given processor to all connected channels.

  @param me            Thread context of the caller.
  @param as_sender     As a sender
  @param id            Id of the processor
  @param[out] handles  The set of handles to access the channels.

  @return false on success, true on error.
 */
bool PX_exchange_info::attach(PX_proc *me, bool as_sender, uint id,
                              PX_exchange_handles &handles) {
  PX_PRINT_INFO("attach as %s %u", (as_sender ? "sender" : "receiver"), id);

  auto attach_channel = [&](PX_exchange_channel *channel) {
    PX_exchange_handle *handle;
    if (channel->attach(me, handle)) return true;

    // Set up the live handle of its peer processor
    uint slot = peer_slot(handle->channel_id(), id, as_sender);
    assert(slot < m_receivers + m_senders);
    handle->set_worker_handle(m_live_handles.at(slot));

    handles.push_back(handle);
    return false;
  };

  if (as_sender) {
    for (uint i = id * m_receivers, j = 0; j < m_receivers; i++, j++) {
      if (attach_channel(m_channels.at(i))) return true;
    }
  } else {
    for (uint i = id; i < num_channels(); i += m_receivers) {
      if (attach_channel(m_channels.at(i))) return true;
    }
  }

  return false;
}

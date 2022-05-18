#ifndef PX_EXCHANGE_INFO_INCLUDED
#define PX_EXCHANGE_INFO_INCLUDED

#include "my_base.h"
#include "px_exchange_channel.h"
#include <unordered_map>

class PX_worker_handle;
class PX_proc;
class THD;

/**
  Channel selection strategy.

  A producer selects channels to send data message by specified algorithm:

    - PX_GATHER_EXCHANGE, select the only receiver
    - PX_BOARDCAST_EXCHANGE, select all channels
    - PX_RESHUFFLE_EXCHANGE, select by m_reshuffle_key

  In other words, it is selecting all or by shuffle key.

  In contrast, a receiver never knows which channel has any message in advance,
  so it has to check all channels.
 */
enum PX_exchange_type {
  PX_INVALID_EXCHANGE = 0,
  PX_GATHER_EXCHANGE,
  PX_RESHUFFLE_EXCHANGE,
  PX_BOARDCAST_EXCHANGE
};

enum PX_exchange_format {
  PX_COMPACT_ROW = 0,
  PX_MYSQL_RECORD,
  PX_CSI_CHUNK
};

typedef uint (*reshuffle_func_t)(mem_root_deque<Item *> *);

using PX_exchange_handles =
    std::vector<PX_exchange_handle *, Malloc_allocator<PX_exchange_handle *>>;

/**
  Provide communications between two sets of processor threads.

  It is effectively the exchange server, and the processors are clients.

  One set are producers, and the other are consumers. Each producer is connected
  to each consumer through a single-producer single-consumer channel, although
  there is not necessarily any traffic on the channel.

  An exchange instance manages all channels, message distribution strategy,
  conversions between wire format and application data structures, as well as
  several handles to interact with processors.

  Each processor runs a tree of iterators. Special bridging iterators are
  used to wrap the channels as sink or source of data message, namely PX_sender,
  PX_receiver and PX_receiver_merge. Processors are involved through serveral
  kinds of handles, namely processor handle (PX_proc), live handle
  (PX_worker_handle), and channel handle (PX_exchange_handle).

    - processor handle, to wait for message and wake on arrival, to check status
    - processor-live handle, used by a processor to check its peer state
    - channel handle, used by a processor to send or receive messages

*/
class PX_exchange_info {
 public:
  PX_exchange_info(THD *thd, PX_exchange_type exchange_type, PX_channel_type type,
                  uint senders, uint receivers, PX_exchange_format format,
                  bool need_materialize, reshuffle_func_t reshuffle_func);
  ~PX_exchange_info() {}

  PX_exchange_type type() { return m_type; }
  PX_exchange_format format() { return m_format; }
  void set_dop(uint senders, uint receivers);
  void set_exchange_id(uint id) { m_exchange_id = id; }
  uint exchange_id() const { return m_exchange_id; }
  void set_top_exchange() { m_top_exchange = true; }
  bool is_top_exchange() const { return m_top_exchange; }

  bool init();
  void clean();

  bool register_proc(PX_proc *me, bool as_sender, uint &id);
  bool deregister_proc(bool as_sender, uint id);
  bool attach(PX_proc *me, bool as_sender, uint id,
              PX_exchange_handles &handles);

 private:
  friend class PX_worker_handle_impl;

  PX_exchange_channel *create_channel(uint channel_id, PX_channel_type channel_type);

  /// Attach to given channel.
  bool attach_channel(PX_exchange_channel *channel, PX_proc *me,
                      bool as_sender, uint id,
                      std::vector<PX_exchange_handle *> &handles);

  /**
    Get the peer slot for a given processor.

    There are m_senders sender processors and m_receiver receiver processors.
    Senders and receivers are identified separately, both starting from zero.

    There is one channel per each pair of producer and sender. Channels are
    also identified starting from zero. The registry of channels can be
    considered as a two-dimension array, m_channels[m_senders][m_receivers].

    The processors are registered in separate ranges of the same registry,
    namely m_proc_handles and m_live_handles. The
    first range is reserved for senders and the other for receivers.

    So the formula between sender id (s), receiver id (r) and channel id (c) is

      (formula-1) c = s * m_receivers + r

    And the formula between sender id (s), receiver id (r) and slot number is

      (formula-2) slot = s, for any sender
      (formula-3) slot = m_senders + r, for any receiver

    Exploring formula-1 we get

      (formula-4) r = c - s * m_receivers
      (formula-5) s = (c - r) / m_receivers

    These formulas are used to locate in registries.

    @param c          Id of given channel
    @param id         Id of the given processor
    @param as_sender  The given processor is a sender

    @return slot number of the peer processor
   */
  uint peer_slot(uint c, uint id, bool as_sender) const {
    uint slot = as_sender ?
        (m_senders + c - id * m_receivers) : // fomular-3 and fomular-4
        ((c - id) / m_receivers);            // formula-2 and formula-5
   assert(slot < m_senders + m_receivers);
   return slot;
  }

  /**
    Get the slot for a given processor.

    @param c          Id of given channel
    @param id         Id of the given processor

    @return slot number of the given processor
   */
  uint slot(uint id, bool as_sender) const {
    uint slot = as_sender ? id /*formula-2*/ : m_senders + id /*formula-3*/;
    assert(slot < m_senders + m_receivers);
    return slot;
  }

  uint num_procs() { return m_senders + m_receivers; }
  uint num_channels() { return m_senders * m_receivers; }

 private:
  /// Identifier of the exchange instance
  uint m_exchange_id{0};
  bool m_top_exchange{false};
  /// The query coordinator
  THD *m_coordinator_thd;

  /// Channel type
  PX_channel_type m_channel_type{PX_INVALID_CHANNEL};
  /// Message wire format
  PX_exchange_format m_format{PX_COMPACT_ROW};
  /// Message distribution stragety
  PX_exchange_type m_type{PX_INVALID_EXCHANGE};

  /**
    Hash function for PX_RESHUFFLE_EXCHANGE strategy.

    TODO: apply the function to select channel handles among connected.
   */
  reshuffle_func_t reshuffle_func;

  /// The number of sender processors
  uint m_senders{0};
  /// The number of receiver processors
  uint m_receivers{0};
  /// Sender id generator
  std::atomic<uint> m_sender_id{0};
  /// Receiver id generator
  std::atomic<uint> m_receiver_id{0};

  // registries
  std::vector<PX_exchange_channel *,
              Malloc_allocator<PX_exchange_channel *>> m_channels;
  std::vector<PX_proc *,
              Malloc_allocator<PX_proc*>> m_proc_handles;
  std::vector<PX_worker_handle *,
              Malloc_allocator<PX_worker_handle *>> m_live_handles;

#ifndef DBUG_OFF
  bool m_inited{false};
#endif
};

/**
  Processor handle for checking status, a.k.a. live handle.

  Each parallel thread acquires and releases a certain slot in the context when
  it starts and finishes its work, prespectively. So that others are able to
  detect its status with respect to parallel processing by the slot number.
*/
class PX_worker_handle_impl : public PX_worker_handle {
 public:
  PX_worker_handle_impl(PX_exchange_info *exchange, uint slot) :
      PX_worker_handle(slot), m_exchange(exchange) {
    assert(m_exchange && slot != UINT_MAX);
  }
  virtual ~PX_worker_handle_impl() {}

  PX_handle_status check_worker_status() override {
    return proc() ? proc()->check_status() : NOT_YET_STARTED;
  }

 private:
  PX_proc *proc() { return m_exchange->m_proc_handles.at(id()); }

  /// Context for parallel threads
  PX_exchange_info *m_exchange;
};

/**
  A global context for an SQL statement to hold all exchange instances.

  It is a statement-level context, provided by the coordinator and can be
  accessed by all workers. The coordinator populates the context by analyzing
  the iterator tree before any worker is started.

  Note that an exchange acts as an arc of a virtual global graph. Each parallel
  thread has the same private graph represented by nodes (DFO). Each node refers
  to the global graph by special low-level iterators. Since the graph is
  actually a tree, each node has only one arc connecting to its parent, they can
  share the same id.
*/
class PX_exchange_context {
 public:
  /// Add an exchange.
  void insert(PX_exchange_info *exchange_info) {
    m_exchange_info_map.emplace(exchange_info->exchange_id(), exchange_info);
  }

  /// Get an exchange by id, or nullptr if none.
  PX_exchange_info *get(int64_t id) {
    auto itr = m_exchange_info_map.find(id);
    if (itr == m_exchange_info_map.end()) return nullptr;
    return (itr->second);
  }

 private:
  std::unordered_map<int64_t, PX_exchange_info*> m_exchange_info_map;
};

#endif

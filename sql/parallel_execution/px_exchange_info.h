#ifndef PX_EXCHANGE_INFO_INCLUDED
#define PX_EXCHANGE_INFO_INCLUDED

#include "my_base.h"
#include "px_exchange_channel.h"
#include "semaphore.h"
#include <unordered_map>

class PX_mq;
class PX_worker_handle;
class PX_proc;
class PX_mq_handle;
class THD;

enum PX_exchange_type {
  PX_INVALID_EXCHANGE = 0,
  PX_GATHER_EXCHANGE,
  PX_RESHUFFLE_EXCHANGE,
  PX_BOARDCAST_EXCHANGE
};

enum PX_exchange_format {
  PX_COMPACT_ROW = 0,
  PX_UNCOMPACT_ROW,
  PX_CSI_CHUNK
};

/**
  The class represents the query context info of
  all the exchange senders and exchange receivers
  between two dfo. A {sender, receiver} pair will
  connect to a exchange channel which used to
  send/receive data.
*/
class PX_exchange_info {
 public:
  PX_exchange_info(THD *thd, PX_exchange_type exchange_type, PX_channel_type type,
      uint senders, uint receivers, PX_exchange_format format, bool need_materialize);
  ~PX_exchange_info() { delete m_barrier; }

  bool init();
  void clean();

  bool register_receiver(THD *receiver, uint receiver_no);
  bool register_sender(THD *sender, uint sender_no);

  bool init_sender(uint sender_no);
  bool init_receiver(uint receiver_no);

  bool attach_sender(uint sender_no);
  bool attach_receiver(uint receiver_no);
  void detach_sender(uint sender_no);
  void detach_receiver(uint receiver_no);

  void receiver_wait(uint receiver_no);

  void get_sender_channel(uint sender_no, std::vector<PX_exchange_channel *> &channels);
  void get_receiver_channel(uint receiver_no, std::vector<PX_exchange_channel *> &channels);

  PX_exchange_type type() { return m_type; }
  PX_exchange_format format() { return m_format; }

  uint find_channel_no(uint sender_no, uint receiver_no);
  PX_exchange_channel *get_channel(uint channel_no);

  void lock() { mysql_mutex_lock(&m_lock); }
  void unlock() { mysql_mutex_unlock(&m_lock); }
  void release_in_stage_over();
  void set_dop(uint senders, uint receivers);

  uint sender_num() { return m_senders; }
  uint receiver_num() { return m_receivers; }

  void set_exchange_id(uint id) { m_exchange_id = id; }
  uint exchange_id() const { return m_exchange_id; }

  void set_top_exchange() { m_top_exchange = true; }
  bool is_top_exchange() const { return m_top_exchange; }

  void exchange_wait() { m_barrier->exchange_wait(); }
  void exchange_senders_post() { m_barrier->exchange_senders_post(); }
  void exchange_receivers_post() { m_barrier->exchange_receivers_post(); }

  void schedule_wait() { m_barrier->schedule_wait(); }
  void schedule_senders_post() { m_barrier->schedule_senders_post(); }
  void schedule_receivers_post() { m_barrier->schedule_receivers_post(); }

 private:
  // The query coordinator.
  THD *m_coordinator_thd{nullptr};
  PX_channel_type m_channel_type{PX_INVALID_CHANNEL};
  // The exchange channels used to send/receive data.
  std::vector<PX_exchange_channel *> m_channels;
  // The number of exchange senders of the DFO.
  uint m_senders{0};
  // The number of exchange receiver of the DFO.
  uint m_receivers{0};
  PX_exchange_type m_type{PX_INVALID_EXCHANGE};
  PX_exchange_format m_format{PX_COMPACT_ROW};
  bool m_need_materialize{false};
  PX_stage_barrier *m_barrier{nullptr};
  mysql_mutex_t m_lock;
  uint m_exchange_id{0};
  bool m_top_exchange{false};
};

/**
  Each SQL parallel execution has exchange context. which contains
  all exchange informations, the context remains in coordinator's
  and workers' THD.

  Coordinator generate and create all exchange info by traversing
  iterator tree, workers can get the exchange info before running.
*/
class PX_exchange_context
{
 public:
  /* Insert the exchange info into a hash map. */
  void insert(PX_exchange_info *exchange_info)
  {
    m_exchange_info_map.insert(PX_exchange_info_pair(
      exchange_info->exchange_id(), exchange_info));
  }

  /* Get the exchange info from hash map, return nullptr if error.*/
  PX_exchange_info *get(int64_t id)
  {
    PX_exchange_info_map::iterator itr = m_exchange_info_map.find(id);
    if (itr == m_exchange_info_map.end()) return nullptr;
    return (itr->second);
  }

 private:
  typedef std::pair<int64_t, PX_exchange_info *> PX_exchange_info_pair;
  typedef std::unordered_map<int64_t, PX_exchange_info*> PX_exchange_info_map;
  PX_exchange_info_map m_exchange_info_map; //hash map.
};

#endif
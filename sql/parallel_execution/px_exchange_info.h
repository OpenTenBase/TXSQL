#ifndef PX_EXCHANGE_INFO_INCLUDED
#define PX_EXCHANGE_INFO_INCLUDED

#include "my_base.h"
#include "px_exchange_channel.h"

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
  ~PX_exchange_info() {}

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

  void destroy_release();
  void lock() { mysql_mutex_lock(&m_lock); }
  void unlock() { mysql_mutex_unlock(&m_lock); }

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
  mysql_mutex_t m_lock;
};

#endif

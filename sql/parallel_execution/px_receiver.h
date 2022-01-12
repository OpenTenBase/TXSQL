#ifndef PX_EXCHANGE_RECEIVER_INCLUDED
#define PX_EXCHANGE_RECEIVER_INCLUDED

#include <vector>

#include "my_base.h"
#include "mem_root_deque.h"
#include "px.h"

class Field;
class Item;
class PX_exchange_info;
class PX_exchange_channel;
class PX_mq_handle;
class THD;
class TABLE;

/**
  The class represents a exchange receiver backend member.
  A receiver connect to a exchange channel which can
  communicate with a exchange sender. 
*/
class PX_receiver {
 public:
  PX_receiver();
  PX_receiver(uint receiver_no, PX_exchange_info *pei, THD *thd, TABLE *table);

  virtual bool init();
  virtual bool next();
  virtual void end();

  virtual bool attach();
  bool decompact_row(uchar *data, Size msg_len);
  uint get_receiver_no() { return m_receiver_no; }
  PX_exchange_info *get_pei() { return m_pei; }
  THD *get_thd() { return m_thd; }
  TABLE *get_table() { return m_table; }
  uint senders() { return m_channels.size(); }

 private:
  bool read_compact_row(void **datap, Size *len);
  void mqueue_mmove(uint next_channel, uint active_channels);
  void decompact_field(Field *field, uchar *data, uint &ptr_offset);

 public:
  std::vector<PX_exchange_channel *> m_channels;

 private:
  uint m_receiver_no{INT_MAX};
  PX_exchange_info *m_pei{nullptr};
  THD *m_thd{nullptr};
  // The exchange receiver tmp table.
  TABLE *m_table{nullptr};
  /** The next channel to receive data. */
  uint m_next_channel{0};
  /** The number of left channels. */
  uint m_active_channels{0};
};

#endif

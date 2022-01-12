#ifndef PX_SENDER_INCLUDED
#define PX_SENDER_INCLUDED

#include "my_base.h"
#include "mem_root_deque.h"

class Field;
class Item;
class PX_exchange_info;
class PX_mq_handle;
class PX_field_data;
class TABLE;
class THD;

/**
  The PX_sender represents a exchange sender. A PX_sender
  will connect with each PX_receiver through a PX_exchange_channel.
*/
class PX_sender{
 public:
  PX_sender(uint sender_no, PX_exchange_info *pei, THD *thd,
            TABLE *table, mem_root_deque<Item *> *send_fields,
            mem_root_deque<Item *> *shuffle_key);
  ~PX_sender() {}

  bool init();
  bool send();
  void end();

  bool attach();

 private:
  bool send_compact_row();
  bool prepare_compact_row();
  bool make_compact_row(uint16 &null_len, uint32 &total_copy_bytes);
  uint32 make_compact_field(Field *field, PX_field_data *px_field);

  uint cal_reshuffle_channel(mem_root_deque<Item *> *reshuffle_key);

 private:
  uint m_sender_no{INT_MAX};
  PX_exchange_info *m_pei{nullptr};
  THD *m_thd{nullptr};
  TABLE *m_table{nullptr};
  mem_root_deque<Item *> *m_send_fields{nullptr};
  PX_field_data *m_compact_row{nullptr};
  bool *m_skip_array{nullptr};
  char *m_skip_flag{nullptr};
  mem_root_deque<Item *> *m_reshuffle_key{nullptr};
};

#endif

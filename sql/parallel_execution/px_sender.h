#ifndef PX_SENDER_INCLUDED
#define PX_SENDER_INCLUDED

#include "my_base.h"
#include "mem_root_deque.h"
#include "sql/iterators/row_iterator.h"
#include "sql/sql_tmp_table.h"
#include "sql/sql_optimizer.h"  // JOIN

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
class PX_sender : public RowIterator {
 public:
  PX_sender(THD *thd, uint sender_no, PX_exchange_info *pei,
            unique_ptr_destroy_only<RowIterator> source,
            TABLE *table, mem_root_deque<Item *> *send_fields,
            mem_root_deque<Item *> *shuffle_key,
            Temp_table_param *temp_table_param);
  ~PX_sender() {}

  bool init();
  bool send();
  void end();

  bool attach();

  bool Init() override { return attach(); }
  int Read() override { return send() ? -1 : 0; }

  void set_exchange_info(PX_exchange_info *ex_info) { m_pei = ex_info; }

  void SetNullRowFlag(bool is_null_row) override { m_source->SetNullRowFlag(is_null_row);}
  void UnlockRow() override { m_source->UnlockRow(); }
  virtual std::string str() override { return "PX_Send"; }
  virtual PhysicalRowIteratorType type() override { return PHY_PX_SEND; }
  virtual void adjust_children() override { add_child(m_source.get()); }

 private:
  bool send_compact_row();
  bool prepare_compact_row();
  bool make_compact_row(uint16 &null_len, uint32 &total_copy_bytes);
  uint32 make_compact_field(Field *field, PX_field_data *px_field);

  uint cal_reshuffle_channel(mem_root_deque<Item *> *reshuffle_key);

 private:
  THD *m_thd{nullptr};
  uint m_sender_no{INT_MAX};
  PX_exchange_info *m_pei{nullptr};
  unique_ptr_destroy_only<RowIterator> m_source;
  TABLE *m_table{nullptr};
  mem_root_deque<Item *> *m_send_fields{nullptr};
  PX_field_data *m_compact_row{nullptr};
  bool *m_skip_array{nullptr};
  char *m_skip_flag{nullptr};
  mem_root_deque<Item *> *m_reshuffle_key{nullptr};
  Temp_table_param *m_temp_table_param{nullptr};
  std::vector<Field *> m_fields;
};

#endif

#ifndef PX_EXCHANGE_RECEIVER_INCLUDED
#define PX_EXCHANGE_RECEIVER_INCLUDED

#include <vector>

#include "my_base.h"
#include "mem_root_deque.h"
#include "px.h"
#include "sql/table.h"
#include "sql/iterators/row_iterator.h"

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
class PX_receiver : public RowIterator {
 public:
  PX_receiver(THD *thd, uint receiver_no, PX_exchange_info *pei, JOIN *join,
    unique_ptr_destroy_only<RowIterator> source, TABLE *table, int ref_slice);
  ~PX_receiver() {}

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

  bool Init() override { return attach(); }
  int Read() override { return next() ? -1 : 0; }

  void StartPSIBatchMode() override {
    // TODO: receiver should send this message to sender through mq.
    m_source->StartPSIBatchMode();
  }
  void EndPSIBatchModeIfStarted() override {
    // TODO: receiver should send this message to sender through mq.
    m_source->EndPSIBatchModeIfStarted();
  }

  void set_exchange_info(PX_exchange_info *ex_info) { m_pei = ex_info; }

  void SetNullRowFlag(bool is_null_row) override {
    if (is_null_row) { m_table->set_null_row();
    } else { m_table->reset_null_row(); }
  }
  void UnlockRow() override { m_table->file->unlock_row(); }
  virtual std::string str() override { return "PX_RECEIVE"; }
  virtual PhysicalRowIteratorType type() override { return PHY_PX_RECEIVE; }
  virtual void adjust_children() override { add_child(m_source.get()); }


 private:
  bool read_compact_row(void **datap, Size *len);
  void mqueue_mmove(uint next_channel, uint active_channels);
  void decompact_field(Field *field, uchar *data, uint &ptr_offset);

 public:
  std::vector<PX_exchange_channel *> m_channels;

 private:
  THD *m_thd{nullptr};
  uint m_receiver_no{INT_MAX};
  PX_exchange_info *m_pei{nullptr};
  JOIN *m_join{nullptr};
  unique_ptr_destroy_only<RowIterator> m_source;
  // The exchange receiver tmp table.
  TABLE *m_table{nullptr};
  /** The next channel to receive data. */
  uint m_next_channel{0};
  /** The number of left channels. */
  uint m_active_channels{0};
  int m_ref_slice{0};
  int m_input_slice{0};
  std::vector<Field *> m_fields;
};

#endif

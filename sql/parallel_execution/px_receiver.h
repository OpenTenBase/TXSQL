#ifndef PX_EXCHANGE_RECEIVER_INCLUDED
#define PX_EXCHANGE_RECEIVER_INCLUDED

#include "my_base.h"
#include "mem_root_deque.h"
#include "px.h"
#include "sql/table.h"
#include "sql/iterators/row_iterator.h"

#include "px_exchange_info.h" // PX_exchange_handles

class Field;
class Item;
class PX_exchange_info;
class PX_exchange_handle;
class PX_codec;
class THD;
class TABLE;

void SwitchSlice(JOIN *join, int slice_num);

/**
  A bridging iterator that takes an exchange as data source.

  It is effectively an exchange client.

  Any message received through the exchange is decoded and saved in a record
  buffer so that upper iterators can access.
*/
class PX_receiver : public RowIterator {
 public:
  PX_receiver(THD *thd, uint receiver_id, PX_exchange_info *pei, JOIN *join,
              unique_ptr_destroy_only<RowIterator> source,
              mem_root_deque<TABLE *> *tables, int ref_slice);
  ~PX_receiver() {}

  bool Init() override;
  int Read() override;
  virtual void End();

 public:
  void StartPSIBatchMode() override {
    // TODO: receiver should send this message to sender through mq.
    m_source->StartPSIBatchMode();
  }
  void EndPSIBatchModeIfStarted() override {
    // TODO: receiver should send this message to sender through mq.
    m_source->EndPSIBatchModeIfStarted();
  }

  void SetNullRowFlag(bool is_null_row) override {
    if (is_null_row) {
      for (TABLE *table : *m_tables) table->set_null_row();
    } else {
      for (TABLE *table : *m_tables) table->reset_null_row();
    }
  }
  void UnlockRow() override {
    for (TABLE *table : *m_tables) table->file->unlock_row();
  }

  void set_exchange_info(PX_exchange_info *pei) { m_pei = pei; }
  PX_exchange_info *get_pei() const { return m_pei; }
  uint get_receiver_id() { return m_receiver_id; }
  PX_exchange_info *get_pei() { return m_pei; }
  TABLE *get_table() { return m_tables->front(); }
  PX_codec *get_codec() { return m_codec; }

  virtual std::string str() override { return "PX_RECEIVE"; }
  virtual PhysicalRowIteratorType type() override { return PHY_PX_RECEIVE; }
  virtual void adjust_children() override { add_child(m_source.get()); }

 private:
  int receive(void **datap, Size *len);
  PX_proc *me() const;

 protected:
  void detach();

  /// Channel handles to receive messages
  PX_exchange_handles m_handles;

 private:
  JOIN *m_join{nullptr};
  unique_ptr_destroy_only<RowIterator> m_source;

  /// Exchange instance
  PX_exchange_info *m_pei{nullptr};
  uint m_receiver_id{INT_MAX};

  /// The table providing record buffer, record[0].
  const mem_root_deque<TABLE *> *m_tables{nullptr};
  /// Decoder output fields
  std::vector<Field *> m_fields;
  /// The decoder
  PX_codec *m_codec{nullptr};

  /// The channel to read
  uint m_cursor{0};
  /// The number of consecutive empty channels
  uint m_skip_count{0};

  int m_ref_slice{0};
  int m_input_slice{0};
};

#endif

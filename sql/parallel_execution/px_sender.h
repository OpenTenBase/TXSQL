#ifndef PX_SENDER_INCLUDED
#define PX_SENDER_INCLUDED

#include "px_executor.h"

#include "my_base.h"
#include "mem_root_deque.h"
#include "sql/iterators/row_iterator.h"
#include "sql/sql_tmp_table.h"
#include "sql/sql_optimizer.h"  // JOIN

#include "px_exchange_info.h" // PX_exchange_handles

class Field;
class Item;
class PX_exchange_info;
class PX_exchange_handle;
class TABLE;
class THD;
class PX_codec;

/**
  A bridging iterator that takes an exchange as data sink.

  It is effectively an exchange client.

  Any source row is encoded to a message then sent through the exchange.
*/
class PX_sender : public RowIterator {
 public:
  PX_sender(THD *thd, uint sender_no, PX_exchange_info *pei,
            unique_ptr_destroy_only<RowIterator> source,
            std::vector<TABLE *> *tables, mem_root_deque<Item *> *send_fields,
            mem_root_deque<Item *> *shuffle_key,
            Temp_table_param *temp_table_param, bool use_item,
            unique_ptr_destroy_only<RowIterator> table_path);
  ~PX_sender() {}

  bool Init() override;
  int Read() override;
  void End();


  void StartPSIBatchMode() override {
    if (!m_materialize) {
      m_source->StartPSIBatchMode();
    } else {
      m_table_path->StartPSIBatchMode();
    }
  }
  void EndPSIBatchModeIfStarted() override {
    if (!m_materialize) {
      m_source->EndPSIBatchModeIfStarted();
    } else {
      m_table_path->EndPSIBatchModeIfStarted();
    }
  }

  void SetNullRowFlag(bool is_null_row) override {
    if (!m_materialize) {
      m_source->SetNullRowFlag(is_null_row);
    } else {
      m_table_path->SetNullRowFlag(is_null_row);
    }
  }
  void UnlockRow() override {
    if (!m_materialize) {
      m_source->UnlockRow();
    } else {
      m_table_path->UnlockRow();
    }
  }

  void set_exchange_info(PX_exchange_info *pei) { m_pei = pei; }
  PX_exchange_info *get_pei() const { return m_pei; }

  virtual std::string str() override { return "PX_Send"; }
  virtual PhysicalRowIteratorType type() override { return PHY_PX_SEND; }
  virtual void adjust_children() override { add_child(m_source.get()); }

 private:
  bool register_to_exchange();
  bool attach();
  void detach();
  PX_proc *me() const;

  unique_ptr_destroy_only<RowIterator> m_source;
  bool m_materialize;
  /// Temporary table for materialization
  unique_ptr_destroy_only<RowIterator> m_table_path;

  /// Exchange instance
  PX_exchange_info *m_pei{nullptr};
  uint m_sender_id{INT_MAX};

  /// Channel handles to send messages
  PX_exchange_handles m_handles;

  bool m_use_item;
  Temp_table_param *m_temp_table_param{nullptr};
  /// The encoder
  PX_codec *m_codec{nullptr};

  const std::vector<TABLE *> *m_tables{nullptr};
  /// Encoder input fields
  std::vector<Field *> m_fields;
  /// Encoder input items
  mem_root_deque<Item *> *m_send_fields{nullptr};

  mem_root_deque<Item *> *m_reshuffle_key{nullptr};
};

#endif

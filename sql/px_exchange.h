#ifndef SQL_EXCHANGE_LOCAL_PX_EXCHANGE_H_
#define SQL_EXCHANGE_LOCAL_PX_EXCHANGE_H_

#include <vector>

#include "include/my_inttypes.h"
#include "sql/field.h"          // Field
#include "sql/iterators/row_iterator.h"   // RowIterator
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_tmp_table.h"

typedef Field *SourceItem;
typedef std::vector<Field *> ExchangeSourceItem;
constexpr uint EXCHANGE_BUFFER_SIZE = 1UL << 16;
constexpr uint MAX_EXCHANGE_NUM = 4;

class PX_Gather_base : public TableRowIterator {
 public:
  PX_Gather_base(THD *thd, TABLE *table, uchar *record)
      : TableRowIterator(thd, table),
        m_fields(nullptr),
        m_field_num(0),
        m_null_flag_offset(0),
        m_record_buffer(record) {}

  virtual ~PX_Gather_base() {
    delete m_fields;
  }

 protected:
  bool init() {
    m_fields = new ExchangeSourceItem;
    for (Field **pfield = table()->field; *pfield != nullptr; ++pfield) {
      Field *field = *pfield;
      if (bitmap_is_set(table()->read_set, field->field_index()))
        m_fields->push_back(field);
    }
    m_field_num = m_fields->size();
    m_null_flag_offset = m_field_num / 8;
    return false;
  }

  void convert_record_to_field();

 protected:
  ExchangeSourceItem *m_fields;

 private:
  uint m_field_num;
  uint m_null_flag_offset;
  /* | .. NULL_flag2 NULL_flag1(1bit) | field1 field2 ... | */
  uchar *m_record_buffer;
};

class PX_Gather : public PX_Gather_base {
 public:
  PX_Gather(THD *thd, TABLE *table, uchar *record, JOIN *join, int ref_slice,
            Temp_table_param *temp_table_param,
            unique_ptr_destroy_only<RowIterator> send)
      : PX_Gather_base(thd, table, record),
        m_join(join),
        m_ref_slice(ref_slice),
        m_input_slice(-1),
        m_temp_table_param(temp_table_param),
        m_source(move(send)) {}

  ~PX_Gather() {}

  bool Init() override {
    if (init()) return true;
    m_input_slice = m_join->get_ref_item_slice();
    return m_source->Init();
  }

  /**
   * @brief Read record from message queue(a shared memory actually), and write
   * into temporary table.
   * 
   * @return -1 on EOF
   * @return 0 on success
   * @return 1 on failure
   */
  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }
  void UnlockRow() override { m_source->UnlockRow(); }

 private:
  JOIN *m_join;
  int m_ref_slice;
  int m_input_slice;
  Temp_table_param *m_temp_table_param;
  unique_ptr_destroy_only<RowIterator> m_source;
};

class PX_Send : public TableRowIterator {
 public:
  PX_Send(THD *thd, TABLE *table, mem_root_deque<Item *> *send_fields,
          Temp_table_param *temp_table_param, uchar *record,
          unique_ptr_destroy_only<RowIterator> source)
      : TableRowIterator(thd, table),
        m_fields(nullptr),
        m_send_fields(send_fields),
        m_field_num(0),
        m_null_flag_offset(0),
        m_temp_table_param(temp_table_param),
        m_record_buffer(record),
        m_source(move(source)) {}

  virtual ~PX_Send() {
    delete m_fields;
  }

  bool Init() override {
    m_fields = new ExchangeSourceItem;
    for (Field **pfield = table()->field; *pfield != nullptr; ++pfield) {
      Field *field = *pfield;
      if (bitmap_is_set(table()->read_set, field->field_index()))
        m_fields->push_back(field);
    }
    m_field_num = m_fields->size();
    m_null_flag_offset = m_field_num / 8;

    return m_source->Init();
  }

  /**
   * @brief 
   * 
   * @return -1 on EOF
   * @return 0 on success
   * @return 1 on failure
   */
  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }

  void UnlockRow() override { m_source->UnlockRow(); }

 protected:
  bool convert_field_to_record();

 protected:
  ExchangeSourceItem *m_fields;
  mem_root_deque<Item *> *m_send_fields;

 private:
  uint m_field_num;
  uint m_null_flag_offset;
  Temp_table_param *m_temp_table_param;
  uchar *m_record_buffer;

 protected:
  unique_ptr_destroy_only<RowIterator> m_source;
};

#endif

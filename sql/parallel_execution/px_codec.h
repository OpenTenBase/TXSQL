#ifndef PX_CODEC_INCLUDED
#define PX_CODEC_INCLUDED

#include <vector>
#include "include/mem_root_deque.h"
#include "px.h"

struct PX_iovec;
class Temp_table_param;
class THD;
class Item;
class Field;
/**
  Base codec class. 

  A codec is used to encode items or fields to certain exchange
  format or decode in the reverse direction.
*/
class PX_codec {
 public:
  PX_codec() {}
  virtual ~PX_codec() {}

  virtual int init(mem_root_deque<Item *> *items, std::vector<Field *> *fields) = 0;
  virtual int encode(std::vector<PX_iovec> &memory_trunks) = 0;
  virtual int decode(uchar *data, Size len) = 0;
};

/**
  Codec for compact row format.

  A compact row consists of a few protocol fields. It is to optimize memory
  usage against MySQL record format which reserves memory by type-defined size
  rather than real data size.

  There are four protocol fields for hidden information: length, property
  bitmap and its length, and an optional row id (ref).

  Each user field has a const property and a null property. Only a field
  which is non-const and non-null gets delivered in a protocol field. 
  A user field is of either fixed length or variable length, and requires
  explicit type information to encode or decode.

  Hidden information are also known as hidden fields, and decribed in the
  property bitmap. In other words, the first byte of bitmap is 00000001,
  indicating row id is non-const and missing. (not implemented)

  The physical layout of a compact row is:

  | length | bitmap_len | bitmap |  ref  | field1 | field2 | ... |
  (4-bytes)     (2)    (bitmap_len) (2)

  Note that 2-byte bitmap_len is sufficient for MAX_FIELDS(4096), because
  the max number of bitmap bytes is (4096 * 2 + 7 ) / 8 = 1024 bytes (2^10).

  The data length of a fixed-length field is defined by the type. The data
  length of a variable-length field is encoded as the length part whose
  size is also defined by the type.

  A fixed-length field:

  | data |

  A variable-length field:

  | length | data |
   (1 or 2)
*/
class PX_compact_codec : public PX_codec {
 public:
  struct PX_field_data {
    uchar *m_ptr{nullptr};
    uint32 m_len{0};
    /*
      Set to false to ignore the field from compact row.
      It is an optimization of message size for null
      field or basic_const_item().
    */
    bool m_need_send{true};
  };
 public:
  PX_compact_codec(THD *thd, bool use_item, Temp_table_param *temp_table_param);
  ~PX_compact_codec();

  int init(mem_root_deque<Item *> *items, std::vector<Field *> *fields) override;
  int encode(std::vector<PX_iovec> &memory_trunks) override;
  int decode(uchar *data, Size len) override;

 private:
  bool compact_items();
  bool compact_fields();
  uint32 make_compact_field(Field *field, PX_field_data *px_field);
  int decompact_field(Field *field, bool is_null, uchar *data,
                      uint &ptr_offset);

  void reset();
  THD *get_thd() { return m_thd; }
 private:
  THD *m_thd{nullptr};
  /*
    If true, exchange would send the result of items.
    Otherwise, exchange would send the field in table directly.
  */
  bool m_use_item{false};
  mem_root_deque<Item *> *m_items{nullptr};
  std::vector<Field *> *m_fields{};
  Temp_table_param *m_temp_table_param{nullptr};
  PX_field_data *m_compact_row{nullptr};
  bool *m_skip_array{nullptr};
  char *m_skip_flag{nullptr};
  uint16 null_len{0};
  uint32 total_copy_bytes{0};
  uint null_num{0};
};

#endif

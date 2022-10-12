#ifndef PX_TYPES_INCLUDED
#define PX_TYPES_INCLUDED

/**
  Use this AggTypr to sign the state of aggregate.

  PX_LOCAL_AGG and PX_FINAL_AGG mean this aggrgeate is split for parallel
  execution. PX_LOCAL_AGG signs aggregate in worker, PX_FINAL_AGG signs
  aggregate in coordinator.
*/
enum class AggType {
  PX_NONE = 0,
  PX_LOCAL_AGG,
  PX_FINAL_AGG
};

enum PX_SCAN_TYPE {
  PX_TABLE_SCAN,
  PX_INDEX_SCAN,
  PX_RANGE_SCAN,
  PX_REF_SCAN,
  PX_DEPEND_REF_SCAN,
  PX_INVALID_SCAN
};

class TABLE;
class TABLE_REF;

/**
  This class represents the description information of the parallel table,
  based on which dynamic partitioning is done.
*/
class PX_table_descriptor {
 public:
  PX_table_descriptor(TABLE *table, PX_SCAN_TYPE type,
                      uint keyno, bool reverse_scan) :
      m_table(table),
      m_type(type),
      m_keyno(keyno),
      m_reverse_scan(reverse_scan) {}

  TABLE *table() { return m_table; }
  PX_SCAN_TYPE type() { return m_type; }
  uint keyno() { return m_keyno; }
  bool reverse_scan() { return m_reverse_scan; }

 private:
  TABLE *m_table{nullptr};
  PX_SCAN_TYPE m_type{PX_INVALID_SCAN};
  uint m_keyno{UINT_MAX};
  bool m_reverse_scan{false};
};

#endif

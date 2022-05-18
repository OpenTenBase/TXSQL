#include "px_receiver.h"
#include "px_binary_heap.h"

#define MAX_RECORD_STORE 10
#define RECORD_BUFFER_SIZE 128

/** Compare two nodes in heap. */
extern bool heap_compare_records(int a, int b, void *arg);

/**
  Wrapper of record in message queue
*/
typedef struct mq_record_struct {
  /** The message data in message queue. */
  uchar *m_data;
  /** The message length. */
  uint32 m_length;
  /** The length of buffer used to cache this message. */
  uint32 m_buffer_len;
  mq_record_struct() :
      m_data(nullptr), m_length(0), m_buffer_len(RECORD_BUFFER_SIZE) {}
} mq_record_st;

/**
  Batch of cached message queue records.
  records[i] is the i-th worker's cached records.
  When all messages have been read from message queue,
  set completed = true.
*/
typedef struct mq_records_batch_struct {
  mq_record_st **records;
  /* Total number of records cached */
  int n_total;
  /* Number of records have been read */
  int n_read;
  bool completed;
} mq_records_batch_st;

/**
  The class represents a exchange receiver with merge sort.
  We use a binary heap to to the sort.
*/
class PX_receiver_merge : public PX_receiver {
 public:
  // PX_receiver_merge();
  PX_receiver_merge(THD *thd, uint receiver_no, PX_exchange_info *pei,
      TABLE *table, Filesort *sort, unique_ptr_destroy_only<RowIterator> source,
      JOIN *join, int ref_slice);
  virtual ~PX_receiver_merge() {}

  bool Init() override;
  int Read() override;
  void End() override;

 public:
  Filesort *get_filesort() { return m_sort; }
  Sort_param *get_sort_param() { return m_sort_param; }
  mq_record_st *get_record(uint k) {
    assert(k < senders());
    mq_record_st *record = m_min_records[k];
    return record;
  }
  uchar *get_key(int i) {
    assert(0 <= i && i < 2);
    return keys[i];
  }

 private:
  uint senders() { return m_handles.size(); }
  /** alloc space for sort */
  bool alloc();
  /** build the binary heap */
  void build_heap();
  /** get minimum record */
  mq_record_st *get_min_record();
  /**
    read one message from the id-th message queue and then
    copy it to m_record_groups[id].records[i].
  */
  bool load_group_record(uint id, int i, bool *completed, bool nowait);
  /**
    fetch the current minimum record of the id-th records
    group and store it in m_min_records.
  */
  bool read_group(uint id, bool nowait);
  /** try to load a batch of messages in no-blocking mode */
  void load_group_records(int id);
  /** store message from table->record as a mq_record_st */
  bool store_mq_record(mq_record_st *rec, uchar *data, uint32 msg_len);

 private:
  JOIN *m_join{nullptr};
  int m_ref_slice{0};
  
  Sort_param *m_sort_param;
  // sort structure.
  Filesort *m_sort;
  // compared-keys of two nodes in heap.
  uchar *keys[2]{NULL};
  /**
    The array of minimum records.
    when all records in one worker have been ordered, the
    global min/max record must in m_min_records.
  */
  mq_record_st **m_min_records;
  // array of minimum recors of each worker's group.
  mq_records_batch_st *m_record_groups;
  // binary heap for doing merge sort.
  binary_heap *m_heap;
  // whether the heap has inited.
  bool m_init_heap;
  // tmp key for comparing.
  uchar *m_tmp_key;
};

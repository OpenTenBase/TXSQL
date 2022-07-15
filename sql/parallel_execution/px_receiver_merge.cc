#include "px_receiver_merge.h"
#include "px_receiver.h"
#include "px_mq.h"
#include "px_exchange_info.h"
#include "px_codec.h"
#include "sql/filesort.h"
#include "sql/sql_optimizer.h"
#include "sql/log.h"

PX_receiver_merge::PX_receiver_merge(
    THD *thd, uint receiver_id, PX_exchange_info *pei,
    mem_root_deque<TABLE *> *tables, Filesort *sort,
    unique_ptr_destroy_only<RowIterator> source, JOIN *join, int ref_slice)
    : PX_receiver(thd, receiver_id, pei, join, move(source), tables, ref_slice),
      m_join(join),
      m_ref_slice(ref_slice),
      m_sort_param(nullptr),
      m_sort(sort),
      m_min_records(nullptr),
      m_record_groups(nullptr),
      m_heap(nullptr),
      m_init_heap(false),
      m_tmp_key(nullptr) {}

bool PX_receiver_merge::Init() {
  /* Register the PX_process to PX_exchange_info and attach to responding channels. */
  if (PX_receiver::Init()) return true;

  TABLE *table = get_table();
  /*
    1) Generate the sort_order and sort_param
    2) Generate the compare keys
    3) Alloc space for merge sort structures
  */
  int curr_slice = m_join->current_ref_item_slice;
  SwitchSlice(m_join, m_ref_slice);

  if (m_sort && m_sort->m_order) {
    // generate sort_order.
    int s_length = m_sort->px_make_sortorder(m_sort->m_order, false);
    if (!s_length) {
      assert(0);
      goto err;
    }

    // generate sort_param.
    m_sort_param = new (thd()->mem_root) Sort_param();
    if (!m_sort_param) {
      my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "PX_receiver_merge::Init()");
      goto err;
    }

    m_sort_param->init_for_filesort(
        m_sort, make_array(m_sort->sortorder, s_length),
        sortlength(thd(), m_sort->sortorder, s_length), {table}, senders(),
        false);

    m_sort_param->local_sortorder =
        Bounds_checked_array<st_sort_field>(m_sort->sortorder, s_length);
  }

  SwitchSlice(m_join, curr_slice);

  if (m_sort_param) {
    int key_len = m_sort_param->max_record_length() + 1;
    keys[0] = new (thd()->mem_root) uchar[key_len];
    if (keys[0] == nullptr) {
      my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "PX_receiver_merge::Init()");
      goto err;
    }
    keys[1] = new (thd()->mem_root) uchar[key_len];
    if (keys[1] == nullptr) {
      my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "PX_receiver_merge::Init()");
      goto err;
    }

    memset(keys[0], 0, key_len);
    memset(keys[1], 0, key_len);
  }

  if (alloc()) {
    goto err;
  }

  return false;

err:
  if (thd()->killed) {
    thd()->send_kill_message();
  }
  End();
  return true;
}

/**
  Get the min/max record from the channels.

  @return 1 error occurs or killed, -1 for EOF , 0 for read success.
*/
int PX_receiver_merge::Read() {
  assert(get_pei()->format() == PX_COMPACT_ROW);
  int result = 0;
  mq_record_st *record = get_min_record();

  // TODO: There is no distinction between error and EOF cases.
  if (!record) {
    result = -1;
    goto end;
  }

  if (get_pei()->format() == PX_COMPACT_ROW) {
    result = get_codec()->decode(record->m_data, record->m_length);
    if (result) goto end;
  }

  return 0;
end:
  if (thd()->killed) {
    thd()->send_kill_message();
  }
  End();
  return result;
}

/**
  Clean for PX_receiver_merge.
*/
void PX_receiver_merge::End() {
  PX_receiver::End();

  if (m_heap) {
    m_heap->cleanup();
    destroy(m_heap);
    m_heap = nullptr;
  }

  if (m_sort_param) {
    destroy(m_sort_param);
    m_sort_param = nullptr;
  }
}

/**
  Alloc space for sort-related structures, and
  then init the binary heap.

  @param false if success, true if fail.
*/
bool PX_receiver_merge::alloc() {
  uint i, j;
  uint readers = senders();

  m_min_records = new (thd()->mem_root) mq_record_st *[readers] { NULL };
  if (!m_min_records) goto err;

  for (i = 0; i < readers; i++) {
    m_min_records[i] = new (thd()->mem_root) mq_record_st();
    if (!m_min_records[i]) goto err;
  }

  m_record_groups = new (thd()->mem_root) mq_records_batch_st[readers];
  if (!m_record_groups) goto err;

  for (i = 0; i < readers; i++) {
    m_record_groups[i].records =
        new (thd()->mem_root) mq_record_st *[MAX_RECORD_STORE] { NULL };
    if (!m_record_groups[i].records) goto err;

    for (j = 0; j < MAX_RECORD_STORE; j++) {
      m_record_groups[i].records[j] = new (thd()->mem_root) mq_record_st();
      if (!m_record_groups[i].records[j]) goto err;

      m_record_groups[i].records[j]->m_data =
          new (thd()->mem_root) uchar[RECORD_BUFFER_SIZE];
      if (!m_record_groups[i].records[j]->m_data) goto err;
    }
  }

  m_heap = new (thd()->mem_root)
      binary_heap(readers + 1, this, heap_compare_records, thd());
  if (!m_heap || m_heap->init_binary_heap()) goto err;

  return false;

err:
  my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "PX_receiver_merge::alloc()");
  return true;
}

mq_record_st *PX_receiver_merge::get_min_record() {
  int i;

  /*
    If the binary heap has not built, build heap firstly.
    If we have built the heap:
      1) obtain the top element of binary heap, and get the sender no.
      2) try to read a record from sender in a blocking mode. If get a
      record, push it into binary heap and adjust the binary heap; otherwise
      there is no more record in message queue of the sender, remove the
      i-th worker from binary heap.
    Then we can fetch the top element of binary heap
  */
  if (!m_init_heap) {
    build_heap();
  } else {
    i = m_heap->first();
    if (read_group(i, false)) {
      m_heap->replace_first(i);
    } else {
      m_heap->remove_first();
    }
  }

  if (m_heap->empty()) {
    return nullptr;
  } else {
    i = m_heap->first();
    return m_min_records[i];
  }
}

void PX_receiver_merge::build_heap() {
  uint ngroups = senders();

  for (uint i = 0; i < ngroups; i++) {
    m_record_groups[i].completed = false;
    m_record_groups[i].n_read = 0;
    m_record_groups[i].n_total = 0;
    m_min_records[i]->m_data = nullptr;
  }

  // Reset for binary heap.
  m_heap->reset();
  /*
    when nowait = false, ensure that each slot has
    one record through read message from MQ in a blocking mode.
  */
  bool nowait = false;

reread:
  for (uint i = 0; i < ngroups; i++) {
    if (!m_record_groups[i].completed) {
      if (m_min_records[i]->m_data == nullptr) {
        if (read_group(i, nowait)) m_heap->add_unorderd(i);
      } else {
        load_group_records(i);
      }
    }
  }

  /** recheck each slot in m_records */
  for (uint i = 0; i < ngroups; i++) {
    if (!m_record_groups[i].completed && !m_min_records[i]->m_data) {
      nowait = false;
      goto reread;
    }
  }

  /** build the binary heap */
  m_heap->build();
  m_init_heap = true;
}

bool PX_receiver_merge::read_group(uint id, bool nowait) {
  assert(id < senders());
  mq_records_batch_st *rec_group = &m_record_groups[id];

  /* the record has been fetched into records_batch. */
  if (rec_group->n_read < rec_group->n_total) {
    m_min_records[id] = rec_group->records[rec_group->n_read++];
    return true;
  } else if (rec_group->completed) {
    return false;
  } else {
    if (rec_group->n_read == rec_group->n_total) {
      rec_group->n_read = rec_group->n_total = 0;
    }

    /* fetch the record from the id-th message queue. */
    int i = rec_group->n_read;
    if (!load_group_record(id, i, &rec_group->completed, nowait)) {
      return false;
    }

    rec_group->n_total++;
    m_min_records[id] = rec_group->records[rec_group->n_read++];

    /** load a batch of records into the id-th record group */
    load_group_records(id);
    return true;
  }
}

void PX_receiver_merge::load_group_records(int id) {
  mq_records_batch_st *rec_group = &m_record_groups[id];
  /* try to read message from message queue with a non-blocking mode */
  for (int i = rec_group->n_total; i < MAX_RECORD_STORE; i++) {
    if (!load_group_record(id, i, &rec_group->completed, true)) break;
    // now, read a new message
    rec_group->n_total++;
  }
}

bool PX_receiver_merge::load_group_record(uint32 id, int i, bool *completed, bool nowait) {
  auto handle = m_handles[id];
  uchar *data = nullptr;
  Size msg_len = 0;

  if (completed) {
    *completed = false;
  }

  /** receive one message from message queue. */
  PX_io_error result = handle->receive((void **)&data, &msg_len, nowait);
  mq_records_batch_st *rec_group = &m_record_groups[id];

  if (result == PX_IO_EOF) {
    rec_group->completed = true;
    return false;
  }

  if (result == PX_IO_WOULD_BLOCK) {
    return false;
  }

  // copy data into m_record_groups[id].records[i]
  if (store_mq_record(rec_group->records[i], data, msg_len)) {
    return true;
  }

  return false;
}

/**
  convert the message from table->record[0] as mq_record_struct
  @param rec the struct to be doing the binary heap sort
  @param data table->record[0]
  @param msg_len record length

  @return true if success, false if fail.
*/
bool PX_receiver_merge::store_mq_record(mq_record_st *rec, uchar *data, uint32 msg_len) {
  /*
    Making a deep copy from data to rec. firstly, determine whether rec has
    enough space to copy data. If there is not enough space, alloc it, everytime
    we make the new_buffer_len = old_buffer_len * 2.
  */
  if (msg_len > rec->m_buffer_len) {
    if (rec->m_data) {
      destroy(rec->m_data);
    }

    uint32 new_buffer_len = rec->m_buffer_len;
    while (msg_len > new_buffer_len) {
      new_buffer_len *= 2;
    }

    rec->m_data = new (thd()->mem_root) uchar[new_buffer_len];
    if (!rec->m_data) {
      goto err;
    }

    rec->m_buffer_len = new_buffer_len;
  }

  memcpy(rec->m_data, data, msg_len);
  rec->m_length = msg_len;
  return true;

err:
  my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "(PX_receiver_merge::store_mq_record)");
  return false;
}

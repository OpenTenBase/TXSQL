#include "px_receiver_merge.h"
#include "px_receiver.h"
#include "px_mq.h"
#include "px_exchange_info.h"
#include "px_codec.h"
#include "sql/filesort.h"
#include "sql/sql_optimizer.h"
#include "sql/log.h"
#include "sql/sql_optimizer.h"  // JOIN

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
  TABLE *table = get_table();
  uint curr_slice = 0;

  /*
    Register the PX_process to PX_exchange_info and attach to responding channels.
    Notes: The End() of PX_receiver should have been called in PX_receiver::Init
    if error occurs.
  */
  if (PX_receiver::Init()) return true;
  assert(thd()->px_receiver);
  thd()->px_receiver = this;

  /*
    1) Generate the sort_order and sort_param
    2) Generate the compare keys
    3) Alloc space for merge sort structures
  */
  curr_slice = m_join->current_ref_item_slice;
  SwitchSlice(m_join, m_ref_slice);

  if (m_sort && m_sort->m_order) {
    // generate sort_order.
    uint s_length = m_sort->px_make_sortorder(m_sort->m_order, false);
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
  
    /*
      When sorting using priority queue, we cannot use packed addons.
      Without PQ, we can try.
    */
    m_sort_param->try_to_pack_addons();
  }

  SwitchSlice(m_join, curr_slice);

  if (m_sort_param) {
    // Don't write handler::ref to sort key now.
    m_sort_param->px_skip_write_ref();
    /*
      Parallel query merge sort will use two temporary buffers to save the sort key.
      By default, the length of the sort key is set to Sort_param::m_fixed_rec_length.
      For sorting fields of json type, Sort_param::m_fixed_rec_length will be set to 4G,
      so the maximum sort key length of json type will be explicitly limited.
    */
    uint key_len = m_sort_param->max_record_length() == UINT_MAX ?
        MAX_SORT_LENGTH : m_sort_param->max_record_length() + 1;

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
  thd()->px_receiver = nullptr;
  return true;
}

/**
  Get the min/max record from the channels.

  @return 1 error occurs or killed, -1 for EOF , 0 for read success.
*/
int PX_receiver_merge::Read() {
  assert(thd()->px_receiver == this && get_pei()->format() == PX_COMPACT_ROW);
  int result = 0;
  mq_record_st *min_rec = nullptr;

  result = get_min_record(&min_rec);
  if (result != 0) goto end;

  assert(min_rec && min_rec->m_data);
  if (get_pei()->format() == PX_COMPACT_ROW) {
    result = get_codec()->decode(min_rec->m_data, min_rec->m_length);
    if (result) goto end;
  }

  return 0;
end:
  if (thd()->killed) {
    thd()->send_kill_message();
  }
  End();
  thd()->px_receiver = nullptr;
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

int PX_receiver_merge::get_min_record(mq_record_st **record) {
  int i;
  int result = 0;
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
    result = build_heap();
    if (result == 1) return result;
  } else {
    i = m_heap->first();
    result = read_group(i, false);

    if (result == 0) {
      m_heap->replace_first(i);
    } else if (result == -1) {
      m_heap->remove_first();
    } else {
      return result;
    }
  }

  if (m_heap->empty()) {
    *record = nullptr;
    return -1;
  } else {
    i = m_heap->first();
    *record = m_min_records[i];
  }

  return 0;
}

int PX_receiver_merge::build_heap() {
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
  int result = 0;

reread:
  for (uint i = 0; i < ngroups; i++) {
    if (!m_record_groups[i].completed) {
      if (m_min_records[i]->m_data == nullptr) {
        result = read_group(i, nowait);
        /*
          Try to read valid records from the corresponding channel of each
          producer and load them into the record group. If the read reaches
          EOF, it means that the producer does not need to be added to the heap.
        */
        if (result == 0) {
          m_heap->add_unorderd(i);
        } else if (result == -1) {
          continue;
        } else {
          return result;
        }
      } else {
        result = load_group_records(i);
        if (result == 1) return result;
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
  return 0;
}

/*
  Attempt to read records from the record group. If there is a cached
  record in the record group, load it directly into m_min_records and
  return 0; otherwise, return -1 when it is found that the EOF has
  been read; otherwise, read the record from the corresponding channel
  into the record group.

  @return 0 read a valid record, -1 read eof, 1 error occurs
*/
int PX_receiver_merge::read_group(uint id, bool nowait) {
  assert(id < senders());
  mq_records_batch_st *rec_group = &m_record_groups[id];

  /* the record has been fetched into records_batch. */
  if (rec_group->n_read < rec_group->n_total) {
    m_min_records[id] = rec_group->records[rec_group->n_read++];
    return 0;
  } else if (rec_group->completed) {
    return -1;
  } else {
    if (rec_group->n_read == rec_group->n_total) {
      rec_group->n_read = rec_group->n_total = 0;
    }

    /* fetch the record from the id-th message queue. */
    uint i = rec_group->n_read;
    int result = load_group_record(id, i, &rec_group->completed, nowait);
    if (result != 0) {
      return result;
    }

    rec_group->n_total++;
    m_min_records[id] = rec_group->records[rec_group->n_read++];

    /* load a batch of records into the id-th record group */
    result = load_group_records(id);
    /*
      This position indicates that there must be at least one valid
      record in the record group, so unless load_group_records
      reports an error (the return value is equal to 1), it returns 0
    */
    return (result == 1) ? 1 : 0;
  }
}

int PX_receiver_merge::load_group_records(uint id) {
  mq_records_batch_st *rec_group = &m_record_groups[id];
  /* try to read message from message queue with a non-blocking mode */
  for (uint i = rec_group->n_total; i < MAX_RECORD_STORE; i++) {
    // Only the return value equal to 0 can ensure that a valid record is read into the record group
    int result = load_group_record(id, i, &rec_group->completed, true);
    if (result != 0) {
      return result;
    }
    // now, read a new message
    rec_group->n_total++;
  }
  return 0;
}

/*
  Attempt to read a record into the record group.
    1) If the record is successfully read from the corresponding channel, store it in the record buffer and return 0
    2) If it is found to read EOF, return -1 directly
    3) Read the error from the exchange channel or transfer it to the record buffer to report an error, return 1
    4) Reading records from channel shows PX_IO_WOULD_BLOCK, returns 2

  @return  0 load a valid record to record group
*/
int PX_receiver_merge::load_group_record(uint32 id, int i, bool *completed, bool nowait) {
  auto handle = m_handles[id];
  uchar *data = nullptr;
  Size msg_len = 0;
  int result = 0;

  if (completed) {
    *completed = false;
  }

  /** receive one message from message queue. */
  PX_io_error err = handle->receive((void **)&data, &msg_len, nowait);
  mq_records_batch_st *rec_group = &m_record_groups[id];

  DBUG_EXECUTE_IF("px_load_group_record_error1", {
    thd()->killed = THD::KILL_QUERY;
    err = PX_IO_ERROR;
  });

  switch (err) {
    case PX_IO_OK: {
      result = 0;
      goto load_to_group;
      break;
    }
    case PX_IO_EOF: {
      result = -1;
      rec_group->completed = true;
      goto end;
      break;
    }
    case PX_IO_WOULD_BLOCK: {
      result = -2;
      goto end;
      break;
    }
    case PX_IO_ERROR: {
      result = 1;
      goto end;
      break;
    }
    default: {
      assert(0);
      result = 1;
      goto end;
      break;
    }
  }

load_to_group:
  assert(err == PX_IO_OK && result == 0);
  // copy data into m_record_groups[id].records[i]
  if (!store_mq_record(rec_group->records[i], data, msg_len)) {
    return 1;
  }

end:
  return result;
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

  DBUG_EXECUTE_IF("px_store_mq_record_error", {
    goto err;
  });

  memcpy(rec->m_data, data, msg_len);
  rec->m_length = msg_len;
  return true;

err:
  my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "PX_receiver_merge::store_mq_record()");
  return false;
}

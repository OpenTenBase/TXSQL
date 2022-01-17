// #include "px_receiver_merge.h"
// #include "px_mq.h"
// #include "px_exchange_info.h"
// #include "sql/filesort.h"
// #include "sql/log.h"

// PX_receiver_merge::PX_receiver_merge()
//     : PX_receiver(),
//       m_sort_param(nullptr),
//       m_sort(nullptr),
//       m_min_records(nullptr),
//       m_record_groups(nullptr),
//       m_heap(nullptr),
//       m_init_heap(false),
//       m_tmp_key(nullptr) {}

// PX_receiver_merge::PX_receiver_merge(uint receiver_no, PX_exchange_info *pei,
//     THD *thd, TABLE *table, Filesort *sort)
//     : PX_receiver(receiver_no, pei, thd, table),
//       m_sort_param(nullptr),
//       m_sort(sort),
//       m_min_records(nullptr),
//       m_record_groups(nullptr),
//       m_heap(nullptr),
//       m_init_heap(false),
//       m_tmp_key(nullptr) {}

// /**
//   Init message queue handlers and then init sort-related structure

//   @return false if success, true if fail.
// */
// bool PX_receiver_merge::init() {
//   if (PX_receiver::init()) return true;

//   THD *thd = get_thd();
//   TABLE *table = get_table();
//   /*
//     1) Generate the sort_order and sort_param
//     2) Generate the compare keys
//     3) Alloc space for merge sort structures
//   */
//   if (m_sort && m_sort->m_order) {
//     // generate sort_order.
//     int s_length = m_sort->px_make_sortorder(m_sort->m_order, false);
//     if (!s_length) return true;

//     // generate sort_param.
//     m_sort_param = new (thd->mem_root) Sort_param();
//     if (!m_sort_param) return true;

//     m_sort_param->init_for_filesort(
//         m_sort, make_array(m_sort->sortorder, s_length),
//         sortlength(thd, m_sort->sortorder, s_length), {table}, senders(),
//         false);

//     m_sort_param->local_sortorder =
//         Bounds_checked_array<st_sort_field>(m_sort->sortorder, s_length);
//   }

//   if (m_sort_param) {
//     int key_len = m_sort_param->max_record_length() + 1;
//     keys[0] = new (thd->mem_root) uchar[key_len];
//     keys[1] = new (thd->mem_root) uchar[key_len];

//     if (keys[0] == nullptr || keys[1] == nullptr) return true;

//     memset(keys[0], 0, key_len);
//     memset(keys[1], 0, key_len);
//   }

//   return alloc();
// }

// /**
//   Get the min/max record and then convert to mysql format.

//   @return true error occurs or killed, fail read success!
// */
// bool PX_receiver_merge::next() {
//   mq_record_st *record = get_min_record();

//   if (!record) {
//     get_pei()->detach_receiver(get_receiver_no());
//     return true;
//   }

//   if (get_pei()->format() == PX_COMPACT_ROW) {
//     decompact_row(record->m_data, record->m_length);
//   } else {
//     // Has not support yet!
//     DBUG_ASSERT(0);
//   }

//   return true;
// }

// /**
//   Alloc space for sort-related structures, and
//   then init the binary heap.

//   @param false if success, true if fail.
// */
// bool PX_receiver_merge::alloc() {
//   uint i, j;
//   uint readers = senders();
//   THD *thd = get_thd();

//   m_min_records = new (thd->mem_root) mq_record_st *[readers] { NULL };
//   if (!m_min_records) goto err;

//   for (i = 0; i < readers; i++) {
//     m_min_records[i] = new (thd->mem_root) mq_record_st();
//     if (!m_min_records[i]) goto err;
//   }

//   m_record_groups = new (thd->mem_root) mq_records_batch_st[readers];
//   if (!m_record_groups) goto err;

//   for (i = 0; i < readers; i++) {
//     m_record_groups[i].records =
//         new (thd->mem_root) mq_record_st *[MAX_RECORD_STORE] { NULL };
//     if (!m_record_groups[i].records) goto err;

//     for (j = 0; j < MAX_RECORD_STORE; j++) {
//       m_record_groups[i].records[j] = new (thd->mem_root) mq_record_st();
//       if (!m_record_groups[i].records[j]) goto err;

//       m_record_groups[i].records[j]->m_data =
//           new (thd->mem_root) uchar[RECORD_BUFFER_SIZE];
//       if (!m_record_groups[i].records[j]->m_data) goto err;
//     }
//   }

//   m_heap = new (thd->mem_root)
//       binary_heap(readers + 1, this, heap_compare_records, thd);
//   if (!m_heap || m_heap->init_binary_heap()) goto err;

//   return false;

// err:
//   sql_print_error("error in allocate space in PX_receiver_merge::alloc().");
//   return true;
// }

// mq_record_st *PX_receiver_merge::get_min_record() {
//   int i;

//   /*
//     If the binary heap has not built, build heap firstly.
//     If we have built the heap:
//       1) obtain the top element of binary heap, and get the sender no.
//       2) try to read a record from sender in a blocking mode. If get a
//       record, push it into binary heap and adjust the binary heap; otherwise
//       there is no more record in message queue of the sender, remove the 
//       i-th worker from binary heap.
//     Then we can fetch the top element of binary heap
//   */
//   if (!m_init_heap) {
//     build_heap();
//   } else {
//     i = m_heap->first();
//     if (read_group(i, false)) {
//       m_heap->replace_first(i);
//     } else {
//       m_heap->remove_first();
//     }
//   }

//   if (m_heap->empty()) {
//     return nullptr;
//   } else {
//     i = m_heap->first();
//     return m_min_records[i];
//   }
// }

// void PX_receiver_merge::build_heap() {
//   uint ngroups = senders();

//   for (uint i = 0; i < ngroups; i++) {
//     m_record_groups[i].completed = false;
//     m_record_groups[i].n_read = 0;
//     m_record_groups[i].n_total = 0;
//     m_min_records[i]->m_data = nullptr;
//   }

//   // Reset for binary heap.
//   m_heap->reset();
//   /*
//     when nowait = false, ensure that each slot has
//     one record through read message from MQ in a blocking mode.
//   */
//   bool nowait = false;

// reread:
//   for (uint i = 0; i < ngroups; i++) {
//     if (!m_record_groups[i].completed) {
//       if (m_min_records[i]->m_data == nullptr) {
//         if (read_group(i, nowait)) m_heap->add_unorderd(i);
//       } else {
//         load_group_records(i);
//       }
//     }
//   }

//   /** recheck each slot in m_records */
//   for (uint i = 0; i < ngroups; i++) {
//     if (!m_record_groups[i].completed && !m_min_records[i]->m_data) {
//       nowait = false;
//       goto reread;
//     }
//   }

//   /** build the binary heap */
//   m_heap->build();
//   m_init_heap = true;
// }

// bool PX_receiver_merge::read_group(uint id, bool nowait) {
//   DBUG_ASSERT(id < senders());
//   mq_records_batch_st *rec_group = &m_record_groups[id];

//   /* the record has been fetched into records_batch. */
//   if (rec_group->n_read < rec_group->n_total) {
//     m_min_records[id] = rec_group->records[rec_group->n_read++];
//     return true;
//   } else if (rec_group->completed) {
//     return false;
//   } else {
//     if (rec_group->n_read == rec_group->n_total) {
//       rec_group->n_read = rec_group->n_total = 0;
//     }

//     /* fetch the record from the id-th message queue. */
//     int i = rec_group->n_read;
//     if (!load_group_record(id, i, &rec_group->completed, nowait)) {
//       return false;
//     }

//     rec_group->n_total++;
//     m_min_records[id] = rec_group->records[rec_group->n_read++];

//     /** load a batch of records into the id-th record group */
//     load_group_records(id);
//     return true;
//   }
// }

// void PX_receiver_merge::load_group_records(int id) {
//   mq_records_batch_st *rec_group = &m_record_groups[id];
//   /* try to read message from message queue with a non-blocking mode */
//   for (int i = rec_group->n_total; i < MAX_RECORD_STORE; i++) {
//     if (!load_group_record(id, i, &rec_group->completed, true)) break;
//     // now, read a new message
//     rec_group->n_total++;
//   }
// }

// bool PX_receiver_merge::load_group_record(uint32 id, int i, bool *completed, bool nowait) {
//   auto channel = m_channels[id];
//   uchar *data = nullptr;
//   Size msg_len = 0;

//   if (completed) {
//     *completed = false;
//   }

//   /** receive one message from message queue. */
//   PX_mq_result result = (PX_mq_result)channel->receive((void **)&data, &msg_len, nowait);
//   mq_records_batch_st *rec_group = &m_record_groups[id];

//   if (result == PX_MQ_DETACHED) {
//     rec_group->completed = true;
//     return false;
//   }

//   if (result == PX_MQ_WOULD_BLOCK) {
//     return false;
//   }

//   // copy data into m_record_groups[id].records[i]
//   if (store_mq_record(rec_group->records[i], data, msg_len)) {
//     return true;
//   }

//   return false;
// }

// /**
//   convert the message from table->record[0] as mq_record_struct
//   @param rec the struct to be doing the binary heap sort
//   @param data table->record[0]
//   @param msg_len record length

//   @return true if success, false if fail.
// */
// bool PX_receiver_merge::store_mq_record(mq_record_st *rec, uchar *data, uint32 msg_len) {
//   THD *thd = get_thd();

//   /*
//     Making a deep copy from data to rec. firstly, determine whether rec has
//     enough space to copy data. If there is not enough space, alloc it, everytime
//     we make the new_buffer_len = old_buffer_len * 2.
//   */
//   if (msg_len > rec->m_buffer_len) {
//     if (rec->m_data) {
//       delete []rec->m_data;
//     }

//     uint32 new_buffer_len = rec->m_buffer_len;
//     while (msg_len > new_buffer_len) {
//       new_buffer_len *= 2;
//     }
  
//     rec->m_data = new (thd->mem_root) uchar[new_buffer_len];
//     if (!rec->m_data) {
//       goto err;
//     }

//     rec->m_buffer_len = new_buffer_len;
//   }

//   memcpy(rec->m_data, data, msg_len);
//   rec->m_length = msg_len;
//   return true;

// err:
//   my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "(PX_receiver_merge::store_mq_record)");
//   return false;
// }


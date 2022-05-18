#include "sql/item.h"
#include "sql/sql_class.h"
#include "px_sender.h"
#include "px_exchange_info.h"
#include "px_exchange_channel.h"
#include "px_executor.h"
#include "px_codec.h"
#include "include/my_dbug.h"
#include "sql/query_result.h"
#include "sql/log.h"
#include "sql/sql_class.h"
#include "field_types.h"

PX_sender::PX_sender(THD *thd, uint sender_no, PX_exchange_info *pei,
                     unique_ptr_destroy_only<RowIterator> source, TABLE *table,
                     mem_root_deque<Item *> *send_fields,
                     mem_root_deque<Item *> *shuffle_key,
                     Temp_table_param *temp_table_param,  bool use_item,
                     unique_ptr_destroy_only<RowIterator> table_path)
    : RowIterator(thd),
      m_source(move(source)),
      m_materialize(table_path),
      m_table_path(move(table_path)),
      m_pei(pei),
      m_sender_id(0),
      m_handles(Malloc_allocator<PSI_memory_key>(PSI_INSTRUMENT_ME)),
      m_use_item(use_item),
      m_temp_table_param(temp_table_param),
      m_table(table),
      m_fields(),
      m_send_fields(send_fields),
      m_reshuffle_key(shuffle_key) {}

PX_proc *PX_sender::me() const { return thd()->px_executor->proc(); }

bool PX_sender::Init() {
  assert(m_pei && !m_codec && m_handles.empty());

  // Register and get receiver id.
  if (m_pei->register_proc(me(), /*as_sender=*/true, m_sender_id)) {
    goto err;
  }

  // Attach to all connected channels.
  if (m_pei->attach(me(), /*as_sender=*/true, m_sender_id, m_handles)) {
    goto err;
  }

  // Create codec.
  switch (m_pei->format()) {
    case PX_COMPACT_ROW: {
      m_codec = new (thd()->mem_root) PX_compact_codec(thd(), m_use_item, m_temp_table_param);
      if (m_codec == nullptr) {
        my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "creating codec",
                 "PX_sender::register_to_exchange()");
        goto err;
      }
      break;
    }
    default:
      assert(0);
      goto err;
  }

  if (m_source->Init()) {
    goto err;
  }

  if (m_materialize) {
    if (!m_table->is_created()) {
      if (instantiate_tmp_table(thd(), m_table)) {
        goto err;
      }
      empty_record(m_table);
    } else {
      m_table->file->ha_index_or_rnd_end();  // @todo likely unneeded => remove
      m_table->file->ha_delete_all_rows();
    }

    while (true) {
      int error = m_source->Read();
      if (error > 0 || thd()->is_error())
        goto err;
      else if (error < 0)
        break;
      else if (thd()->killed) {
        thd()->send_kill_message();
        goto err;
      }

      error = m_table->file->ha_write_row(m_table->record[0]);
      if (error == 0) {
        continue;
      }

      // create_ondisk_from_heap will generate error if needed.
      if (!m_table->file->is_ignorable_error(error)) {
        bool is_duplicate;
        if (create_ondisk_from_heap(thd(), m_table, error, true, true, &is_duplicate))
          goto err; /* purecov: inspected */
        // Table's engine changed; index is not initialized anymore.
        if (m_table->hash_field) m_table->file->ha_index_init(0, false);
        // if (!is_duplicate) ++*stored_rows;
      } else {
        // An ignorable error means duplicate key, ie. we deduplicated
        // away the row. This is seemingly separate from
        // check_unique_constraint(), which only checks hash indexes.
      }
    }
    if (m_table_path->Init()) goto err;
  }

  // Set up codec with its input items or fields.
  if (m_use_item) {
    assert(m_send_fields->size());
    if (m_codec->init(m_send_fields, nullptr)) goto err;
  } else {
    for (Field **pfield = m_table->field; *pfield != nullptr; ++pfield) {
      Field *field = *pfield;
      if (bitmap_is_set(m_table->read_set, field->field_index()))
        m_fields.push_back(field);
    }
    if (m_codec->init(nullptr, &m_fields)) goto err;
  }

  return false;

err:
  detach();
  return true;
}

/**
  Send data to selected channels.

  @return 0 for success, -1 for EOF and 1 for error
*/
int PX_sender::Read() {
  std::vector<PX_iovec> out_fields;
  int result = 1;
#ifndef DBUG_OFF
  String buf;
#endif

  if (!m_materialize) result = m_source->Read();
  else result = m_table_path->Read();

  if (result || thd()->killed) goto end;

  // Convert read data to protocol format.
  if (m_codec->encode(out_fields)) {
    result = 1;
    goto end;
  }

#ifndef DBUG_OFF
  for (auto &handle : m_handles) {
    buf.append(" ");
    buf.append_ulonglong(handle->channel_id());
  }
  PX_PRINT_DEBUG("send to channels%s", buf.c_ptr_safe());
#endif

  for (auto &handle: m_handles) {
    // TODO skip unmatch handle m_reshuffle_key m_sender_id

    PX_io_error error = handle->send(out_fields.data(), out_fields.size(),
                                     /*nowait=*/false);

    // Never gets non-blocking because of it is a blocking send.
    assert(error != PX_IO_WOULD_BLOCK);

    /*
      Because data flow must be ended by the producer side, any send that is
      not successful indicates an error.
     */
    if (error != PX_IO_OK) {
      result = 1;
      goto end;
    }
  }

  result = 0;

end:
  if (result) detach();

  return result;
}

void PX_sender::End() {
  if (m_codec) {
    destroy(m_codec);
  }
}

void PX_sender::detach() {
  for (auto &handle : m_handles) {
    handle->detach();
  }
}

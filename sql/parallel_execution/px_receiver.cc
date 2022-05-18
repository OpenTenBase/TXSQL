#include "px_atomic.h"
#include "px_receiver.h"
#include "px_sender.h"
#include "px_exchange_info.h"
#include "px_executor.h"
#include "px_codec.h"
#include "include/my_dbug.h"
#include "sql/field.h"

#include "sql/sql_class.h"
#include "sql/sql_optimizer.h"

void SwitchSlice(JOIN *join, int slice_num) {
  if (-1 != slice_num && !join->ref_items[slice_num].is_null()) {
    join->set_ref_item_slice(slice_num);
  }
}

PX_receiver::PX_receiver(THD *thd, uint receiver_id, PX_exchange_info *pei,
  JOIN *join, unique_ptr_destroy_only<RowIterator> source,
  TABLE *table, int ref_slice)
    : RowIterator(thd),
      m_handles(Malloc_allocator<PSI_memory_key>(PSI_INSTRUMENT_ME)),
      m_join(join),
      m_source(move(source)),
      m_pei(pei),
      m_receiver_id(receiver_id),
      m_table(table),
      m_fields(),
      m_ref_slice(ref_slice) {}

PX_proc *PX_receiver::me() const { return thd()->px_executor->proc(); }

bool PX_receiver::Init() {
  assert(m_pei && !m_codec && m_handles.empty());

  // Register and get receiver id.
  if (m_pei->register_proc(me(), /*as_sender=*/false, m_receiver_id)) {
    goto err;
  }

  // Attach to all connected channels.
  if (m_pei->attach(me(), /*as_sender=*/false, m_receiver_id, m_handles)) {
    goto err;
  }

  // Create codec and set its output fields.
  for (Field **pfield = m_table->field; *pfield != nullptr; ++pfield) {
    Field *field = *pfield;
    if (bitmap_is_set(m_table->read_set, field->field_index()))
      m_fields.push_back(field);
  }
  switch (m_pei->format()) {
    case PX_COMPACT_ROW: {
      m_codec = new (thd()->mem_root) PX_compact_codec(thd(), /*use_item=*/false, nullptr);
      if (m_codec == nullptr) {
        my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "creating codec", "PX_receiver::Init()");
        goto err;
      }
      if (m_codec->init(nullptr, &m_fields)) {
        goto err;
      }
      break;
    }
    default:
      assert(0);
      goto err;
  }

  if (m_join) {
    m_input_slice = m_join->get_ref_item_slice();
  }

  return false;

err:
  detach();
  return true;
}

#ifndef DBUG_OFF
static const char* cstr(THD::killed_state state) {
  const char *s;
  switch (state) {
    case THD::NOT_KILLED:
      s = "NOT_KILLED";
      break;
    case THD::KILL_CONNECTION:
      s = "KILL_CONNECTION(ER_SERVER_SHUTDOWN)";
      break;
    case THD::KILL_QUERY:
      s = "KILL_QUERY(ER_QUERY_INTERRUPTED)";
      break;
    case THD::KILL_TIMEOUT:
      s = "KILL_TIMEOUT(ER_QUERY_TIMEOUT)";
      break;
    case THD::KILLED_NO_VALUE:
      s= "KILLED_NO_VALUE";
      break;
    default:
      s = "???";
      assert(0);
      break;
  }
  return s;
}
#endif

/**
  Read from all channels.

  @return 0 for success, -1 for EOF and 1 for error
*/
int PX_receiver::Read() {
  int result = 0;
  uchar *data = nullptr;
  Size len = 0;
  if (m_join) SwitchSlice(m_join, m_input_slice);

  assert(m_pei->format() == PX_COMPACT_ROW);
  result = receive((void **)&data, &len);

  if (result || thd()->killed) goto err;

  // Decode received data to table fields.
#ifndef DBUG_OFF
  memset(m_table->record[0], 255, m_table->s->reclength);
#endif
  if (m_codec->decode(data, len)) {
    assert(0);
    return 1;
  }

  if (m_join) SwitchSlice(m_join, m_ref_slice);

  PX_PRINT_DEBUG("read one row");
  return result;

err:
  PX_PRINT_ERROR("read error %d killed %s", result, cstr(thd()->killed));
  return result;  
}

void PX_receiver::End() {
  if (m_codec) {
    destroy(m_codec);
  }
}

/**
  Read from non-empty channels one after another, and wait if all are empty.

  @return 0 for success, -1 for EOF and 1 for error.
*/
int PX_receiver::receive(void **datap, Size *len) {
  int result = 1;

  for (;;) {
#ifndef DBUG_OFF
    String buf;
    for (auto &handle : m_handles) {
      buf.append(" ");
      buf.append_ulonglong(handle->channel_id());
    }
    PX_PRINT_DEBUG("receive from channels%s", buf.c_ptr_safe());
#endif

    if (thd()->is_killed()) {
      PX_PRINT_ERROR("receive killed");
      result = 1;
      goto end;
    }

    if (m_handles.empty()) {
      result = -1;
      goto end;
    }

    assert(m_cursor < m_handles.size());
    PX_exchange_handle *handle = m_handles[m_cursor];
    PX_io_error error = handle->receive(datap, len, /*nowait=*/true);
    switch (error) {
      case PX_IO_OK: {
        m_skip_count = 0;
        result = 0;
        goto end;
      }
      case PX_IO_WOULD_BLOCK: {
        // Non-blocking code suggests next channel.
        m_skip_count++;
        m_cursor++;
        if (m_cursor >= m_handles.size()) {
          m_cursor = 0;
        }
        // All are active but empty, just wait.
        if (m_skip_count >= m_handles.size()) {
          m_skip_count = 0;
          PX_proc *me = thd()->px_executor->proc();
          // FIXME Waiting without timeout might be also working here?
          WaitLatch(me, /*timeout=*/100);
          ResetLatch(me);
        }
        continue;
      }
      case PX_IO_ERROR: {
        // FIXME Is kill already handled elsewhere?
        result = 1;
        goto end;
      }
      case PX_IO_EOF: {
        // Detach before it becomes unreachable.
        handle->detach();

        auto pos = m_handles.begin();
        std::advance(pos, m_cursor);
        m_handles.erase(pos);
        if (m_cursor >= m_handles.size()) {
          m_cursor--;
        }
        if (m_skip_count >= m_handles.size()) {
          m_skip_count--;
        }
        assert(m_handles.empty() ||
               (m_cursor < m_handles.size() && m_skip_count < m_handles.size()));
        continue;
      }
      default: {
        assert(0);
        result = 1;
        goto end;
      }
    }
  }

end:
  assert((result == 0 && !m_handles.empty()) ||
         (result == 1 && !m_handles.empty()) ||
         (result == -1 && m_handles.empty()));

  // No more read remaining handles on ERROR
  if (result) detach();

  return result;
}

void PX_receiver::detach() {
  for (auto &handle : m_handles) {
    handle->detach();
  }
}

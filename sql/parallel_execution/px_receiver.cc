#include "px_atomic.h"
#include "px_receiver.h"
#include "px_sender.h"
#include "px_exchange_info.h"
#include "px_executor.h"
#include "px_codec.h"
#include "include/my_dbug.h"
#include "sql/field.h"
#include "scope_guard.h"  // create_scope_guard

#include "sql/sql_class.h"
#include "sql/sql_optimizer.h"

void SwitchSlice(JOIN *join, int slice_num) {
  if (slice_num >= 1 && !join->ref_items[slice_num].is_null()) {
    join->set_ref_item_slice(slice_num);
  }
}

PX_receiver::PX_receiver(THD *thd, uint receiver_id, PX_exchange_info *pei,
                         JOIN *join,
                         unique_ptr_destroy_only<RowIterator> source,
                         mem_root_deque<TABLE *> *tables, int ref_slice)
    : RowIterator(thd),
      m_handles(Malloc_allocator<PSI_memory_key>(PSI_INSTRUMENT_ME)),
      m_join(join),
      m_source(move(source)),
      m_pei(pei),
      m_receiver_id(receiver_id),
      m_tables(tables),
      m_fields(),
      m_ref_slice(ref_slice) {}

PX_proc *PX_receiver::me() const { return thd()->px_executor->proc(); }

bool PX_receiver::Init() {
  /*
    In some scenarios, a task may have a set of exchange receiver operators instead of
    one exchange receiver operator, such as multiple query blocks combined by union.
    For example: (select * from t1 order by a limit 10) union (select * from t2);
    In the first union branch of this query, due to the existence of limit, the exchange
    receiver operator itself cannot perceive the end of the query, so it will miss End
    call. The processing at this time includes two situations:
      1) If an exception occurs before the exchange receiver operator attach of the second
        branch, the task ends, and the End interface is explicitly called at the end of the
        task scheduling.
      2) If no exception occurs before the exchange receiver operator of the second branch
        is attached, the End interface of the exchange operator of the previous branch is
        explicitly called before the attach.
    Otherwise, the receiver of the previous branch may not be detached, so the sender cannot
    sense the end of the query and is blocked in data exchange; and after the receiver of
    the second branch is started, the receiver is blocked waiting for the sender to attach.
  */
  if (thd()->px_receiver) {
    assert(thd()->px_receiver != this);
    thd()->px_receiver->End();
    thd()->px_receiver = nullptr;
  }

  assert(m_pei && !m_codec && m_handles.empty() && !thd()->px_receiver);

  // Register and get receiver id.
  if (m_pei->register_proc(me(), /*as_sender=*/false, m_receiver_id)) {
    goto err;
  }

  // Attach to all connected channels.
  if (m_pei->attach(me(), /*as_sender=*/false, m_receiver_id, m_handles)) {
    goto err;
  }

  // Create codec and set its output fields.
  for (const TABLE *table : *m_tables) {
    for (Field **pfield = table->field; *pfield != nullptr; ++pfield) {
      Field *field = *pfield;
      if (bitmap_is_set(table->read_set, field->field_index()))
        m_fields.push_back(field);
    }
  }
  switch (m_pei->format()) {
    case PX_COMPACT_ROW: {
      m_codec = new (thd()->mem_root) PX_compact_codec(thd(), /*use_item=*/false, nullptr);

      DBUG_EXECUTE_IF("px_codec_create_error", {
        if (m_codec) destroy(m_codec);
        m_codec = nullptr;
      });

      if (m_codec == nullptr || DBUG_EVALUATE_IF("px_receiver_init_error1", true, false)) {
        my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "creating codec", "PX_receiver::Init()");
        goto err;
      }

      if (m_codec->init(nullptr, &m_fields, m_tables)) {
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

  thd()->px_receiver = this;
  return false;

err:
  /*
   Before PX_receiver exits, make sure to call the End function
   to clean up resources and detach from the exchange channel.
   Note: The End function can only be called once.
  */
  End();
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
  assert(thd()->px_receiver == this);
  int result = 0;
  uchar *data = nullptr;
  Size len = 0;

  auto switch_to_output_slice = create_scope_guard([&] {
    if (m_join) SwitchSlice(m_join, m_ref_slice);
  });
  if (m_join) SwitchSlice(m_join, m_input_slice);

  assert(m_pei->format() == PX_COMPACT_ROW);
  result = receive((void **)&data, &len);

  if (result || thd()->killed) goto err;

  // Decode received data to table fields.
#ifndef DBUG_OFF
  for (TABLE *table : *m_tables) {
    memset(table->record[0], 255, table->s->reclength);
  }
#endif
  if (m_codec->decode(data, len)) {
    assert(0);
    result = 1;
    goto err;
  }

  PX_PRINT_DEBUG("read one row");
  return result;

err:
  PX_PRINT_ERROR("read error %d killed %s", result, cstr(thd()->killed));
  if (thd()->killed) {
    thd()->send_kill_message();
  }
  End();
  thd()->px_receiver = nullptr;
  return result;
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
    // THD killed before receive data
    DBUG_EXECUTE_IF("px_error_before_receive_data", {
      thd()->killed = THD::KILL_QUERY;
    });

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
    // THD killed after receiver data
    DBUG_EXECUTE_IF("px_error_after_receive_data", {
      thd()->killed = THD::KILL_QUERY;
      error = PX_IO_ERROR;
    });

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

  return result;
}

void PX_receiver::End() {
  if (m_codec) {
    destroy(m_codec);
    m_codec = nullptr;
  }
  detach();
}

void PX_receiver::detach() {
  for (auto &handle : m_handles) {
    handle->detach();
  }
  m_handles.clear();
}

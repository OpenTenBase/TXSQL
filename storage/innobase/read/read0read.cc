/*****************************************************************************

Copyright (c) 1996, 2022, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file read/read0read.cc
 Cursor read

 Created 2/16/1997 Heikki Tuuri
 *******************************************************/

#include "read0read.h"
#include "clone0clone.h"

#include "srv0srv.h"
#include "trx0sys.h"

/*
-------------------------------------------------------------------------------
FACT A: Cursor read view on a secondary index sees only committed versions
-------
of the records in the secondary index or those versions of rows created
by transaction which created a cursor before cursor was created even
if transaction which created the cursor has changed that clustered index page.

PROOF: We must show that read goes always to the clustered index record
to see that record is visible in the cursor read view. Consider e.g.
following table and SQL-clauses:

create table t1(a int not null, b int, primary key(a), index(b));
insert into t1 values (1,1),(2,2);
commit;

Now consider that we have a cursor for a query

select b from t1 where b >= 1;

This query will use secondary key on the table t1. Now after the first fetch
on this cursor if we do a update:

update t1 set b = 5 where b = 2;

Now second fetch of the cursor should not see record (2,5) instead it should
see record (2,2).

We also should show that if we have delete t1 where b = 5; we still
can see record (2,2).

When we access a secondary key record maximum transaction id is fetched
from this record and this trx_id is compared to up_limit_id in the view.
If trx_id in the record is greater or equal than up_limit_id in the view
cluster record is accessed.  Because trx_id of the creating
transaction is stored when this view was created to the list of
trx_ids not seen by this read view previous version of the
record is requested to be built. This is build using clustered record.
If the secondary key record is delete-marked, its corresponding
clustered record can be already be purged only if records
trx_id < low_limit_no. Purge can't remove any record deleted by a
transaction which was active when cursor was created. But, we still
may have a deleted secondary key record but no clustered record. But,
this is not a problem because this case is handled in
row_sel_get_clust_rec() function which is called
whenever we note that this read view does not see trx_id in the
record. Thus, we see correct version. Q. E. D.

-------------------------------------------------------------------------------
FACT B: Cursor read view on a clustered index sees only committed versions
-------
of the records in the clustered index or those versions of rows created
by transaction which created a cursor before cursor was created even
if transaction which created the cursor has changed that clustered index page.

PROOF:  Consider e.g.following table and SQL-clauses:

create table t1(a int not null, b int, primary key(a));
insert into t1 values (1),(2);
commit;

Now consider that we have a cursor for a query

select a from t1 where a >= 1;

This query will use clustered key on the table t1. Now after the first fetch
on this cursor if we do a update:

update t1 set a = 5 where a = 2;

Now second fetch of the cursor should not see record (5) instead it should
see record (2).

We also should show that if we have execute delete t1 where a = 5; after
the cursor is opened we still can see record (2).

When accessing clustered record we always check if this read view sees
trx_id stored to clustered record. By default we don't see any changes
if record trx_id >= low_limit_id i.e. change was made transaction
which started after transaction which created the cursor. If row
was changed by the future transaction a previous version of the
clustered record is created. Thus we see only committed version in
this case. We see all changes made by committed transactions i.e.
record trx_id < up_limit_id. In this case we don't need to do anything,
we already see correct version of the record. We don't see any changes
made by active transaction except creating transaction. We have stored
trx_id of creating transaction to list of trx_ids when this view was
created. Thus we can easily see if this record was changed by the
creating transaction. Because we already have clustered record we can
access roll_ptr. Using this roll_ptr we can fetch undo record.
We can now check that undo_no of the undo record is less than undo_no of the
trancaction which created a view when cursor was created. We see this
clustered record only in case when record undo_no is less than undo_no
in the view. If this is not true we build based on undo_rec previous
version of the record. This record is found because purge can't remove
records accessed by active transaction. Thus we see correct version. Q. E. D.
-------------------------------------------------------------------------------
FACT C: Purge does not remove any delete-marked row that is visible
-------
in any cursor read view.

PROOF: We know that:
 1: Currently active read views in trx_sys_t::view_list are ordered by
    ReadView::low_limit_no in descending order, that is,
    newest read view first.

 2: Purge clones the oldest read view and uses that to determine whether there
    are any active transactions that can see the to be purged records.

Therefore any joining or active transaction will not have a view older
than the purge view, according to 1.

When purge needs to remove a delete-marked row from a secondary index,
it will first check that the DB_TRX_ID value of the corresponding
record in the clustered index is older than the purge view. It will
also check if there is a newer version of the row (clustered index
record) that is not delete-marked in the secondary index. If such a
row exists and is collation-equal to the delete-marked secondary index
record then purge will not remove the secondary index record.

Delete-marked clustered index records will be removed by
row_purge_remove_clust_if_poss(), unless the clustered index record
(and its DB_ROLL_PTR) has been updated. Every new version of the
clustered index record will update DB_ROLL_PTR, pointing to a new UNDO
log entry that allows the old version to be reconstructed. The
DB_ROLL_PTR in the oldest remaining version in the old-version chain
may be pointing to garbage (an undo log record discarded by purge),
but it will never be dereferenced, because the purge view is older
than any active transaction.

For details see: row_vers_old_has_index_entry() and row_purge_poss_sec()

Some additional issues:

What if trx_sys->view_list == NULL and some transaction T1 and Purge both
try to open read_view at same time. Only one can acquire trx_sys->mutex.
In which order will the views be opened? Should it matter? If no, why?

The order does not matter. No new transactions can be created and no running
RW transaction can commit or rollback (or free views). AC-NL-RO transactions
will mark their views as closed but not actually free their views.
*/


std::atomic<int64_t> ReadView::m_s_ts{1};


void CopyFreeSnapshot::init() {
  m_clock.store(VALID_BASE_TS);
  m_min_view_ts = VALID_BASE_TS;
  do_update_min();

  m_routine_running = true;
  m_update_min_thread = std::thread(&CopyFreeSnapshot::update_min_routine, this);
  m_convert_purge_thread = std::thread(&CopyFreeSnapshot::convert_snapshot_and_purge_hash_routine, this);
}

void CopyFreeSnapshot::destroy() {
  m_routine_running = false;
  m_update_min_thread.join();
  m_convert_purge_thread.join();
}

bool get_active_min(rw_trx_hash_element_t *element,
                    CopyFreeSnapshot::GetMinArg *arg){

  trx_t *elem_trx = element->trx.load();
  if (elem_trx == nullptr) {
    // the trx has finished and exited from hash,
    // ignore it when calculate low_limit_no and up_limit_id
    return false;
  }

  trx_id_t elem_no = element->no.load();
  if (elem_no != TRX_ID_MAX && elem_no < arg->low_limit_no) {
    arg->low_limit_no = elem_no;
  }

  if (element->id < arg->up_limit_id) {
    arg->up_limit_id = element->id;
  }
  return false;
}

// return false if nothing changed
bool CopyFreeSnapshot::do_update_min() {

  static trx_id_t prev_up_limit_id = 0;
  static trx_id_t prev_low_limit_no = 0;
  static trx_id_t prev_max_trx_id = 0;

  bool changed = true;

  // update m_up_limit_id, m_low_limit_no by iterating the rw_trx_hash
  trx_id_t max_trx_id = trx_sys->get_max_trx_id_safe();
  GetMinArg get_min_arg;
  get_min_arg.up_limit_id = max_trx_id;
  get_min_arg.low_limit_no = max_trx_id;
  get_min_arg.min_view_ts = get_clock();

  TRX_HASH_ITERATE(nullptr, get_active_min, &get_min_arg);

  m_up_limit_id = get_min_arg.up_limit_id;
  m_low_limit_no = get_min_arg.low_limit_no;

  if (prev_up_limit_id == m_up_limit_id
      && prev_low_limit_no == m_low_limit_no
      && prev_max_trx_id == max_trx_id) {
    changed = false;
  }

  prev_up_limit_id = m_up_limit_id;
  prev_low_limit_no = m_low_limit_no;
  prev_max_trx_id = max_trx_id;

  return changed;
}

void CopyFreeSnapshot::update_min_routine() {

  /* if no event happend, we slow done the check */
  uint64_t quiet_count = 0;
  static const uint64_t QUIET_COUNT_THRESHOLD = 1000;
  static const uint64_t QUIET_SLEEP_US = 5 * 1000; // 5 * 1000 us = 5 ms

  while (m_routine_running) {
    bool changed = do_update_min();
    if (changed) {
      std::this_thread::sleep_for(std::chrono::microseconds(srv_txsql_copy_free_snapshot_update_min_interval_us));
      quiet_count = 0;
    } else if (++quiet_count >= QUIET_COUNT_THRESHOLD) {
      std::this_thread::sleep_for(std::chrono::microseconds(QUIET_SLEEP_US));
      quiet_count = QUIET_COUNT_THRESHOLD;
    }
  }
}

bool purge_hash_elem(rw_trx_hash_element_t *element,
                     CopyFreeSnapshot::PurgeArg *arg) {
  if (unlikely(arg->to_purge_count >= CopyFreeSnapshot::PurgeArg::MAX_SIZE)) {
    return true;
  }
  if (CopyFreeSnapshot::is_valid_timestamp(element->del_ts)
      && element->del_ts < arg->min_view_ts) {
    arg->to_purge[arg->to_purge_count++] = element;
  }
  return false;
}

// true if some hash element purged
bool CopyFreeSnapshot::do_purge_hash(PurgeArg &arg) {
  assert(arg.to_purge != nullptr);
  arg.min_view_ts = m_min_view_ts.load();
  arg.to_purge_count = 0;

  TRX_HASH_ITERATE(nullptr, purge_hash_elem, &arg);

  LF_PINS *pins = trx_sys->rw_trx_hash.get_pins();
  for (int64_t i = 0; i < arg.to_purge_count; i++) {
    rw_trx_hash_element_t *e = arg.to_purge[i];
    trx_sys->rw_trx_hash.raw_erase(e, pins);
  }
  trx_sys->rw_trx_hash.put_pins(pins);

  return arg.to_purge_count > 0;
}

// convert a copy-free snapshot to copy-style snapshot
class Converter {
public:
  Converter(uint64_t old_ts) : m_old_ts(old_ts) {}
  bool operator()(trx_t *trx) {
    mutex_enter(&trx->view_mutex);
    auto &read_view = trx->read_view;
    auto view_ts = read_view->get_view_ts();
    if (read_view->get_state() == READ_VIEW_STATE_OPEN
        && CopyFreeSnapshot::is_valid_timestamp(view_ts)
        && view_ts < m_old_ts) {
      // this trx is an old one with a copy-free snapshot
      read_view->convert_to_copy();
    }
    mutex_exit(&trx->view_mutex);
    return false;
  }
private:
  /* determine if a snapshot is old */
  uint64_t m_old_ts;
};

void CopyFreeSnapshot::convert_snapshots() {
  // Iterate the rw_trx_hash only once for all old transactions
  // use old_ts to check if a trx is an old one
  uint64_t old_ts = get_clock() - srv_txsql_copy_free_snapshot_rw_hash_size_threshold;
  Converter c(old_ts);
  trx_sys->mysql_trx_list.foreach(c);
}

// Iterate all trx (rw + ro) snapshot to get min view_ts.
class MinViewTsGetter {
public:
  MinViewTsGetter() {
    m_min_view_ts = CopyFreeSnapshot::get_instance().get_clock();
  }
  bool operator()(trx_t *trx) {
    mutex_enter(&trx->view_mutex);
    if (trx->read_view->get_state() == READ_VIEW_STATE_OPEN) {
      uint64_t trx_view_ts = trx->read_view->get_view_ts();
      ut_ad(CopyFreeSnapshot::is_valid_timestamp(trx_view_ts)
            || trx_view_ts == CopyFreeSnapshot::DISABLE);
      if (CopyFreeSnapshot::is_valid_timestamp(trx_view_ts)
          && trx_view_ts < m_min_view_ts) {
        m_min_view_ts = trx_view_ts;
      }
    }
    mutex_exit(&trx->view_mutex);
    return false;
  }
  uint64_t get_min_view_ts() const { return m_min_view_ts; }
private:
  uint64_t m_min_view_ts;
};

void CopyFreeSnapshot::convert_snapshot_and_purge_hash_routine() {

  PurgeArg arg;
  arg.to_purge = (rw_trx_hash_element_t**)malloc(sizeof(rw_trx_hash_element_t*) * PurgeArg::MAX_SIZE);

  uint64_t quiet_count = 0;
  static const uint64_t QUIET_COUNT_THRESHOLD = 1000;
  static const uint64_t QUIET_SLEEP_US = 5 * 1000; // 5 * 1000 us = 5 ms

  while (m_routine_running) {
    // do convert snapshot from copy-free to copy-style
    if (trx_sys->rw_trx_hash.size() > srv_txsql_copy_free_snapshot_rw_hash_size_threshold) {
      convert_snapshots();
    }

    // get min view ts by iterating all trx list
    MinViewTsGetter min_view_ts_getter;
    trx_sys->mysql_trx_list.foreach(min_view_ts_getter);
    uint64_t new_min_view_ts = min_view_ts_getter.get_min_view_ts();
    if (new_min_view_ts > m_min_view_ts.load()) {
      m_min_view_ts.store(new_min_view_ts);
    }

    bool purged = do_purge_hash(arg);
    if (purged) {
      std::this_thread::sleep_for(std::chrono::microseconds(srv_txsql_copy_free_snapshot_update_min_interval_us));
      quiet_count = 0;
    } else if (++quiet_count >= QUIET_COUNT_THRESHOLD) {
      std::this_thread::sleep_for(std::chrono::microseconds(QUIET_SLEEP_US));
      quiet_count = QUIET_COUNT_THRESHOLD;
    }
  }

  free(arg.to_purge);
}

uint64_t CopyFreeSnapshot::get_remain_trx_count() {
  do_update_min();

  PurgeArg arg;
  arg.to_purge = (rw_trx_hash_element_t**)malloc(sizeof(rw_trx_hash_element_t*) * PurgeArg::MAX_SIZE);
  while (do_purge_hash(arg)) { /* do again until nothing can be purged */ }

  free(arg.to_purge);

  return trx_sys->rw_trx_hash.size();
}

/** ReadView constructor */
ReadView::ReadView()
    : m_low_limit_id(),
      m_up_limit_id(),
      m_creator_trx_id(),
      m_attach_trx_id(),
      m_ids(),
      m_low_limit_no(),
      m_hash_erase_version() {
  m_state = READ_VIEW_STATE_CLOSED;
  ut_d(m_view_low_limit_no = 0);
  m_trx = nullptr;
  m_view_ts.store(CopyFreeSnapshot::DISABLE);
}

/**
ReadView destructor */
ReadView::~ReadView() {
  // Do nothing
}

/** Constructor */
MVCC::MVCC() {
  m_cached_view = new ReadView();
  m_cached_view_lock = static_cast<rw_lock_t*>(ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY, sizeof(rw_lock_t)));
  rw_lock_create(trx_sys_mvcc_lock_key, m_cached_view_lock, SYNC_NO_ORDER_CHECK);
}



MVCC::~MVCC() {
  delete m_cached_view;
  rw_lock_free(m_cached_view_lock);
  ut::free(m_cached_view_lock);
}

// true for visible
bool ReadView::changes_visible_new(trx_id_t id, const table_name_t &name) const {
  uint64_t del_ts = trx_sys->rw_trx_hash.get_elem_del_ts(m_trx, id);
  if (del_ts == CopyFreeSnapshot::VALID_BASE_TS) {
    // hash element has been purged
    return true;
  }
  if (del_ts == CopyFreeSnapshot::ACTIVE) {
    return false;
  }
  if (unlikely(del_ts == CopyFreeSnapshot::DELETING)) {
    while (del_ts == CopyFreeSnapshot::DELETING) {
      PAUSE();
      del_ts = trx_sys->rw_trx_hash.get_elem_del_ts(m_trx, id);
    }
  }
  return del_ts < m_view_ts.load();
}

// true for visibale
bool ReadView::changes_visible_old(trx_id_t id, const table_name_t &name) const {
  if (m_ids.empty()) {
    return true;
  }
  return (!std::binary_search(m_ids.begin(), m_ids.end(), id));
}

bool ReadView::changes_visible(trx_id_t id, const table_name_t &name) const {
  if (srv_read_only_mode) {
    return true;
  }

  ut_ad(id > 0);
  ut_ad(m_up_limit_id > 0);

  if (id < m_up_limit_id || id == m_creator_trx_id || id == m_attach_trx_id) {
    return true;
  }

  check_trx_id_sanity(id, name);

  if (id >= m_low_limit_id.load()) {
    return false;
  }

  do {
    if (!srv_txsql_enable_copy_free_snapshot
        || m_view_ts.load() == CopyFreeSnapshot::DISABLE) {
      return changes_visible_old(id, name);
    } else {
      bool v = changes_visible_new(id, name);
      if (likely(CopyFreeSnapshot::is_valid_timestamp(m_view_ts.load()))) {
        return v;
      }
      ut_a(m_view_ts.load() == CopyFreeSnapshot::DISABLE);
    }
  } while (true);
}

inline void ReadView::take_snapshot(trx_t *trx) {
  int64_t ret_hash_erase_version = 0;
  trx_id_t ret_low_limit_id = TRX_ID_MAX;
  trx_sys->snapshot_ids(trx, &m_ids, &ret_low_limit_id, &m_low_limit_no, &ret_hash_erase_version);

  std::sort(m_ids.begin(), m_ids.end());
  m_low_limit_id.store(ret_low_limit_id);
  m_up_limit_id = (m_ids.empty() ? ret_low_limit_id: m_ids.front());
  ut_d(m_view_low_limit_no = m_low_limit_no);
  m_hash_erase_version.store(ret_hash_erase_version);
}

struct CopyIdListArg {
  uint64_t view_ts;
  trx_id_t low_limit_id;
  trx_id_t up_limit_id;
  trx_ids_t &ids;

  CopyIdListArg(uint64_t view_ts_arg,
                trx_id_t low_limit_id_arg,
                trx_id_t up_limit_id_arg,
                trx_ids_t &ids_arg)
    : view_ts(view_ts_arg),
      low_limit_id(low_limit_id_arg),
      up_limit_id(up_limit_id_arg),
      ids(ids_arg) {}
};

static bool copy_active_trx(rw_trx_hash_element_t *elem, CopyIdListArg *arg) {
  if (elem->id < arg->up_limit_id || elem->id >= arg->low_limit_id) {
    // only copy trx id in [arg->up_limit_id, arg->low_limit_id_arg)
    return false;
  }

  uint64_t del_ts = elem->del_ts.load();
  if (del_ts == CopyFreeSnapshot::ACTIVE) {
    // active trx
    arg->ids.push_back(elem->id);
    return false;
  }
  if (del_ts == CopyFreeSnapshot::DELETING) {
    while ((del_ts = elem->del_ts.load()) == CopyFreeSnapshot::DELETING) {
      PAUSE();
    }
  }
  if (del_ts >= arg->view_ts) {
    arg->ids.push_back(elem->id);
  }
  return false;
}

inline void ReadView::take_snapshot_copy_free(trx_t *trx, bool force_copy) {
#ifdef UNIV_DEBUG
  if (trx) {
    ut_ad(mutex_own(&trx->view_mutex));
  }
#endif

  m_view_ts.store(CopyFreeSnapshot::get_instance().get_clock());
  m_low_limit_id = trx_sys->get_max_trx_id_safe();
  m_up_limit_id = CopyFreeSnapshot::get_instance().get_up_limit_id();
  m_low_limit_no = CopyFreeSnapshot::get_instance().get_low_limit_no();

  if (force_copy) {
    CopyIdListArg arg(m_view_ts.load(), m_low_limit_id, m_up_limit_id, m_ids);
    TRX_HASH_ITERATE(trx, copy_active_trx, &arg);
    std::sort(m_ids.begin(), m_ids.end());
    m_view_ts.store(CopyFreeSnapshot::DISABLE);
  }
}

void ReadView::convert_to_copy() {
  ut_ad(CopyFreeSnapshot::is_valid_timestamp(m_view_ts.load()));

  CopyIdListArg copy_arg(m_view_ts.load(), m_low_limit_id, m_up_limit_id, m_ids);
  TRX_HASH_ITERATE(nullptr, copy_active_trx, &copy_arg);

  std::sort(m_ids.begin(), m_ids.end());
  m_view_ts.store(CopyFreeSnapshot::DISABLE);
}

// check if the view is still up-to-date
inline bool view_is_uptodate(const ReadView *view) {
  return view->low_limit_id() == trx_sys->get_rw_trx_hash_version() &&
         view->get_hash_erase_version() == trx_sys->get_hash_erase_version();
}

void ReadView::snapshot(trx_t *trx, bool force_copy) {
  m_trx = trx;
  if (trx == nullptr) {
    m_ids.clear();
    // this is the purge thread, take snapshot and just return.
    if (srv_txsql_enable_copy_free_snapshot) {
      this->take_snapshot_copy_free(trx, force_copy);
    } else {
      this->take_snapshot(trx);
    }
    return;
  }
  ut_ad(trx->read_view == this);
  if (state() == READ_VIEW_STATE_OPEN) {
    // already opened, just return.
    ut_ad(!srv_read_only_mode);
    return;
  }
  if (unlikely(srv_read_only_mode)) {
    return;
  }

  mutex_enter(&trx->view_mutex);
  m_creation_start = ReadView::get_ts();
  if (srv_txsql_enable_copy_free_snapshot) {
    this->take_snapshot_copy_free(trx, force_copy);
  } else {
    this->take_snapshot(trx);
  }
  DBUG_EXECUTE_IF("sleep_when_taking_snapshot",
      std::this_thread::sleep_for(std::chrono::microseconds(2ll * 1000 * 1000)););
  m_creator_trx_id = trx->id;
  m_state = READ_VIEW_STATE_OPEN;
  m_creation_end = ReadView::get_ts();
  mutex_exit(&trx->view_mutex);

  if (trx->mysql_thd != nullptr) {
    m_attach_trx_id = thd_get_attach_trx_id(trx->mysql_thd);
  } else {
    m_attach_trx_id = 0;
  }
}

void ReadView::open_by_copy(ReadView *other) {
  clone_from(other);
  m_creator_trx_id = 0;
  m_attach_trx_id = 0;
  m_state.store(READ_VIEW_STATE_OPEN, std::memory_order_release);
}

void ReadView::merge(ReadView* other) {
  ut_ad(other != this);
  if (m_low_limit_no > other->low_limit_no()) {
    m_low_limit_no = other->low_limit_no();
  }
  if (m_low_limit_id > other->low_limit_id()) {
    m_low_limit_id = other->low_limit_id();
  }

  trx_ids_t::iterator dst = m_ids.begin();
  for (const trx_id_t id : other->m_ids) {
    if (id >= m_low_limit_id)
      break;
loop:
    if (dst == m_ids.end()) {
      m_ids.push_back(id);
      dst = m_ids.end();
      continue;
    }
    if (*dst < id) {
      dst++;
      goto loop;
    } else if (*dst > id) {
      dst = m_ids.insert(dst, id) + 1;
    }
  }

  m_ids.erase(std::lower_bound(dst, m_ids.end(), m_low_limit_id), m_ids.end());

  m_up_limit_id = m_ids.empty() ? (m_low_limit_id.load()) : (m_ids.front());
  ut_ad(m_up_limit_id <= m_low_limit_id);
}

void ReadView::clone_from(const ReadView *other) {
  m_low_limit_id.store(other->m_low_limit_id.load());
  m_low_limit_no = other->m_low_limit_no;
  m_up_limit_id = other->up_limit_id();
  m_ids = other->m_ids; // copy
  m_hash_erase_version.store(other->get_hash_erase_version());
  m_view_ts.store(other->m_view_ts);
  ut_d(m_view_low_limit_no = other->m_view_low_limit_no);
}

void ReadView::clone_copy_free_view_from(const ReadView *other) {
  m_low_limit_id.store(other->m_low_limit_id.load());
  m_low_limit_no = other->m_low_limit_no;
  m_up_limit_id = other->up_limit_id();
  m_view_ts.store(other->m_view_ts);
  ut_d(m_view_low_limit_no = other->m_view_low_limit_no);
}

void MVCC::view_open(ReadView *view, trx_t *trx, uint64_t gts) {
  ut_ad(!srv_read_only_mode);
  ut_ad(!trx->view_assigned);
  ut_a(view != nullptr);

  /* Create a snapshot and add to list */
  view->snapshot(trx, false);
  view->m_gts = gts;
}

class ViewCounter {
public:
  ViewCounter() : m_size(0) {}
  bool operator()(trx_t *trx) {
    if (trx->read_view->get_state() == READ_VIEW_STATE_OPEN) {
      m_size++;
    }
    return false;
  }
  ulint size() { return m_size; }
private:
  ulint m_size;
};

ulint MVCC::size() const {
  ViewCounter view_counter;
  trx_sys->mysql_trx_list.foreach(view_counter);
  return view_counter.size();
}

void MVCC::clone_oldest_view(ReadView *view, bool fast) {
  /*
   After Copy Free Snapshot, we use clone_oldest_view_new to handle
   two situations(srv_txsql_enable_copy_free_snapshot is ON or OFF)
   instead of code like this:

   if (srv_txsql_enable_copy_free_snapshot) {
     clone_oldest_view_new(view);
   } else {
     clone_oldest_view_old(view);
   }
   */

  clone_oldest_view_new(view, fast);

  /** Update view to block purging transaction till GTID is persisted */
  auto &gtid_persistor = clone_sys->get_gtid_persistor();
  auto gtid_oldest_trxno = gtid_persistor.get_oldest_trx_no();
  view->reduce_low_limit(gtid_oldest_trxno);
}

struct ViewCreation
{
  int64_t m_start;
  int64_t m_end;
  ViewCreation *m_prev;
  ViewCreation *m_next;

  ViewCreation()
    : m_start(0),
      m_end(0),
      m_prev(nullptr),
      m_next(nullptr) {}
};

class OldestViewGetterForCopyFree {
public:
  OldestViewGetterForCopyFree(ReadView *cv, ReadView *cfv, bool fast)
    : m_copy_view(cv),
      m_copy_free_view(cfv),
      m_fast(fast) {
    assert(cv && cfv);
    m_list.m_prev = &m_list;
    m_list.m_next = &m_list;
    has_copy_free_view = false;
    total_tested_view_count = 0;
    total_merged_view_count = 0;
    m_list_length_cur = 0;
    m_list_length_max = 0;
  }

  ~OldestViewGetterForCopyFree() {
    ViewCreation *iter = m_list.m_next;
    while (iter != &m_list) {
      ViewCreation *n = iter->m_next;
      delete iter;
      iter = n;
    }
  }

  void merge_view_against_list(ReadView *trx_view) {
    if (m_fast) {
      ViewCreation *iter = m_list.m_next;
      bool need_merge = true;
      while (iter != &m_list) {
        if (trx_view->m_creation_start > iter->m_end) {
          // this trx_view is newer than some min read view in list,
          // do not merge it
          need_merge = false;
          break;
        } else if (iter->m_start > trx_view->m_creation_end) {
          // this trx_view is older than some min read view in list,
          // remove the read view from list
          iter->m_next->m_prev = iter->m_prev;
          iter->m_prev->m_next = iter->m_next;
          ViewCreation *next = iter->m_next;
          delete iter;
          iter = next;
          m_list_length_cur--;
        } else {
          // trx_view and iter are overlap
          iter = iter->m_next;
        }
      } // end while
      if (need_merge) {
        m_copy_view->merge(trx_view);
        ViewCreation *vc = new ViewCreation;
        vc->m_start = trx_view->m_creation_start;
        vc->m_end = trx_view->m_creation_end;
        vc->m_prev = &m_list;
        vc->m_next = m_list.m_next;
        m_list.m_next->m_prev = vc;
        m_list.m_next = vc;
        total_merged_view_count++;
        m_list_length_cur++;
        if (m_list_length_cur > m_list_length_max) {
          m_list_length_max = m_list_length_cur;
        }
      }
    } else {
      // not fast, just merge every read view
      m_copy_view->merge(trx_view);
    }
  }

  bool operator()(trx_t *trx) {
    ReadView tmp_view;
    mutex_enter(&trx->view_mutex);
    ReadView *trx_view = trx->read_view;
    if (trx_view->get_state() == READ_VIEW_STATE_OPEN) {
      total_tested_view_count++;
      bool recheck = false;
      do {
        recheck = false;
        if (trx_view->get_view_ts() == CopyFreeSnapshot::DISABLE) {
          merge_view_against_list(trx_view);
        } else {
          tmp_view.clone_copy_free_view_from(trx_view);
          if (CopyFreeSnapshot::is_valid_timestamp(tmp_view.get_view_ts())) {
            merge_copy_free_view(&tmp_view, m_copy_free_view);
            has_copy_free_view = true;
          } else { // the view is just converted
            recheck = true;
          }
        }
      } while (recheck);
    } // end if view state == OPEN
    mutex_exit(&trx->view_mutex);
    return false;
  }

  bool reach_time(int interval) {
    static volatile std::atomic<int> last_time{0};
    const auto current_time = std::chrono::steady_clock::now();
    const auto current_time_in_sec =
        std::chrono::duration_cast<std::chrono::seconds>(
            current_time.time_since_epoch())
            .count();
    int old_time = last_time.load();
    if ((interval + last_time) < current_time_in_sec
        && last_time.compare_exchange_weak(old_time, current_time_in_sec)) {
      return true;
    }
    DBUG_EXECUTE_IF("test_reach_time", return true;);
    return false;
  }

  ReadView *finish() {
    if (has_copy_free_view) {
      m_copy_free_view->convert_to_copy();
      m_copy_view->merge(m_copy_free_view);
    }
    DBUG_EXECUTE_IF("test_reach_time", total_merged_view_count=65; total_tested_view_count=66;);
    if (m_fast
        && total_merged_view_count > 64
        && (total_merged_view_count * 10 > total_tested_view_count)) {
      if (reach_time(60 * 5)) { // 5 minutes
        sql_print_information("clone_oldest_view too many read views merged, merged=%ld, "
            "total=%ld, list length max=%ld, has_copy_free_view=%d",
            total_merged_view_count, total_tested_view_count,
            m_list_length_max, has_copy_free_view);
      }
    }
    DBUG_PRINT("clone_oldest_view",
               ("too many read views merged, fast_flag=%d, merged=%ld, "
                "total=%ld, list_length_max=%ld, has_copy_free_view=%d",
                m_fast, total_merged_view_count, total_tested_view_count,
                m_list_length_max, has_copy_free_view));
    ut_d(m_copy_view->m_view_low_limit_no = m_copy_view->m_low_limit_no);
    return m_copy_view;
  }

  void merge_copy_free_view(ReadView *from, ReadView *to) {
    to->m_up_limit_id = std::min(to->m_up_limit_id, from->m_up_limit_id);
    to->m_low_limit_no = std::min(to->m_low_limit_no, from->m_low_limit_no);
    to->m_low_limit_id = std::min(to->m_low_limit_id.load(), from->m_low_limit_id.load());
    /** Why copy free snapshot is so easy to merge ?
     *  Because it eliminates the scanning of rw_trx_hash which makes
     *  all read views are totally ordered in terms of m_ids(limit_id/no not included) */
    to->m_view_ts = std::min(to->m_view_ts.load(), from->m_view_ts.load());
  }

private:
  bool has_copy_free_view;
  ReadView *m_copy_view;
  ReadView *m_copy_free_view;
  /** a double linked list.
   since we get min read views in a partial order set in a streaming way,
   we use this list as currently min read view set */
  ViewCreation m_list;
  bool m_fast;

  /** some monitor information */
  int64_t total_tested_view_count;
  int64_t total_merged_view_count;
  int64_t m_list_length_cur;
  int64_t m_list_length_max;
};

void MVCC::clone_oldest_view_new(ReadView *view, bool fast) {

  ReadView *copy_view = new ReadView;
  ReadView *copy_free_view = new ReadView;

  copy_view->snapshot(nullptr, true);
  copy_free_view->snapshot(nullptr, false);

  OldestViewGetterForCopyFree getter(copy_view, copy_free_view, fast);
  trx_sys->mysql_trx_list.foreach(getter);

  const ReadView *res = getter.finish();
  view->clone_from(res);

  delete copy_view;
  delete copy_free_view;
}

/**
Close a view created by the above function.
@param view             view allocated by trx_open.
@param own_mutex        true if caller owns trx_sys_t::mutex */
void MVCC::view_close(trx_t *trx) {
  ReadView* view = trx->read_view;
  ut_a(view);
  ut_a(trx->view_assigned);

  view->close();
  trx->view_assigned = false;
}

/**
TDSQL: Set the view GTS. Note: This should be set only for trx that mix normal
select and withgts select.
@param view     Set the gts for this view
@param id       Transaction id to set
@param gts      Global timestamp */
void
MVCC::set_view_gts(ReadView* view, uint64_t gts)
{
  if (!view->m_gts) {
    //mutex_enter(&trx_sys->mutex);
    view->m_gts = gts;
    //mutex_exit(&trx_sys->mutex);
  }
}

static inline std::string log_dir_path() {
  if (srv_log_group_home_dir[strlen(srv_log_group_home_dir) -1] == '/') {
    return std::string(srv_log_group_home_dir);
  } else {
    return (std::string(srv_log_group_home_dir) + std::string("/"));
  }
}

void MVCC::delete_snapshot() {
  std::string tmp_path = log_dir_path() + std::string(TMP_SNAPSHOT_FILE_NAME);
  unlink(tmp_path.c_str());

  std::string path = log_dir_path() + std::string(SNAPSHOT_FILE_NAME);
  unlink(path.c_str());
}

/** Store purge_sys->snapshot_view into file. */
void
MVCC::persist_snapshot(ReadView *snapshot_view) {
  ut_a(snapshot_view != nullptr);
  /** Delete if there's history file */
  delete_snapshot();

  trx_ids_t trx_ids;
  trx_ids.clear();

  snapshot_view->copy_trx_ids(trx_ids);

  ulint size = (1 + 1 + 1 + 1) * 8 + 4 + trx_ids.size() * 8;

  size = ut_uint64_align_up(size, UNIV_PAGE_SIZE);

  byte* log_buf = static_cast<byte*>(
      ut::aligned_zalloc(size + UNIV_PAGE_SIZE, UNIV_PAGE_SIZE));

  /* Prepare the buffer */
  mach_write_to_8(log_buf, snapshot_view->up_limit_id());
  mach_write_to_8(log_buf + 8, snapshot_view->low_limit_id());
  mach_write_to_8(log_buf + 16, snapshot_view->low_limit_no());
  mach_write_to_8(log_buf + 24, snapshot_view->gts());
  mach_write_to_4(log_buf + 32, trx_ids.size());

  byte* ptr = log_buf + 36;
  ulint i = 0;
  while (i < trx_ids.size()) {
    mach_write_to_8(ptr + i * 8, trx_ids[i]);
    i++;
  }

  /* Create a tmp file */
  std::string tmp_path = log_dir_path() + std::string(TMP_SNAPSHOT_FILE_NAME);

  bool ret;
  pfs_os_file_t handle = os_file_create(innodb_log_file_key, tmp_path.c_str(),
      OS_FILE_CREATE, OS_FILE_NORMAL,
      OS_LOG_FILE, srv_read_only_mode, &ret);

  ut_a(ret);

  dberr_t io_err;
  IORequest request(IORequest::WRITE);
  request.disable_compression();
  io_err = os_file_write(request, tmp_path.c_str(), handle, log_buf, 0, size);
  ut_a(io_err == DB_SUCCESS);

  os_file_flush(handle);
  os_file_close(handle);

  std::string path = log_dir_path() + std::string(SNAPSHOT_FILE_NAME);

  os_file_rename(innodb_log_file_key, tmp_path.c_str(), path.c_str());

  ut::aligned_free(log_buf);
}

void ReadView::parse_snapshot(byte *snapshot_buf) {
  m_up_limit_id = mach_read_from_8(snapshot_buf);
  m_low_limit_id = mach_read_from_8(snapshot_buf + 8);
  m_low_limit_no = mach_read_from_8(snapshot_buf + 16);
  m_gts = mach_read_from_8(snapshot_buf + 24);
  ulint id_count = mach_read_from_4(snapshot_buf + 32);

  m_ids.clear();
  ulint i = 0;

  byte* ptr = snapshot_buf + 36;
  while (i < id_count) {
    trx_id_t id = mach_read_from_8(ptr + i * 8);
    ut_a(id > 0);
    ut_a(id < m_low_limit_id);
    i++;
    m_ids.push_back(id);
  }

  m_creator_trx_id = 0;
}

bool
MVCC::read_snapshot(ReadView* &view) {
  std::string path = log_dir_path() + std::string(SNAPSHOT_FILE_NAME);

  os_file_stat_t stat_info;
  dberr_t err;
  err = os_file_get_status(path.c_str(), &stat_info, false, srv_read_only_mode);
  if (err == DB_NOT_FOUND) {
    return false;
  }

  bool ret;
  pfs_os_file_t handle = os_file_create_simple(
      innodb_log_file_key, path.c_str(),
      OS_FILE_OPEN, OS_FILE_READ_ONLY, srv_read_only_mode, &ret);

  if (!ret) {
    return false;
  }

  ulint size = os_file_get_size(handle);

  byte* log_buf = static_cast<byte*>(
      ut::aligned_zalloc(size + UNIV_PAGE_SIZE, UNIV_PAGE_SIZE));

  IORequest request(IORequest::READ);
  request.disable_compression();

  err = os_file_read(request, path.c_str(), handle, log_buf, 0, size);

  if (view == nullptr) {
    view = new ReadView();
  }

  view->parse_snapshot(log_buf);

  ut::aligned_free(log_buf);
  os_file_close(handle);

  return true;
}

#if defined(HAVE_PX)
void ReadView::px_clone_from(const ReadView *other) {
  clone_from(other);
  m_creator_trx_id = other->m_creator_trx_id;
}
#endif /* defined(HAVE_PX) */

/**
 Changes from txsql end.
*/

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


// if true, use a cached read view to avoid do lf_hash iterate in some cases.
static bool USE_CACHED_READ_VIEW = true;

/**
ReadView constructor */
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

// true for visibale
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

  if (id >= m_low_limit_id.load(std::memory_order_relaxed)) {
    return false;
  } else if (m_ids.empty()) {
    return true;
  }
  return (!std::binary_search(m_ids.begin(), m_ids.end(), id));
}

inline void ReadView::take_snapshot(trx_t *trx) {
  int64_t ret_hash_erase_version = 0;
  trx_id_t ret_low_limit_id = TRX_ID_MAX;

  trx_sys->snapshot_ids(trx, &m_ids, &ret_low_limit_id, &m_low_limit_no, &ret_hash_erase_version);

  std::sort(m_ids.begin(), m_ids.end());
  m_low_limit_id.store(ret_low_limit_id);
  m_up_limit_id = (m_ids.empty() ? ret_low_limit_id: m_ids.front());
  ut_d(m_view_low_limit_no = m_low_limit_no);
  m_hash_erase_version.store(ret_hash_erase_version, std::memory_order_relaxed);
}

// check if the view is still up-to-date
inline bool view_is_uptodate(const ReadView *view) {
  return view->low_limit_id() == trx_sys->get_rw_trx_hash_version() &&
         view->get_hash_erase_version() == trx_sys->get_hash_erase_version();
}



bool ReadView::try_use_cached_view() {
  bool can_use_cached = false;
  trx_sys->mvcc->cached_view_slock();
  if (view_is_uptodate(trx_sys->mvcc->cached_view())) {
    this->clone_from(trx_sys->mvcc->cached_view());
    can_use_cached = true;
  }
  trx_sys->mvcc->cached_view_sunlock();
  return can_use_cached;
}

void ReadView::try_install_cached_view() const {
  if (!view_is_uptodate(this)) {
    // optimized check, without m_cached_view_lock
    return;
  }
  if (trx_sys->mvcc->cached_view_try_xlock()) {
    if (view_is_uptodate(this)) {
      // this view is not stale
      trx_sys->mvcc->cached_view()->clone_from(this);
    }
    trx_sys->mvcc->cached_view_xunlock();
  }
}

void ReadView::snapshot(trx_t *trx) {
  if (trx == nullptr) {
    // this is the purge thread, take snapshot and just return.
    take_snapshot(nullptr);
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
  if (USE_CACHED_READ_VIEW) {
    if (try_use_cached_view()) {
      m_creator_trx_id = trx->id;
      m_state = READ_VIEW_STATE_OPEN;
      mutex_exit(&trx->view_mutex);
      return;
    }
  }

  this->take_snapshot(trx);
  m_creator_trx_id = trx->id;
  m_state = READ_VIEW_STATE_OPEN;
  mutex_exit(&trx->view_mutex);

  if (USE_CACHED_READ_VIEW) {
    try_install_cached_view();
  }

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
  m_low_limit_id.store(other->m_low_limit_id.load(std::memory_order_relaxed), std::memory_order_relaxed);
  m_low_limit_no = other->m_low_limit_no;
  m_up_limit_id = other->up_limit_id();
  m_ids = other->m_ids; // copy
  m_hash_erase_version.store(other->get_hash_erase_version(), std::memory_order_relaxed);
  ut_d(m_view_low_limit_no = other->m_view_low_limit_no);
}

void MVCC::view_open(ReadView *view, trx_t *trx, uint64_t gts) {
  ut_ad(!srv_read_only_mode);
  ut_ad(!trx->view_assigned);
  ut_a(view != nullptr);

  /* Create a snapshot and add to list */
  view->snapshot(trx);
  view->m_gts = gts;
}

ulint MVCC::size() const {
  ulint size = 0;
  trx_sys_mutex_enter();
  for (const trx_t *trx = UT_LIST_GET_FIRST(trx_sys->mysql_trx_list); trx != nullptr;
       trx = UT_LIST_GET_NEXT(mysql_trx_list, trx)) {
    if (trx->read_view->get_state() == READ_VIEW_STATE_OPEN) {
      size++;
    }
  }
  trx_sys_mutex_exit();
  return size;
}

/** Clones the oldest view and stores it in view. No need to
call view_close(). The caller owns the view that is passed in.
It will also move the closed views from the m_views list to the
m_free list. This function is called by Purge to determine whether it should
purge the delete marked record or not.
@param view             Preallocated view, owned by the caller */
void MVCC::clone_oldest_view(ReadView *view, bool fast) {
  view->snapshot(nullptr);

  trx_sys_mutex_enter();
  for (trx_t *trx = UT_LIST_GET_FIRST(trx_sys->mysql_trx_list); trx != nullptr;
       trx = UT_LIST_GET_NEXT(mysql_trx_list, trx)) {
    mutex_enter(&trx->view_mutex);
    if (trx->read_view->get_state() == READ_VIEW_STATE_OPEN) {
      if (fast) {
        view->m_up_limit_id = std::min(view->m_up_limit_id, trx->read_view->m_up_limit_id);
        view->m_low_limit_no = std::min(view->m_low_limit_no, trx->read_view->m_low_limit_no);
      } else {
        view->merge(trx->read_view);
      }
    }
    mutex_exit(&trx->view_mutex);
  }
  trx_sys_mutex_exit();
  ut_d(view->m_view_low_limit_no = view->m_low_limit_no);

  /** Update view to block purging transaction till GTID is persisted */
  auto &gtid_persistor = clone_sys->get_gtid_persistor();
  auto gtid_oldest_trxno = gtid_persistor.get_oldest_trx_no();
  view->reduce_low_limit(gtid_oldest_trxno); 
  if (fast) {
    view->m_low_limit_id = view->m_up_limit_id;
    view->m_ids.clear();
  }
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

void ReadView::px_clone_from(const ReadView *other) {
  clone_from(other);
  m_creator_trx_id = other->m_creator_trx_id;
}

/**
 Changes from txsql end.
*/

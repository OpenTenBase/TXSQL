/*****************************************************************************

Copyright (c) 1996, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** Minimum number of elements to reserve in ReadView::ids_t */
static const ulint MIN_TRX_IDS = 32;

/** If set to true, it'll make use of global view to take snapshot
without iterating lf_hash */
bool opt_use_cloned_view = true;
/**
ReadView constructor */
ReadView::ReadView()
    : m_low_limit_id(),
      m_up_limit_id(),
      m_creator_trx_id(),
      m_ids(),
      m_low_limit_no() {
  m_state = READ_VIEW_STATE_CLOSED;
}

/**
ReadView destructor */
ReadView::~ReadView() {
  // Do nothing
}

/** Constructor */
MVCC::MVCC() {
  m_valid_view = false;
  m_clone_view = new ReadView();
  m_clone_lock = static_cast<rw_lock_t *>(ut_malloc_nokey(sizeof(rw_lock_t)));
  rw_lock_create(trx_sys_mvcc_lock_key, m_clone_lock, SYNC_NO_ORDER_CHECK);
}

MVCC::~MVCC() {
  delete m_clone_view;
  rw_lock_free(m_clone_lock);
  ut_free(m_clone_lock);
}

inline bool ReadView::reuse() {
  return (empty() &&
          m_low_limit_id == trx_sys_get_max_trx_id() &&
          m_creator_trx_id == 0);
}

inline void ReadView::take_snapshot(trx_t *trx) {
  trx_sys->snapshot_ids(trx, &m_ids, &m_low_limit_id, &m_low_limit_no);
  std::sort(m_ids.begin(), m_ids.end());
  m_up_limit_id= m_ids.empty() ? m_low_limit_id : m_ids.front();
}

bool ReadView::changes_visible(trx_id_t id, const table_name_t &name) const {
  if (srv_read_only_mode) {
    return true;
  }

  ut_ad(id > 0);
  ut_ad(m_up_limit_id > 0);

  if (id < m_up_limit_id || id == m_creator_trx_id) {
    return (true);
  }

  check_trx_id_sanity(id, name);

  if (id >= m_low_limit_id) {
    return (false);

  } else if (m_ids.empty()) {
    return (true);
  }

  return (!std::binary_search(m_ids.begin(), m_ids.end(), id));
}

void ReadView::snapshot(trx_t *trx) {
  if (trx == nullptr) {
    /* This is purge thread, take the snapshot and go back. */
    take_snapshot(nullptr);

    return;
  }

  ut_ad(this == trx->read_view);
  ut_ad(!trx->is_background);

  bool new_snapshot = false;
  switch (state()) {
    case READ_VIEW_STATE_OPEN:
      ut_ad(!srv_read_only_mode);
      return;
    case READ_VIEW_STATE_CLOSED:
      if (srv_read_only_mode) {
        return;
      }

      mutex_enter(&trx->view_mutex);
      if (reuse()) {
        goto reopen;
      }
      
      if (opt_use_cloned_view &&
          trx_sys->mvcc->is_clone_valid_relaxed() &&
          !trx->is_dd_trx &&
          !thd_is_log_apply_thread(trx->mysql_thd)) {
        trx_sys->mvcc->clone_slock();

        /* Double check */
        if (trx_sys->mvcc->is_clone_valid()) {
          clone(trx_sys->mvcc->global_view());
          creator_trx_id(trx->id);
          trx_sys->mvcc->clone_sunlock();
          m_state = READ_VIEW_STATE_OPEN;
          mutex_exit(&trx->view_mutex);
          
          return;
        } else {
          trx_sys->mvcc->clone_sunlock();
        }
      }
      
      break;
    default:
      ut_error;
      break;
  }

  take_snapshot(trx);

  new_snapshot = true;

reopen:
  m_creator_trx_id = trx->id;
  m_state.store(READ_VIEW_STATE_OPEN, std::memory_order_release);
  mutex_exit(&trx->view_mutex);
  
  if (new_snapshot &&
      (opt_use_cloned_view
       || trx_sys->mvcc->global_view()->low_limit_id() > 0) &&
      (low_limit_id() == trx_sys_get_max_trx_id()) &&
      !trx_sys->mvcc->is_clone_valid() &&
      trx_sys->mvcc->clone_xtrylock()) {

    if (!opt_use_cloned_view) {
      trx_sys->mvcc->global_view()->init();
    } else {
      trx_sys->mvcc->global_view()->clone(this);
      trx_sys->mvcc->set_view_flag(low_limit_id() == trx_sys_get_max_trx_id());
    }
    
    trx_sys->mvcc->clone_xunlock();
  }
}

void ReadView::subset(ReadView* other) {
  ut_ad(other != this);
  if (m_low_limit_no > other->low_limit_no())
    m_low_limit_no= other->low_limit_no();
  if (m_low_limit_id > other->low_limit_id())
    m_low_limit_id= other->low_limit_id();

  trx_ids_t::iterator dst= m_ids.begin();
  for (trx_ids_t::const_iterator src= other->m_ids.begin();
       src != other->m_ids.end(); src++) {
    if (*src >= m_low_limit_id)
      break;
loop:
    if (dst == m_ids.end()) {
      m_ids.push_back(*src);
      dst= m_ids.end();
      continue;
    }
    if (*dst < *src) {
      dst++;
      goto loop;
    } else if (*dst > *src) {
      dst= m_ids.insert(dst, *src) + 1;
    }
  }

  m_ids.erase(std::lower_bound(dst, m_ids.end(), m_low_limit_id),
              m_ids.end());

  m_up_limit_id= m_ids.empty() ? m_low_limit_id : m_ids.front();
  ut_ad(m_up_limit_id <= m_low_limit_id);
}

void ReadView::clone(ReadView *other) {
  m_low_limit_id = other->low_limit_id();
  m_low_limit_no = other->low_limit_no();
  m_up_limit_id = other->up_limit_id();
  m_ids = other->m_ids;
}

/**
Allocate and create a view.
@param view		view owned by this class created for the
                        caller. Must be freed by calling view_close()
@param trx		transaction instance of caller */
void MVCC::view_open(ReadView *view, trx_t *trx) {
  ut_ad(!srv_read_only_mode);
  ut_ad(!trx->register_view);

  ut_a(view != nullptr);

  /* Create a snapshot and add to list */
  view->snapshot(trx);

  return;
}

ulint MVCC::size() const {
  ulint size = 0;
  trx_sys_mutex_enter();
  for (const trx_t *trx = UT_LIST_GET_FIRST(trx_sys->mysql_trx_list); trx != NULL;
      trx = UT_LIST_GET_NEXT(mysql_trx_list, trx)) {
    if (trx->read_view->get_state() == READ_VIEW_STATE_OPEN) {
      size++;
    }
  }
  trx_sys_mutex_exit();

  return (size);
}

/** Clones the oldest view and stores it in view. No need to
call view_close(). The caller owns the view that is passed in.
This function is called by Purge to determine whether it should
purge the delete marked record or not.
@param view		Preallocated view, owned by the caller */

void MVCC::clone_oldest_view(ReadView *view) {

  view->snapshot(nullptr);
  
  clone_slock();
  if (is_clone_valid()) {
     view->subset(m_clone_view);
  }
  clone_sunlock();
  
  trx_sys_mutex_enter();

  for (trx_t *trx = UT_LIST_GET_FIRST(trx_sys->mysql_trx_list); trx != NULL;
      trx = UT_LIST_GET_NEXT(mysql_trx_list, trx)) {
    mutex_enter(&trx->view_mutex);
    if (trx->read_view->get_state() == READ_VIEW_STATE_OPEN) {
      view->subset(trx->read_view);
    }
    mutex_exit(&trx->view_mutex);
  }

  trx_sys_mutex_exit();
  
  /* Update view to block purging transaction till GTID is persisted. */
  auto &gtid_persistor = clone_sys->get_gtid_persistor();
  auto gtid_oldest_trxno = gtid_persistor.get_oldest_trx_no();
  view->reduce_low_limit(gtid_oldest_trxno);
}

/**
@return the number of active views */

/**
Close a view created by the above function.
@param view		view allocated by trx_open. */

void MVCC::view_close(trx_t *trx) {
  ReadView* view = trx->read_view;
  ut_a(view);
  ut_a(trx->register_view);

  view->close();
  
  trx->register_view = false;
}

/**
Set the view creator transaction id. Note: This shouldbe set only
for views created by RW transactions.
@param view		Set the creator trx id for this view
@param id		Transaction id to set */

void MVCC::set_view_creator_trx_id(ReadView *view, trx_id_t id) {
  ut_ad(id > 0);

  view->creator_trx_id(id);
}

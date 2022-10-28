/*****************************************************************************

Copyright (c) 1997, 2022, Oracle and/or its affiliates.

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

/** @file include/read0types.h
 Cursor read

 Created 2/16/1997 Heikki Tuuri
 *******************************************************/

#ifndef read0types_h
#define read0types_h

#include <algorithm>
#include "dict0mem.h"

#include "trx0types.h"
#include "trx0tlog.h"

// Friend declaration
class MVCC;

/** View is not visible to purge thread. */
#define READ_VIEW_STATE_CLOSED 0
/** View is visible to purge thread. */
#define READ_VIEW_STATE_OPEN 1

/** Read view lists the trx ids of those transactions for which a consistent
read should not see the modifications to the database. */

class ReadView {
 public:
  ReadView();
  ~ReadView();
  /** Check whether transaction id is valid.
  @param[in]    id              transaction id to check
  @param[in]    name            table name */
  static void check_trx_id_sanity(trx_id_t id, const table_name_t &name);

  /** Check whether the changes by id are visible.
  @param[in]    id      transaction id to check against the view
  @param[in]    name    table name
  @return whether the view sees the modifications of id. */
  [[nodiscard]] bool changes_visible(trx_id_t id,
                                     const table_name_t &name) const;

  /**
  @param id             transaction to check
  @return true if view sees transaction id */
  bool sees(trx_id_t id) const { return (id < m_up_limit_id); }

  /** TDSQL: Check whether the changes by id are visible.
  @param[in]  id    transaction id to check against the view
  @param[in]  name  table name
  @param[in]  is_active whether the transaction is active when it is accessed
  @return whether the view sees the modifications of id. */
  bool changes_visible_withgts(trx_id_t id, const table_name_t& name,
      bool is_active  = false) const MY_ATTRIBUTE((warn_unused_result))
  {
    if (is_current_trx(id)) {
      return true;
    }

    /*It means that for the newly started transaction after
    the current read transaction, its GTS must be higher than
    the current m_gts, so it must not be visible*/
    if (id > m_low_limit_id /* max_commit_trx_id */ ) {
      return false;
    }

    bool purged = false;
    uint64_t gts = tlog_mgr->get_gts(id,purged);

    /* If true = is_active, it means that it is an active transaction
     * and its gts = 0. Here gts is not 0, indicating that it has changed
     * from the active state to the prepare or commit state.*/
    if (true == is_active && gts != 0) {

      /* active GTS transactions must not be visible */
      return false;
    }

    if (purged) {

      /*Mapping record has been purged. Maybe it's a very old record
      and must be visible.*/
      return true;

    } else if (gts > 0) {

      return sees_withgts(gts);
    } else {

      /* This record is an normal transaction without GTS attributes */
      return  changes_visible(id,name);
    }
  }

  /**
  @param id transaction to check
  @return true if view sees transaction id */
  bool sees_withgts(uint64_t gts) const {
    return(gts <= m_gts);
  }

  /**
  @param id transaction to check
  @return true if it is current transaction */
  bool is_current_trx(trx_id_t id) const {
    return (id == m_creator_trx_id);
  }

  /**
  Mark the view as closed */
  void close() {
    ut_ad(state() == READ_VIEW_STATE_CLOSED ||
          state() == READ_VIEW_STATE_OPEN);
    m_state.store(READ_VIEW_STATE_CLOSED, std::memory_order_release);
  }

uint32_t get_state() const {
    return m_state.load(std::memory_order_acquire);
  }

  uint32_t state() const {
    return m_state.load(std::memory_order_relaxed);
  }

  bool is_open() const {
    ut_ad(state() == READ_VIEW_STATE_CLOSED ||
          state() == READ_VIEW_STATE_OPEN);
    return state() == READ_VIEW_STATE_OPEN;
  }

  inline void take_snapshot(trx_t *trx);

  bool try_use_cached_view();

  void try_install_cached_view() const;

  /**
  Write the limits to the file.
  @param file           file to write to */
  void print_limits(FILE *file) const {
    if (is_open()) {
      fprintf(file, "Trx read view will not see trx with"
            " id >= " TRX_ID_FMT ", sees < " TRX_ID_FMT "\n",
            m_low_limit_id.load(), m_up_limit_id);
    }
  }

  /** Check and reduce low limit number for read view. Used to
  block purge till GTID is persisted on disk table.
  @param[in]    trx_no  transaction number to check with */
  void reduce_low_limit(trx_id_t trx_no) {
    if (trx_no < m_low_limit_no) {
      /* Save low limit number set for Read View for MVCC. */
      ut_d(m_view_low_limit_no = m_low_limit_no);
      m_low_limit_no = trx_no;
    }
  }

  /** Reinit the read view */
  void init() {
    m_low_limit_no = 0;
    m_low_limit_id = 0;
    m_up_limit_id = 0;
    m_gts = 0;
    m_ids.clear();
  }

  /**
  @return the low limit no */
  trx_id_t low_limit_no() const { return (m_low_limit_no); }

  /**
  @return the low limit id */
  trx_id_t low_limit_id() const { return (m_low_limit_id.load()); }
  
  trx_id_t up_limit_id() const { return (m_up_limit_id); }

  int64_t get_hash_erase_version() const { return m_hash_erase_version.load(std::memory_order_relaxed); }

  uint64_t gts() const {
    return m_gts;
  }

  void parse_snapshot(byte *snapshot_buf);

  /**
  @return true if there are no transaction ids in the snapshot */
  bool empty() const { return (m_ids.empty()); }

  int id_size() const { return (m_ids.size()); }

  /** Take a merge of two read view */
  void merge(ReadView *other);

  /** Clone from another read view */
  void clone_from(const ReadView *other);

  /** Take a snapshot of current transaction state
  @param[in] trx  transaction object
  @param[in] add_list true if the read view needs adding to list */
  void snapshot(trx_t *trx);

#ifdef UNIV_DEBUG
  /**
  @return the view low limit number */
  trx_id_t view_low_limit_no() const { return (m_view_low_limit_no); }

  /**
  @param rhs            view to compare with
  @return truen if this view is less than or equal rhs */
  bool le(const ReadView *rhs) const {
    return (m_low_limit_no <= rhs->m_low_limit_no);
  }
#endif /* UNIV_DEBUG */

  /**
  Set the creator transaction id, existing id must be 0 */
  void creator_trx_id(trx_id_t id) {
    // ut_ad(m_creator_trx_id == 0);
    m_creator_trx_id = id;
  }

  void copy_trx_ids(trx_ids_t &ids) {
    ids = m_ids;
  }

  friend class MVCC;

 private:
  // Disable copying
  ReadView(const ReadView &);
  ReadView &operator=(const ReadView &);

 private:
   /**
  View state.
  Start view open:
  READ_VIEW_STATE_CLOSED -> READ_VIEW_STATE_OPEN

  Close view:
  READ_VIEW_STATE_OPEN -> READ_VIEW_STATE_CLOSED
  */
  std::atomic<uint32_t> m_state;

  /** The read should not see any transaction with trx id >= this
  value. In other words, this is the "high water mark". */
  std::atomic<trx_id_t> m_low_limit_id;

  /** The read should see all trx ids which are strictly
  smaller (<) than this value.  In other words, this is the
  low water mark". */
  trx_id_t m_up_limit_id;

  /** trx id of creating transaction, set to TRX_ID_MAX for free
  views. */
  trx_id_t m_creator_trx_id;

  /** Set of RW transactions that was active when this snapshot
  was taken */
  trx_ids_t m_ids;

  /** The view does not need to see the undo logs for transactions
  whose transaction number is strictly smaller (<) than this value:
  they can be removed in purge if not needed by other views */
  trx_id_t m_low_limit_no;

  std::atomic<int64_t> m_hash_erase_version;

  uint64_t m_gts;
#ifdef UNIV_DEBUG
  /** The low limit number up to which read views don't need to access
  undo log records for MVCC. This could be higher than m_low_limit_no
  if purge is blocked for GTID persistence. Currently used for debug
  variable INNODB_PURGE_VIEW_TRX_ID_AGE. */
  trx_id_t m_view_low_limit_no;
#endif /* UNIV_DEBUG */

};

#endif

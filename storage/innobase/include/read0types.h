/*****************************************************************************

Copyright (c) 1997, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

// Friend declaration
class MVCC;

/** Read view lists the trx ids of those transactions for which a consistent
read should not see the modifications to the database. */

class ReadView {

 public:
  ReadView();
  ~ReadView();
  /** Check whether transaction id is valid.
  @param[in]	id		transaction id to check
  @param[in]	name		table name */
  static void check_trx_id_sanity(trx_id_t id, const table_name_t &name);

  /** Check whether the changes by id are visible.
  @param[in]	id	transaction id to check against the view
  @param[in]	name	table name
  @return whether the view sees the modifications of id. */
  bool changes_visible(trx_id_t id, const table_name_t &name) const
      MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(id > 0);

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

  /**
  @param id		transaction to check
  @return true if view sees transaction id */
  bool sees(trx_id_t id) const { return (id < m_up_limit_id); }

  /**
  Write the limits to the file.
  @param file		file to write to */
  void print_limits(FILE *file) const {
    fprintf(file,
            "Trx read view will not see trx with"
            " id >= " TRX_ID_FMT ", sees < " TRX_ID_FMT "\n",
            m_low_limit_id, m_up_limit_id);
  }

  /** Check and reduce low limit number for read view. Used to
  block purge till GTID is persisted on disk table.
  @param[in]	trx_no	transaction number to check with */
  void reduce_low_limit(trx_id_t trx_no) {
    if (trx_no < m_low_limit_no) {
      /* Save low limit number set for Read View for MVCC. */
      ut_d(m_view_low_limit_no = m_low_limit_no);
      m_low_limit_no = trx_no;
    }
  }

  /**
  @return the low limit no */
  trx_id_t low_limit_no() const { return (m_low_limit_no); }

  /**
  @return the low limit id */
  trx_id_t low_limit_id() const { return (m_low_limit_id); }

  trx_id_t up_limit_id() const { return (m_up_limit_id); }
  /**
  @return true if there are no transaction ids in the snapshot */
  bool empty() const { return (m_ids.empty()); }

  int id_size() const { return (m_ids.size()); }


  /** Check and reuse the cached read view
  @return true if it can be reused. */
  bool reuse();

  /** Set cache flag to true */
  void cache(bool new_val) {
    m_cached = new_val; 
  }

  /** Return true if the read view is cached on view list */
  bool is_cached() const {
    return (m_cached.load());
  }

  /** The purge thread may remove read view from list, Meanwhile
  it needs be marked ad abandoned.*/
  void mark_abandoned() {
    m_abandoned = true;
  }

  /** Return true if read view is abandoned */
  bool is_abandoned() {
    return (m_abandoned.load());
  }

  /** Take a subset of two read view */
  void subset(ReadView *other);

  /** Take a snapshot of current transaction state
  @param[in] trx  transaction object
  @param[in] add_list true if the read view needs adding to list */
  void snapshot(trx_t *trx, bool add_list);
#ifdef UNIV_DEBUG
  /**
  @return the view low limit number */
  trx_id_t view_low_limit_no() const { return (m_view_low_limit_no); }

  /**
  @param rhs		view to compare with
  @return truen if this view is less than or equal rhs */
  bool le(const ReadView *rhs) const {
    return (m_low_limit_no <= rhs->m_low_limit_no);
  }
#endif /* UNIV_DEBUG */
 private:

  /** Flag that indicates the read view is cached on
  view list. It can be only be changed by the session
  that owns the view */
  std::atomic<bool> m_cached;

  /** Flag that indicates the read view is removed from
  view list by purge thread, Note that m_cached may still
  be true. */
  std::atomic<bool> m_abandoned;
  
  /**
  Set the creator transaction id, existing id must be 0 */
  void creator_trx_id(trx_id_t id) {
    ut_ad(m_creator_trx_id == 0);
    m_creator_trx_id = id;
  }

  friend class MVCC;

 private:
  // Disable copying
  ReadView(const ReadView &);
  ReadView &operator=(const ReadView &);

 private:
  /** The read should not see any transaction with trx id >= this
  value. In other words, this is the "high water mark". */
  trx_id_t m_low_limit_id;

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

#ifdef UNIV_DEBUG
  /** The low limit number up to which read views don't need to access
  undo log records for MVCC. This could be higher than m_low_limit_no
  if purge is blocked for GTID persistence. Currently used for debug
  variable INNODB_PURGE_VIEW_TRX_ID_AGE. */
  trx_id_t m_view_low_limit_no;
#endif /* UNIV_DEBUG */

  typedef UT_LIST_NODE_T(ReadView) node_t;

  /** List of read views in trx_sys */
  byte pad1[64 - sizeof(node_t)];
  node_t m_view_list;
};

#endif

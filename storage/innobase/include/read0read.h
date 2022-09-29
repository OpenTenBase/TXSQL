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

/** @file include/read0read.h
 Cursor read

 Created 2/16/1997 Heikki Tuuri
 *******************************************************/

#ifndef read0read_h
#define read0read_h

#include <stddef.h>
#include <algorithm>

#include "read0types.h"
#include "univ.i"

/** The MVCC read view manager */
class MVCC {
 public:
  /** Constructor **/
  explicit MVCC();

  /** Destructor */
  ~MVCC();

  /** Allocate and create a view.
  @param view   View owned by this class created for the caller. Must be
  freed by calling view_close()
  @param trx    Transaction instance of caller */
  void view_open(ReadView *view, trx_t *trx);

  /** Close a view created by the above function. */
  void view_close(trx_t *trx);

  /** Clones the oldest view and stores it in view. No need to
  call view_close(). The caller owns the view that is passed in.
  It will also move the closed views from the m_views list to the
  m_free list. This function is called by Purge to determine whether it should
  purge the delete marked record or not.
  @param view           Preallocated view, owned by the caller */
  void clone_oldest_view(ReadView *view, bool fast = false);

  /**
  @return the number of active views */
  ulint size() const;

  /**
  Set the view creator transaction id. Note: This shouldbe set only
  for views created by RW transactions.
  @param view   Set the creator trx id for this view
  @param id     Transaction id to set */
  static void set_view_creator_trx_id(ReadView *view, trx_id_t id) {
    ut_ad(id > 0);
    view->creator_trx_id(id);
  }

  ReadView *cached_view() { return m_cached_view; }

  void cached_view_slock() { rw_lock_s_lock(m_cached_view_lock, UT_LOCATION_HERE); }

  void cached_view_sunlock() { rw_lock_s_unlock(m_cached_view_lock); }

  void cached_view_xlock() { rw_lock_x_lock(m_cached_view_lock, UT_LOCATION_HERE); }

  bool cached_view_try_xlock() { return rw_lock_x_lock_nowait(m_cached_view_lock, UT_LOCATION_HERE); }

  void cached_view_xunlock() { rw_lock_x_unlock(m_cached_view_lock); }


 private:
  // Prevent copying
  MVCC(const MVCC &);
  MVCC &operator=(const MVCC &);

  // cached view to avoid do lf_hash iterate
  ReadView *m_cached_view;
  rw_lock_t *m_cached_view_lock;
};

#endif /* read0read_h */

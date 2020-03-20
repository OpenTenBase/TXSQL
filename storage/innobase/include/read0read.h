/*****************************************************************************

Copyright (c) 1997, 2018, Oracle and/or its affiliates. All Rights Reserved.

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
#include "ut0link_buf.h"

/** The MVCC read view manager */
class MVCC {
 public:
  /** Constructor */
  explicit MVCC();

  /** Destructor. */
  ~MVCC();

  /**
  Allocate and create a view.
  @param view		view owned by this class created for the
                          caller. Must be freed by calling close()
  @param trx		transaction creating the view */
  void view_open(ReadView *view, trx_t *trx);

  /**
  Close a view created by the above function. */
  void view_close(trx_t *trx);

  /** Clones the oldest view and stores it in view. No need to
  call view_close(). The caller owns the view that is passed in.
  It will also move the closed views from the m_views list to the
  m_free list. This function is called by Purge to create it view.
  @param view		Preallocated view, owned by the caller */
  void clone_oldest_view(ReadView *view);

  /**
  @return the number of active views */
  ulint size() const;

  /**
  Set the view creator transaction id. Note: This shouldbe set only
  for views created by RW transactions. */
  static void set_view_creator_trx_id(ReadView *view, trx_id_t id);

  void set_view_flag(bool new_flag) {
    m_valid_view.store(new_flag, std::memory_order_release);
  }

  bool is_clone_valid() const {
    return (m_valid_view.load(std::memory_order_acquire));
  }

  bool is_clone_valid_relaxed() const {
    return (m_valid_view.load(std::memory_order_relaxed));
  }

  void clone_slock() {
    rw_lock_s_lock(m_clone_lock);
  }

  void clone_sunlock() {
    rw_lock_s_unlock(m_clone_lock);
  }

  void clone_xlock() {
    rw_lock_x_lock(m_clone_lock);
  }

  bool clone_xtrylock() {
    return (rw_lock_x_lock_nowait(m_clone_lock));
  }

  void clone_xunlock() {
    rw_lock_x_unlock(m_clone_lock);
  }

  ReadView* global_view() { return (m_clone_view); }
 private:
  // Prevent copying
  MVCC(const MVCC &);
  MVCC &operator=(const MVCC &);

  /** Pointer to a cached read view.If m_valid_view is true,
  we can directly clone read view from it so can reduce cpu
  cost of iterating lf_hash. */
  ReadView *m_clone_view;

  /** Flag to indicate if m_clone_view can be used to create
  user read view. */
  std::atomic<bool> m_valid_view;

  /** read-write lock to protect m_clone_view */
  rw_lock_t *m_clone_lock;
};

extern bool opt_use_cloned_view;
#endif /* read0read_h */

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

/** The link_buf size for tracking the process of taking snapshot. */
#define MAX_SLOTS 1048576
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
  Close a view created by the above function.
  @param view		view allocated by trx_open.
  @param own_mutex	true if caller owns trx_sys_t::mutex */
  void view_close(trx_t *trx, bool own_mutex);

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

  void add_list(ReadView* view);

  void remove_list(ReadView* view);

  uint64_t register_slot();

  void unregister_slot(uint64_t slot_id);

  void view_closer_task();

  void start_view_closer();

  void stop_view_closer();
#ifdef UNIV_DEBUG
  bool view_on_list(ReadView* view);
#endif

 private:
  /**
  Validates a read view list. */
  bool validate() const;

 private:
  // Prevent copying
  MVCC(const MVCC &);
  MVCC &operator=(const MVCC &);

 private:
  typedef UT_LIST_BASE_NODE_T(ReadView) view_list_t;

  /** Active and closed views, the closed views will have the
  creator trx id set to TRX_ID_MAX */
  view_list_t m_views;

  /** It uses background thread to advance tail of m_add_recently.
  This flag indicates if the thread is active or not */
  std::atomic<bool> m_view_closer_active;
  
  os_event_t  m_view_closer_event;

  /** Increased everytime before taking a snapshot */
  std::atomic<uint64_t> m_create_counter;
 
  /** Track the process of taking snapshot. */
  Link_buf<uint64_t> m_add_recently;
};

#endif /* read0read_h */

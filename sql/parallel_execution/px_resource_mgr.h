/* Copyright (c) 2009, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef PX_RESOURCE_MGR_INCLUDED
#define PX_RESOURCE_MGR_INCLUDED

#include "my_dbug.h"
#include "sql/sql_class.h"
#include "mysql/psi/mysql_mutex.h"  // mysql_mutex_t

class PX_resource_manager {
 private:
  static PX_resource_manager *m_instance; // resource manager instance.

  PX_resource_manager() {}
  ~PX_resource_manager() {}
  /* This class is non-copyable. */
  PX_resource_manager(const PX_resource_manager &);
  PX_resource_manager &operator=(const PX_resource_manager &);

 public:
  static PX_resource_manager *get_instance() {
    assert(m_instance != nullptr);
    return m_instance;
  }
  /**
    Allocate cpu cores from the PX resource manager. 
    @return true if enough to allocate, false otherwise.
  */
  static bool acquire(int64_t cpu_cores);

  /* Recycle resource and decrease the used threadpool size.*/
  static void release(int64_t cpu_cores);

  /**
    Initialize must be called before get_instance() can be used.
    @return true if initialization failed, false otherwise.
  */
  static bool init_instance();
  /** Destroy the singleton instance. */
  static void destroy_instance();
};

#endif // PX_RESOURCE_MGR_INCLUDED
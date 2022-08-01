/* Copyright (c) 2020, Oracle and/or its affiliates.

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

#include "sql/join_optimizer/access_path.h"
#include "sql/join_optimizer/materialize_path_parameters.h"
#if defined(HAVE_PX)
#include "sql/parallel_execution/px_access_path.h"  //EquivalenceCheckHelper
#endif /* defined(HAVE_PX) */

#if defined(HAVE_PX)
bool MaterializePathParameters::eq(const MaterializePathParameters *other) const {
  if (query_blocks.size() != other->query_blocks.size() ||
      !EquivalenceCheckHelper::eq_table_share(table, other->table) ||  
      ref_slice != other->ref_slice ||
      rematerialize != other->rematerialize ||
      limit_rows != other->limit_rows ||
      reject_multiple_rows != other->reject_multiple_rows) {
    return false;
  }

  if (invalidators != nullptr) {
    if (other->invalidators == nullptr ||
        invalidators->size() != other->invalidators->size()) {
      return false;
    }
    const AccessPath *other_invalidator = (*invalidators)[0];
    for (const AccessPath *invalidator : *invalidators) {
      if (other_invalidator == nullptr ||
          strlen(invalidator->cache_invalidator().name) !=
              strlen(other_invalidator->cache_invalidator().name) ||
          strncmp(invalidator->cache_invalidator().name,
                  other_invalidator->cache_invalidator().name,
                  strlen(invalidator->cache_invalidator().name)) != 0) {
        return false;
      }
    }
  } else if (other->invalidators != nullptr) {
    return false;
  }

  if (cte != nullptr) {
    if (other->cte == nullptr ||
        cte->tmp_tables.size() != other->cte->tmp_tables.size()) {
      return false;
    }
    int tmp_table_id = 0;
    for (TABLE_LIST *table_ref : cte->tmp_tables) {
      TABLE_LIST *other_table_ref = other->cte->tmp_tables[tmp_table_id++];
      if (other_table_ref == nullptr) {
        return false;
      }
      if (!EquivalenceCheckHelper::eq_table_share(table_ref->table,
                                                  other_table_ref->table)) {
        return false;
      }
    }
  }

  return true;
}
#endif /* defined(HAVE_PX) */

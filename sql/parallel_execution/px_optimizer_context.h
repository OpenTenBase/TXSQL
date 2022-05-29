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

#ifndef PX_OPTIMIZER_CONTEXT_INCLUDED
#define PX_OPTIMIZER_CONTEXT_INCLUDED

#include "mysql/psi/mysql_thread.h" // mysql_mutex_t
#include "my_base.h"                // key_part_map
#include "my_alloc.h"               // free_root
#include "sql/table.h"              // table_share
#include "sql/mem_root_allocator.h" // Mem_root_allocator
#include "sql/malloc_allocator.h"   // Malloc_allocator
#include "sql/key.h"                // KEY
#include <map>

class THD;
class ha_statistics;
struct MEM_ROOT;

#define OPT_STATS_RUNNING(thd) \
  (cdb_optimization_context_cache_enabled && \
      thd && (thd)->m_is_optimizing)

#define OPT_STATS_ENABLED(thd, TABLE) \
  (OPT_STATS_RUNNING(thd) && table && \
      (table)->s->table_category == TABLE_CATEGORY_USER)

#define OPT_STATS_CACHE(thd) \
  ((thd)->m_is_worker ? (thd)->px_coordinator->opt_stats : (thd)->opt_stats)

#define OPT_STATS_GET(type, thd, ...) \
  OPT_STATS_CACHE(thd)->get_##type(__VA_ARGS__)

#define OPT_STATS_SET(type, thd, ...) \
  OPT_STATS_CACHE(thd)->set_##type(__VA_ARGS__)

#define OPT_STATS_CHECK(err, type, thd) \
  do { if (!err) (thd)->type##_id++; } while (0)

#define OPT_STATS_ERR(type, thd) \
  {                                                                       \
    sql_print_warning("optimization context: Thread(%d) optimization"     \
                      "context mismatch (%s)", (thd)->thread_id(), type); \
    my_error(ER_CDB_OPTIMIZATION_CONTEXT_INCONSISTENT, MYF(0),            \
              (thd)->thread_id(), type);                                  \
  }

/**
  The key standing for arguments of records_in_range(). Used to look up
  the index dive cache (index_dive_map).
*/
struct index_dive_args {
  const TABLE_SHARE *s;
  uint keyno;

  // key_range of min end point
  const uchar *min_end_key;
  uint min_key_length;
  key_part_map min_keypart_map;

  // key_range of max end point
  const uchar *max_end_key;
  uint max_key_length;
  key_part_map max_keypart_map;

  bool init(MEM_ROOT *mem_root, const TABLE_SHARE *table_share,
            const uint keynr, const key_range *min_endp,
            const key_range *max_endp);

  bool eq(const TABLE_SHARE *other_s, const uint other_keyno,
          const key_range *min_endp, const key_range *max_endp) const {
    if (s != other_s ||
        keyno != other_keyno) {
      return false;
    }

    if (min_endp) {
      if (min_key_length != min_endp->length ||
          min_keypart_map != min_endp->keypart_map) {
        return false;
      }
      if (min_end_key && min_endp->key) {
        if (memcmp(min_end_key, min_endp->key, min_key_length) != 0)
          return false;
      } else if (min_end_key || min_endp->key) {
        return false;
      }
    } else if (min_key_length) {
      return false;
    }

    if (max_endp) {
      if (max_key_length != max_endp->length ||
          max_keypart_map != max_endp->keypart_map) {
        return false;
      }
      if (max_end_key && max_endp->key) {
        if (memcmp(max_end_key, max_endp->key, max_key_length) != 0)
          return false;
      } else if (max_end_key || max_endp->key) {
        return false;
      }
    } else if (max_key_length) {
      return false;
    }

    return true;
  }
};

struct Index_dive_args_comparator {
  bool operator() (const index_dive_args &args1,
                   const index_dive_args &args2) const {
    if (args1.s != args2.s) {
      return args1.s < args2.s;
    }
    if (args1.keyno != args2.keyno) {
      return args1.keyno < args2.keyno;
    }
    if (args1.min_key_length != args2.min_key_length) {
      return args1.min_key_length < args2.min_key_length;
    }
    if (args1.min_keypart_map != args2.min_keypart_map) {
      return args1.min_keypart_map < args2.min_keypart_map;
    }
    if (args1.max_key_length != args2.max_key_length) {
      return args1.max_key_length < args2.max_key_length;
    }
    if (args1.max_keypart_map != args2.max_keypart_map) {
      return args1.max_keypart_map < args2.max_keypart_map;
    }

    if (args2.min_end_key) {
      if (!args1.min_end_key) {
        return true;
      }
      int less = memcmp(args1.min_end_key, args2.min_end_key, args1.min_key_length);
      if (less != 0) {
        return less;
      }
    } else if (args1.min_end_key) {
      return false;
    }

    if (args2.max_end_key) {
      if (!args1.max_end_key) {
        return false;
      }
      return memcmp(args1.max_end_key, args2.max_end_key, args1.max_key_length) < 0;
    } else if (args1.max_end_key) {
      return true;
    }

    return false;
  }
};

struct records_per_key_args {
  const KEY *key;
};

struct KEY_comparator {
  bool operator() (const records_per_key_args &key1,
                   const records_per_key_args &key2) const {
    if (key1.key->table->s == key2.key->table->s) {
      return strcmp(key1.key->name, key2.key->name) < 0;
    }
    return key1.key->table->s < key2.key->table->s;
  }
};

/**
   Statistics cache for a top statement. (unsupported substatement)
*/
class Stats_cache {
private:
  MEM_ROOT *m_mem_root;

  // cache table stats
  std::map<TABLE_SHARE *, ha_statistics *, std::less<TABLE_SHARE *>,
           Malloc_allocator<std::pair<TABLE_SHARE * const, ha_statistics *>>
           > ha_stats_map;

  // cache optimization contexts in table_share
  std::map<records_per_key_args, rec_per_key_t *, KEY_comparator,
           Malloc_allocator<std::pair<records_per_key_args const, rec_per_key_t *>>
           > records_per_key_map;

  // cache the value effected by fun::info()
  std::map<records_per_key_args, double, KEY_comparator,
           Malloc_allocator<std::pair<records_per_key_args const, double>>
           > in_memory_estimate_map;

  // cache the rows of index dive
  std::map<index_dive_args, ha_rows, Index_dive_args_comparator,
           Malloc_allocator<std::pair<index_dive_args const, ha_rows>>
           > index_dive_map;

  bool set_ha_stats(const TABLE *table);

public:
  bool get_ha_stats(const TABLE *table, ha_statistics *&stats);

  bool set_ha_info(const TABLE *table);
  bool get_has_records_per_key(const KEY *key, const uint key_part_no,
                               bool &has_rec_per_key);
  bool get_records_per_key(const KEY *key, const uint key_part_no,
                           rec_per_key_t &rec_per_key);
  bool get_in_memory_estimate(const KEY *key, double &estimate);

  bool set_index_dive(const TABLE *table,
                      const uint keyno, const key_range *min_endp,
                      const key_range *max_endp, const ha_rows rows);
  bool get_index_dive(const TABLE *table,
                      const uint keyno, const key_range *min_endp,
                      const key_range *max_endp, ha_rows &rows);
#ifndef DBUG_OFF
  // debug index dive map
  void print_index_dive_map(int error, const TABLE *table,
                            const uint keyno, const key_range *min_endp,
                            const key_range *max_endp);
#endif

  void trace_stats(THD *thd);

  void clear();

  Stats_cache(MEM_ROOT *mem_root);
};

/**
  The correctness of parallel execution depends on the consistency
  execution plan between the worker and the coordinator. So that,
  the worker will generate the same DFO like it generated by the
  coordinator.

  Therefore, we need to ensure that the optimizer environment of
  worker threads are consistent with that of the coordinator thread.

  The optimizer environment of the worker thread is copied from
  the coordinator thread. The function copies:
    - THD::variables
    - THD::table_plugin (default_storage_engine)
    - THD::temp_table_plugin (default_tmp_storage_engine)
    - THD::charset
    - THD::user_vars
*/
extern bool post_init_worker_thd(THD *coordinator_thd, THD *worker_thd);

extern void begin_optimization_context(THD *thd);

extern bool end_optimization_context(THD *thd);

#endif  // PX_OPTIMIZER_CONTEXT_INCLUDED

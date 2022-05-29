#include "px_optimizer_context.h"

#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "mutex_lock.h"           // MUTEX_LOCK
#include "mysys_err.h"
#include "my_dbug.h"
#include "sql/item.h"
#include "sql/key.h"
#include "sql/item_func.h"        // user_var_entry
#include "sql/opt_trace.h"        // Opt_trace_object
#include "sql/opt_trace_context.h"
#include "sql/sql_class.h"        // THD
#include "sql/sql_plugin.h"       // intern_plugin_lock
#include "sql/log.h"              // sql_print_warning()
#include "thr_lock.h"
#include "thr_mutex.h"

#define my_intern_plugin_lock(A, B) intern_plugin_lock(A, B)
#define my_intern_plugin_lock_ci(A, B) intern_plugin_lock(A, B)

bool index_dive_args::init(MEM_ROOT *mem_root,
                           const TABLE_SHARE *table_share,
                           const uint keynr,
                           const key_range *min_endp,
                           const key_range *max_endp) {
  s = table_share;
  keyno = keynr;

  if (min_endp) {
    min_key_length = min_endp->length;
    min_keypart_map = min_endp->keypart_map;
    if (min_endp->length) {
      uchar* tmp_end_key = new (mem_root) uchar[min_endp->length];
      if (tmp_end_key) {
        memcpy(tmp_end_key, min_endp->key, min_endp->length);
        min_end_key = tmp_end_key;
      } else {
        sql_print_warning("optimization context: cannot create end_key");
        return true;
      }
    }
  } else {
    min_key_length = min_keypart_map = 0;
    min_end_key = nullptr;
  }

  if (max_endp) {
    max_key_length = max_endp->length;
    max_keypart_map = max_endp->keypart_map;
    if (max_endp->length) {
      uchar* tmp_end_key = new (mem_root) uchar[max_endp->length];
      if (tmp_end_key) {
        memcpy(tmp_end_key, max_endp->key, max_endp->length);
        max_end_key = tmp_end_key;
      } else {
        sql_print_warning("optimization context: cannot create end_key");
        return true;
      }
    }
  } else {
    max_key_length = max_keypart_map = 0;
    max_end_key = nullptr;
  }

  return false;
}

Stats_cache::Stats_cache(MEM_ROOT *mem_root)
    : m_mem_root(mem_root),
      ha_stats_map(std::less<TABLE_SHARE *>(),
                   Malloc_allocator<std::pair<const TABLE_SHARE *,
                      ha_statistics *>>(key_memory_optimizer_context)),
      records_per_key_map(KEY_comparator(),
                          Malloc_allocator<std::pair<const records_per_key_args,
                              rec_per_key_t *>>(key_memory_optimizer_context)),
      in_memory_estimate_map(KEY_comparator(),
                             Malloc_allocator<std::pair<const records_per_key_args,
                                 double>>(key_memory_optimizer_context)), 
      index_dive_map(Index_dive_args_comparator(),
                     Malloc_allocator<std::pair<const index_dive_args,
                         ha_rows>>(key_memory_optimizer_context)) {}

void Stats_cache::clear() {
  ha_stats_map.clear();
  records_per_key_map.clear();
  in_memory_estimate_map.clear();
  index_dive_map.clear();
}

bool Stats_cache::get_index_dive(const TABLE *table, const uint keyno,
                                 const key_range *min_endp,
                                 const key_range *max_endp,
                                 ha_rows &rows) {
  for (const auto &member : index_dive_map) {
    if(member.first.eq(table->s, keyno, min_endp, max_endp)) {
      rows = member.second;
      return false;
    }
  }

  return true;
}

bool Stats_cache::set_index_dive(const TABLE *table,
                                 const uint keyno,
                                 const key_range *min_endp,
                                 const key_range *max_endp,
                                 const ha_rows rows) {
  index_dive_args *dive_args = new (m_mem_root) index_dive_args;
  if (dive_args->init(m_mem_root, table->s, keyno, min_endp, max_endp)) {
    return true;
  }

  index_dive_map.emplace(*dive_args, rows);
  return false;
}

#ifndef DBUG_OFF
void Stats_cache::print_index_dive_map(int error, const TABLE *table,
                                       const uint keyno,
                                       const key_range *min_endp,
                                       const key_range *max_endp) {
  index_dive_args dive_args;
  dive_args.init(m_mem_root, table->s, keyno, min_endp, max_endp);
  String range_min;
  range_min.set_charset(system_charset_info);
  if (dive_args.min_end_key && *dive_args.min_end_key) {
    range_min.append(STRING_WITH_LEN("NULL"));
  }
  range_min.append(STRING_WITH_LEN("-"));
  for (uint i = 1; i < dive_args.min_key_length; i++) {
    range_min.append(_dig_vec_lower[*(dive_args.min_end_key + i) >> 4]);
    range_min.append(_dig_vec_lower[*(dive_args.min_end_key + i) & 0x0F]);
  }

  String range_max;
  if (dive_args.max_end_key && *dive_args.max_end_key) {
    range_max.append(STRING_WITH_LEN("NULL"));
  }
  range_max.append(STRING_WITH_LEN("-"));
  for (uint i = 1; i < dive_args.max_key_length; i++) {
    range_max.append(_dig_vec_lower[*(dive_args.max_end_key + i) >> 4]);
    range_max.append(_dig_vec_lower[*(dive_args.max_end_key + i) & 0x0F]);
  }
  sql_print_warning("optimization context debug: %s find %lu:%s(%lu)-%s(%lu)",
                    error ? "cannot" : "", keyno,
                    range_min.c_ptr(), dive_args.min_key_length,
                    range_max.c_ptr(), dive_args.max_key_length);

  for (const auto &member : index_dive_map) {
    const index_dive_args *args = &(member.first);

    String min_str;
    min_str.set_charset(system_charset_info);
    if (args->min_end_key && *(args->min_end_key)) {
      min_str.append(STRING_WITH_LEN("NULL"));
    }
    min_str.append(STRING_WITH_LEN("-"));
    for (uint i = 1; i < args->min_key_length; i++) {
      min_str.append(_dig_vec_lower[*(args->min_end_key + i) >> 4]);
      min_str.append(_dig_vec_lower[*(args->min_end_key + i) & 0x0F]);
    }

    String max_str;
    max_str.set_charset(system_charset_info);
    if (args->max_end_key && *(args->max_end_key)) {
      max_str.append(STRING_WITH_LEN("NULL"));
    }
    max_str.append(STRING_WITH_LEN("-"));
    for (uint i = 1; i < args->max_key_length; i++) {
      max_str.append(_dig_vec_lower[*(args->max_end_key + i) >> 4]);
      max_str.append(_dig_vec_lower[*(args->max_end_key + i) & 0x0F]);
    }
    sql_print_warning("optimization context debug: %lu:%s(%lu)-%s(%lu)",
                      args->keyno, min_str.c_ptr(), args->min_key_length,
                      max_str.c_ptr(), args->max_key_length);
  }
}
#endif

bool Stats_cache::get_ha_stats(const TABLE *table,
                               ha_statistics *&stats) {
  auto it = ha_stats_map.find(table->s);
  if (it != ha_stats_map.end()) {
    stats = it->second;
    return false;
  }

  stats = nullptr;
  return true;
}

bool Stats_cache::set_ha_stats(const TABLE *table) {
  ha_statistics *stats = new (m_mem_root) ha_statistics;
  if (!stats) {
    sql_print_warning("optimization context: cannot create ha_statistics");
    return true;
  }
  stats->copy_from(&table->file->stats);

  ha_stats_map.emplace(table->s, stats);
  return false;
}

bool Stats_cache::get_has_records_per_key(const KEY *key,
                                          const uint key_part_no,
                                          bool &has_rec_per_key) {
  records_per_key_args key_args;
  key_args.key = key;

  auto it = records_per_key_map.find(key_args);
  if (it != records_per_key_map.end()) {
    has_rec_per_key = (it->second[key_part_no] &&
                       it->second[key_part_no] != REC_PER_KEY_UNKNOWN);
    return false;
  }

  return true;
}

bool Stats_cache::get_records_per_key(const KEY *key,
                                      const uint key_part_no,
                                      rec_per_key_t &rec_per_key) {
  records_per_key_args key_args;
  key_args.key = key;

  auto it = records_per_key_map.find(key_args);
  if (it != records_per_key_map.end()) {
    rec_per_key = it->second[key_part_no];
    return false;
  }

  return true;
}

bool Stats_cache::get_in_memory_estimate(const KEY *key, double &estimate) {
  records_per_key_args key_args;
  key_args.key = key;

  auto it = in_memory_estimate_map.find(key_args);
  if (it != in_memory_estimate_map.end()) {
    estimate = it->second;
    return false;
  }

  return true;
}

bool Stats_cache::set_ha_info(const TABLE *table) {
  if (set_ha_stats(table)) {
    return true;
  }

  for (uint i = 0; i < table->s->keys; i++) {
    KEY *key = &table->key_info[i];

    records_per_key_args key_args;
    key_args.key = key;

    auto it = records_per_key_map.find(key_args);
    if (it != records_per_key_map.end()) {
      assert(false);
      return true;
    }

    rec_per_key_t *tmp_rec_per_keys_float =
        new (m_mem_root) rec_per_key_t[key->actual_key_parts];
    if (!tmp_rec_per_keys_float) {
      sql_print_warning("optimization context: cannot create ha_info cache");
      return true;
    }
    for (ulong j = 0; j < key->actual_key_parts; j++) {
      tmp_rec_per_keys_float[j] = key->rec_per_key_float[j];
    }

    records_per_key_map.emplace(key_args, tmp_rec_per_keys_float);

    in_memory_estimate_map.emplace(key_args, key->m_in_memory_estimate);
  }

  return false;
}

void Stats_cache::trace_stats(THD *thd) {
  Opt_trace_context *const trace = &thd->opt_trace;

  Opt_trace_object trace_wrapper(trace, Opt_trace_context::STATISTICS);
  Opt_trace_array trace_statistics(trace, "optimizer_statistics");

  for (auto &it : ha_stats_map) {
    Opt_trace_object trace_table(trace, "table_object");
    trace_table.add_utf8("db", it.first->db.str ? it.first->db.str : "");
    trace_table.add_utf8("table", it.first->table_name.str);

    {
      Opt_trace_object trace_ha_stats(trace, "table_statistics");
      trace_ha_stats.add("data_file_length", it.second->data_file_length);
      trace_ha_stats.add("delete_length", it.second->delete_length);
      trace_ha_stats.add("records", it.second->records);
      trace_ha_stats.add("deleted", it.second->deleted);
      trace_ha_stats.add("table_in_mem_estimate",
                         it.second->table_in_mem_estimate);
    }

    Opt_trace_array trace_keys(trace, "index_statistics");
    for (auto &key_it : records_per_key_map) {
      if (key_it.first.key->table->s != it.first) continue;
      Opt_trace_object trace_key(trace, "key_object");
      trace_key.add_utf8("name", key_it.first.key->name);

      {
        for (auto &mem_it : in_memory_estimate_map) {
          if (mem_it.first.key->table->s != it.first) continue;
          trace_key.add("in_memory_estimate", mem_it.second);
          break;
        }
      }

      {
        Opt_trace_array trace_rec_per_key(trace, "records_per_key");
        for (uint i = 0; i < key_it.first.key->actual_key_parts; i++) {
          trace_rec_per_key.add(key_it.second[i]);
        }
      }

      {
        Opt_trace_array trace_rec_per_key(trace, "records_in_range");
        for (const auto &member : index_dive_map) {
          const index_dive_args *args = &(member.first);
          if (args->s != it.first) continue;
          if (strcmp(args->s->key_info[args->keyno].name,
                     key_it.first.key->name)) continue;

          Opt_trace_object trace_dive(trace, "records_in_range");

          String min_str;
          min_str.set_charset(system_charset_info);
          if (args->min_end_key && *(args->min_end_key)) {
            min_str.append(STRING_WITH_LEN("NULL"));
          }
          min_str.append(STRING_WITH_LEN("-"));
          for (uint i = 1; i < args->min_key_length; i++) {
            min_str.append(_dig_vec_lower[*(args->min_end_key + i) >> 4]);
            min_str.append(_dig_vec_lower[*(args->min_end_key + i) & 0x0F]);
          }
          trace_dive.add_utf8("min_endp", min_str.c_ptr());

          String max_str;
          max_str.set_charset(system_charset_info);
          if (args->max_end_key && *(args->max_end_key)) {
            max_str.append(STRING_WITH_LEN("NULL"));
          }
          max_str.append(STRING_WITH_LEN("-"));
          for (uint i = 1; i < args->max_key_length; i++) {
            max_str.append(_dig_vec_lower[*(args->max_end_key + i) >> 4]);
            max_str.append(_dig_vec_lower[*(args->max_end_key + i) & 0x0F]);
          }
          trace_dive.add_utf8("max_endp", max_str.c_ptr());

          trace_dive.add("rows", member.second);
        }
      }
    } // keys

  } // table
}

bool post_init_worker_thd(THD *coordinator_thd, THD *worker_thd) {
  assert(coordinator_thd && worker_thd);
  DBUG_TRACE;

  plugin_ref old_table_plugin = worker_thd->variables.table_plugin;
  plugin_ref old_temp_table_plugin = worker_thd->variables.temp_table_plugin;

  worker_thd->variables.table_plugin = nullptr;
  worker_thd->variables.temp_table_plugin = nullptr;

  // 1. copy variables
  cleanup_variables(worker_thd, &worker_thd->variables);
  worker_thd->variables = coordinator_thd->variables;

  // 2. update table_plugin
  mysql_mutex_lock(&LOCK_plugin);
  worker_thd->variables.table_plugin =
    my_intern_plugin_lock(nullptr, coordinator_thd->variables.table_plugin);
  intern_plugin_unlock(nullptr, old_table_plugin);
  worker_thd->variables.temp_table_plugin = my_intern_plugin_lock(
    nullptr, coordinator_thd->variables.temp_table_plugin);
  intern_plugin_unlock(nullptr, old_temp_table_plugin);
  mysql_mutex_unlock(&LOCK_plugin); 

  // 3. copy charset
  worker_thd->charset_is_system_charset =
      coordinator_thd->charset_is_system_charset;
  worker_thd->charset_is_collation_connection =
      coordinator_thd->charset_is_collation_connection;
  worker_thd->charset_is_character_set_filesystem =
      coordinator_thd->charset_is_character_set_filesystem;

  // 4. copy user_vars
  // get lock for get_variable
  mysql_mutex_lock(&worker_thd->LOCK_thd_data);

  worker_thd->user_vars.clear();
  worker_thd->user_vars.reserve(coordinator_thd->user_vars.size());

  // default charset
  const CHARSET_INFO *cs = coordinator_thd->variables.collation_connection;

  for (const auto &key_and_value : coordinator_thd->user_vars) {
    user_var_entry *sql_uvar = key_and_value.second.get();

    /*
      deep copy for worker thd, for each coordinator's entry:
      - build locally a new entry (construction)
      - copy value from coordinator's entry
      - add it to the worker_thd.
      Thus, the entry will destruction with worker thd.
    */

    /* Copy VARIABLE_NAME */
    const char *name = sql_uvar->entry_name.ptr();
    size_t name_length = sql_uvar->entry_name.length();
    const Name_string key(name, name_length);

    user_var_entry *entry = get_variable(worker_thd, key, cs);
    if (entry != nullptr) {
      /* Copy VARIABLE_VALUE */
      bool null_value;
      String *str_value;
      String str_buffer;
      uint decimals = 0;
      str_value = sql_uvar->val_str(&null_value, &str_buffer, decimals);
      if (str_value != nullptr) {
        entry->store(str_value->ptr(), str_value->length(),
                    STRING_RESULT, cs, DERIVATION_IMPLICIT,
                    false /* unsigned_arg */);
      } else {
        entry->store(nullptr, 0,
                    STRING_RESULT, cs, DERIVATION_IMPLICIT,
                    false /* unsigned_arg */);
      }
    }
  }

  mysql_mutex_unlock(&worker_thd->LOCK_thd_data);

  return false;
}

void begin_optimization_context(THD *thd) {
  assert(!thd->m_is_optimizing);
  thd->m_is_optimizing = true;
  // copy the optimizer related version before optimization in coordinator
  if (OPT_STATS_RUNNING(thd) && !thd->m_is_worker) {
    /*
      TODO deep copy outline / optimizer cost / rewriter
      At present, since they will not be modified frequently, we only
      check whether they have been modified (version number).
    */
    thd->saved_outline_reload_version = outline_reload_version;
    thd->saved_optimizer_cost_reload_version = optimizer_cost_reload_version;
    thd->saved_rewriter_plugin_reload_version = rewriter_plugin_reload_version;
  }
}

bool end_optimization_context(THD *thd) {
  assert(thd->m_is_optimizing);
  thd->m_is_optimizing = false;
  if (cdb_optimization_context_cache_enabled) {
    if (unlikely(thd->opt_trace.is_started())) {
      OPT_STATS_CACHE(thd)->trace_stats(thd);
    }
  }

  if (OPT_STATS_RUNNING(thd) && thd->m_is_worker && !thd->in_sub_stmt) {
    assert(thd->px_coordinator);
    assert(thd->px_coordinator->saved_outline_reload_version != -1L);
    assert(thd->px_coordinator->saved_optimizer_cost_reload_version != -1L);
    assert(thd->px_coordinator->saved_rewriter_plugin_reload_version != -1L);

    if (thd->ha_stats_id != thd->px_coordinator->ha_stats_id) {
      OPT_STATS_ERR("ha_stats_id", thd);
      return true;
    } else if (thd->index_dive_id != thd->px_coordinator->index_dive_id) {
      OPT_STATS_ERR("index_dive_id", thd);
      return true;
    } else if (thd->px_coordinator->saved_outline_reload_version !=
        outline_reload_version) {
      OPT_STATS_ERR("outline version", thd);
      return true;
    } else if (thd->px_coordinator->saved_optimizer_cost_reload_version !=
        optimizer_cost_reload_version) {
      OPT_STATS_ERR("optimizer_cost version", thd);
      return true;
    } else if (thd->px_coordinator->saved_rewriter_plugin_reload_version !=
        rewriter_plugin_reload_version) {
      OPT_STATS_ERR("rewriter version", thd);
      return true;
    } else if (!thd->use_px) {
      OPT_STATS_ERR("execution mode", thd);
      return true;
    }
  }
  return false;
}

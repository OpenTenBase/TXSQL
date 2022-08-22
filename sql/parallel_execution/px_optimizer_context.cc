#include "px_optimizer_context.h"

#include "sql/parallel_execution/opt_interface.h"  // Opt_ctx_client
#include "sql/parallel_execution/px_interface.h"   // PX_ROLE_COORDINATOR

#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "mutex_lock.h"           // MUTEX_LOCK
#include "mysys_err.h"            // EE_CAPACITY_EXCEEDED
#include "my_dbug.h"
#include "sql/item.h"
#include "sql/key.h"
#include "sql/item_func.h"        // user_var_entry
#include "sql/opt_trace.h"        // Opt_trace_object
#include "sql/opt_trace_context.h"
#include "sql/sql_class.h"        // THD
#include "sql/sql_lex.h"          // LEX
#include "sql/sql_plugin.h"       // intern_plugin_lock
#include "sql/current_thd.h"
#include "sql/derror.h"           // ER_THD
#include "sql/error_handler.h"    // Internal_error_handler
#include "thr_lock.h"
#include "thr_mutex.h"
#include "mysql/thread_pool_priv.h"

#define my_intern_plugin_lock(A, B) intern_plugin_lock(A, B)
#define my_intern_plugin_lock_ci(A, B) intern_plugin_lock(A, B)

/*
  Note that optimizer context kicks in before parallel optimizer, so PX_PRINT_
  macros might not work. Here are simulations.
 */

#define OPT_CTX_TEXT(fmt, ...) \
    ("px:%ld:%u:%u " fmt, 0L, 0, (current_thd)->thread_id(), \
     ##__VA_ARGS__)
#define OPT_CTX_WARN(fmt, ...) \
    DBUG_PRINT("pwarn", OPT_CTX_TEXT(fmt, ##__VA_ARGS__))
#define OPT_CTX_TRACE(fmt, ...) \
    DBUG_PRINT("ptrace", OPT_CTX_TEXT(fmt, ##__VA_ARGS__))
#define OPT_CTX_TRACE_CLIENT(fmt, ...) \
    DBUG_PRINT("ptrace", \
            OPT_CTX_TEXT("opt:%d " fmt, m_nested_level, ##__VA_ARGS__))
#define OPT_CTX_TRACE_TABLE(op, table, fmt, ...) \
    DBUG_PRINT("ptrace", \
        OPT_CTX_TEXT("opt:%d %s %s.%s (%s) " fmt, m_nested_level, (op), \
            (table)->s->db.str, (table)->s->table_name.str, (table)->alias, \
            ##__VA_ARGS__))
#define OPT_CTX_TRACE_KEY(op, key, fmt, ...) \
    DBUG_PRINT("ptrace", \
        OPT_CTX_TEXT("opt:%d %s %s.%s (%s) %s %p " fmt, m_nested_level, (op), \
            (key)->table ? (key)->table->s->db.str : "?", \
            (key)->table ? (key)->table->s->table_name.str : "?", \
            (key)->table ? (key)->table->alias : "?", \
            (key)->name, key, ##__VA_ARGS__))

inline const char *to_str(enum enum_opt_call_type type) {
  switch (type) {
    case OPT_CALL_HA_STAT:
      return "ha_stats_id";
    case OPT_CALL_INDEX_DIVE:
      return "index_dive_id";
    default:
      return "?";
  }
};

inline const char *to_str(enum enum_opt_repo_type type) {
  switch (type) {
    case OPT_REPO_OUTLINE:
      return "outline version";
    case OPT_REPO_COST:
      return "optimizer_cost version";
    case OPT_REPO_REWRITER:
      return "rewriter version";
    default:
      return "?";
  }
}

static bool post_init_worker_thd(THD *coordinator_thd, THD *worker_thd);

/**
  Optimization context.

  An optimization context represents all that are needed for an optimization
  process, including optimizer settings, rules and statistics.

  It is expected that a context object could be built in one optimization
  process, and reused in another one to generate the same execution plan.

  Because MySQL allows execution during optimization. It is effectively an
  execution context.
 */
class Opt_ctx {
 public:
   Opt_ctx(PSI_memory_key psi_memory_key);
   ~Opt_ctx();

  void reset();
  void cleanup();

  /// Statistics cache.
  Stats_cache *stats_cache() { return &m_opt_stats; }

  const LEX_CSTRING &query() const { return m_query_string; }
  const LEX_CSTRING &db() const { return m_db; }

  /// Set the query in optimization.
  void set_query(LEX_CSTRING query_arg);
  /// Set the default db.
  void set_db(const LEX_CSTRING &new_db);
  /// Set optimizer rule repo version.
  void set_rule_version(enum enum_opt_repo_type type, long long version);
  void set_calls(enum enum_opt_call_type type, int calls);
  /// Set given THD as a source of optimization context.
  void set_env(THD *thd);
#ifndef DBUG_OFF
  void set_source_session(Opt_dbug_session *session);
  Opt_dbug_session *source_session();

  /**
    Apply DBUG session to given THD. Must be called in the given thread,
    because DBUG setting stack is thread local. Was DBUG_SET().
   */
  bool dbug_init_thd(THD *thd);
#endif
  /// Apply the optimization context to given THD. Was post_init_worker_thd().
  bool init_thd(THD *thd);
  /**
    Init in the specified thread, e.g. operations related to thread locals.

    Although dbug_init_thd() also must be called in the current thread, it is
    separated to avoid double-init because in some cases it is invoked out of
    band, preceding regular init here.
   */
  bool post_init_thd(THD *thd);

 private:
  // Provide the source execution environment, although far from clear now.
  THD *m_thd;

  // Original query text. May be different from THD::m_query_string due to
  // Rewriter plugin.
  LEX_CSTRING m_query_string;
  // Original default db.
  LEX_CSTRING m_db;

#ifndef DBUG_OFF
  // DBUG session state of the source thread.
  Opt_dbug_session *m_source_session;
#endif

  MEM_ROOT m_stats_cache_alloc;
  /// The statistics cache.
  Stats_cache m_opt_stats;

 public:
  /// Counters for statistic calls. Used as a loose validation.
  int m_calls[OPT_CALL_TYPE_LEN];
  /**
    Versions for optimizer rule repositories.

    Used to implement optimistic locking mechanism. They are saved by the
    coordinator at the beginning of optimization, and verified by each worker
    at the end of optimization. Any version change is likely to get a different
    SQL execution plan for the parallel thread, thus failing parallel execution.
    However, it is expected that such changes are very rare, so it is fairly OK
    to be optimistic. Plan change is also doubly checked by comparing physical
    plan structures.
   */
  long long m_versions[OPT_REPO_TYPE_LEN];
};

extern "C" void sql_alloc_error_handler(void);

Opt_ctx::Opt_ctx(PSI_memory_key psi_memory_key)
    : m_thd(), m_query_string(NULL_CSTR), m_db(NULL_CSTR),
      m_stats_cache_alloc(psi_memory_key, 16384 /* 16 kB */),
      m_opt_stats(&m_stats_cache_alloc) {
  assert(current_thd);
  m_stats_cache_alloc.set_max_capacity(current_thd->variables.txsql_optimizer_context_max_mem_size);
  m_stats_cache_alloc.set_error_for_capacity_exceeded(true);
  m_stats_cache_alloc.set_error_handler(sql_alloc_error_handler);
  for (int i = 0; i < OPT_CALL_TYPE_LEN; i++) m_calls[i] = 0;
  for (int i = 0; i < OPT_REPO_TYPE_LEN; i++) m_versions[i] = -1L;
}

Opt_ctx::~Opt_ctx() {}

void Opt_ctx::reset() {
  cleanup();
  m_thd = nullptr;
  m_query_string = NULL_CSTR;
  m_db = NULL_CSTR;
  for (int i = 0; i < OPT_CALL_TYPE_LEN; i++) m_calls[i] = 0;
  for (int i = 0; i < OPT_REPO_TYPE_LEN; i++) m_versions[i] = -1L;
}

void Opt_ctx::cleanup() {
  m_opt_stats.clear();
  m_stats_cache_alloc.Clear();
}

void Opt_ctx::set_query(LEX_CSTRING query_arg) {
  m_query_string = query_arg;
}

void Opt_ctx::set_db(const LEX_CSTRING &new_db) {
  m_db = new_db;
}

void Opt_ctx::set_calls(enum enum_opt_call_type type, int calls) {
  m_calls[type] = calls;
}

void Opt_ctx::set_rule_version(enum enum_opt_repo_type type,
                              long long version) {
  m_versions[type] = version;
}

void Opt_ctx::set_env(THD *thd) {
  m_thd = thd;
}

#ifndef DBUG_OFF
void Opt_ctx::set_source_session(Opt_dbug_session *session) {
  m_source_session = session;
}

Opt_dbug_session *Opt_ctx::source_session() {
  return m_source_session;
}

bool Opt_ctx::dbug_init_thd(THD *thd) {
  return m_source_session->dbug_init_thd(thd);
}
#endif

bool Opt_ctx::init_thd(THD *thd) {
  return post_init_worker_thd(m_thd, thd);
}

bool Opt_ctx::post_init_thd(THD *thd) {
  return false;
}

// Intercepting client.

inline bool ignore_table(TABLE *table) {
  // Intercepted call may be on TABLE_SHARE, when there is no table.
  // Statistic calls for system tables are usually irrelative to current
  // optimization. Ignore them even for an involved system table, because
  // it is not regular user query.
  return !table || table->s->table_category != TABLE_CATEGORY_USER;
}

Opt_ctx_client::Opt_ctx_client(PSI_memory_key psi_memory_key, THD *thd)
    : m_thd(thd), m_nested_level(0), m_optimizing(false), m_opt_ctx(),
      m_mode(OPT_CTX_NATIVE) {
  for (int i = 0; i < OPT_CALL_TYPE_LEN; i++) m_calls[i] = 0;
  for (int i = 0; i < OPT_REPO_TYPE_LEN; i++) m_versions[i] = -1L;
#ifndef DBUG_OFF
  m_dbug_session = &thd->opt_dbug_session;
#endif
}

Opt_ctx_client::~Opt_ctx_client() {}

void Opt_ctx_client::reset_for_next_command() {
  OPT_CTX_TRACE_CLIENT("reset");
  assert(m_nested_level == 0);

  m_nested_level = 0;
  for (int i = 0; i < OPT_CALL_TYPE_LEN; i++) m_calls[i] = 0;
  for (int i = 0; i < OPT_REPO_TYPE_LEN; i++) m_versions[i] = -1L;
}

void Opt_ctx_client::cleanup_after_query() {
  OPT_CTX_TRACE_CLIENT("cleanup");
  if (m_nested_level > 0) return;

  // Release the context and, if necessary, destroy it.
  m_opt_ctx.reset();
  m_mode = OPT_CTX_NATIVE;
}

void Opt_ctx_client::begin_sub_statement() {
  OPT_CTX_TRACE_CLIENT("begin_sub_statement");
  assert(m_nested_level >= 0);
  m_nested_level++;
}

void Opt_ctx_client::end_sub_statement() {
  OPT_CTX_TRACE_CLIENT("end_sub_statement");
  m_nested_level--;
  assert(m_nested_level >= 0);
}

void Opt_ctx_client::begin_optimization() {
  OPT_CTX_TRACE_CLIENT("begin_optimization");
  if (m_nested_level > 0) return;

  if (!OPT_CTX_ENABLED(m_thd)) {
    return;
  }

  m_optimizing = true;

  assert(m_nested_level == 0);
  if (m_mode != OPT_CTX_NATIVE) {
    assert(outline_reload_version != -1);
    assert(optimizer_cost_reload_version != -1);
    assert(rewriter_plugin_reload_version != -1);
    m_versions[OPT_REPO_OUTLINE] = outline_reload_version;
    m_versions[OPT_REPO_COST] = optimizer_cost_reload_version;
    m_versions[OPT_REPO_REWRITER] = rewriter_plugin_reload_version;
  }
}

void Opt_ctx_client::end_optimization() {
  OPT_CTX_TRACE_CLIENT("end_optimization");
  if (m_nested_level > 0) return;

  m_optimizing = false;

  if (!OPT_CTX_ENABLED(m_thd)) {
    return;
  }

  assert(m_nested_level == 0);
  if (m_mode == OPT_CTX_RECORD) {
    // Save the source context.
    m_opt_ctx->set_env(m_thd);
    for (int i = 0; i < OPT_CALL_TYPE_LEN; i++) {
      m_opt_ctx->set_calls((enum enum_opt_call_type)i, m_calls[i]);
    }
    for (int i = 0; i < OPT_REPO_TYPE_LEN; i++) {
      m_opt_ctx->set_rule_version((enum enum_opt_repo_type)i, m_versions[i]);
    }
  }
}

bool Opt_ctx_client::validate() {
  OPT_CTX_TRACE_CLIENT("validate");
  if (m_nested_level > 0 || m_mode  == OPT_CTX_NATIVE) return false;

  const char *error_type = nullptr;

  long long versions[OPT_REPO_TYPE_LEN];
  versions[OPT_REPO_OUTLINE] = outline_reload_version;
  versions[OPT_REPO_COST] = optimizer_cost_reload_version;
  versions[OPT_REPO_REWRITER] = rewriter_plugin_reload_version;
  for (int i = 0; i < OPT_REPO_TYPE_LEN; i++) {
    if (versions[i] != m_versions[i]) {
      error_type = to_str((enum enum_opt_repo_type)i);
      goto err;
    }
  }

  if (m_mode == OPT_CTX_REPLAY) {
    for (int i = 0; i < OPT_REPO_TYPE_LEN; i++) {
      if (versions[i] != m_opt_ctx->m_versions[i]) {
        error_type = to_str((enum enum_opt_repo_type)i);
        goto err;
      }
    }
    for (int i = 0; i < OPT_CALL_TYPE_LEN; i++) {
      if (m_calls[i] != m_opt_ctx->m_calls[i]) {
        error_type = to_str((enum enum_opt_call_type)i);
        goto err;
      }
    }
  }

  return false;

err:

  OPT_CTX_WARN("optimization context: Thread(%u) optimization "
                 "context mismatch (%s)", m_thd->thread_id(), error_type);
  my_error(ER_CDB_OPTIMIZATION_CONTEXT_INCONSISTENT, MYF(0),
           m_thd->thread_id(), error_type);

  return true;
}

void Opt_ctx_client::trace_stats() {
  if (m_mode != OPT_CTX_NATIVE) {
    m_opt_ctx->stats_cache()->trace_stats(m_thd);
  }
}

inline const char *to_str(enum enum_opt_ctx_mode mode) {
  switch (mode) {
    case OPT_CTX_NATIVE:
      return "native";
    case OPT_CTX_RECORD:
      return "record";
    case OPT_CTX_REPLAY:
      return "replay";
    default:
      return "?";
  }
}

void Opt_ctx_client::set_ctx(enum enum_opt_ctx_mode mode,
                             std::shared_ptr<Opt_ctx> ctx) {
  assert(mode != OPT_CTX_REPLAY || ctx);
  OPT_CTX_TRACE_CLIENT("set_ctx %p %s", ctx.get(), to_str(mode));

  if (m_nested_level > 0) return;

  if (mode != OPT_CTX_NATIVE && !ctx) {
    ctx.reset(new (std::nothrow) Opt_ctx(key_memory_optimizer_context));
  }
  // Switch to non-native mode only if having connected to an optimization
  // context.
  if (ctx) {
    m_opt_ctx = ctx;
    m_mode = mode;
#ifndef DBUG_OFF
    // The replayer inherits DBUG session state through the context.
    if (m_mode == OPT_CTX_RECORD) {
      m_opt_ctx->set_source_session(m_dbug_session);
    } else if (m_mode == OPT_CTX_REPLAY) {
      m_dbug_session = m_opt_ctx->source_session();
    }
#endif
  } else {
    m_mode = OPT_CTX_NATIVE;
#ifndef DBUG_OFF
    m_dbug_session = &m_thd->opt_dbug_session;
#endif
  }
}

bool Opt_ctx_client::init_query() {
  bool ret = false;
  if (m_mode == OPT_CTX_REPLAY) {
    OPT_CTX_TRACE_CLIENT("init_query %s", m_opt_ctx->query().str);
    // void
    m_thd->set_query(m_opt_ctx->query());
  } else {
    assert(0);
    ret = true;
  }
  return ret;
}

bool Opt_ctx_client::init_db() {
  bool ret = false;
  if (m_mode == OPT_CTX_REPLAY) {
    OPT_CTX_TRACE_CLIENT("init_db %s",
                         m_opt_ctx->db().str ? m_opt_ctx->db().str : "(null)");
    ret = m_thd->set_db(m_opt_ctx->db());
  } else {
    assert(0);
    ret = true;
  }
  return ret;
}

#ifndef DBUG_OFF
bool Opt_ctx_client::dbug_init_thd() {
  bool ret = false;
  if (m_mode == OPT_CTX_REPLAY) {
    ret = m_opt_ctx->dbug_init_thd(m_thd);
  } else {
    assert(0);
    ret = true;
  }
  return ret;
}
#endif

bool Opt_ctx_client::init_thd() {
  bool ret = false;
  if (m_mode == OPT_CTX_REPLAY) {
    OPT_CTX_TRACE_CLIENT("init_thd");
    ret = m_opt_ctx->init_thd(m_thd);
  } else {
    assert(0);
    ret = true;
  }
  return ret;
}

bool Opt_ctx_client::post_init_thd() {
  assert(current_thd == m_thd);
  bool ret = false;
  if (m_mode == OPT_CTX_REPLAY) {
    OPT_CTX_TRACE_CLIENT("post_init_thd");
    ret = m_opt_ctx->post_init_thd(m_thd);
  } else {
    assert(0);
    ret = true;
  }
  return ret;
}

void Opt_ctx_client::set_query(LEX_CSTRING query_arg) {
  OPT_CTX_TRACE_CLIENT("set_query %s", query_arg.str);
  if (m_nested_level == 0 && m_mode == OPT_CTX_RECORD) {
    m_opt_ctx->set_query(query_arg);
  }
}

void Opt_ctx_client::set_db(const LEX_CSTRING &new_db) {
  OPT_CTX_TRACE_CLIENT("set_db %s", new_db.str ? new_db.str : "(null)");
  if (m_nested_level == 0 && m_mode == OPT_CTX_RECORD) {
    m_opt_ctx->set_db(new_db);
  }
}

int Opt_ctx_client::info(TABLE *table, uint flag) {
  OPT_CTX_TRACE_CLIENT(
      "info %s.%s (%s) %x", table->s->db.str, table->s->table_name.str,
      table->alias, flag);

  // See the comment of m_optimizing.
  if (!m_optimizing || m_mode == OPT_CTX_NATIVE || ignore_table(table))
    return 0;

  assert(m_mode != OPT_CTX_NATIVE);
  ha_statistics *stats = nullptr;
  int err = m_opt_ctx->stats_cache()->get_ha_stats(table, stats);
  if (err) {
    if (m_mode == OPT_CTX_REPLAY) {
      assert(0);
      // Translate cache miss (true) to proper error code.
      err = HA_ERR_INTERNAL_ERROR;
      goto end;
    }

    if (m_mode == OPT_CTX_RECORD) {
      m_calls[OPT_CALL_HA_STAT]++;
      err = m_opt_ctx->stats_cache()->set_ha_info(table);
    }
  } else {
    // Overwrite stats because it has no accessor function, while interactions
    // with the others are intercepted in their callback functions.
    table->file->stats.copy_from(stats);
    m_calls[OPT_CALL_HA_STAT]++;
  }

end:
  return err;
}

#ifndef DBUG_OFF
static void print_memory(const void *ptr, size_t length, String &out) {
  for (uint i = 0; i < length; i++) {
    out.append(_dig_vec_lower[*((const char*)ptr + i) >> 4]);
    out.append(_dig_vec_lower[*((const char*)ptr + i) & 0x0F]);
  }
}

static void print_endp(key_range *endp, String &out) {
  if (!endp) return;
  char buf[sizeof(longlong)*2+1];
  if (endp->key) print_memory(endp->key, 1, out);
  else out.append("?");
  out.append(STRING_WITH_LEN("-"));
  if (endp->key) print_memory(endp->key + 1, endp->length - 1, out);
  else out.append("?");
  out.append(STRING_WITH_LEN("-"));
  sprintf(buf, "%lx", endp->keypart_map);
  out.append(buf);
  out.append(STRING_WITH_LEN("-"));
  sprintf(buf, "%x", endp->flag);
  out.append(buf);
}
#endif

ha_rows Opt_ctx_client::records_in_range(TABLE *table, uint keyno,
                                  key_range *min_endp,
                                  key_range *max_endp) {
  ha_rows rows;

  if (!m_optimizing || m_mode == OPT_CTX_NATIVE || ignore_table(table))
    rows = table->file->records_in_range(keyno, min_endp, max_endp);
  else {
    int err = true;
    Stats_cache *cache = m_opt_ctx->stats_cache();
    err = cache->get_index_dive(table, keyno, min_endp, max_endp, rows);
    if (err) {
      if (m_mode == OPT_CTX_REPLAY) {
#ifndef DBUG_OFF
        cache->print_index_dive_map(err, table, keyno, min_endp, max_endp);
#endif
        assert(0);
        rows = HA_POS_ERROR;
        goto end;
      }
      rows = table->file->records_in_range(keyno, min_endp, max_endp);
      if (rows != HA_POS_ERROR) {
        if (cache->set_index_dive(table, keyno, min_endp, max_endp, rows)) {
          rows = HA_POS_ERROR;
          goto end;
        }
        m_calls[OPT_CALL_INDEX_DIVE]++;
      }
    } else {
      m_calls[OPT_CALL_INDEX_DIVE]++;
    }
  }

end:
#ifndef DBUG_OFF
  String out;
  print_endp(min_endp, out);
  out.append("..");
  print_endp(max_endp, out);
  out.append('\0');
  OPT_CTX_TRACE_TABLE(
      "records_in_range", table, "%u %s %llu", keyno, out.ptr(), rows);
#endif
  return rows;
}

bool Opt_ctx_client::has_records_per_key(const KEY *key, uint key_part_no) {
  bool has;

  if (!m_optimizing || m_mode == OPT_CTX_NATIVE || ignore_table(key->table))
    has = key->has_records_per_key_low(key_part_no);
  else {
    assert(key_part_no < key->actual_key_parts);
    bool err = m_opt_ctx->stats_cache()->
        get_has_records_per_key(key, key_part_no, has);
    if (err) {
      // Never miss because info() result is cached.
      assert(0);
      has = false;
      goto end;
    }
  }

end:
  OPT_CTX_TRACE_KEY("has_records_per_key", key, "%u %d",  key_part_no, has);
  return has;
}

rec_per_key_t Opt_ctx_client::records_per_key(const KEY *key,
                                              uint key_part_no) {
  rec_per_key_t tmp_rec_per_key;
  if (!m_optimizing || m_mode == OPT_CTX_NATIVE || ignore_table(key->table))
    tmp_rec_per_key = key->records_per_key_low(key_part_no);
  else {
    assert(key_part_no < key->actual_key_parts);
    if (m_opt_ctx->stats_cache()->get_records_per_key(
            key, key_part_no, tmp_rec_per_key)) {
      // Never miss because info() result is cached.
      assert(0);
      tmp_rec_per_key = REC_PER_KEY_UNKNOWN;
    }
  }
  OPT_CTX_TRACE_KEY(
      "records_per_key", key, "%u %g", key_part_no, tmp_rec_per_key);
  return tmp_rec_per_key;
}

void Opt_ctx_client::set_records_per_key(KEY *key, uint key_part_no,
                                  rec_per_key_t rec_per_key_est) {
  OPT_CTX_TRACE_CLIENT(
      "set_records_per_key %p %u %g", key, key_part_no, rec_per_key_est);
  // Complete the callback path of handler::info().
  key->set_records_per_key_low(key_part_no, rec_per_key_est);
}

bool Opt_ctx_client::supports_records_per_key(const KEY *key) {
  // Test any array set by set_rec_per_key_array().
  bool ret = key->supports_records_per_key_low();
  OPT_CTX_TRACE_KEY("supports_records_per_key", key, "%d", ret);
  return ret;
}

void Opt_ctx_client::set_rec_per_key_array(KEY *key, ulong *rec_per_key_arg,
                                    rec_per_key_t *rec_per_key_float_arg) {
  OPT_CTX_TRACE_CLIENT("set_rec_per_key_array %p %p %p", key, rec_per_key_arg,
                       rec_per_key_float_arg);
  // When a table is opened by open_table_from_share(), the arrays are
  // implicitly set to the shared instances in TABLE_SHARE. Whether or not
  // the arrays are set, is related to properties of the table rather than
  // operation mode or thread role.
  key->set_rec_per_key_array_low(rec_per_key_arg, rec_per_key_float_arg);
}

double Opt_ctx_client::in_memory_estimate(const KEY *key) {
  double tmp_estimate;
  if (!m_optimizing || m_mode == OPT_CTX_NATIVE || ignore_table(key->table))
    tmp_estimate = key->in_memory_estimate_low();
  else {
    if (m_opt_ctx->stats_cache()->get_in_memory_estimate(key, tmp_estimate)) {
      // Never miss because info() result is cached.
      assert(0);
      tmp_estimate = IN_MEMORY_ESTIMATE_UNKNOWN;
    }
  }
  assert(tmp_estimate == IN_MEMORY_ESTIMATE_UNKNOWN ||
              (tmp_estimate >= 0.0 && tmp_estimate <= 1.0));
  OPT_CTX_TRACE_KEY("in_memory_estimate", key, "%g", tmp_estimate);
  return tmp_estimate;
}

void Opt_ctx_client::set_in_memory_estimate(KEY *key,
                                            double in_memory_estimate) {
  OPT_CTX_TRACE_CLIENT("set_in_memory_estimate %p %g", key, in_memory_estimate);
  // Complete the callback path of handler::info().
  key->set_in_memory_estimate_low(in_memory_estimate);
}

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
        OPT_CTX_WARN("optimization context: cannot create end_key");
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
        OPT_CTX_WARN("optimization context: cannot create end_key");
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

/**
  Error handling class for optimizer context. We handle only out of memory
  error here. This is to give a hint to the user to
  raise txsql_optimizer_context_max_mem_size if required.
  Warning for the memory error is pushed only once. The consequent errors
  will be ignored.
*/
class Px_optimizer_context_error_handler : public Internal_error_handler {
 public:
  Px_optimizer_context_error_handler()
      : m_is_mem_error(false) {}

  bool handle_condition(THD *thd, uint sql_errno, const char *,
                        Sql_condition::enum_severity_level *level,
                        const char *) override {
    if (*level == Sql_condition::SL_ERROR) {
      /* Out of memory error is reported only once. Return as handled */
      if (m_is_mem_error && sql_errno == EE_CAPACITY_EXCEEDED) return true;
      if (sql_errno == EE_CAPACITY_EXCEEDED) {
        m_is_mem_error = true;
        /* Convert the error into a warning. */
        *level = Sql_condition::SL_WARNING;
        push_warning_printf(
            thd, Sql_condition::SL_WARNING, ER_CAPACITY_EXCEEDED,
            ER_THD(thd, ER_CAPACITY_EXCEEDED),
            (ulonglong)thd->variables.txsql_optimizer_context_max_mem_size,
            "ER_PX_CAPACITY_EXCEEDED_IN_OPTIMIZER_CONTEXT",
            ER_THD(thd, ER_PX_CAPACITY_EXCEEDED_IN_OPTIMIZER_CONTEXT));

        /* close the cache and fallback to serial explain */
        OPT_CTX(thd).set_ctx(OPT_CTX_NATIVE);

        if (thd->lex->is_explain()) {
          return true;
        }

#if defined(HAVE_PX)
        mysql_mutex_lock(&LOCK_optimizer_context_memory_exceeded_counter);
        txsql_max_optimizer_context_memory_exceeded++;
        mysql_mutex_unlock(&LOCK_optimizer_context_memory_exceeded_counter);
#endif /* defined(HAVE_PX) */
        my_error(ER_PX_CAPACITY_EXCEEDED_IN_OPTIMIZER_CONTEXT, MYF(0));
        return true;
      }
    }
    return false;
  }

 private:
  bool m_is_mem_error;
};

bool Stats_cache::set_index_dive(const TABLE *table,
                                 const uint keyno,
                                 const key_range *min_endp,
                                 const key_range *max_endp,
                                 const ha_rows rows) {
  assert(current_thd);
  Px_optimizer_context_error_handler error_handler;
  current_thd->push_internal_handler(&error_handler);

  index_dive_args *dive_args = new (m_mem_root) index_dive_args;
  bool error = current_thd->is_error();
  if (error) {
    return error;
  }

  if (!dive_args ||
      dive_args->init(m_mem_root, table->s, keyno, min_endp, max_endp)) {
    current_thd->pop_internal_handler();
    return true;
  }
  current_thd->pop_internal_handler();

  index_dive_map.emplace(*dive_args, rows);
  return false;
}

#ifndef DBUG_OFF
void Stats_cache::print_index_dive_map(int error, const TABLE *table,
                                       const uint keyno,
                                       const key_range *min_endp,
                                       const key_range *max_endp) {
  index_dive_args dive_args;
  assert(current_thd);
  Px_optimizer_context_error_handler error_handler;
  current_thd->push_internal_handler(&error_handler);
  bool err =
      dive_args.init(m_mem_root, table->s, keyno, min_endp, max_endp);
  current_thd->pop_internal_handler();

  if (err) {
    return;
  }

  String range_min;
  // TODO: see 'ranges' of range_scan_alternatives tab
  range_min.set_charset(system_charset_info);
  range_min.append(STRING_WITH_LEN("<"));
  for (uint i = 0; i < dive_args.min_key_length; i++) {
    range_min.append(_dig_vec_lower[*(dive_args.min_end_key + i) >> 4]);
    range_min.append(_dig_vec_lower[*(dive_args.min_end_key + i) & 0x0F]);
  }
  range_min.append(STRING_WITH_LEN(">"));

  String range_max;
  range_max.append(STRING_WITH_LEN("<"));
  for (uint i = 0; i < dive_args.max_key_length; i++) {
    range_max.append(_dig_vec_lower[*(dive_args.max_end_key + i) >> 4]);
    range_max.append(_dig_vec_lower[*(dive_args.max_end_key + i) & 0x0F]);
  }
  range_max.append(STRING_WITH_LEN(">"));
  OPT_CTX_WARN("optimization context debug: %s find %u:%s(%u)-%s(%u)",
                 error ? "cannot" : "", keyno,
                 range_min.c_ptr(), dive_args.min_key_length,
                 range_max.c_ptr(), dive_args.max_key_length);

  for (const auto &member : index_dive_map) {
    const index_dive_args *args = &(member.first);

    String min_str;
    // TODO: see 'ranges' of range_scan_alternatives tab
    min_str.set_charset(system_charset_info);
    min_str.append(STRING_WITH_LEN("<"));
    for (uint i = 0; i < args->min_key_length; i++) {
      min_str.append(_dig_vec_lower[*(args->min_end_key + i) >> 4]);
      min_str.append(_dig_vec_lower[*(args->min_end_key + i) & 0x0F]);
    }
    min_str.append(STRING_WITH_LEN(">"));

    String max_str;
    max_str.set_charset(system_charset_info);
    max_str.append(STRING_WITH_LEN("<"));
    for (uint i = 0; i < args->max_key_length; i++) {
      max_str.append(_dig_vec_lower[*(args->max_end_key + i) >> 4]);
      max_str.append(_dig_vec_lower[*(args->max_end_key + i) & 0x0F]);
    }
    max_str.append(STRING_WITH_LEN(">"));
    OPT_CTX_WARN("optimization context debug: %u:%s(%u)-%s(%u)",
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
  assert(current_thd);
  Px_optimizer_context_error_handler error_handler;
  current_thd->push_internal_handler(&error_handler);

  ha_statistics *stats = new (m_mem_root) ha_statistics;
  bool error = current_thd->is_error();
  current_thd->pop_internal_handler();

  if (error || !stats) {
    OPT_CTX_WARN("optimization context: cannot create ha_statistics");
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
    if (!key->supports_records_per_key()) continue;

    records_per_key_args key_args;
    key_args.key = key;

    auto it = records_per_key_map.find(key_args);
    if (it != records_per_key_map.end()) {
      assert(false);
      return true;
    }

    assert(current_thd);
    Px_optimizer_context_error_handler error_handler;
    current_thd->push_internal_handler(&error_handler);

    rec_per_key_t *tmp_rec_per_keys_float =
        new (m_mem_root) rec_per_key_t[key->actual_key_parts];
    bool error = current_thd->is_error();
    current_thd->pop_internal_handler();

    if (error || !tmp_rec_per_keys_float) {
      OPT_CTX_WARN("optimization context: cannot create ha_info cache");
      return true;
    }
    for (ulong j = 0; j < key->actual_key_parts; j++) {
      if (key->rec_per_key_float[j] != REC_PER_KEY_UNKNOWN)
        tmp_rec_per_keys_float[j] = key->rec_per_key_float[j];
      else if (key->rec_per_key[j] != 0)
        tmp_rec_per_keys_float[j] = static_cast<rec_per_key_t>(key->rec_per_key[j]);
      else
        tmp_rec_per_keys_float[j] = REC_PER_KEY_UNKNOWN;
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
          trace_dive.add_utf8("ranges", "see 'ranges' of range_scan_alternatives tab");
          trace_dive.add("rows", member.second);
        }
      }
    } // keys

  } // table
}

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
static bool post_init_worker_thd(THD *coordinator_thd, THD *worker_thd) {
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
      entry->store(sql_uvar);
    }
  }

  mysql_mutex_unlock(&worker_thd->LOCK_thd_data);

  // 5. copy other vars in thd
  // for LOAD DATA INFILE
  worker_thd->file_id = coordinator_thd->file_id;
  // remote (peer) port
  worker_thd->peer_port = coordinator_thd->peer_port;
  worker_thd->start_time = coordinator_thd->start_time;
  worker_thd->user_time = coordinator_thd->user_time;
  worker_thd->start_utime = coordinator_thd->start_utime;
  //worker_thd->utime_after_lock = coordinator_thd->utime_after_lock;

  worker_thd->time_zone_used = coordinator_thd->time_zone_used;
  worker_thd->rand_used = coordinator_thd->rand_used;

  // for trx
#if defined(HAVE_PX)
  worker_thd->px_trx = coordinator_thd->px_trx;
#endif /* defined(HAVE_PX) */
  worker_thd->tx_isolation = coordinator_thd->tx_isolation;
  worker_thd->tx_read_only = coordinator_thd->tx_read_only;
  DBUG_EXECUTE_IF("px_force_isolation", {
    worker_thd->tx_isolation = ISO_REPEATABLE_READ;
  });

  // is null
  worker_thd->arg_of_last_insert_id_function =
      coordinator_thd->arg_of_last_insert_id_function;
  worker_thd->first_successful_insert_id_in_prev_stmt =
    coordinator_thd->first_successful_insert_id_in_prev_stmt;
  worker_thd->first_successful_insert_id_in_prev_stmt_for_binlog =
    coordinator_thd->first_successful_insert_id_in_prev_stmt_for_binlog;
  worker_thd->first_successful_insert_id_in_cur_stmt =
    coordinator_thd->first_successful_insert_id_in_cur_stmt;
  worker_thd->stmt_depends_on_first_successful_insert_id_in_prev_stmt =
    coordinator_thd->stmt_depends_on_first_successful_insert_id_in_prev_stmt;

  // copy source value of user() / Item_func_user
#if defined(HAVE_PX)
  worker_thd->m_security_ctx->px_copy_from(coordinator_thd->m_security_ctx);
#ifndef DBUG_OFF
  coordinator_thd->px_worker_executing = true;
#endif
#endif /* defined(HAVE_PX) */

  worker_thd->m_main_security_ctx.set_user_ptr(
      coordinator_thd->security_context()->user().str,
      coordinator_thd->security_context()->user().length);
  worker_thd->m_main_security_ctx.set_host_or_ip_ptr(
      coordinator_thd->security_context()->host_or_ip().str,
      coordinator_thd->security_context()->host_or_ip().length);

  if (coordinator_thd->is_cmd_skip_readonly())
    worker_thd->set_skip_readonly_check();

  /*
    Ticket store (MDL_context::m_ticket_store) is not thread-safe by design,
    although it still allows concurrent reads.

    Using the coordinator as a MDL caching proxy for all workers (#449) is
    based on the assumption that the ticket store is read only with respect to
    concurrent access. Such an assumption does not stand because certain tickets
    are short-lived thus not in the ticket store, and any parallel thread might
    issue new MDL requests when evaluating certain SQL functions.

    Here each worker makes a copy of the ticket store of the coordinator, thus
    becomes standalone with respect to MDL. The only exception is that
    MDL_EXPLICIT should be released explicity, and is prevented by
    LEX::check_px_execution().
   */
  for (int i = 0; i < MDL_DURATION_END; i++) {
    worker_thd->mdl_context.clone_tickets(&coordinator_thd->mdl_context,
                                          (enum_mdl_duration)i);
  }

  thd_set_net_read_write(worker_thd, 0);

  return false;
}

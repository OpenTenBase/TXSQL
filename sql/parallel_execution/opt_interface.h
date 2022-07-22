/**
  @file sql/parallel_execution/opt_interface.h

  Optimizer features.

  1. Optimization context.

  Provided through optimization context interceptor (Opt_ctx_client).
*/

#ifndef OPT_INTERFACE_INCLUDED
#define OPT_INTERFACE_INCLUDED

#include "my_base.h"  // ha_rows
#include "mysql/components/services/bits/psi_memory_bits.h" // PSI_memory_key

#include "sql/key.h"        // rec_per_key_t
#include "sql/sql_class.h"  // THD, because it is used by OPT_CTX_ macros.
#include "sql/parallel_execution/px_interface.h" // txsql_parallel_execution_enabled

class THD;
class TABLE;
class KEY;
class key_range;
class Opt_ctx;
class Opt_dbug_session;

/// Opt_ctx_client operation mode.
enum enum_opt_ctx_mode {
  /// Exactly the same behavior as without intercepting client.
  OPT_CTX_NATIVE,
  /// As a cold caching proxy. Any single statistic call is never issued twice.
  /// This is a trivial difference than the native behavior.
  OPT_CTX_RECORD,
  /// As a warmup cache. Any statistic call must be served by the cache.
  OPT_CTX_REPLAY
};

/// Statistic call types. The number of calls serves as a loose validation
/// mechanism, with the assumption that building the same plan gets the same
/// number of calls.
enum enum_opt_call_type {
  OPT_CALL_HA_STAT,
  OPT_CALL_INDEX_DIVE,
  OPT_CALL_TYPE_LEN
};

/// Identify optimizer rule repositories which are likely to get plan changes.
enum enum_opt_repo_type {
  /// mysql.outline
  OPT_REPO_OUTLINE,
  /// mysql.server_cost mysql.engine_cost
  OPT_REPO_COST,
  /// query_rewrite.rewrite_rules
  OPT_REPO_REWRITER,
  OPT_REPO_TYPE_LEN
};

/**
  Optimization context intercepting client.

  An optimization context represents all that are needed for an optimization
  process, including optimizer settings, rules and statistics.

  It is expected that a context object could be built in one optimization
  process, and reused in another one to generate the same execution plan.
  To achieve this goal, a dedicated client connecting to the context object
  is attached to intercept optimizer context calls as well as extract settings
  and rules from the environment.

  An intercepting operation may introduce additional errors, however, it still
  conforms to the call convention in each intercepting site. It is observed
  that special values are reserved as errors, then THD::is_error() could be
  tested after each interaction.
 */
class Opt_ctx_client {
 public:
   Opt_ctx_client(PSI_memory_key psi_memory_key, THD *thd);
   ~Opt_ctx_client();

  /// To intercept THD::reset_for_next_command().
  void reset_for_next_command();
  /// To intercept THD::cleanup_after_query().
  void cleanup_after_query();

  /// To intercept THD::reset_sub_statement_state().
  void begin_sub_statement();
  /// To intercept THD::restore_sub_statement_state().
  void end_sub_statement();

  /// To intercept the beginning of optimization in
  /// Sql_cmd_dml::execute_inner().
  void begin_optimization();
  /// To intercept the end of optimization in Sql_cmd_dml::execute_inner().
  void end_optimization();

  /// Validate optimization against the connected context.
  bool validate();
  /// Add statistics used in optimization to opt trace.
  void trace_stats();

  THD* thd() const { return m_thd; }
  int level() const { return m_nested_level; }
  enum enum_opt_ctx_mode mode() const { return m_mode; }
  std::shared_ptr<Opt_ctx> opt_ctx() const { return m_opt_ctx; }

  /// Change mode and connect to a context, and create one if necessary.
  void set_ctx(enum enum_opt_ctx_mode mode,
               std::shared_ptr<Opt_ctx> ctx = nullptr);

  /// Initialize the query text of the attached thd by the connected
  /// optimization context.
  bool init_query();
  /// Initialize the default db of the attached thd.
  bool init_db();
#ifndef DBUG_OFF
  /// Init the attached (current) thd by the connected optimization context.
  bool dbug_init_thd();
#endif
  /// Init the attached thd by the connected optimization context.
  bool init_thd();
  /// Init in the running thread. e.g. manipulate thread-locals, by the
  /// connected optimization context.
  bool post_init_thd();

  /// Set the query in optimization.
  void set_query(LEX_CSTRING query_arg);
  /// Set the default db.
  void set_db(const LEX_CSTRING &new_db);

  /// To intercept handler::info().
  int info(TABLE *table, uint flag);
  /// To intercept handler::records_in_range().
  ha_rows records_in_range(TABLE *table, uint keyno, key_range *min_endp,
                          key_range *max_endp);

  /// To intercept KEY::has_records_per_key().
  bool has_records_per_key(const KEY *key, uint key_part_no);
  /// To intercept KEY::records_per_key().
  rec_per_key_t records_per_key(const KEY *key, uint key_part_no);
  /// To intercept KEY::set_records_per_key().
  void set_records_per_key(KEY *key, uint key_part_no,
                           rec_per_key_t rec_per_key_est);
  bool supports_records_per_key(const KEY *key);
  /// To intercept KEY::set_rec_per_key_array().
  void set_rec_per_key_array(KEY *key, ulong *rec_per_key_arg,
                             rec_per_key_t *rec_per_key_float_arg);
  /// To intercept KEY::in_memory_estimate().
  double in_memory_estimate(const KEY *key);
  /// To intercept KEY::set_in_memory_estimate().
  void set_in_memory_estimate(KEY *key, double in_memory_estimate);

 private:
  /// The THD that the client is attached to.
  THD *m_thd;
  /// Sub statement nesting level.
  int m_nested_level;
  /**
    Note that there are essentially four kinds of optimizer statistics, namely
    1. ha_statistics
    2. record per key
    3. in memory estimate
    4. records in range

    handler::info() with proper flags gets 1~3 and writes by callbacks of
    direct handler::stats assignments, KEY::set_records_per_key() and
    KEY::set_in_memory_estimate(), respectively. Be warned that these callback
    functions are also used to set up temporary KEY objects, thus intercepted
    calls could see partial objects.

    Besides certain purposes, e.g. detecting exact rows, handler::info() is
    essentially called in two sites. The first is in handler::open(), and the
    second is in TABLE_LIST::fetch_number_of_rows() right before optimization.

    It is really messy to intercept sites other than the second, because they
    are scattered around. If only the second is intercepted, it is impossible
    to tell whether a cache miss of statistics request therebefore is expected
    behavior or not.

    As a result, an explicit state (m_optimizing) is added to indicate having
    passed the second site.

    In addition, intercepting info() should not be simply serving by the cache,
    because info() is not limited to fetch optimizer statistics. Instead info()
    must be called regardless of modes, then non-native modes overwrites by
    cached values.

    Obviously, statistics for (internal) temporary tables should be removed from
    context, because the table is private to the statement (thread).
   */
  bool m_optimizing;

  /// The optimization context that the client is currently connected to.
  std::shared_ptr<Opt_ctx> m_opt_ctx;
#ifndef DBUG_OFF
  /// DBUG interactive session state of either the attached THD or connected
  /// optimization context.
  Opt_dbug_session *m_dbug_session;
#endif
  /// Instruct the client to act as native behavior, to build
  /// a consistent context or to replay with a complete context.
  enum enum_opt_ctx_mode m_mode;

  /// Counters for statistic calls.
  int m_calls[OPT_CALL_TYPE_LEN];
  /// Versions for optimizer rule repositories.
  long long m_versions[OPT_REPO_TYPE_LEN];
};

// Avoid dependency on px_interface.h
extern unsigned long px_max_parallel_threads;

/**
  Tell if optimization context is enabled as well as applicable.
    1. Currently enabled implicitly as a sub feature of parallel execution.
    2. Almost useless for system threads, although applicable except for
       optimizer rule repositiory init version assertions (init_file.test).
    3. The intercepting client is attached.
 */
#define OPT_CTX_ENABLED(thd) \
  (px_max_parallel_threads > 0 && \
   txsql_parallel_execution_enabled && \
   (thd)->system_thread == NON_SYSTEM_THREAD && \
   (thd)->opt_ctx_client)
/// Access to the interceptor, assuming the protection of OPT_CTX_ENABLED().
#define OPT_CTX(thd) (*(thd)->opt_ctx_client)

/// RAII class to ensure the end of optimization in current nested lelvel.
class Auto_optimization_scope {
 public:
  Auto_optimization_scope(THD *thd) : m_thd(thd), m_need_close(false) {}
  ~Auto_optimization_scope() {
    if (m_need_close) end();
  }
  void begin() {
    assert(!m_need_close);
    m_need_close = true;
    OPT_CTX(m_thd).begin_optimization();
  }
  void end() {
    assert(m_need_close);
    m_need_close = false;
    OPT_CTX(m_thd).end_optimization();
  }
 private:
  THD *m_thd;
  bool m_need_close;
};

#endif  // OPT_INTERFACE_INCLUDED

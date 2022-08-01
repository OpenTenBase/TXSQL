#ifndef PX_INTERFACE_INCLUDED
#define PX_INTERFACE_INCLUDED

/**
  @file sql/parallel_execution/px_interface.h

  PX public interface.
 */

#include "my_config.h"

#if defined(HAVE_OPT_CTX)
#include "sql/parallel_execution/opt_interface.h"
#else
#error "Optimization context is required by parallel execution."
#endif

#include "sql/sql_class.h"  // THD, because it is used by PX_ macros.

bool px_init(void);
void px_destroy(void);

extern unsigned long txsql_max_parallel_worker_threads;
extern bool txsql_parallel_fallback_in_execution;
extern bool txsql_parallel_execution_enabled;

class THD;
struct SHOW_VAR;

int show_txsql_parallel_stmt_executed(THD *, SHOW_VAR *var, char *buff);
int show_txsql_parallel_stmt_fallback(THD *, SHOW_VAR *var, char *buff);
int show_txsql_parallel_stmt_error(THD *, SHOW_VAR *var, char *buff);
int show_txsql_parallel_threads_currently_used(THD *, SHOW_VAR *var, char *buff);
int show_txsql_parallel_stmt_thread_refused(THD *, SHOW_VAR *var, char *buff);
int show_txsql_parallel_stmt_hint_executed(THD *, SHOW_VAR *var, char *buff);
int show_txsql_parallel_stmt_memory_refused(THD *, SHOW_VAR *var, char *buff);

void reset_txsql_parallel_stmt_executed();
void reset_txsql_parallel_stmt_fallback();
void reset_txsql_parallel_stmt_error();
void reset_txsql_parallel_stmt_thread_refused();
void reset_txsql_parallel_stmt_hint_executed();
void reset_txsql_parallel_stmt_memory_refused();

class THD;
class RowIterator;
struct AccessPath;
class JOIN;
class PX_executor;

#define PX_ENABLED(thd) (txsql_max_parallel_worker_threads > 0 && txsql_parallel_execution_enabled)
#define PX_EXECUTOR(thd) (thd)->px_executor

#define PX_ROOT_ITERATOR(unit) (unit)->root_iterator()
#define PX_ROOT_ACCESS_PATH(unit) (unit)->root_access_path()
#define PX_ROOT_JOIN(unit) \
    ((unit)->is_union() ? \
     ((unit)->fake_query_block ? (unit)->fake_query_block->join : nullptr) : \
     (unit)->first_query_block()->join)

bool px_validate(THD *thd);

bool px_execute_init(THD *thd, RowIterator *root_itrator, AccessPath *root_path,
                     JOIN *root_join, int64_t &dop);
bool px_explain_init(THD *thd, AccessPath *root_path, JOIN *root_join);
bool px_execute_in_coordinator(THD *thd, int64_t dop);
bool px_execute_in_worker(THD *thd);

void fallback_to_serial_execution(THD *thd, Parser_state *parser_state,
                                  const char *query_string,
                                  size_t query_length);

#endif

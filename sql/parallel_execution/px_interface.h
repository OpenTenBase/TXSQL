#ifndef PX_INTERFACE_INCLUDED
#define PX_INTERFACE_INCLUDED

#include "sql/sql_class.h"  // THD

/**
  @file sql/parallel_execution/px_interface.h

  PX public interface.
 */

#define PX_ENABLED(thd) (px_max_parallel_threads > 0)

bool px_init(void);
void px_destroy(void);

extern unsigned long px_max_parallel_threads;
extern bool px_fallback_in_execution;

int show_px_stmt_executed(THD *, SHOW_VAR *var, char *buff);
int show_px_stmt_fallback(THD *, SHOW_VAR *var, char *buff);
int show_px_stmt_error(THD *, SHOW_VAR *var, char *buff);
int show_px_used_threadpool_size(THD *, SHOW_VAR *var, char *buff);

class THD;
class RowIterator;
struct AccessPath;
class JOIN;
class PX_executor;

/*
  Assume thd->use_px, otherwise the coordinator is indistinguishable from
  serial executor.
 */
#define PX_ROLE_COORDINATOR(thd) !(thd)->m_is_worker
#define PX_ROLE_WORKER(thd) (thd)->m_is_worker
#define PX_EXECUTOR(thd) (thd)->px_executor

bool px_execute_init(THD *thd, RowIterator *root_itrator, AccessPath *root_path,
                     JOIN *root_join, int64_t &dop);
bool px_execute_in_coordinator(THD *thd, RowIterator *root_iterator, int64_t dop);
bool px_execute_in_worker(THD *thd, RowIterator *root_iterator);

void fallback_to_serial_execution(THD *thd, Parser_state *parser_state);

#endif

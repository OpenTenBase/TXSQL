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

bool px_optimize(THD *thd, RowIterator *root_itrator, AccessPath *root_path,
                 JOIN *root_join, int64_t &dop);
bool px_execute_in_coordinator(THD *thd, RowIterator *root_iterator, int64_t dop);
bool px_execute_in_worker(THD *thd, RowIterator *root_iterator);

#endif

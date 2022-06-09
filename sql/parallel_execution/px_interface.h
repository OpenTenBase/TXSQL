#ifndef PX_INTERFACE_INCLUDED
#define PX_INTERFACE_INCLUDED

/**
  @file sql/parallel_execution/px_interface.h

  PX public interface.
 */

#define PX_ENABLED(thd) (px_max_parallel_threads > 0)

bool px_init(void);
void px_destroy(void);


extern unsigned long px_max_parallel_threads;
extern bool px_fallback_in_execution;

#endif

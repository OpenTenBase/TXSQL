#ifndef PX_INTERFACE_INCLUDED
#define PX_INTERFACE_INCLUDED

/**
  @file sql/parallel_execution/px_interface.h

  PX public interface.
 */

#define PX_ENABLED(thd) (px_max_parallel_threads > 0)

extern void px_init_psi_keys(void);

extern unsigned long px_max_parallel_threads;
extern bool px_fallback_in_execution;

#endif

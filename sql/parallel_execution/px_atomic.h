#ifndef PX_ATOMIC_INCLUDED
#define PX_ATOMIC_INCLUDED

#include "my_inttypes.h"

/*
 * px_(read|write)_barrier - prevent the CPU from reordering memory access
 *
 * A read barrier must act as a compiler barrier, and in addition must
 * guarantee that any loads issued prior to the barrier are completed before
 * any loads issued after the barrier.  Similarly, a write barrier acts
 * as a compiler barrier, and also orders stores.  Read and write barriers
 * are thus weaker than a full memory barrier, but stronger than a compiler
 * barrier.  In practice, on machines with strong memory ordering, read and
 * write barriers may require nothing more than a compiler barrier.
 */
#define px_write_barrier() __atomic_thread_fence(__ATOMIC_RELEASE)
#define px_read_barrier() __atomic_thread_fence(__ATOMIC_ACQUIRE)

/*
 * px_compiler_barrier - prevent the compiler from moving code across
 *
 * A compiler barrier need not (and preferably should not) emit any actual
 * machine code, but must act as an optimization fence: the compiler must not
 * reorder loads or stores to main memory around the barrier.  However, the
 * CPU may still reorder loads or stores at runtime, if the architecture's
 * memory model permits this.
 */
#define px_compiler_barrier() __asm__ __volatile__("" ::: "memory")

/*
 * px_memory_barrier - prevent the CPU from reordering memory access
 *
 * A memory barrier must act as a compiler barrier, and in addition must
 * guarantee that all loads and stores issued prior to the barrier are
 * completed before any loads or stores issued after the barrier.  Unless
 * loads and stores are totally ordered (which is not the case on most
 * architectures) this requires issuing some sort of memory fencing
 * instruction.
 */
#define px_memory_barrier() __sync_synchronize()

/** atomic CAS operation */
static inline bool px_atomic_compare_exchange_u64(volatile uint64 *ptr,
                                                  uint64 *expected,
                                                  uint64 newval) {
  bool ret;
  uint64 current;
  current = __sync_val_compare_and_swap(ptr, *expected, newval);
  ret = current == *expected;
  *expected = current;
  return ret;
}

/** atomic read uint64 value */
static inline uint64 px_atomic_read_u64(volatile uint64 *ptr) {
  uint64 old = 0;
  px_atomic_compare_exchange_u64(ptr, &old, 0);
  return old;
}

/** atomic write uint64 value */
static inline uint64 px_atomic_write_u64(volatile uint64 *ptr, uint64 val) {
  uint64 old = *ptr;
  while (!px_atomic_compare_exchange_u64(ptr, &old, val)) {
    ;
  }
  return old;
}

#endif
#ifndef PX_TYPES_INCLUDED
#define PX_TYPES_INCLUDED

/**
  Use this AggTypr to sign the state of aggregate.

  PX_LOCAL_AGG and PX_FINAL_AGG mean this aggrgeate is split for parallel
  execution. PX_LOCAL_AGG signs aggregate in worker, PX_FINAL_AGG signs
  aggregate in coordinator.
*/
enum class AggType {
  PX_NONE = 0,
  PX_LOCAL_AGG,
  PX_FINAL_AGG
};

#endif

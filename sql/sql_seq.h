/*
  Created in August 2018 by daviezhao
  Oracle server style sequence interface.
*/
#ifndef SQL_SEQ_H
#define SQL_SEQ_H
#include <stdint.h>
#include <map>
#include <utility>
#include <string>
#include <pthread.h>
#include "my_compiler.h"
#include "sql/auth/partitioned_rwlock.h"

class THD;

class Sequence
{
public:
  typedef int64_t seq_val_t;
  const static seq_val_t InvalidSeqValue= 0x8000000000000000; // -2^63
private:
  int step;
  int refcnt;
  seq_val_t start, cur_val;// current value that's already allocated
  seq_val_t max, max_cached; // max cached sequence value.
  seq_val_t min, min_cached; // max cached sequence value.
  bool cycle;

  uint n_cache;
  std::string db;
  std::string name;
  mutable pthread_mutex_t mutex;
  bool reserve_range();
  bool set_next_val(); // set the next_val in storage
  seq_val_t next_val;

public:
  class Scoped_pthread_mutex {
    pthread_mutex_t &mtx_ref;
  public:
    Scoped_pthread_mutex(pthread_mutex_t &mtx) : mtx_ref(mtx) {
      pthread_mutex_lock(&mtx_ref);
    }

    ~Scoped_pthread_mutex() {
      pthread_mutex_unlock(&mtx_ref);
    }
  };

  Sequence(const std::string &db_, const std::string &name_, int step_, seq_val_t start_,
      bool cycle_, uint n_cache_=20, seq_val_t max_= InvalidSeqValue,
      seq_val_t min_= InvalidSeqValue)
    :
      step(step_), refcnt(0), start(start_), cur_val(InvalidSeqValue),
      max(max_), max_cached(InvalidSeqValue),
      min(min_), min_cached(InvalidSeqValue),
      cycle(cycle_), n_cache(n_cache_), db(db_), name(name_), next_val(InvalidSeqValue) {
    pthread_mutex_init(&mutex, NULL);
  }

  ~Sequence() {
    pthread_mutex_destroy(&mutex);
  }

  void set_max_cached(seq_val_t v) {
    cur_val = max_cached= v;
  }

  void set_min_cached(seq_val_t v) {
    cur_val = min_cached= v;
  }

  int get_step() const {
    Scoped_pthread_mutex spm(mutex);
    return step;
  }

  seq_val_t get_start() const {
    Scoped_pthread_mutex spm(mutex);
    return start;
  }

  seq_val_t get_max() const {
    Scoped_pthread_mutex spm(mutex);
    return max;
  }

  seq_val_t get_min() const {
    Scoped_pthread_mutex spm(mutex);
    return min;
  }

  bool get_cycle() const {
    Scoped_pthread_mutex spm(mutex);
    return cycle;
  }

  uint get_n_cache() const {
    Scoped_pthread_mutex spm(mutex);
    return n_cache;
  }

  void set_step(int step_) {
    Scoped_pthread_mutex spm(mutex);
    step = step_;
  }

  void set_start(seq_val_t start_) {
    Scoped_pthread_mutex spm(mutex);
    start = start_;
  }

  void set_max(seq_val_t v) {
    Scoped_pthread_mutex spm(mutex);
    max = v;
  }

  void set_min(seq_val_t v) {
    Scoped_pthread_mutex spm(mutex);
    min = v;
  }

  void set_cycle(bool v) {
    Scoped_pthread_mutex spm(mutex);
    cycle = v;
  }

  void set_n_cache(uint n_cache_) {
    Scoped_pthread_mutex spm(mutex);
    n_cache = n_cache_;
  }

  std::string get_name() const {
    Scoped_pthread_mutex spm(mutex);
    return name;
  }

  void set_name(const std::string &name_) {
    Scoped_pthread_mutex spm(mutex);
    name = name_;
  }

  std::string get_db() const {
    Scoped_pthread_mutex spm(mutex);
    return db;
  }

  void set_db(const std::string &db_) {
    Scoped_pthread_mutex spm(mutex);
    db = db_;
  }

  /**
    Return next sequence value.
    @param res [out] takes out next sequence value.
    @retval true on error, false ok. res is intace on error return.
  */
  bool get_next_val(seq_val_t &res) {
    Scoped_pthread_mutex spm(mutex);
    if (unlikely(next_val != InvalidSeqValue)) {
      cur_val= next_val;
      next_val= InvalidSeqValue;
      res= cur_val;
      return false;
    }
    if ((step > 0 && max_cached == InvalidSeqValue) ||
        (step < 0 && min_cached == InvalidSeqValue) ||
        (step > 0 && cur_val + step > max_cached) ||
        (step < 0 && cur_val + step < min_cached)) {
      if (reserve_range()) {
        return true;
      }
    } else {
      cur_val += step;
    }

    res = cur_val;
    return false;
  }

  int alter_args(THD *thd, int step_, Sequence::seq_val_t max_,
                 Sequence::seq_val_t min_, bool cycle_, uint ncache);
  void inc_ref() {
    Scoped_pthread_mutex spm(mutex);
    refcnt++;
  }

  void dec_ref() {
    Scoped_pthread_mutex spm(mutex);
    refcnt--;
  }

  int ref_cnt() const {
    Scoped_pthread_mutex spm(mutex);
    return refcnt;
  }

  bool set_next_val(seq_val_t val, bool next, seq_val_t &out) {
    Scoped_pthread_mutex spm(mutex);

    // if not allowed cycled the value set must be valid
    if (!cycle &&
        ((step > 0 && val < cur_val) || (step < 0 && val > cur_val))) {
      out = 0;
      return true;
    }

    if (next) {
      next_val = val + step;
    } else {
      next_val = val;
    }

    if (next_val > max || next_val < min) {
      out = val; // out of range, check before nextval
      next_val = InvalidSeqValue;
      return true;
    }

    // store it to table
    if (set_next_val()) {
      out = val;
      next_val = InvalidSeqValue;
      return true;
    }

    out = val;
    return false;
  }
};

extern Partitioned_rwlock seq_cache_lock;
extern uint64_t seq_cache_version;

Sequence *get_sequence(THD *thd, const std::string&db, const std::string&name,
                       bool acquire_lock);
int create_sequence(THD *thd, const std::string&db, const std::string&name,
                    int step_, Sequence::seq_val_t start_, Sequence::seq_val_t max_,
                    Sequence::seq_val_t min_, bool cycle_, uint ncache, bool if_not_exists);
int drop_sequence(THD *thd, const std::string&db, const std::string&name, bool if_exists);
void close_sequence(Sequence *seq);
/* Clear all cached sequences and increase version */
void clear_sequence_cache(bool double_check);
int lock_db_sequences(THD *thd, const char *db,bool skipTableNotExists = false);
int drop_db_sequences(const THD *thd, const char *db, bool do_binlogging= false);
uint alter_seq_thds(uint from, uint to, int *err);
const uint MAX_SEQ_WORKER_THDS= 256;
void inc_seq_version(void);
bool seq_need_reload();

/* For recyle bin */
void db_object_recycle(THD *thd, const char *dbname, uint64_t schema_id);
void db_object_clear(THD *thd, time_t before_time);
void db_object_restore(THD *thd, const char *dbname, uint64_t schema_id);
#endif //!SQL_SEQ_H

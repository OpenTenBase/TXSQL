/*
  Created in August 2018 by daviezhao
  2021, Port & Modified by Lucaszhai
  Oracle server style sequence implementation.
*/
#include "sql_seq.h"
#include "sql_class.h"
#include "my_thread.h"
#include "log.h"
#include "sql_parse.h"
#include "sql_base.h"
#include "m_ctype.h"
#include "sql_thd_internal_api.h"
#include "rpl_table_access.h"
#include "mdl.h"
#include "sql_lex.h"
#include "table.h"
#include "field.h"
#include "mysqld.h"
#include "derror.h"
#include "sp_cache.h"
#include <deque>
#include <atomic>
#include <boost/multiprecision/cpp_int.hpp>


extern uint num_seq_threads;
class Seq_work_param;
class Seq_task;

static void *sql_work(void *param);
static int load_db_seqs(const std::string &db, bool doing_insert= false,bool skipTableNotExists = false);
static void report_seq_errors(Seq_task &t);
static int update_sequence_def(const Sequence *seq);

int64_t sql_worker_qlen = 0;

typedef std::map<std::string, Sequence*> Seq_name_map;
typedef std::map<std::string, Seq_name_map> Seq_db_map;

Partitioned_rwlock seq_cache_lock;
uint64_t seq_cache_version = 1;
std::atomic<uint64_t> seq_version{1};

void inc_seq_version() {
  seq_version++;
}

bool seq_need_reload() {
  return (seq_version.load() != seq_cache_version);
}

class Seq_cache {
  Seq_db_map all_seqs;
  pthread_mutex_t mutex;
public:
  Seq_cache() {
    pthread_mutex_init(&mutex, NULL);
  }

  void clear() {
    /* Free all sequences */
    for (auto& elem : all_seqs) {
      for (auto& seq_elem : elem.second) {
        delete seq_elem.second;
      }

      elem.second.clear();
    }

    all_seqs.clear();
  }

  ~Seq_cache() {
    pthread_mutex_destroy(&mutex);
  }

  Sequence *find(const std::string&db, const std::string&name);
  int drop(const std::string&db, const std::string&name, Sequence *&out);
  int insert(const std::string&db, const std::string&name, Sequence*seq);
  int load_db_seqs(THD *thd, const std::string &dbname);
  int lock_db_sequences(THD *thd, const char *db, bool skipTableNotExists = false);
  int drop_db_sequences(const char *db);
  bool db_has_sequence(const char *db);
  pthread_mutex_t &get_mutex() { return mutex; }
};

Seq_cache g_seq_cache;

class Seq_taskq;
struct Seq_task {
  enum Type {
    CREATE_SEQUENCE_TABLE,
    LOAD_SEQUENCE,
    CREATE_SEQUENCE,
    ALTER_SEQUENCE,
    DROP_SEQUENCE,
    FETCH_SEQUENCE,
    DROP_DB_SEQ,
    RECYCLE_SEQUENCE,
    CLEAR_SEQUENCE,
    RESTORE_SEQUENCE,
    RECYCLE_ROUTINE,
    CLEAR_ROUTINE,
    RESTORE_ROUTINE
  };
  std::string db, name;
  Type type;
  int error;
  std::string errstr;
  bool done;
  bool m_do_binlogging;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  my_thread_id m_thid;
  pid_t m_tid;
  enum enum_thread_type thread_type;
  bool is_slave_thread;

  void set_thread_type(const THD *thd) {
    /*
    this is made a no-op because a slave worker thread does something extra
    after executing a command, such as calling slave_execute_deferred_events().
    I've modified check_readonly() to allow bg thread to always execute any sql
    stmt, and bg threads that should not execute sql stmts(like slow deleter)
    should avoid calling mysql_parse() at all, like how slow deleter does.

    thread_type= thd->system_thread;
    is_slave_thread= thd->slave_thread;
    */
  }

  static const char *type_str(Type t) {
    // must be consistent with Seq_task::Type definition.
    static char *type_strs[] = {
        (char *)"CREATE_SEQUENCE_TABLE", (char *)"LOAD_SEQUENCE",
        (char *)"CREATE_SEQUENCE",       (char *)"ALTER_SEQUENCE",
        (char *)"DROP_SEQUENCE",         (char *)"FETCH_SEQUENCE",
        (char *)"DROP_DB_SEQ",           (char *)"RECYCLE_SEQUENCE",
        (char *)"CLEAR_SEQUENCE",        (char *)"RESTORE_SEQUENCE",
        (char *)"RECYCLE_ROUTINE",       (char *)"CLEAR_ROUTINE",
        (char *)"RESTORE_ROUTINE"};
    return type_strs[t];
  }

  Seq_task(const std::string &db_, const std::string &name_, Type t) : db(db_), name(name_),
      type(t) {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    done = false;
    error = 0;
    m_thid = 0;
    m_tid = 0;
    m_do_binlogging = true;
    thread_type = NON_SYSTEM_THREAD;
    is_slave_thread = false;
  }

  ~Seq_task() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
  }

  void mark_done(int err= 0, const char *errmsg= NULL) {
    Sequence::Scoped_pthread_mutex spm(mutex);
    done = true;
    error = err;
    if (errmsg) {
      errstr= errmsg;
    }
    pthread_cond_signal(&cond);
  }

  void wait() {
    Sequence::Scoped_pthread_mutex spm(mutex);
    if (!done) {
      pthread_cond_wait(&cond, &mutex);
    }
  }

  int get_error() const { return error; }
  void clear_error() {
    Sequence::Scoped_pthread_mutex spm(mutex);
    error = 0;
    errstr = "";
  }
  void set_thread_id(my_thread_id thid, pid_t tid) {
    m_thid = thid;
    m_tid = tid;
  }

  bool do_binlogging() const { return m_do_binlogging; }
  void do_binlogging(bool b) { m_do_binlogging= b; }
};

struct Seq_create_task : public Seq_task {
  Seq_create_task(const Sequence *s) :
    Seq_task(s->get_db(), s->get_name(), CREATE_SEQUENCE), seq(s) {}
  const Sequence *seq;// don't free this object, it's not owned by this task.
};

struct Seq_alter_task : public Seq_task {
  Seq_alter_task(const Sequence *s) :
    Seq_task(s->get_db(), s->get_name(), ALTER_SEQUENCE), seq(s) {}
  const Sequence *seq;// don't free this object, it's not owned by this task.
};

struct Seq_fetch_task : public Seq_task {
  Seq_fetch_task(const std::string &db_, const std::string &name_, Sequence::seq_val_t val) :
    Seq_task(db_, name_, FETCH_SEQUENCE), curval(val) {}
  Sequence::seq_val_t curval;
};

struct Object_recycle_task : public Seq_task {
  Object_recycle_task(const std::string &db_, uint64_t id, Type t,
                      bool do_binloging)
      : Seq_task(db_, std::string(""), t), schema_id(id) {
    do_binlogging(do_binloging);
  }

  /* Used to identify the recycled sequence */
  uint64_t schema_id;
};

struct Object_clear_task : public Seq_task {
  Object_clear_task(time_t purge_time, Type t, bool do_binloging) :
    Seq_task(std::string(""), std::string(""), t), before_time(purge_time) {
      do_binlogging(do_binloging);
    }

  time_t before_time;
};

struct Object_restore_task : public Seq_task {
  Object_restore_task(const std::string db_, uint64_t id, Type t,
                      bool do_binloging)
      : Seq_task(db_, std::string(""), t), schema_id(id) {
      do_binlogging(do_binloging);
  }

  uint64_t schema_id;
};

class Seq_taskq {
  std::deque<Seq_task *>taskq;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int n_tasks;
public:
  Seq_taskq() {
    n_tasks= 0;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
  }

  ~Seq_taskq() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
  }

  /*
    This task queue object doesn't own appended task objects,
    caller should create and destroy them.
  */
  void append_task(Seq_task *t) {
    Sequence::Scoped_pthread_mutex spm(mutex);

    assert(!t->done);
    taskq.push_back(t);
    n_tasks++;
    assert((uint)n_tasks == taskq.size());
    sql_worker_qlen = n_tasks;
    pthread_cond_signal(&cond);
  }

  Seq_task *fetch_task(int64_t &qlen, bool *exit_flag) {
    struct timespec deadline;
    Sequence::Scoped_pthread_mutex spm(mutex);
    while (taskq.empty()) // More than 1 threads can be signaled,so we must recheck empty().
    {
      clock_gettime(CLOCK_REALTIME, &deadline);
      deadline.tv_sec += 3;

      /*
        server exiting.
      */
      if (*exit_flag)
        return nullptr;

      pthread_cond_timedwait(&cond, &mutex, &deadline);
    }
    Seq_task *t = taskq.front();
    taskq.pop_front();
    n_tasks--;
    assert((uint)n_tasks == taskq.size());
    qlen = n_tasks;
    return t;
  }
};

Seq_taskq seq_tasks;
struct Seq_work_param {
  bool exit, running;
  int pc_ret;// pthread_create return value;
  pthread_t thd_handle;

  Seq_work_param()
    : exit(false), running(false), pc_ret(0xffffffff) {}
};

static Seq_work_param seq_work_params[MAX_SEQ_WORKER_THDS];

/*
  alter the number of sequence worker threads, from 'from' to 'to' number of
  sequence worker threads. create to-from new threads or terminate from-to threads.
  @param from/to alter number of seq threads from 'from' to 'to'.
  @param err[out] bring back pthread_create error number.
  @retval 0 on sucess, (0,256) the slot index within seq_work_params array that
  pthread_create failed. 0xffffffff on invalid parameter error.
*/
uint alter_seq_thds(uint from, uint to, int *err) {
  if (from > MAX_SEQ_WORKER_THDS || to > MAX_SEQ_WORKER_THDS || !err) {
    return (uint)-1;
  }

  if (from == to) {
    return 0;
  }

  if (from > to) {
    for (uint i = to; i < from; i++) {
      Seq_work_param *pp = seq_work_params+i;
      pp->exit = true;
      pp->running = false;
    }

    for (uint i= to; i < from; i++) {
      Seq_work_param *pp = seq_work_params+i;
      pthread_join(pp->thd_handle, NULL);
    }
  }
  else {
    for (uint i= from; i < to; i++) {
      Seq_work_param*pp = seq_work_params+i;
      pp->exit = false;
      if ((pp->pc_ret = pthread_create(&(pp->thd_handle), NULL, sql_work, pp))) {
        *err= pp->pc_ret;
        return i;
      }
      pp->running= true;
    }
  }

  return 0;
}

static void *sql_work(void*param0) {
  char sql_buf[FN_REFLEN*2];
  Seq_work_param *param = (Seq_work_param *)param0;

  my_thread_init();
  THD *thd = create_thd(false, true, false, 0, 0);
  thd->set_new_thread_id(); // avoid assert check fail under debug mode
  thd->variables.option_bits |= OPTION_AUTOCOMMIT;//we must use autocommit in this thd
  thd->variables.option_bits &= ~OPTION_NOT_AUTOCOMMIT;
  thd->variables.option_bits &= ~OPTION_BEGIN;
  thd->server_status |= SERVER_STATUS_AUTOCOMMIT;
  thd->variables.binlog_format = BINLOG_FORMAT_ROW;

  while (!param->exit) {
    Parser_state parser_state;
    uint sql_len;
    ulonglong tmp_disable_binlog__save_options= 0;
    Seq_task *task = seq_tasks.fetch_task(sql_worker_qlen, &param->exit);
    if (!task) {
      break;
    }

    switch (task->type) {
      case Seq_task::FETCH_SEQUENCE: {
        Seq_fetch_task *ftask = (Seq_fetch_task *)task;
        snprintf(sql_buf, sizeof(sql_buf),
                 "UPDATE mysql.tdsql_sequences set curval= %ld where db='%s' and name='%s'",
                  ftask->curval, ftask->db.c_str(), ftask->name.c_str());
        break;
      }
      case Seq_task::CREATE_SEQUENCE: {
        Seq_create_task *ctask = (Seq_create_task *)task;
        snprintf(sql_buf, sizeof(sql_buf),
                 "INSERT INTO mysql.tdsql_sequences values('%s', '%s', %ld, %ld,%d,%ld,%ld,%d,%u)",
                 ctask->db.c_str(), ctask->name.c_str(), Sequence::InvalidSeqValue,
                 ctask->seq->get_start(), ctask->seq->get_step(),
                 ctask->seq->get_max(), ctask->seq->get_min(),
                 ctask->seq->get_cycle(), ctask->seq->get_n_cache());
        break;
      }
      case Seq_task::CREATE_SEQUENCE_TABLE: {
        snprintf(sql_buf, sizeof(sql_buf),
                 "CREATE TABLE mysql.tdsql_sequences("
                 "db varchar(128) not null,"
                 "name varchar(128) not null,"
                 "curval bigint not null, start bigint not null,"
                "step int not null,"
                 "max_value bigint not null,"
                 "min_value bigint not null,"
                 "do_cycle bool not null, n_cache int unsigned not null,"
                 "primary key(db,name)) engine=innodb, charset=utf8");
        break;
      }
      case Seq_task::DROP_SEQUENCE: {
        snprintf(sql_buf, sizeof(sql_buf),
                 "DELETE FROM mysql.tdsql_sequences where db='%s' and name='%s'",
                 task->db.c_str(), task->name.c_str());
        break;
      }
      case Seq_task::ALTER_SEQUENCE: {
        Seq_alter_task *atask = (Seq_alter_task *)task;
        snprintf(sql_buf, sizeof(sql_buf),
                 "UPDATE mysql.tdsql_sequences set step= %d, max_value=%ld,min_value=%ld,"
                 "do_cycle=%d, n_cache=%u where db='%s' and name='%s'",
                 atask->seq->get_step(), atask->seq->get_max(), atask->seq->get_min(),
                 atask->seq->get_cycle(), atask->seq->get_n_cache(),
                 atask->seq->get_db().c_str(), atask->seq->get_name().c_str());
        break;
      }
      case Seq_task::LOAD_SEQUENCE: {
        g_seq_cache.load_db_seqs(thd, task->db);
        snprintf(sql_buf, sizeof(sql_buf),
                "The equivalence of 'SELECT * FROM mysql.tdsql_sequences where db='%s' and name='%s''",
                task->db.c_str(), task->name.c_str());
        break;
      }
      case Seq_task::DROP_DB_SEQ: {
        snprintf(sql_buf, sizeof(sql_buf),
                 "Delete from mysql.tdsql_sequences where db='%s'",
                 task->db.c_str());
        break;
      }
      case Seq_task::RECYCLE_SEQUENCE: {
        Object_recycle_task *rtask = (Object_recycle_task *)task;
        snprintf(
            sql_buf, sizeof(sql_buf),
            "UPDATE mysql.tdsql_sequences set db = concat(unix_timestamp(), "
            "'_', %lu, '__recycle_bin__', '%s') where db = '%s'",
            rtask->schema_id, rtask->db.c_str(), rtask->db.c_str());
        break;
      }
      case Seq_task::CLEAR_SEQUENCE: {
        Object_clear_task *ctask = (Object_clear_task *)task;
        snprintf(sql_buf, sizeof(sql_buf),
                 "delete from mysql.tdsql_sequences where db like "
                 "'%%__recycle_bin__%%' and substring_index(db, '_', 1) <= %ld",
                 ctask->before_time);
        break;
      }
      case Seq_task::RESTORE_SEQUENCE: {
        Object_restore_task *rtask = (Object_restore_task *)task;
        snprintf(sql_buf, sizeof(sql_buf),
                 "update mysql.tdsql_sequences set db = '%s' where db like "
                 "'%%__recycle_bin__%s'",
                 rtask->db.c_str(), rtask->db.c_str());
        break;
      }
      case Seq_task::RECYCLE_ROUTINE: {
        Object_recycle_task *rtask = (Object_recycle_task *)task;
        snprintf(sql_buf, sizeof(sql_buf),
                 "update mysql.routines set name = concat(unix_timestamp(), "
                 "'_', '%s', '__recycle_bin__', name) where schema_id = %lu",
                 rtask->db.c_str(), rtask->schema_id);
        break;
      }
      case Seq_task::CLEAR_ROUTINE: {
        Object_clear_task *ctask = (Object_clear_task *)task;
        snprintf(
            sql_buf, sizeof(sql_buf),
            "delete from mysql.routines where name like '%%__recycle_bin__%%' "
            "and substring_index(name, '_', 1) <= %ld",
            ctask->before_time);
        break;
      }
      case Seq_task::RESTORE_ROUTINE: {
        Object_restore_task *rtask = (Object_restore_task *)task;
        snprintf(sql_buf, sizeof(sql_buf),
                 "update mysql.routines set name = substring_index(name, "
                 "'__recycle_bin__', -1), schema_id = %lu where name like "
                 "'%%%s__recycle_bin__%%'",
                 rtask->schema_id, rtask->db.c_str());
        break;
      }
      default:
        assert(0);
        break;
    }

    if (task->type == Seq_task::LOAD_SEQUENCE) {
      goto task_done;
    }

    sql_len = strlen(sql_buf);
    thd->set_query(sql_buf, sql_len);
    thd->set_query_id(next_query_id());
    task->set_thread_id(thd->thread_id(), /*fixme*/0);
    thd->system_thread = task->thread_type;
    thd->slave_thread = task->is_slave_thread;

    if (!parser_state.init(thd, sql_buf, sql_len)) {
      assert(thd->m_digest == nullptr);
      thd->m_digest = &thd->m_digest_state;
      assert(thd->m_statement_psi == nullptr);
      thd->m_statement_psi = MYSQL_START_STATEMENT(
          &thd->m_statement_state, stmt_info_rpl.m_key, thd->db().str,
          thd->db().length, thd->charset(), nullptr);
      if (!task->do_binlogging()) {
        tmp_disable_binlog__save_options= thd->variables.option_bits;
        thd->variables.option_bits&= ~OPTION_BIN_LOG;
      }

      dispatch_sql_command(thd, &parser_state);
    //  thd->update_server_status();
      thd->send_statement_status();
      // If we disabled binlogging, reenable it here.
      if (!task->do_binlogging()) {
        thd->variables.option_bits= tmp_disable_binlog__save_options;
      }

task_done:
      int got_err = thd->is_error() ? thd->get_stmt_da()->mysql_errno() : 0;
      if (got_err) {
        /*
          Do not error out, keep working, simply write errors to error log, and
          let user session thread to do ordinary error reporting.
        */
        if(ER_NO_SUCH_TABLE == got_err) { //if table don't exist,just print normal information
        } else {
          sql_print_error("Sequence worker: when executing %s got error(%d): %s.",
                           sql_buf, thd->get_stmt_da()->mysql_errno(),
                           thd->get_stmt_da()->message_text());
        }

        if (got_err == ER_KEY_NOT_FOUND) {
          task->mark_done(ER_SEQUENCE_NOT_FOUND);
        } else if (got_err == ER_DUP_KEY) {
          task->mark_done(ER_SEQUENCE_EXISTS);
        } else {
          task->mark_done(thd->get_stmt_da()->mysql_errno(),thd->get_stmt_da()->message_text());
        }
      }
      else {
        task->mark_done();
      }
    }

    thd->get_stmt_da()->reset_diagnostics_area();
    thd->get_stmt_da()->reset_condition_info(thd);

    thd->set_catalog(NULL_CSTR);
    thd->set_db(NULL_CSTR);                 /* will free the current database */
    thd->reset_query();
    thd->lex->sql_command = SQLCOM_END;

    /* Mark the statement completed. */
    MYSQL_END_STATEMENT(thd->m_statement_psi, thd->get_stmt_da());
    thd->m_statement_psi = nullptr;
    thd->m_digest = nullptr;

    // like finish_command reset the mem_root at end
    if (thd->mem_root->allocated_size() < 5 * thd->variables.query_prealloc_size)
      thd->mem_root->ClearForReuse();
    else
      thd->mem_root->Clear();
  }

  thd->system_thread = SYSTEM_THREAD_BACKGROUND;// restore the value for destroy_thd() to work correctly.
  destroy_thd(thd);
  my_thread_end();
  return 0;
}

// if sequence not found, caller should report
// my_error(ER_SEQUENCE_NOT_FOUND, MYF(0), db.c_str(), name.c_str());
Sequence *Seq_cache::find(const std::string&db, const std::string&name) {
  Sequence::Scoped_pthread_mutex spm(mutex);
  Seq_db_map::iterator i= all_seqs.find(db);

  if (i == all_seqs.end() && ::load_db_seqs(db)) {
    return nullptr;
  }

  i = all_seqs.find(db);
  Seq_name_map::iterator j;

  if (i == all_seqs.end() || (j = i->second.find(name)) == i->second.end()) {
    return nullptr;
  }

  return j->second;
}

/*
  delete target(db, name) Sequence object from g_all_seqs, return the found
  obj's ptr, or 0 if not found, or the sequence is in use.a
  @param[out] out takes back found sequence. not modified if sequence not found.
  @retval -1: sequence not found;
          0: success, dropped from the global cache;
          1: sequence in use
          2: other errors.
*/
int Seq_cache::drop(const std::string&db, const std::string&name, Sequence *&out) {
  Sequence::Scoped_pthread_mutex spm(mutex);
  Seq_db_map::iterator i = all_seqs.find(db);
  Seq_name_map::iterator j;

  if (i == all_seqs.end() && ::load_db_seqs(db)) {
    return 2;
  }

  i = all_seqs.find(db);

  if (i == all_seqs.end() || (j= i->second.find(name)) == i->second.end()) {
    return -1;
  }

  Sequence *seq = j->second;
  if (seq->ref_cnt() <= 0) {
    i->second.erase(j);
    out = seq;
    return 0;
  }
  else {
    my_error(ER_SEQUENCE_IN_USE, MYF(0), db.c_str(), name.c_str());
    return 1;
  }
}

/*
  Returns 0 on success, 1 if seq already exists.
*/
int Seq_cache::insert(const std::string&db, const std::string&name, Sequence*seq) {
  Sequence::Scoped_pthread_mutex spm(mutex);
  Seq_db_map::iterator i = all_seqs.find(db);

  if (i == all_seqs.end() && ER_NO_SUCH_TABLE == ::load_db_seqs(db, true)) {
    Seq_name_map name_map;
    /*
       The mysql.tdsql_sequences table doesn't exist, create it. This only
       happens if the db instance was created by an earlier version of mysqld.
       And we only do so in this function, not in drop/find() member functions
       because it makes sense that if a sequence doesn't even exist, it can't be
       used. Only when user wants to create a sequence we need to create this
       table.
    */
    name_map.insert(std::make_pair(name, seq));
    all_seqs.insert(std::make_pair(db, name_map));
    Seq_task t(db, name, Seq_task::CREATE_SEQUENCE_TABLE);
    seq_tasks.append_task(&t);
    t.wait();
    // when we return, our task has been executed.
    if (t.get_error()) {
      report_seq_errors(t);
      return 1;
    }
  }
  else {
    i = all_seqs.find(db);
    assert(i != all_seqs.end()); // *i created in Seq_cache::load_db_seqs().

    Seq_name_map::iterator j;
    j = i->second.find(name);
    if (j != i->second.end()) {
      return 1;
    } else {
      i->second.insert(std::make_pair(name, seq));
    }
  }
  return 0;
}

/*
  reserve n_cache seq values, the values are cached in memory. store the max
  cached/min cached value into table. and assign cur_val to the 1st unallocated
  newly cached seq value.

  seq range may be consumed, and if cycling, recycle, otherwise fail.

  @retval true on error, false success.
*/
bool Sequence::reserve_range() {
  bool ends = false;
  uint ncache = n_cache;

  if (step > 0 && max_cached != InvalidSeqValue && max_cached + step > max) {
    if (cycle) {
      cur_val = min;
      /*
        Here cur_val is NOT used yet, normally cur_val is the current allocated
        value, thus decrement.
      */
      ncache--;
    } else {
      ends = true;
    }
  }
  else if (step < 0 && min_cached != InvalidSeqValue && min_cached + step < min) {
    if (cycle) {
      cur_val = max;
      // same as above.
      ncache--;
    } else {
      ends = true;
    }
  }

  if (ends) {
    my_error(ER_NON_CYCLIC_SEQUENCE_CONSUMED, MYF(0),
             name.c_str(), step, db.c_str(), step > 0 ? max : min);
    return true;
  }

  // reversing increment.
  if ((step > 0 && max_cached == InvalidSeqValue) ||
      (step < 0 && min_cached == InvalidSeqValue)) {
    // Fetching 1st value from a brand new sequence, and continue with existing
    // one.
    if (cur_val == InvalidSeqValue) {
      cur_val = start;
      ncache--;
    }
  }

  seq_val_t val= cur_val + step*((int)ncache);// without the conversion to int,
  // a negative step causes the product to overflow.

  if (step > 0) {
    if (val > max) // this could never happen to a newly recycled case.
      max_cached = val= cur_val + ((max - cur_val) / step) * step;
    else
      max_cached = val;
  }

  if (step < 0) {
    if (val < min) // this could never happen to a newly recycled case.
      min_cached = val= cur_val + ((min - cur_val) / step) * step;
    else
      min_cached = val;
  }

  if (ncache == n_cache) // cur_val not allocated yet.
    cur_val += step;

  Seq_fetch_task t(db, name, step > 0 ? max_cached : min_cached);
  seq_tasks.append_task(&t);
  t.wait();
  // when we return, our task has been executed.
  if (t.get_error()) {
    report_seq_errors(t);
    return true;
  }

  return false;
}

bool Sequence::set_next_val()
{
  Seq_fetch_task t(db, name, next_val);
  seq_tasks.append_task(&t);
  t.wait();
  // when we return, our task has been executed.
  if (t.get_error()) {
    report_seq_errors(t);
    return true;
  }

  return false;
}

class Seq_table_access : public System_table_access {
public:
  Open_tables_backup m_backup;
  Seq_table_access()
  {}

  virtual void before_open(THD* thd) override
  {
    m_flags = (MYSQL_OPEN_IGNORE_GLOBAL_READ_LOCK |
              MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY |
              MYSQL_OPEN_IGNORE_FLUSH |
              MYSQL_LOCK_IGNORE_TIMEOUT);
  }
};

static int load_db_seqs(const std::string &db, bool doing_insert, bool skipTableNotExists) {

  /* don't load sequence for recycle_bin */
  if (my_strcasecmp(system_charset_info,
                    db.c_str(), RECYCLE_BIN_SCHEMA_NAME.str) == 0) {
    return 0;
  }

  Seq_task t(db, "", Seq_task::LOAD_SEQUENCE);
  seq_tasks.append_task(&t);
  t.wait();
  // when we return, our task has been executed.
  // we don't report in this case because we will immediately do a create table.
  if(t.get_error() == ER_NO_SUCH_TABLE && skipTableNotExists) {
    t.clear_error();
  }

  if (t.get_error() == ER_NO_SUCH_TABLE && !doing_insert)
    report_seq_errors(t);
  return t.get_error();
}

int Seq_cache::load_db_seqs(THD *thd, const std::string &dbname) {
  Seq_table_access sta;
//  LEX_CSTRING db= {(char*)"mysql", 5};
//  LEX_CSTRING tbl= {(char*)"tdsql_sequences", 15};
  LEX_CSTRING db = {STRING_WITH_LEN("mysql")};
  LEX_CSTRING tbl = {STRING_WITH_LEN("tdsql_sequences")};

  int err = 0, ret = 0;
  TABLE *table= 0;

  if ((err = sta.open_table(thd, db, tbl, 7, TL_READ_HIGH_PRIORITY, &table, &sta.m_backup)))
    return err;

  /*
    It is safe here to write/read all_seqs without holding g_seq_cache.mutex,
    because the paths this function is called already locks that mutex (in the
    caller's thread and waits until the task finishes here),
    so there can be no other threads accessing all_seqs.
  */
  Seq_name_map name_map;
  all_seqs.insert(std::make_pair(dbname, name_map));
  Seq_db_map::iterator itr_db = all_seqs.find(dbname);

  Field **fields= table->field;
  uchar user_key[MAX_KEY_LENGTH];

  empty_record(table);

  fields[0]->set_notnull();
  if ((err = fields[0]->store(dbname.c_str(), dbname.length(),
                             &my_charset_utf8_unicode_ci)))
  {
    // Couldn't reach here.
    table->file->print_error(err, MYF(0));
    goto end;
  }

  key_copy(user_key, table->record[0], table->key_info,
           table->key_info->key_length);

  if ((err = table->file->ha_index_init(0, 1))) {
    table->file->print_error(err, MYF(0));
    goto end;
  }

  if ((err = table->file->ha_index_read_map(table->record[0], user_key,
                                             1,
                                             HA_READ_KEY_OR_NEXT))) {
    DBUG_PRINT ("info", ("Row not found"));
    goto end;
  }

  while(!err) {
    String str0;

    table->field[0]->val_str(&str0);
    std::string dbname0(str0.ptr(), str0.length());
    if (dbname0 != dbname)
      break;

    String str;

    table->field[1]->val_str(&str);
    std::string name(str.ptr(), str.length());

    longlong start, maxval, minval;
    int step;

    start = table->field[3]->val_int();
    step = table->field[4]->val_int();
    maxval = table->field[5]->val_int();
    minval = table->field[6]->val_int();

    bool is_cycle = table->field[7]->val_int();
    uint ncache = (uint)table->field[8]->val_int();

    Sequence *seq = new Sequence(dbname, name, step, start, is_cycle, ncache, maxval, minval);
    if (step > 0) {
      seq->set_max_cached(table->field[2]->val_int());
    } else {
      seq->set_min_cached(table->field[2]->val_int());
    }

    itr_db->second.insert(std::make_pair(name, seq));
    err = table->file->ha_index_next(table->record[0]);

    // Report error via my_error().
    if (err && err != HA_ERR_END_OF_FILE) {
      table->file->print_error(err, MYF(0));
      break;
    }
  }

end:
  table->file->ha_index_end();
  if (err && err != HA_ERR_END_OF_FILE)
    ret = -1;
  if (table)
    sta.close_table(thd, table, &sta.m_backup, (ret != 0), true);
  return ret;
}

static int validate_seq_args(int &step_, Sequence::seq_val_t &start_, Sequence::seq_val_t &max_,
                  Sequence::seq_val_t &min_, bool cycle_, uint ncache) {
  if (step_ == 0) {
    my_error(ER_WRONG_SEQ_ARGS, MYF(0), "step", "step != 0");
    return 1;
  }

  if (step_ > 0) {
    if (max_ == Sequence::InvalidSeqValue)
      max_ = 0x7fffffffffffffff; // 2^63-1. In Oracle this is 10^27
    if (min_ == Sequence::InvalidSeqValue)
      min_ = 1;
    if (start_ == Sequence::InvalidSeqValue)
      start_ = min_;
  }

  if (step_ < 0) {
    if (max_ == Sequence::InvalidSeqValue)
      max_ = -1;
    if (min_ == Sequence::InvalidSeqValue)
      min_ = 0x8000000000000000; // -2^63. In Oracle this is -(10^26)
    if (start_ == Sequence::InvalidSeqValue)
      start_ = max_;
  }

  if (min_ >= max_) {
    my_error(ER_WRONG_SEQ_ARGS, MYF(0), "maxvalue and minvalue", "minvalue < maxvalue must be true.");
    return 1;
  }

  namespace mp = boost::multiprecision;
  mp::uint128_t dist = max_ - min_; // the right value can overflow, thus using 128bit integers.
  if (mp::uint128_t(abs(step_)) >= dist) {
    my_error(ER_WRONG_SEQ_ARGS, MYF(0), "step", "step < maxvalue-minvalue must be true.");
    return 1;
  }
  if (start_ < min_ || start_ > max_) {
    my_error(ER_WRONG_SEQ_ARGS, MYF(0), "start", "start must be in [minvalue, maxvalue]");
    return 1;
  }
  if (mp::uint128_t(ncache) >= mp::uint128_t((dist/mp::uint128_t(abs(step_))))) {
    my_error(ER_WRONG_SEQ_ARGS, MYF(0), "cache", "cache < (maxvalue - minvalue)/abs(step)");
    return 1;
  }

  return 0;
}


int create_sequence(THD *thd, const std::string&db, const std::string&name,
                    int step_, Sequence::seq_val_t start_, Sequence::seq_val_t max_,
                    Sequence::seq_val_t min_, bool cycle_, uint ncache, bool if_not_exists) {
  if (validate_seq_args(step_, start_, max_, min_, cycle_, ncache))
    return 1;
  std::string seq_name = name;
  if (1 == lower_case_table_names) {
    std::transform(seq_name.begin(), seq_name.end(), seq_name.begin(), ::tolower);
  }
  Sequence *pseq = new Sequence(db, seq_name, step_, start_, cycle_, ncache, max_, min_);

  if (lock_object_name(thd, MDL_key::SEQUENCE, db.c_str(), seq_name.c_str())) {
    delete pseq;
    return 1;
  }

  DBUG_EXECUTE_IF("create_sequence_pause_after_lock", sleep(20););

  if (g_seq_cache.insert(db, seq_name, pseq)) {
    delete pseq;
    if (if_not_exists) {
      push_warning_printf(thd, Sql_condition::SL_NOTE,
                          ER_SEQUENCE_EXISTS, ER_THD(thd, ER_SEQUENCE_EXISTS),
                          seq_name.c_str(), db.c_str());
      return 0;
    } else {
      my_error(ER_SEQUENCE_EXISTS, MYF(0), seq_name.c_str(), db.c_str());
      return 1;
    }
  }

  Seq_create_task t(pseq); // pseq is inserted into g_seq_cache.
  t.set_thread_type(thd);
  seq_tasks.append_task(&t);
  t.wait();
  // when we return, our task has been executed.
  if (t.get_error()) {
    /*
      If error is ER_SEQUENCE_EXISTS, it means someone inserted an sequence row
      directly into the definition table, which is not allowed, and we don't
      know whether that definition is the same as current one, so we should
      require mysqld to restart to read the definition.
    */
    if (t.get_error() == ER_SEQUENCE_EXISTS)
      my_error(ER_SEQUENCE_EXISTS_ONLY_IN_TABLE, MYF(0), seq_name.c_str(), db.c_str());
    else
      report_seq_errors(t);
    return 1;
  }

  return 0;
}

int Sequence::alter_args(THD *thd, int step_, Sequence::seq_val_t max_,
               Sequence::seq_val_t min_, bool cycle_, uint ncache) {
  Sequence::seq_val_t start0 = start;
  // if nothing changes, simply return.
  if (!(step_ != step || max_ != max || min_ != min || cycle_ != cycle || ncache != n_cache))
    return 0;

  int ret = 1;

  if (lock_object_name(thd, MDL_key::SEQUENCE, db.c_str(), name.c_str()))
    goto err;

  DBUG_EXECUTE_IF("alter_sequence_pause_after_lock", sleep(20););
  if (validate_seq_args(step_, start0, max_, min_, cycle_, ncache))
    goto err;

  if (start0 != start) {
    my_error(ER_WRONG_SEQ_ARGS, MYF(0), "start", "start can't be altered in 'alter sequence statement.");
    goto err;
  }

  if (cur_val != InvalidSeqValue && (max_ < cur_val || min_ > cur_val)) {
    my_error(ER_WRONG_SEQ_ARGS, MYF(0), "max/min", "current sequence value must be in [min,max]");
    goto err;
  }

  //min_cached= max_cached = InvalidSeqValue; keep growing, don't start over.

  step = step_;
  max = max_;
  min = min_;
  cycle = cycle_;
  n_cache = ncache;

  ret= update_sequence_def(this);
err:
  return ret;
}

// for alter sequence stmt.
static int update_sequence_def(const Sequence *seq) {
  Seq_alter_task t(seq);// seq is in g_seq_cache.
  seq_tasks.append_task(&t);
  t.wait();
  // when we return, our task has been executed.
  if (t.get_error()) {
    report_seq_errors(t);
    return 1;
  }

  return 0;
}

int drop_sequence(THD *thd, const std::string&db, const std::string&name, bool if_exists) {
  Sequence *seq = nullptr;

  std::string seq_name = name;
  if (1 == lower_case_table_names) {
    std::transform(seq_name.begin(), seq_name.end(), seq_name.begin(), ::tolower);
  }
  if (lock_object_name(thd, MDL_key::SEQUENCE, db.c_str(), seq_name.c_str()))
    return 1;
  // don't inc_ref() here otherwise seq cache can't drop seq.
  int ret = g_seq_cache.drop(db, seq_name, seq);

  if (ret < 0) {
    // target sequence not found.
    if (!if_exists) {
      my_error(ER_SEQUENCE_NOT_FOUND, MYF(0), seq_name.c_str(), db.c_str());
      ret= 1;
      goto end;
    } else {
      push_warning_printf(thd, Sql_condition::SL_NOTE,
                          ER_SEQUENCE_NOT_FOUND, ER_THD(thd, ER_SEQUENCE_NOT_FOUND),
                          seq_name.c_str(), db.c_str());
      ret= 0;
      goto end;
    }
  } else if (ret > 0) {
    ret = 1; // target sequence in use.
    goto end;
  }

  {
    Seq_task t(db, seq_name, Seq_task::DROP_SEQUENCE);
    t.set_thread_type(thd);
    seq_tasks.append_task(&t);
    t.wait();
    // when we return, our task has been executed.
    if (t.get_error()) {
      // restore cache state on error.
      if (t.get_error() != ER_SEQUENCE_NOT_FOUND) {
        g_seq_cache.insert(db, seq_name, seq);
        report_seq_errors(t);
        return 1; // return directly and avoid delete the seq in cache
      } else if (if_exists) {
        goto end;
      }
    }
    ret= 0;
  }
end:
  delete seq;
  return ret;
}

class Release_thd_seq_locks : public MDL_release_locks_visitor {
private:
  std::string m_seq_name;
  std::string m_db_name;
  THD *thd;
public:
  explicit Release_thd_seq_locks(THD *thd0, const char *db, const char *seq = NULL)
    : thd(thd0) {
    if (seq)
      m_seq_name= seq;
    if (db)
      m_db_name= db;
  }

  virtual bool release(MDL_ticket *ticket) override {
    if (m_db_name.length() == 0 && m_seq_name.length() > 0)
      m_db_name= thd->db().str;
    return (ticket->get_key()->mdl_namespace() == MDL_key::SEQUENCE &&
            ((m_db_name.length() == 0 && m_seq_name.length() == 0) ||
             (strcmp(m_db_name.c_str(), ticket->get_key()->db_name()) == 0 &&
              (m_seq_name.length() == 0 ||
               strcmp(m_seq_name.c_str(), ticket->get_key()->name()) == 0))));
  }
};


void THD::release_seq_refs() {
  /*
    Release sequence only when commit/rollback is an outermost operation.
    Rpl_table_access::open/close_table() does internal txn commit/rollback and
    we can't do this in that case.
  */
  if (ending_internal_txn)
    return;

  for (Thd_seq_db_map::iterator i= seq_dbs.begin(); i != seq_dbs.end(); ++i) {
    for (Thd_seq_name_map::iterator j= i->second.begin(); j != i->second.end(); ++j) {
      close_sequence(j->second->get_seq_obj());
      j->second->set_seq_obj(nullptr);
    }
  }
}

bool THD::release_seq_refs(const char *db0, const char *seq0)
{
  bool ret = false;

  if (db0 == nullptr && (seq0 != nullptr && this->db().str == nullptr)) {
    my_error(ER_NO_DB_ERROR, MYF(0));
    return true;
  }

  std::string name;
  if (seq0) {
    name = std::string(seq0);
    if (1 == lower_case_table_names) {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    }
  }
  if (db0 != nullptr || (db0= this->db().str) != nullptr) {
    std::string dbname(db0);
    Thd_seq_db_map::iterator i = seq_dbs.find(dbname);
    if (i == seq_dbs.end()) {
      goto done1; // nothing to do, but not an error.
    }

    if (seq0) {
      Thd_seq_name_map::iterator j= i->second.find(name);
      if (j == i->second.end())
        goto done1; // nothing to do, but not an error.
      delete j->second;
      i->second.erase(j);
    } else {
      for (Thd_seq_name_map::iterator j= i->second.begin(); j != i->second.end(); ++j) {
        delete j->second;
      }
      i->second.clear();
    }
  } else {
    for (Thd_seq_db_map::iterator i= seq_dbs.begin(); i != seq_dbs.end(); ++i) {
      for (Thd_seq_name_map::iterator j= i->second.begin(); j != i->second.end(); ++j) {
        delete j->second;
      }
      i->second.clear();
    }
  }
done1:
  /*
   Always release locks no matter seq refs are found or not above.
  */
  Release_thd_seq_locks r(this, db0, name.c_str());
  this->mdl_context.release_locks(&r);
  return ret;
}

/*
  Open sequence for a session, to be called whenever a seq is
  used/dropped/altered, need to acquire mdl locks.
  For create/drop/alter seq, need MDL_X, to exclude each other; and among them
  only alter calls get_sequence() but lock will be acquired in alter_args();

  For value references, we have to acuqire an explicit shared lock and release the lock
  when the session exits or on explicit command(unlock sequence), because a
  sequence reference goes beyond a statement/transaction.

  @param acquire_lock whether to acquire the SHARED EXPLICIT SEQUENCE lock.
  @retval 0 if sequence not found, or sequence is locked. In the latter case
  error is reported already. caller should only report error for 'sequence not
  found' case.
      seq ptr found.
*/
Sequence *get_sequence(THD *thd, const std::string&db, const std::string&name,
                       bool acquire_lock) {
  std::string seq_name = name;
  if (1 == lower_case_table_names) {
    std::transform(seq_name.begin(), seq_name.end(), seq_name.begin(), ::tolower);
  }
  if (acquire_lock) {
    MDL_request mdl_request;
    MDL_REQUEST_INIT(&mdl_request, MDL_key::SEQUENCE, db.c_str(),
                     seq_name.c_str(), MDL_SHARED, MDL_TRANSACTION);
    if (thd->mdl_context.acquire_lock(&mdl_request, thd->variables.lock_wait_timeout))
      return nullptr;
  }

  Sequence *seq = g_seq_cache.find(db, seq_name);
  /*
    Increment the ref count. Ref counting currently is correctly maintained but
    not used, because we have a limitless Sequence cache(g_seq_cache). If in
    future we want to limit the number of open Sequence objects cached, we can
    close seqs that are no longer referenced.
  */
  if (seq) {
    seq->inc_ref();
  }

  return seq;
}

/*
  Close a session's sequence reference. To be called at end of a session(THD) or
  when a THD_seq reference is destroyed. We don't release mdl locks here because
  sequence value reference lives beyond statement/transaction lifespan, so the
  shared locks are EXPLICIT locks that can only be released by 'unlock
  sequence' command.
*/
void close_sequence(Sequence *seq) {
  if (seq) {
    seq->dec_ref();
  }
}

void clear_sequence_cache(bool double_check) {
  Partitioned_rwlock_write_guard guard(&seq_cache_lock);

  if (double_check && !seq_need_reload()) {
    return;
  }

  g_seq_cache.clear();
  /* Tell all thd to clear its local cache */
  seq_cache_version = ++seq_version;
}

static void report_seq_errors(Seq_task &t) {
  if (t.get_error() == ER_SEQUENCE_EXISTS)
    my_error(ER_SEQUENCE_EXISTS, MYF(0), t.name.c_str(), t.db.c_str());
  else if (t.get_error() == ER_SEQUENCE_NOT_FOUND)
    my_error(ER_SEQUENCE_NOT_FOUND, MYF(0), t.name.c_str(), t.db.c_str());
  else
    my_error(ER_SEQUENCE_ASYNC_BG_WORKER_ERROR, MYF(0), t.m_thid, t.m_tid,
             t.get_error(), t.errstr.c_str(), Seq_task::type_str(t.type),
             t.db.c_str(), t.name.c_str());
}

int Seq_cache::lock_db_sequences(THD *thd, const char *db,bool skipTableNotExists) {
  Sequence::Scoped_pthread_mutex spm(mutex);
  Seq_db_map::iterator i = all_seqs.find(db);

  if (i == all_seqs.end() && ::load_db_seqs(db,false,skipTableNotExists))
    return -1;

  i = all_seqs.find(db); // now find it.
  if(i == all_seqs.end()) { //can't find it
    thd->clear_error();
    thd->get_stmt_da()->reset_condition_info(thd);
    return 0;
  }

  Seq_name_map::iterator j;
  MDL_request_list mdl_requests;
  for (j = i->second.begin(); j != i->second.end(); ++j) {
    MDL_request *mdl_request = new (thd->mem_root) MDL_request;
    MDL_REQUEST_INIT(mdl_request, MDL_key::SEQUENCE,
                     db, j->first.c_str(), MDL_EXCLUSIVE, MDL_TRANSACTION);
    mdl_requests.push_front(mdl_request);
  }

  int ret = thd->mdl_context.acquire_locks(&mdl_requests, thd->variables.lock_wait_timeout);
  return ret;
}

int lock_db_sequences(THD *thd, const char *db,bool skipTableNotExists) {
  return g_seq_cache.lock_db_sequences(thd, db,skipTableNotExists);
}

/*
  Drop database 'db''s cached sequence objects and its seqname->seq map from the
  seq cache.
*/
int Seq_cache::drop_db_sequences(const char *db) {
  Sequence::Scoped_pthread_mutex spm(mutex);
  Seq_db_map::iterator i = all_seqs.find(db);

  // update cache
  if (i != all_seqs.end())
  {
    Seq_name_map::iterator j;
    for (j = i->second.begin(); j != i->second.end(); ++j)
    {
      delete j->second;
    }

    all_seqs.erase(i);
  }

  return 0;
}

bool Seq_cache::db_has_sequence(const char *db) {
  Sequence::Scoped_pthread_mutex spm(mutex);
  Seq_db_map::iterator i = all_seqs.find(db);
  return (i != all_seqs.end() && !i->second.empty());
}

/*
  Drop sequences of 'db' from seq cache and also from mysql.tdsql_sequences
  table. The sequences are supposed to be locked by caller.
*/
int drop_db_sequences(const THD *thd, const char *db, bool do_binlogging) {
  int ret;
/** If there's no worker thread running, this is startup, so do nothing. */
  if (!seq_work_params[0].running) {
    assert(!g_seq_cache.db_has_sequence(db));
    return 0;
  }

  if (!g_seq_cache.db_has_sequence(db)) {
   Sequence::Scoped_pthread_mutex spm(g_seq_cache.get_mutex());
   load_db_seqs(std::string(db), false, true);
  }

  bool has_seq = g_seq_cache.db_has_sequence(db);

  /* Remove element from cache.The db element may be empty
  and we need to erase it from cache. */
  if ((ret= g_seq_cache.drop_db_sequences(db))) {
    return ret;
  }

  if (!has_seq) {
    return 0;
  }

  Seq_task t(db, "", Seq_task::DROP_DB_SEQ);
  t.set_thread_type(thd);

  /*
    Currently this is only called as part of 'drop database' operation, thus we
    must turn off binlogging.
  */
  t.do_binlogging(do_binlogging);

  seq_tasks.append_task(&t);
  t.wait();
  // when we return, our task has been executed.
  /*
  The caller(drop db ) can not fail in the middle of the drop process so the
  error return is ignored, thus we can't report error here.
  if (t.get_error())
  {
    report_seq_errors(t);
    return 1;
  }
*/
  return 0;
}

void db_object_recycle(THD *thd, const char *dbname, uint64_t schema_id) {
  g_seq_cache.drop_db_sequences(dbname);
  Object_recycle_task t(dbname, schema_id, Seq_task::RECYCLE_SEQUENCE, true);
  seq_tasks.append_task(&t);
  t.wait();

  if (t.get_error()) {
    push_warning_printf(thd, Sql_condition::SL_NOTE,
                        ER_RECYCLE_BIN_FAIL_OP_OBJECTS,
                        ER_THD(thd, ER_RECYCLE_BIN_FAIL_OP_OBJECTS), "recycle",
                        "sequence", dbname, t.get_error());
  }

  /* Recycle routine */
  Object_recycle_task t2(dbname, schema_id, Seq_task::RECYCLE_ROUTINE, false);
  t2.thread_type = SYSTEM_THREAD_DD_MODIFY;
  seq_tasks.append_task(&t2);
  t2.wait();

  if (t2.get_error()) {
    push_warning_printf(thd, Sql_condition::SL_NOTE,
                        ER_RECYCLE_BIN_FAIL_OP_OBJECTS,
                        ER_THD(thd, ER_RECYCLE_BIN_FAIL_OP_OBJECTS), "recycle",
                        "routine", dbname, t2.get_error());
  }

  sp_cache_invalidate();
}

void db_object_clear(THD *thd, time_t before_time) {
  Object_clear_task t(before_time, Seq_task::CLEAR_SEQUENCE, true);
  seq_tasks.append_task(&t);
  t.wait();

  /* Clear routine */
  Object_clear_task t2(before_time, Seq_task::CLEAR_ROUTINE, false);
  t2.thread_type = SYSTEM_THREAD_DD_MODIFY;
  seq_tasks.append_task(&t2);
  t2.wait();

  sp_cache_invalidate();
}

void db_object_restore(THD *thd, const char *dbname, uint64_t schema_id) {
  Object_restore_task t(dbname, schema_id, Seq_task::RESTORE_SEQUENCE, true);
  seq_tasks.append_task(&t);
  t.wait();

  if (t.get_error()) {
    push_warning_printf(thd, Sql_condition::SL_NOTE,
                        ER_RECYCLE_BIN_FAIL_OP_OBJECTS,
                        ER_THD(thd, ER_RECYCLE_BIN_FAIL_OP_OBJECTS), "restore",
                        "sequence", dbname, t.get_error());
  }

  /* Restore routine */
  Object_restore_task t2(dbname, schema_id, Seq_task::RESTORE_ROUTINE, false);
  t2.thread_type = SYSTEM_THREAD_DD_MODIFY;
  seq_tasks.append_task(&t2);
  t2.wait();

  if (t2.get_error()) {
    push_warning_printf(thd, Sql_condition::SL_NOTE,
                        ER_RECYCLE_BIN_FAIL_OP_OBJECTS,
                        ER_THD(thd, ER_RECYCLE_BIN_FAIL_OP_OBJECTS), "restore",
                        "routine", dbname, t2.get_error());
  }

  sp_cache_invalidate();
}

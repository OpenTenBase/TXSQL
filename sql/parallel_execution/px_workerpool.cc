#include "px_workerpool.h"
#include "sql/psi_memory_key.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "px_executor.h"
#include "sql/log.h"
#if defined(HAVE_OPT_CTX)
#include "sql/parallel_execution/opt_interface.h"  // OPT_CTX
#endif
#include "sql/mysqld_thd_manager.h"

static const int right_deep_tree_limit = 64;
static void* thread_func_in_worker(void *);

// TODO: more mysql_create_thread PSI interface.
PSI_stage_info stage_waiting_workers_generate_plan =
  {0, "Waiting workers generate physical plan", 0, PSI_DOCUMENT_ME};
PSI_stage_info stage_waiting_finish_query_finally =
  {0, "Waiting query finish finally", 0, PSI_DOCUMENT_ME};
PSI_stage_info stage_waiting_workers_finish_task =
  {0, "Waiting workers finish task", 0, PSI_DOCUMENT_ME};

PSI_stage_info stage_waiting_signal_to_begin_query =
  {0, "Waiting to begin query", 0, PSI_DOCUMENT_ME};
PSI_stage_info stage_waiting_signal_to_begin_task =
  {0, "Waiting to dispatch task", 0, PSI_DOCUMENT_ME};

void px_send_command(cond_with_lock_t *px_cond)
{
  mysql_mutex_lock(&px_cond->LOCK_worker_signal);
  px_cond->flag_COND_signal++;
  mysql_cond_signal(&px_cond->COND_worker_signal);
  mysql_mutex_unlock(&px_cond->LOCK_worker_signal);
}

void px_wait_finally(THD *thd, cond_with_lock_t *px_cond,
                          const PSI_stage_info *stage,
                          const char *src_function, const char *src_file,
                          int src_line)
{
  mysql_mutex_lock(&px_cond->LOCK_worker_signal);
  thd->enter_cond(&px_cond->COND_worker_signal,
                  &px_cond->LOCK_worker_signal,
                  stage, nullptr, src_function, src_file, src_line);
  while (!px_cond->flag_COND_signal)
    mysql_cond_wait(&px_cond->COND_worker_signal,
                    &px_cond->LOCK_worker_signal);
  px_cond->flag_COND_signal = 0;
  mysql_mutex_unlock(&px_cond->LOCK_worker_signal);
  thd->exit_cond(nullptr, src_function, src_file, src_line);
}

void px_wait_for_signal(THD *thd, cond_with_lock_t *px_cond,
                        const PSI_stage_info *stage,
                        const char *src_function, const char *src_file,
                        int src_line)
{
  mysql_mutex_lock(&px_cond->LOCK_worker_signal);
  thd->enter_cond(&px_cond->COND_worker_signal,
                  &px_cond->LOCK_worker_signal,
                  stage, nullptr, src_function, src_file, src_line);
  while (!px_cond->flag_COND_signal && !thd->killed)
    mysql_cond_wait(&px_cond->COND_worker_signal,
                    &px_cond->LOCK_worker_signal);
  px_cond->flag_COND_signal--;
  mysql_mutex_unlock(&px_cond->LOCK_worker_signal);
  thd->exit_cond(nullptr, src_function, src_file, src_line);
}

void px_send_report(cond_with_lock_t *px_cond, std::atomic<int> *done,
                  int num_workers)
{
  mysql_mutex_lock(&px_cond->LOCK_worker_signal);
  int num_workers_done = 0;
  num_workers_done = done->fetch_add(1, std::memory_order_relaxed)+1;
  if (num_workers_done == num_workers) {
    px_cond->flag_COND_signal++;
    mysql_cond_signal(&px_cond->COND_worker_signal);
  }
  mysql_mutex_unlock(&px_cond->LOCK_worker_signal);
}

/**
  Control creatation of worker threads, create physical threads for one query
  and set up synchronization mechanism. Currently we create these threads in
  each time, A threadpool may be added to handle concurrent queries.
*/
worker_pool_t* create_worker_threads(int num_threads, const THD_array &list)
{
  int i, j, ret = 0;
  worker_pool_t *worker_pool;

  worker_pool = (worker_pool_t*) malloc (sizeof (worker_pool_t));
  if (worker_pool == nullptr) 
    return nullptr;

  worker_pool->num_workers = num_threads;
  worker_pool->bitmap_map = (MY_BITMAP *)malloc (sizeof (MY_BITMAP));
  if (worker_pool->bitmap_map == nullptr)
    goto fail_of_bitmap_map;

  // We allocate 64 bitmaps to handle most 64 right deep dfo tree.
  worker_pool->bitmap = (MY_BITMAP **)malloc (sizeof (MY_BITMAP *) * right_deep_tree_limit);
  if (worker_pool->bitmap == nullptr)
    goto fail_of_bitmap;

  worker_pool->args = (void **)malloc (sizeof (void *) * num_threads);
  if (worker_pool->args == nullptr) 
    goto fail_of_args;

  worker_pool->threads = (pthread_t *)malloc (sizeof (pthread_t) * num_threads);
  if (worker_pool->threads == nullptr) 
    goto fail_of_threads;

  worker_pool->thread_args = (worker_thread_arg *)malloc (
    sizeof (worker_thread_arg) * num_threads);
  if (worker_pool->thread_args == nullptr) 
    goto fail_of_thread_args;

  // Set the bitmap of the concurrent dfo execution.
  for (j = 0; j < right_deep_tree_limit; ++j) {
    worker_pool->bitmap[j] = (MY_BITMAP *)malloc (sizeof (MY_BITMAP));
    if (worker_pool->bitmap[j] == nullptr)
      goto fail_of_malloc;
  }

  bitmap_init(worker_pool->bitmap_map, nullptr, right_deep_tree_limit);
  bitmap_init(&worker_pool->threads_bitmap, nullptr, num_threads);

  mysql_mutex_init(key_LOCK_Running_Task_Barrier,
    &worker_pool->mutex_task, MY_MUTEX_INIT_FAST);

  px_condition_init(&worker_pool->sem_workers_done);
  px_condition_init(&worker_pool->sem_tasks_done);
  px_condition_init(&worker_pool->sem_query_done);

  worker_pool->finished = 0;
  worker_pool->num_query_done = 0;
  worker_pool->num_workers_done = 0;
  for (i = 0; i < num_threads; ++i) {// Init the arguments of all workers.
    px_condition_init(&worker_pool->thread_args[i].sem_worker_signal);
    px_condition_init(&worker_pool->thread_args[i].sem_task_signal);

    worker_pool->thread_args[i].num_query_done = &worker_pool->num_query_done;
    worker_pool->thread_args[i].num_workers_done = &worker_pool->num_workers_done;
    worker_pool->thread_args[i].sem_workers_done = &worker_pool->sem_workers_done;
    worker_pool->thread_args[i].sem_tasks_done = &worker_pool->sem_tasks_done;
    worker_pool->thread_args[i].sem_query_done = &worker_pool->sem_query_done;
    worker_pool->thread_args[i].finished = &worker_pool->finished;
    worker_pool->thread_args[i].bitmap = &worker_pool->bitmap;
    worker_pool->thread_args[i].bitmap_map = &worker_pool->bitmap_map;
    worker_pool->thread_args[i].mutex_task = &worker_pool->mutex_task;
    worker_pool->thread_args[i].thread_func = &worker_pool->thread_func;
    worker_pool->thread_args[i].thread_func_arg = &worker_pool->args[i];
    worker_pool->thread_args[i].num_workers = &worker_pool->num_workers;
    worker_pool->thread_args[i].is_equivalent_plan = false;
    worker_pool->thread_args[i].px_exchange_context = &worker_pool->px_exchange_context;
    worker_pool->thread_args[i].worker_thd = list[i];
    ret = pthread_create (&worker_pool->threads[i], &connection_attrib,
                          thread_func_in_worker, &worker_pool->thread_args[i]);
    /**
      Currently, once a worker thread is successfully created, it immediately
      go to `rebuild_query_execution()`, `pthread_create` may return error,
      resulting in workers incompletely created. So created threads should be
      forbidden to parse, optimize or execute.
    */
    if (i == 1 && !ret) {
      DBUG_EXECUTE_IF("create_partial_threads_error", {i++; ret=true;});
    }

    if (ret) 
      goto fail_of_create;
  }

  return worker_pool;

fail_of_create:
  worker_pool->num_workers = i--;
  while (i >= 0) {
    worker_pool->thread_args[i].worker_thd->px_create_failed = true;
    px_send_command(&worker_pool->thread_args[i].sem_worker_signal);
    --i;
  }
  px_wait_finally(current_thd, &worker_pool->sem_query_done,
                  &stage_waiting_finish_query_finally,
                  __FUNCTION__, __FILE__, __LINE__);

  for (i = 0; i < worker_pool->num_workers; ++i)
    destroy_cond_for_worker(&worker_pool->thread_args[i].sem_task_signal);
  for (i = 0; i < worker_pool->num_workers; ++i)
    destroy_cond_for_worker(&worker_pool->thread_args[i].sem_worker_signal);
  destroy_cond_for_worker(&worker_pool->sem_query_done);
  destroy_cond_for_worker(&worker_pool->sem_tasks_done);
  destroy_cond_for_worker(&worker_pool->sem_workers_done);

  mysql_mutex_destroy(&worker_pool->mutex_task);

  bitmap_free(worker_pool->bitmap_map);
  bitmap_free(&worker_pool->threads_bitmap);

fail_of_malloc:
  --j;
  while (j >= 0) {
    free(worker_pool->bitmap[j]);
    --j;
  }
  free (worker_pool->thread_args);

fail_of_thread_args:
  free (worker_pool->threads);
fail_of_threads:
  free (worker_pool->args);
fail_of_args:
  free (worker_pool->bitmap);
fail_of_bitmap:
  free (worker_pool->bitmap_map);
fail_of_bitmap_map:

  return nullptr;
}

void release_worker_threads(worker_pool_t *worker_pool)
{
  for (int i = 0; i < worker_pool->num_workers; ++i)
    destroy_cond_for_worker(&worker_pool->thread_args[i].sem_task_signal);
  for (int i = 0; i < worker_pool->num_workers; ++i)
    destroy_cond_for_worker(&worker_pool->thread_args[i].sem_worker_signal);
  destroy_cond_for_worker(&worker_pool->sem_query_done);
  destroy_cond_for_worker(&worker_pool->sem_tasks_done);
  destroy_cond_for_worker(&worker_pool->sem_workers_done);
  mysql_mutex_destroy(&worker_pool->mutex_task);
  for (int i = 0; i < right_deep_tree_limit; ++i) {
    if (bitmap_is_set(worker_pool->bitmap_map, i))
      bitmap_free(worker_pool->bitmap[i]);
    free(worker_pool->bitmap[i]);
  }
  free (worker_pool->bitmap);
  bitmap_free(worker_pool->bitmap_map);
  free (worker_pool->bitmap_map);
  bitmap_free(&worker_pool->threads_bitmap);
  free (worker_pool->args);
  free (worker_pool->threads);
  free (worker_pool->thread_args);
  free (worker_pool);
}

/* Init the px condition with lock. */
void px_condition_init(cond_with_lock_t *condition_with_lock)
{
  condition_with_lock->flag_COND_signal = 0;
  mysql_mutex_init(key_px_worker_lock,
  &condition_with_lock->LOCK_worker_signal, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_px_worker_cond,
  &condition_with_lock->COND_worker_signal);
}

/**
  Set thread function and task dop. The thread_func has two level:
    1. rebuild_query_execution outside;
    2. execute_task_in_worker inside;
*/
void set_threads_args(worker_pool_t *worker_pool, worker_func thread_func,
                      void **args)
{
  worker_pool->thread_func = thread_func;
  for (int i = 0; i < worker_pool->num_workers; ++i)
    worker_pool->args[i] = args[i];
}

/**
  Outside synchronization control for whole query, to start query.
  This mainly worked for parse, optimize and cleanup.
*/
bool begin_query(worker_pool_t *worker_pool)
{
  worker_pool->num_workers_done = 0;
  for (int i = 0; i < worker_pool->num_workers; ++i)
    px_send_command(&worker_pool->thread_args[i].sem_worker_signal);
  return false;
}

/**
  Outside synchronization control for whole query, query barrier.
  This mainly worked for parse, optimize and cleanup.
*/
bool wait_workers_generate_plan(worker_pool_t *worker_pool, THD *thd)
{
  px_wait_for_signal(thd, &worker_pool->sem_workers_done,
                     &stage_waiting_workers_generate_plan,
                     __FUNCTION__, __FILE__, __LINE__);
  return false;
}

void wait_for_begin_query(worker_thread_arg *arg)
{
  px_wait_for_signal(arg->worker_thd, &arg->sem_worker_signal,
                     &stage_waiting_signal_to_begin_query,
                     __FUNCTION__, __FILE__, __LINE__);
}

/**
  Outside synchronization control for whole query, ready to post.
  This mainly worked for parse, optimize and cleanup.
*/
bool optimize_finsh_signal(worker_thread_arg* arg, bool error)
{
  px_send_report(arg->sem_workers_done, arg->num_workers_done,
                 *arg->num_workers);
  if (error) {
    px_wait_for_signal(arg->worker_thd, &arg->sem_task_signal,
                       &stage_waiting_workers_finish_task,
                       __FUNCTION__, __FILE__, __LINE__);
  }
  return false;
}

void finish_query(worker_thread_arg* arg)
{
  px_send_report(arg->sem_query_done, arg->num_query_done,
                 *arg->num_workers);
}

/**
  Allocate threads for task. Set task execution bitmap, record
  which threads are executing, return task execution group.
*/
bool allocate_threads(worker_pool_t *worker_pool, Worker_exec_ctx *ctx)
{
  int chosen = -1;
  int threads_num = ctx->dfo()->dop();
  // Allocate threads for task by dop of task.
  for (int i = 0, j = 0; i < worker_pool->num_workers && j < threads_num; ++i)
    if (!bitmap_is_set(&worker_pool->threads_bitmap, i)) {
      j++;
      bitmap_set_bit(&ctx->bitmap, i);
      bitmap_set_bit(&worker_pool->threads_bitmap, i);
    }
  // Set task execution bitmap, record which threads are executing
  for (int i = 0; i < right_deep_tree_limit; ++i)
    if (!bitmap_is_set(worker_pool->bitmap_map, i)) {
      chosen = i;
      MY_BITMAP *to_bitmap = worker_pool->bitmap[chosen];
      bitmap_init(to_bitmap, nullptr, worker_pool->num_workers);
      bitmap_copy(to_bitmap, &ctx->bitmap);
      bitmap_set_bit(worker_pool->bitmap_map, i);
      break;
    }

  assert(-1 != chosen);
  // task execution group.
  ctx->set_group_id(chosen);
  return false;
}

/**
  Inside synchronization control for task dispatch, to start task.
  This mainly worked for query plan slice running.
*/
bool begin_task(worker_pool_t *worker_pool, Worker_exec_ctx *ctx)
{
  for (int i = 0; i < worker_pool->num_workers; ++i) {
    worker_thread_arg *arg = &worker_pool->thread_args[i];
    if (bitmap_is_set(&ctx->bitmap, i))
      px_send_command(&arg->sem_task_signal);
  }
  return false;
}

/**
  Inside synchronization control for task dispatch, task barrier.
  This mainly worked for query plan slice running.
*/
bool wait_workers_finish_task(worker_pool_t *worker_pool, Worker_exec_ctx *ctx)
{
  for (int i = 0; i < worker_pool->num_workers; ++i)
    if (bitmap_is_set(&ctx->bitmap, i))
      bitmap_clear_bit(&worker_pool->threads_bitmap, i);
  px_wait_for_signal(current_thd, &worker_pool->sem_tasks_done,
                     &stage_waiting_workers_finish_task,
                     __FUNCTION__, __FILE__, __LINE__);
  return false;
}

void wait_for_begin_task(worker_thread_arg *arg)
{
  px_wait_for_signal(arg->worker_thd, &arg->sem_task_signal,
                     &stage_waiting_signal_to_begin_task,
                     __FUNCTION__, __FILE__, __LINE__);
  arg->worker_thd->is_running_task = true;
}

/**
  Inside synchronization control for task dispatch, ready to post.
  This mainly worked for query plan slice running.
*/
void finish_task(worker_thread_arg *arg)
{
  assert(arg->worker_thd);
  int group_id = arg->worker_thd->thread_group_id;
  mysql_mutex_lock(arg->mutex_task);
  MY_BITMAP *chosen_bitmap = (*arg->bitmap)[group_id];
  bitmap_clear_bit(chosen_bitmap, arg->worker_thd->worker_id);
  if (bitmap_is_clear_all(chosen_bitmap)) {
    bitmap_clear_bit(*arg->bitmap_map, group_id);
    bitmap_free((*arg->bitmap)[group_id]);
    px_send_command(arg->sem_tasks_done);
  }
  arg->worker_thd->is_running_task = false;
  mysql_mutex_unlock(arg->mutex_task);
}

/**
  When inside synchronization finished, set finished to 1 and all workers jump
  out of loop, every worker begin to cleanup in each. 
*/
void schedule_over(worker_pool_t *worker_pool, THD *thd)
{
  assert (worker_pool->finished == 0);
  worker_pool->finished = 1;
  worker_pool->num_workers_done = 0;
  for (int i = 0; i < worker_pool->num_workers; ++i) {
    worker_thread_arg *arg = &worker_pool->thread_args[i];
    px_send_command(&arg->sem_task_signal);
  }
}

/* Cleanup worker pool for the SQL PX execution. */
void schedule_end(worker_pool_t *worker_pool)
{
  px_wait_finally(current_thd, &worker_pool->sem_query_done,
                  &stage_waiting_finish_query_finally,
                  __FUNCTION__, __FILE__, __LINE__);
}

void destroy_cond_for_worker(cond_with_lock_t *cond)
{
  mysql_cond_destroy(&cond->COND_worker_signal);
  mysql_mutex_destroy(&cond->LOCK_worker_signal);
}

/** 
  The thread function which will execute parse and optimize step, it
  belongs to outside logic, function jump out when finshed set to 1.
*/
static void* thread_func_in_worker(void *arg)
{
  worker_func thread_func;
  void *thread_func_arg = nullptr;
  worker_thread_arg *thread_arg = (worker_thread_arg *) arg;
  my_thread_init();

  thread_arg->worker_thd->thread_stack = (char *)&arg;
  thread_arg->worker_thd->set_new_thread_id();
  thread_arg->worker_thd->store_globals();

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  thd_manager->add_thd(thread_arg->worker_thd);

#if defined(HAVE_OPT_CTX)
  assert(OPT_CTX_ENABLED(thread_arg->worker_thd));
#ifndef DBUG_OFF
  OPT_CTX(thread_arg->worker_thd).dbug_init_thd();
#endif
#else
#ifndef DBUG_OFF
  THD *coordinator_thd = thread_arg->worker_thd->px_coordinator;
  for (auto &val : coordinator_thd->dbug_vals) {
    DBUG_SET(val);
    PX_PRINT_INFO("DBUG_SET %s", val);
  }
#endif
#endif

  wait_for_begin_query (thread_arg);

  if (thread_arg->worker_thd->px_create_failed)
    goto finish;

  if (thread_arg->worker_thd->killed)
    goto finish;

  thread_func = *(thread_arg->thread_func);
  thread_func_arg = *(thread_arg->thread_func_arg);
  // Only worked for parse, optimize, execute and clean.
  (*thread_func)(thread_func_arg);

 finish:
  thread_arg->worker_thd->release_resources();
  thd_manager->remove_thd(thread_arg->worker_thd);
  finish_query(thread_arg);
  my_thread_end();
  my_thread_exit(nullptr);
  return nullptr;
}

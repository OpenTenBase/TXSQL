#include "px_workerpool.h"
#include "mysql/psi/mysql_thread.h"
#include "px_executor.h"
#include "sql/log.h"

static const int right_deep_tree_limit = 64;
static void* thread_func_in_worker(void *);

/**
  Control creatation of worker threads, create physical threads for one query
  and set up synchronization mechanism. Currently we create these threads in
  each time, A threadpool may be added to handle concurrent queries.
*/
worker_pool_t* create_worker_threads(int num_threads)
{
  int i, j, ret = 0;
  worker_pool_t *worker_pool;
  pthread_attr_t attr;

  worker_pool = (worker_pool_t*) malloc (sizeof (worker_pool_t));
  if (worker_pool == nullptr) 
    return nullptr;

  worker_pool->num_workers = num_threads;
  worker_pool->bitmap_map = (uint64_t)0UL;

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

  mysql_mutex_init(key_LOCK_Running_Task_Barrier,
    &worker_pool->mutex_task, MY_MUTEX_INIT_FAST);
  ret = pthread_semphore_init (&worker_pool->sem_workers_done, 0, 0);
  if (ret != 0) 
    goto fail_of_all_workers_done;

  pthread_attr_init (&attr);
  pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  worker_pool->finished = 0;
  for (i = 0; i < num_threads; ++i) {
    // Init the arguments of all workers.
    pthread_semphore_init(&(worker_pool->thread_args[i].sem_inner), 0, 0);
    pthread_semphore_init(&(worker_pool->thread_args[i].sem_outer), 0, 0);
    worker_pool->thread_args[i].sem_workers_done = 
      &worker_pool->sem_workers_done;
    worker_pool->thread_args[i].sem_tasks_done =
      &worker_pool->sem_tasks_done;
    worker_pool->thread_args[i].num_workers_done = 
      &worker_pool->num_workers_done;
    worker_pool->thread_args[i].finished = &worker_pool->finished;
    worker_pool->thread_args[i].bitmap = &worker_pool->bitmap;
    worker_pool->thread_args[i].bitmap_map = &worker_pool->bitmap_map;
    worker_pool->thread_args[i].mutex_task = &worker_pool->mutex_task;
    worker_pool->thread_args[i].thread_func = &worker_pool->thread_func;
    worker_pool->thread_args[i].thread_func_arg = &worker_pool->args[i];
    worker_pool->thread_args[i].num_workers = &worker_pool->num_workers;
    // The default value is set to unequal, which needs to be checked by each worker
    worker_pool->thread_args[i].is_equivalent_plan = false;
    worker_pool->thread_args[i].px_exchange_context = &worker_pool->px_exchange_context;
    ret = pthread_create (&worker_pool->threads[i], &attr, thread_func_in_worker,
                          &worker_pool->thread_args[i]);
    if (ret) 
      goto fail_of_create;
  }

  // Set the bitmap of the concurrent dfo execution.
  for (j = 0; j < right_deep_tree_limit; ++j) {
    worker_pool->bitmap[j] = (MY_BITMAP *)malloc (sizeof (MY_BITMAP));
    if (worker_pool->bitmap[j] == nullptr)
      goto fail_of_malloc;
  }

  bitmap_init(&worker_pool->threads_bitmap, nullptr, num_threads);

  return worker_pool;

fail_of_malloc:
  --j;
  while (j >= 0) {
    free(worker_pool->bitmap[j]);
    --j;
  }

fail_of_create:
  --i;
  while (i >= 0) {
    pthread_cancel(worker_pool->threads[i]);
    --i;
  }
fail_of_all_workers_done:
  free (worker_pool->thread_args);
fail_of_thread_args:
  free (worker_pool->threads);
fail_of_threads:
  free (worker_pool->args);
fail_of_args:
  free (worker_pool->bitmap);
fail_of_bitmap:

  return nullptr;
}

/**
  Set thread function and task dop. The thread_func has two level:
    1. rebuild_query_execution outside;
    2. execute_task_in_worker inside;
*/
void set_threads_args(worker_pool_t *worker_pool, worker_func thread_func,
                      void **args, int num_workers)
{
  worker_pool->thread_func = thread_func;
  for (int i = 0; i < num_workers; ++i)
    worker_pool->args[i] = args[i];
}

/**
  Outside synchronization control for whole query, to start query.
  This mainly worked for parse, optimize and cleanup.
*/
bool query_execute_start(worker_pool_t *worker_pool)
{
  worker_pool->bitmap_map = (uint64_t)0UL;
  worker_pool->num_workers_done = 0;
  for (int i = 0; i < worker_pool->num_workers; ++i) {
    if (0 != pthread_semphore_post(&(worker_pool->thread_args[i].sem_outer)))
      return true;
  }
  return false;
}

/**
  Outside synchronization control for whole query, query barrier.
  This mainly worked for parse, optimize and cleanup.
*/
bool query_execute_barrier(worker_pool_t *worker_pool)
{
  if (0 != pthread_semphore_wait(&worker_pool->sem_workers_done))
    return true;
  return false;
}

/**
  Outside synchronization control for whole query, ready to post.
  This mainly worked for parse, optimize and cleanup.
*/
bool optimize_finsh_signal(worker_thread_arg* arg)
{
  int num_workers_done = 0;
  num_workers_done = arg->num_workers_done->fetch_add(1,
                       std::memory_order_relaxed)+1;
  // All workers have done.
  if (num_workers_done == *arg->num_workers) {
    sql_print_information("optimize_finsh_signal sem_workers_done open !!!!");
    if (0 != pthread_semphore_post(arg->sem_workers_done))
      return true;
  }
  return false;
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
    if (!(worker_pool->bitmap_map & (1 << i))) {
      chosen = i;
      MY_BITMAP *to_bitmap = worker_pool->bitmap[chosen];
      bitmap_init(to_bitmap, nullptr, worker_pool->num_workers);
      bitmap_copy(to_bitmap, &ctx->bitmap);
      worker_pool->bitmap_map |= ((uint64_t)1 << chosen);
      break;
    }
  sql_print_information("set_threads_bitmap: group[%d]", chosen);
  assert(-1 != chosen);
  // task execution group.
  ctx->set_group_id(chosen);
  return false;
}

/**
  Inside synchronization control for task dispatch, to start task.
  This mainly worked for query plan slice running.
*/
bool task_execute_start(worker_pool_t *worker_pool, Worker_exec_ctx *ctx)
{
  worker_pool->num_workers_done = 0;
  for (int i = 0; i < worker_pool->num_workers; ++i) {
    if (bitmap_is_set(&ctx->bitmap, i))
      if (0 != pthread_semphore_post(&(worker_pool->thread_args[i].sem_inner)))
        return true;
  }
  return false;
}

/**
  Inside synchronization control for task dispatch, task barrier.
  This mainly worked for query plan slice running.
*/
bool task_execute_barrier(worker_pool_t *worker_pool, Worker_exec_ctx *ctx)
{
  for (int i = 0; i < worker_pool->num_workers; ++i)
    if (bitmap_is_set(&ctx->bitmap, i))
      bitmap_clear_bit(&worker_pool->threads_bitmap, i);
  if (0 != pthread_semphore_wait(&worker_pool->sem_workers_done))
    return true;
  return false;
}

/**
  Inside synchronization control for task dispatch, ready to post.
  This mainly worked for query plan slice running.
*/
void task_finish_signal(worker_thread_arg *arg)
{
  assert(arg->worker_thd);
  int group_id = arg->worker_thd->thread_group_id;
  mysql_mutex_lock(arg->mutex_task);
  MY_BITMAP *chosen_bitmap = (*arg->bitmap)[group_id];
  bitmap_clear_bit(chosen_bitmap, arg->worker_thd->worker_id);
  if (bitmap_is_clear_all(chosen_bitmap)) {
    sql_print_information("task_finish_signal. group: %d", group_id);
    *(arg->bitmap_map) ^= group_id;
    sql_print_information("task_finish_signal sem_workers_done open !!!!");
    pthread_semphore_post(arg->sem_workers_done);
  }
  mysql_mutex_unlock(arg->mutex_task);
}

/**
  When inside synchronization finished, set finished to 1 and all workers jump
  out of loop, every worker begin to cleanup in each. 
*/
bool worker_pool_destroy(worker_pool_t *worker_pool)
{
  assert (worker_pool->finished == 0);
  worker_pool->bitmap_map = (uint64_t)0UL;
  worker_pool->num_workers_done = 0;
  for (int i = 0; i < worker_pool->num_workers; ++i) {
    worker_pool->finished = 1;
    if (0 != pthread_semphore_post(&worker_pool->thread_args[i].sem_inner))
      return true;
  }
  if (0 != pthread_semphore_wait(&worker_pool->sem_workers_done))
    return true;
  return false;
}

/* Cleanup worker pool for the SQL PX execution. */
void worker_pool_cleanup(worker_pool_t *worker_pool)
{
  mysql_mutex_destroy(&worker_pool->mutex_task);
  sem_destroy(&worker_pool->sem_workers_done);
  for (int i = 0; i < right_deep_tree_limit; ++i)
    free(worker_pool->bitmap[i]);
  free (worker_pool->bitmap);
  free (worker_pool->args);
  free (worker_pool->threads);
  free (worker_pool->thread_args);
  free (worker_pool);
}

void handle_optimize_finish_error(worker_thread_arg* arg)
{
  optimize_finsh_signal(arg);
  pthread_semphore_wait(&arg->sem_inner);
}

/** 
  The thread function which will execute parse and optimize step, it
  belongs to outside logic, function jump out when finshed set to 1.
*/
static void* thread_func_in_worker(void *arg)
{
  worker_func thread_func;
  int num_workers_done = 0;
  worker_thread_arg *thread_arg = (worker_thread_arg *) arg;
  void *thread_func_arg;
  my_thread_init();

  while (true) { // Execute once.
    sem_wait (&thread_arg->sem_outer);
    if (*thread_arg->finished)
      break;

    thread_func = *(thread_arg->thread_func);
    thread_func_arg = *(thread_arg->thread_func_arg);

    // Only worked for parse and optimize.
    (*thread_func)(thread_func_arg);

    num_workers_done = thread_arg->num_workers_done->fetch_add(1,
                         std::memory_order_relaxed)+1;
    if (num_workers_done == *thread_arg->num_workers) {
      // All workers have done.
      pthread_semphore_post(thread_arg->sem_workers_done);
    }
  }

  sem_destroy (&thread_arg->sem_outer);
  num_workers_done = thread_arg->num_workers_done->fetch_add(1,
                       std::memory_order_relaxed)+1;
  if (num_workers_done == *thread_arg->num_workers) {
    // All workers have done.
    pthread_semphore_post(thread_arg->sem_workers_done);
  }

  my_thread_end();
  my_thread_exit(nullptr);
  return nullptr;
}

#include "px_workerpool.h"

static void* worker_func_inner(void *);

/**
  Control creatation of worker pool, create physical threads for one query and
  set up synchronization mechanism. Currently we create these threads in each
  time, A threadpool may be added to handle concurrent queries.
*/
worker_pool_t* worker_pool_create(int num_threads)
{
  int i, ret = 0;
  worker_pool_t *worker_pool;
  pthread_attr_t attr;

  worker_pool = (worker_pool_t*) malloc (sizeof (worker_pool_t));
  if (worker_pool == nullptr) 
    return nullptr;

  worker_pool->num_threads = num_threads;
  worker_pool->num_workers = num_threads;

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

  ret = sem_init (&worker_pool->sem_workers_done, 0, 0);
  if (ret != 0) 
    goto fail_of_all_workers_done;

  pthread_attr_init (&attr);
  pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  worker_pool->finished = 0;
  for (i = 0; i < num_threads; ++i) {
    // Init the arguments of all workers.
    sem_init(&(worker_pool->thread_args[i].sem_task_run), 0, 0);
    sem_init(&(worker_pool->thread_args[i].sem_parse_optimize_run), 0, 0);
    worker_pool->thread_args[i].sem_workers_done = 
      &worker_pool->sem_workers_done;
    worker_pool->thread_args[i].sem_tasks_done =
      &worker_pool->sem_tasks_done;
    worker_pool->thread_args[i].num_workers_done = 
      &worker_pool->num_workers_done;
    worker_pool->thread_args[i].finished = &worker_pool->finished;
    worker_pool->thread_args[i].thread_func = &worker_pool->thread_func;
    worker_pool->thread_args[i].thread_func_arg = &worker_pool->args[i];
    worker_pool->thread_args[i].num_workers = &worker_pool->num_workers;
    worker_pool->thread_args[i].error = false;
        
    ret = pthread_create (&worker_pool->threads[i], &attr, worker_func_inner,
                          &worker_pool->thread_args[i]);
    if (ret) 
      goto fail_of_create;
    }

  return worker_pool;

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

  return nullptr;
}

/**
  Set the thread function which will be executed and the parallel degree. In two
  level synchronization, outside control we set parse and optimize func, inside
  control we set task run func.
*/
int worker_pool_set(worker_pool_t *worker_pool, worker_func thread_func,
                    void **args, int num_workers)
{
  int i;
  worker_pool->thread_func = thread_func;

  assert (num_workers <= worker_pool->num_threads);
  worker_pool->num_workers = num_workers;

  for (i = 0; i < num_workers; ++i) {
    worker_pool->args[i] = args[i];
  }

  return 0;
}

/**
  Outside synchronization control for the whole query, set to start the query.
  This mainly worked for parse, optimize and cleanup.
*/
int worker_pool_begin_query(worker_pool_t *worker_pool)
{
  int i, ret;
  if (worker_pool->num_workers == 0)
    return 0;

  worker_pool->num_workers_done = 0;
  for (i = 0; i < worker_pool->num_workers; ++i) {
    ret = sem_post(&(worker_pool->thread_args[i].sem_parse_optimize_run));
    if (ret != 0) 
      return -1;
  }

  return 0;
}

/* Outside synchronization control for the whole query, set to wait the query.*/
int worker_pool_wait_query(worker_pool_t *worker_pool)
{
  int ret;
  if (worker_pool->num_workers == 0)
    return 0;

  ret = sem_wait(&worker_pool->sem_workers_done);
  if (ret != 0) 
    return -1;

  return 0;
}

/**
  Inside synchronization control for the tasks pair execution, set to start the
  task, this will be invoked in every tasks pair execution.
*/
int worker_pool_begin_task(worker_pool_t *worker_pool)
{
  int i, ret;
  if (worker_pool->num_workers == 0)
    return 0;

  worker_pool->num_workers_done = 0;
  for (i = 0; i < worker_pool->num_workers; ++i) {
    ret = sem_post(&(worker_pool->thread_args[i].sem_task_run));
    if (ret != 0) 
      return -1;
  }

  return 0;
}

/* Inside synchronization control for the each task pair, set to wait the query. */
int worker_pool_wait_task(worker_pool_t *worker_pool)
{
  int ret;
  if (worker_pool->num_workers == 0)
    return 0;

  ret = sem_wait(&worker_pool->sem_workers_done);
  if (ret != 0) 
    return -1;

  return 0;
}

/**
  When inside synchronization finished, set finished to 1 and all workers jump
  out of loop, every worker begin to cleanup in each. 
*/
int worker_pool_destroy(worker_pool_t *worker_pool)
{
  int i,result = 0;

  assert (worker_pool->finished == 0);

  worker_pool->num_workers = worker_pool->num_threads;
  worker_pool->num_workers_done = 0;
    
  for (i = 0; i < worker_pool->num_threads; ++i) {
    worker_pool->finished = 1;
    sem_post(&worker_pool->thread_args[i].sem_task_run);
  }

  sem_wait(&worker_pool->sem_workers_done);

  return result;
}

/* Cleanup worker pool. */
int worker_pool_cleanup(worker_pool_t *worker_pool)
{
  sem_destroy(&worker_pool->sem_workers_done);
  free (worker_pool->args);
  free (worker_pool->threads);
  free (worker_pool->thread_args);
  free (worker_pool);
  return false;
}

/* Wait other workers finish their parse and optimize work. */
int worker_pool_optimize_end(worker_thread_arg* arg)
{
  int ret, num_workers_done = 0;
  num_workers_done = fetch_and_inc(arg->num_workers_done) + 1;
  // All workers have done.
  if (num_workers_done == *arg->num_workers)
    ret = sem_post(arg->sem_workers_done);
  if (ret != 0) return -1;
  return 0;
}

/** 
  The thread function which will execute parse and optimize step, it belongs to
  outside logic, the function jump out when finshed set to 1.
*/
static void* worker_func_inner(void *arg)
{
  worker_func thread_func;
  int num_workers_done = 0;
  worker_thread_arg *thread_arg = (worker_thread_arg *) arg;
  void *thread_func_arg;

  while (true) { // Execute once.
    sem_wait (&thread_arg->sem_parse_optimize_run);
    if (*thread_arg->finished)
      break;

    thread_func = *(thread_arg->thread_func);
    thread_func_arg = *(thread_arg->thread_func_arg);

    // Only worked for parse and optimize.
    (*thread_func)(thread_func_arg);

    num_workers_done = fetch_and_inc(thread_arg->num_workers_done) + 1;
    if (num_workers_done == *thread_arg->num_workers) {
      // All workers have done.
      sem_post(thread_arg->sem_workers_done);
    }
  }

  sem_destroy (&thread_arg->sem_parse_optimize_run);
  num_workers_done = fetch_and_inc(thread_arg->num_workers_done) + 1;
  if (num_workers_done == *thread_arg->num_workers) {
    // All workers have done.
    sem_post(thread_arg->sem_workers_done);
  }

  return nullptr;
}

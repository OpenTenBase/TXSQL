/* Copyright (C) 2012 Monty Program Ab

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
   USA */

#include <atomic>

#include <time.h>
#include "my_sys.h"
#include "my_systime.h"
#include "mysql/psi/mysql_socket.h"
#include "mysql/thread_pool_priv.h"  // thd_is_transaction_active()
#include "sql/debug_sync.h"
#include "sql/item.h"
#include "sql/log.h"
#include "sql/mysqld.h"
#include "sql/mysqld_thd_manager.h"
#include "sql/protocol_classic.h"
#include "sql/sql_class.h"
#include "sql/sql_connect.h"
#include "sql/sql_plist.h"
#include "sql/threadpool.h"
#include "violite.h"
#ifdef __linux__
#include <sys/epoll.h>
bool threadpool_workaround_epoll_bug;
typedef struct epoll_event native_event;
#endif
#if defined(__FreeBSD__) || defined(__APPLE__)
#include <sys/event.h>
typedef struct kevent native_event;
#endif
#if defined(__sun)
#include <port.h>
typedef port_event_t native_event;
#endif

#include "sql/conn_handler/connection_handler_impl.h"

/** Maximum number of native events a listener can read in one go */
#define MAX_EVENTS 1024

/** Define if wait_begin() should create threads if necessary without waiting
for stall detection to kick in */
#define THREADPOOL_CREATE_THREADS_ON_WAIT

/* Possible values for thread_pool_high_prio_mode */
const char *threadpool_high_prio_mode_names[] = {"transactions", "statements",
                                                 "none", NullS};

/** Indicates that threadpool was initialized*/
static bool threadpool_started = false;

/** Max wait times before timer_thread exit. */
#define MAX_WAIT_TIMES 10

/*
  Define PSI Keys for performance schema.
  We have a mutex per group, worker threads, condition per worker thread,
  and timer thread  with its own mutex and condition.
*/

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_group_mutex;
static PSI_mutex_key key_timer_mutex;
static PSI_mutex_info mutex_list[] = {
    {&key_group_mutex, "group_mutex", 0, 0, PSI_DOCUMENT_ME},
    {&key_timer_mutex, "timer_mutex", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

static PSI_cond_key key_worker_cond;
static PSI_cond_key key_timer_cond;
static PSI_cond_info cond_list[] = {
    {&key_worker_cond, "worker_cond", 0, 0, PSI_DOCUMENT_ME},
    {&key_timer_cond, "timer_cond", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

static PSI_thread_key key_worker_thread;
static PSI_thread_key key_timer_thread;
static PSI_thread_info thread_list[] = {
    {&key_worker_thread, "worker_thread", "worker_th", 0, 0, PSI_DOCUMENT_ME},
    {&key_timer_thread, "timer_thread", "timer_th", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};
#endif  // HAVE_PSI_INTERFACE

struct thread_group_t;

/* Per-thread structure for workers */
struct worker_thread_t {
  ulonglong event_count; /* number of request handled by this thread */
  thread_group_t *thread_group;
  worker_thread_t *next_in_list;
  worker_thread_t **prev_in_list;

  mysql_cond_t cond;
  bool woken;
};

typedef I_P_List<
    worker_thread_t,
    I_P_List_adapter<worker_thread_t, &worker_thread_t::next_in_list,
                     &worker_thread_t::prev_in_list>,
    I_P_List_counter>
    worker_list_t;

struct connection_t {
  THD *thd;
  thread_group_t *thread_group;
  connection_t *next_in_queue;
  connection_t **prev_in_queue;
  ulonglong when_enqueued;
  ulonglong abs_wait_timeout;
  bool logged_in;
  bool bound_to_poll_descriptor;
  bool waiting;
  uint tickets;
  bool dump_thread;
  bool from_per_thread;
#ifdef HAVE_TDSQL
  int recursion_tp_wait_beginNum;
#endif /* HAVE_TDSQL */
};

typedef I_P_List<connection_t,
                 I_P_List_adapter<connection_t, &connection_t::next_in_queue,
                                  &connection_t::prev_in_queue>,
                 I_P_List_counter, I_P_List_fast_push_back<connection_t>>
    connection_queue_t;

struct alignas(128) thread_group_t {
  mysql_mutex_t mutex;
  connection_queue_t queue;
  connection_queue_t high_prio_queue;
  worker_list_t waiting_threads;
  worker_thread_t *listener;
  pthread_attr_t *pthread_attr;
  int pollfd;
  int thread_count;
  int active_thread_count;
  int connection_count;
  int waiting_thread_count;
  int dump_thread_count;
  /* Stats for the deadlock detection timer routine.*/
  int io_event_count;
  int queue_event_count;
  ulonglong last_thread_creation_time;
  int shutdown_pipe[2];
  bool shutdown;
  bool stalled;

  /* NO. of times a thread is created in the pool */
  ulonglong create_thread_num;

  /* NO. of times a thread is waken up */
  ulonglong wake_thread_num;

  /**
    NO. of times a worker thread finds that too
    many threads in the pool are running and thus decides not to pickup an event
    to work on.
  */
  ulonglong oversubscribed_num;

  /**
    NO. of times worker threads in the pool
    wait for another chance to work on an connection/event. Note that a worker
    thread may decide to not work on them if the thread pool is oversubscribed
  */
  ulonglong mysql_cond_timedwait_num;

  /**
    NO. of times the timer thread finds that there is no listener and no events
    are picked from any connections of this thread group.
  */
  ulonglong check_stall_nolistener;

  /**
    NO. of times the timer thread finds that there is no connections picked from
    this group's work queue by any worker threads.
  */
  ulonglong check_stall_stall;

  /* total NO. of times connections are picked from high-prio queue */
  ulonglong get_high_prio_queue_num;

  /* total NO. of times connections are picked from normal queue */
  ulonglong get_normal_queue_num;

  /**
    max time(in microsecs) a client's request stays in the queue. a good
    threadpool scheduler should keep this small.
  */
  std::atomic<ulonglong> max_req_latency_us;

  /*Total NO. of connections killed because no req sent from client. */
  std::atomic<ulonglong> conns_timeout_killed;

  /**
    NO. of connections moved into/out of the thread group, because too many
    active threads running in it/ in some other group.
  */
  ulonglong connections_moved_in;
  ulonglong connections_moved_out;

  /**
    NO. of connections moved from/to Per_thread.
  */
  ulonglong connections_moved_from_per_thread;
  ulonglong connections_moved_to_per_thread;

  /**
    Events consumed by this group, reset when 'show threadpool status'.
  */
  ulonglong events_consumed;

  /**
    Total waiting time of events after enqueue, reset when
    'show threadpool status'.
  */
  ulonglong total_usecs_in_queue;

#ifdef HAVE_TDSQL
  ulonglong oversubscribed_paral_num;
  char padding[192];
#else
  char padding[200];
#endif /* HAVE_TDSQL */
};

static_assert(sizeof(thread_group_t) == 512,
              "sizeof(thread_group_t) must be 512 to avoid false sharing");

static thread_group_t all_groups[MAX_THREAD_GROUPS];
static uint group_count;

/**
 Used for printing "pool blocked" message, see
 print_pool_blocked_message();
*/
static ulonglong pool_block_start;

/* Global timer for all groups  */
struct pool_timer_t {
  mysql_mutex_t mutex;
  mysql_cond_t cond;
  std::atomic<uint64> current_microtime;
  std::atomic<uint64> next_timeout_check;
  int tick_interval;
  bool shutdown;
  bool exit;
};

static pool_timer_t pool_timer;

static void queue_put(thread_group_t *thread_group, connection_t *connection);
static int wake_thread(thread_group_t *thread_group) noexcept;
static void handle_event(connection_t *connection);
static int wake_or_create_thread(thread_group_t *thread_group,
                                 bool admin_connection = false);
static int create_worker(thread_group_t *thread_group,
                         bool admin_connection = false) noexcept;
static void *worker_main(void *param);
static void check_stall(thread_group_t *thread_group);
static void connection_abort(connection_t *connection);
static void set_next_timeout_check(uint64 abstime);
static void print_pool_blocked_message(bool) noexcept;

/**
 Asynchronous network IO.

 We use native edge-triggered network IO multiplexing facility.
 This maps to different APIs on different Unixes.

 Supported are currently Linux with epoll, Solaris with event ports,
 OSX and BSD with kevent. All those API's are used with one-shot flags
 (the event is signalled once client has written something into the socket,
 then socket is removed from the "poll-set" until the  command is finished,
 and we need to re-arm/re-register socket)

 No implementation for poll/select/AIO is currently provided.

 The API closely resembles all of the above mentioned platform APIs
 and consists of following functions.

 - io_poll_create()
 Creates an io_poll descriptor
 On Linux: epoll_create()

 - io_poll_associate_fd(int poll_fd, int fd, void *data)
 Associate file descriptor with io poll descriptor
 On Linux : epoll_ctl(..EPOLL_CTL_ADD))

 - io_poll_disassociate_fd(int pollfd, int fd)
  Associate file descriptor with io poll descriptor
  On Linux: epoll_ctl(..EPOLL_CTL_DEL)


 - io_poll_start_read(int poll_fd,int fd, void *data)
 The same as io_poll_associate_fd(), but cannot be used before
 io_poll_associate_fd() was called.
 On Linux : epoll_ctl(..EPOLL_CTL_MOD)

 - io_poll_wait (int pollfd, native_event *native_events, int maxevents,
   int timeout_ms)

 wait until one or more descriptors added with io_poll_associate_fd()
 or io_poll_start_read() becomes readable. Data associated with
 descriptors can be retrieved from native_events array, using
 native_event_get_userdata() function.


 On Linux: epoll_wait()
*/

#if defined(__linux__)
#ifndef EPOLLRDHUP
/* Early 2.6 kernel did not have EPOLLRDHUP */
#define EPOLLRDHUP 0
#endif
static int io_poll_create() noexcept { return epoll_create(1); }

static int io_poll_associate_fd(int pollfd, int fd, void *data) noexcept {
  struct epoll_event ev;
  ev.data.u64 = 0; /* Keep valgrind happy */
  ev.data.ptr = data;
  ev.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLRDHUP | EPOLLONESHOT;
  return epoll_ctl(pollfd, EPOLL_CTL_ADD, fd, &ev);
}

static int io_poll_start_read(int pollfd, int fd, void *data) noexcept {
  struct epoll_event ev;
  ev.data.u64 = 0; /* Keep valgrind happy */
  ev.data.ptr = data;
  ev.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLRDHUP | EPOLLONESHOT;
  if (threadpool_workaround_epoll_bug)
  {
    /*
      On buggy kernel, replace EPOLL_CTL_MOD with EPOLL_CTL_DEL/ADD.
     */
    int rc = epoll_ctl(pollfd, EPOLL_CTL_DEL, fd, NULL);
    return (rc) ? rc : epoll_ctl(pollfd, EPOLL_CTL_ADD, fd, &ev);
  }
  return epoll_ctl(pollfd, EPOLL_CTL_MOD, fd, &ev);
}

static int io_poll_disassociate_fd(int pollfd, int fd) noexcept {
  struct epoll_event ev;
  return epoll_ctl(pollfd, EPOLL_CTL_DEL, fd, &ev);
}

/*
 Wrapper around epoll_wait.
 NOTE - in case of EINTR, it restarts with original timeout. Since we use
 either infinite or 0 timeouts, this is not critical
*/
static int io_poll_wait(int pollfd, native_event *native_events, int maxevents,
                        int timeout_ms) noexcept {
  int ret;
  do {
    ret = epoll_wait(pollfd, native_events, maxevents, timeout_ms);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

static void *native_event_get_userdata(native_event *event) noexcept {
  return event->data.ptr;
}

#elif defined(__FreeBSD__) || defined(__APPLE__)
static int io_poll_create() noexcept { return kqueue(); }

static int io_poll_start_read(int pollfd, int fd, void *data) noexcept {
  struct kevent ke;
  EV_SET(&ke, fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, data);
  return kevent(pollfd, &ke, 1, 0, 0, 0);
}

static int io_poll_associate_fd(int pollfd, int fd, void *data) noexcept {
  struct kevent ke;
  EV_SET(&ke, fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, data);
  return io_poll_start_read(pollfd, fd, data);
}

static int io_poll_disassociate_fd(int pollfd, int fd) noexcept {
  struct kevent ke;
  EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
  return kevent(pollfd, &ke, 1, 0, 0, 0);
}

static int io_poll_wait(int pollfd, struct kevent *events, int maxevents,
                        int timeout_ms) noexcept {
  struct timespec ts;
  int ret;
  if (timeout_ms >= 0) {
    ts.tv_sec = timeout_ms / 1000;
    ts.tv_nsec = (timeout_ms % 1000) * 1000000;
  }
  do {
    ret = kevent(pollfd, 0, 0, events, maxevents,
                 (timeout_ms >= 0) ? &ts : nullptr);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

static void *native_event_get_userdata(native_event *event) noexcept {
  return event->udata;
}

#elif defined(__sun)

static int io_poll_create() noexcept { return port_create(); }

static int io_poll_start_read(int pollfd, int fd, void *data) noexcept {
  return port_associate(pollfd, PORT_SOURCE_FD, fd, POLLIN, data);
}

static int io_poll_associate_fd(int pollfd, int fd, void *data) noexcept {
  return io_poll_start_read(pollfd, fd, data);
}

static int io_poll_disassociate_fd(int pollfd, int fd) noexcept {
  return port_dissociate(pollfd, PORT_SOURCE_FD, fd);
}

static int io_poll_wait(int pollfd, native_event *events, int maxevents,
                        int timeout_ms) noexcept {
  struct timespec ts;
  int ret;
  uint_t nget = 1;
  if (timeout_ms >= 0) {
    ts.tv_sec = timeout_ms / 1000;
    ts.tv_nsec = (timeout_ms % 1000) * 1000000;
  }
  do {
    ret = port_getn(pollfd, events, maxevents, &nget,
                    (timeout_ms >= 0) ? &ts : nullptr);
  } while (ret == -1 && errno == EINTR);
  assert(nget < INT_MAX);
  return (int)nget;
}

static void *native_event_get_userdata(native_event *event) noexcept {
  return event->portev_user;
}
#else
#error not ported yet to this OS
#endif

void tp_change_active_thread(THD *thd, int command, bool inc)
{
  DBUG_ENTER("tp_change_active_thread");
  assert(command == COM_BINLOG_DUMP_GTID || command == COM_BINLOG_DUMP ||
         command == SQLCOM_PURGE || command == SQLCOM_PURGE_BEFORE);
  connection_t *con = (connection_t *)thd->event_scheduler.data;
  if (con)
  {
    thread_group_t *group = con->thread_group;
    mysql_mutex_lock(&group->mutex);
    if (inc)
    {
      group->dump_thread_count--;
      con->dump_thread = false;
      group->active_thread_count++;
    }
    else
    {
      group->dump_thread_count++;
      con->dump_thread = true;
      group->active_thread_count--;
    }
    mysql_mutex_unlock(&group->mutex);
  }
  DBUG_VOID_RETURN;
}

namespace {

/*
  Prevent too many active threads executing at the same time, if the workload is
  not CPU bound.
*/

inline bool too_many_active_threads(
    const thread_group_t &thread_group) noexcept {
  return (thread_group.active_thread_count >=
              1 + (int)threadpool_oversubscribe &&
          !thread_group.stalled);
}

#ifdef HAVE_TDSQL
/**
   Whether the high priority or normal queue is congested --- the 1st message
   has been waiting for too long (longer than
   threadpool_queue_congest_req_timeout milli seconds) or the queue is overly
   long(longer than threadpool_queue_congest_threshold)
*/
static inline bool req_queue_congested(thread_group_t *tg) {
  /**
     We use threadpool_oversubscribe_parall for tdsql-mariadb-10.0.10's eager
     mode as before. the thread_pool_eager_mode in tdsql-mariadb-10.1.9
     controls completly new working behaviors.
  */
  if (threadpool_oversubscribe_parall) {
    ulonglong cur = 0;
    connection_t *c1 = NULL, *c2 = NULL;
    if (tg->queue.elements() > threadpool_queue_congest_threshold ||
        tg->high_prio_queue.elements() > threadpool_queue_congest_threshold ||
        ((cur = my_microsecond_getsystime()) &&
         (((c1 = tg->queue.front()) &&
           cur >= (c1->when_enqueued +
                   threadpool_queue_congest_req_timeout * 1000)) ||
          ((c2 = tg->high_prio_queue.front()) &&
           cur >= (c2->when_enqueued +
                   threadpool_queue_congest_req_timeout * 1000))))) {
      ++tg->oversubscribed_paral_num;
      return true;
    }
  }

  return false;
}

static inline bool canActiveMoreThread(thread_group_t *tg) {
  if (threadpool_eager_mode) {
    /* The eager mode may create more thread */
    return (tg->thread_count < tg->connection_count + 1);
  } else {
    return (!tg->active_thread_count ||
            (tg->active_thread_count <
                 (int)threadpool_oversubscribe_extra_threads &&
             req_queue_congested(tg)));
  }
}
#else /* HAVE_TDSQL */
/*
  Limit the number of 'busy' threads by 1 + thread_pool_oversubscribe. A thread
  is busy if it is in either the active state or the waiting state (i.e. between
  thd_wait_begin() / thd_wait_end() calls).
*/
inline bool too_many_busy_threads(const thread_group_t &thread_group) noexcept {
  return (thread_group.active_thread_count + thread_group.waiting_thread_count >=
          1 + (int)threadpool_oversubscribe);
}
#endif /* HAVE_TDSQL */

/*
   Checks if a given connection is eligible to enter the high priority queue
   based on its current thread_pool_high_prio_mode value, available high
   priority tickets and transactional state and whether any locks are held.
*/

inline bool connection_is_high_prio(const connection_t &c) noexcept {
  const ulong mode = c.thd->variables.threadpool_high_prio_mode;

  return (mode == TP_HIGH_PRIO_MODE_STATEMENTS) ||
         (mode == TP_HIGH_PRIO_MODE_TRANSACTIONS && c.tickets > 0 &&
          (thd_is_transaction_active(c.thd) ||
           c.thd->variables.option_bits & OPTION_TABLE_LOCK ||
           c.thd->locked_tables_mode != LTM_NONE ||
           c.thd->mdl_context.has_locks() ||
           c.thd->global_read_lock.is_acquired() ||
           c.thd->mdl_context.has_locks(MDL_key::USER_LEVEL_LOCK) ||
           c.thd->mdl_context.has_locks(MDL_key::LOCKING_SERVICE)));
}

}  // namespace

/* Dequeue element from a workqueue */

static connection_t *queue_get(thread_group_t *thread_group) noexcept {
  DBUG_ENTER("queue_get");
  thread_group->queue_event_count++;
  connection_t *c = nullptr;

#ifdef HAVE_TDSQL
  if ((c = thread_group->high_prio_queue.front())) {
    thread_group->get_high_prio_queue_num++;
    thread_group->high_prio_queue.remove(c);
  } else if ((c = thread_group->queue.front())) {
    thread_group->get_normal_queue_num++;
    thread_group->queue.remove(c);
  }
#else  /* HAVE_TDSQL */
  if ((c = thread_group->high_prio_queue.front())) {
    thread_group->high_prio_queue.remove(c);
    thread_group->get_high_prio_queue_num++;
  }
  /*
    Don't pick events from the low priority queue if there are too many
    active + waiting threads.
  */
  else if (!too_many_busy_threads(*thread_group) &&
           (c = thread_group->queue.front())) {
    thread_group->queue.remove(c);
    thread_group->get_normal_queue_num++;
  }
#endif /* HAVE_TDSQL */

  DBUG_RETURN(c);
}

class Thd_timeout_checker : public Do_THD_Impl {
 private:
  pool_timer_t *const m_timer;

 public:
  Thd_timeout_checker(pool_timer_t *timer) noexcept : m_timer(timer) {}

  virtual ~Thd_timeout_checker() {}

  virtual void operator()(THD *thd) noexcept override {
    if (thd_get_net_read_write(thd) != 1) return;

    connection_t *connection = (connection_t *)thd->event_scheduler.data;
    if (!connection) return;

    if (connection->abs_wait_timeout <
        m_timer->current_microtime.load(std::memory_order_relaxed)) {
      /* Wait timeout exceeded, kill connection. */
      mysql_mutex_lock(&thd->LOCK_thd_data);
      thd->killed = THD::KILL_CONNECTION;
      tp_post_kill_notification(thd);
      mysql_mutex_unlock(&thd->LOCK_thd_data);
      connection->thread_group->conns_timeout_killed++;
    } else {
      set_next_timeout_check(connection->abs_wait_timeout);
    }
  }
};

/*
  Handle wait timeout :
  Find connections that have been idle for too long and kill them.
  Also, recalculate time when next timeout check should run.
*/

static void timeout_check(pool_timer_t *timer) {
  DBUG_TRACE;

  /* Reset next timeout check, it will be recalculated in the loop below */
  timer->next_timeout_check.store(ULLONG_MAX, std::memory_order_relaxed);

  Thd_timeout_checker thd_timeout_checker(timer);
  Global_THD_manager::get_instance()->do_for_all_thd_copy(&thd_timeout_checker);

  return;
}

/*
 Timer thread.

  Periodically, check if one of the thread groups is stalled. Stalls happen if
  events are not being dequeued from the queue, or from the network, Primary
  reason for stall can be a lengthy executing non-blocking request. It could
  also happen that thread is waiting but wait_begin/wait_end is forgotten by
  storage engine. Timer thread will create a new thread in group in case of
  a stall.

  Besides checking for stalls, timer thread is also responsible for terminating
  clients that have been idle for longer than wait_timeout seconds.

  TODO: Let the timer sleep for long time if there is no work to be done.
  Currently it wakes up rather often on and idle server.
*/

static void *timer_thread(void *param) noexcept {
  my_thread_init();
  DBUG_TRACE;

  pool_timer_t *timer = (pool_timer_t *)param;
  timer->next_timeout_check.store(ULLONG_MAX, std::memory_order_relaxed);
  timer->current_microtime.store(my_microsecond_getsystime(),
                                 std::memory_order_relaxed);

  for (;;) {
    struct timespec ts;

    set_timespec_nsec(&ts, timer->tick_interval * 1000000ULL);
    mysql_mutex_lock(&timer->mutex);
    int err = mysql_cond_timedwait(&timer->cond, &timer->mutex, &ts);
    if (timer->shutdown) {
      mysql_mutex_unlock(&timer->mutex);
      break;
    }
    if (err == ETIMEDOUT) {
      timer->current_microtime.store(my_microsecond_getsystime(),
                                     std::memory_order_relaxed);

      /* Check stalls in thread groups */
      for (size_t i = 0; i < array_elements(all_groups); i++) {
        if (all_groups[i].connection_count) check_stall(&all_groups[i]);
      }

      /* Check if any client exceeded wait_timeout */
      if (timer->next_timeout_check.load(std::memory_order_relaxed) <=
          timer->current_microtime.load(std::memory_order_relaxed))
        timeout_check(timer);
    }
    mysql_mutex_unlock(&timer->mutex);
  }

  mysql_mutex_destroy(&timer->mutex);
  my_thread_end();
  timer->exit = true;
  return NULL;
}

/*
  Check if both the high and low priority queues are empty.

  NOTE: we also consider the low priority queue empty in case it has events, but
  they cannot be processed due to the too_many_busy_threads() limit in
  txsql mode or due to too_many_active_threads() limit in tdsql mode.
*/
static bool queues_are_empty(const thread_group_t &tg) noexcept {
#ifdef HAVE_TDSQL
  return (tg.high_prio_queue.is_empty() && tg.queue.is_empty());
#else  /* HAVE_TDSQL */
  return (tg.high_prio_queue.is_empty() &&
          (tg.queue.is_empty() || too_many_busy_threads(tg)));
#endif /* HAVE_TDSQL */
}

static void check_stall(thread_group_t *thread_group) {
  if (mysql_mutex_trylock(&thread_group->mutex) != 0) {
    /* Something happens. Don't disturb */
    return;
  }

  /*
    Check if listener is present. If not,  check whether any IO
    events were dequeued since last time. If not, this means
    listener is either in tight loop or thd_wait_begin()
    was forgotten. Create a new worker(it will make itself listener).
  */
#ifdef HAVE_TDSQL
  if (!thread_group->listener && !thread_group->io_event_count) {
    thread_group->check_stall_nolistener++;
    wake_or_create_thread(thread_group);

    if (!threadpool_eager_mode) {
      mysql_mutex_unlock(&thread_group->mutex);
      return;
    }
  }
#else  /* HAVE_TDSQL */
  if (!thread_group->listener && !thread_group->io_event_count) {
    thread_group->check_stall_nolistener++;
    wake_or_create_thread(thread_group);
    mysql_mutex_unlock(&thread_group->mutex);
    return;
  }
#endif /* HAVE_TDSQL */

  /*  Reset io event count */
  thread_group->io_event_count = 0;

  /*
    Check whether requests from the workqueues are being dequeued.

    The stall detection and resolution works as follows:

    1. There is a counter thread_group->queue_event_count for the number of
       events removed from the queues. Timer resets the counter to 0 on each
    run.
    2. Timer determines stall if this counter remains 0 since last check
       and at least one of the high and low priority queues is not empty.
    3. Once timer determined a stall it sets thread_group->stalled flag and
       wakes and idle worker (or creates a new one, subject to throttling).
    4. The stalled flag is reset, when an event is dequeued.

    Q : Will this handling lead to an unbound growth of threads, if queues
    stall permanently?
    A : No. If queues stall permanently, it is an indication for many very long
    simultaneous queries. The maximum number of simultanoues queries is
    max_connections, further we have threadpool_max_threads limit, upon which no
    worker threads are created. So in case there is a flood of very long
    queries, threadpool would slowly approach thread-per-connection behavior.
    NOTE:
    If long queries never wait, creation of the new threads is done by timer,
    so it is slower than in real thread-per-connection. However if long queries
    do wait and indicate that via thd_wait_begin/end callbacks, thread creation
    will be faster.
  */
  if (!thread_group->queue_event_count && !queues_are_empty(*thread_group)) {
    thread_group->check_stall_stall++;
    thread_group->stalled = true;

#ifdef HAVE_TDSQL
        /**
      If too many active threads, the wakenup or newly-created thread will not
      do anything but sleep and exit, which is a waste of system resources. So
      don't wake or create in this case, each of the currently running active
      thread will pick up next job when it finishes its current one.

      Still wake up just in case the thread pool stalls --- this is most
      important.

      In eager mode, wake up 5 sleeping threads before creating a new one.
    */
    int num_woken = 0;
    if (threadpool_eager_mode) {
      if (thread_group->connection_count + 1 > thread_group->thread_count) {
        int needWakeOrCreate =
            thread_group->connection_count + 1 - thread_group->thread_count;
        while (needWakeOrCreate > 0) {
          if (wake_thread(thread_group)) {
            break;
          }

          ++num_woken;
          --needWakeOrCreate;
        }
      }
    }

    if (num_woken == 0) {
      wake_or_create_thread(thread_group);
    }
#else  /* HAVE_TDSQL */
    wake_or_create_thread(thread_group);
#endif /* HAVE_TDSQL */
  }

  /* Reset queue event count */
  thread_group->queue_event_count = 0;

  mysql_mutex_unlock(&thread_group->mutex);
}

static bool start_timer(pool_timer_t *timer) noexcept {
  my_thread_handle thread_id;
  DBUG_TRACE;
  mysql_mutex_init(key_timer_mutex, &timer->mutex, NULL);
  mysql_cond_init(key_timer_cond, &timer->cond);
  timer->shutdown = false;
  if (mysql_thread_create(key_timer_thread, &thread_id, NULL, timer_thread, timer))
    return true;
  return false;
}

static void stop_timer(pool_timer_t *timer) noexcept {
  DBUG_TRACE;
  mysql_mutex_lock(&timer->mutex);
  timer->shutdown = true;
  mysql_cond_signal(&timer->cond);
  mysql_mutex_unlock(&timer->mutex);

#ifndef NDEBUG
  /* Wait until timer thread has stopped. */
  int wait_times = 0;
  while (timer->exit == false && wait_times < MAX_WAIT_TIMES) {
    wait_times++;
    /* Wait 1/5 of timer->tick_interval each time. */
    usleep(timer->tick_interval * 1000ULL / 5);
  }
#endif

  return;
}

/**
  Poll for socket events and distribute them to worker threads
  In many case current thread will handle single event itself.

  @return a ready connection, or NULL on shutdown
*/
static connection_t *listener(thread_group_t *thread_group) {
  DBUG_TRACE;
  connection_t *retval = nullptr;

  for (;;) {
    if (thread_group->shutdown) break;

    native_event ev[MAX_EVENTS];
    int cnt = io_poll_wait(thread_group->pollfd, ev, MAX_EVENTS, -1);

    if (cnt <= 0) {
      assert(thread_group->shutdown);
      break;
    }

    mysql_mutex_lock(&thread_group->mutex);

    if (thread_group->shutdown) {
      mysql_mutex_unlock(&thread_group->mutex);
      break;
    }

    thread_group->io_event_count += cnt;

    /*
     We got some network events and need to make decisions : whether
     listener  hould handle events and whether or not any wake worker
     threads so they can handle events.

     Q1 : Should listener handle an event itself, or put all events into
     queue  and let workers handle the events?

     Solution :
     Generally, listener that handles events itself is preferable. We do not
     want listener thread to change its state from waiting  to running too
     often, Since listener has just woken from poll, it better uses its time
     slice and does some work. Besides, not handling events means they go to
     the  queue, and often to wake another worker must wake up to handle the
     event. This is not good, as we want to avoid wakeups.

     The downside of listener that also handles queries is that we can
     potentially leave thread group  for long time not picking the new
     network events. It is not  a major problem, because this stall will be
     detected  sooner or later by  the timer thread. Still, relying on timer
     is not always good, because it may "tick" too slow (large timer_interval)

     We use following strategy to solve this problem - if queue was not empty
     we suspect flood of network events and listener stays, Otherwise, it
     handles a query.


     Q2: If queue is not empty, how many workers to wake?

     Solution:
     We generally try to keep one thread per group active (threads handling
     queries   are considered active, unless they stuck in inside some "wait")
     Thus, we will wake only one worker, and only if there is not active
     threads currently,and listener is not going to handle a query. When we
     don't wake, we hope that  currently active  threads will finish fast and
     handle the queue. If this does  not happen, timer thread will detect stall
     and wake a worker.

     NOTE: Currently nothing is done to detect or prevent long queuing times.
     A solutionc for the future would be to give up "one active thread per
     group" principle, if events stay  in the queue for too long, and just wake
     more workers.
    */

    const bool listener_picks_event =
        thread_group->high_prio_queue.is_empty() &&
        thread_group->queue.is_empty();

    /*
      If listener_picks_event is set, listener thread will handle first event,
      and put the rest into the queue. If listener_pick_event is not set, all
      events go to the queue.
    */
    for (int i = (listener_picks_event) ? 1 : 0; i < cnt; i++) {
      connection_t *c = (connection_t *)native_event_get_userdata(&ev[i]);
      c->when_enqueued = my_micro_time();
      if (connection_is_high_prio(*c)) {
        c->tickets--;
        thread_group->high_prio_queue.push_back(c);
      } else {
        c->tickets = c->thd->variables.threadpool_high_prio_tickets;
        thread_group->queue.push_back(c);
      }
    }

#ifdef HAVE_TDSQL
        int canWakeNum = 0;
    if (threadpool_eager_mode || threadpool_listen_eager_mode) {
      if ((int)threadpool_oversubscribe > thread_group->active_thread_count) {
        canWakeNum =
            threadpool_oversubscribe - thread_group->active_thread_count;
      }
    } else {
      if (threadpool_oversubscribe_parall &&
          (int)threadpool_oversubscribe_extra_threads >
              thread_group->active_thread_count) {
        canWakeNum = threadpool_oversubscribe_extra_threads -
                     thread_group->active_thread_count;
      }
    }

    /* If we got >1 jobs, try to wake up enough (but not too many) threads to
    work on them. */
    if (listener_picks_event) {
      --cnt;
    }

    if (canWakeNum > cnt) {
      canWakeNum = cnt;
    } else if (canWakeNum < 0) {
      /* threadpool_oversubscribe/threadpool_oversubscribe_extra_threads may
      change on fly. */
      canWakeNum = 0;
    }

    while (canWakeNum > 0) {
      /**
        If not enough threads, let's leave it to the timer thread to create
        threads because it's a bit expensive, and in a heavily loaded system
        such situation is rare.
      */
      if (wake_thread(thread_group)) {
        break;
      }

      --canWakeNum;
    }

    if (canWakeNum && canActiveMoreThread(thread_group)) {
      wake_or_create_thread(thread_group);
    }
#endif /* HAVE_TDSQL */

    if (listener_picks_event) {
      /* Handle the first event. */
      thread_group->get_normal_queue_num++;
      retval = (connection_t *)native_event_get_userdata(&ev[0]);
      retval->when_enqueued = 0;
      mysql_mutex_unlock(&thread_group->mutex);
      break;
    }

#ifndef HAVE_TDSQL
    if (thread_group->active_thread_count == 0) {
      /* We added some work items to queue, now wake a worker. */
      if (wake_thread(thread_group)) {
        /*
          Wake failed, hence groups has no idle threads. Now check if there are
          any threads in the group except listener.
        */
        if (thread_group->thread_count == 1) {
          /*
            Currently there is no worker thread in the group, as indicated by
            thread_count == 1 (this means listener is the only one thread in
            the group).
            The queue is not empty, and listener is not going to handle
            events. In order to drain the queue,  we create a worker here.
            Alternatively, we could just rely on timer to detect stall, and
            create thread, but waiting for timer would be an inefficient and
            pointless delay.
          */
          create_worker(thread_group);
        }
      }
    }
#endif /* !HAVE_TDSQL */
    mysql_mutex_unlock(&thread_group->mutex);
  }

  return retval;
}

/**
  Adjust thread counters in group or global
  whenever thread is created or is about to exit

  @param thread_group
  @param count -  1, when new thread is created
                 -1, when thread is about to exit
*/

static void add_thread_count(thread_group_t *thread_group,
                             int32 count) noexcept {
  thread_group->thread_count += count;
  /* worker starts out and end in "active" state */
  thread_group->active_thread_count += count;
  tp_stats.num_worker_threads.fetch_add(count, std::memory_order_relaxed);
}

/**
  Creates a new worker thread.
  thread_mutex must be held when calling this function

  NOTE: in rare cases, the number of threads can exceed
  threadpool_max_threads, because we need at least 2 threads
  per group to prevent deadlocks (one listener + one worker)
*/

static int create_worker(thread_group_t *thread_group,
                         bool admin_connection) noexcept {
  my_thread_handle thread_id;
  bool max_threads_reached = false;
  int err;

  DBUG_TRACE;
  if (!admin_connection &&
      tp_stats.num_worker_threads.load(std::memory_order_relaxed) >=
          (int)threadpool_max_threads &&
      thread_group->thread_count >= 2) {
    err = 1;
    max_threads_reached = true;
    goto end;
  }

  err = mysql_thread_create(key_worker_thread, &thread_id,
                            thread_group->pthread_attr, worker_main,
                            thread_group);
  if (!err) {
    thread_group->create_thread_num++;
    thread_group->last_thread_creation_time = my_microsecond_getsystime();
    Global_THD_manager::get_instance()->inc_thread_created();
    add_thread_count(thread_group, 1);
  } else {
    set_my_errno(errno);
  }

end:
  if (err)
    print_pool_blocked_message(max_threads_reached);
  else
    pool_block_start = 0; /* Reset pool blocked timer, if it was set */

  return err;
}

/**
 Calculate microseconds throttling delay for thread creation.

 The value depends on how many threads are already in the group:
 small number of threads means no delay, the more threads the larger
 the delay.

 The actual values were not calculated using any scientific methods.
 They just look right, and behave well in practice.

 TODO: Should throttling depend on thread_pool_stall_limit?
*/
static ulonglong microsecond_throttling_interval(
    const thread_group_t &thread_group) noexcept {
  const int count = thread_group.thread_count;

  if (count < 4) return 0;

  if (count < 8) return 50 * 1000;

  if (count < 16) return 100 * 1000;

  return 200 * 1000;
}

/**
  Wakes a worker thread, or creates a new one.

  Worker creation is throttled, so we avoid too many threads
  to be created during the short time.

  If admin_connection is true, a new thread is created regardless of any other
  limits.
*/
static int wake_or_create_thread(thread_group_t *thread_group,
                                 bool admin_connection) {
  DBUG_TRACE;

  if (thread_group->shutdown) return 0;

  if (wake_thread(thread_group) == 0) return 0;

  if (thread_group->thread_count > thread_group->connection_count) return -1;

#ifdef HAVE_TDSQL
  if (thread_group->active_thread_count == 0 || admin_connection ||
      threadpool_eager_mode) {
    /*
     We're better off creating a new thread here  with no delay, either there
     are no workers at all, or they all are all blocking and there was no
     idle  thread to wakeup. Smells like a potential deadlock or very slowly
     executing requests, e.g sleeps or user locks.
    */
    return create_worker(thread_group, admin_connection);
  }
#else /* HAVE_TDSQL */
  if (thread_group->active_thread_count == 0 || admin_connection) {
    /*
     We're better off creating a new thread here  with no delay, either there
     are no workers at all, or they all are all blocking and there was no
     idle  thread to wakeup. Smells like a potential deadlock or very slowly
     executing requests, e.g sleeps or user locks.
    */
    return create_worker(thread_group, admin_connection);
  }
#endif /* HAVE_TDSQL */

  const ulonglong now = my_microsecond_getsystime();
  const ulonglong time_since_last_thread_created =
      (now - thread_group->last_thread_creation_time);

  /* Throttle thread creation. */
  if (time_since_last_thread_created >
      microsecond_throttling_interval(*thread_group)) {
    return create_worker(thread_group);
  }

  return -1;
}

static int thread_group_init(thread_group_t *thread_group,
                             pthread_attr_t *thread_attr) noexcept {
  DBUG_TRACE;
  thread_group->pthread_attr = thread_attr;
  mysql_mutex_init(key_group_mutex, &thread_group->mutex, NULL);
  thread_group->pollfd = -1;
  thread_group->shutdown_pipe[0] = -1;
  thread_group->shutdown_pipe[1] = -1;
  thread_group->create_thread_num = 0;
  thread_group->oversubscribed_num = 0;
  thread_group->mysql_cond_timedwait_num = 0;
  thread_group->check_stall_nolistener = 0;
  thread_group->check_stall_stall = 0;
  thread_group->get_high_prio_queue_num = 0;
  thread_group->get_normal_queue_num = 0;
  thread_group->max_req_latency_us = 0;
  thread_group->conns_timeout_killed = 0;
  thread_group->connections_moved_in = 0;
  thread_group->connections_moved_out = 0;
  thread_group->connections_moved_from_per_thread = 0;
  thread_group->connections_moved_to_per_thread = 0;
  thread_group->events_consumed = 0;
#ifdef HAVE_TDSQL
  thread_group->oversubscribed_paral_num = 0;
#endif /* HAVE_TDSQL */

  return 0;
}

static void thread_group_destroy(thread_group_t *thread_group) noexcept {
  mysql_mutex_destroy(&thread_group->mutex);
  if (thread_group->pollfd != -1) {
    close(thread_group->pollfd);
    thread_group->pollfd = -1;
  }
  for (int i = 0; i < 2; i++) {
    if (thread_group->shutdown_pipe[i] != -1) {
      close(thread_group->shutdown_pipe[i]);
      thread_group->shutdown_pipe[i] = -1;
    }
  }
}

/**
  Wake sleeping thread from waiting list
*/

static int wake_thread(thread_group_t *thread_group) noexcept {
  DBUG_TRACE;
  worker_thread_t *thread = thread_group->waiting_threads.front();
  if (thread) {
    thread->woken = true;
    thread_group->wake_thread_num++;
    thread_group->waiting_threads.remove(thread);
    mysql_cond_signal(&thread->cond);
    return 0;
  }
  return 1; /* no thread in waiter list => missed wakeup */
}

/**
  Shutdown for thread group
*/

static void thread_group_close(thread_group_t *thread_group) noexcept {
  DBUG_TRACE;

  mysql_mutex_lock(&thread_group->mutex);
  if (thread_group->thread_count == 0) {
    mysql_mutex_unlock(&thread_group->mutex);
    thread_group_destroy(thread_group);
    return;
  }

  thread_group->shutdown = true;
  thread_group->listener = nullptr;

  if (pipe(thread_group->shutdown_pipe)) {
    mysql_mutex_unlock(&thread_group->mutex);
    return;
  }

  /* Wake listener */
  if (io_poll_associate_fd(thread_group->pollfd, thread_group->shutdown_pipe[0],
                           nullptr)) {
    mysql_mutex_unlock(&thread_group->mutex);
    return;
  }
  char c = 0;
  if (write(thread_group->shutdown_pipe[1], &c, 1) < 0) {
    mysql_mutex_unlock(&thread_group->mutex);
    return;
  }

  /* Wake all workers. */
  while (wake_thread(thread_group) == 0) {
  }

  mysql_mutex_unlock(&thread_group->mutex);

  mysql_mutex_lock(&thread_group->mutex);
  while (thread_group->thread_count > 0) {
    mysql_mutex_unlock(&thread_group->mutex);
    my_sleep(10000);
    mysql_mutex_lock(&thread_group->mutex);
  }
  mysql_mutex_unlock(&thread_group->mutex);

  thread_group_destroy(thread_group);

  return;
}

/*
  Add work to the queue. Maybe wake a worker if they all sleep.

  Currently, this function is only used when new connections need to
  perform login (this is done in worker threads).

*/

static void queue_put(thread_group_t *thread_group, connection_t *connection) {
  DBUG_TRACE;

  mysql_mutex_lock(&thread_group->mutex);
  connection->tickets = connection->thd->variables.threadpool_high_prio_tickets;
  connection->when_enqueued = my_micro_time();
  thread_group->queue.push_back(connection);

#ifdef HAVE_TDSQL
  if (!thread_group->listener || canActiveMoreThread(thread_group)) {
    wake_or_create_thread(thread_group, connection->thd->is_admin_connection());
  }
#else  /* HAVE_TDSQL */
  if (thread_group->active_thread_count == 0) {
    wake_or_create_thread(thread_group, connection->thd->is_admin_connection());
  }
#endif /* HAVE_TDSQL */

  mysql_mutex_unlock(&thread_group->mutex);

  return;
}

/**
  Retrieve a connection with pending event.

  Pending event in our case means that there is either a pending login request
  (if connection is not yet logged in), or there are unread bytes on the socket.

  If there are no pending events currently, thread will wait.
  If timeout specified in abstime parameter passes, the function returns NULL.

  @param current_thread - current worker thread
  @param thread_group - current thread group
  @param abstime - absolute wait timeout

  @return
  connection with pending event.
  NULL is returned if timeout has expired,or on shutdown.
*/

static connection_t *get_event(worker_thread_t *current_thread,
                               thread_group_t *thread_group,
                               struct timespec *abstime) {
  DBUG_TRACE;
  connection_t *connection = nullptr;
  int err = 0;

  mysql_mutex_lock(&thread_group->mutex);
  assert(thread_group->active_thread_count >= 0);

  for (;;) {
    const bool oversubscribed = too_many_active_threads(*thread_group);
    if (thread_group->shutdown) break;

#ifdef HAVE_TDSQL
    /* Check if queue is not empty */
    if (!oversubscribed || threadpool_eager_mode) {
      connection = queue_get(thread_group);
      if (connection) {
        /**
          If there is no listener, wake up one, it will likely become one if
          oversubscribed; if it doesn't, it will pick up more work to do and
          this piece of code will very quickly wake up a listener. We don't
          care if a thread is created or not, if it isn't, timer thread will
          do, and such situation is rare in a heavily loaded system.
        */
        if (!thread_group->listener) {
          wake_thread(thread_group);
        }

        break;
      }
    } else {
      if (oversubscribed) {
        thread_group->oversubscribed_num++;
      }
    }

    /* If there is  currently no listener in the group, become one. */
    if (!thread_group->listener) {
      thread_group->listener = current_thread;
      thread_group->active_thread_count--;
      mysql_mutex_unlock(&thread_group->mutex);

      connection = listener(thread_group);

      mysql_mutex_lock(&thread_group->mutex);
      thread_group->active_thread_count++;
      /* There is no listener anymore, it just returned. */
      thread_group->listener = nullptr;
      break;
    }

    /*
      Last thing we try before going to sleep is to
      pick a single event via epoll, without waiting (timeout 0)
    */
    if (!oversubscribed || threadpool_eager_mode) {
      native_event nev;
      if (io_poll_wait(thread_group->pollfd, &nev, 1, 0) == 1) {
        thread_group->io_event_count++;
        connection = (connection_t *)native_event_get_userdata(&nev);

        /*
          Since we are going to perform an out-of-order event processing for the
          connection, first check whether it is eligible for high priority
          processing. We can get here even if there are queued events, so it
          must either have a high priority ticket, or there must be not too many
          busy threads (as if it was coming from a low priority queue).
        */
        bool haveCompute = false;
        if (connection_is_high_prio(*connection)) {
          haveCompute = true;
          connection->tickets--;
          connection->when_enqueued = 0;
        } else if (!threadpool_eager_mode &&
                   too_many_active_threads(*thread_group)) {
          /*
            Not eligible for high priority processing. Restore tickets and put
            it into the low priority queue.
          */

          connection->tickets =
              connection->thd->variables.threadpool_high_prio_tickets;
          connection->when_enqueued = my_micro_time();
          thread_group->queue.push_back(connection);
          connection = nullptr;
        }

        if (connection) {
          if (!haveCompute) {
            thread_group->get_normal_queue_num++;
          }
          thread_group->queue_event_count++;
          break;
        }
      }
    }
#else  /* HAVE_TDSQL */
    /* Check if queue is not empty */
    if (!oversubscribed) {
      connection = queue_get(thread_group);
      if (connection) break;
    } else {
      thread_group->oversubscribed_num++;
    }

    /* If there is currently no listener in the group, become one. Do not
     * become listener when oversubscribed, because listener may choose
     * to handle the event itself, and then we are overloaded */
    if(!oversubscribed && !thread_group->listener) {
      thread_group->listener = current_thread;
      thread_group->active_thread_count--;
      mysql_mutex_unlock(&thread_group->mutex);

      connection = listener(thread_group);

      mysql_mutex_lock(&thread_group->mutex);
      thread_group->active_thread_count++;
      /* There is no listener anymore, it just returned. */
      thread_group->listener = nullptr;
      break;
    }

    /*
      Last thing we try before going to sleep is to
      pick a single event via epoll, without waiting (timeout 0)
    */
    if (!oversubscribed) {
      native_event nev;
      if (io_poll_wait(thread_group->pollfd, &nev, 1, 0) == 1) {
        thread_group->io_event_count++;
        connection = (connection_t *)native_event_get_userdata(&nev);

        /*
          Since we are going to perform an out-of-order event processing for the
          connection, first check whether it is eligible for high priority
          processing. We can get here even if there are queued events, so it
          must either have a high priority ticket, or there must be not too many
          busy threads (as if it was coming from a low priority queue).
        */
        if (connection_is_high_prio(*connection)) {
          connection->tickets--;
        } else if (too_many_busy_threads(*thread_group)) {
          /*
            Not eligible for high priority processing. Restore tickets and put
            it into the low priority queue.
          */
          connection->tickets =
              connection->thd->variables.threadpool_high_prio_tickets;
          connection->when_enqueued = my_micro_time();
          thread_group->queue.push_back(connection);
          connection = nullptr;
        }

        if (connection) {
          thread_group->get_normal_queue_num++;
          thread_group->queue_event_count++;
          connection->when_enqueued = 0;
          break;
        }
      }
    }
#endif /* HAVE_TDSQL */

    /* And now, finally sleep */
    current_thread->woken = false; /* wake() sets this to true */

    /*
      Add current thread to the head of the waiting list  and wait.
      It is important to add thread to the head rather than tail
      as it ensures LIFO wakeup order (hot caches, working inactivity timeout)
    */
    thread_group->waiting_threads.push_front(current_thread);

    thread_group->active_thread_count--;
    thread_group->mysql_cond_timedwait_num++;
    if (abstime) {
      err = mysql_cond_timedwait(&current_thread->cond, &thread_group->mutex,
                                 abstime);
    } else {
      err = mysql_cond_wait(&current_thread->cond, &thread_group->mutex);
    }
    thread_group->active_thread_count++;

    if (!current_thread->woken) {
      /*
        Thread was not signalled by wake(), it might be a spurious wakeup or
        a timeout. Anyhow, we need to remove ourselves from the list now.
        If thread was explicitly woken, than caller removed us from the list.
      */
      thread_group->waiting_threads.remove(current_thread);
    }

    if (err) break;
  }

  thread_group->stalled = false;
  mysql_mutex_unlock(&thread_group->mutex);

  return connection;
}

/**
  Tells the pool that worker starts waiting  on IO, lock, condition,
  sleep() or similar.
*/

static void wait_begin(thread_group_t *thread_group, bool dump_thread) noexcept {
  DBUG_TRACE;
  mysql_mutex_lock(&thread_group->mutex);
  if (!dump_thread)
    thread_group->active_thread_count--;
  thread_group->waiting_thread_count++;

  assert(thread_group->active_thread_count >= 0);
  assert(thread_group->connection_count > 0);

#ifdef HAVE_TDSQL
  /**
    Make sure there is a listner. If already so, also wake up threads to
    process the queue.
    DO NOT make this if branch easily reached otherwise performance is hurt
    badly.
  */
  if (!too_many_active_threads(*thread_group) &&
      (!thread_group->listener || !queues_are_empty(*thread_group))) {
    /**
      Group might stall while this thread waits, thus wake up a worker
      to prevent stall.
    */
    wake_or_create_thread(thread_group, false);
  }
#else /* HAVE_TDSQL */
#ifdef THREADPOOL_CREATE_THREADS_ON_WAIT
  if ((thread_group->active_thread_count == 0) &&
      (!queues_are_empty(*thread_group) || !thread_group->listener)) {
    /*
      Group might stall while this thread waits, thus wake
      or create a worker to prevent stall.
    */
    wake_or_create_thread(thread_group);
  }
#endif /*THREADPOOL_CREATE_THREADS_ON_WAIT */
#endif /* HAVE_TDSQL */

  mysql_mutex_unlock(&thread_group->mutex);
  return;
}

/**
  Tells the pool has finished waiting.
*/

static void wait_end(thread_group_t *thread_group, bool dump_thread) noexcept {
  DBUG_TRACE;
  mysql_mutex_lock(&thread_group->mutex);
  if (!dump_thread)
    thread_group->active_thread_count++;
  thread_group->waiting_thread_count--;
  mysql_mutex_unlock(&thread_group->mutex);
  return;
}

/**
  Allocate/initialize a new connection structure.
*/

static connection_t *alloc_connection(THD *thd) noexcept {
  DBUG_TRACE;
  DBUG_EXECUTE_IF("simulate_tp_alloc_connection_oom", return nullptr;);

  connection_t *connection = (connection_t *)my_malloc(
      key_memory_thread_pool_connection, sizeof(connection_t), 0);
  if (connection) {
    connection->thd = thd;
    connection->waiting = false;
    connection->logged_in = false;
    connection->bound_to_poll_descriptor = false;
    connection->when_enqueued = 0;
    connection->abs_wait_timeout = ULLONG_MAX;
    connection->tickets = 0;
    connection->dump_thread = false;
    connection->from_per_thread = false;
#ifdef HAVE_TDSQL
  connection->recursion_tp_wait_beginNum = 0;
#endif /* HAVE_TDSQL */
  }
  return connection;
}

/**
  Add a new connection to thread pool..
*/

bool Thread_pool_connection_handler::add_connection(
    Channel_info *channel_info) {
  DBUG_TRACE;

  THD *const thd = channel_info->create_thd();

  if (unlikely(!thd)) {
    channel_info->send_error_and_close_channel(ER_OUT_OF_RESOURCES, 0, false);
    return true;
  }

  connection_t *const connection = alloc_connection(thd);

  if (unlikely(!connection)) {
    thd->get_protocol_classic()->end_net();
    delete thd;
    channel_info->send_error_and_close_channel(ER_OUT_OF_RESOURCES, 0, false);
    return true;
  }

  delete channel_info;

  thd->set_new_thread_id();
  thd->start_utime = my_micro_time();

  thd->scheduler = &tp_event_functions;

  Global_THD_manager::get_instance()->add_thd(thd);

  thd->event_scheduler.data = connection;

  /* Assign connection to a group. */
  thread_group_t *group = &all_groups[thd->thread_id() % group_count];

  connection->thread_group = group;

  mysql_mutex_lock(&group->mutex);
  group->connection_count++;
  mysql_mutex_unlock(&group->mutex);

  /*
    Add connection to the work queue. Actual login
    will be done by a worker thread.
  */
  connection->when_enqueued = my_micro_time();
  queue_put(group, connection);
  return false;
}

/**
  Terminate connection.
*/

static void connection_abort(connection_t *connection) {
  DBUG_TRACE;
  thread_group_t *group = connection->thread_group;

  threadpool_remove_connection(connection->thd);

  mysql_mutex_lock(&group->mutex);
  group->connection_count--;
  mysql_mutex_unlock(&group->mutex);

  my_free(connection);
  return;
}

/**
  Keep connection_t's socket and free its resource. This function is called
  when connection_t is switching from Thread_pool to Per_thread.
  Modified from connection_abort.
*/

static void clean_connection_from_thread_pool(connection_t *connection) {
  DBUG_TRACE;
  thread_group_t *group = connection->thread_group;

  Vio *vio = connection->thd->get_protocol_classic()->get_vio();
  const int fd = mysql_socket_getfd(vio->mysql_socket);
  mysql_mutex_lock(&group->mutex);
  io_poll_disassociate_fd(group->pollfd, fd);
  mysql_mutex_unlock(&group->mutex);

  threadpool_remove_psi(connection->thd);

  mysql_mutex_lock(&group->mutex);
  group->connection_count--;
  mysql_mutex_unlock(&group->mutex);

  my_free(connection);
  return;
}

/**
  MySQL scheduler callback : kill connection
*/

void tp_post_kill_notification(THD *thd) noexcept {
  DBUG_TRACE;
  if (current_thd == thd || thd->system_thread) return;

  Vio *vio = thd->get_protocol_classic()->get_vio();
  if (vio) vio_cancel(vio, SHUT_RD);
  return;
}

/**
  MySQL scheduler callback: wait begin
*/

void tp_wait_begin(THD *thd, int type MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;
  assert(thd);
  connection_t *connection = (connection_t *)thd->event_scheduler.data;

#ifdef HAVE_TDSQL
  if (connection) {
    ++connection->recursion_tp_wait_beginNum;

    if (connection->recursion_tp_wait_beginNum == 1) {
      assert(!connection->waiting);
      connection->waiting = true;
      wait_begin(connection->thread_group, thd);
    }
  }
#else  /* HAVE_TDSQL */
  if (connection) {
    assert(!connection->waiting);
    connection->waiting = true;
    wait_begin(connection->thread_group, connection->dump_thread);
  }
#endif /* HAVE_TDSQL */

  return;
}

/**
  MySQL scheduler callback: wait end
*/

void tp_wait_end(THD *thd) {
  DBUG_TRACE;
  assert(thd);

  connection_t *connection = (connection_t *)thd->event_scheduler.data;
#ifdef HAVE_TDSQL
  if (connection) {
    --connection->recursion_tp_wait_beginNum;
    if (0 == connection->recursion_tp_wait_beginNum) {
      assert(connection->waiting);
      connection->waiting = false;
      wait_end(connection->thread_group, thd);
    }
  }
#else  /* HAVE_TDSQL */
  if (connection) {
    assert(connection->waiting);
    connection->waiting = false;
    wait_end(connection->thread_group, connection->dump_thread);
  }
#endif /* HAVE_TDSQL */
  return;
}

static void set_next_timeout_check(uint64 abstime) {
  DBUG_TRACE;
  while (abstime < pool_timer.next_timeout_check.load()) {
    uint64 old = pool_timer.next_timeout_check.load();
    pool_timer.next_timeout_check.compare_exchange_weak(old, abstime);
  }
  return;
}

/**
  Set wait timeout for connection.
*/

static void set_wait_timeout(connection_t *c) noexcept {
  DBUG_TRACE;
  /*
    Calculate wait deadline for this connection.
    Instead of using my_microsecond_getsystime() which has a syscall
    overhead, use pool_timer.current_microtime and take
    into account that its value could be off by at most
    one tick interval.
  */

  /* !!!FIXME: change variables.net_wait_timeout to get_wait_timeout in "kill
  idle transaction timeout" feature. */
  c->abs_wait_timeout =
      pool_timer.current_microtime.load(std::memory_order_relaxed) +
      1000LL * pool_timer.tick_interval +
      1000000LL * c->thd->variables.net_wait_timeout;

  set_next_timeout_check(c->abs_wait_timeout);
  return;
}

/**
  Handle a (rare) special case,where connection needs to
  migrate to a different group because group_count has changed
  after thread_pool_size setting.
*/

static int change_group(connection_t *c, thread_group_t *old_group,
                        thread_group_t *new_group) {
  assert(c->thread_group == old_group);

  /* Remove connection from the old group. */
  if (c->bound_to_poll_descriptor) {
    Vio *const vio = c->thd->get_protocol_classic()->get_vio();
    const int fd = mysql_socket_getfd(vio->mysql_socket);
    mysql_mutex_lock(&old_group->mutex);
    io_poll_disassociate_fd(old_group->pollfd, fd);
    c->bound_to_poll_descriptor = false;
  } else {
    mysql_mutex_lock(&old_group->mutex);
  }
  c->thread_group->connection_count--;
  c->thread_group->connections_moved_out++;
  mysql_mutex_unlock(&old_group->mutex);

  /* Add connection to the new group. */
  mysql_mutex_lock(&new_group->mutex);
  c->thread_group = new_group;
  new_group->connection_count++;
  new_group->connections_moved_in++;
  /* Ensure that there is a listener in the new group. */
  int ret = 0;
  if (!new_group->thread_count) ret = create_worker(new_group);
  mysql_mutex_unlock(&new_group->mutex);
  return ret;
}

static int start_io(connection_t *connection) {
  /*
    Usually, connection will stay in the same group for the entire
    connection's life. However, we do allow group_count to
    change at runtime, which means in rare cases when it changes is
    connection should need to migrate  to another group, this ensures
    to ensure equal load between groups.

    So we recalculate in which group the connection should be, based
    on thread_id and current group count, and migrate if necessary.
  */
  thread_group_t *const group =
      &all_groups[connection->thd->thread_id() % group_count];

  if (group != connection->thread_group) {
    if (change_group(connection, connection->thread_group, group)) return -1;
  }

  /*
    Bind to poll descriptor if not yet done.
  */
  Vio *vio = connection->thd->get_protocol_classic()->get_vio();
  int fd = mysql_socket_getfd(vio->mysql_socket);
  if (!connection->bound_to_poll_descriptor) {
    connection->bound_to_poll_descriptor = true;
    return io_poll_associate_fd(group->pollfd, fd, connection);
  }

  return io_poll_start_read(group->pollfd, fd, connection);
}

static void set_max_req_inq_wait_time(connection_t *connection) {
  const ulonglong in_queue_latency =
      (connection->when_enqueued > 0
           ? my_microsecond_getsystime() - connection->when_enqueued
           : 0);

  ulonglong old = connection->thread_group->max_req_latency_us.load(
      std::memory_order_relaxed);
  while (in_queue_latency > old &&
         !connection->thread_group->max_req_latency_us.compare_exchange_weak(
             old, in_queue_latency)) {
  }
}

static void handle_event(connection_t *connection) {
  DBUG_TRACE;
  int err;
  THD *thd = connection->thd;

  mysql_mutex_lock(&connection->thread_group->mutex);
  connection->thread_group->total_usecs_in_queue +=
      (connection->when_enqueued > 0
           ? my_micro_time() - connection->when_enqueued
           : 0);
  connection->thread_group->events_consumed++;
  mysql_mutex_unlock(&connection->thread_group->mutex);

  set_max_req_inq_wait_time(connection);

  if (!connection->logged_in) {
    err = threadpool_add_connection(connection->thd);
    connection->logged_in = true;
  } else {
    if (connection->from_per_thread) {
      connection->from_per_thread = false;
      connection->thd->to_thread_pool += 1;

      mysql_mutex_lock(&connection->thread_group->mutex);
      connection->thread_group->connections_moved_from_per_thread++;
      mysql_mutex_unlock(&connection->thread_group->mutex);
      /*
        threadpool_process_request_prepare do preparation for connection from
        per_thread. If the return value is equal to false, then there is no
        data in vio, skip threadpool_process_request this time.
      */
      if (!threadpool_process_request_prepare(connection->thd)) goto io;
    }

    err = threadpool_process_request(connection->thd);
    connection->when_enqueued = 0;
  }

  if (err) goto end;

io:
  /* Check if we need to switch this connection to Per_thread mode. */
  if (Connection_handler_manager::thread_handling ==
      Connection_handler_manager::SCHEDULER_ONE_THREAD_PER_CONNECTION) {
    /*
      Now that the thread_handling was changed. Let's see what we should do:
      DISABLED_SWITCH_MODE: should not be here.
      STABLE_SWITCH_MODE: doing nothing, it only cares about new connections.
      FAST_SWITCH_MODE: move current connection to per_thread.
      SHARP_SWITCH_MODE: kill current connection and force users to reconnect.
   */
    assert(Connection_handler_manager::thread_handling_switch_mode !=
           Connection_handler_manager::DISABLED_SWITCH_MODE);
    if (Connection_handler_manager::thread_handling_switch_mode ==
        Connection_handler_manager::FAST_SWITCH_MODE) {
      Connection_handler_manager *handler_manager =
          Connection_handler_manager::get_instance();

      mysql_mutex_lock(&connection->thread_group->mutex);
      connection->thread_group->connections_moved_to_per_thread++;
      mysql_mutex_unlock(&connection->thread_group->mutex);

      /*
        We have to clean threadpool connection before migrate to per_thread.
        If we failed to create handle_connection thread in migrating in
        in the extrem case, we have no better choice and we have to free
        resources, close connection and force user to reconnect.
      */
      clean_connection_from_thread_pool(connection);
      if (handler_manager
              ->get_connection_handler(Connection_handler_manager::
                                           SCHEDULER_ONE_THREAD_PER_CONNECTION)
              ->migrate(thd)) {
        threadpool_remove_connection_except_psi(thd);
      }
      return;
    } else if (Connection_handler_manager::thread_handling_switch_mode ==
               Connection_handler_manager::SHARP_SWITCH_MODE) {
      err = 1;
      goto end;
    }
  }

  set_wait_timeout(connection);
  err = start_io(connection);

end:
  if (err) connection_abort(connection);
  return;
}

/**
  Worker thread's main
*/

static void *worker_main(void *param) {
  my_thread_init();

  DBUG_TRACE;

  thread_group_t *thread_group = (thread_group_t *)param;

  /* Init per-thread structure */
  worker_thread_t this_thread;
  mysql_cond_init(key_worker_cond, &this_thread.cond);
  this_thread.thread_group = thread_group;
  this_thread.event_count = 0;

#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_THREAD_CALL(set_thread_account)
  (NULL, 0, NULL, 0);
#endif

  /* Run event loop */
  for (;;) {
    connection_t *connection;
    struct timespec ts;
    set_timespec_coarse(&ts, threadpool_idle_timeout);
    connection = get_event(&this_thread, thread_group, &ts);
    if (!connection) break;
    this_thread.event_count++;
    handle_event(connection);
  }

  /* Thread shutdown: cleanup per-worker-thread structure. */
  mysql_cond_destroy(&this_thread.cond);

  mysql_mutex_lock(&thread_group->mutex);
  add_thread_count(thread_group, -1);
  mysql_mutex_unlock(&thread_group->mutex);

  my_thread_end();
  return nullptr;
}

bool tp_init() {
  DBUG_TRACE;
  threadpool_started = true;

  for (uint i = 0; i < array_elements(all_groups); i++) {
    thread_group_init(&all_groups[i], get_connection_attrib());
  }
  tp_set_threadpool_size(threadpool_size);
  if (group_count == 0) {
    /* Something went wrong */
    sql_print_error("Can't set threadpool size to %d", threadpool_size);
    return true;
  }
#ifdef HAVE_PSI_INTERFACE
  mysql_mutex_register("threadpool", mutex_list, array_elements(mutex_list));
  mysql_cond_register("threadpool", cond_list, array_elements(cond_list));
  mysql_thread_register("threadpool", thread_list, array_elements(thread_list));
#endif

  pool_timer.tick_interval = threadpool_stall_limit;
  pool_timer.exit = false;
  if (start_timer(&pool_timer)) {
    sql_print_error("failed to start threadpool timer thread");
    return true;
  }
  return false;
}

void tp_end() noexcept {
  DBUG_TRACE;

  if (!threadpool_started) return;

  stop_timer(&pool_timer);
  for (uint i = 0; i < array_elements(all_groups); i++) {
    thread_group_close(&all_groups[i]);
  }
  threadpool_started = false;
  return;
}

/**
  Connection migrate from per thread to thread pool.
  Modified from Thread_pool_connection_handler::add_connection.
  @return true when failed, or false when success.
*/
bool Thread_pool_connection_handler::migrate(THD *thd) {
  connection_t *const connection = alloc_connection(thd);
  if (!connection) {
    return true;
  }

  connection->logged_in = true;
  connection->from_per_thread = true;

  thd->scheduler = &tp_event_functions;
  thd->event_scheduler.data = connection;
  /* Assign connection to a group. */
  thread_group_t *group = &all_groups[thd->thread_id() % group_count];
  connection->thread_group = group;
  mysql_mutex_lock(&group->mutex);
  group->connection_count++;
  mysql_mutex_unlock(&group->mutex);
  /*
    Add connection to the work queue. Actual login
    will be done by a worker thread.
  */
  connection->when_enqueued = my_micro_time();
  queue_put(group, connection);
  return false;
}

/** Ensure that poll descriptors are created when threadpool_size changes */

void tp_set_threadpool_size(uint size) noexcept {
  if (!threadpool_started) return;

  bool success = true;
  for (uint i = 0; i < size; i++) {
    thread_group_t *group = &all_groups[i];
    mysql_mutex_lock(&group->mutex);
    if (group->pollfd == -1) {
      group->pollfd = io_poll_create();
      success = (group->pollfd >= 0);
      if (!success) {
        sql_print_error("io_poll_create() failed, errno=%d\n", errno);
        break;
      }
    }
    mysql_mutex_unlock(&all_groups[i].mutex);
    if (!success) {
      group_count = i;
      return;
    }
  }
  group_count = size;
}

void tp_set_threadpool_stall_limit(uint limit) noexcept {
  if (!threadpool_started) return;
  mysql_mutex_lock(&(pool_timer.mutex));
  pool_timer.tick_interval = limit;
  mysql_mutex_unlock(&(pool_timer.mutex));
  mysql_cond_signal(&(pool_timer.cond));
}

/**
 Calculate number of idle/waiting threads in the pool.

 Sum idle threads over all groups.
 Don't do any locking, it is not required for stats.
*/

int tp_get_idle_thread_count() noexcept {
  int sum = 0;
  for (uint i = 0;
       i < array_elements(all_groups) && (all_groups[i].pollfd >= 0); i++) {
    sum += (all_groups[i].thread_count - all_groups[i].active_thread_count);
  }
  return sum;
}

/* Report threadpool problems */

/**
   Delay in microseconds, after which "pool blocked" message is printed.
   (30 sec == 30 Mio usec)
*/
#define BLOCK_MSG_DELAY 30 * 1000000

#define MAX_THREADS_REACHED_MSG \
  "Threadpool could not create additional thread to handle queries, because the \
number of allowed threads was reached. Increasing 'thread_pool_max_threads' \
parameter can help in this situation.\n \
If 'admin_port' parameter is set, you can still connect to the database with \
superuser account (it must be TCP connection using admin_port as TCP port) \
and troubleshoot the situation. \
A likely cause of pool blocks are clients that lock resources for long time. \
'show processlist' or 'show engine innodb status' can give additional hints."

#define CREATE_THREAD_ERROR_MSG "Can't create threads in threadpool (errno=%d)."

/**
 Write a message when blocking situation in threadpool occurs.
 The message is written only when pool blocks for BLOCK_MSG_DELAY (30) seconds.
 It will be just a single message for each blocking situation (to prevent
 log flood).
*/

static void print_pool_blocked_message(bool max_threads_reached) noexcept {
  ulonglong now = my_microsecond_getsystime();
  static bool msg_written;

  if (pool_block_start == 0) {
    pool_block_start = now;
    msg_written = false;
  }

  if (!msg_written && ((now > pool_block_start + BLOCK_MSG_DELAY) ||
                       (now == pool_block_start))) {
    if (max_threads_reached)
      sql_print_error(MAX_THREADS_REACHED_MSG);
    else
      sql_print_error(CREATE_THREAD_ERROR_MSG, my_errno);

    if (now > pool_block_start) {
      sql_print_information("Threadpool has been blocked for %u seconds\n",
                            (uint)((now - pool_block_start) / 1000000));
    }
    /* avoid reperated messages for the same blocking situation */
    msg_written = true;
  }
}

bool show_threadpool_status(THD *thd) {
  Protocol *protocol = thd->get_protocol();
  mem_root_deque<Item *> field_list(thd->mem_root);
  DBUG_TRACE;

  field_list.push_back(new Item_return_int("groupid", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("connection_count", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("thread_count", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("havelistener", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("active_thread_count", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("waiting_thread_count", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("waiting_threads_size", 10, MYSQL_TYPE_LONG));
  field_list.push_back(new Item_return_int("queue_size", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("high_prio_queue_size", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("get_high_prio_queue_num", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("get_normal_queue_num", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("create_thread_num", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("wake_thread_num", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("oversubscribed_num", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("mysql_cond_timedwait_num", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("check_stall_nolistener", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("check_stall_stall", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("max_req_latency_us", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("conns_timeout_killed", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("connections_moved_in", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("connections_moved_out", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_return_int("connections_moved_from_per_thread",
                                           10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_return_int("connections_moved_to_per_thread",
                                           10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("events_consumed", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_return_int("average_wait_usecs_in_queue", 10,
                                           MYSQL_TYPE_LONGLONG));
#ifdef HAVE_TDSQL
  field_list.push_back(
    new Item_return_int("max_req_latency_us", 10, MYSQL_TYPE_LONGLONG));
#endif /* HAVE_TDSQL */

  if (thd->send_result_metadata(field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    return true;
  }

  uint grp_cnt = group_count;
  for (uint i = 0; i < grp_cnt; ++i) {
    protocol->start_row();
    thread_group_t *group = all_groups + i;
    mysql_mutex_lock(&group->mutex);

    protocol->store(i);
    protocol->store(group->connection_count);
    protocol->store(group->thread_count);
    protocol->store(group->listener == NULL ? 0 : 1);
    protocol->store(group->active_thread_count);
    protocol->store(group->waiting_thread_count);
    protocol->store(group->waiting_threads.elements());
    protocol->store(group->queue.elements());
    protocol->store(group->high_prio_queue.elements());
    protocol->store(group->get_high_prio_queue_num);
    protocol->store(group->get_normal_queue_num);
    protocol->store(group->create_thread_num);
    protocol->store(group->wake_thread_num);
    protocol->store(group->oversubscribed_num);
    protocol->store(group->mysql_cond_timedwait_num);
    protocol->store(group->check_stall_nolistener);
    protocol->store(group->check_stall_stall);
    protocol->store(group->max_req_latency_us.load());
    protocol->store(group->conns_timeout_killed.load());
    protocol->store(group->connections_moved_in);
    protocol->store(group->connections_moved_out);
    protocol->store(group->connections_moved_from_per_thread);
    protocol->store(group->connections_moved_to_per_thread);
    protocol->store(group->events_consumed);
    protocol->store(group->events_consumed == 0
                        ? 0
                        : group->total_usecs_in_queue / group->events_consumed);

    group->events_consumed = 0;
    group->total_usecs_in_queue = 0;
#ifdef HAVE_TDSQL
       protocol->store(group->oversubscribed_paral_num);
#endif /* HAVE_TDSQL */
    mysql_mutex_unlock(&group->mutex);

    if (protocol->end_row()) {
      return true;
    }
  }

  my_eof(thd);
  return false;
}

/*
 * thd_bottom_half.cpp
 *
 *  Created on: 2014.4.5
 *      Author: harlylei
 *  Modified by: daviezhao
 1. use std::deque<CThdKey> instead of std::set<CThdKey> for better
 back-insertion and removal performance.
 2. Bug fix: make sure a THD is always used/owned/attached by one thread only.
 Otherwise there can be undefined behaviors and system crash in mysqld.
 3. Bug fix: make sure a THD is not processed by the answering thread if it's
 not alive anymore.
 4. Bug fix: make sure if a connection C should be aborted but C is to be
 handled by bottom half threads, it's removed from the udp sver thread, and
 if C is already dispatched to an answering thread T, C will be aborted by T
 instead. Without this fix, mysqld will crash when C is handled by T since C
 is already destroyed by the threadpool worker thread.
 */
//Functionity: the last part of statement
#include <string.h>
#include <utility>

#include <my_thread_local.h>
#include "log.h"
#include "my_sys.h"
#include "binlog.h"
#include "sql_parse.h"
#include "mysqld.h"
#include "protocol_classic.h"
#include "mysql/psi/mysql_idle.h"
#include "mysql/psi/mysql_socket.h"
#include "thd_bottom_half.h"

PSI_stage_info stage_waiting_for_sqlasyn_ack_from_slave = { 0, "Waiting for sqlasyn ACK from slave", 0 };
PSI_stage_info stage_waiting_for_dispatch_thd_to_ans_thread = { 0, "dispatched thd to answering thread(ack'ed)", 0 };

struct connection_t;
extern void connection_abort(connection_t *connection);
bool connection_should_abort(connection_t *connection);
void lock_conn_sqlasync(connection_t *connection);
void unlock_conn_sqlasync(connection_t *connection);
bool thd_connection_alive(THD *thd);
extern bool bindThdToEvent(THD* thd);
extern int detach_io(THD* thd);

ulonglong sqlasyn_get_slave_ans = 0;
ulonglong sqlasyn_get_slave_ans_skip = 0;
ulonglong sqlasyn_deal_trx_by_ans = 0;
ulonglong sqlasyn_deal_trx_by_fast_ans = 0;
ulonglong sqlasyn_exceed_warn_num = 0;
ulonglong sqlasyn_timeout_num = 0;

static bool enable_sql_asyn_when_ok= false;

extern MYSQL_BIN_LOG mysql_bin_log;

using namespace VarBufNS;

CLocalMysqlThread::CLocalMysqlThread() {
    m_threadID = 0;
    initThread();
    m_threadstate = 0;
}

CLocalMysqlThread::~CLocalMysqlThread() {
    pthread_attr_destroy(&m_attr);
}
int CLocalMysqlThread::initThread() {
    pthread_attr_init(&m_attr);
    int iRet = 0;

    iRet = pthread_attr_setscope(&m_attr, PTHREAD_SCOPE_SYSTEM);
    if (iRet != 0) {
        fprintf(stderr, "set PTHREAD_SCOPE_SYSTEM error %d\n", iRet);
        pthread_attr_destroy(&m_attr);
        return -1;
    }
    return 0;
}
int CLocalMysqlThread::start() {
    if (m_threadID > 0) { /* Already started */ 
        return 0;
    }

    int iRet = 0;
    m_threadstate = 1; // set the flag to running state
    if ((iRet = pthread_create(&m_threadID, &m_attr, &startThread,(void *) this)) != 0) {
        fprintf(stderr, "create thread error %d\n", iRet);
        pthread_attr_destroy(&m_attr);
        errno = iRet;
        m_threadstate = 0; //set the flag to stopping state
        return -1;
    }
    return 0;
}

void* CLocalMysqlThread::startThread(void* arg) {
    if (0 == arg)
        return 0;
    CLocalMysqlThread* ptr = (CLocalMysqlThread*)arg;
    ptr->run();

    return 0;
}

int CLocalMysqlThread::stop() {
    m_threadstate = 0;

    return 0;
}

/*
  Save thd into bottom-half dispatcher thread in order to resume executing
  the sql command of thd when slaves have received binlogs of thd's executing
  txn T0.
  @param [out] haveGetAns whether slave have already received binlogs of thd's
  currently executing txn T0. If true, the bottom half of T0 will be executed
  immediately rather than asyncly.
  @return true if the thd is successfully stored into the bottom-half dispatcher
  thread. false otherwise.
*/
struct connection_t;
bool CThdBottomHalf::saveThd(enum enum_server_command command, THD *thd, bool error, bool &haveGetAns) {

    haveGetAns = false;
    CThdKey key(command, thd, error);

    bool result = false;

    THD_STAGE_INFO(thd, stage_waiting_for_sqlasyn_ack_from_slave);

    DBUG_EXECUTE_IF("stop_before_saveThd",
            {
                    sql_print_warning("saveThd before sleep,thd:%s",thd->toString().c_str());
                    sleep(10);
                    sql_print_warning("saveThd after sleep,thd:%s",thd->toString().c_str());
            };
    );

    do {
        CTGuard<CTMutex> gaurd(m_mutex);

        if(m_stop_all) {
          set_thd_error_server_stop(thd);//dba stop mysqld,set error and return directly
          haveGetAns = true;
          result = false;
          break;
        }

        if (m_newstBinlogInfoAns < thd->ack_binlog_pos()) {
            m_thdContainer.push_back(key);
            result = true;
        } else { //already be answered so keep going
            ++sqlasyn_deal_trx_by_fast_ans;
            haveGetAns = true;
        }
    } while (0);

    return result;
}

/*
  Remove thd from container, returns true if removed, false if 'thd' is already
  dispatched to an answering thread.
  Called when the connection is to be killed, and thd must be a session that has
  its bottom half work done by bottom half threads.
*/
bool CThdBottomHalf::remove_thd(const THD *thd)
{
  // Can only remove a thd that is ever given to us here.
  DBUG_ASSERT(thd->m_asyncAns);
  CTGuard<CTMutex> gaurd(m_mutex);
  for (ThdQueue_t::iterator i= m_thdContainer.begin();
       i != m_thdContainer.end(); ++i) {
    if (i->getThd() == thd) {
      // It's OK to erase here since no iterator used after the erase.
      m_thdContainer.erase(i);
      return true;
    }
  }
  return false;
}

void CThdBottomHalf::reset_answer(void) {
  CTGuard<CTMutex> gaurd(m_mutex);
  m_newstBinlogInfoAns.reset();
}

void CThdBottomHalf::commit_timeout_trxs(void) {
  THD *old_thd = current_thd;
  MEM_ROOT **old_root = THR_MALLOC;
  CTGuard<CTMutex> gaurd(m_mutex);

  for (ThdQueue_t::iterator iter = m_thdContainer.begin();
    iter != m_thdContainer.end(); ++iter) {
    if (iter->is_processed())
      continue;
    // it's timeout and uncommitted trxs
    if (iter->getThd()->m_sql_asyn_deal_stage == THD::WAIT_TIMEOUT) {
      CThdBottomHalfAnsThread::deal_answered_thd(*iter);
      iter->mark_processed();
      sqlasync_uncommitted_timeout_trxs--;
    }
  }
  pop_processed();
  assert(sqlasync_uncommitted_timeout_trxs == 0);
  // recover the thread local variables
  current_thd = old_thd;
  THR_MALLOC = old_root;
}

/** Deal with answers from slave */
void CThdBottomHalf::dealBinlogPosAns(const Thd_Trans_binlog_info &ack_info) {

    CTGuard<CTMutex> gaurd(m_mutex);

    ++sqlasyn_get_slave_ans;
    if (!m_newstBinlogInfoAns.less(ack_info)) {
        ++sqlasyn_get_slave_ans_skip;
        return;
    }

    /*
      When we receive an ack, re-enable sqlasyn if we turned it off. BUT this ack
      might not be one that can really push forward txn commits, if slave starts
      replication from a very old position, and if we simply
      reenable sqlasyn, the committing txns will timeout, and on timeout sqlasyn
      will be again turned off, and then again turned on here, and many txns
      would be made waiting for acks which are doomed to timeout. So, only
      reenable sqlasyn when the ack reaches the current binlog file.
    */
    if (tdsql_allow_async && enable_sql_asyn_when_ok) {
        LOG_INFO log_info;
        mysql_bin_log.get_current_log(&log_info);

        /* Get current file number */
        const char *ptr = strrchr((log_info.log_file_name + dirname_length(log_info.log_file_name)), '.');
        DBUG_ASSERT(ptr != nullptr);

        if (ack_info.file_no() >= strtoul(ptr + 1, nullptr, 10)) {
            enable_sql_asyn_when_ok= false;
            g_sqlAsyn= true;
        }
    }

    const uint64_t ack_time = getMonotonic_sec(); // current time, in seconds
    // must copy under m_mutex.
    m_newstBinlogInfoAns.set(ack_info.file_no(), ack_info.pos());
    /*
      dispatch all bottom-half work that are do-able to answering threads.
      a do-able one is one whose txn's binlog have been received by slaves.
      the order that the answering threads reply to clients is arbitrary,
      but that's OK since all of them are guaranteed to have committed
      successfully on current master and slaves and future master(if a
      master switch happens soon).

      if a txn T2 modifies a row R0 that was inserted by txn T1, and user
      has issued 'commit' to T1 but T1's bottom half is sitll in
      this->m_thdContainer, then something strange and intresting can happen:
      since T1 has already SE-committed on master, T2 is able to see and
      modify the row R0 --- although T1's user doesn't know that T1 has
      already committed, other connections know already at this moment.

      But since T2's
      binlog is after that of T1, when T2's binlog is received by slaves,
      so will those of T1, thus when we can reply to client that T2 has
      commited, T1 are guaranteed to exist on current master and
      slaves together with T2. So a committed txn(T2)'s changes will always be made based on
      committed data, this is guaranteed by binlog order as above logic.
      Although it's likely that T2's user receives the 'commit OK' message
      earlier than T1's user, and this is OK and irrelevant.

      However, if T2 only reads R0 and is read only, then when T2
      ends, it's likely that because of master crashes before slave
      receiving T1's binlog, user will later not be able to find the row
      R0 after connecting to the new master, although he did see it in
      the old master.
    */
    for (ThdQueue_t::iterator iter = m_thdContainer.begin();
         iter != m_thdContainer.end(); ++iter) {
        if (iter->is_processed())
          continue;
        iter->setAckTime(ack_time);
        if (m_newstBinlogInfoAns < iter->getThd()->ack_binlog_pos()) {
            /*
              No need to keep searching, because even if there can be more
              waiters <= m_newstBinlogInfoAns, they can be freed when next
              larger position is ack'ed.
              It's more efficient to use a deque instead of a set because there are very
              frequent and massive insertion/deletions, and deque does so in
              O(1) complexity, but set does so generally in O(NlogN) complexity.
              The only benifit of using a set is be able to skip following
              searches(if any) here, but that's a O(1) gain.
            */
            break;
        } else {
            /*
              Don't erase(iter) here because deque's iterator invalidation rules
              is complex and varies in implementations. Let's be safe, mark
              it processed and at the end of the function after this
              iteration, pop processed items at head of the deque.
            */
            iter->mark_processed();
            // Round robin assign the bottom-half jobs to each answering
            // thread, in binlog order
            THD_STAGE_INFO(iter->getThd(), stage_waiting_for_dispatch_thd_to_ans_thread);

            ++sqlasyn_deal_trx_by_ans;

            m_ansThread[iter->getThd()->thread_id() % m_threadNum].push(*iter);
        }
        // Here m_mutex is locked.
    }
    pop_processed();
}

void CThdBottomHalf::set_thd_error_server_stop(THD *thd) {

  //set error information
  thd->get_stmt_da()->set_overwrite_status(true);
  my_error(ER_SESSION_WAS_KILLED, MYF(0));
  thd->get_stmt_da()->set_overwrite_status(false);

  //print this error information,That means stop does enter here, so make sure you do it in the right order.
  sql_print_error("CThdBottomHalf::set_thd_error_server_stop thd thread, reason: %s",thd->toString().c_str());

}

/*
  Pop out processed items until the 1st item is not processed, or until the
  container is empty. This function assumes the 'm_mutex' is locked.

  returns the NO. of items poped.
*/
size_t CThdBottomHalf::pop_processed() {
  size_t cnt= 0;

  while (m_thdContainer.size() > 0 && m_thdContainer.front().is_processed()) {
    m_thdContainer.pop_front();
    cnt++;
  }

  return cnt;
}

void CThdBottomHalf::pop_all() {

  CTGuard<CTMutex> gaurd(m_mutex);

  m_stop_all = true;

  for (ThdQueue_t::iterator iter = m_thdContainer.begin();
      iter != m_thdContainer.end(); ++iter) {
    if (iter->is_processed())
      continue;

    iter->mark_processed();
    THD_STAGE_INFO(iter->getThd(), stage_waiting_for_dispatch_thd_to_ans_thread);
    m_ansThread[iter->getThd()->thread_id() % m_threadNum].push(*iter);
  }

  pop_processed();
}

void CThdBottomHalf::do_timeout_loop() { //deal with timeout session
    int num_processed = 0;
    my_thread_init();
    while (m_threadstate) {
        num_processed = 0;
        do {
            CTGuard<CTMutex> gaurd(m_mutex);

            const uint64_t cur = getMonotonic_sec(); // current timestamp in seconds

            bool sqlasyn = g_sqlAsyn; // use a consistent g_sqlAsyn in case it's modified between below uses.

            for (ThdQueue_t::iterator iter = m_thdContainer.begin();
                  iter != m_thdContainer.end(); ++iter) {
                if (iter->is_processed())
                    continue;

                if (sqlasyn &&
                    cur > (uint64_t)(iter->getReqTime() + g_sqlAsynTimeout) &&
                    tdsql_allow_async) {
                  /*
                    Use this var because sqlasyn might be OFF and
                    tdsql_allow_sync might be ON at the same time, and
                    we must not turn on sqlasyn in such a combination.
                  */
                  enable_sql_asyn_when_ok= true;
                  g_sqlAsyn= false;// degrade to async replication.
                  sqlasyn= false;
                }

                if (unlikely(!sqlasyn || iter->getThd()->is_killed())) {
                  iter->mark_processed();
                  num_processed++;
                  THD_STAGE_INFO(iter->getThd(), stage_waiting_for_dispatch_thd_to_ans_thread);
                  m_ansThread[iter->getThd()->thread_id() % m_threadNum].push(*iter);  
                  continue;
                } 

                // timeout
                if (cur > (uint64_t)(iter->getReqTime() + g_sqlAsynTimeout)) {
                  iter->setTimeout (true);
                  sqlasyn_timeout_num++;  
                  if (iter->getThd()->m_delay_commit) {
                    if (iter->getThd()->m_sql_asyn_deal_stage == THD::WAIT_ACK_STAGE) {
                      // It's ok, the work is not heavy so process it in timer thread
                      CThdBottomHalfAnsThread::deal_answered_thd(*iter);
                    }
                  } else {
                    iter->mark_processed();
                    num_processed++;
                    THD_STAGE_INFO(iter->getThd(), stage_waiting_for_dispatch_thd_to_ans_thread);
                    m_ansThread[iter->getThd()->thread_id() % m_threadNum].push(*iter);
                  }
                } else {
                  /*
                    This is OK because CThdKey::m_reqTime is set at construction time, so
                    they are increasing in the deque.
                  */
                  break;
                }
            }
            // m_mutex is locked here.
            pop_processed();
        } while (0);

        if (num_processed == 0)
          sleep(1);
    }
    my_thread_end();
}

Ack_container::container_iter Ack_container::min_ack() {

  Ack_container::container_iter min_iter = m_container.begin();
  if (min_iter == m_container.end()) {
    return min_iter;
  }

  Ack_container::container_iter iter = min_iter;
  for (++iter; iter != m_container.end(); ++iter) {
    if (iter->second.second.less(min_iter->second.second)) {
      min_iter = iter;
    }
  }

  return min_iter;
}

void Ack_container::resize() {
  CTGuard<CTMutex> gaurd(m_mutex);

  uint32_t new_size = g_sqlAsyncNSlaves;

  if (m_container.size() <= new_size) {
    return;
  }

  /** Erase until the size is satisfied */
  while (m_container.size() > new_size) {
    auto iter = min_ack();
    m_container.erase(iter);
  }

  auto itr = min_ack();
  DBUG_ASSERT(itr != m_container.end());

  g_thdBottomHalf->dealBinlogPosAns(itr->second.second);
}

void Ack_container::copy(std::vector<AckInfo> &infos) {
  infos.clear();

  CTGuard<CTMutex> gaurd(m_mutex);

  if (g_sqlAsyncNSlaves == 1 &&
      m_one_slave_info.ack_pos.file_no() > 0) {
    /* Copy from m_one_slave_info */
    infos.push_back(m_one_slave_info);

    return;
  }

  for (auto itr = m_container.begin(); itr != m_container.end(); itr++) {
    AckInfo info;
    info.server_id = itr->first;
    info.ack_time = itr->second.first;
    info.ack_pos = itr->second.second;

    infos.push_back(info);
  }
}

void Ack_container::process(const Thd_Trans_binlog_info &new_ack_info, uint64_t server_id, time_t ack_time) {
  CTGuard<CTMutex> gaurd(m_mutex);

  uint max_slaves = g_sqlAsyncNSlaves;

  if (max_slaves == 1) {
    /* recomment this line , Maintain the best compatibility in performance
     * When G_SQLASyncnSlaves is changed from 1 to N, it doesn't matter much if this record is missing.
     * Anyway, it will be covered soon after waiting for multiple ack
     */
//    m_container[thread_id] = new_ack_info;
    g_thdBottomHalf->dealBinlogPosAns(new_ack_info);

    m_one_slave_info.server_id = server_id;
    m_one_slave_info.ack_time = ack_time;
    m_one_slave_info.ack_pos = new_ack_info;

    return;
  }

  auto itr = m_container.find(server_id);

  if (itr != m_container.end()) {
    /* Update the stored value */
    if (itr->second.second.less(new_ack_info)) {
      itr->second = std::make_pair(ack_time, new_ack_info);
    }
  } else if (m_container.size() < max_slaves) {
    /* Insert the new element */
    m_container[server_id] = std::make_pair(ack_time, new_ack_info);
  } else {
    /* The container is full, so find the min element
    and replace it if possible */
    itr = min_ack();
    DBUG_ASSERT(itr != m_container.end());
    if (itr->second.second.less(new_ack_info)) {
      /* Erase min element and insert new one.*/
      m_container.erase(itr);
      m_container[server_id] = std::make_pair(ack_time, new_ack_info);
    } else {
      /** The min element is even larger than current one, so
      skip it. */
      return;
    }
  }

  if (m_container.size() < max_slaves) {
    /* do nothing because we don't have enough slave */
    return;
  }

  itr = min_ack();
  DBUG_ASSERT(itr != m_container.end());

  g_thdBottomHalf->dealBinlogPosAns(itr->second.second);
}

bool CThdBottomHalf::do_request(const char* buf, int len, const char* ip) {
  if (len <(int)(VarBufNS::CloudCommHead::getMinLen())) {
    sql_print_error("get req[ip:%s,msglen:%d] < CloudCommHead::getMinLen():%u \n",
        ip, len, VarBufNS::CloudCommHead::getMinLen());
    return false;
  }

  CloudCommHead* newCommHead =(CloudCommHead*)(const_cast<char*>(buf));
  if (len != newCommHead->getLen()) {
    sql_print_error("get req[ip:%s,msglen:%d] != req should len:%d \n", ip, len, newCommHead->getLen());
    return false;
  }

  Thd_Trans_binlog_info ack_info;

  if (0 == strcmp(newCommHead->classname, "BinlogPosAns")) {
    BinlogPosAns *binlogAns = (BinlogPosAns*) newCommHead;
    binlogAns->decode();

    //sql_print_information("get binlog ans:%s",binlogAns->toString().c_str());//just for test,don't need delete this line

    ack_info.set(binlogAns->getFileName(), binlogAns->log_pos);

    /* If packet is from older version, we give it a fake id */
    g_thdBottomHalf->ack_container.process(ack_info, binlogAns->get_server_id(), time(0));

    return true;
  }

  return false;
}

class Worker_thread_context {
#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_thread *const psi_thread;
#endif
#ifndef DBUG_OFF
  const my_thread_id thread_id;
#endif
 public:
  Worker_thread_context() noexcept
#ifdef HAVE_PSI_THREAD_INTERFACE
    :
    psi_thread(PSI_THREAD_CALL(get_thread)())
#ifndef DBUG_OFF
    ,
#endif
#endif
#ifndef DBUG_OFF
#ifndef HAVE_PSI_THREAD_INTERFACE
    :
#endif
        thread_id(my_thread_var_id())
#endif
  {
  }

  ~Worker_thread_context() noexcept {
#ifdef HAVE_PSI_THREAD_INTERFACE
    PSI_THREAD_CALL(set_thread)(psi_thread);
#endif
#ifndef DBUG_OFF
    set_my_thread_var_id(thread_id);
#endif
    current_thd = nullptr;
    THR_MALLOC = nullptr;
  }
};

/*
 Attach/associate the connection with the OS thread,
 */
static bool thread_attach(THD* thd) {
#ifndef DBUG_OFF
    set_my_thread_var_id(thd->thread_id());
#endif
    thd->thread_stack =(char*) &thd;
    thd->store_globals();
#ifdef HAVE_PSI_THREAD_INTERFACE
    PSI_THREAD_CALL(set_thread)(thd->get_psi());
#endif
    mysql_socket_set_thread_owner(thd->get_protocol_classic()->get_vio()->mysql_socket);
    return 0;
}

void CThdBottomHalfAnsThread::deal_answered_thd(const CThdKey &thdKey, bool stopped) {
  THD *the_thd= thdKey.getThd();
  /*
    If a connection is already dispatched to an answering thread, it is
    never aborted by the threadpool-worker thread, so here we always have
    valid the_thd pointers.
  */
  connection_t *connection =(connection_t*)(the_thd->event_scheduler.data);
  assert(connection);
  if (!connection) {
    sql_print_error("get thd,but connection is NULL");
    return;
  }
  /*
    Should lock m_lockWithSqlAsyn here, rather than inside
    bindThdToEvent() below, and unlock it after bindThdToEvent() below.
    then check that the connection isn't marked for us to abort
    here, if so, abort it.
  */
  lock_conn_sqlasync(connection);
  if (connection_should_abort(connection)) {
    // Must unlock the mutex before destroying it
    // otherwise pthread causes undefined behavior.
    unlock_conn_sqlasync(connection);
    sql_print_error("connection_should_abort is true,so call connection_abort");
    connection_abort(connection);
    return;
  }

  Worker_thread_context worker_context;

  thread_attach(the_thd);

  // Have to define them here because of the 'goto' below.
  bool thd_timeout = false;
  bool bind_result = false;
  Vio *vio = nullptr;
  // Check aliveness after attaching to the thd. The session/connection
  // may have been killed by user, and if so we finish process the trx
  if (!thd_connection_alive(the_thd) &&
      the_thd->is_killed() != ER_SERVER_SHUTDOWN) {
     // When connection is dead, we need to call connection_abort()(see
     // handle_event() for same processing). So flip this switch, don't
     // bother to define and use another flag variable.
     goto conn_gone;
  }

  thd_timeout= thdKey.isTimeout();
  //execute the last part
  if (thd_timeout) {
    /*
      The OK status was already set by the DML statement so here we have
      to allow overwrite status to set error status.
    */
    the_thd->get_stmt_da()->set_overwrite_status(true);
    my_error(ER_SYNC_TIMEOUT, MYF(0));
    the_thd->get_stmt_da()->set_overwrite_status(false);
    // Timeout error already logged, not gonna repeat here.
  } else if (g_sqlAsynWarnTimeout > 0) {
    const uint64 cur = getMonotonic_sec();
    if ((uint)(cur - thdKey.getReqTime()) > g_sqlAsynWarnTimeout) {
      sqlasyn_exceed_warn_num++;
      sql_print_error("session waiting for ack of binlog pos (%u,%llu) cost [%ld] sec,exceed %d sec",
                      thdKey.getThd()->ack_binlog_pos().file_no(),
                      thdKey.getThd()->ack_binlog_pos().pos(),
                      (int64_t)(cur - thdKey.getReqTime()), g_sqlAsynWarnTimeout);
    }
  }

  if (!thd_timeout) { // normal case
    finish_command(thdKey.getCommand(), the_thd, nullptr, thdKey.isError());
    if (!stopped && the_thd->is_killed() != ER_SERVER_SHUTDOWN) bind_result = bindThdToEvent(the_thd);
  } else { // timeout
    if (the_thd->m_delay_commit) {
      if (the_thd->m_sql_asyn_deal_stage == THD::WAIT_ACK_STAGE) {
        // no commit in engine
        do_finish_command(thdKey.getCommand(), the_thd, nullptr, thdKey.isError());
        vio = the_thd->get_protocol_classic()->get_vio();
        if (vio) vio_cancel(vio, SHUT_WR); // close the connection under connection lock
        the_thd->m_sql_asyn_deal_stage = THD::WAIT_TIMEOUT;
        sqlasync_uncommitted_timeout_trxs++;
        bind_result = true; // keep the connection alive
      } else {
        assert(the_thd->m_sql_asyn_deal_stage == THD::WAIT_TIMEOUT);
        delay_commit_trx(the_thd); // commit trx now
      }
    } else {
      finish_command(thdKey.getCommand(), the_thd, nullptr, thdKey.isError());
    }
  }
conn_gone:
  unlock_conn_sqlasync(connection);
  if (!bind_result) {
    sql_print_error("aborting in bottom half answering thread, reason: %s",
                    (thd_timeout ? "tdsql ack timeout" : ((!thd_connection_alive(the_thd)) ? "connection killed" : "bind-poll error")));
    connection_abort(connection);
  }
}

int CThdBottomHalfAnsThread::run() {

    my_thread_init();

    CThdKey thdKey;
    while (true) {
        bool got_msg = m_queue.getmsg(2000, thdKey);
        /*
          If notified to exit, we will finish processing all bottom-half work
          before exiting this thread, so that all work in queue are processed,
          otherwise the corresponding clients won't know that their txns have
          committed successfully.
        */
        if (!got_msg) {
            if (m_threadstate)
                continue;
            else {
              /* double check */
              got_msg = m_queue.getmsg(0, thdKey);
              if (!got_msg) {
                break;
              }
            }
        }

        // do the real work
        deal_answered_thd(thdKey, !m_threadstate);
    }

    my_thread_end();

    return 0;
}


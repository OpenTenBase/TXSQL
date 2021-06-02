/*
 * thd_bottom_half.h
 *
 *  Created on: 2014.4.5
 *      Author: harlylei
 *  Modified by: daviezhao
 *          for: bug fixing and performance optimization.
 */

#ifndef THD_BOTTOM_HALF_H_
#define THD_BOTTOM_HALF_H_

#include <deque>
#include <string>
#include <time.h>
#include "sql/log.h" // LogError
#include "sql/mutexlock.h"

#include "sql/sql_class.h"
#include "sql/varbufproto.h"
#include "sql/udpsvr.h"
#include <unordered_map>

#pragma pack(1)

inline uint64_t getMonotonic_msec() {
  struct timespec tm;
  clock_gettime (CLOCK_MONOTONIC, &tm);

  return tm.tv_sec * 1000 + tm.tv_nsec / 1000000;
}

inline uint64_t getMonotonic_sec() {
  return getMonotonic_msec () / 1000;
}

//after this version,we will support m_server_id for BinlogPosAns
enum BinlogPosAnsVer : uint16_t {
  DefaultVer = 1,
  NewVer_20210420 = 2,
};

struct BinlogPosAns: public VarBufNS::CloudCommHead {
public:
    my_off_t log_pos; //binlog offset
private:
    VarBufNS::VarValue m_filename; //binlog name
    uint64_t m_server_id;//we can safely add fields before m_varBuf

    typedef VarBufNS::VarBufMgn VarBufType;
    VarBufType m_varBuf; //use getVarBuf to access this varaible.
public:
    BinlogPosAns() : log_pos(0),m_server_id(0) {
        CloudBaseConstruct;
        snprintf(classname, sizeof(classname), "BinlogPosAns");
        ver = NewVer_20210420;
    }

    void decode() {}

    std::string toString() {
        char buf[1024] = { 0 };
        snprintf(buf, sizeof(buf), "filename:%s,filePos:%llu,server id:%lu", getFileName(), log_pos, get_server_id());
        return buf;
    }

    //set file name
    char* setFileName(const void* ptr, unsigned int len) {
        return getVarBuf()->assigenVarValue(m_filename, ptr, len);
    }

    const char* getFileName() {
        return getVarBuf()->getBuf(m_filename.valIndex);
    }

    void set_server_id(const uint64_t server_id ) {
      m_server_id = server_id;
    }

    //Keep old versions compatible,when new master get old slave ans,get_server_id() will return 0
    uint64_t get_server_id() {
      if (ver >= NewVer_20210420) {
        return m_server_id;
      } else {
        return 0;
      }
    }

    CloudBaseFun(BinlogPosAns);
private:
    BinlogPosAns(const BinlogPosAns &);
    const BinlogPosAns & operator=(const BinlogPosAns & lft);
};

#pragma pack()

struct connection_t;
// The element stored in container
//enum enum_server_command command, THD *thd,bool error
class CThdKey {
public:
    CThdKey() : m_thd(NULL), m_command(COM_END),
                 m_error(false), m_timeout(false), m_processed(false) {
        m_reqTime = getMonotonic_sec();
        m_ackTime = 0;
    }

    CThdKey(enum_server_command command MY_ATTRIBUTE((unused)), THD *thd, bool error) :
             m_thd(thd), m_command(COM_END),
             m_error(error), m_timeout(false), m_processed(false) {
        m_reqTime = getMonotonic_sec();
        m_ackTime = 0;
    }

    inline enum_server_command getCommand() const { return m_command; }

    inline bool isError() const { return m_error; }

    inline THD* getThd() const { return m_thd; }

    inline uint64_t getReqTime() const { return m_reqTime; }

    uint64_t getAckTime() const { return m_ackTime; }

    void setAckTime(uint64_t acktime) { m_ackTime = acktime; }

    bool isTimeout() const { return m_timeout; }

    void setTimeout(bool timeout) { m_timeout = timeout; }

    bool is_processed() const { return m_processed; }

    void mark_processed(bool processed = true) {
      m_processed= processed;
    }

private:
    THD *m_thd;
    //it records when this instance was created: when bottom-half work
    //was enqueued(and top half done)
    uint64_t m_reqTime;
    uint64_t m_ackTime;
    enum_server_command m_command;
    bool m_error;

    bool m_timeout; // Record if it's timeout session 
    bool m_processed;
};

class CLocalMysqlThread {
public:
    CLocalMysqlThread();
    virtual ~CLocalMysqlThread();

    int start();

    pthread_t getThreadID() const { return m_threadID; }

    //Set state to stop
    virtual int stop();

    //fore to stop the thread
    void join() {
        int err = 0;
        if ((err = pthread_join(m_threadID, NULL))) {
          fprintf(stderr, "CLocalMysqlThread::join(): pthread_join() returned %d", err);
        }
    }

protected:
    virtual int run()=0; 
    int m_threadstate; 

private:
    int initThread();
    static void *startThread(void*);

private:
    pthread_t m_threadID;
    pthread_attr_t m_attr;
};

//Ack thread
class CThdBottomHalfAnsThread: public CLocalMysqlThread {
public:
    int run();

    bool push(const CThdKey &thd) {
        return m_queue.postmsg(thd);
    }

    virtual ~CThdBottomHalfAnsThread() {}

private:
    typedef CTMsgObjQueueCond<CThdKey> MsgQueue_t;
    MsgQueue_t m_queue;
};

typedef std::unordered_map<uint64_t, std::pair<time_t,Thd_Trans_binlog_info>> AckMap;

struct AckInfo {
  uint64_t server_id;
  time_t ack_time;
  Thd_Trans_binlog_info ack_pos;
};

class Ack_container {
public:
  Ack_container() {
    m_container.clear();
  }

  typedef AckMap::iterator container_iter;
  container_iter min_ack();

  void resize();

  void process(const Thd_Trans_binlog_info &new_ack_info, uint64_t thread_id, time_t ack_time);

  void copy(std::vector<AckInfo> &infos);

private:
  AckMap m_container;
  AckInfo m_one_slave_info;
  CTMutex m_mutex;
};

class CThdBottomHalf: public CUdpServer, public CLocalMysqlThread {
public:
    CThdBottomHalf(const char*ip, unsigned short localPort, int threadnum)
                   :m_ip(ip), m_port(localPort), m_threadNum(threadnum > 16 ? 16 : threadnum),
                   timeout_thrdid(0),m_stop_all(false), m_ansThread(NULL) {
        /*
          The same number of CThdBottomHalfAnsThread threads as the NO. of
          worker threads in thread pool, but these answering threads work for
          worker threads of each and every worker threads of all thread groups,
          they are not further assigned to each group.
          One thread group
          can have as many threads as needed, as long as total number of worker
          threads don't exceed 'thread_pool_max_threads'. All such worker
          threads serve client connections.
        */
        m_ansThread = new CThdBottomHalfAnsThread[m_threadNum];
    }

    void pop_all();

    void stop_all() {
      //Pop out all tasks before stopping answer threads
      pop_all();

      // stop all answering threads.
      for (int i = 0; i < m_threadNum; ++i) {
        m_ansThread[i].stop();
      }

      // stop udp svr thread. this won't be able to stop udp svr thread
      // because loopdealreq() is doing a blocking call (recvfrom)
      // which won't return. need to do timed poll() and check the stop flag.
      // this is trivial because udp server thread doesn't call
      // my_thread_init/end() anyway.
      stop_udpsvr();

      this->stop();// stop the timeout thread.

      // Join all threads to avoid resource leaks.
      for (int i = 0; i < m_threadNum; ++i) {
        m_ansThread[i].join();
      }

      int err = pthread_join(timeout_thrdid, NULL);
      if (err) {
        fprintf(stderr, "Fail to join timeout thread.\n");
      }
    }

    bool init() {
        if (open(m_ip.c_str(), m_port) < 0) {
            return false;
        }

        for (int i = 0; i < m_threadNum; ++i) {
            m_ansThread[i].start();
        }

        start(); 

        int err = pthread_create(&timeout_thrdid, NULL, CThdBottomHalf::threadfun_do_timeout, this);
        if (err) {
          fprintf(stderr, "Fail to create thread threadfun_do_timeout\n");
        }

        return true;
    }

    // override UdpServer::do_request, handle slave acknowledgement of binlog
    // receipt.
    bool do_request(const char* buf, int len, const char* ip);

    static void* threadfun_do_timeout(void* arg) {
        ((CThdBottomHalf*) arg)->do_timeout_loop();
        return NULL;
    }

    void do_timeout_loop();

    size_t pop_processed();

    int run() {
        loopdealreq();
        return 0;
    }

    bool saveThd(enum enum_server_command command, THD *thd, bool error, bool & haveGetAns);

    // Remove the thd from m_thdContainer, if it has already been dispatched to
    // an answering thread, note it down in m_removed_thds for later checks by the
    // answering thread.
    bool remove_thd(const THD *thd);

    void dealBinlogPosAns(const Thd_Trans_binlog_info &ack_info);

    ~CThdBottomHalf() {}

    void set_thd_error_server_stop(THD *thd);

    void reset_answer();

    Ack_container ack_container;
private:
    // ipV4 address
    const std::string m_ip;
    // port is in host byte order.
    const unsigned short m_port;

    const int m_threadNum;
    pthread_t timeout_thrdid;
    CTMutex m_mutex;
    Thd_Trans_binlog_info m_newstBinlogInfoAns;
    bool m_stop_all;//weather stop mysqld
    typedef std::deque<CThdKey> ThdQueue_t;
    ThdQueue_t m_thdContainer;

    CThdBottomHalfAnsThread *m_ansThread;
};


extern ulonglong sqlasyn_get_slave_ans;
extern ulonglong sqlasyn_get_slave_ans_skip;
extern ulonglong sqlasyn_deal_trx_by_ans;
extern ulonglong sqlasyn_deal_trx_by_fast_ans;
extern ulonglong sqlasyn_exceed_warn_num;
extern ulonglong sqlasyn_timeout_num;

#endif /* THD_BOTTOM_HALF_H_ */

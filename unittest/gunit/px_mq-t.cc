/* Simple unit tests for message queue. */

#include "my_config.h"

#include <gtest/gtest.h>
#include <sys/types.h>
#include <random>

#include "sha2.h"           // EVP_*
#include "sql/sql_class.h"  // THD

#include "unittest/gunit/test_utils.h"    // Server_initializer
#include "unittest/gunit/thread_utils.h"  // Thread

#include "sql/parallel_execution/px.h"
#include "sql/parallel_execution/px_mq.h"

namespace pq_mq_unittest {

using my_testing::Server_initializer;
using thread::Thread;

class PX_mq_test : public ::testing::Test {
 protected:
  void SetUp() override { initializer.SetUp(); }
  void TearDown() override { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

class Message {
 public:
  Size get_length() const { return m_length; }
  char *get_payload() const { return m_payload.get(); }

  void reserve(int length) {
    m_length = length;
    m_payload.reset(new char[m_length]);
  }

 private:
  size_t m_length;
  std::unique_ptr<char> m_payload;
};

class Message_store {
 public:
  Message_store() {
    m_sha1_context = EVP_MD_CTX_create();
    EVP_DigestInit_ex(m_sha1_context, EVP_sha1(), nullptr);
    m_count = 0;
  }
  ~Message_store() {
    EVP_MD_CTX_destroy(m_sha1_context);
    m_sha1_context = nullptr;
  }
  virtual bool has_next() const { return false; }
  virtual Message next() { return {}; }
  virtual bool add(Size, void *) { return false; }

  std::string digest() {
    unsigned char buf[64];
    unsigned int hash_size;
    EVP_DigestFinal_ex(m_sha1_context, buf, &hash_size);
    std::string s((const char *)buf, 64);
    return s;
  }
  size_t size() const { return m_count; }

 protected:
  void update(Size length, void *data) {
    EVP_DigestUpdate(m_sha1_context, data, length);
    m_count++;
  }

 private:
  EVP_MD_CTX *m_sha1_context;
  size_t m_count;
};

class Message_store_random : public Message_store {
 public:
  Message_store_random(size_t count, size_t max_length)
      : m_count(count),
        m_maxlen(max_length),
        m_n(),
        m_rd(),
        m_gen(m_rd()),
        m_distrib(0, m_maxlen - 1) {}
  ~Message_store_random() {}

  bool has_next() const override { return m_n < m_count; }

  Message next() override {
    assert(m_n < m_count);
    m_n++;

    char c = s_charset[m_n % s_charset.size()];

    int len = m_distrib(m_gen);

    Message message;
    message.reserve(len);
    char *buf = message.get_payload();
    for (auto j = 0; j < len; j++) {
      buf[j] = c;
    }

    update(message.get_length(), message.get_payload());

    return message;
  }

 private:
  size_t m_count;
  size_t m_maxlen;

  size_t m_n;
  std::random_device m_rd;
  std::mt19937 m_gen;
  std::uniform_int_distribution<int> m_distrib;

  static std::array<char, 2> s_charset;
};

class Message_store_checksum : public Message_store {
 public:
  bool add(Size length, void *data) override {
    update(length, data);
    return false;
  }
};

std::array<char, 2> Message_store_random::s_charset = {'x', 'y'};

class Mock_worker_handle : public PX_worker_handle {
 public:
  Mock_worker_handle(PX_handle_status status)
      : PX_worker_handle(nullptr), m_status(status) {}

  void die() { m_status = KILLED; }
  void start() { m_status = STARTED; }

  // Mock worker status
  virtual PX_handle_status check_worker_status() { return m_status; }

 private:
  PX_handle_status m_status;
};

class PX_thread : public Thread {
 public:
  void init(THD *thd, PX_mq *mq, Mock_worker_handle *me,
            Mock_worker_handle *peer, Message_store *store) {
    m_mq = mq;
    m_me = me;
    m_peer = peer;
    m_thd = thd;
    m_proc_me.reset(new PX_proc(m_thd));
    m_store = store;
  }

 protected:
  THD *m_thd;
  std::unique_ptr<PX_proc> m_proc_me;
  PX_mq *m_mq;
  Mock_worker_handle *m_me;
  Mock_worker_handle *m_peer;
  Message_store *m_store;
};

typedef size_t checksum_t;
class Checksum {
 public:
  virtual void update(const Message &message) = 0;
};

class Sender_thread : public PX_thread {
 public:
  void run() override {
    m_me->start();

    // Attach to mq as sender.
    m_mq->set_sender(m_proc_me.get());
    PX_mq_handle handle(m_mq, m_proc_me.get(), malloc, free);
    handle.set_worker_handle(m_peer);

    // Send messages
    while (m_store->has_next()) {
      Message message = m_store->next();
      PX_mq_result res = handle.send(message.get_length(),
                                     message.get_payload(), /*no_wait=*/false);
      if (res != PX_MQ_SUCCESS) {
        break;
      }
    }

    handle.detach();
    m_me->die();
  }
};

class Receiver_thread : public PX_thread {
 public:
  void run() override {
    // Indicate peer is started.
    m_me->start();

    // Attach to mq as receiver.
    m_mq->set_receiver(m_proc_me.get());
    PX_mq_handle handle(m_mq, m_proc_me.get(), malloc, free);
    handle.set_worker_handle(m_peer);

    // Receive messages.
    for (;;) {
      Size length;
      void *payload;
      PX_mq_result res = handle.receive(&length, &payload, /*nowait=*/false);
      if (res != PX_MQ_SUCCESS) {
        break;
      }
      m_store->add(length, payload);
    }

    handle.detach();
    m_me->die();
  }
};

TEST_F(PX_mq_test, SingleThread) {
  // s - sender, r - receiver

  THD thd_s(false), thd_r(false);
  thd_s.server_id = 1;
  thd_s.set_new_thread_id();
  thd_r.server_id = 1;
  thd_r.set_new_thread_id();

  Mock_worker_handle wh_s(NOT_YET_STARTED);
  Mock_worker_handle wh_r(NOT_YET_STARTED);

  // A sequence of crafted messages ('1' 'aaaaaaaaaa' '' '4444') will be
  // sent through the 32-byte queue. They are 8-byte (MAXIMUM_ALIGNOF) aligned:
  // 01000000000000 31xxxxxxxxxxxxxx 0a00000000000000 6161616161616161
  // 6161xxxxxxxxxx 0000000000000000 0400000000000000 34343434xxxxxxxx

  const size_t RING_SIZE = 32;
  //std::unique_ptr<char> ptr(new char[RING_SIZE]);
  PX_mq mq(RING_SIZE, malloc, free);
  mq.init();
  //memset(ptr.get(), 0xff, RING_SIZE);

  PX_proc proc_s(&thd_s);
  PX_proc proc_r(&thd_r);

  // Attach sender.
  mq.set_sender(&proc_s);
  PX_mq_handle mqh_s(&mq, &proc_s, malloc, free);
  mqh_s.set_worker_handle(&wh_r);

  // Attach receiver.
  mq.set_receiver(&proc_r);
  PX_mq_handle mqh_r(&mq, &proc_r, malloc, free);
  mqh_r.set_worker_handle(&wh_s);

  // Indicate worker start.
  wh_s.start();
  wh_r.start();

  Size length_s;
  Size length_r;
  const void *payload_s;
  void *payload_r;
  PX_mq_result res_s;
  PX_mq_result res_r;

  // A too-big message.
  initializer.set_expected_error(ER_DATA_OUT_OF_RANGE);
  res_s = mqh_s.send(ULONG_MAX, (const void *)1, /*no_wait=*/false);
  EXPECT_EQ(PX_MQ_ERROR, res_s);
  initializer.set_expected_error(ER_UNKNOWN_ERROR);

  // A small message by blocking interface.
  std::string message_1s("1");
  length_s = message_1s.size();
  payload_s = message_1s.c_str();

  res_s = mqh_s.send(length_s, payload_s, /*no_wait=*/false);
  EXPECT_EQ(PX_MQ_SUCCESS, res_s);

  res_r = mqh_r.receive(&length_r, &payload_r, /*nowait=*/false);
  EXPECT_EQ(PX_MQ_SUCCESS, res_r);

  EXPECT_EQ(length_s, length_r);
  std::string message_1r((const char *)payload_r, length_r);
  EXPECT_EQ(message_1s, message_1r);

  // A wrapped message by non-blocking interface.
  // Non-blocking is required to switch between the handles.
  std::string message_2s("aaaaaaaaaa");
  length_s = message_2s.size();
  payload_s = message_2s.c_str();

  res_s = mqh_s.send(length_s, payload_s, /*no_wait=*/true);
  EXPECT_EQ(PX_MQ_WOULD_BLOCK, res_s);

  res_r = mqh_r.receive(&length_r, &payload_r, /*nowait=*/true);
  EXPECT_EQ(PX_MQ_WOULD_BLOCK, res_r);

  res_s = mqh_s.send(length_s, payload_s, /*no_wait=*/true);
  EXPECT_EQ(PX_MQ_SUCCESS, res_s);

  res_r = mqh_r.receive(&length_r, &payload_r, /*nowait=*/true);
  EXPECT_EQ(PX_MQ_SUCCESS, res_r);

  EXPECT_EQ(length_s, length_r);
  std::string message_2r((const char *)payload_r, length_r);
  EXPECT_EQ(message_2s, message_2r);

  // An empty messsage. Received payload points right after the length word.
  length_s = 0;
  payload_s = (const void *)0;
  length_r = 1;
  payload_r = (void *)1;

  res_s = mqh_s.send(length_s, payload_s, /*no_wait=*/false);
  EXPECT_EQ(PX_MQ_SUCCESS, res_s);

  res_r = mqh_r.receive(&length_r, &payload_r, /*nowait=*/false);
  EXPECT_EQ(PX_MQ_SUCCESS, res_r);

  EXPECT_EQ(length_s, length_r);
  EXPECT_NE(payload_r, (void *)1);

  // The fourth message by blocking interface.
  std::string message_4s("4444");
  length_s = message_4s.size();
  payload_s = message_4s.c_str();

  res_s = mqh_s.send(length_s, payload_s, /*no_wait=*/false);
  EXPECT_EQ(PX_MQ_SUCCESS, res_s);

  res_r = mqh_r.receive(&length_r, &payload_r, /*nowait=*/false);
  EXPECT_EQ(PX_MQ_SUCCESS, res_r);

  EXPECT_EQ(length_s, length_r);
  std::string message_4r((const char *)payload_r, length_r);
  EXPECT_EQ(message_4s, message_4r);

  // Cleanup.

  mqh_s.detach();
  wh_s.die();

  mqh_r.detach();
  wh_r.die();
}

TEST_F(PX_mq_test, Concurrent) {
  THD sender_thd(false), receiver_thd(false);
  sender_thd.server_id = 1;
  sender_thd.set_new_thread_id();
  receiver_thd.server_id = 1;
  receiver_thd.set_new_thread_id();

  Mock_worker_handle sender_handle(NOT_YET_STARTED);
  Mock_worker_handle receiver_handle(NOT_YET_STARTED);

  const size_t RING_SIZE = 1024;
  //std::unique_ptr<char> ptr(new char[RING_SIZE]);
  PX_mq mq(RING_SIZE, malloc, free);
  mq.init();

  Sender_thread sender;
  Receiver_thread receiver;

  int message_count = 1000;
  int max_length = 65536;
  Message_store_random store_s(message_count, max_length);
  Message_store_checksum store_r;

  sender.init(&sender_thd, &mq, &sender_handle, &receiver_handle, &store_s);
  receiver.init(&receiver_thd, &mq, &receiver_handle, &sender_handle, &store_r);

  sender.start();
  receiver.start();

  sender.join();
  receiver.join();

  EXPECT_EQ(message_count, store_s.size());
  EXPECT_EQ(store_s.size(), store_r.size());
  EXPECT_EQ(store_s.digest(), store_r.digest());
}

}  // namespace pq_mq_unittest

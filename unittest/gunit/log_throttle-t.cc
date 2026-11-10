/* Copyright (c) 2013, 2022, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <gtest/gtest.h>
#include <sys/types.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>

#include "my_systime.h"
#include "my_thread.h"
#include "mysql/psi/mysql_mutex.h"
#include "sql/log.h"
#include "sql/sql_class.h"
#include "unittest/gunit/test_utils.h"

namespace log_throttle_unittest {

using my_testing::Server_initializer;

int summary_count = 0;
char last_query[10];

static bool slow_logger(THD *, const char *query, size_t, bool, ulonglong,
                        ulonglong) {
  summary_count++;
  strcpy(last_query, query);
  return false;
}

class LogThrottleTest : public ::testing::Test {
 protected:
  void SetUp() override {
    initializer.SetUp();
    mysql_mutex_init(0, &m_mutex, MY_MUTEX_INIT_FAST);
    summary_count = 0;
  }

  void TearDown() override {
    initializer.TearDown();
    mysql_mutex_destroy(&m_mutex);
  }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
  mysql_mutex_t m_mutex;
};

// Slow_log_throttles test cases starts from here.

/*
  Test basic functionality - throttling, eligibility, printing of summary of
                             Slow_log_throttle.
*/
TEST_F(LogThrottleTest, SlowLogBasic) {
  ulong threshold = 2;
  ulong window = 1000000;
  Slow_log_throttle throttle(&threshold, &m_mutex, window, slow_logger, "%lu");

  // Should not be throttled
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));

  // Should not be throttled, not eligible
  EXPECT_FALSE(throttle.log(thd(), false));

  // Flush and check that summary was not printed
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(0, summary_count);

  // Should be throttled
  EXPECT_TRUE(throttle.log(thd(), true));

  // Flush and check that summary was printed
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(1, summary_count);

  // Flush and check that summary was not printed again
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(1, summary_count);

  // Get another summary printed
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(2, summary_count);
}

// Test changes to the threshold level of slow logger
TEST_F(LogThrottleTest, SlowLogThresholdChange) {
  ulong threshold = 2;
  ulong window = 1000000;
  Slow_log_throttle throttle(&threshold, &m_mutex, window, slow_logger, "%lu");

  // Reach threshold
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));

  // Flush and check that summary was not printed
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(0, summary_count);

  // Reducing the threshold must not invent suppressed entries.
  threshold = 1;
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(0, summary_count);

  // Increase threshold and reach it
  threshold = 3;
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));

  // Flush and check that summary was not printed
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(0, summary_count);
}

// Test number of suppressed messages written by slow logger
TEST_F(LogThrottleTest, SlowLogSuppressCount) {
  ulong threshold = 1;
  ulong window = 1000000;
  Slow_log_throttle throttle(&threshold, &m_mutex, window, slow_logger, "%lu");

  // Suppress 3 events
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(1, summary_count);
  EXPECT_STREQ("3", last_query);

  // Suppress 2 events
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), false));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), false));
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(2, summary_count);
  EXPECT_STREQ("2", last_query);
}
// End of Slow_log_throttles test cases.

TEST_F(LogThrottleTest, ActualSuppressionAfterRateChange) {
  for (ulong new_rate : {0UL, 2UL, 5UL, 7UL, 100UL}) {
    SCOPED_TRACE(new_rate);
    ulong threshold = 5;
    Slow_log_throttle throttle(&threshold, &m_mutex, 60000000, slow_logger,
                               "%lu");
    for (int i = 0; i < 40; ++i)
      EXPECT_EQ(i >= 5, throttle.log(thd(), true));
    threshold = new_rate;
    EXPECT_TRUE(throttle.flush(thd()));
    EXPECT_STREQ("35", last_query);
    EXPECT_FALSE(throttle.flush(thd()));
    if (new_rate == 100) {
      // The original count (40) was not reset: exactly 60 slots remain.
      for (int i = 0; i < 60; ++i) EXPECT_FALSE(throttle.log(thd(), true));
      EXPECT_TRUE(throttle.log(thd(), true));
    } else if (new_rate > 0) {
      // Preserve the original reset on flush when count exceeds the rate.
      for (ulong i = 0; i < new_rate; ++i)
        EXPECT_FALSE(throttle.log(thd(), true));
      EXPECT_TRUE(throttle.log(thd(), true));
    } else {
      for (int i = 0; i < 10; ++i) EXPECT_FALSE(throttle.log(thd(), true));
    }
  }
}

static ulonglong last_lock_time = 0, last_exec_time = 0;
static bool timing_logger(THD *, const char *query, size_t, bool,
                          ulonglong lock_time, ulonglong exec_time) {
  last_lock_time = lock_time;
  last_exec_time = exec_time;
  return slow_logger(nullptr, query, 0, true, lock_time, exec_time);
}

TEST_F(LogThrottleTest, SummaryRetiresBothTimingTotals) {
  ulong threshold = 1;
  Slow_log_throttle throttle(&threshold, &m_mutex, 60000000, timing_logger,
                             "%lu");
  EXPECT_FALSE(throttle.log(thd(), true));
  thd()->start_utime = my_micro_time() - 60000000;
  thd()->pop_lock_usec(7);
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(7ULL, last_lock_time);
  EXPECT_GE(last_exec_time, 60000000ULL);
  EXPECT_FALSE(throttle.flush(thd()));

  EXPECT_FALSE(throttle.log(thd(), true));
  thd()->start_utime = my_micro_time() - 1000;
  const ulonglong start = thd()->start_utime;
  thd()->pop_lock_usec(3);
  EXPECT_TRUE(throttle.log(thd(), true));
  const ulonglong elapsed = my_micro_time() - start;
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_STREQ("1", last_query);
  EXPECT_EQ(3ULL, last_lock_time);
  EXPECT_GE(last_exec_time, 1000ULL);
  EXPECT_LE(last_exec_time, elapsed);
  EXPECT_FALSE(throttle.flush(thd()));
}

TEST_F(LogThrottleTest, ExpiryDrainsStatisticsAfterRateIncrease) {
  ulong threshold = 1;
  Slow_log_throttle throttle(&threshold, &m_mutex, 100000, timing_logger,
                             "%lu");
  EXPECT_FALSE(throttle.log(thd(), true));
  thd()->start_utime = my_micro_time();
  thd()->pop_lock_usec(7);
  EXPECT_TRUE(throttle.log(thd(), true));
  threshold = 100;
  std::this_thread::sleep_for(std::chrono::milliseconds(110));
  EXPECT_FALSE(throttle.log(thd(), false));
  EXPECT_EQ(1, summary_count);
  EXPECT_STREQ("1", last_query);
  EXPECT_EQ(7ULL, last_lock_time);
  EXPECT_FALSE(throttle.log(thd(), false));
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(1, summary_count);
  threshold = 1;
  EXPECT_FALSE(throttle.log(thd(), true));
  thd()->start_utime = my_micro_time();
  thd()->pop_lock_usec(3);
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(3ULL, last_lock_time);
}

static std::atomic<ulonglong> reported_count{0}, reported_lock_time{0};
static bool concurrent_logger(THD *, const char *query, size_t, bool,
                              ulonglong lock_time, ulonglong) {
  reported_count.fetch_add(std::strtoull(query, nullptr, 10));
  reported_lock_time.fetch_add(lock_time);
  return false;
}

TEST_F(LogThrottleTest, ConcurrentSummaryConservesActualSuppression) {
  ulong threshold = 7;
  Slow_log_throttle throttle(&threshold, &m_mutex, 60000000, concurrent_logger,
                             "%lu");
  reported_count = 0;
  reported_lock_time = 0;
  std::atomic<ulonglong> actual_count{0}, actual_lock_time{0};
  std::vector<std::thread> workers;
  for (unsigned i = 0; i < 8; ++i) {
    workers.emplace_back([&, i] {
      my_thread_init();
      {
        THD local(false);
        for (int j = 0; j < 2000; ++j) {
          local.start_utime = my_micro_time();
          local.pop_lock_usec(i + 1);
          if (throttle.log(&local, true)) {
            actual_count.fetch_add(1);
            actual_lock_time.fetch_add(i + 1);
          }
          if (j % 17 == 0) throttle.flush(&local);
        }
      }
      my_thread_end();
    });
  }
  for (auto &worker : workers) worker.join();
  throttle.flush(thd());
  EXPECT_GT(actual_count.load(), 0ULL);
  EXPECT_EQ(actual_count.load(), reported_count.load());
  EXPECT_EQ(actual_lock_time.load(), reported_lock_time.load());
  EXPECT_FALSE(throttle.flush(thd()));
}

}  // namespace log_throttle_unittest

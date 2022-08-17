
#include <gtest/gtest.h>
#include <sys/time.h>
#include <cstdio>
#include <string>
#include <vector>
#include "my_config.h"
#include "sql/binlog.h"
#include "sql/mysqld.h"

namespace BinlogTest_unittest {
class BinlogTest : public ::testing::Test {
 protected:
  BinlogTest() : mysql_bin_log(&sync_binlog_period) {}
  virtual void SetUp() {}

  MYSQL_BIN_LOG mysql_bin_log;
};

TEST_F(BinlogTest, ParseBinlogSuffixTest) {
  char *log_file_name_pointer = mysql_bin_log.get_log_fname();
  ulong value;
  printf("current log_file_name=%s\n", log_file_name_pointer);

  // not init
  value = mysql_bin_log.get_binlog_suffix_idx();
  EXPECT_EQ(ulong(0), value);

  // generate new name
  std::vector<std::string> names_to_del;
  char new_name[FN_REFLEN];
  const char *log_name = "./mysql-bin";
  EXPECT_EQ(0, mysql_bin_log.generate_new_name(new_name, log_name));
  EXPECT_EQ(std::string(log_name) + ".000001", new_name);
  names_to_del.push_back(new_name);
  mysql_bin_log.update_bin_suffix_idx();
  EXPECT_EQ(ulong(1), mysql_bin_log.get_binlog_suffix_idx());

  EXPECT_EQ(0, mysql_bin_log.generate_new_name(new_name, log_name));
  EXPECT_EQ(std::string(log_name) + ".000002", new_name);
  names_to_del.push_back(new_name);
  mysql_bin_log.update_bin_suffix_idx();
  EXPECT_EQ(ulong(2), mysql_bin_log.get_binlog_suffix_idx());

  // cleanup no files really created
  /*
  std::vector<std::string>::iterator to_del_itor = names_to_del.begin();
  while (to_del_itor != names_to_del.end()) {
    EXPECT_EQ(0, remove((*to_del_itor).c_str()));
    ++to_del_itor;
  }
  */
}

long GetMicrotime() {
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

TEST_F(BinlogTest, PerfTest) {
  // default we do not run the perf test, if need to run, uncomment the code
  // below
  /*
  using namespace std;
  //ulong value;
  const char *log_name = "./perf-test-mysql-bin";
  char ext_buf[FN_REFLEN];
  char new_name[FN_REFLEN];

  //const int FILE_NUM_MAX = 100000;
  const ulong FILE_NUM_MAX = 5000;
  vector<string> tmp_files;
  //prepare file
  string pre_fix(log_name);
  int fd = 0;
  for (ulong i = 1; i <= FILE_NUM_MAX; ++i) {
    EXPECT_GE(sprintf(ext_buf, "perf-test-mysql-bin.%06lu", i), 0);
    fd = open(ext_buf, O_RDWR|O_CREAT, 0755);
    EXPECT_GE(fd, 0);
    tmp_files.push_back(ext_buf);
  }
  printf("******%lu file created******\n", FILE_NUM_MAX);

  //test scan
  const int try_times = 500;
  printf("log_file_name=%s\n", mysql_bin_log.get_log_fname());
  printf("suffix=%lu\n", mysql_bin_log.get_binlog_suffix_idx());
  ulong start_time = GetMicrotime();
  printf("[BEGIN SCAN] time=%lu\n", start_time);
  for (int i = 0; i < try_times; ++i) {
    EXPECT_EQ(0, mysql_bin_log.generate_new_name(new_name, log_name));
    //printf("new_name=%s\n", new_name);
  }
  ulong end_time = GetMicrotime();
  printf("[END SCAN] time=%lu\n", end_time);
  printf("[SCAN TIMES=%d cost total=%lu avg=%f\n]", try_times,
         (end_time - start_time), (double(end_time) - double(start_time)) /
  try_times);

  //test given current name
  printf("*******************************\n");
  EXPECT_GE(sprintf(mysql_bin_log.get_log_fname(),
  "./perf-test-mysql-bin.%06lu", FILE_NUM_MAX), 0); printf("log_file_name=%s\n",
  mysql_bin_log.get_log_fname()); printf("suffix=%lu\n",
  mysql_bin_log.get_binlog_suffix_idx()); start_time = GetMicrotime();
  printf("[BEGIN DIRECT] time=%lu\n", start_time);
  for (int i = 0; i < try_times; ++i) {
    EXPECT_EQ(0, mysql_bin_log.generate_new_name(new_name, log_name));
    mysql_bin_log.update_bin_suffix_idx();
    //printf("new_name=%s\n", new_name);
  }
  end_time = GetMicrotime();
  printf("[END DIRECT] time=%lu\n", end_time);
  printf("[DIRECT TIMES=%d cost total=%lu avg=%f\n]", try_times,
         (end_time - start_time), (double(end_time) - double(start_time)) /
  try_times);
  //delete file
  vector<string>::iterator start = tmp_files.begin();
  while (start != tmp_files.end()) {
    EXPECT_EQ(0, remove((*start).c_str()));
    ++start;
  }
  printf("******%lu file deleted******\n", FILE_NUM_MAX);
  */
}

}  // end of namespace BinlogTest_unittest

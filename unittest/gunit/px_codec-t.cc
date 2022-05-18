/* Simple unit tests for px_codec. */
#include "my_config.h"
#include <gtest/gtest.h>
#include <sys/types.h>
#include "sql/sql_class.h"  // THD
#include "unittest/gunit/test_utils.h"    // Server_initializer
#include "unittest/gunit/thread_utils.h"  // Thread
#include "unittest/gunit/parsertest.h"    // parse
#include "unittest/gunit/mock_field_long.h"
#include "unittest/gunit/temptable/mock_field_varstring.h"
#include "unittest/gunit/temptable/table_helper.h" // Table_helper
#include "sql/parallel_execution/px_codec.h"  // PX_compact_codec
#include "sql/parallel_execution/px_mq.h"  // PX_iovec
#include "unittest/gunit/fake_table.h" // Fake_table

namespace px_codec_unittest {
using my_testing::Server_initializer;
using thread::Thread;
using temptable_test::Table_helper;

class PX_codec_test : public ::testing::Test {
 protected:
  void SetUp() override { initializer.SetUp(); }
  void TearDown() override { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

TEST_F(PX_codec_test, FieldList) {
  // create table t1 (c1 int not null, c2 int, c3 varchar(20) not null);
  Mock_field_long c1_s(false);
  Mock_field_long c2_s(false);
  Mock_field_varstring c3_s(nullptr, "c3", 20, false);
  Fake_TABLE t1_s(&c1_s, &c2_s, &c3_s);
  t1_s.in_use = thd();

  longlong c1 = 1;
  std::string c3 = "abc";

  // insert t1 values(1, null, "abc");
  c1_s.make_writable();
  c2_s.make_writable();
  c3_s.make_writable();
  c1_s.store(c1, false);
  c2_s.set_null();
  c3_s.store(c3.c_str(), c3.length(), system_charset_info);

  // select c1,c2,c3,"test_codec" from t1;
  std::vector<Field *> s_fields;
  s_fields.push_back(&c1_s);
  s_fields.push_back(&c2_s);
  s_fields.push_back(&c3_s);

  // Encode
  PX_compact_codec codec(thd(), /*use_item=*/false, nullptr);
  codec.init(nullptr, &s_fields);
  std::vector<PX_iovec> memory_trunks;
  codec.encode(memory_trunks);

  // Adapt encoder output for decoder input.
  Size len = 0;
  for (auto &iov : memory_trunks) {
    len += iov.len;
  }
  std::unique_ptr<char[]> ptr(new char[len]);
  uchar *data = (uchar*) ptr.get();
  Size offset = 0;
  for (auto &iov : memory_trunks) {
    memcpy(data + offset, iov.data, iov.len);
    offset += iov.len;
  }

  // Same t1 definition.
  Mock_field_long c1_r(false);
  Mock_field_long c2_r(false);
  Mock_field_varstring c3_r(nullptr, "c3", 20, false);
  Fake_TABLE t1_r(&c1_r, &c2_r, &c3_r);
  t1_r.in_use = thd();

  EXPECT_FALSE(c2_r.is_null());

  std::vector<Field *> r_fields;
  r_fields.push_back(&c1_r);
  r_fields.push_back(&c2_r);
  r_fields.push_back(&c3_r);

  // Decode
  PX_compact_codec codec_r(thd(), /*use_item=*/false, nullptr);
  codec_r.init(nullptr, &r_fields);
  codec_r.decode(data, len);

  // Check
  String tmp;
  EXPECT_EQ(c1_r.val_int(), c1);
  EXPECT_TRUE(c2_r.is_null());
  EXPECT_STREQ(c3_r.val_str(nullptr, &tmp)->c_ptr(), c3.c_str());
}

TEST_F(PX_codec_test, ItemList) {
  // create table t1 (c1 int not null, c2 int, c3 varchar(20) not null);
  Mock_field_long c1_s(false);
  Mock_field_long c2_s(false);
  Mock_field_varstring c3_s(nullptr, "c3", 20, false);
  Fake_TABLE t1_s(&c1_s, &c2_s, &c3_s);
  t1_s.in_use = thd();

  longlong c1 = 1;
  std::string c3 = "abc";

  // insert t1 values(1, null, "abc");
  c1_s.make_writable();
  c2_s.make_writable();
  c3_s.make_writable();
  c1_s.store(c1, false);
  c2_s.set_null();
  c3_s.store(c3.c_str(), c3.length(), system_charset_info);

  // select c1,c2,c3,"test_codec" from t1;
  mem_root_deque<Item *> item_list(thd()->mem_root);
  item_list.push_back(new Item_field(&c1_s));
  item_list.push_back(new Item_field(&c2_s));
  item_list.push_back(new Item_field(&c3_s));
  item_list.push_back(new Item_string("test_codec", strlen("test_codec"), system_charset_info));

  // Encode
  PX_compact_codec codec(thd(), /*use_item=*/true, nullptr);
  codec.init(&item_list, nullptr);
  std::vector<PX_iovec> memory_trunks;
  codec.encode(memory_trunks);

  // Adapt encoder output for decoder input.
  Size len = 0;
  for (auto &iov : memory_trunks) {
    len += iov.len;
  }
  std::unique_ptr<char[]> ptr(new char[len]);
  uchar *data = (uchar*) ptr.get();
  Size offset = 0;
  for (auto &iov : memory_trunks) {
    memcpy(data + offset, iov.data, iov.len);
    offset += iov.len;
  }

  // Same t1 definition.
  Mock_field_long c1_r(false);
  Mock_field_long c2_r(false);
  Mock_field_varstring c3_r(nullptr, "c3", 20, false);
  Fake_TABLE t1_r(&c1_r, &c2_r, &c3_r);
  t1_r.in_use = thd();

  EXPECT_FALSE(c2_r.is_null());

  std::vector<Field *> r_fields;
  r_fields.push_back(&c1_r);
  r_fields.push_back(&c2_r);
  r_fields.push_back(&c3_r);

  // Decode
  PX_compact_codec codec_r(thd(), /*use_item=*/false, nullptr);
  codec_r.init(nullptr, &r_fields);
  codec_r.decode(data, len);

  // Check
  String tmp;
  EXPECT_EQ(c1_r.val_int(), c1);
  EXPECT_TRUE(c2_r.is_null());
  EXPECT_STREQ(c3_r.val_str(nullptr, &tmp)->c_ptr(), c3.c_str());
}

}  // namespace px_codec_unittest

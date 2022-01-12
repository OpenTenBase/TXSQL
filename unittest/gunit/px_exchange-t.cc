/* Simple unit tests for exchange operators. */

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

#include "sql/parallel_execution/px_exchange_info.h"
#include "sql/parallel_execution/px_receiver.h"
#include "sql/parallel_execution/px_sender.h"

namespace pq_exchange_unittest {
using my_testing::Server_initializer;
using thread::Thread;
using temptable_test::Table_helper;

class PX_exchange_test : public ::testing::Test {
 protected:
  void SetUp() override { initializer.SetUp(); }
  void TearDown() override { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

TEST_F(PX_exchange_test, SingleThread) {
  // s - sender, r - receiver
  // make the table.
  const char *table_name = "t1";
  Table_helper table_helper_s(table_name, thd());
  table_helper_s.add_field_long("c1", false);
  table_helper_s.add_field_varstring("c2", 20, false);
  table_helper_s.finalize();

  Field_long *f1_s = table_helper_s.field<Field_long>(0);
  Field_varstring *f2_s = table_helper_s.field<Field_varstring>(1);

  mem_root_deque<Item *> fields(thd()->mem_root);
  fields.push_back(new Item_field(f1_s));
  fields.push_back(new Item_field(f2_s));

  Table_helper table_helper_r(table_name, thd());
  table_helper_r.add_field_long("c1", false);
  table_helper_r.add_field_varstring("c2", 20, false);
  table_helper_r.finalize();

  longlong c1 = 1;
  std::string c2 = "abc";
  f1_s->store(c1, false);
  f2_s->store(c2.c_str(), c2.length(), system_charset_info);

  PX_exchange_info pei(thd(), /*exchange_type=*/PX_GATHER_EXCHANGE, /*channel_type=*/PX_MQ_CHANNEL,
                       /*senders=*/1, /*receivers=*/1, /*exchange_format=*/PX_COMPACT_ROW, /*need_materialize=*/false);
  PX_sender sender(/*sender_no=*/0, &pei, thd(), table_helper_s.table(), &fields, nullptr);
  PX_receiver receiver(/*receiver_no=*/0, &pei, thd(), table_helper_r.table());

  pei.init();
  sender.init();
  receiver.init();

  sender.attach();
  receiver.attach();

  sender.send();
  sender.end();

  receiver.next();
  receiver.end();

  pei.clean();

  String tmp;
  Field_long *f1_r = table_helper_r.field<Field_long>(0);
  Field_varstring *f2_r = table_helper_r.field<Field_varstring>(1);

  EXPECT_EQ(f1_r->val_int(), c1);
  EXPECT_STREQ(f2_r->val_str(nullptr, &tmp)->c_ptr(), c2.c_str());
}

}  // namespace pq_mq_unittest

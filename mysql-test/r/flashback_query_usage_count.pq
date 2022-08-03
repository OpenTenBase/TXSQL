set global innodb_backquery_enable=ON;
Ensure that some messages get logged to the error log file
include/assert_grep.inc [TXSQL: backquery is enabled.]
set global innodb_backquery_enable=OFF;
include/assert_grep.inc [TXSQL: backquery is disabled.]
include/assert.inc [flashback_query_counts = 0]
create table t1(id int, c1 int);
include/assert.inc [flashback_query_counts = 1]
drop table if exists t1;

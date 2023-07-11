set global innodb_backquery_enable=ON;
Ensure that some messages get logged to the error log file
include/assert_grep.inc [TXSQL: backquery is enabled.]
set global innodb_backquery_enable=OFF;
include/assert_grep.inc [TXSQL: backquery is disabled.]
include/assert.inc [flashback_query_counts = 0]
create table t1(id int, c1 int);
include/assert.inc [flashback_query_counts = 1]
set global innodb_backquery_persistent=ON;
show global status like '%txsql_persistent_flashback_query_counts';
Variable_name	Value
Innodb_txsql_persistent_flashback_query_counts	1
set global innodb_backquery_persistent=OFF;
show global status like '%txsql_persistent_flashback_query_counts';
Variable_name	Value
Innodb_txsql_persistent_flashback_query_counts	0
drop table if exists t1;

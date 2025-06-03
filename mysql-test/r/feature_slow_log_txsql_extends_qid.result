TRUNCATE TABLE mysql.slow_log;
SET @old_log_output = @@global.log_output;
SET @old_slow_query_log = @@global.slow_query_log;
SET @old_slow_query_log_file = @@global.slow_query_log_file;
SET @old_long_query_time = @@global.long_query_time;
SET @old_txsql_extend_slow_log_level = @@global.txsql_extend_slow_log_level;
SET GLOBAL slow_query_log_file = '.../log/qid_slow.log';
SET GLOBAL log_output = 'FILE';
SET GLOBAL slow_query_log = 'ON';
SET SESSION long_query_time = 0;
SELECT @@global.slow_query_log_file;
SET SESSION txsql_extend_slow_log_level = 1;
SELECT @@SESSION.txsql_extend_slow_log_level;
@@SESSION.txsql_extend_slow_log_level
1
/*4:16777217-1656991730-1*/ select sleep(1);
sleep(1)
0
/*4:16777217-1656991730-2*/select @@version_comment limit 1;
@@version_comment
#
select /*4:16777217-1656991730-3*/ sleep(1);
sleep(1)
0
/*4:16777217-1656994730-2*/ /*sets:allsets */select @@version_comment limit 1;
@@version_comment
#
/*sets:allsets */ /*4:16777217-1656995730-2*/select @@version_comment limit 1;
@@version_comment
#
select @@version_comment limit 1;
@@version_comment
#
/*sets:allsets */ /*4:16777217-1656995731-1*/select @@version_comment limit 1;
@@version_comment
#
/*4:16777a217-1656991730-1*/ select sleep(1);
sleep(1)
0
/*4:16777217-165699999999999999999999999999999999999999999999991730-1*/ select sleep(1);
sleep(1)
0
/*4:16777a217-1656991730-*/ select sleep(1);
sleep(1)
0
/*4-16777a217:1656991730-1*/ select sleep(1);
sleep(1)
0
SET SESSION txsql_extend_slow_log_level = 0;
SELECT @@SESSION.txsql_extend_slow_log_level;
@@SESSION.txsql_extend_slow_log_level
0
/*4:16777217-1656991730-1*/ select sleep(1);
sleep(1)
0
/*4:16777217-1656991730-2*/select @@version_comment limit 1;
@@version_comment
#
select /*4:16777217-1656991730-3*/ sleep(1);
sleep(1)
0
/*4:16777a217-1656991730-1*/ select sleep(1);
sleep(1)
0
/*4:16777217-165699999999999999999999999999999999999999999999991730-1*/ select sleep(1);
sleep(1)
0
/*4:16777a217-1656991730-3*2*/ select sleep(1);
sleep(1)
0
/*4-16777a217:1656991730-1*/ select sleep(1);
sleep(1)
0
CREATE TABLE test_log (sql_text TEXT);
LOAD DATA LOCAL INFILE '.../log/qid_slow.log'
  INTO TABLE test_log FIELDS TERMINATED BY '\n' LINES TERMINATED BY '\n';
select sql_text from test_log where sql_text like '%Query_id:%';
sql_text
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 4:16777217-1656991730-1
# Query_id: 4:16777217-1656991730-1
# Query_id: 4:16777217-1656991730-1
# Query_id: 4:16777217-1656991730-2
# Query_id: 4:16777217-1656991730-2
# Query_id: 4:16777217-1656991730-2
# Query_id: 4:16777217-1656991730-3
# Query_id: 4:16777217-1656991730-3
# Query_id: 4:16777217-1656994730-2
# Query_id: 4:16777217-1656995730-2
# Query_id: 4:16777217-1656995730-2
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 4:16777217-1656995731-1
# Query_id: 4:16777217-1656995731-1
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
# Query_id: 0:00-00-0
drop table test_log;
SET SESSION long_query_time = @old_long_query_time;
SET GLOBAL  slow_query_log_file = @old_slow_query_log_file;
SET GLOBAL  slow_query_log = @old_slow_query_log;
SET GLOBAL  log_output = @old_log_output;
SET GLOBAL  txsql_extend_slow_log_level= @old_txsql_extend_slow_log_level;

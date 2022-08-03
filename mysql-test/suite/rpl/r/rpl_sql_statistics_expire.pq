include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
CREATE TABLE `t1` (
`c1` int(11) AUTO_INCREMENT,
`c2` int(11),
PRIMARY KEY (c1)
) ENGINE=InnoDB;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
CREATE TABLE `t2` (
`c1` int(11),
`c2` int(11) AUTO_INCREMENT,
`c3` int(11),
PRIMARY KEY (c2, c1)
) ENGINE=InnoDB;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
set session transaction isolation level repeatable read;
insert into t1 values(10,10),(20,20), (30, 30), (40, 40);
insert t2 (c1, c3) select * from t1;
insert into t2 (c1, c3) values (1, 1);
update t1 set c2 = 10;
delete from t1;
SELECT * FROM  information_schema.CDB_SQL_STATISTICS;
DIGEST_MD5	2c5e70e946eda6f2a3f64a25216c312430709f1caf4776df7a2888782ec85cdf
AVER_AFFECTED_ROWS	3
AVER_AFFECTED_ROWS	4
AVER_AFFECTED_ROWS	4
DIGEST_MD5	648ac29b93e6a26e394fa82725aa3968c1fe12603f512fa491c50b64e2226adf
DIGEST_MD5	fc785cbe6e1d9d772af5e9de85bced702ddcd949a8657e35745cd7f7334c66d8
DIGEST_TEXT	DELETE FROM `t1`
DIGEST_TEXT	INSERT `t2` ( `c1` , `c3` ) SELECT * FROM `t1`
DIGEST_TEXT	UPDATE `t1` SET `c2` = ?
EXECUTE_COUNT	1
EXECUTE_COUNT	1
EXECUTE_COUNT	1
FIRST_UPDATE_TIMESTAMP	#
FIRST_UPDATE_TIMESTAMP	#
FIRST_UPDATE_TIMESTAMP	#
LAST_ACCESS_TIMESTAMP	#
LAST_ACCESS_TIMESTAMP	#
LAST_ACCESS_TIMESTAMP	#
LAST_AFFECTED_ROWS	3
LAST_AFFECTED_ROWS	4
LAST_AFFECTED_ROWS	4
LAST_UPDATE_TIMESTAMP	#
LAST_UPDATE_TIMESTAMP	#
LAST_UPDATE_TIMESTAMP	#
SQL_COMMAND	DELETE
SQL_COMMAND	INSERT_SELECT
SQL_COMMAND	UPDATE
STMT_BINLOG_FORMAT_IF_POSSIBLE	TRUE
STMT_BINLOG_FORMAT_IF_POSSIBLE	TRUE
STMT_BINLOG_FORMAT_IF_POSSIBLE	TRUE
TOTAL_AFFECTED_ROWS	3
TOTAL_AFFECTED_ROWS	4
TOTAL_AFFECTED_ROWS	4
set global cdb_sql_statistics_info_threshold=1;
SELECT * FROM  information_schema.CDB_SQL_STATISTICS;
DIGEST_MD5	648ac29b93e6a26e394fa82725aa3968c1fe12603f512fa491c50b64e2226adf
AVER_AFFECTED_ROWS	4
DIGEST_TEXT	DELETE FROM `t1`
EXECUTE_COUNT	1
FIRST_UPDATE_TIMESTAMP	#
LAST_ACCESS_TIMESTAMP	#
LAST_AFFECTED_ROWS	4
LAST_UPDATE_TIMESTAMP	#
SQL_COMMAND	DELETE
STMT_BINLOG_FORMAT_IF_POSSIBLE	TRUE
TOTAL_AFFECTED_ROWS	4
SELECT * FROM  information_schema.CDB_SQL_STATISTICS;
DIGEST_MD5	648ac29b93e6a26e394fa82725aa3968c1fe12603f512fa491c50b64e2226adf
AVER_AFFECTED_ROWS	4
DIGEST_TEXT	DELETE FROM `t1`
EXECUTE_COUNT	1
FIRST_UPDATE_TIMESTAMP	#
LAST_ACCESS_TIMESTAMP	#
LAST_AFFECTED_ROWS	4
LAST_UPDATE_TIMESTAMP	#
SQL_COMMAND	DELETE
STMT_BINLOG_FORMAT_IF_POSSIBLE	TRUE
TOTAL_AFFECTED_ROWS	4
set global cdb_sql_statistics_info_threshold=1;
SELECT * FROM  information_schema.CDB_SQL_STATISTICS;
DIGEST_MD5	648ac29b93e6a26e394fa82725aa3968c1fe12603f512fa491c50b64e2226adf
AVER_AFFECTED_ROWS	4
DIGEST_TEXT	DELETE FROM `t1`
EXECUTE_COUNT	1
FIRST_UPDATE_TIMESTAMP	#
LAST_ACCESS_TIMESTAMP	#
LAST_AFFECTED_ROWS	4
LAST_UPDATE_TIMESTAMP	#
SQL_COMMAND	DELETE
STMT_BINLOG_FORMAT_IF_POSSIBLE	TRUE
TOTAL_AFFECTED_ROWS	4
set global cdb_sql_statistics_info_threshold=1000;
select * from t1;
c1	c2
select * from t2;
c1	c2	c3
1	8	1
10	1	10
20	2	20
30	3	30
40	4	40
include/sync_slave_sql_with_master.inc
select * from t1;
c1	c2
select * from t2;
c1	c2	c3
1	8	1
10	1	10
20	2	20
30	3	30
40	4	40
drop table t1;
drop table t2;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

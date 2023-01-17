##################
# auto stats of rpl
##################
include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
# Auto statistics depends on the host:port info reported by the slave
show global variables like 'report_host';
Variable_name	report_host
Value	127.0.0.1
show global variables like 'report_port';
Variable_name	report_port
Value	CURRENT_PORT
show variables like '%cdb_statistics%';
Variable_name	Value
SET GLOBAL cdb_auto_statistics_enabled=1;
# check host:port of statistics node on master node
show cdb_statistics_node;
Host	Port	Status
127.0.0.1	CURRENT_PORT	NULL
show cdb_statistics_tasks;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	Status
#
# check status
#
SELECT @@cdb_auto_statistics_enabled;
@@cdb_auto_statistics_enabled
1
SELECT @@histogram_history_enabled;
@@histogram_history_enabled
0
SELECT @@histogram_statistics_concurrency;
@@histogram_statistics_concurrency
10
SELECT @@auto_stats_thread_monitor_interval;
@@auto_stats_thread_monitor_interval
1
SELECT @@auto_stats_interval_duration;
@@auto_stats_interval_duration
23
SET GLOBAL HISTOGRAM_HISTORY_ENABLED= 1;
SET GLOBAL HISTOGRAM_HISTORY_VERSIONS_LIMIT= 3;
SELECT COUNT(*) FROM information_schema.processlist WHERE user='auto_statistics_manager';
COUNT(*)
1
SELECT COUNT(*) FROM information_schema.processlist WHERE user='auto_statistics_worker';
COUNT(*)
0
#
# General test auto statistics
#
# Note: Histogram data is identified (hid) by '$."number-of-buckets-specified"' by design.
set global innodb_stats_recalc_with_histogram = on;
# empty
show cdb_statistics_tasks;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	Status
CREATE TABLE tbl1(c1 INT);
INSERT INTO tbl1(c1) VALUES (1),(1),(2),(3),(5);
INSERT INTO tbl1(c1) VALUES (1),(1),(2),(3),(5);
# test automatic histogram recalculation is triggered by INNODB
show cdb_statistics_tasks;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	Status
test	tbl1	c1	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
100
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
100
set global innodb_stats_recalc_with_histogram = default;
delete from mysql.column_statistics_history;
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 1 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c1'.
show cdb_statistics_tasks;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	Status
test	tbl1	c1	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
show cdb_statistics_tasks;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	Status
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
1
#
# Test AUTO histogram apply
#
SELECT JSON_REMOVE(HISTOGRAM, '$."last-updated"') AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
{"buckets": [[1, 5, 1.0, 4]], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "equi-height", "number-of-buckets-specified": 1}
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 USING HISTORY -1;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	The histogram on column 'c1' restored with history version.
SELECT JSON_REMOVE(HISTOGRAM, '$."last-updated"') AS HISTOGRAM FROM information_schema.COLUMN_STATISTICS WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
{"buckets": [[1, 5, 1.0, 4]], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "equi-height", "number-of-buckets-specified": 1}
# Check auto apply the latest version (-1)
# Without 'ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 USING HISTORY -1;'
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
1
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
1
# Test HISTOGRAM_HISTORY_ENABLED not effect auto statistics
SET GLOBAL HISTOGRAM_HISTORY_ENABLED= 0;
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 4 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c1'.
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
4
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
1
4
DROP TABLE tbl1;
#
# test auto statistics work status
#
CREATE TABLE tbl1(c1 INT, c2 int, c3 int);
# auto statistics: on
# creation pending status
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1,c2,c3;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c1'.
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c2'.
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c3'.
# auto statistics: off
set global cdb_auto_statistics_enabled = off;
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1,c2,c3;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram statistics created for column 'c2'.
test.tbl1	histogram	status	Histogram statistics created for column 'c3'.
# auto statistics: off
set global cdb_auto_statistics_enabled = on;
set global auto_stats_node_selection = 'LOCAL';
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1,c2,c3;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram statistics created for column 'c2'.
test.tbl1	histogram	status	Histogram statistics created for column 'c3'.
DROP TABLE tbl1;
set global cdb_auto_statistics_enabled = on;
set global auto_stats_node_selection = 'DEDICATED';
# empty
SELECT * FROM mysql.column_statistics_history;
schema_name	table_name	column_name	histogram	version	creation_time	options
SELECT * FROM information_schema.column_statistics;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME	HISTOGRAM
#
# Test RENAME COLUMN (before apply)
#
CREATE TABLE tbl1(c1 INT);
INSERT INTO tbl1(c1) VALUES(1);
# Create HISTOGRAM for tbl1
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 1 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c1'.
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
0
# Rename c1 to c2
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1";
HISTOGRAM
0
ALTER TABLE tbl1 RENAME COLUMN c1 to c2;
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c2";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1";
HISTOGRAM
0
DROP TABLE tbl1;
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1";
HISTOGRAM
0
#
# Test RENAME COLUMN (after apply)
#
CREATE TABLE tbl1(c1 INT);
INSERT INTO tbl1(c1) VALUES(1);
# Create HISTOGRAM for tbl1
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 1 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c1'.
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
1
# Rename c1 to c2
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1";
HISTOGRAM
1
ALTER TABLE tbl1 RENAME COLUMN c1 to c2;
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c2";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1";
HISTOGRAM
0
select * from performance_schema.mutex_instances where name like '%statistics%';
NAME	OBJECT_INSTANCE_BEGIN	LOCKED_BY_THREAD_ID
wait/synch/mutex/sql/LOCK_statistics_tasks_queue	TIMESTAMP	NULL
wait/synch/mutex/sql/LOCK_statistics_tasks_queue	TIMESTAMP	NULL
wait/synch/mutex/sql/LOCK_statistics_tasks_queue	TIMESTAMP	NULL
DROP TABLE tbl1;
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1";
HISTOGRAM
0
#
# Test RENAME/DROP TABLE
#
CREATE TABLE tbl1(c1 INT);
INSERT INTO tbl1(c1) VALUES(1),(1),(2),(3),(5);
# Create 3 history records for tbl1
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 1 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c1'.
#
# check information_schema
#
select * from information_schema.CDB_AUTO_STATS_TASK_STATUS;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	STATUS
test	tbl1	c1	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
select * from information_schema.CDB_AUTO_STATS_NODE_STATUS;
HOST	PORT	STATUS
127.0.0.1	CURRENT_PORT	
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 2 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c1'.
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
2
# Rename tbl1 to tbl1_renamed
RENAME TABLE tbl1 TO tbl1_renamed;
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1_renamed" AND COLUMN_NAME="c1";
HISTOGRAM
2
# DROP TABLE tbl1_renamed
DROP TABLE tbl1_renamed;
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1_renamed" AND COLUMN_NAME="c1";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1_renamed";
HISTOGRAM
0
#
# test less function of Priority_queue (multiple tables with multiple columns)
#
set global auto_stats_thread_monitor_interval = 5;
CREATE TABLE tbl1(c1 INT, c2 int);
CREATE TABLE tbl2(c1 INT, c2 int);
INSERT INTO tbl1(c1,c2) VALUES(1,1),(1,2),(2,3),(3,4),(5,6);
INSERT INTO tbl2(c1,c2) VALUES(1,1),(1,2),(2,3),(3,4),(5,6);
#
# add multiple tasks
#
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c1'.
ANALYZE TABLE tbl2 UPDATE HISTOGRAM ON c1;
Table	Op	Msg_type	Msg_text
test.tbl2	histogram	Pending	Histogram statistics creation pending for column 'c1'.
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c2;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c2'.
ANALYZE TABLE tbl2 UPDATE HISTOGRAM ON c2;
Table	Op	Msg_type	Msg_text
test.tbl2	histogram	Pending	Histogram statistics creation pending for column 'c2'.
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1,c2;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c1'.
test.tbl1	histogram	Pending	Histogram statistics creation pending for column 'c2'.
ANALYZE TABLE tbl2 UPDATE HISTOGRAM ON c1,c2;
Table	Op	Msg_type	Msg_text
test.tbl2	histogram	Pending	Histogram statistics creation pending for column 'c1'.
test.tbl2	histogram	Pending	Histogram statistics creation pending for column 'c2'.
select * from information_schema.CDB_AUTO_STATS_TASK_STATUS;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	STATUS
test	tbl1	c1	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
test	tbl1	c2	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
test	tbl2	c1	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
test	tbl2	c2	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
show cdb_statistics_tasks;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	Status
test	tbl1	c1	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
test	tbl1	c2	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
test	tbl2	c1	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
test	tbl2	c2	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	CURRENT_TIMESTAMP	0	ON_PENDING
set global auto_stats_thread_monitor_interval = 1;
show cdb_statistics_tasks;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	Status
drop table tbl1,tbl2;
SELECT * FROM mysql.column_statistics_history;
schema_name	table_name	column_name	histogram	version	creation_time	options
show cdb_statistics_tasks;
TABLE_SCHEMA	TABLE_NAME	COLUMN_NAME	CREATED_TIME	EXECUTE_TIME	LAST_EXECUTE_TIME	ATTEMPTS	Status
create table t1 (c1 varchar(10), c2 varchar(10), c3 varchar(10), c4 varchar(10), c5 varchar(10), c6 varchar(10), c7 varchar(10), c8 varchar(10), c9 varchar(10), c10 varchar(10), c11 varchar(10), c12 varchar(10), c13 varchar(10), c14 varchar(10), c15 varchar(10), c16 varchar(10), c17 varchar(10), c18 varchar(10), c19 varchar(10), c20 varchar(10), c21 varchar(10), c22 varchar(10), c23 varchar(10), c24 varchar(10), c25 varchar(10), c26 varchar(10), c27 varchar(10), c28 varchar(10), c29 varchar(10), c30 varchar(10), c31 varchar(10), c32 varchar(10), c33 varchar(10), c34 varchar(10), c35 varchar(10), c36 varchar(10), c37 varchar(10), c38 varchar(10), c39 varchar(10), c40 varchar(10), c41 varchar(10), c42 varchar(10), c43 varchar(10), c44 varchar(10), c45 varchar(10), c46 varchar(10), c47 varchar(10), c48 varchar(10), c49 varchar(10), c50 varchar(10), c51 varchar(10), c52 varchar(10), c53 varchar(10), c54 varchar(10), c55 varchar(10), c56 varchar(10), c57 varchar(10), c58 varchar(10), c59 varchar(10), c60 varchar(10), c61 varchar(10), c62 varchar(10), c63 varchar(10), c64 varchar(10), c65 varchar(10), c66 varchar(10), c67 varchar(10), c68 varchar(10), c69 varchar(10), c70 varchar(10), c71 varchar(10), c72 varchar(10), c73 varchar(10), c74 varchar(10), c75 varchar(10), c76 varchar(10), c77 varchar(10), c78 varchar(10), c79 varchar(10), c80 varchar(10), c81 varchar(10), c82 varchar(10), c83 varchar(10), c84 varchar(10), c85 varchar(10), c86 varchar(10), c87 varchar(10), c88 varchar(10), c89 varchar(10), c90 varchar(10), c91 varchar(10), c92 varchar(10), c93 varchar(10), c94 varchar(10), c95 varchar(10), c96 varchar(10), c97 varchar(10), c98 varchar(10), c99 varchar(10), c100 varchar(10), c101 varchar(10), c102 varchar(10), c103 varchar(10), c104 varchar(10), c105 varchar(10), c106 varchar(10), c107 varchar(10), c108 varchar(10), c109 varchar(10), c110 varchar(10), c111 varchar(10), c112 varchar(10), c113 varchar(10), c114 varchar(10), c115 varchar(10), c116 varchar(10), c117 varchar(10), c118 varchar(10), c119 varchar(10), c120 varchar(10), c121 varchar(10), c122 varchar(10), c123 varchar(10), c124 varchar(10), c125 varchar(10), c126 varchar(10), c127 varchar(10), c128 varchar(10));
alter table t1 add column c129 char(10) default 'abc';
insert into t1 set c1='1', c2='1', c3='1', c4='1', c5='1', c6='1', c7='1', c8='1', c9='1', c10='1', c11='1', c12='1', c13='1', c14='1', c15='1', c16='1', c17='1', c18='1', c19='1', c20='1', c21='1', c22='1', c23='1', c24='1', c25='1', c26='1', c27='1', c28='1', c29='1', c30='1', c31='1', c32='1', c33='1', c34='1', c35='1', c36='1', c37='1', c38='1', c39='1', c40='1', c41='1', c42='1', c43='1', c44='1', c45='1', c46='1', c47='1', c48='1', c49='1', c50='1', c51='1', c52='1', c53='1', c54='1', c55='1', c56='1', c57='1', c58='1', c59='1', c60='1', c61='1', c62='1', c63='1', c64='1', c65='1', c66='1', c67='1', c68='1', c69='1', c70='1', c71='1', c72='1', c73='1', c74='1', c75='1', c76='1', c77='1', c78='1', c79='1', c80='1', c81='1', c82='1', c83='1', c84='1', c85='1', c86='1', c87='1', c88='1', c89='1', c90='1', c91='1', c92='1', c93='1', c94='1', c95='1', c96='1', c97='1', c98='1', c99='1', c100='1', c101='1', c102='1', c103='1', c104='1', c105='1', c106='1', c107='1', c108='1', c109='1', c110='1', c111='1', c112='1', c113='1', c114='1', c115='1', c116='1', c117='1', c118='1', c119='1', c120='1', c121='1', c122='1', c123='1', c124='1', c125='1', c126='1', c127='1', c128='1', c129='1';
insert into t1 set c1='1', c2='1', c3='1', c4='1', c5='1', c6='1', c7='1', c8='1', c9='1', c10='1', c11='1', c12='1', c13='1', c14='1', c15='1', c16='1', c17='1', c18='1', c19='1', c20='1', c21='1', c22='1', c23='1', c24='1', c25='1', c26='1', c27='1', c28='1', c29='1', c30='1', c31='1', c32='1', c33='1', c34='1', c35='1', c36='1', c37='1', c38='1', c39='1', c40='1', c41='1', c42='1', c43='1', c44='1', c45='1', c46='1', c47='1', c48='1', c49='1', c50='1', c51='1', c52='1', c53='1', c54='1', c55='1', c56='1', c57='1', c58='1', c59='1', c60='1', c61='1', c62='1', c63='1', c64='1', c65='1', c66='1', c67='1', c68='1', c69='1', c70='1', c71='1', c72='1', c73='1', c74='1', c75='1', c76='1', c77='1', c78='1', c79='1', c80='1', c81='1', c82='1', c83='1', c84='1', c85='1', c86='1', c87='1', c88='1', c89='1', c90='1', c91='1', c92='1', c93='1', c94='1', c95='1', c96='1', c97='1', c98='1', c99='1', c100='1', c101='1', c102='1', c103='1', c104='1', c105='1', c106='1', c107='1', c108='1', c109='1', c110='1', c111='1', c112='1', c113='1', c114='1', c115='1', c116='1', c117='1', c118='1', c119='1', c120='1', c121='1', c122='1', c123='1', c124='1', c125='1', c126='1', c127='1', c128='1', c129='1';
insert into t1 set c1='6',c121='b121',c128='b128';
insert into t1 set c1='6',c121='b121',c128='b128';
insert into t1 set c1='1', c2='1', c3='1', c4='1', c5='1', c6='1', c7='1', c8='1', c9='1', c10='1', c11='1', c12='1', c13='1', c14='1', c15='1', c16='1', c17='1', c18='1', c19='1', c20='1', c21='1', c22='1', c23='1', c24='1', c25='1', c26='1', c27='1', c28='1', c29='1', c30='1', c31='1', c32='1', c33='1', c34='1', c35='1', c36='1', c37='1', c38='1', c39='1', c40='1', c41='1', c42='1', c43='1', c44='1', c45='1', c46='1', c47='1', c48='1', c49='1', c50='1', c51='1', c52='1', c53='1', c54='1', c55='1', c56='1', c57='1', c58='1', c59='1', c60='1', c61='1', c62='1', c63='1', c64='1', c65='1', c66='1', c67='1', c68='1', c69='1', c70='1', c71='1', c72='1', c73='1', c74='1', c75='1', c76='1', c77='1', c78='1', c79='1', c80='1', c81='1', c82='1', c83='1', c84='1', c85='1', c86='1', c87='1', c88='1', c89='1', c90='1', c91='1', c92='1', c93='1', c94='1', c95='1', c96='1', c97='1', c98='1', c99='1', c100='1', c101='1', c102='1', c103='1', c104='1', c105='1', c106='1', c107='1', c108='1', c109='1', c110='1', c111='1', c112='1', c113='1', c114='1', c115='1', c116='1', c117='1', c118='1', c119='1', c120='1', c121='1', c122='1', c123='1', c124='1', c125='1', c126='1', c127='1', c128='1', c129='1';
insert into t1 set c1='1', c2='1', c3='1', c4='1', c5='1', c6='1', c7='1', c8='1', c9='1', c10='1', c11='1', c12='1', c13='1', c14='1', c15='1', c16='1', c17='1', c18='1', c19='1', c20='1', c21='1', c22='1', c23='1', c24='1', c25='1', c26='1', c27='1', c28='1', c29='1', c30='1', c31='1', c32='1', c33='1', c34='1', c35='1', c36='1', c37='1', c38='1', c39='1', c40='1', c41='1', c42='1', c43='1', c44='1', c45='1', c46='1', c47='1', c48='1', c49='1', c50='1', c51='1', c52='1', c53='1', c54='1', c55='1', c56='1', c57='1', c58='1', c59='1', c60='1', c61='1', c62='1', c63='1', c64='1', c65='1', c66='1', c67='1', c68='1', c69='1', c70='1', c71='1', c72='1', c73='1', c74='1', c75='1', c76='1', c77='1', c78='1', c79='1', c80='1', c81='1', c82='1', c83='1', c84='1', c85='1', c86='1', c87='1', c88='1', c89='1', c90='1', c91='1', c92='1', c93='1', c94='1', c95='1', c96='1', c97='1', c98='1', c99='1', c100='1', c101='1', c102='1', c103='1', c104='1', c105='1', c106='1', c107='1', c108='1', c109='1', c110='1', c111='1', c112='1', c113='1', c114='1', c115='1', c116='1', c117='1', c118='1', c119='1', c120='1', c121='1', c122='1', c123='1', c124='1', c125='1', c126='1', c127='1', c128='1', c129='1';
insert into t1 set c1='1', c2='1', c3='1', c4='1', c5='1', c6='1', c7='1', c8='1', c9='1', c10='1', c11='1', c12='1', c13='1', c14='1', c15='1', c16='1', c17='1', c18='1', c19='1', c20='1', c21='1', c22='1', c23='1', c24='1', c25='1', c26='1', c27='1', c28='1', c29='1', c30='1', c31='1', c32='1', c33='1', c34='1', c35='1', c36='1', c37='1', c38='1', c39='1', c40='1', c41='1', c42='1', c43='1', c44='1', c45='1', c46='1', c47='1', c48='1', c49='1', c50='1', c51='1', c52='1', c53='1', c54='1', c55='1', c56='1', c57='1', c58='1', c59='1', c60='1', c61='1', c62='1', c63='1', c64='1', c65='1', c66='1', c67='1', c68='1', c69='1', c70='1', c71='1', c72='1', c73='1', c74='1', c75='1', c76='1', c77='1', c78='1', c79='1', c80='1', c81='1', c82='1', c83='1', c84='1', c85='1', c86='1', c87='1', c88='1', c89='1', c90='1', c91='1', c92='1', c93='1', c94='1', c95='1', c96='1', c97='1', c98='1', c99='1', c100='1', c101='1', c102='1', c103='1', c104='1', c105='1', c106='1', c107='1', c108='1', c109='1', c110='1', c111='1', c112='1', c113='1', c114='1', c115='1', c116='1', c117='1', c118='1', c119='1', c120='1', c121='1', c122='1', c123='1', c124='1', c125='1', c126='1', c127='1', c128='1', c129='1';
insert into t1 set c1='1', c2='1', c3='1', c4='1', c5='1', c6='1', c7='1', c8='1', c9='1', c10='1', c11='1', c12='1', c13='1', c14='1', c15='1', c16='1', c17='1', c18='1', c19='1', c20='1', c21='1', c22='1', c23='1', c24='1', c25='1', c26='1', c27='1', c28='1', c29='1', c30='1', c31='1', c32='1', c33='1', c34='1', c35='1', c36='1', c37='1', c38='1', c39='1', c40='1', c41='1', c42='1', c43='1', c44='1', c45='1', c46='1', c47='1', c48='1', c49='1', c50='1', c51='1', c52='1', c53='1', c54='1', c55='1', c56='1', c57='1', c58='1', c59='1', c60='1', c61='1', c62='1', c63='1', c64='1', c65='1', c66='1', c67='1', c68='1', c69='1', c70='1', c71='1', c72='1', c73='1', c74='1', c75='1', c76='1', c77='1', c78='1', c79='1', c80='1', c81='1', c82='1', c83='1', c84='1', c85='1', c86='1', c87='1', c88='1', c89='1', c90='1', c91='1', c92='1', c93='1', c94='1', c95='1', c96='1', c97='1', c98='1', c99='1', c100='1', c101='1', c102='1', c103='1', c104='1', c105='1', c106='1', c107='1', c108='1', c109='1', c110='1', c111='1', c112='1', c113='1', c114='1', c115='1', c116='1', c117='1', c118='1', c119='1', c120='1', c121='1', c122='1', c123='1', c124='1', c125='1', c126='1', c127='1', c128='1', c129='1';
drop table t1;
SET GLOBAL HISTOGRAM_HISTORY_VERSIONS_LIMIT= 5;
SET GLOBAL HISTOGRAM_HISTORY_ENABLED= 0;
include/rpl_end.inc

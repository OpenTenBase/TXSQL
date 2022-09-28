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
SET GLOBAL HISTOGRAM_HISTORY_VERSIONS_LIMIT= 5;
SET GLOBAL HISTOGRAM_HISTORY_ENABLED= 0;
include/rpl_end.inc

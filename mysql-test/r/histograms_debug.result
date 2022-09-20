# Testing of histogram statistics that uses DEBUG functionality.
#
# Simulate a failure due to dropping histograms during DROP TABLE
#
CREATE TABLE t1 (col1 INT, col2 INT);
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col1, col2 WITH 10 BUCKETS;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col1'.
test.t1	histogram	status	Histogram statistics created for column 'col2'.
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
test	t1	col2
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
test	t1	col2
SELECT COUNT(*) FROM information_schema.TABLES
WHERE TABLE_SCHEMA = 'test' AND TABLE_NAME = 't1';
COUNT(*)
1
SET DEBUG='+d,fail_after_drop_histograms';
DROP TABLE t1;
ERROR HY000: Unable to remove column statistics for column 'dummy_column' in table 'test'.'t1'
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
test	t1	col2
SELECT COUNT(*) FROM information_schema.TABLES
WHERE TABLE_SCHEMA = 'test' AND TABLE_NAME = 't1';
COUNT(*)
1
SET DEBUG='-d,fail_after_drop_histograms';
#
# Simulate a failure due to dropping histograms during ALTER TABLE
#
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
test	t1	col2
SELECT COUNT(*) FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = 'test' AND TABLE_NAME = 't1' AND COLUMN_NAME = 'col2';
COUNT(*)
1
SET DEBUG='+d,fail_after_drop_histograms';
ALTER TABLE t1 DROP COLUMN col2;
ERROR HY000: Unable to remove column statistics for column 'dummy_column' in table 'test'.'t1'
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
test	t1	col2
SELECT COUNT(*) FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = 'test' AND TABLE_NAME = 't1' AND COLUMN_NAME = 'col2';
COUNT(*)
1
SET DEBUG='-d,fail_after_drop_histograms';
#
# Simulate a failure due to renaming histograms during ALTER TABLE RENAME
#
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
test	t1	col2
SELECT COUNT(*) FROM information_schema.TABLES
WHERE TABLE_SCHEMA = 'test' AND TABLE_NAME = 't1';
COUNT(*)
1
SET DEBUG='+d,fail_after_rename_histograms';
ALTER TABLE t1 RENAME TO t2;
ERROR HY000: Unable to update column statistics for column 'dummy_column' in table 'test'.'t1'
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
test	t1	col2
SELECT COUNT(*) FROM information_schema.TABLES
WHERE TABLE_SCHEMA = 'test' AND TABLE_NAME = 't1';
COUNT(*)
1
SET DEBUG='-d,fail_after_rename_histograms';
DROP TABLE t1;
#
# Check that histogram with sampling works as expected
#
SET DEBUG='+d,histogram_force_sampling';
CREATE TABLE t1 (col1 DOUBLE);
INSERT INTO t1 SELECT RAND(1);
INSERT INTO t1 SELECT RAND(2) FROM t1;
INSERT INTO t1 SELECT RAND(3) FROM t1;
INSERT INTO t1 SELECT RAND(4) FROM t1;
INSERT INTO t1 SELECT RAND(5) FROM t1;
INSERT INTO t1 SELECT RAND(6) FROM t1;
INSERT INTO t1 SELECT RAND(7) FROM t1;
INSERT INTO t1 SELECT RAND(8) FROM t1;
INSERT INTO t1 SELECT RAND(9) FROM t1;
INSERT INTO t1 SELECT RAND(10) FROM t1;
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col1 WITH 4 BUCKETS;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col1'.
SELECT schema_name, table_name, column_name,
JSON_REMOVE(histogram, '$."last-updated"')
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME	JSON_REMOVE(histogram, '$."last-updated"')
test	t1	col1	{"buckets": [], "data-type": "double", "null-values": 0.0, "collation-id": 8, "sampling-rate": 0.5, "histogram-type": "singleton", "number-of-buckets-specified": 4}
SET DEBUG='-d,histogram_force_sampling';
DROP TABLE t1;
#
# Check that compressed histogram mcv resizing works as expected
#
set global cdb_compressed_histogram_enabled = on;
SET DEBUG='+d,histogram_force_sampling';
CREATE TABLE t1 (col_integer INT);
INSERT INTO t1 VALUES (1), (2), (3), (4), (5), (6), (7), (8), (9), (10);
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col_integer WITH 10 BUCKETS;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col_integer'.
SELECT schema_name, table_name, column_name,
JSON_REMOVE(histogram, '$."last-updated"')
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME	JSON_REMOVE(histogram, '$."last-updated"')
test	t1	col_integer	{"mcv-list": [[1, 0.05], [2, 0.1], [3, 0.15], [4, 0.2], [5, 0.25], [6, 0.3], [7, 0.35], [8, 0.4], [9, 0.45], [10, 0.5]], "data-type": "int", "equi-height": [], "null-values": 0.0, "collation-id": 8, "sampling-rate": 0.5, "histogram-type": "compressed", "distinct-values": 20.0, "number-of-buckets-specified": 10}
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col_integer WITH 9 BUCKETS;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col_integer'.
SELECT schema_name, table_name, column_name,
JSON_REMOVE(histogram, '$."last-updated"')
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME	JSON_REMOVE(histogram, '$."last-updated"')
test	t1	col_integer	{"mcv-list": [], "data-type": "int", "equi-height": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10], "null-values": 0.0, "collation-id": 8, "sampling-rate": 0.5, "histogram-type": "compressed", "distinct-values": 20.0, "number-of-buckets-specified": 9}
DELETE FROM t1;
INSERT INTO t1 VALUES (1), (2), (3), (4), (5), (7), (7), (7), (7), (10);
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col_integer WITH 7 BUCKETS;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col_integer'.
SELECT schema_name, table_name, column_name,
JSON_REMOVE(histogram, '$."last-updated"')
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME	JSON_REMOVE(histogram, '$."last-updated"')
test	t1	col_integer	{"mcv-list": [[1, 0.05], [2, 0.1], [3, 0.15], [4, 0.2], [5, 0.25], [7, 0.45], [10, 0.5]], "data-type": "int", "equi-height": [], "null-values": 0.0, "collation-id": 8, "sampling-rate": 0.5, "histogram-type": "compressed", "distinct-values": 10.0, "number-of-buckets-specified": 7}
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col_integer WITH 6 BUCKETS;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col_integer'.
SELECT schema_name, table_name, column_name,
JSON_REMOVE(histogram, '$."last-updated"')
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME	JSON_REMOVE(histogram, '$."last-updated"')
test	t1	col_integer	{"mcv-list": [[7, 0.2]], "data-type": "int", "equi-height": [1, 1, 2, 3, 4, 5, 10], "null-values": 0.0, "collation-id": 8, "sampling-rate": 0.5, "histogram-type": "compressed", "distinct-values": 10.0, "number-of-buckets-specified": 6}
SET DEBUG='-d,histogram_force_sampling';
SET global cdb_compressed_histogram_enabled=0;
DROP TABLE t1;
#
# Bug#26020352 WL8943:ASSERTION `M_THD->GET_TRANSACTION()->IS_EMPTY(
#              TRANSACTION_CTX::STMT) && M
#
CREATE TABLE t1 (col1 INT);
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col1 WITH 10 BUCKETS;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col1'.
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
SET DEBUG='+d,histogram_fail_after_open_table';
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col1 WITH 10 BUCKETS;
Table	Op	Msg_type	Msg_text
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
SET DEBUG='-d,histogram_fail_after_open_table';
DROP TABLE t1;
#
# Bug#26027240 WL8943:VIRTUAL BOOL SQL_CMD_ANALYZE_TABLE::EXECUTE(THD*):
#              ASSERTION `FALSE' FAIL
#
CREATE TABLE t1 (col1 INT);
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col1 WITH 10 BUCKETS;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col1'.
SET DEBUG='+d,histogram_fail_during_lock_for_write';
ANALYZE TABLE t1 DROP HISTOGRAM ON col1;
Table	Op	Msg_type	Msg_text
# Since we have simulated a fail, the histogram should still be present.
# However, since this is a simulation of failure no error is reported.
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
test	t1	col1
SET DEBUG='-d,histogram_fail_during_lock_for_write';
ANALYZE TABLE t1 DROP HISTOGRAM ON col1;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics removed for column 'col1'.
# The histogram should now be gone.
SELECT schema_name, table_name, column_name
FROM information_schema.COLUMN_STATISTICS;
SCHEMA_NAME	TABLE_NAME	COLUMN_NAME
DROP TABLE t1;
#
# Bug#26772858 MDL FOR COLUMN STATISTICS IS NOT PROPERLY REFLECTED IN
# P_S.METADATA_LOCKS
#
CREATE TABLE t1 (col1 INT);
SET DEBUG_SYNC='store_histogram_after_write_lock SIGNAL histogram_1_waiting WAIT_FOR continue_store_histogram';
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col1 WITH 2 BUCKETS;;
SET DEBUG_SYNC='now WAIT_FOR histogram_1_waiting';
SELECT OBJECT_TYPE, OBJECT_SCHEMA, OBJECT_NAME, COLUMN_NAME
FROM performance_schema.metadata_locks
WHERE LOCK_TYPE = "EXCLUSIVE"
    AND OBJECT_TYPE = "COLUMN STATISTICS"
  ORDER BY OBJECT_TYPE, OBJECT_SCHEMA, OBJECT_NAME, COLUMN_NAME;
OBJECT_TYPE	OBJECT_SCHEMA	OBJECT_NAME	COLUMN_NAME
COLUMN STATISTICS	test	t1	col1
SET DEBUG_SYNC='mdl_acquire_lock_wait SIGNAL histogram_2_lock_waiting';
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col1 WITH 2 BUCKETS;;
SET DEBUG_SYNC='now WAIT_FOR histogram_2_lock_waiting';
SELECT OBJECT_TYPE, OBJECT_SCHEMA, OBJECT_NAME, INDEX_NAME
FROM performance_schema.events_waits_current
WHERE OBJECT_TYPE = "COLUMN STATISTICS"
  ORDER BY OBJECT_TYPE, OBJECT_SCHEMA, OBJECT_NAME, INDEX_NAME;
OBJECT_TYPE	OBJECT_SCHEMA	OBJECT_NAME	INDEX_NAME
COLUMN STATISTICS	test	t1	col1
SET DEBUG_SYNC='now SIGNAL continue_store_histogram';
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col1'.
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col1'.
DROP TABLE t1;
#
# Bug#27672693  HISTOGRAMS: ASSERTION FAILED: !THD->TX_READ_ONLY
#
CREATE TABLE t1(col1 INT);
SET LOCAL TRANSACTION READ ONLY;
INSERT INTO t1 (col1) VALUES (1);
ERROR 25006: Cannot execute statement in a READ ONLY transaction.
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col1 WITH 16 BUCKETS;
Table	Op	Msg_type	Msg_text
	histogram	Error	The server is in read-only mode.
SET LOCAL TRANSACTION READ WRITE;
INSERT INTO t1 (col1) VALUES (1);
ANALYZE TABLE t1 UPDATE HISTOGRAM ON col1 WITH 16 BUCKETS;
Table	Op	Msg_type	Msg_text
test.t1	histogram	status	Histogram statistics created for column 'col1'.
DROP TABLE t1;
#
# Additional tests for bug#29634540 "DROP DATABASE OF 1 MILLION
# TABLES ...".
#
# Check that DROP DATABASE statement doesn't acquire metadata locks
# on column statistics while dropping it.
#
CREATE DATABASE mysqltest;
CREATE TABLE mysqltest.t1 (i INT);
INSERT INTO mysqltest.t1 VALUES (1), (2), (3);
ANALYZE TABLE mysqltest.t1 UPDATE HISTOGRAM ON i WITH 10 BUCKETS;
Table	Op	Msg_type	Msg_text
mysqltest.t1	histogram	status	Histogram statistics created for column 'i'.
CREATE TABLE mysqltest.t2 (j INT);
# Run DROP DATABASE which will be paused near its end.
SET DEBUG_SYNC = 'rm_table_no_locks_before_binlog SIGNAL drop_waiting WAIT_FOR drop_resume';
DROP DATABASE mysqltest;
connect con1, localhost, root,,;
# Wait until DROP DATABASE gets paused and check what MDL
# it has acquired. There should be X locks on both tables
# and no locks on column statistics.
SET DEBUG_SYNC = 'now WAIT_FOR drop_waiting';
SELECT object_type, object_schema, object_name, column_name, lock_type
FROM performance_schema.metadata_locks
WHERE object_schema = "mysqltest"
  ORDER BY object_type, object_schema, object_name, column_name, lock_type;
object_type	object_schema	object_name	column_name	lock_type
SCHEMA	mysqltest	NULL	NULL	EXCLUSIVE
TABLE	mysqltest	t1	NULL	EXCLUSIVE
TABLE	mysqltest	t2	NULL	EXCLUSIVE
#
# Run concurrent ANALYZE TABLE ... DROP HISTOGRAM on table from
# database being dropped.
ANALYZE TABLE mysqltest.t1 DROP HISTOGRAM ON i;
connect con2, localhost, root,,;
# Check that it will be blocked thanks to table MDL.
# Unpause DROP DATABASE.
SET DEBUG_SYNC = 'now SIGNAL drop_resume';
disconnect con2;
connection con1;
# Reap ANALYZE TABLE ... DROP HISTOGRAM, which should not have
# found any histograms since database has been dropped.
Table	Op	Msg_type	Msg_text
mysqltest.t1	histogram	Error	No histogram statistics found for column 'i'.
disconnect con1;
connection default;
# Reap DROP DATABASE
# Clean-up.
SET DEBUG_SYNC = 'RESET';
#
# Bug #97172 UNABLE TO OPEN TABLE WITH INCOMPATIBLE HISTOGRAM
#            ASSERTION: thd->is_system_thread() || thd->killed || thd->is_error()
#
# We will inject a dummy histogram to simulate an incompatible histogram.
#
SET debug='+d,skip_dd_table_access_check';
#
# The dummy histogram will be ignored on opening table,
# and removed on drop table.
#
CREATE TABLE tbl_int (col1 int);
INSERT tbl_int VALUES (1);
INSERT mysql.column_statistics
VALUES (1, 1, unhex("746573741F74626C5F696E741F636F6C31"),
"test", "tbl_int", "col1",
"{\"histogram-type\":\"dummy\",\"data-type\":\"int\"}",
NULL);
FLUSH TABLE tbl_int;
SELECT * from tbl_int;
col1
1
DROP TABLE tbl_int;
SELECT COUNT(*) AS should_be_0 from information_schema.COLUMN_STATISTICS;
should_be_0
0
#
# The dummy histogram will be removed on DROP HISTOGRAM.
#
CREATE TABLE tbl_int (col1 int);
INSERT mysql.column_statistics
VALUES (1, 1, unhex("746573741F74626C5F696E741F636F6C31"),
"test", "tbl_int", "col1",
"{\"histogram-type\":\"dummy\",\"data-type\":\"int\"}",
NULL);
ANALYZE TABLE tbl_int DROP HISTOGRAM ON col1;
Table	Op	Msg_type	Msg_text
test.tbl_int	histogram	status	Histogram statistics removed for column 'col1'.
SELECT COUNT(*) AS should_be_0 FROM information_schema.COLUMN_STATISTICS;
should_be_0
0
DROP TABLE tbl_int;
#
# The dummy histogram will be overwritten on UPDATE HISTOGRAM.
#
CREATE TABLE tbl_int (col1 int);
INSERT tbl_int VALUES (1);
INSERT mysql.column_statistics
VALUES (1, 1, unhex("746573741F74626C5F696E741F636F6C31"),
"test", "tbl_int", "col1",
"{\"histogram-type\":\"dummy\",\"data-type\":\"int\"}",
NULL);
FLUSH TABLE tbl_int;
SELECT JSON_REMOVE(histogram, '$."last-updated"') FROM information_schema.COLUMN_STATISTICS;
JSON_REMOVE(histogram, '$."last-updated"')
{"data-type": "int", "histogram-type": "dummy"}
ANALYZE TABLE tbl_int update histogram on col1;
Table	Op	Msg_type	Msg_text
test.tbl_int	histogram	status	Histogram statistics created for column 'col1'.
SELECT JSON_REMOVE(histogram, '$."last-updated"') FROM information_schema.COLUMN_STATISTICS;
JSON_REMOVE(histogram, '$."last-updated"')
{"buckets": [[1, 1.0]], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
SELECT * FROM tbl_int;
col1
1
DROP TABLE tbl_int;
#
# The dummy histogram aborts RENAME TABLE
#
CREATE TABLE tbl_int (col1 int);
INSERT mysql.column_statistics
VALUES (1, 1, unhex("746573741F74626C5F696E741F636F6C31"),
"test", "tbl_int", "col1",
"{\"histogram-type\":\"dummy\",\"data-type\":\"int\"}",
NULL);
RENAME TABLE tbl_int TO tbl_int_2;
ERROR HY000: Unable to rename column statistics with incompatible histogram for column 'col1' in table 'test'.'tbl_int'. Please drop the histogram and try again.
ANALYZE TABLE tbl_int DROP HISTOGRAM ON col1;
Table	Op	Msg_type	Msg_text
test.tbl_int	histogram	status	Histogram statistics removed for column 'col1'.
RENAME TABLE tbl_int TO tbl_int_2;
DROP TABLE tbl_int_2;
SET DEBUG='-d,skip_dd_table_access_check';

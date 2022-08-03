##################
# UPDATE HISTOGRAM ... USING HISTORY ...
##################
SET GLOBAL HISTOGRAM_HISTORY_ENABLED= 1;
SET GLOBAL HISTOGRAM_HISTORY_VERSIONS_LIMIT= 3;
# Note: Histogram data is identified (hid) by '$."number-of-buckets-specified"' by design.
CREATE TABLE tbl1(c1 INT);
INSERT INTO tbl1(c1) VALUES(1),(1),(2),(3),(5);
#
# Create 3 history records
#
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 1 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 2 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 3 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
#
# Test USING HISTORY ...
#
# Check exact version (ver1)
# ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 USING HISTORY ver1;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	The histogram on column 'c1' restored with history version.
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
1
# Check relative version (last)
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 USING HISTORY -1;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	The histogram on column 'c1' restored with history version.
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
3
# Check relative version (second to last)
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 USING HISTORY -2;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	The histogram on column 'c1' restored with history version.
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
2
# Check relative version (out of range)
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 USING HISTORY -4;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Error	The histogram history on column 'c1' load failed.
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 USING HISTORY 0;
ERROR 22003: Version of histogram value is out of range in 'ANALYZE TABLE'
# Check exact version (out of range)
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 USING HISTORY 1;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	Error	The histogram history on column 'c1' load failed.
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
2
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
1
2
3
# Number of records exceeds HISTOGRAM_HISTORY_VERSIONS_LIMIT
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 4 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
2
3
4
# Change the value of HISTOGRAM_HISTORY_VERSIONS_LIMIT to 4
SET GLOBAL HISTOGRAM_HISTORY_VERSIONS_LIMIT= 4;
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 5 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
2
3
4
5
# Test HISTOGRAM_HISTORY_ENABLED
SET GLOBAL HISTOGRAM_HISTORY_ENABLED= 0;
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 6 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
6
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
2
3
4
5
SET GLOBAL HISTOGRAM_HISTORY_ENABLED= 1;
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 USING DATA '{"buckets": [[1, 0.4], [2, 0.6], [3, 0.8], [5, 1.0]], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 7}';
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
7
SELECT JSON_EXTRACT(HISTOGRAM, '$."number-of-buckets-specified"') AS hid FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
hid
3
4
5
7
DROP TABLE tbl1;
#
# Test RENAME COLUMN
#
CREATE TABLE tbl1(c1 INT);
INSERT INTO tbl1(c1) VALUES(1),(1),(2),(3),(5);
# Create 3 history records for tbl1
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 1 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 2 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 3 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
3
# Rename c1 to c2
ALTER TABLE tbl1 RENAME COLUMN c1 to c2;
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM information_schema.column_statistics WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c2";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c2";
HISTOGRAM
0
DROP TABLE tbl1;
#
# Test RENAME/DROP TABLE
#
CREATE TABLE tbl1(c1 INT);
INSERT INTO tbl1(c1) VALUES(1),(1),(2),(3),(5);
# Create 3 history records for tbl1
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 1 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 2 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
ANALYZE TABLE tbl1 UPDATE HISTOGRAM ON c1 WITH 3 BUCKETS;
Table	Op	Msg_type	Msg_text
test.tbl1	histogram	status	Histogram statistics created for column 'c1'.
test.tbl1	histogram	status	Histogram history created for column 'c1'.
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
3
# Rename tbl1 to tbl1_renamed
RENAME TABLE tbl1 TO tbl1_renamed;
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1" AND COLUMN_NAME="c1";
HISTOGRAM
0
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1_renamed" AND COLUMN_NAME="c1";
HISTOGRAM
3
# DROP TABLE tbl1_renamed
DROP TABLE tbl1_renamed;
SELECT COUNT(*) AS HISTOGRAM FROM mysql.column_statistics_history WHERE TABLE_NAME="tbl1_renamed" AND COLUMN_NAME="c1";
HISTOGRAM
0
SET GLOBAL HISTOGRAM_HISTORY_VERSIONS_LIMIT= 5;
SET GLOBAL HISTOGRAM_HISTORY_ENABLED= 0;

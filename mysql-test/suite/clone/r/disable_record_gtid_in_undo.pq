# Create table
CREATE TABLE t1(col1 INT PRIMARY KEY, col2 char(64), FULLTEXT KEY fts_index(col2));
INSERT INTO t1 VALUES(10, 'clone row 1');
INSERT INTO t1 VALUES(20, 'clone row 2');
INSERT INTO t1 VALUES(30, 'clone row 3');
SELECT * from t1 ORDER BY col1;
col1	col2
10	clone row 1
20	clone row 2
30	clone row 3
# Install Clone Plugin
INSTALL PLUGIN clone SONAME 'CLONE_PLUGIN';
SELECT PLUGIN_NAME, PLUGIN_STATUS FROM INFORMATION_SCHEMA.PLUGINS
WHERE PLUGIN_NAME LIKE '%clone%';
PLUGIN_NAME	PLUGIN_STATUS
clone	ACTIVE
set global txsql_disable_gtid_in_undo=ON;
# Case1: Check GTID, turn txsql_disable_gtid_in_undo from ON to OFF will flush gtids to table.
select @@gtid_executed;
@@gtid_executed
master_uuid:1-6
select * from mysql.gtid_executed;
source_uuid	interval_start	interval_end
master_uuid	1	6
INSERT INTO t1 VALUES(50, 'clone row 5');
select @@gtid_executed;
@@gtid_executed
master_uuid:1-7
select * from mysql.gtid_executed;
source_uuid	interval_start	interval_end
master_uuid	1	6
set global txsql_disable_gtid_in_undo=OFF;
select @@gtid_executed;
@@gtid_executed
master_uuid:1-7
select * from mysql.gtid_executed where interval_end=7;
source_uuid	interval_start	interval_end
master_uuid	#	7
# Case2: Unable to run clone statement when txsql_disable_gtid_in_undo=ON.
set global txsql_disable_gtid_in_undo=ON;
CLONE LOCAL DATA DIRECTORY = 'CLONE_DATADIR';
ERROR HY000: Unable to run clone statement when txsql_disable_gtid_in_undo=ON.
# Case3: Clone data.
set global txsql_disable_gtid_in_undo=OFF;
SET GLOBAL clone_autotune_concurrency = OFF;
SET GLOBAL clone_max_concurrency = 8;
CLONE LOCAL DATA DIRECTORY = 'CLONE_DATADIR';
select ID, STATE, ERROR_NO from performance_schema.clone_status;
ID	STATE	ERROR_NO
1	Completed	0
select ID, STAGE, STATE from performance_schema.clone_progress;
ID	STAGE	STATE
1	DROP DATA	Completed
1	FILE COPY	Completed
1	FILE SYNC	Completed
1	PAGE COPY	Completed
1	RECOVERY	Not Started
1	REDO COPY	Completed
1	RESTART	Not Started
# restart: --datadir=CLONE_DATADIR
# Validate data
SELECT * from t1 ORDER BY col1;
col1	col2
10	clone row 1
20	clone row 2
30	clone row 3
50	clone row 5
INSERT INTO t1 VALUES(40, 'clone row 4');
SELECT * from t1 ORDER BY col1;
col1	col2
10	clone row 1
20	clone row 2
30	clone row 3
40	clone row 4
50	clone row 5
# Cleanup
# restart:
SELECT * from t1 ORDER BY col1;
col1	col2
10	clone row 1
20	clone row 2
30	clone row 3
50	clone row 5
DROP TABLE t1;
UNINSTALL PLUGIN clone;

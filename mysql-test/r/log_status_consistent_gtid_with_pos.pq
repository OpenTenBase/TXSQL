CREATE TABLE t1(c1 INT);
SET debug_sync = "bgc_after_sync_stage_before_commit_stage SIGNAL synced WAIT_FOR continue";
INSERT INTO t1 VALUES(1);
SET debug_sync = "now WAIT_FOR synced";
SELECT local FROM performance_schema.log_status;
local
{"error": "binlog pos and gtid are not consistent", "gtid_executed": "SERVER_UUID:1", "binary_log_file": "binlog.000001", "binary_log_position": 618}
SET debug_sync = "now SIGNAL continue";
SELECT local FROM performance_schema.log_status;
local
{"gtid_executed": "SERVER_UUID:1-2", "binary_log_file": "binlog.000001", "binary_log_position": 618}
INSERT INTO t1 VALUES(2);
SET global binlog_order_commits=OFF;
SHOW VARIABLES LIKE "%binlog_order_commits%";
Variable_name	Value
binlog_order_commits	OFF
SET debug_sync = "bgc_after_sync_stage_before_commit_stage SIGNAL synced WAIT_FOR continue";
INSERT INTO t1 VALUES(3);
SET debug_sync = "now WAIT_FOR synced";
SELECT local FROM performance_schema.log_status;
local
{"error": "binlog pos and gtid are not consistent", "gtid_executed": "SERVER_UUID:1-3", "binary_log_file": "binlog.000001", "binary_log_position": 1164}
SET debug_sync = "now SIGNAL continue";
SELECT local FROM performance_schema.log_status;
local
{"gtid_executed": "SERVER_UUID:1-4", "binary_log_file": "binlog.000001", "binary_log_position": 1164}
INSERT INTO t1 VALUES(3);
SET global binlog_order_commits=ON;
DROP TABLE t1;

include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
CREATE TABLE t1 (a INT);
call mtr.add_suppression("Statement is unsafe because it is being used inside a XA transaction");
include/sync_slave_io_with_master.inc
# Adding debug point 'before_transaction_commit' to @@GLOBAL.debug
include/stop_slave_io.inc
BEGIN;
INSERT INTO t1 VALUES(1);
UPDATE t1 SET a= 2;
DELETE FROM t1;
COMMIT;
include/start_slave_io.inc
SET DEBUG_SYNC='now WAIT_FOR in_transaction';
Success!
SET DEBUG_SYNC= 'now SIGNAL finish_check';
include/sync_slave_io_with_master.inc
Success!
include/stop_slave_io.inc
XA START 'xa';
INSERT INTO t1 VALUES(1);
UPDATE t1 SET a= 2;
DELETE FROM t1;
XA END 'xa';
XA PREPARE 'xa';
include/start_slave_io.inc
SET DEBUG_SYNC='now WAIT_FOR in_transaction';
Success!
SET DEBUG_SYNC= 'now SIGNAL finish_check';
include/sync_slave_io_with_master.inc
Success!
include/stop_slave_io.inc
XA COMMIT 'xa';
include/start_slave_io.inc
SET DEBUG_SYNC='now WAIT_FOR in_transaction';
Success!
SET DEBUG_SYNC= 'now SIGNAL finish_check';
include/sync_slave_io_with_master.inc
Success!
DROP TABLE t1;
include/sync_slave_sql_with_master.inc
# Removing debug point 'before_transaction_commit' from @@GLOBAL.debug
include/rpl_end.inc

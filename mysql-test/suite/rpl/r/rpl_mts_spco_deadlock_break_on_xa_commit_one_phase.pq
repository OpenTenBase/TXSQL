#
# 1. Create source-replica topology.
include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
call mtr.add_suppression("Unsafe statement written to the binary log using statement format*.");
#
# 2. Setup testing environment.
#    2.1 Create table on source and insert few rows.
#
CREATE TABLE t1(c1 INT PRIMARY KEY, c2 INT, INDEX(c2)) ENGINE = InnoDB;
CREATE TABLE t2(c1 INT PRIMARY KEY) ENGINE = InnoDB;
#
# 2.2 Setup necessary variables on replica server.
#
include/sync_slave_sql_with_master.inc
include/stop_slave_sql.inc
SET @saved_replica_preserve_commit_order = @@GLOBAL.replica_preserve_commit_order;
SET GLOBAL replica_preserve_commit_order = ON;
SET @saved_replica_parallel_type = @@GLOBAL.replica_parallel_type;
Warnings:
Warning	1287	'@@replica_parallel_type' is deprecated and will be removed in a future release.
SET @saved_replica_parallel_workers = @@GLOBAL.replica_parallel_workers;
SET @saved_replica_transaction_retries = @@GLOBAL.replica_transaction_retries;
SET GLOBAL replica_parallel_type = "LOGICAL_CLOCK";
Warnings:
Warning	1287	'@@replica_parallel_type' is deprecated and will be removed in a future release.
SET GLOBAL replica_parallel_workers = 3;
SET GLOBAL replica_transaction_retries = 0;
#
# 2.3 Set innodb_lock_wait_timeout to 300 to make test work without issues
# on slow platform runs.
#
SET @saved_innodb_lock_wait_timeout = @@GLOBAL.innodb_lock_wait_timeout;
SET GLOBAL innodb_lock_wait_timeout = 300;
#
# 2.4 Adding debug point.
#
# Adding debug point 'rpl_before_report_commit_order_deadlock' to @@GLOBAL.debug
# Adding debug point 'rpl_fake_cod_deadlock' to @@GLOBAL.debug
#
# 2.5 Add error suppressions.
#
call mtr.add_suppression("Worker .* failed executing transaction");
call mtr.add_suppression("The slave coordinator and worker threads are stopped");
call mtr.add_suppression("Unsafe statement written to the binary log using statement format*.");
#
# 2.6 An extra connection needed on Slave.
#
#
# 3. Actual testing starts from here.
#
# 3.1 Insert basic data on Master
#
[connection master]
INSERT INTO t1 VALUES(1, 1),(2, 2);
#
# 3.2 Enabling set_commit_parent_100 debug point
#     on Master to make two conflicting transactions
#     run parallely by two worker threads on Slave.
#
# Adding debug point 'set_commit_parent_100' to @@SESSION.debug
#
# 3.3 Execute Transaction-1
#
INSERT INTO t1 VALUES(3, 1);
#
# 3.4 Execute Transaction-2
#
SET TIMESTAMP = 200;
XA START '1';
DELETE FROM t1 WHERE c2 <= 1;
INSERT INTO t2 values(1);
XA END '1';
XA COMMIT '1' ONE PHASE;
SET TIMESTAMP = UNIX_TIMESTAMP();
#
# 3.5 Execute Transaction-3
#
INSERT INTO t1 values(10,10);
#
# 3.6 On Slave, begin a transaction (Transacation-4)
#     (that blocks 'Transacation-1').
#
[connection slave1]
BEGIN;
INSERT INTO t1 VALUES(3, 3);
#
# 3.7 On Slave, begin a transaction (Transacation-5)
#     that can block INSERT in 'Transacation-2'.
#
[connection slave]
BEGIN;
INSERT INTO t2 VALUES(1);
#
# 3.8 Start SQL thread, let Transaction-1 & 2 reach SQL workers
#
include/start_slave_sql.inc
#
# 3.9 Wait till the worker threads pick up those transacations
#     (Transaction 1 and Transaction 2). They will wait for locks due
#     local transacations (4 & 5).
#
# 3.10 Rollback Transaction-4 which will release lock required for
#      Transaction-1. Debug sync make sure Tansaction-1 reach to
#      'report_deadlock'.
#
[connection slave1]
ROLLBACK;
SET DEBUG_SYNC= "now wait_for reach_report_deadlock";
#
# 3.11 Add debug point make sure Transaction-2 reach commit ordere wait
#
# Adding debug point 'rpl_before_commit_order_manager_wait_200' to @@GLOBAL.debug
#
# 3.12 Commit Transaction-5 which will release lock required for
#      Transaction-2.
#
[connection slave]
ROLLBACK;
include/wait_for_slave_sql_error.inc [errno=1213]
#
# 4. Cleanup
# 4.1 Clear debug point.
#
# Removing debug point 'rpl_before_report_commit_order_deadlock' from @@GLOBAL.debug
# Removing debug point 'rpl_fake_cod_deadlock' from @@GLOBAL.debug
# Removing debug point 'rpl_before_commit_order_manager_wait_200' from @@GLOBAL.debug
#
# 4.2 Clean the data on Slave.
#
include/stop_slave_io.inc
RESET REPLICA;
DROP TABLE t1, t2;
#
# 4.3 Clear system variables.
#
SET GLOBAL replica_preserve_commit_order = @saved_replica_preserve_commit_order;
SET GLOBAL innodb_lock_wait_timeout = @saved_innodb_lock_wait_timeout;
SET GLOBAL replica_parallel_type = @saved_replica_parallel_type;
Warnings:
Warning	1287	'@@replica_parallel_type' is deprecated and will be removed in a future release.
SET GLOBAL replica_parallel_workers = @saved_replica_parallel_workers;
SET GLOBAL replica_transaction_retries = @saved_replica_transaction_retries;
#
# 4.4 Clear debug point.
#
[connection master]
# Removing debug point 'set_commit_parent_100' from @@SESSION.debug
#
# 4.5 Clean the data on Master.
#
DROP TABLE t1, t2;
include/rpl_end.inc

include/only_with_option.inc [GLOBAL.slave_transaction_retries > 2]
include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
call mtr.add_suppression('Slave SQL for channel');
CREATE TABLE `t1` (
id int,
c1 int
) ENGINE=InnoDB;
include/sync_slave_sql_with_master.inc
include/stop_slave.inc
[connection master]
BEGIN;
INSERT INTO t1 VALUES (1,1);
INSERT INTO t1 VALUES (2,2);
INSERT INTO t1 VALUES (3,3);
INSERT INTO t1 VALUES (4,4);
COMMIT;
[connection slave]
# Adding debug point 'error_on_write_rows_log_event_apply' to @@GLOBAL.debug
include/start_slave.inc
[connection master]
include/sync_slave_sql_with_master.inc
# Removing debug point 'error_on_write_rows_log_event_apply' from @@GLOBAL.debug
[connection master]
DROP TABLE t1;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

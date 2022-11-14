include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
include/install_semisync.inc
Warnings:
Note	1287	'rpl_semi_sync_master' is deprecated and will be removed in a future release. Please use rpl_semi_sync_source instead
Warnings:
Note	1287	'rpl_semi_sync_slave' is deprecated and will be removed in a future release. Please use rpl_semi_sync_replica instead
#
# prepare
[connection master]
SET GLOBAL rpl_semi_sync_master_timeout = 3000;
set global rpl_semi_sync_master_trace_level=255;
set global rpl_semi_sync_master_wait_forever=ON;
show variables like "rpl_semi_sync%";
Variable_name	Value
rpl_semi_sync_master_enabled	ON
rpl_semi_sync_master_timeout	3000
rpl_semi_sync_master_trace_level	255
rpl_semi_sync_master_wait_for_slave_count	1
rpl_semi_sync_master_wait_forever	ON
rpl_semi_sync_master_wait_no_slave	ON
rpl_semi_sync_master_wait_point	AFTER_SYNC
create table t1 (a int);
insert into t1 values (1);
select * from t1;;
a	1
set global rpl_semi_sync_slave_trace_level=255;
select * from t1;;
a	1
[connection master]
SET global DEBUG = '+d,semi_sync_wait_during_shutdown';
set global binlog_group_commit_sync_delay= 500000;
[connection slave]
SET global DEBUG = '+d,semi_sync_wait_during_shutdown';
#
# run
[connection master]
insert into t1 values (2);
[connection master1]
insert into t1 values (3);
[connection master2]
# shutdown master
[connection master]
ERROR HY000: The binlog(file: master-bin.000001, end_pos: <pos>) of the transaction may not be synchronized to slave when server shutdown.
[connection master1]
ERROR HY000: The binlog(file: master-bin.000001, end_pos: <pos>) of the transaction may not be synchronized to slave when server shutdown.
include/rpl_start_server.inc [server_number=1]
select * from t1;
a
1
2
3
# get the times of retry
DATE_TIME [Note] [MY-040009] [Repl] received ack or shutdown, retry to check binlog file and postion
DATE_TIME [Note] [MY-040009] [Repl] received ack or shutdown, retry to check binlog file and postion
DATE_TIME [Note] [MY-040009] [Repl] received ack or shutdown, retry to check binlog file and postion
[connection master]
select * from t1;
a
1
2
3
DROP TABLE t1;
include/rpl_sync.inc
[connection slave]
set global rpl_semi_sync_slave_trace_level=0;
set global debug= '-d,semi_sync_wait_during_shutdown';
include/uninstall_semisync_replica.inc
[connection master]
set global DEBUG= '-d,semi_sync_wait_during_shutdown';
set global binlog_group_commit_sync_delay= default;
SET GLOBAL rpl_semi_sync_master_enabled= OFF;
include/uninstall_semisync_source.inc
include/rpl_end.inc

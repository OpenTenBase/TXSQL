include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
#
# Prepare
#
[connection master]
create table t1 (a varchar(2048));
include/sync_slave_sql_with_master.inc
show tables;
Tables_in_test
t1
#
# Run
#
[connection master]
set global debug="d,get_binlog_end_pos_before_flush_net";
set global debug="+d,close_connections_before_kill_binlog_dump";
show variables like "debug";
Variable_name	Value
debug	d,get_binlog_end_pos_before_flush_net,close_connections_before_kill_binlog_dump
begin;
insert into t1 values(repeat("a", 1024)),(repeat("b", 1024)),(repeat("c", 1024));
commit;
[connection slave]
include/stop_slave_io.inc
# Shutdown Server
include/rpl_stop_server.inc [server_number=1]
include/rpl_start_server.inc [server_number=1]
[connection slave]
include/start_slave_io.inc
#
# Cleanup
#
[connection master]
DROP TABLE t1;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
stop replica;
create table t1(id int not null primary key,c1 int);
xa start 'xatest1';
insert into t1 values(1,1),(2,2);
xa end 'xatest1';
xa prepare 'xatest1';
start replica io_thread;
include/sync_slave_io_with_master.inc
show relaylog events in 'slave-relay-bin.000003';
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
slave-relay-bin.000003	4	Format_desc	2	#	#
slave-relay-bin.000003	126	Previous_gtids	2	#	#
slave-relay-bin.000003	157	Rotate	1	#	#
slave-relay-bin.000003	205	Format_desc	1	#	#
slave-relay-bin.000003	327	Rotate	0	#	#
slave-relay-bin.000003	375	Gtid	1	#	#
slave-relay-bin.000003	452	Query	1	#	#
slave-relay-bin.000003	591	Gtid	1	#	#
slave-relay-bin.000003	670	Query	1	#	#
slave-relay-bin.000003	772	Table_map	1	#	#
slave-relay-bin.000003	821	Write_rows	1	#	#
slave-relay-bin.000003	874	Query	1	#	#
slave-relay-bin.000003	974	XA_prepare	1	#	#
include/rpl_stop_server.inc [server_number=2]
#ib_16384_0.dblwr
#ib_16384_1.dblwr
#innodb_redo
#innodb_temp
__txsql_recycle_bin__
auto.cnf
ib_buffer_pool
ibdata1
mtr
mysql
mysql.ibd
performance_schema
slave-bin.000001
slave-bin.index
slave-relay-bin.000001
slave-relay-bin.000002
slave-relay-bin.000003
slave-relay-bin.index
sys
test
tlog_0
undo_001
undo_002
include/rpl_start_server.inc [server_number=2]
#ib_16384_0.dblwr
#ib_16384_1.dblwr
#innodb_redo
#innodb_temp
__txsql_recycle_bin__
auto.cnf
ib_buffer_pool
ibdata1
ibtmp1
mtr
mysql
mysql.ibd
performance_schema
slave-bin.000001
slave-bin.000002
slave-bin.index
slave-relay-bin.000001
slave-relay-bin.000002
slave-relay-bin.000003
slave-relay-bin.000004
slave-relay-bin.index
sys
test
tlog_0
undo_001
undo_002
show relaylog events in 'slave-relay-bin.000003';
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
slave-relay-bin.000003	4	Format_desc	2	#	#
slave-relay-bin.000003	126	Previous_gtids	2	#	#
slave-relay-bin.000003	157	Rotate	1	#	#
slave-relay-bin.000003	205	Format_desc	1	#	#
slave-relay-bin.000003	327	Rotate	0	#	#
slave-relay-bin.000003	375	Gtid	1	#	#
slave-relay-bin.000003	452	Query	1	#	#
slave-relay-bin.000003	591	Gtid	1	#	#
slave-relay-bin.000003	670	Query	1	#	#
slave-relay-bin.000003	772	Table_map	1	#	#
slave-relay-bin.000003	821	Write_rows	1	#	#
start replica;
xa commit 'xatest1';
include/sync_slave_sql_with_master.inc
select * from t1;
id	c1
1	1
2	2
drop table t1;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

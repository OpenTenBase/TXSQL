include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
create table t1(id1 int, c1 int);
create table t2(id2 int, c2 int);
insert returning
insert into t1 values (1,1),(2,2),(3,3) returning id1, c1;
id1	c1
1	1
2	2
3	3
insert into t2 select * from t1 returning id2, c2;
id2	c2
1	1
2	2
3	3
include/show_binlog_events.inc
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
master-bin.000001	#	Query	#	#	use `test`; create table t1(id1 int, c1 int)
master-bin.000001	#	Query	#	#	use `test`; create table t2(id2 int, c2 int)
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; insert into t1 values (1,1),(2,2),(3,3) returning id1, c1
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; insert into t2 select * from t1 returning id2, c2
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
include/sync_slave_sql_with_master.inc
select * from t1;
id1	c1
1	1
2	2
3	3
select * from t2;
id2	c2
1	1
2	2
3	3
include/show_binlog_events.inc
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
slave-bin.000001	#	Query	#	#	use `test`; create table t1(id1 int, c1 int)
slave-bin.000001	#	Query	#	#	use `test`; create table t2(id2 int, c2 int)
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t1 values (1,1),(2,2),(3,3) returning id1, c1
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t2 select * from t1 returning id2, c2
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
delete returning
delete from t1 where id1=1 returning c1;
c1
1
select * from t1;
id1	c1
2	2
3	3
include/show_binlog_events.inc
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
master-bin.000001	#	Query	#	#	use `test`; create table t1(id1 int, c1 int)
master-bin.000001	#	Query	#	#	use `test`; create table t2(id2 int, c2 int)
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; insert into t1 values (1,1),(2,2),(3,3) returning id1, c1
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; insert into t2 select * from t1 returning id2, c2
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; delete from t1 where id1=1 returning c1
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
include/sync_slave_sql_with_master.inc
select * from t1;
id1	c1
2	2
3	3
include/show_binlog_events.inc
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
slave-bin.000001	#	Query	#	#	use `test`; create table t1(id1 int, c1 int)
slave-bin.000001	#	Query	#	#	use `test`; create table t2(id2 int, c2 int)
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t1 values (1,1),(2,2),(3,3) returning id1, c1
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t2 select * from t1 returning id2, c2
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; delete from t1 where id1=1 returning c1
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
update returning
update t2 set id2=id2+1 returning c2;
c2
1
2
3
select * from t2;
id2	c2
2	1
3	2
4	3
include/show_binlog_events.inc
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
master-bin.000001	#	Query	#	#	use `test`; create table t1(id1 int, c1 int)
master-bin.000001	#	Query	#	#	use `test`; create table t2(id2 int, c2 int)
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; insert into t1 values (1,1),(2,2),(3,3) returning id1, c1
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; insert into t2 select * from t1 returning id2, c2
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; delete from t1 where id1=1 returning c1
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; update t2 set id2=id2+1 returning c2
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
include/sync_slave_sql_with_master.inc
select * from t2;
id2	c2
2	1
3	2
4	3
include/show_binlog_events.inc
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
slave-bin.000001	#	Query	#	#	use `test`; create table t1(id1 int, c1 int)
slave-bin.000001	#	Query	#	#	use `test`; create table t2(id2 int, c2 int)
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t1 values (1,1),(2,2),(3,3) returning id1, c1
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t2 select * from t1 returning id2, c2
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; delete from t1 where id1=1 returning c1
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; update t2 set id2=id2+1 returning c2
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
delete from t1;
delete from t2;
insert into t1 values (1,1),(2,2),(3,3);
insert into t2 values (3,3);
replace into t2(id2,c2) values(1,1),(2,2) returning (select id1 from t1);
ERROR 21000: Subquery returns more than 1 row
delete from t2 returning (select * from t1);
ERROR 21000: Operand should contain 1 column(s)
delete from t1 returning (select id1 from t1);
ERROR HY000: You can't specify target table 't1' for update in FROM clause
include/sync_slave_sql_with_master.inc
include/show_binlog_events.inc
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
slave-bin.000001	#	Query	#	#	use `test`; create table t1(id1 int, c1 int)
slave-bin.000001	#	Query	#	#	use `test`; create table t2(id2 int, c2 int)
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t1 values (1,1),(2,2),(3,3) returning id1, c1
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t2 select * from t1 returning id2, c2
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; delete from t1 where id1=1 returning c1
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; update t2 set id2=id2+1 returning c2
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; delete from t1
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; delete from t2
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t1 values (1,1),(2,2),(3,3)
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
slave-bin.000001	#	Query	#	#	BEGIN
slave-bin.000001	#	Query	#	#	use `test`; insert into t2 values (3,3)
slave-bin.000001	#	Xid	#	#	COMMIT /* XID */
drop table t1, t2;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

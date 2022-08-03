include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
set @saved_writeset_extraction = @@global.transaction_write_set_extraction;
set @saved_dependency_tracking = @@global.binlog_transaction_dependency_tracking;
set global transaction_write_set_extraction=xxhash64;
set transaction_write_set_extraction=xxhash64;
set global binlog_transaction_dependency_tracking=writeset;
set binlog_format = mixed;
show variables like '%binlog_format%';
Variable_name	Value
binlog_format	MIXED
create table t1(id int auto_increment primary key, c1 int);
create table t2(c1 int);
create table t3(c1 int);
insert into t2 values(1);
include/sync_slave_sql_with_master.inc
stop replica;
set @saved_slave_preserve_commit_order=@@global.slave_preserve_commit_order;
set @saved_slave_parallel_type=@@global.slave_parallel_type;
set @saved_slave_parallel_workers=@@global.slave_parallel_workers;
set global slave_preserve_commit_order=OFF;
set global slave_parallel_type=logical_clock;
set global slave_parallel_workers=2;
begin;
update t2 set c1=2 where c1=1;
commit;
begin;
insert into t3 values(1);
update t2 set c1=3 where c1=2;
insert into t1(c1) select * from t2;
commit;
begin;
update t2 set c1=9 where c1=3;
insert into t1(c1) select * from t2;
commit;
lock table t3 read;
start replica;
unlock tables;
include/sync_slave_sql_with_master.inc
select * from t2;
c1
9
select * from t2;
c1
9
stop replica;
set global slave_preserve_commit_order=@saved_slave_preserve_commit_order;
set global slave_parallel_type=@saved_slave_parallel_type;
set global slave_parallel_workers=@saved_slave_parallel_workers;
start replica;
set global binlog_transaction_dependency_tracking = @saved_dependency_tracking;
set global transaction_write_set_extraction = @saved_writeset_extraction;
drop table if exists t1;
drop table if exists t2;
drop table if exists t3;
include/rpl_end.inc

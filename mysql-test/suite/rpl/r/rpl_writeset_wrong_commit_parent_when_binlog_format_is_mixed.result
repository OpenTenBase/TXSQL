include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
set @saved_writeset_extraction = @@global.transaction_write_set_extraction;
Warnings:
Warning	1287	'@@transaction_write_set_extraction' is deprecated and will be removed in a future release.
set @saved_dependency_tracking = @@global.binlog_transaction_dependency_tracking;
set global transaction_write_set_extraction=xxhash64;
Warnings:
Warning	1287	'@@transaction_write_set_extraction' is deprecated and will be removed in a future release.
set transaction_write_set_extraction=xxhash64;
Warnings:
Warning	1287	'@@transaction_write_set_extraction' is deprecated and will be removed in a future release.
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
set @saved_replica_preserve_commit_order=@@global.replica_preserve_commit_order;
set @saved_replica_parallel_type=@@global.replica_parallel_type;
Warnings:
Warning	1287	'@@replica_parallel_type' is deprecated and will be removed in a future release.
set @saved_replica_parallel_workers=@@global.replica_parallel_workers;
set global replica_preserve_commit_order=OFF;
set global replica_parallel_type=logical_clock;
Warnings:
Warning	1287	'@@replica_parallel_type' is deprecated and will be removed in a future release.
set global replica_parallel_workers=2;
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
set global replica_preserve_commit_order=@saved_replica_preserve_commit_order;
set global replica_parallel_type=@saved_replica_parallel_type;
Warnings:
Warning	1287	'@@replica_parallel_type' is deprecated and will be removed in a future release.
set global replica_parallel_workers=@saved_replica_parallel_workers;
start replica;
set global binlog_transaction_dependency_tracking = @saved_dependency_tracking;
set global transaction_write_set_extraction = @saved_writeset_extraction;
Warnings:
Warning	1287	'@@transaction_write_set_extraction' is deprecated and will be removed in a future release.
drop table if exists t1;
drop table if exists t2;
drop table if exists t3;
include/rpl_end.inc

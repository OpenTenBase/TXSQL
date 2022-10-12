include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
create database tdb_pt;
use tdb_pt;
create table t1(f1 int);
use tdb_pt;
select * from t1;
f1
insert into t1 values (1);
set binlog_format='statement';
set cdb_pt_table_checksum=true;
use tdb_pt;
create table t2(f1 int);
insert into t2 values (1);
insert into t2 values (2);
update t2 set f1 = 11 where f1 = 1;
delete from t2 where f1 = 11;
select sleep (3);
sleep (3)
0
use tdb_pt;
create table t3(f1 int);
set binlog_format='statement';
set cdb_pt_table_checksum=true;
use tdb_pt;
insert into t3 values (1);
use tdb_pt;
select * from t1;
f1
1
select * from t3;
f1
select * from t2;
ERROR 42S02: Table 'tdb_pt.t2' doesn't exist
use tdb_pt;
drop table t2;
use tdb_pt;
drop table t1;
drop table t3;
drop database tdb_pt;
include/rpl_end.inc

include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
call mtr.add_suppression("Statement violates GTID consistency .*");
set global cdb_more_gtid_feature_supported=OFF;
create table t(id int) engine=innodb;
create table tt(id int, c int) engine=innodb;
create table t1(id int) engine=myisam;
insert into t values(1);
insert into t1 values(1);
====case1====
create table t_0 select * from t;
select * from t_0;
id
1
drop table t_0;
====case2====
begin;
insert into t values(1);
create temporary table tx(id int);
insert into tx values(2);
commit;
select * from tx;
id
2
drop table tx;
====case3====
begin;
insert into t values(3);
insert into t1 values(3);
ERROR HY000: Statement violates GTID consistency: Updates to non-transactional tables can only be done in either autocommitted statements or single-statement transactions, and never in the same statement as updates to transactional tables.
rollback;
set global cdb_more_gtid_feature_supported=ON;
====case1====
create table t_1 select * from t;
select * from t_1;
id
1
1
drop table t_1;
====case2====
begin;
insert into t values(1);
create temporary table tx2(id int);
insert into tx2 values(2);
select * from tx2;
id
2
drop table tx2;
insert into tt values(44, 55);
create temporary table xx(id int);
alter table xx add column c int;
insert into xx select * from tt;
insert into xx values(55, 66);
drop table xx;
drop table tt;
commit;
====case3====
begin;
insert into t values(3);
insert into t1 values(3);
commit;
set binlog_format=statement;
create table t2(id int primary key, c int) engine=innodb;
create table t3(id int primary key, c int) engine=myisam;
begin;
insert into t2 values(1,1);
create temporary table xx (id int primary key, c int);
ERROR HY000: Statement violates GTID consistency: CREATE TEMPORARY TABLE and DROP TEMPORARY TABLE are not allowed inside a transaction or inside a procedure in a transactional context when @@session.binlog_format=STATEMENT.
insert into xx select * from t2;
ERROR 42S02: Table 'test.xx' doesn't exist
insert into t3 select * from xx;
ERROR 42S02: Table 'test.xx' doesn't exist
commit;
drop table t2, t3;
set binlog_format=mixed;
create table t2(id int primary key, c int) engine=innodb;
create table t3(id int primary key, c int) engine=myisam;
begin;
insert into t2 values(1,1);
create temporary table xx (id int primary key, c int);
insert into xx select * from t2;
insert into t3 select * from xx;
ERROR HY000: Statement violates GTID consistency: Updates to non-transactional tables can only be done in either autocommitted statements or single-statement transactions, and never in the same statement as updates to transactional tables.
commit;
drop table xx, t2, t3;
====cleanup====
drop table t, t1;
SET GLOBAL cdb_more_gtid_feature_supported= 0;
include/rpl_end.inc

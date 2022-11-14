include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
#
# Prepare
#
[on master]
create database test1 charset utf8mb4;
use test1;
show create database test1;
Database	Create Database
test1	CREATE DATABASE `test1` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci */ /*!80016 DEFAULT ENCRYPTION='N' */
create temporary table t1 (a varchar(256)) charset utf8mb3;
Warnings:
Warning	1287	'utf8mb3' is deprecated and will be removed in a future release. Please use utf8mb4 instead
#
# Run 
#
[on master]
create table t2 like t1;
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` varchar(256) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3
insert into t2 values("aaa");
[on slave]
include/sync_slave_sql_with_master.inc
use test1;
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` varchar(256) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3
#
# Cleanup 
#
[on master]
drop database test1;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
CREATE TABLE t1 AS SELECT ST_POINTFROMGEOHASH("0123", 4326);
CREATE TABLE t2 AS SELECT ST_POINTFROMGEOHASH("0123", 4326) union select null;
create table t3 SELECT JSON_ARRAY(1, "abc", NULL, TRUE, CURTIME());
create table t4 select now();
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `ST_POINTFROMGEOHASH("0123", 4326)` point NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `ST_POINTFROMGEOHASH("0123", 4326)` geometry DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
show create table t3;
Table	Create Table
t3	CREATE TABLE `t3` (
  `JSON_ARRAY(1, "abc", NULL, TRUE, CURTIME())` json DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
show create table t4;
Table	Create Table
t4	CREATE TABLE `t4` (
  `now()` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
include/sync_slave_sql_with_master.inc
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `ST_POINTFROMGEOHASH("0123", 4326)` point NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `ST_POINTFROMGEOHASH("0123", 4326)` geometry DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
show create table t3;
Table	Create Table
t3	CREATE TABLE `t3` (
  `JSON_ARRAY(1, "abc", NULL, TRUE, CURTIME())` json DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
show create table t4;
Table	Create Table
t4	CREATE TABLE `t4` (
  `now()` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
drop table t1, t2, t3, t4;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

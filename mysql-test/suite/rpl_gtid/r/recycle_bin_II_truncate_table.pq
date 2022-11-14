include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
#
#  Prepare
#
call mtr.add_suppression("access __txsql_recycle_bin__ fail, the query string is");
# open recycle bin on master
set global cdb_recycle_bin_enabled = on;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
# open recycle bin on slave
set global cdb_recycle_bin_enabled = on;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
#
# Run 
#
#
# case1. replicate truncate without cdb_recycle_bin_enabled enabled
#
#
# case1.1. replicate truncate table while the cdb_recycle_bin_enable of
#          slave is off
set global cdb_recycle_bin_enabled= off;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
create table t1 (a int);
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
insert into t1 values(1);
#
# on master
select * from t1;
a
1
truncate t1;
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
select * from t1;
a
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
#
# on slave
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
select * from t1;
a
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
#
# case1.2. replicate truncate table while the cdb_recycle_bin_enable of
#          slave is on
set global cdb_recycle_bin_enabled= on;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
create table t2 (a int);
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
insert into t2 values(1);
#
# on master
select * from t2;
a
1
truncate t2;
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
select * from t2;
a
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
#
# on slave
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
select * from t2;
a
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
#
# case2. differencent table type 
#
#
# case2.1. partition table
#
create table t3 (a int) PARTITION BY LIST (a) (
PARTITION p0 VALUES IN (1),
PARTITION p1 VALUES IN (2)
);
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
insert into t3 values(1);
insert into t3 values(2);
#
# on master
select * from t3;
a
1
2
truncate t3;
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
select * from t3;
a
show create table t3;
Table	Create Table
t3	CREATE TABLE `t3` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
/*!50100 PARTITION BY LIST (`a`)
(PARTITION p0 VALUES IN (1) ENGINE = InnoDB,
 PARTITION p1 VALUES IN (2) ENGINE = InnoDB) */
#
# on slave
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
select * from t3;
a
show create table t3;
Table	Create Table
t3	CREATE TABLE `t3` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
/*!50100 PARTITION BY LIST (`a`)
(PARTITION p0 VALUES IN (1) ENGINE = InnoDB,
 PARTITION p1 VALUES IN (2) ENGINE = InnoDB) */
#
# case2.2. Don't support temporary table
#   drop temporary table with "drop table" will occur ER_NO_SUCH_TABLE error 
CREATE TEMPORARY TABLE t4(
a INT PRIMARY KEY,
b VARCHAR(255)
);
insert into t4 values (1, "abc");
truncate table t4;
ERROR 42S02: Table 'test.t4' doesn't exist
drop temporary table t4;
#
# case2.3. non-innodb engine table 
#
CREATE TABLE t5(a int) ENGINE=MyISAM;
show create table t5;
Table	Create Table
t5	CREATE TABLE `t5` (
  `a` int DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
insert into t5 values(1);
#
# on master
select * from t5;
a
1
truncate t5;
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t5	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
select * from t5;
a
show create table t5;
Table	Create Table
t5	CREATE TABLE `t5` (
  `a` int DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
#
# on slave
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t5	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
select * from t5;
a
show create table t5;
Table	Create Table
t5	CREATE TABLE `t5` (
  `a` int DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
#
# case3. crash safe test
#        
create table t6 (a int);
insert into t6 values(1);
call mtr.add_suppression("Scanned file");
call mtr.add_suppression("is missing!");
#
# case3.1 crash before first commit
#        
set global cdb_recycle_bin_enabled = on;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
set session debug="+d,rb_crash_before_commit1";
#
# crash at rb_crash_before_commit1
truncate t6;
ERROR HY000: Lost connection to MySQL server during query
# Restart the master server
include/rpl_start_server.inc [server_number=1]
select * from t6;
a
1
set session debug="-d,rb_crash_before_commit1";
show variables like 'debug';
Variable_name	Value
debug	
#
# case3.2 crash after first commit
#        
set global cdb_recycle_bin_enabled = on;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
set session debug="+d,rb_crash_after_commit1";
#
# crash at rb_crash_after_commit1
truncate t6;
ERROR HY000: Lost connection to MySQL server during query
# Restart the master server
include/rpl_start_server.inc [server_number=1]
select * from t6;
a
set session debug="-d,rb_crash_after_commit1";
show variables like 'debug';
Variable_name	Value
debug	
drop table if exists t6;
create table t6 (a int);
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t5	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
insert into t6 values(2);
#
# case3.3 crash after second commit
#        
set global cdb_recycle_bin_enabled = on;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
set session debug="+d,rb_crash_after_commit2";
#
# crash at rb_crash_after_commit2
truncate t6;
ERROR HY000: Lost connection to MySQL server during query
# Restart the master server
include/rpl_start_server.inc [server_number=1]
select * from t6;
a
set session debug="-d,rb_crash_after_commit2";
show variables like 'debug';
Variable_name	Value
debug	
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t5	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
#
# case4. miscellany 
#        
#
# case4.1 create table like error 
#        
set global cdb_recycle_bin_enabled = on;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
create table t7(a int);
insert into t7 values(1);
set session debug="+d,rb_fail_to_create_table";
truncate table t7;
ERROR HY000: Long database name and identifier for object resulted in path length exceeding 512 characters. Path: './test/t7'.
select * from t7;
a
1
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t5	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
set session debug="-d,rb_fail_to_create_table";
#
# case4.2 with foreign key 
#  
set global cdb_recycle_bin_enabled = on;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
create table t8_parent(a int primary key);
create table t8_child(
a int not null,
b int not null,
primary key (a),
foreign key (b) references t8_parent(a)
);
insert into t8_parent values(1);
insert into t8_child values(1, 1);
drop table t8_parent;
ERROR HY000: Cannot drop table 't8_parent' referenced by a foreign key constraint 't8_child_ibfk_1' on table 't8_child'.
drop table t8_parent, t8_child;
ERROR HY000: Cannot drop table 't8_parent' referenced by a foreign key constraint 't8_child_ibfk_1' on table 't8_child'.
truncate t8_parent;
ERROR 42000: Cannot truncate a table referenced in a foreign key constraint (`test`.`t8_child`, CONSTRAINT `t8_child_ibfk_1`)
drop table  t8_child;
drop table t8_parent;
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t5	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t8_child	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t8_parent	<DROP_TIME>	<PURGE_TIME>
show tables;
Tables_in_test
t1
t2
t3
t5
t6
t7
call sys.recycle_bin_recycle_table("<RECYCLED_TABLE>");
ERROR HY000: Cannot recovery a table which references to the table in <RECYCLED_TABLE>) first.
#
# case4.3 scheduler thread clear recycle 
#  
drop table t1,t2,t3,t5,t6,t7;
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t3	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t5	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t5	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t6	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t7	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t8_child	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t8_parent	<DROP_TIME>	<PURGE_TIME>
show tables in __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
#
# Cleanup 
# 
set global cdb_recycle_bin_enabled=0;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
set global cdb_recycle_bin_enabled=0;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
[connection master]
include/rpl_end.inc

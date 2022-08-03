include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
#
#  Prepare
#
call mtr.add_suppression("access __cdb_recycle_bin__ fail, the query string is");
# open recycle bin on master
set global cdb_recycle_bin_enabled = on;
# open recycle bin on slave
set global cdb_recycle_bin_enabled = on;
#
# Run 
#
#
# case1. show status about events
#
set global cdb_recycle_bin_enabled= off;
set global cdb_recycle_bin_retention=1;
set global event_scheduler=off;
show variables like 'event_scheduler';
Variable_name	Value
event_scheduler	OFF
set global event_scheduler=on;
show variables like 'event_scheduler';
Variable_name	Value
event_scheduler	ON
show events;
Db	Name	Definer	Time zone	Type	Execute at	Interval value	Interval field	Starts	Ends	Status	Originator	character_set_client	collation_connection	Database Collation
select * from INFORMATION_SCHEMA.EVENTS;
EVENT_CATALOG	EVENT_SCHEMA	EVENT_NAME	DEFINER	TIME_ZONE	EVENT_BODY	EVENT_DEFINITION	EVENT_TYPE	EXECUTE_AT	INTERVAL_VALUE	INTERVAL_FIELD	SQL_MODE	STARTS	ENDS	STATUS	ON_COMPLETION	CREATED	LAST_ALTERED	LAST_EXECUTED	EVENT_COMMENT	ORIGINATOR	CHARACTER_SET_CLIENT	COLLATION_CONNECTION	DATABASE_COLLATION
#
# case2. test auto purge 
#
create table t1 (a int);
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
insert into t1 values(1);
drop table t1;
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
show tables in __cdb_recycle_bin__;
Tables_in___cdb_recycle_bin__
<RECYCLED_TABLE>
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
show tables in __cdb_recycle_bin__;
Tables_in___cdb_recycle_bin__
#
# Cleanup 
# 
set global cdb_recycle_bin_enabled=0;
set global cdb_recycle_bin_enabled=0;
set global cdb_recycle_bin_retention=default;
[connection master]
include/rpl_end.inc

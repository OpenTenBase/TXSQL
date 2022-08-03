include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
#
# recycle bin test
#
call mtr.add_suppression("access __cdb_recycle_bin__ fail, the query string is");
#
# drop temporary table with "drop table" will occur ER_NO_SUCH_TABLE error
set global cdb_recycle_bin_enabled=on;
set global cdb_recycle_bin_enabled= off;

#
# drop table with binlog synchronization
#

# phase1. create table and insert into data
create table t1 (a int);
insert into t1 values (1);
select * from t1;
a
1
create table t2 (a int);
insert into t2 values (1);
select * from t2;
a
1
include/sync_slave_sql_with_master.inc
select * from t1;
a
1
select * from t2;
a
1

# phase2. drop table and select mysql.recycle_bin_info
drop table t1;
drop table t2;
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
show tables from __cdb_recycle_bin__;
Tables_in___cdb_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
include/sync_slave_sql_with_master.inc
# on slave
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t1	<DROP_TIME>	<PURGE_TIME>
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
show tables from __cdb_recycle_bin__;
Tables_in___cdb_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>

# phase3. recovery table t1 from recycle_bin
# recycle_table must login tencentroot
call sys.recycle_bin_recycle_table("<t1_RECYCLED_TABLE>");
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
show tables from __cdb_recycle_bin__;
Tables_in___cdb_recycle_bin__
<RECYCLED_TABLE>
include/sync_slave_sql_with_master.inc
# on slave
select * from mysql.recycle_bin_info;
table_name	origin_schema	origin_table	drop_time	purge_time
<RECYCLED_TABLE>	test	t2	<DROP_TIME>	<PURGE_TIME>
show tables from  __cdb_recycle_bin__;
Tables_in___cdb_recycle_bin__
<RECYCLED_TABLE>

# phase4. recovery table t1 from recycle_bin
#
# clear environment
#
set global cdb_recycle_bin_enabled=0;
use test;
set global cdb_recycle_bin_enabled=off;
drop table t1;
set global cdb_recycle_bin_enabled=on;
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
set global cdb_recycle_bin_enabled=0;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

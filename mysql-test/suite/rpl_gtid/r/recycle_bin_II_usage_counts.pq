include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
#
#  Prepare
#
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
# case1. execute the operation 
#
set global cdb_recycle_bin_enabled= off;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
set global event_scheduler=off;
set global event_scheduler=on;
create table t1 (a int);
insert into t1 values (1);
# drop table
drop table t1;
# recovery table
call sys.recycle_bin_recycle_table("<RECYCLED_TABLE>");
# truncate table
truncate table t1;
select * from t1;
a
# purge table
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
#
# case2. check the statistic value
#
include/assert.inc [drop_table_counts = 1]
include/assert.inc [truncate_table_counts = 1]
include/assert.inc [recovery_table_counts = 1]
include/assert.inc [purge_table_counts = 1]
#
# Cleanup 
# 
set global cdb_recycle_bin_enabled=0;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
drop table t1;
set global cdb_recycle_bin_enabled=0;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
[connection master]
include/rpl_end.inc

# open recycle bin
set global cdb_recycle_bin_enabled = on;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.
create table t1(a int);
#
# test prepare stmt from "truncate ..."
#
prepare stmt1 from "truncate table t1";
execute stmt1;
execute stmt1;
select count(*) from mysql.recycle_bin_info;
count(*)
2
show tables from  __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
#
# test prepare stmt from "drop ..."
#
prepare stmt2 from "drop table t1";
execute stmt2;
create table t1(a int);
execute stmt2;
select count(*) from mysql.recycle_bin_info;
count(*)
4
show tables from  __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
<RECYCLED_TABLE>
#
# clear environment
#
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
call sys.recycle_bin_purge_table("<RECYCLED_TABLE>");
set global cdb_recycle_bin_enabled=0;
Warnings:
Warning	1287	'@@cdb_recycle_bin_enabled' is deprecated and will be removed in a future release. Please use txsql_recycle_bin_enabled instead.

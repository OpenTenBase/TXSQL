
# ==== Purpose ====
#
# show and check truncate table 
#
# ==== Usage ====
#
# Parameters:
#
#

--echo #
--echo # on master
eval select * from $table_name;
eval truncate $table_name;

--replace_column 1 <RECYCLED_TABLE>  4 <DROP_TIME>  5 <PURGE_TIME>
select * from mysql.recycle_bin_info;
--replace_column 1 <RECYCLED_TABLE> 
show tables in __txsql_recycle_bin__;
eval select * from $table_name;
eval show create table $table_name;

--echo #
--echo # on slave
--sync_slave_with_master
--connection slave 
--replace_column 1 <RECYCLED_TABLE>  4 <DROP_TIME>  5 <PURGE_TIME>
select * from mysql.recycle_bin_info;
--replace_column 1 <RECYCLED_TABLE> 
show tables in __txsql_recycle_bin__;
eval select * from $table_name;
eval show create table $table_name;

--connection master

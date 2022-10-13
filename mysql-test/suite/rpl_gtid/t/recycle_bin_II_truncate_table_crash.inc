
# ==== Purpose ====
#
# show and check truncate table 
#
# ==== Usage ====
#
# Parameters:
#
#

connect (con_tencentroot_master_$crash_point, 127.0.0.1, tencentroot, '', '', $MASTER_MYPORT);
set global cdb_recycle_bin_enabled = on;

--connection master 
--exec echo "wait" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
eval set session debug="+d,$crash_point";
--echo #
--echo # crash at $crash_point
# 2013 - CR_SERVER_LOST
--error 2013
truncate t6;
--source include/wait_until_disconnected.inc

--enable_reconnect
--echo # Restart the master server
--let $rpl_server_number= 1
--source include/rpl_start_server.inc

--disable_reconnect
select * from t6;

eval set session debug="-d,$crash_point";
show variables like 'debug';

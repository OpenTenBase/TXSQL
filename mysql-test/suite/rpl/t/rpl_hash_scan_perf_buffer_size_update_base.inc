
--echo # update a column

connection conn_root0;
eval update t1 set a=repeat($update_str, 650);
connection conn_root1;
eval update t2 set a=repeat($update_str, 650);

--echo # on slave
let $slave_sync_timeout = 86400;
--source include/wait_for_slave_to_sync_with_master.inc
connection slave;
eval select count(*) from t1 where a=repeat($update_str, 650);
eval select count(*) from t2 where a=repeat($update_str, 650);

--echo # ==============================================================
--echo # Switch test

connection conn_root_slave;
let $slave_parallel_type_saved=`select @@slave_parallel_type`;
let $slave_parallel_workers_saved=`select @@slave_parallel_workers`;
let $cdb_hash_scan_max_agg_event_size_saved=`select @@cdb_hash_scan_max_agg_event_size`;
let $slave_rows_search_algorithms_saved=`select @@slave_rows_search_algorithms`;

call mtr.add_suppression("The slave coordinator and worker threads are stopped, possibly leaving data in inconsistent state.");


connection conn_root_slave;
set global slave_rows_search_algorithms='INDEX_SCAN,TABLE_SCAN';
set global cdb_hash_scan_max_agg_event_size=16384;
STOP SLAVE SQL_THREAD;
source include/wait_for_slave_sql_to_stop.inc;
set global slave_parallel_type=LOGICAL_CLOCK;
set global slave_parallel_workers=2;
start slave SQL_THREAD;
source include/wait_for_slave_sql_to_start.inc;

connection conn_root0;
update t1 set a=repeat('b', 650);

connection conn_root_slave;

let $switch_count= 0;
while ($switch_count < 15)
{
  set global cdb_hash_scan_max_agg_event_size=16384;
  sleep 0.5;
  set global cdb_hash_scan_max_agg_event_size=0;
  sleep 0.5;
  inc $switch_count;
}

set global cdb_hash_scan_max_agg_event_size=16384;

connection master;
let $slave_sync_timeout = 86400;
--source include/wait_for_slave_to_sync_with_master.inc

connection conn_root0;
update t1 set a=repeat('c', 650);

let $slave_sync_timeout = 86400;
--source include/wait_for_slave_to_sync_with_master.inc

--echo #
--echo # Switch test cleanup
connection conn_root_slave;
STOP SLAVE SQL_THREAD;
source include/wait_for_slave_sql_to_stop.inc;
eval set global slave_parallel_type='$slave_parallel_type_saved';
eval set global slave_parallel_workers=$slave_parallel_workers_saved;
start slave SQL_THREAD;
source include/wait_for_slave_sql_to_start.inc;
eval set global cdb_hash_scan_max_agg_event_size=$cdb_hash_scan_max_agg_event_size_saved;
eval set global slave_rows_search_algorithms='$slave_rows_search_algorithms_saved';
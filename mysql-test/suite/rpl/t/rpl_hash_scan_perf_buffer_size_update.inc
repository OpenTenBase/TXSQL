
--echo # ==============================================================
--echo # Update test
connection conn_tencentroot_slave;
let $slave_parallel_type_saved=`select @@slave_parallel_type`;
let $slave_parallel_workers_saved=`select @@slave_parallel_workers`;
let $cdb_hash_scan_max_agg_event_size_saved=`select @@cdb_hash_scan_max_agg_event_size`;
let $slave_rows_search_algorithms_saved=`select @@slave_rows_search_algorithms`;


--echo #
--echo # case 1: HASH_SCAN
connection conn_tencentroot_slave;
set global slave_rows_search_algorithms='INDEX_SCAN,HASH_SCAN';
set global cdb_hash_scan_max_agg_event_size=16384;
let update_str='b';
--source rpl_hash_scan_perf_buffer_size_update_base.inc

--echo #
--echo # case 2: TABLE_SCAN
connection conn_tencentroot_slave;
set global slave_rows_search_algorithms='INDEX_SCAN,TABLE_SCAN';
let update_str='c';
--source rpl_hash_scan_perf_buffer_size_update_base.inc

--echo #
--echo # case 3: HASH_SCAN + MTS
connection conn_tencentroot_slave;
STOP SLAVE SQL_THREAD;
source include/wait_for_slave_sql_to_stop.inc;
set global slave_parallel_type=LOGICAL_CLOCK;
set global slave_parallel_workers=2;
start slave SQL_THREAD;
source include/wait_for_slave_sql_to_start.inc;
set global slave_rows_search_algorithms='INDEX_SCAN,HASH_SCAN';
let update_str='d';
--source rpl_hash_scan_perf_buffer_size_update_base.inc

--echo #
--echo # case 4: TABLE_SCAN + MTS
connection conn_tencentroot_slave;
set global slave_rows_search_algorithms='INDEX_SCAN,TABLE_SCAN';
let update_str='e';
--source rpl_hash_scan_perf_buffer_size_update_base.inc


--echo # ==============================================================
--echo # Update test cleanup
connection master;
--sync_slave_with_master
connection conn_tencentroot_slave;
STOP SLAVE SQL_THREAD;
source include/wait_for_slave_sql_to_stop.inc;

eval set global slave_parallel_type='$slave_parallel_type_saved';
eval set global slave_parallel_workers=$slave_parallel_workers_saved;
start slave SQL_THREAD;
source include/wait_for_slave_sql_to_start.inc;
eval set global cdb_hash_scan_max_agg_event_size=$cdb_hash_scan_max_agg_event_size_saved;
eval set global slave_rows_search_algorithms='$slave_rows_search_algorithms_saved';

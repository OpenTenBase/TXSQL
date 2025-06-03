# Hash scan optimization test framework, it help other testcase generate data.
# The implementation process is divided three step:
# 1. Base prepare: insert data in different tables using parallel;
# 2. Base run: run testcase script, it is given by the variable of $run_script;
# 3. Base cleanup: cleanup executed data
# usage:
#    let $target= 2;
#    let $con_limit= 2;
#    let $run_script=rpl_hash_scan_perf_buffer_size_update.inc;
#    --source rpl_hash_scan_perf_buffer_size_base.inc
#
#
--source include/master-slave.inc
--source include/have_binlog_format_row.inc
--let $rpl_set_enforce_gtid_consistency= 1
--let $rpl_gtid_mode= ON
--source include/rpl_set_gtid_mode.inc

--echo # ==============================================================
--echo # Base prepare
connection master;

--disable_abort_on_error
--disable_query_log
--disable_result_log


--echo #
--echo # step1. create $table_limit tables

let $table_count= 1;
while ($table_count <= $table_limit)
{
  eval CREATE TABLE t$table_count(
    id int not null,
    a varchar(1000) not null
  ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
  inc $table_count;
}

--echo #
--echo # step2. establish $con_limit connections

let $con_count= 0;
while ($con_count < $con_limit)
{
  connect(conn_root$con_count,localhost,root,,);
  inc $con_count;
}
connect (conn_root_slave, 127.0.0.1, root, ,, $SLAVE_MYPORT,);

--echo #
--echo # step3. save some of variables value
connection conn_root_slave;
let $base_cdb_hash_scan_max_agg_event_size_saved=`select @@cdb_hash_scan_max_agg_event_size`;
let $base_slave_rows_search_algorithms_saved=`select @@slave_rows_search_algorithms`;

set global cdb_hash_scan_max_agg_event_size=16384;
set global slave_rows_search_algorithms='INDEX_SCAN,HASH_SCAN';


--echo #
--echo # step4. insert data for each table
let $global_id=0;
let $iteration= 0;
let $len_start= 500;
let $len_end= 800;
while ($iteration < $target)
{
  let $con_count= 0;
  while ($con_count < $con_limit)
  {
    connection conn_root$con_count;
    let $table_sel=`select ($con_count%$table_limit)+1`;
    let $query= insert into t$table_sel value ($global_id, '$global_id');
    inc $global_id;
    let $len= $len_start;
    while ($len < $len_end)
    {
      let $query= $query,($global_id, concat('$global_id', repeat('a', $len)));
      inc $global_id;
      inc $len;
    }
    --send_eval $query
    inc $con_count;
  }

  let $con_count= 0;
  while ($con_count < $con_limit)
  {
    connection conn_root$con_count;
    reap;
    inc $con_count;
  }

  inc $iteration;
  --echo ## Prepare progress rate: iteration: $iteration , target: $target
}

--enable_result_log
--enable_query_log
--enable_abort_on_error

--echo #
--echo # On slave
--echo # Sync data with master and check data
let $slave_sync_timeout= 86400;
--source include/wait_for_slave_to_sync_with_master.inc
connection slave;

let $table_count= 1;
while ($table_count <= $table_limit)
{
  eval select count(*) from t$table_count;
  inc $table_count;
}

--echo # ==============================================================
--echo # Base run
--source $run_script


--echo # ==============================================================
--echo # Base cleanup
connection master;

--echo #
--echo # step1. drop table
--disable_query_log
let $table_count= 1;
while ($table_count <= $table_limit)
{
  eval delete from t$table_count;
  eval DROP TABLE t$table_count;
  inc $table_count;
}
--enable_query_log

--let $rpl_set_enforce_gtid_consistency= 0
--let $rpl_gtid_mode= OFF
--source include/rpl_set_gtid_mode.inc

--echo #
--echo # step2. disconnect link from master
let $con_count= 0;
while ($con_count < $con_limit)
{
  disconnect conn_root$con_count;
  inc $con_count;
}
--sync_slave_with_master
connection conn_root_slave;
eval set global cdb_hash_scan_max_agg_event_size=$base_cdb_hash_scan_max_agg_event_size_saved;
eval set global slave_rows_search_algorithms='$base_slave_rows_search_algorithms_saved';

connection master;
--source include/rpl_end.inc

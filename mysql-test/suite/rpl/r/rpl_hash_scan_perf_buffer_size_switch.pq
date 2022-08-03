include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
include/rpl_set_gtid_mode.inc [ON on servers 1,2]
# ==============================================================
# Base prepare
#
# step1. create 1 tables
#
# step2. establish 5 connections
#
# step3. save some of variables value
#
# step4. insert data for each table
## Prepare progress rate: iteration: 1 , target: 1
#
# On slave
# Sync data with master and check data
stop slave;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
start slave;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
select count(*) from t1;
count(*)
1505
# ==============================================================
# Base run
# ==============================================================
# Switch test
call mtr.add_suppression("The slave coordinator and worker threads are stopped, possibly leaving data in inconsistent state.");
set global slave_rows_search_algorithms='INDEX_SCAN,TABLE_SCAN';
Warnings:
Warning	1287	'@@slave_rows_search_algorithms' is deprecated and will be removed in a future release.
set global cdb_hash_scan_max_agg_event_size=16384;
STOP SLAVE SQL_THREAD;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
include/wait_for_slave_sql_to_stop.inc
set global slave_parallel_type=LOGICAL_CLOCK;
set global slave_parallel_workers=2;
start slave SQL_THREAD;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
include/wait_for_slave_sql_to_start.inc
update t1 set a=repeat('b', 650);
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
set global cdb_hash_scan_max_agg_event_size=0;
set global cdb_hash_scan_max_agg_event_size=16384;
stop slave;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
start slave;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
update t1 set a=repeat('c', 650);
stop slave;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
start slave;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
#
# Switch test cleanup
STOP SLAVE SQL_THREAD;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
include/wait_for_slave_sql_to_stop.inc
set global slave_parallel_type='DATABASE';
set global slave_parallel_workers=0;
start slave SQL_THREAD;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
include/wait_for_slave_sql_to_start.inc
set global cdb_hash_scan_max_agg_event_size=16384;
set global slave_rows_search_algorithms='INDEX_SCAN,HASH_SCAN';
Warnings:
Warning	1287	'@@slave_rows_search_algorithms' is deprecated and will be removed in a future release.
# ==============================================================
# Base cleanup
#
# step1. drop table
include/rpl_set_gtid_mode.inc [OFF on servers 1,2]
#
# step2. disconnect link from master
set global cdb_hash_scan_max_agg_event_size=0;
set global slave_rows_search_algorithms='INDEX_SCAN,HASH_SCAN';
Warnings:
Warning	1287	'@@slave_rows_search_algorithms' is deprecated and will be removed in a future release.
include/rpl_end.inc

include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
include/rpl_set_gtid_mode.inc [ON on servers 1,2]
# ==============================================================
# Base prepare
#
# step1. create 2 tables
#
# step2. establish 2 connections
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
301
select count(*) from t2;
count(*)
301
# ==============================================================
# Base run
# ==============================================================
# Update test
#
# case 1: HASH_SCAN
set global slave_rows_search_algorithms='INDEX_SCAN,HASH_SCAN';
Warnings:
Warning	1287	'@@slave_rows_search_algorithms' is deprecated and will be removed in a future release.
set global cdb_hash_scan_max_agg_event_size=16384;
# update a column
update t1 set a=repeat('b', 650);
update t2 set a=repeat('b', 650);
# on slave
stop slave;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
start slave;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
select count(*) from t1 where a=repeat('b', 650);
count(*)
301
select count(*) from t2 where a=repeat('b', 650);
count(*)
301
#
# case 2: TABLE_SCAN
set global slave_rows_search_algorithms='INDEX_SCAN,TABLE_SCAN';
Warnings:
Warning	1287	'@@slave_rows_search_algorithms' is deprecated and will be removed in a future release.
# update a column
update t1 set a=repeat('c', 650);
update t2 set a=repeat('c', 650);
# on slave
stop slave;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
start slave;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
select count(*) from t1 where a=repeat('c', 650);
count(*)
301
select count(*) from t2 where a=repeat('c', 650);
count(*)
301
#
# case 3: HASH_SCAN + MTS
STOP SLAVE SQL_THREAD;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
include/wait_for_slave_sql_to_stop.inc
set global slave_parallel_type=LOGICAL_CLOCK;
Warnings:
Warning	1287	'@@slave_parallel_type' is deprecated and will be removed in a future release. Please use replica_parallel_type instead.
set global slave_parallel_workers=2;
Warnings:
Warning	1287	'@@slave_parallel_workers' is deprecated and will be removed in a future release. Please use replica_parallel_workers instead.
start slave SQL_THREAD;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
include/wait_for_slave_sql_to_start.inc
set global slave_rows_search_algorithms='INDEX_SCAN,HASH_SCAN';
Warnings:
Warning	1287	'@@slave_rows_search_algorithms' is deprecated and will be removed in a future release.
# update a column
update t1 set a=repeat('d', 650);
update t2 set a=repeat('d', 650);
# on slave
stop slave;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
start slave;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
select count(*) from t1 where a=repeat('d', 650);
count(*)
301
select count(*) from t2 where a=repeat('d', 650);
count(*)
301
#
# case 4: TABLE_SCAN + MTS
set global slave_rows_search_algorithms='INDEX_SCAN,TABLE_SCAN';
Warnings:
Warning	1287	'@@slave_rows_search_algorithms' is deprecated and will be removed in a future release.
# update a column
update t1 set a=repeat('e', 650);
update t2 set a=repeat('e', 650);
# on slave
stop slave;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
start slave;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
select count(*) from t1 where a=repeat('e', 650);
count(*)
301
select count(*) from t2 where a=repeat('e', 650);
count(*)
301
# ==============================================================
# Update test cleanup
STOP SLAVE SQL_THREAD;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
include/wait_for_slave_sql_to_stop.inc
set global slave_parallel_type='LOGICAL_CLOCK';
Warnings:
Warning	1287	'@@slave_parallel_type' is deprecated and will be removed in a future release. Please use replica_parallel_type instead.
set global slave_parallel_workers=4;
Warnings:
Warning	1287	'@@slave_parallel_workers' is deprecated and will be removed in a future release. Please use replica_parallel_workers instead.
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

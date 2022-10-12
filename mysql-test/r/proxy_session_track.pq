#
# Default values of the session track variables
#

show variables like '%session_track%';
Variable_name	Value
session_track_gtids	OFF
session_track_lock	OFF
session_track_schema	ON
session_track_state_change	OFF
session_track_system_variables	time_zone,autocommit,character_set_client,character_set_results,character_set_connection
session_track_transaction_info	OFF
session_track_udf_var	OFF

# Turn on session_track_lock
set session_track_lock=1;

##Test for ftwrl
flush table with read lock;
-- Tracker : SESSION_TRACK_TXSQL_LOCK
-- SESSION_LOCK_FTWRL

unlock tables;
-- Tracker : SESSION_TRACK_TXSQL_LOCK
-- SESSION_UNLOCK

create database tdb1;
use tdb1;
-- Tracker : SESSION_TRACK_SCHEMA
-- tdb1

create table t1 (f1 int);
flush table t1 with read lock;
-- Tracker : SESSION_TRACK_TXSQL_LOCK
-- SESSION_LOCK_FTWRL

unlock tables;
-- Tracker : SESSION_TRACK_TXSQL_LOCK
-- SESSION_UNLOCK


#Test for last_insert_id
# Turn on last_insert_id
set session_track_system_variables=last_insert_id;
select last_insert_id(101);
last_insert_id(101)
101

#Test for user var
##single user variable
set session_track_udf_var=1;
set @txsql_proxy_var1='a';
-- Tracker : SESSION_TRACK_TXSQL_UDF_VARS
-- txsql_proxy_var1=a

set @txsql_proxy_var2=1;
-- Tracker : SESSION_TRACK_TXSQL_UDF_VARS
-- txsql_proxy_var2=1

set @txsql_proxy_var3=1.1;
-- Tracker : SESSION_TRACK_TXSQL_UDF_VARS
-- txsql_proxy_var3=1.1


##multi user variable
set @multi1='a', @multi2=1, @multi3=1.1;
-- Tracker : SESSION_TRACK_TXSQL_UDF_VARS
-- multi1=a
-- multi2=1
-- multi3=1.1


#clean up and restore 
drop database tdb1;
-- Tracker : SESSION_TRACK_SCHEMA
-- 

set session_track_lock=default;
set session_track_system_variables=default;
set session_track_udf_var=default;

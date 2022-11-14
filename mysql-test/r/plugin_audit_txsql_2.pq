install plugin audit_txsql soname "audit_txsql.so";
file is not exist in current configuration
file is not exist in current configuration
set global audit_txsql_audit_mode="all";
set global audit_txsql_rotate_write_log=OFF;
Found append pos file
Found append log file
file is not exist in current configuration
create database audit_2;
use audit_2;
create table t (a int primary key);
use test;
show tables;
Tables_in_test
drop database audit_2;
set global audit_txsql_filter_db="";
set global audit_txsql_rotate_write_log=on;
set global audit_txsql_filter_user="root#admin#abc";
set global audit_txsql_filter_user="#";
set global audit_txsql_filter_user="";
set global audit_txsql_filter_user="root#";
set global audit_txsql_filter_user="#root#";
set global audit_txsql_filter_user="root";
set global audit_txsql_filter_ip="127.0.0.1#127.0.0.10";
set global audit_txsql_filter_ip="9.30.17.135";
set global audit_txsql_rotate_write_log=ON;
Found rotate pos file
Found rotate log file
create database audit_2;
use audit_2;
create table t (a int primary key);
use test;
show tables;
Tables_in_test
drop database audit_2;
set global audit_txsql_audit_dir=@@global.tmpdir;
show databases;
Database
information_schema
mtr
mysql
performance_schema
sys
test
Found rotate pos file
Found rotate log file
create database audit_2;
use audit_2;
create table t (a int primary key);
use test;
show tables;
Tables_in_test
drop database audit_2;
set global audit_txsql_rotate_write_log=off;
Found append pos file
Found append log file
set global audit_txsql_log_file_max_size=10485760;
set global audit_txsql_rotate_file_count = 3;
set global audit_txsql_truncate_length = 2048;
use test;
create table t(a int primary key);
insert into t values (1), (2), (3);
select * from t;
a
1
2
3
select * from t where a=1;
a
1
update t set a=4 where a=3;
delete from t;
drop table t;
set global audit_txsql_log_file_max_size = 8*1024*1024;
set global audit_txsql_truncate_length = 2048;
set global audit_txsql_rotate_file_count = 3;
show status like "%ignore_actions%";
Variable_name	Value
audit_txsql_ignore_actions	0
show status like "%truncate_count%";
Variable_name	Value
audit_txsql_truncate_counts	0
uninstall plugin audit_txsql;
Warnings:
Warning	1620	Plugin is busy and will be uninstalled on shutdown

show variables like "%audit_txsql%";
Variable_name	Value
install plugin audit_txsql soname 'audit_txsql.so';
set global audit_txsql_rotate_write_log=ON;
set global audit_txsql_audit_mode="ALL";
set global audit_txsql_audit_dir=@@global.datadir;
Found pos file
Found log file
set global audit_txsql_audit_dir=@@global.tmpdir;
show databases;
Database
information_schema
mtr
mysql
performance_schema
sys
test
Found pos file
Found log file
set global audit_txsql_audit_mode="filter";
set global audit_txsql_filter_db="test#mysql#audit";
create database audit;
use audit;
create table t (a int);
use test;
show tables;
Tables_in_test
drop database audit;
set global audit_txsql_filter_db="";
set global audit_txsql_filter_user="root#admin#abc";
set global audit_txsql_filter_user="#";
set global audit_txsql_filter_user="";
set global audit_txsql_filter_user="root#";
set global audit_txsql_filter_user="#root#";
set global audit_txsql_filter_user="root";
set global audit_txsql_filter_ip="127.0.0.1#127.0.0.10";
set global audit_txsql_filter_ip="192.168.0.1";
set global audit_txsql_log_file_max_size=10485760;
set global audit_txsql_rotate_file_count = 3;
set global audit_txsql_truncate_length = 2048;
use test;
create table t(a int);
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

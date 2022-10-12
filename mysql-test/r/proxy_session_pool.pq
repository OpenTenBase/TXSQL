# Proxy request collation return for session pool.
SET SESSION DEBUG='+d,proxy_collation_change';
set names 'utf8mb4' collate 'utf8mb4_unicode_ci';
# Proxy request found_rows() value immediately
SET SESSION DEBUG='+d,proxy_found_rows_return';
SET SESSION SESSION_TRACK_UDF_VAR='ON';
create table tfr(f1 int, f2 varchar(10));
insert into tfr value (1, '1'), (2, '2'), (3, '3'),(4, '4'),(4, '4');
SELECT SQL_CALC_FOUND_ROWS * FROM tfr LIMIT 2,2;
f1	f2
3	3
4	4
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
Warnings:
# Proxy request row_count() value immediately
SET SESSION DEBUG='+d,proxy_found_rows_return';
SET SESSION SESSION_TRACK_UDF_VAR='ON';
create table trc(f1 int, f2 varchar(10));
insert into trc value (1, '1'), (2, '2'), (3, '3'),(4, '4'),(4, '4');
SET SESSION DEBUG='+d,proxy_row_count_return';
SET SESSION SESSION_TRACK_UDF_VAR='ON';
update trc set f1 = 10 where f1 = 1;
select row_count();
row_count()
1
# Proxy set found_rows return value
set txsql_found_rows = 100;
select found_rows();
found_rows()
100
Warning	1287	FOUND_ROWS() is deprecated and will be removed in a future release. Consider using COUNT(*) instead.
Warnings:
# Proxy set row_count return value
set txsql_row_count = 200;
select row_count();
row_count()
200
SET SESSION DEBUG='-d,proxy_collation_change';
SET SESSION DEBUG='-d,proxy_found_rows_return';
SET SESSION DEBUG='-d,proxy_row_count_return';
drop table tfr;
drop table trc;
# Proxy set database
SET SESSION session_track_system_variables='txsql_use_db';
SET SESSION DEBUG='+d,proxy_use_db_return';
create database tdb1;
set txsql_use_db = 'tdb1';
select database();
database()
tdb1
set txsql_use_db = '';
select database();
database()
NULL
set txsql_use_db = 'asdfd';
ERROR 42000: Unknown database 'asdfd'
SET SESSION DEBUG='+d,proxy_use_db_return';
SET SESSION session_track_system_variables=default;
drop database tdb1;

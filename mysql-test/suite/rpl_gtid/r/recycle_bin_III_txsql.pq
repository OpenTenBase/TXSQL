include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
#
#  Prepare
#
# open recycle bin on master
set global txsql_recycle_bin_enabled = on;
# open recycle bin on slave
set global txsql_recycle_bin_enabled = on;
#
# Run 
#
#
# case1. Test clear stmt
#
#
# case1.1. clear table while the txsql_recycle_bin_enable of slave is off
#
set global txsql_recycle_bin_enabled= off;
CREATE DATABASE test_recycle_bin;
use test_recycle_bin;
create table aa(a int);
create table bb(a int);
create table cc(a int);
create table dd(a int);
create view vb as select * from bb;
drop table aa;
drop table bb;
drop table cc;
drop table dd;
create table aa(a int);
create table bb(a int);
create table cc(a int);
create table dd(a int);
drop database test_recycle_bin;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
clear recycle_bin test_recycle_bin.zz;
Warnings:
Warning	4193	No corresponding record was found in recycle_bin.
clear recycle_bin test_recycle_bin.aa before timestamp(DATE_SUB(NOW(), INTERVAL 10 DAY));
Warnings:
Warning	4193	No corresponding record was found in recycle_bin.
clear recycle_bin test_recycle_bin.aa before timestamp(DATE_ADD(NOW(), INTERVAL 10 DAY));
clear recycle_bin all test_recycle_bin.bb;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	aa	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	aa	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
clear recycle_bin;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
#
# case1.2. clear table while the txsql_recycle_bin_enable of slave is on
#
set global txsql_recycle_bin_enabled= off;
CREATE DATABASE test_recycle_bin;
use test_recycle_bin;
create table aa(a int);
create table bb(a int);
create table cc(a int);
create table dd(a int);
create view vb as select * from bb;
drop table aa;
drop table bb;
drop table cc;
drop table dd;
create table aa(a int);
create table bb(a int);
create table cc(a int);
create table dd(a int);
drop database test_recycle_bin;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
clear recycle_bin test_recycle_bin.zz;
Warnings:
Warning	4193	No corresponding record was found in recycle_bin.
clear recycle_bin test_recycle_bin.aa before timestamp(DATE_SUB(NOW(), INTERVAL 10 DAY));
Warnings:
Warning	4193	No corresponding record was found in recycle_bin.
clear recycle_bin test_recycle_bin.aa before timestamp(DATE_ADD(NOW(), INTERVAL 10 DAY));
clear recycle_bin all test_recycle_bin.bb;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	aa	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	aa	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
clear recycle_bin;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
#
# case2. Test restore stmt
#
#
# case2.1. restore table while the txsql_recycle_bin_enable of slave is off
#
set global txsql_recycle_bin_enabled= off;
CREATE DATABASE test_recycle_bin;
use test_recycle_bin;
create table aa(a int);
create table bb(a int);
create table cc(a int);
create table dd(a int);
create view vb as select * from bb;
drop table aa;
drop table bb;
drop table cc;
drop table dd;
create table aa(a int);
create table bb(a int);
create table cc(a int);
create table dd(a int);
drop database test_recycle_bin;
restore test_recycle_bin.aa from recycle_bin;
ERROR 42000: Unknown database 'test_recycle_bin'
restore database test_recycle_bin from recycle_bin;
ERROR HY000: There's more than one table with test_recycle_bin.aa, please specify timestamp to choose the correct one
create database test_recycle_bin;
restore test_recycle_bin.zz from recycle_bin;
ERROR HY000: No corresponding record was found in recycle_bin.
restore test_recycle_bin.aa from recycle_bin with recycle_name aaa;
ERROR HY000: No corresponding record was found in recycle_bin.
restore test_recycle_bin.aa from recycle_bin with timestamp(DATE_ADD(NOW(), INTERVAL 10 DAY));
ERROR HY000: No corresponding record was found in recycle_bin.
restore test_recycle_bin.aa from recycle_bin;
restore test_recycle_bin.bb from recycle_bin with recycle_name <recycle_name>;
restore test_recycle_bin.cc from recycle_bin with timestamp(<drop_tim>);
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
restore database test_recycle_bin from recycle_bin;
ERROR HY000: There's more than one table with test_recycle_bin.dd, please specify timestamp to choose the correct one
set global txsql_recycle_bin_enabled= off;
drop database test_recycle_bin;
create database test_recycle_bin;
set global txsql_recycle_bin_enabled= on;
restore database test_recycle_bin from recycle_bin;
ERROR HY000: There's more than one table with test_recycle_bin.dd, please specify timestamp to choose the correct one
clear recycle_bin test_recycle_bin.dd;
restore database test_recycle_bin from recycle_bin;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
drop database test_recycle_bin;
clear recycle_bin;
#
# case2.2. restore table while the txsql_recycle_bin_enable of slave is on
#
set global txsql_recycle_bin_enabled= on;
CREATE DATABASE test_recycle_bin;
use test_recycle_bin;
create table aa(a int);
create table bb(a int);
create table cc(a int);
create table dd(a int);
create view vb as select * from bb;
drop table aa;
drop table bb;
drop table cc;
drop table dd;
create table aa(a int);
create table bb(a int);
create table cc(a int);
create table dd(a int);
drop database test_recycle_bin;
restore test_recycle_bin.aa from recycle_bin;
ERROR 42000: Unknown database 'test_recycle_bin'
restore database test_recycle_bin from recycle_bin;
ERROR HY000: There's more than one table with test_recycle_bin.aa, please specify timestamp to choose the correct one
create database test_recycle_bin;
restore test_recycle_bin.zz from recycle_bin;
ERROR HY000: No corresponding record was found in recycle_bin.
restore test_recycle_bin.aa from recycle_bin with recycle_name aaa;
ERROR HY000: No corresponding record was found in recycle_bin.
restore test_recycle_bin.aa from recycle_bin with timestamp(DATE_ADD(NOW(), INTERVAL 10 DAY));
ERROR HY000: No corresponding record was found in recycle_bin.
restore test_recycle_bin.aa from recycle_bin;
restore test_recycle_bin.bb from recycle_bin with recycle_name <recycle_name>;
restore test_recycle_bin.cc from recycle_bin with timestamp(<drop_tim>);
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
#
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	dd	#	#	#
test_recycle_bin	bb	#	#	#
test_recycle_bin	cc	#	#	#
test_recycle_bin	dd	#	#	#
restore database test_recycle_bin from recycle_bin;
ERROR HY000: There's more than one table with test_recycle_bin.dd, please specify timestamp to choose the correct one
set global txsql_recycle_bin_enabled= off;
drop database test_recycle_bin;
create database test_recycle_bin;
set global txsql_recycle_bin_enabled= on;
restore database test_recycle_bin from recycle_bin;
ERROR HY000: There's more than one table with test_recycle_bin.dd, please specify timestamp to choose the correct one
clear recycle_bin test_recycle_bin.dd;
restore database test_recycle_bin from recycle_bin;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
drop database test_recycle_bin;
clear recycle_bin;
#
# case3. Test recycle_bin_max_size and recycle_bin_drop_if_exceed_limit
#
use test;
create table aa(a int);
create table bb(a int);
set global txsql_recycle_bin_max_size = 1;
set global txsql_recycle_bin_drop_if_exceed_limit = 0;
drop table bb;
drop table aa;
ERROR HY000: Exceed limit size 1 of recycle bin
set global txsql_recycle_bin_drop_if_exceed_limit = 1;
drop table aa;
Warnings:
Warning	4194	Exceed limit size 1 of recycle bin
set global txsql_recycle_bin_max_size = default;
set global txsql_recycle_bin_drop_if_exceed_limit = default;
clear recycle_bin;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
#
# case4. Test recycle sequence
#
create database db_a;
use db_a;
create tdsql_sequence sb_seq;
select sb_seq.nextval;
sb_seq.nextval
1
select * from mysql.tdsql_sequences;
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
db_a	sb_seq	20	1	1	9223372036854775807	1	0	20
drop database db_a;
select * from mysql.tdsql_sequences;
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
__recycle_bin__db_a	sb_seq	20	1	1	9223372036854775807	1	0	20
create database db_a;
restore database db_a from RECYCLE_BIN;
Warnings:
Warning	4193	No corresponding record was found in recycle_bin.
use db_a;
select sb_seq.nextval;
sb_seq.nextval
21
select * from mysql.tdsql_sequences;
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
db_a	sb_seq	40	1	1	9223372036854775807	1	0	20
use db_a;
select sb_seq.nextval;
sb_seq.nextval
41
drop database db_a;
clear recycle_bin;
Warnings:
Warning	4193	No corresponding record was found in recycle_bin.
#
# case5. Test recycle function and procedure
#
create database db_a;
use db_a;
CREATE FUNCTION hello (s CHAR(20)) RETURNS CHAR(50) DETERMINISTIC RETURN CONCAT('Hello, ',s,'!');
select hello("wx");
hello("wx")
Hello, wx!
drop database db_a;
create database db_a;
use db_a;
select hello("wx");
ERROR 42000: FUNCTION db_a.hello does not exist
restore database db_a from RECYCLE_BIN;
Warnings:
Warning	4193	No corresponding record was found in recycle_bin.
select hello("wx2");
ERROR 42000: FUNCTION db_a.hello does not exist
use db_a;
set allow_access_dd_tables=on;
select * from mysql.routines where name like '%hello%';
id	schema_id	name	type	result_data_type	result_data_type_utf8	result_is_zerofill	result_is_unsigned	result_char_length	result_numeric_precision	result_numeric_scale	result_datetime_precision	result_collation_id	definition	definition_utf8	parameter_str	is_deterministic	sql_data_access	security_type	definer	sql_mode	client_collation_id	connection_collation_id	schema_collation_id	created	last_altered	comment	options	external_language
select hello("wx2");
ERROR 42000: FUNCTION db_a.hello does not exist
drop database db_a;
clear recycle_bin;
Warnings:
Warning	4193	No corresponding record was found in recycle_bin.
create database db_a;
use db_a;
create table t1 (a int);
insert into t1 values (1);
create procedure p1()
begin
select * from t1;
end//
call p1();
a
1
drop database db_a;
create database db_a;
use db_a;
call p1();
ERROR 42000: PROCEDURE db_a.p1 does not exist
restore database db_a from RECYCLE_BIN;
use db_a;
call p1();
ERROR 42000: PROCEDURE db_a.p1 does not exist
use db_a;
call p1();
ERROR 42000: PROCEDURE db_a.p1 does not exist
drop database db_a;
clear recycle_bin;
#
# case6. Test drop table if exists
#
CREATE DATABASE test_recycle_bin;
use test_recycle_bin;
create table aa(a int);
create table bb(a int);
drop table if exists aa, bb, cc, dd;
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
show tables from __txsql_recycle_bin__;
Tables_in___txsql_recycle_bin__
#
#
show recycle_bin;
db	table	recycle_table	drop_time	purge_time
test_recycle_bin	aa	#	#	#
test_recycle_bin	bb	#	#	#
drop database test_recycle_bin;
clear recycle_bin;
#
# Cleanup 
# 
set global txsql_recycle_bin_enabled=0;
set global txsql_recycle_bin_enabled=0;
[connection master]
include/rpl_end.inc

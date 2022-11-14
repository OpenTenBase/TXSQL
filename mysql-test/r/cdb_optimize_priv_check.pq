create database db1;
use db1;
create table t1(id int, c1 int);
create table t2(id int, c1 int);
create table t3(id int);
insert into t1 values(1,1),(2,2),(3,3);
insert into t2 values(1,1),(2,2),(3,3);
create user 'test'@'localhost' identified with mysql_native_password by 'test';
grant select on db1.t1 to 'test'@'localhost';
grant select on db1.t2 to 'test'@'localhost';
grant SESSION_VARIABLES_ADMIN on *.* to 'test'@'localhost';
flush privileges;
set debug = '+d, txsql_print_privilege_check';
use db1;
prepare stmt1 from 'select c1 from t1 where id=1';
prepare stmt2 from 'select c1 from t2 where id=2';
include/assert_grep.inc [check if there are some messages for privileges checking]
execute stmt1;
c1
1
cdb_optimize_ps_priv_check = off, we should check privileges
include/assert_grep.inc [check if there are some messages for privileges checking]
set global cdb_optimize_ps_priv_check = ON;
Warnings:
Warning	1287	'@@cdb_optimize_ps_priv_check' is deprecated and will be removed in a future release. Please use txsql_simplify_priv_check instead.
execute stmt1;
c1
1
cdb_optimize_ps_priv_check = ON, we should check privileges because the privilege version of stmt1 is 0
include/assert_grep.inc [check if there are some messages for privileges checking]
execute stmt1;
c1
1
cdb_optimize_ps_priv_check = ON, no checking for stmt1
include/assert_grep.inc [check if there are some messages for privileges checking]
execute stmt2;
c1
2
cdb_optimize_ps_priv_check = ON, we should check privileges because the privilege version of stmt2 is 0
include/assert_grep.inc [check if there are some messages for privileges checking]
execute stmt2;
c1
2
cdb_optimize_ps_priv_check = ON, no checking for stmt2
include/assert_grep.inc [check if there are some messages for privileges checking]
grant select on db1.t3 to 'test'@'localhost';
flush privileges;
execute stmt1;
c1
1
cdb_optimize_ps_priv_check = ON, check privileges for stmt1 because version mismatch
include/assert_grep.inc [check if there are some messages for privileges checking]
execute stmt2;
c1
2
cdb_optimize_ps_priv_check = ON, check privileges for stmt2 because version mismatch
include/assert_grep.inc [check if there are some messages for privileges checking]
execute stmt1;
c1
1
cdb_optimize_ps_priv_check = ON, no checking for stmt1
include/assert_grep.inc [check if there are some messages for privileges checking]
execute stmt2;
c1
2
cdb_optimize_ps_priv_check = ON, no checking for stmt2
include/assert_grep.inc [check if there are some messages for privileges checking]
revoke select on db1.t1 from 'test'@'localhost';
flush privileges;
execute stmt1;
ERROR 42000: SELECT command denied to user 'test'@'localhost' for table 't1'
cdb_optimize_ps_priv_check = ON, check privileges for stmt1 because version mismatch
include/assert_grep.inc [check if there are some messages for privileges checking]
execute stmt2;
c1
2
cdb_optimize_ps_priv_check = ON, check privileges for stmt2 because version mismatch
include/assert_grep.inc [check if there are some messages for privileges checking]
set debug = '-d, txsql_print_privilege_check';
deallocate prepare stmt1;
deallocate prepare stmt2;
set global cdb_optimize_ps_priv_check = OFF;
Warnings:
Warning	1287	'@@cdb_optimize_ps_priv_check' is deprecated and will be removed in a future release. Please use txsql_simplify_priv_check instead.
drop user 'test'@'localhost';
drop database if exists db1;

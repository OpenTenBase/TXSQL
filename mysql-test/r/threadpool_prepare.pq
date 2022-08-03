# Infrastructure for Threadpool Prepare Memory management
show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,GROUP_REPLICATION_ADMIN,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PERSIST_RO_VARIABLES_ADMIN,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ''@'' TO 'root'@'localhost' WITH GRANT OPTION
select user();
user()
tencentroot@localhost
create database test1;
use test1;
create table t1(a int, b int, c int, unique key idx1(a));
create table t5(a double, b int, primary key (a));
create table t6(a char(32), b int, unique key idx6(a));
create table t9(a datetime, b varchar(32), unique key idx9(a));
create table t10(a datetime, b double, c varchar(32), unique key idx10(a,b));
create table t11(a int primary key, b int, c int, key i1(b));
create user 'user1'@'%';
grant all on test1.* to 'user1'@'%';
insert into t1 values(1,2,3);
insert into t1 values(2,3,4);
insert into t1 values(3,2,3);
insert into t1 values(4,3,4);
insert into t1 values(5,2,3);
insert into t1 values(6,3,4);
insert into t5 values(1.1, 1);
insert into t5 values(10.1, 11);
insert into t6 values('tom', 23);
insert into t6 values('mike', 21);
insert into t9 values('2017-07-26 00:38:14', 'tom');
insert into t9 values('2017-07-26 00:38:19', 'mike');
insert into t11 values(1,2,3);
insert into t11 values(2,3,4);
insert into t11 values(3,4,5);
insert into t11 values(4,4,6);
set global cdb_plan_cache = on;
set global cdb_plan_cache_stats = on;
set global cdb_threadpool_prepare_enabled = on;
set session sql_mode='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';
prepare stmt0 from "select b / ? from t1 where a = ?";
set @val1 = 0;
set @val2 = 2;
execute stmt0 using @val1, @val2;
b / ?
NULL
Warnings:
Warning	1365	Division by 0
set @val1 = 1;
set @val2 = 2;
execute stmt0 using @val1, @val2;
b / ?
3.0000
set @val1 = 0;
set @val2 = 2;
execute stmt0 using @val1, @val2;
b / ?
NULL
Warnings:
Warning	1365	Division by 0
set @val1 = '0';
set @val2 = 1;
execute stmt0 using @val1, @val2;
b / ?
NULL
Warnings:
Warning	1365	Division by 0
show cdb_plan_cache_stat;
sql	mode	hit
select b / ? from t1 where a = ?	PREPARE	3
deallocate prepare stmt0;
set session sql_mode='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,NO_ENGINE_SUBSTITUTION';
Warnings:
Warning	3135	'NO_ZERO_DATE', 'NO_ZERO_IN_DATE' and 'ERROR_FOR_DIVISION_BY_ZERO' sql modes should be used with strict mode. They will be merged with strict mode in a future release.
prepare stmt0 from "select b / ? from t1 where a = ?";
set @val1 = 1;
set @val2 = 2;
execute stmt0 using @val1, @val2;
b / ?
3.0000
set @val1 = 0;
set @val2 = 2;
execute stmt0 using @val1, @val2;
b / ?
NULL
set @val1 = '1';
set @val2 = '2';
execute stmt0 using @val1, @val2;
b / ?
3.0000
set @val1 = 1.32e-1;
set @val2 = 2;
execute stmt0 using @val1, @val2;
b / ?
22.727272727272727
set @val1 = 1.232;
set @val2 = 1;
execute stmt0 using @val1, @val2;
b / ?
1.6233766233766234
set @val1 = 1.1111111111111111111111;
set @val2 = 1;
execute stmt0 using @val1, @val2;
b / ?
1.7999999999999998
show cdb_plan_cache_stat;
sql	mode	hit
select b / ? from t1 where a = ?	PREPARE	8
deallocate prepare stmt0;
prepare stmt1 from "select b from t1 where a = ?";
set @value = 3;
execute stmt1 using @value;
b
2
show cdb_plan_cache_stat;
sql	mode	hit
select b from t1 where a = ?	PREPARE	0
select b / ? from t1 where a = ?	PREPARE	8
set @value = 2;
execute stmt1 using @value;
b
3
set @value = 3.0;
execute stmt1 using @value;
b
2
set @value = '2.0';
execute stmt1 using @value;
b
3
show cdb_plan_cache_stat;
sql	mode	hit
select b from t1 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
deallocate prepare stmt1;
prepare stmt2 from "select b + ? from t1 where a = ?";
set @value1 = 9999;
set @value2 = 2;
execute stmt2 using @value1,@value2;
b + ?
10002
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t1 where a = ?	PREPARE	0
select b from t1 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
set @value1 = 888;
execute stmt2 using @value1,@value2;
b + ?
891
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t1 where a = ?	PREPARE	1
select b from t1 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
set @value2 = 3;
execute stmt2 using @value1,@value2;
b + ?
890
set @value1 = 9.99e-3;
set @value2 = 3.0;
execute stmt2 using @value1,@value2;
b + ?
2.00999
set @value1 = '0.00009';
set @value2 = '2.0';
execute stmt2 using @value1,@value2;
b + ?
3.00009
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t1 where a = ?	PREPARE	2
select b from t1 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
deallocate prepare stmt2;
prepare stmt8 from "select b from t1 where a = ?";
set @value1 = null;
execute stmt8 using @value1;
b
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t1 where a = ?	PREPARE	2
select b from t1 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
set @value2 = 2;
execute stmt8 using @value2;
b
3
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t1 where a = ?	PREPARE	2
select b from t1 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
set @value3 = 3;
execute stmt8 using @value3;
b
2
set @value3 = null;
execute stmt8 using @value3;
b
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t1 where a = ?	PREPARE	2
select b from t1 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
deallocate prepare stmt8;
prepare stmt9 from "select * from t5 where a = ?";
set @value1 = 1.1;
execute stmt9 using @value1;
a	b
1.1	1
set @value1 = 10.1;
execute stmt9 using @value1;
a	b
10.1	11
set @value1 = '10.1';
execute stmt9 using @value1;
a	b
10.1	11
set @value1 = 1010e-2;
execute stmt9 using @value1;
a	b
10.1	11
deallocate prepare stmt9;
prepare stmt10 from 'select b, a from t6 where a = ?';
set @value = 'tom';
execute stmt10 using @value;
b	a
23	tom
set @value = 'mike';
execute stmt10 using @value;
b	a
21	mike
deallocate prepare stmt10;
prepare stmt13 from 'select * from t9 where a = ?';
set @value = '2017-07-26 00:38:14';
execute stmt13 using @value;
a	b
2017-07-26 00:38:14	tom
set @value = '2017-07-26 00:38:19';
execute stmt13 using @value;
a	b
2017-07-26 00:38:19	mike
deallocate prepare stmt13;
prepare stmt15 from 'select * from t9 where ? = a';
set @value = '2017-07-26 00:38:14';
execute stmt15 using @value;
a	b
2017-07-26 00:38:14	tom
set @value = '2017-07-26 00:38:19';
execute stmt15 using @value;
a	b
2017-07-26 00:38:19	mike
deallocate prepare stmt15;
prepare stmt21 from "select * from t1 where a = ? and false";
set @value = 1;
execute stmt21 using @value;
a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select * from t9 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
select b from t1 where a = ?	PREPARE	1
select b + ? from t1 where a = ?	PREPARE	2
select * from t9 where ? = a	PREPARE	1
select b, a from t6 where a = ?	PREPARE	1
select * from t5 where a = ?	PREPARE	3
deallocate prepare stmt21;
prepare stmt22 from "select * from t1 where a = ? and ?";
set @value = 1;
execute stmt22 using @value,@value;
a	b	c
1	2	3
show cdb_plan_cache_stat;
sql	mode	hit
select * from t9 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
select b from t1 where a = ?	PREPARE	1
select b + ? from t1 where a = ?	PREPARE	2
select * from t9 where ? = a	PREPARE	1
select b, a from t6 where a = ?	PREPARE	1
select * from t5 where a = ?	PREPARE	3
deallocate prepare stmt22;
prepare stmt23 from "select * from t1 where a = ? and true";
set @value = 1;
execute stmt23 using @value;
a	b	c
1	2	3
show cdb_plan_cache_stat;
sql	mode	hit
select * from t9 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
select b from t1 where a = ?	PREPARE	1
select b + ? from t1 where a = ?	PREPARE	2
select * from t9 where ? = a	PREPARE	1
select b, a from t6 where a = ?	PREPARE	1
select * from t5 where a = ?	PREPARE	3
deallocate prepare stmt23;
prepare stmt24 from "select * from t1 where a = ? and b = ?";
set @value = 1;
set @value1 = 2;
execute stmt24 using @value,@value1;
a	b	c
1	2	3
show cdb_plan_cache_stat;
sql	mode	hit
select * from t9 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
select b from t1 where a = ?	PREPARE	1
select b + ? from t1 where a = ?	PREPARE	2
select * from t9 where ? = a	PREPARE	1
select b, a from t6 where a = ?	PREPARE	1
select * from t5 where a = ?	PREPARE	3
deallocate prepare stmt24;
prepare stmt25 from "select * from t1 where a = ? or b = ?";
set @value1 = 1;
set @value2 = 2;
execute stmt25 using @value1,@value2;
a	b	c
1	2	3
3	2	3
5	2	3
show cdb_plan_cache_stat;
sql	mode	hit
select * from t9 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
select b from t1 where a = ?	PREPARE	1
select b + ? from t1 where a = ?	PREPARE	2
select * from t9 where ? = a	PREPARE	1
select b, a from t6 where a = ?	PREPARE	1
select * from t5 where a = ?	PREPARE	3
deallocate prepare stmt25;
drop table if exists t_o1;
Warnings:
Note	1051	Unknown table 'test1.t_o1'
create table t_o1 (t1 char(3) primary key) charset utf8mb4;
insert into t_o1 values("ABC");
insert into t_o1 values("ABA");
insert into t_o1 values("AB%");
prepare stmt41 from "select * from t_o1 where t1 = ?";
set @value = 'ABC';
execute stmt41 using @value;
t1
ABC
set @value = 'ABCD';
execute stmt41 using @value;
t1
show cdb_plan_cache_stat;
sql	mode	hit
select * from t9 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
select b from t1 where a = ?	PREPARE	1
select * from t_o1 where t1 = ?	PREPARE	1
select b + ? from t1 where a = ?	PREPARE	2
select * from t9 where ? = a	PREPARE	1
select b, a from t6 where a = ?	PREPARE	1
select * from t5 where a = ?	PREPARE	3
drop table t_o1;
show cdb_plan_cache_stat;
sql	mode	hit
show cdb_plan_cache_stat;
sql	mode	hit
use test1;
set global cdb_plan_cache = on;
set global cdb_plan_cache_stats = on;
prepare stmt61 from "select * from t11 where b = ? limit ?";
set @val1 = 4;
set @val2 = 1;
execute stmt61 using @val1,@val2;
a	b	c
3	4	5
execute stmt61 using @val1,@val2;
a	b	c
3	4	5
show cdb_plan_cache_stat;
sql	mode	hit
prepare stmt62 from "select * from t11 where ? = b limit ?";
set @val1 = 4;
set @val2 = 1;
execute stmt62 using @val1,@val2;
a	b	c
3	4	5
execute stmt62 using @val1,@val2;
a	b	c
3	4	5
show cdb_plan_cache_stat;
sql	mode	hit
set global cdb_plan_cache = off;
set global cdb_plan_cache_stats = off;
set global cdb_plan_cache = on;
set global cdb_plan_cache_stats = on;
create database test9;
use test9;
create table t91(a int primary key, b int, c int);
create table t92(a int primary key, b int, c int, key i3(b));
create table t93(a int primary key, b int, c int, key i4(b,c));
insert into t91 values(1,2,3);
insert into t91 values(2,3,4);
insert into t91 values(3,4,5);
insert into t92 values(1,2,3);
insert into t92 values(2,3,4);
insert into t92 values(3,4,5);
insert into t93 values(1,2,3);
insert into t93 values(2,3,4);
insert into t93 values(3,4,5);
prepare stmt91 from "select b + ? from t91 where a = ?";
set @val1 = 1;
set @val2 = 2;
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt91 using @val1, @val2;
b + ?
4
set @val1 = 1.2e-2;
set @val2 = 2.2e+2;
execute stmt91 using @val1, @val2;
b + ?
set @val1 = 1.2e-3;
set @val2 = 2;
execute stmt91 using @val1, @val2;
b + ?
3.0012
prepare stmt93 from "select * from t92 where ? = b limit ?";
set @val1 = 2;
set @val2 = 1;
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
prepare stmt94 from "select * from t93, t91 where t93.b = ? and ? = t93.c and t93.a = t91.a";
set @val1 = 2;
set @val2 = 1;
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	1
drop database test9;
execute stmt91 using @val1, @val2;
ERROR 42000: Unknown database 'test9'
execute stmt93 using @val1, @val2;
ERROR 42000: Unknown database 'test9'
execute stmt94 using @val1, @val2;
ERROR 42000: Unknown database 'test9'
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	1
create database test9;
use test9;
create table t91(a int primary key, b int, c int);
create table t92(a int primary key, b int, c int, key i3(b));
create table t93(a int primary key, b int, c int, key i4(b,c));
insert into t91 values(1,2,3);
insert into t91 values(2,3,4);
insert into t91 values(3,4,5);
insert into t92 values(1,2,3);
insert into t92 values(2,3,4);
insert into t92 values(3,4,5);
insert into t93 values(1,2,3);
insert into t93 values(2,3,4);
insert into t93 values(3,4,5);
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	1
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	2
use test9;
alter table t91 add column d double not null;
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	2
execute stmt91 using @val1, @val2;
b + ?
4
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	2
drop database test9;
create database test9;
use test9;
create table t91(a int primary key, b int, c int);
create table t92(a int primary key, b int, c int, key i3(b));
create table t93(a int primary key, b int, c int, key i4(b,c));
insert into t91 values(1,2,3);
insert into t91 values(2,3,4);
insert into t91 values(3,4,5);
insert into t92 values(1,2,3);
insert into t92 values(2,3,4);
insert into t92 values(3,4,5);
insert into t93 values(1,2,3);
insert into t93 values(2,3,4);
insert into t93 values(3,4,5);
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	2
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	3
use test9;
drop table t91;
drop table t92;
drop table t93;
execute stmt91 using @val1, @val2;
ERROR 42S02: Table 'test9.t91' doesn't exist
execute stmt93 using @val1, @val2;
ERROR 42S02: Table 'test9.t92' doesn't exist
execute stmt94 using @val1, @val2;
ERROR 42S02: Table 'test9.t93' doesn't exist
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	3
use test9;
create table t91(a int primary key, b int, c int);
create table t92(a int primary key, b int, c int, key i3(b));
create table t93(a int primary key, b int, c int, key i4(b,c));
insert into t91 values(1,2,3);
insert into t91 values(2,3,4);
insert into t91 values(3,4,5);
insert into t92 values(1,2,3);
insert into t92 values(2,3,4);
insert into t92 values(3,4,5);
insert into t93 values(1,2,3);
insert into t93 values(2,3,4);
insert into t93 values(3,4,5);
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	3
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	4
use test9;
alter table t91 drop column a;
alter table t92 drop column b;
alter table t93 drop column c;
execute stmt91 using @val1, @val2;
ERROR 42S22: Unknown column 'a' in 'where clause'
execute stmt93 using @val1, @val2;
ERROR 42S22: Unknown column 'b' in 'where clause'
execute stmt94 using @val1, @val2;
ERROR 42S22: Unknown column 't93.c' in 'where clause'
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	4
alter table t91 add column a int;
alter table t92 add column b int;
alter table t93 add column c int;
execute stmt91 using @val1, @val2;
b + ?
execute stmt93 using @val1, @val2;
a	c	b
execute stmt94 using @val1, @val2;
a	b	c	b	c	a
execute stmt91 using @val1, @val2;
b + ?
execute stmt93 using @val1, @val2;
a	c	b
execute stmt94 using @val1, @val2;
a	b	c	b	c	a
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	4
use test9;
drop table t91;
drop table t92;
drop table t93;
create table t91(a int primary key, b int, c int);
create table t92(a int primary key, b int, c int, key i3(b));
create table t93(a int primary key, b int, c int, key i4(b,c));
insert into t91 values(1,2,3);
insert into t91 values(2,3,4);
insert into t91 values(3,4,5);
insert into t92 values(1,2,3);
insert into t92 values(2,3,4);
insert into t92 values(3,4,5);
insert into t93 values(1,2,3);
insert into t93 values(2,3,4);
insert into t93 values(3,4,5);
use test9;
alter table t91 drop primary key;
alter table t92 drop index i3;
alter table t93 drop index i4;
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	4
use test9;
alter table t91 add primary key(a);
alter table t92 add index i3(b);
alter table t93 add index i4(b,c);
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	4
execute stmt91 using @val1, @val2;
b + ?
4
execute stmt93 using @val1, @val2;
a	b	c
1	2	3
execute stmt94 using @val1, @val2;
a	b	c	a	b	c
show cdb_plan_cache_stat;
sql	mode	hit
select b + ? from t91 where a = ?	PREPARE	5
drop database test9;
set global cdb_plan_cache = off;
set global cdb_plan_cache_stats = off;
show cdb_plan_cache_stat;
sql	mode	hit
select * from t9 where a = ?	PREPARE	1
select b / ? from t1 where a = ?	PREPARE	8
select b from t1 where a = ?	PREPARE	1
select * from t_o1 where t1 = ?	PREPARE	1
select b + ? from t1 where a = ?	PREPARE	2
select * from t9 where ? = a	PREPARE	1
select b, a from t6 where a = ?	PREPARE	1
select * from t5 where a = ?	PREPARE	3
set global cdb_plan_cache = off;
set global cdb_plan_cache_stats = off;
set global cdb_threadpool_prepare_enabled = off;
drop table t1;
drop table t5;
drop table t6;
drop table t9;
drop table t11;
drop database test1;
drop user 'user1'@'%';

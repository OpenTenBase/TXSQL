create  table t1 (a int primary key, b int mask);
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL,
  `b` int DEFAULT NULL MASK,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
create table t2 (a int, b varchar(100) mask);
alter table t2 alter column a set mask;
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int DEFAULT NULL MASK,
  `b` varchar(100) DEFAULT NULL MASK
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
insert into t2 values (1, "12.1"),(2, "13.1"),(11, "1113.2");
select * from t2;
a	b
1	12.1
11	1113.2
2	13.1
alter table t2 alter column a set unmask;
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int DEFAULT NULL,
  `b` varchar(100) DEFAULT NULL MASK
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
select * from t2;
a	b
1	12.1
11	1113.2
2	13.1
alter table t2 alter column b set unmask;
select * from t2;
a	b
1	12.1
11	1113.2
2	13.1
alter table t2 alter column b set mask;
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int DEFAULT NULL,
  `b` varchar(100) DEFAULT NULL MASK
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
create user sb;
grant read_mask, select on *.* to 'sb'@'%';
show grants for 'sb';
Grants for sb@%
GRANT SELECT ON *.* TO `sb`@`%`
GRANT READ_MASK ON *.* TO `sb`@`%`
select * from t2;
a	b
1	12.1
11	1113.2
2	13.1
revoke read_mask on *.* from 'sb'@'%';
create table t3 (a bigint mask);
insert into t3 values (1), (2),(3);
create table t4 (a char(20) mask);
insert into t4 values ("abc"), ("12.4"), ("333");
create table t5 (a float mask);
insert into t5 values (12.12), (34.12);
create table t6 (a decimal mask);
insert into t6 values (12.12), (34.123);
Warnings:
Note	1265	Data truncated for column 'a' at row 1
Note	1265	Data truncated for column 'a' at row 2
create table t7(a int, b int, c int as (a + b) mask);
ERROR HY000: Can not create the column c with MASK attribute
create table t7(a int, b int, c int as (a + b));
alter table t7 alter column c set mask;
ERROR HY000: Can not create the column c with MASK attribute
select * from t2;
a	b
1	***
11	***
2	***
select * from t3;
a
0
0
0
select * from t4;
a
***
***
***
select * from t5;
a
0
0
select * from t6;
a
0
0
Warning	1366	Incorrect DECIMAL value: '0' for column '' at row -1
Warning	1366	Incorrect DECIMAL value: '0' for column '' at row -1
Warnings:
select a from t3  where a = 1;
ERROR HY000: Can not use masked column a as where clause
select * from t3 where a + 1 = 2;
ERROR HY000: Can not use masked column a as where clause
select a from t3;
a
0
0
0
select a + 1 from t3;
a + 1
1
1
1
drop user sb;
drop table t1;
drop table t2;
drop table t3;
drop table t4;
drop table t5;
drop table t6;
drop table t7;

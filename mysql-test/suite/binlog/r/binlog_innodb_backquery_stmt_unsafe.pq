CALL mtr.add_suppression('Unsafe statement written to the binary log using statement format');
set @saved_innodb_backquery_enable = @@global.innodb_backquery_enable;
set global innodb_backquery_enable = ON;
create table t1(id int primary key, c1 varchar(20)) engine=innodb;
insert into t1 values(1,'aaa'),(2,'bbb'),(3,'ccc'),(4,'ddd'),(5,'eee');
update t1 set c1='abcdefg' where id=4;
select * from t1;
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
5	eee
select * from t1 as of timestamp 'time1'
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
create table t2 select * from t1;
select * from t2;
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
5	eee
create table t3 select * from t1 as of timestamp 'time1'
Warnings:
Note	1592	Unsafe statement written to the binary log using statement format since BINLOG_FORMAT = STATEMENT. Statement is unsafe because it is used inside a backquery, which may break replication on slaves.
select * from t3;
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
create table t4 like t1;
create table t5 like t1;
insert into t4 select * from t1;
select * from t4;
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
5	eee
insert into t5 select * from t1 as of timestamp 'time1'
Warnings:
Note	1592	Unsafe statement written to the binary log using statement format since BINLOG_FORMAT = STATEMENT. Statement is unsafe because it is used inside a backquery, which may break replication on slaves.
select * from t5;
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
Ensure that the warning message get logged to the error log file
include/assert_grep.inc [SBR is unsafe for backquery, warning message is printed in error log]
set global innodb_backquery_enable=@saved_innodb_backquery_enable;
drop table if exists t1;
drop table if exists t2;
drop table if exists t3;
drop table if exists t4;
drop table if exists t5;

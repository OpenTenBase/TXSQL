drop table if exists t1;
set binlog_format=row;
set session transaction isolation level read committed;
set binlog_format=row;
set session transaction isolation level read committed;
create table t1 (a int primary key, b int);
insert into t1 values (1,2),(2,3),(3,4);
begin;
update t1 set b = 5 where b = 3;
begin;
set session innodb_lock_wait_timeout = 2;
select * from t1 where b = 5 for update;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
commit;
commit;
set session select_update_enable_semi_read = 1;
begin;
update t1 set b = 10 where b = 5;
begin;
select * from t1;
a	b
1	2
2	5
3	4
select * from t1 where b = 10 for update;
a	b
select * from t1 where b <= 5 for update;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
commit;
commit;
drop table t1;

include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
call mtr.add_suppression("Unsafe statement written to the binary log using statement format");
SET GLOBAL allow_rollback_last_query = 1;
create table t1 (a int primary key, b int);
rollback last query;
ERROR HY000: Can not rollback last query
begin;
insert into t1 values (1,2);
insert into t1 values (2,3);
rollback last query;
commit;
select * from t1;
a	b
1	2
select * from t1;
a	b
1	2
xa begin '123';
insert into t1 values (2,3);
rollback last query;
insert into t1 values (3,4);
xa end '123';
xa prepare '123';
rollback last query;
ERROR HY000: Can not rollback last query
xa commit '123';
select * from t1;
a	b
1	2
3	4
select * from t1;
a	b
1	2
3	4
SET GLOBAL allow_rollback_last_query = 0;
begin;
insert into t1 values (4,5);
insert into t1 values (5,6);
rollback last query;
ERROR HY000: Can not rollback last query
commit;
select * from t1;
a	b
1	2
3	4
4	5
5	6
select * from t1;
a	b
1	2
3	4
4	5
5	6
CREATE PROCEDURE proc1 (IN pk INT)
BEGIN
INSERT INTO t1 VALUES (pk, 8);
INSERT INTO t1 VALUES (pk + 1, 9);
END
//
SET GLOBAL allow_rollback_last_query = 1;
begin;
insert into t1 values (6,7);
call proc1(7);
rollback last query;
commit;
select * from t1;
a	b
1	2
3	4
4	5
5	6
6	7
select * from t1;
a	b
1	2
3	4
4	5
5	6
6	7
DROP PROCEDURE proc1;
drop table t1;
SET GLOBAL allow_rollback_last_query = default;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

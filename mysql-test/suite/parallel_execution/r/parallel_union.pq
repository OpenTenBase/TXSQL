# Infrastructure for test parallel union/union-all execution
show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,READ_MASK,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ``@`` TO `root`@`localhost` WITH GRANT OPTION
select user();
user()
root@localhost
create database test1;
create user 'user1'@'%';
grant all on test1.* to 'user1'@'%';
create table t1(a int, b int, c int, d int);
insert into t1 values(1,2,3,4);
insert into t1 values(1,1,1,1);
insert into t1 values(2,3,4,5);
insert into t1 values(2,2,2,2);
insert into t1 values(3,4,5,6);
insert into t1 values(4,5,6,7);
insert into t1 values(5,6,7,8);
create table t2(a int, b int, c int, d int);
insert into t2 values(1,1,1,1);
insert into t2 values(2,2,2,2);
insert into t2 values(3,3,3,3);
insert into t2 values(4,4,4,4);
insert into t2 values(5,5,5,5);
create table t3(a int, b int, c int, d int, primary key(a), key idx(b));
insert into t3 values(1,1,1,1);
insert into t3 values(2,2,2,2);
insert into t3 values(3,3,3,3);
insert into t3 values(4,4,4,4);
insert into t3 values(5,5,5,5);
insert into t3 values(6,6,6,6);
insert into t3 values(7,7,7,7);
insert into t3 values(8,8,8,8);
insert into t3 values(9,9,9,9);
insert into t3 values(10,10,10,10);
create table t4(a int, b int, c int, d int);
insert into t4 values(100,100,100,100);
# --include save_and_set_px.inc
set txsql_parallel_table_record_threshold = 2;
select * from t1 union select * from t2;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree select * from t1 union select * from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.22..2.65 rows=12)
            -> Union materialize with deduplication  (cost=3.12..5.55 rows=12)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=0.75 rows=5)

explain select * from t1 union select * from t2;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Parallel scan (4 workers)
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Receive from (<sender3>)
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2`
select * from t1 union all select * from t2;
a	b	c	d
1	1	1	1
1	1	1	1
1	2	3	4
2	2	2	2
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree select * from t1 union all select * from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.75 rows=5)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=0.75 rows=5)

explain select * from t1 union all select * from t2;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Parallel scan (4 workers)
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Receive from (<sender3>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union all /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2`
select * from t1 union select * from t2 where a > 1;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree select * from t1 union select * from t2 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.30..2.60 rows=9)
            -> Union materialize with deduplication  (cost=2.87..5.17 rows=9)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)

explain select * from t1 union select * from t2 where a > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	33.33	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender3>)
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` where (`test`.`t2`.`a` > 1)
select * from t1 union all select * from t2 where a > 1;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree select * from t1 union all select * from t2 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.75 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)

explain select * from t1 union all select * from t2 where a > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	33.33	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender3>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union all /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` where (`test`.`t2`.`a` > 1)
select * from t1 where a > 1 union select * from t2 where a > 1;
a	b	c	d
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree select * from t1 where a > 1 union select * from t2 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.63..2.54 rows=4)
            -> Union materialize with deduplication  (cost=2.73..4.64 rows=4)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)

explain select * from t1 where a > 1 union select * from t2 where a > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	33.33	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender3>)
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` where (`test`.`t2`.`a` > 1)
select * from t1 where a > 1 union all select * from t2 where a > 1;
a	b	c	d
2	2	2	2
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree select * from t1 where a > 1 union all select * from t2 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.75 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)

explain select * from t1 where a > 1 union all select * from t2 where a > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	33.33	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender3>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union all /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` where (`test`.`t2`.`a` > 1)
select * from t1 where a > 1 union select * from t3 where a between 1 and 9;
a	b	c	d
1	1	1	1
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree select * from t1 where a > 1 union select * from t3 where a between 1 and 9;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.23..2.64 rows=11)
            -> Union materialize with deduplication  (cost=4.38..6.78 rows=11)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t3.a between 1 and 9)  (cost=2.06 rows=9)
                            -> Parallel index range scan on t3 using PRIMARY over (1 <= a <= 9)  (cost=2.06 rows=9)

explain select * from t1 where a > 1 union select * from t3 where a between 1 and 9;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t3	NULL	range	PRIMARY	PRIMARY	4	NULL	9	100.00	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	9	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	9	100.00	Receive from (<sender3>)
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union /* select#2 */ select `test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t3`.`d` AS `d` from `test`.`t3` where (`test`.`t3`.`a` between 1 and 9)
select * from t1 where a > 1 union all select * from t3 where a between 1 and 9;
a	b	c	d
1	1	1	1
2	2	2	2
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree select * from t1 where a > 1 union all select * from t3 where a between 1 and 9;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=2.06 rows=9)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t3.a between 1 and 9)  (cost=2.06 rows=9)
                            -> Parallel index range scan on t3 using PRIMARY over (1 <= a <= 9)  (cost=2.06 rows=9)

explain select * from t1 where a > 1 union all select * from t3 where a between 1 and 9;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t3	NULL	range	PRIMARY	PRIMARY	4	NULL	9	100.00	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	9	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	9	100.00	Receive from (<sender3>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union all /* select#2 */ select `test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t3`.`d` AS `d` from `test`.`t3` where (`test`.`t3`.`a` between 1 and 9)
select * from t1 where a >=1 union select * from t1 where b > 1;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
explain format=tree select * from t1 where a >=1 union select * from t1 where b > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.55..2.55 rows=5)
            -> Union materialize with deduplication  (cost=2.91..4.92 rows=5)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a >= 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t1.b > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)

explain select * from t1 where a >=1 union select * from t1 where b > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender3>)
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` >= 1) union /* select#2 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`b` > 1)
select * from t1 where a >=1 union all select * from t1 where b > 1;
a	b	c	d
1	1	1	1
1	2	3	4
1	2	3	4
2	2	2	2
2	2	2	2
2	3	4	5
2	3	4	5
3	4	5	6
3	4	5	6
4	5	6	7
4	5	6	7
5	6	7	8
5	6	7	8
explain format=tree select * from t1 where a >=1 union all select * from t1 where b > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a >= 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t1.b > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)

explain select * from t1 where a >=1 union all select * from t1 where b > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender3>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` >= 1) union all /* select#2 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`b` > 1)
select * from t1 where a > 1 union select * from t2 where a = 1 union select * from t3;
a	b	c	d
1	1	1	1
10	10	10	10
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree select * from t1 where a > 1 union select * from t2 where a = 1 union select * from t3;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.20..2.66 rows=13)
            -> Union materialize with deduplication  (cost=4.48..6.95 rows=13)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a = 1)  (cost=0.75 rows=1)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Parallel table scan on t3  (cost=1.25 rows=10)

explain select * from t1 where a > 1 union select * from t2 where a = 1 union select * from t3;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	20.00	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender3>)
3	UNION	t3	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Parallel scan (4 workers)
3	UNION	<sender4>	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Send to (<receiver4>)
3	UNION	<receiver4>	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Receive from (<sender4>)
NULL	UNION RESULT	<union1,2,3>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` where (`test`.`t2`.`a` = 1) union /* select#3 */ select `test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t3`.`d` AS `d` from `test`.`t3`
select * from t1 where a > 1 union all select * from t2 where a = 1 union all select * from t3;
a	b	c	d
1	1	1	1
1	1	1	1
10	10	10	10
2	2	2	2
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree select * from t1 where a > 1 union all select * from t2 where a = 1 union all select * from t3;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.75 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a = 1)  (cost=0.75 rows=1)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
            -> Stream results  (cost=1.25 rows=10)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Parallel table scan on t3  (cost=1.25 rows=10)

explain select * from t1 where a > 1 union all select * from t2 where a = 1 union all select * from t3;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	20.00	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender3>)
3	UNION	t3	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Parallel scan (4 workers)
3	UNION	<sender4>	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Send to (<receiver4>)
3	UNION	<receiver4>	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Receive from (<sender4>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union all /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` where (`test`.`t2`.`a` = 1) union all /* select#3 */ select `test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t3`.`d` AS `d` from `test`.`t3`
select * from t1 where a > 1 union select * from t2 where c > 3 union select * from t1 where a > 1;
a	b	c	d
2	2	2	2
2	3	4	5
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree select * from t1 where a > 1 union select * from t2 where c > 3 union select * from t1 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.41..2.58 rows=6)
            -> Union materialize with deduplication  (cost=3.69..5.86 rows=6)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.c > 3)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)

explain select * from t1 where a > 1 union select * from t2 where c > 3 union select * from t1 where a > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	33.33	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender3>)
3	UNION	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
3	UNION	<sender4>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver4>)
3	UNION	<receiver4>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender4>)
NULL	UNION RESULT	<union1,2,3>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` where (`test`.`t2`.`c` > 3) union /* select#3 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1)
select * from t1 where a > 1 union all select * from t2 where c > 3 union all select * from t1 where a > 1;
a	b	c	d
2	2	2	2
2	2	2	2
2	3	4	5
2	3	4	5
3	4	5	6
3	4	5	6
4	4	4	4
4	5	6	7
4	5	6	7
5	5	5	5
5	6	7	8
5	6	7	8
explain format=tree select * from t1 where a > 1 union all select * from t2 where c > 3 union all select * from t1 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.75 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.c > 3)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)

explain select * from t1 where a > 1 union all select * from t2 where c > 3 union all select * from t1 where a > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	33.33	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender3>)
3	UNION	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
3	UNION	<sender4>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver4>)
3	UNION	<receiver4>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender4>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union all /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` where (`test`.`t2`.`c` > 3) union all /* select#3 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1)
select * from t1 where a > 1 union select * from t2 where c > 1 union select * from t1 where a > 1 union select * from t4;
a	b	c	d
100	100	100	100
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree select * from t1 where a > 1 union select * from t2 where c > 1 union select * from t1 where a > 1 union select * from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.35..2.59 rows=7)
            -> Union materialize with deduplication  (cost=4.09..6.32 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.c > 1)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)

explain select * from t1 where a > 1 union select * from t2 where c > 1 union select * from t1 where a > 1 union select * from t4;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	33.33	Parallel scan (4 workers); Using where
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender3>)
3	UNION	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
3	UNION	<sender4>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver4>)
3	UNION	<receiver4>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender4>)
4	UNION	t4	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Parallel scan (1 workers)
4	UNION	<sender5>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver5>)
4	UNION	<receiver5>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender5>)
NULL	UNION RESULT	<union1,2,3,4>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` where (`test`.`t2`.`c` > 1) union /* select#3 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union /* select#4 */ select `test`.`t4`.`a` AS `a`,`test`.`t4`.`b` AS `b`,`test`.`t4`.`c` AS `c`,`test`.`t4`.`d` AS `d` from `test`.`t4`
select * from t1 where a > 1 union all select * from t2 where c > 1 union all select * from t1 where a > 1 union all select * from t4;
a	b	c	d
100	100	100	100
2	2	2	2
2	2	2	2
2	2	2	2
2	3	4	5
2	3	4	5
3	3	3	3
3	4	5	6
3	4	5	6
4	4	4	4
4	5	6	7
4	5	6	7
5	5	5	5
5	6	7	8
5	6	7	8
explain format=tree select * from t1 where a > 1 union all select * from t2 where c > 1 union all select * from t1 where a > 1 union all select * from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.75 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.c > 1)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.35 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)

select * from t1 where a > 1 union select * from t4 union select * from t1 where b > 1 union select * from t4;
a	b	c	d
1	2	3	4
100	100	100	100
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
explain format=tree select * from t1 where a > 1 union select * from t4 union select * from t1 where b > 1 union select * from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.39..2.58 rows=7)
            -> Union materialize with deduplication  (cost=3.65..5.84 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.b > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)

select * from t1 where a > 1 union all select * from t4 union all select * from t1 where b > 1 union all select * from t4;
a	b	c	d
1	2	3	4
100	100	100	100
100	100	100	100
2	2	2	2
2	2	2	2
2	3	4	5
2	3	4	5
3	4	5	6
3	4	5	6
4	5	6	7
4	5	6	7
5	6	7	8
5	6	7	8
explain format=tree select * from t1 where a > 1 union all select * from t4 union all select * from t1 where b > 1 union all select * from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.35 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.b > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.35 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)

select * from t1 where a > 1 union select * from t4 union select * from t1 union select * from t2 where a > 1 union select * from t3 union select * from t3 where a > 1;
a	b	c	d
1	1	1	1
1	2	3	4
10	10	10	10
100	100	100	100
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree select * from t1 where a > 1 union select * from t4 union select * from t1 union select * from t2 where a > 1 union select * from t3 union select * from t3 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.09..2.88 rows=31)
            -> Union materialize with deduplication  (cost=9.51..12.29 rows=31)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 6; workers: 4)
                        -> Parallel table scan on t3  (cost=1.25 rows=10)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 7; workers: 4)
                        -> Filter: (t3.a > 1)  (cost=2.06 rows=9)
                            -> Parallel index range scan on t3 using PRIMARY over (1 < a)  (cost=2.06 rows=9)

select * from t1 where a > 1 union all select * from t4 union all select * from t1 union all select * from t2 where a > 1 union all select * from t3 union all select * from t3 where a > 1;
a	b	c	d
1	1	1	1
1	1	1	1
1	2	3	4
10	10	10	10
10	10	10	10
100	100	100	100
2	2	2	2
2	2	2	2
2	2	2	2
2	2	2	2
2	2	2	2
2	3	4	5
2	3	4	5
3	3	3	3
3	3	3	3
3	3	3	3
3	4	5	6
3	4	5	6
4	4	4	4
4	4	4	4
4	4	4	4
4	5	6	7
4	5	6	7
5	5	5	5
5	5	5	5
5	5	5	5
5	6	7	8
5	6	7	8
6	6	6	6
6	6	6	6
7	7	7	7
7	7	7	7
8	8	8	8
8	8	8	8
9	9	9	9
9	9	9	9
explain format=tree select * from t1 where a > 1 union all select * from t4 union all select * from t1 union all select * from t2 where a > 1 union all select * from t3 union all select * from t3 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.35 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)
            -> Stream results  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.75 rows=2)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=0.75 rows=2)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
            -> Stream results  (cost=1.25 rows=10)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 6; workers: 4)
                        -> Parallel table scan on t3  (cost=1.25 rows=10)
            -> Stream results  (cost=2.06 rows=9)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 7; workers: 4)
                        -> Filter: (t3.a > 1)  (cost=2.06 rows=9)
                            -> Parallel index range scan on t3 using PRIMARY over (1 < a)  (cost=2.06 rows=9)

(select * from t1 order by b) union (select * from t2 order by c);
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree (select * from t1 order by b) union (select * from t2 order by c);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.22..2.65 rows=12)
            -> Union materialize with deduplication  (cost=3.12..5.55 rows=12)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=0.75 rows=5)

(select * from t1 order by b) union all (select * from t2 order by c);
a	b	c	d
1	1	1	1
1	1	1	1
1	2	3	4
2	2	2	2
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree (select * from t1 order by b) union all (select * from t2 order by c);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.75 rows=5)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=0.75 rows=5)

select a,sum(b) from t1 group by a union select b,sum(c) from t2 group by b;
a	sum(b)
1	1
1	3
2	2
2	5
3	3
3	4
4	4
4	5
5	5
5	6
explain format=tree select a,sum(b) from t1 group by a union select b,sum(c) from t2 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50..2.50 rows=0)
            -> Union materialize with deduplication  (cost=2.50..2.50 rows=0)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=0.75 rows=5)

select a,sum(b) from t1 group by a union all select b,sum(c) from t2 group by b;
a	sum(b)
1	1
1	3
2	2
2	5
3	3
3	4
4	4
4	5
5	5
5	6
explain format=tree select a,sum(b) from t1 group by a union all select b,sum(c) from t2 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=0.75 rows=5)

select a,sum(b) from t1 group by a union select b,sum(c) from t2 group by b union select b,sum(d) from t3 group by b;
a	sum(b)
1	1
1	3
10	10
2	2
2	5
3	3
3	4
4	4
4	5
5	5
5	6
6	6
7	7
8	8
9	9
explain format=tree select a,sum(b) from t1 group by a union select b,sum(c) from t2 group by b union select b,sum(d) from t3 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.26..2.62 rows=10)
            -> Union materialize with deduplication  (cost=3.51..5.88 rows=10)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=0.75 rows=5)
                -> Group aggregate: sum(t3.d)
                    -> PX Receiver (merge sort; slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Group aggregate: sum(t3.d)  (cost=2.25 rows=10)
                                -> Parallel index scan on t3 using idx  (cost=1.25 rows=10)

select a,sum(b) from t1 group by a union all select b,sum(c) from t2 group by b union all select b,sum(d) from t3 group by b;
a	sum(b)
1	1
1	1
1	3
10	10
2	2
2	2
2	5
3	3
3	3
3	4
4	4
4	4
4	5
5	5
5	5
5	6
6	6
7	7
8	8
9	9
explain format=tree select a,sum(b) from t1 group by a union all select b,sum(c) from t2 group by b union all select b,sum(d) from t3 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=0.75 rows=5)
            -> Stream results  (cost=2.25 rows=10)
                -> Group aggregate: sum(t3.d)
                    -> PX Receiver (merge sort; slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Group aggregate: sum(t3.d)  (cost=2.25 rows=10)
                                -> Parallel index scan on t3 using idx  (cost=1.25 rows=10)

select a,sum(b),sum(d) from t1 group by a union select b,sum(b),avg(d) from t2 group by b union select b,sum(d),avg(c) from t3 group by b;
a	sum(b)	sum(d)
1	1	1.0000
1	3	5.0000
10	10	10.0000
2	2	2.0000
2	5	7.0000
3	3	3.0000
3	4	6.0000
4	4	4.0000
4	5	7.0000
5	5	5.0000
5	6	8.0000
6	6	6.0000
7	7	7.0000
8	8	8.0000
9	9	9.0000
explain format=tree select a,sum(b),sum(d) from t1 group by a union select b,sum(b),avg(d) from t2 group by b union select b,sum(d),avg(c) from t3 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.26..2.62 rows=10)
            -> Union materialize with deduplication  (cost=3.51..5.88 rows=10)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=0.75 rows=5)
                -> Group aggregate: sum(t3.d), sum(t3.c), count(t3.c)
                    -> PX Receiver (merge sort; slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Group aggregate: sum(t3.d), sum(t3.c), count(t3.c)  (cost=2.25 rows=10)
                                -> Parallel index scan on t3 using idx  (cost=1.25 rows=10)

select a,sum(b),sum(d) from t1 group by a union all select b,sum(b),avg(d) from t2 group by b union all select b,sum(d),avg(c) from t3 group by b;
a	sum(b)	sum(d)
1	1	1.0000
1	1	1.0000
1	3	5.0000
10	10	10.0000
2	2	2.0000
2	2	2.0000
2	5	7.0000
3	3	3.0000
3	3	3.0000
3	4	6.0000
4	4	4.0000
4	4	4.0000
4	5	7.0000
5	5	5.0000
5	5	5.0000
5	6	8.0000
6	6	6.0000
7	7	7.0000
8	8	8.0000
9	9	9.0000
explain format=tree select a,sum(b),sum(d) from t1 group by a union all select b,sum(b),avg(d) from t2 group by b union all select b,sum(d),avg(c) from t3 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=0.75 rows=5)
            -> Stream results  (cost=2.25 rows=10)
                -> Group aggregate: sum(t3.d), sum(t3.c), count(t3.c)
                    -> PX Receiver (merge sort; slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Group aggregate: sum(t3.d), sum(t3.c), count(t3.c)  (cost=2.25 rows=10)
                                -> Parallel index scan on t3 using idx  (cost=1.25 rows=10)

(select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select * from t3 where c > 1 and d > 2 order by b,c desc);
a	b	c	d
10	10	10	10
2	2	2	2
3	3	3	3
4	4	4	4
5	5	5	5
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree (select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select * from t3 where c > 1 and d > 2 order by b,c desc);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.42..2.58 rows=6)
            -> Union materialize with deduplication  (cost=4.78..6.94 rows=6)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=2.50 rows=5)
                            -> Filter: ((t2.a > 1) and (t2.a is not null))  (cost=0.75 rows=5)
                                -> Parallel table scan on t2  (cost=0.75 rows=5)
                            -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=0.27 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: ((t3.c > 1) and (t3.d > 2))  (cost=1.25 rows=1)
                            -> Parallel table scan on t3  (cost=1.25 rows=10)

(select * from t3 where c > 1 and d > 3 order by a,b) union all (select t2.* from t1,t2,t3 where t1.a = t2.a and t2.a = t3.a order by t1.a);
a	b	c	d
1	1	1	1
1	1	1	1
10	10	10	10
2	2	2	2
2	2	2	2
3	3	3	3
4	4	4	4
4	4	4	4
5	5	5	5
5	5	5	5
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree (select * from t3 where c > 1 and d > 3 order by a,b) union all (select t2.* from t1,t2,t3 where t1.a = t2.a and t2.a = t3.a order by t1.a);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=1.25 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: ((t3.c > 1) and (t3.d > 3))  (cost=1.25 rows=1)
                            -> Parallel table scan on t3  (cost=1.25 rows=10)
            -> Stream results  (cost=5.25 rows=5)
                -> Nested loop inner join  (cost=5.25 rows=5)
                    -> Inner hash join (t1.a = t2.a)  (cost=4.50 rows=5)
                        -> Table scan on t1  (cost=0.07 rows=7)
                        -> Hash
                            -> PX Receiver (slice: 1; workers: 1)
                                -> PX Sender (slice: 3; workers: 4)
                                    -> Filter: (t2.a is not null)  (cost=0.75 rows=5)
                                        -> Parallel table scan on t2  (cost=0.75 rows=5)
                    -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=0.07 rows=1)

(select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select t2.* from t1 left join t2 on t1.a = t2.a where t1.a > 1 order by t1.b desc);
a	b	c	d
2	2	2	2
3	3	3	3
4	4	4	4
5	5	5	5
explain format=tree (select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select t2.* from t1 left join t2 on t1.a = t2.a where t1.a > 1 order by t1.b desc);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.16..2.70 rows=17)
            -> Union materialize with deduplication  (cost=6.07..8.60 rows=17)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=2.50 rows=5)
                            -> Filter: ((t2.a > 1) and (t2.a is not null))  (cost=0.75 rows=5)
                                -> Parallel table scan on t2  (cost=0.75 rows=5)
                            -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=0.27 rows=1)
                -> Left hash join (t2.a = t1.a)  (cost=1.74 rows=12)
                    -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                        -> Table scan on t1  (cost=0.95 rows=7)
                    -> Hash
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Parallel table scan on t2  (cost=0.32 rows=5)

explain (select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select t2.* from t1 left join t2 on t1.a = t2.a where t1.a > 1 order by t1.b desc);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Parallel scan (4 workers); Using where
1	PRIMARY	t3	NULL	eq_ref	PRIMARY	PRIMARY	4	test.t2.a	1	100.00	Using index
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Parallel scan (4 workers); Using where; Using join buffer (hash join)
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Receive from (<sender3>)
2	UNION	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Using where
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t3` join `test`.`t2` where ((`test`.`t3`.`a` = `test`.`t2`.`a`) and (`test`.`t2`.`a` > 1)) union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t1` left join `test`.`t2` on((`test`.`t2`.`a` = `test`.`t1`.`a`)) where (`test`.`t1`.`a` > 1)
select * from t1 union select * from t2 order by a;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	3	3	3
3	4	5	6
4	4	4	4
4	5	6	7
5	5	5	5
5	6	7	8
explain format=tree select * from t1 union select * from t2 order by a;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Sort: a  (cost=3.11..5.40 rows=12)
            -> Table scan on <union temporary>  (cost=2.50 rows=0)
                -> Union materialize with deduplication  (cost=3.11..5.40 rows=12)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 4)
                            -> Parallel table scan on t1  (cost=0.95 rows=7)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)

explain select * from t1 union select * from t2 order by a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Parallel scan (4 workers)
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Receive from (<sender3>)
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary; Using filesort
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` order by `a`
select * from t1 where a > 1 union all select * from t2 union all select * from t3 order by a,b;
a	b	c	d
1	1	1	1
1	1	1	1
10	10	10	10
2	2	2	2
2	2	2	2
2	2	2	2
2	3	4	5
3	3	3	3
3	3	3	3
3	4	5	6
4	4	4	4
4	4	4	4
4	5	6	7
5	5	5	5
5	5	5	5
5	6	7	8
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree select * from t1 where a > 1 union all select * from t2 union all select * from t3 order by a,b;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Sort: a, b  (cost=4.83..7.18 rows=17)
            -> Table scan on <union temporary>  (cost=2.50 rows=0)
                -> Union materialize  (cost=4.83..7.18 rows=17)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 4)
                            -> Filter: (t1.a > 1)  (cost=0.95 rows=2)
                                -> Parallel table scan on t1  (cost=0.95 rows=7)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Parallel table scan on t3  (cost=1.25 rows=10)

explain select * from t1 where a > 1 union all select * from t2 union all select * from t3 order by a,b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	33.33	Parallel scan (4 workers); Using where
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Parallel scan (4 workers)
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Receive from (<sender3>)
3	UNION	t3	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Parallel scan (4 workers)
3	UNION	<sender4>	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Send to (<receiver4>)
3	UNION	<receiver4>	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Receive from (<sender4>)
NULL	UNION RESULT	<union1,2,3>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary; Using filesort
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` > 1) union all /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2` union all /* select#3 */ select `test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t3`.`d` AS `d` from `test`.`t3` order by `a`,`b`
select * from ((select t2.a as c1,t2.b as c2,t2.c as c3,t2.d as c4 from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select t3.a as c1,t3.b as c2,t3.c as c3,t3.d as c4 from t3 where c > 1 and d > 2 order by b,c desc)) as v where v.c1 > 1;
c1	c2	c3	c4
10	10	10	10
2	2	2	2
3	3	3	3
4	4	4	4
5	5	5	5
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree select * from ((select t2.a as c1,t2.b as c2,t2.c as c3,t2.d as c4 from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select t3.a as c1,t3.b as c2,t3.c as c3,t3.d as c4 from t3 where c > 1 and d > 2 order by b,c desc)) as v where v.c1 > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on v  (cost=0.95..2.52 rows=3)
            -> Union materialize with deduplication  (cost=4.61..6.19 rows=3)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=1.33 rows=2)
                            -> Filter: ((t2.a > 1) and (t2.a > 1) and (t2.a is not null))  (cost=0.75 rows=2)
                                -> Parallel table scan on t2  (cost=0.75 rows=5)
                            -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=0.31 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: ((t3.c > 1) and (t3.d > 2) and (t3.a > 1))  (cost=2.06 rows=1)
                            -> Parallel index range scan on t3 using PRIMARY over (1 < a)  (cost=2.06 rows=9)

explain select * from ((select t2.a as c1,t2.b as c2,t2.c as c3,t2.d as c4 from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select t3.a as c1,t3.b as c2,t3.c as c3,t3.d as c4 from t3 where c > 1 and d > 2 order by b,c desc)) as v where v.c1 > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	<derived2>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	NULL
1	PRIMARY	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Send to (<receiver1>)
1	PRIMARY	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	2	100.00	Receive from (<sender1>)
2	DERIVED	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	33.33	Parallel scan (4 workers); Using where
2	DERIVED	t3	NULL	eq_ref	PRIMARY	PRIMARY	4	test.t2.a	1	100.00	Using index
2	DERIVED	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver2>)
2	DERIVED	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender2>)
3	UNION	t3	NULL	range	PRIMARY	PRIMARY	4	NULL	9	11.11	Parallel scan (4 workers); Using where
3	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Send to (<receiver3>)
3	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Receive from (<sender3>)
NULL	UNION RESULT	<union2,3>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `v`.`c1` AS `c1`,`v`.`c2` AS `c2`,`v`.`c3` AS `c3`,`v`.`c4` AS `c4` from (/* select#2 */ select `test`.`t2`.`a` AS `c1`,`test`.`t2`.`b` AS `c2`,`test`.`t2`.`c` AS `c3`,`test`.`t2`.`d` AS `c4` from `test`.`t3` join `test`.`t2` where ((`test`.`t3`.`a` = `test`.`t2`.`a`) and (`test`.`t2`.`a` > 1) and (`test`.`t2`.`a` > 1)) union /* select#3 */ select `test`.`t3`.`a` AS `c1`,`test`.`t3`.`b` AS `c2`,`test`.`t3`.`c` AS `c3`,`test`.`t3`.`d` AS `c4` from `test`.`t3` where ((`test`.`t3`.`c` > 1) and (`test`.`t3`.`d` > 2) and (`test`.`t3`.`a` > 1))) `v`
# --include set_px_to_saved.inc
drop table t1;
drop table t2;
drop table t3;
drop table t4;
drop database test1;
drop user user1;

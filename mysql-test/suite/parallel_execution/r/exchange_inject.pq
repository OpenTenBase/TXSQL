# --include save_and_set_px.inc
set txsql_parallel_table_record_threshold = 2;
#
# Table scan -> exchange
#
select * from t2;
id	col1	col2
1	9	1
10	0	10
2	8	2
3	7	3
4	6	4
5	5	5
6	4	6
7	3	7
8	2	8
9	1	9
select col1+1 from t1;
col1+1
10
11
2
3
4
5
6
7
8
9
#
# Table scan -> filter -> exchange
#
select * from t2 where col1 > 2;
id	col1	col2
1	9	1
2	8	2
3	7	3
4	6	4
5	5	5
6	4	6
7	3	7
select * from t2 having col1 > 2 and col1 < 4;
id	col1	col2
7	3	7
#
# Sort -> exchange
#
select * from t2 order by col1;
id	col1	col2
10	0	10
9	1	9
8	2	8
7	3	7
6	4	6
5	5	5
4	6	4
3	7	3
2	8	2
1	9	1
select * from t2 order by col1 desc;
id	col1	col2
1	9	1
2	8	2
3	7	3
4	6	4
5	5	5
6	4	6
7	3	7
8	2	8
9	1	9
10	0	10
#
# Nested loop join -> exchange
#
select * from t1 join t2 on t1.id=t2.id;
id	col1	col2	id	col1	col2
1	1	1	1	9	1
10	10	10	10	0	10
2	2	2	2	8	2
3	3	3	3	7	3
4	4	4	4	6	4
5	5	5	5	5	5
6	6	6	6	4	6
7	7	7	7	3	7
8	8	8	8	2	8
9	9	9	9	1	9
#
# Table scan -> exchange -> Aggregate -> exchange
#
select sum(col1) from t2;
sum(col1)
45
select sum(col1)+1 from t2;
sum(col1)+1
46
select avg(col1) from t2;
avg(col1)
4.5000
#
# Table scan -> Sort -> exchange -> Nested loop join -> exchange
#
select * from t1 join t2 on t1.id=t2.id order by t1.col1;
id	col1	col2	id	col1	col2
1	1	1	1	9	1
2	2	2	2	8	2
3	3	3	3	7	3
4	4	4	4	6	4
5	5	5	5	5	5
6	6	6	6	4	6
7	7	7	7	3	7
8	8	8	8	2	8
9	9	9	9	1	9
10	10	10	10	0	10
#
# Table scan -> Nested loop join -> exchange -> Sort -> exchange
#
select * from t1 join t2 on t1.id=t2.id order by t1.col1, t2.col1;
id	col1	col2	id	col1	col2
1	1	1	1	9	1
2	2	2	2	8	2
3	3	3	3	7	3
4	4	4	4	6	4
5	5	5	5	5	5
6	6	6	6	4	6
7	7	7	7	3	7
8	8	8	8	2	8
9	9	9	9	1	9
10	10	10	10	0	10
#
# Table scan -> join -> exchange -> TemptableAggregate -> exchange
#
select sum(t1.col1),sum(t2.col1) from t1 join t2 on t1.id=t2.id
group by t1.col2;
sum(t1.col1)	sum(t2.col1)
1	9
10	0
2	8
3	7
4	6
5	5
6	4
7	3
8	2
9	1
#
# join -> exchange -> TemptableAggregate -> exchange -> sort -> exchange
#
select sum(t1.col1),sum(t2.col1) from t1 join t2 on t1.id=t2.id
group by t1.col2 order by t1.col2;
sum(t1.col1)	sum(t2.col1)
1	9
2	8
3	7
4	6
5	5
6	4
7	3
8	2
9	1
10	0
#
# join -> exchange -> TemptableAggregate -> exchange -> filter -> exchange
#
select sum(t1.col1),sum(t2.col1) from t1 join t2 on t1.id=t2.id
group by t1.col2 having sum(t1.col1)>1;
sum(t1.col1)	sum(t2.col1)
10	0
2	8
3	7
4	6
5	5
6	4
7	3
8	2
9	1
select sum(t1.col1),sum(t2.col1) from t1 join t2 on t1.id=t2.id
group by t1.col2 having sum(t1.col1)>1 order by t1.col2 limit 1;
sum(t1.col1)	sum(t2.col1)
2	8
#
# join -> exchange -> Aggregate -> exchange
#
select sum(t1.col1),sum(t2.col1) from t1 join t2 on t1.id=t2.id;
sum(t1.col1)	sum(t2.col1)
55	45
#
# TemptableAggregate -> exchange -> filter -> exchange
#   item->arg[0]->const_item() == true
#
select count(*) from t1 group by col1 having count(*) > 0;
count(*)
1
1
1
1
1
1
1
1
1
1
#
# Aggregate with group by
#
select col2 from t1 group by col2;
col2
1
10
2
3
4
5
6
7
8
9
#
# Aggregate\TemptableAggregate with function
#
select count(*),repeat("a",count(*)) from t1;
count(*)	repeat("a",count(*))
10	aaaaaaaaaa
select col1,repeat("a",col1),count(*) from t1 group by col1;
col1	repeat("a",col1)	count(*)
1	a	1
10	aaaaaaaaaa	1
2	aa	1
3	aaa	1
4	aaaa	1
5	aaaaa	1
6	aaaaaa	1
7	aaaaaaa	1
8	aaaaaaaa	1
9	aaaaaaaaa	1
#
# filter -> exchange -> Aggregate
#
select col1,count(*) from t1 where col1 > 5 group by col1;
col1	count(*)
10	1
6	1
7	1
8	1
9	1
drop table t1, t2;
CREATE TABLE t1 (a INT, b INT, c INT, KEY (a,b));
INSERT INTO t1 VALUES (1,1,1), (1,2,1), (1,3,1), (1,4,1);
INSERT INTO t1 SELECT a, b + 4, 1 FROM t1;
INSERT INTO t1 SELECT a + 1, b, 1 FROM t1;
INSERT INTO t1 SELECT a + 2, b + 8, 1 FROM t1;
drop table t1;
#
# Subselect
#
CREATE TABLE t1 (a INT, b INT);
INSERT INTO t1 VALUES (4, 40), (1, 10), (2, 20), (2, 20), (3, 30);
CREATE TABLE t2 SELECT DISTINCT a FROM t1;
SELECT (SELECT t2.a FROM t2 WHERE t2.a = t1.a) aa, b, COUNT(         b)
FROM t1 GROUP BY aa, b;
aa	b	COUNT(         b)
1	10	1
2	20	2
3	30	1
4	40	1
drop table if exists t1;
drop table if exists t2;
drop procedure if exists p1;
#
# Const non-hidden items in select list
#
CREATE TABLE t1 (
auto int not null auto_increment,
fld1 int(6) unsigned zerofill DEFAULT '000000' NOT NULL,
companynr tinyint(2) unsigned zerofill DEFAULT '00' NOT NULL,
fld3 char(30) DEFAULT '' NOT NULL,
UNIQUE fld1 (fld1),
KEY fld3 (fld3),
PRIMARY KEY (auto)
) charset utf8mb4;
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
create table t2 (
name      char(32) not null,
key (name)
);
INSERT INTO t1 VALUES (346,098005,58,'hopelessness'),(350,098009,58,'imaginable'),(1160,158402,00,'spates');
INSERT INTO t2 VALUES ("spates"), ("spates"), ("spates"), ("spates"),("spates"), ("spates"), ("spates"), ("spates");
select t1.fld3 from t1 where companynr = 58 and fld3 like "%imaginable%";
fld3
imaginable
# t1.fld3 would be a const item.
select t1.fld3,count(*) from t1,t2 where t1.fld1=158402 and t2.name=t1.fld3 group by t2.name;
fld3	count(*)
spates	8
drop table t1,t2;
#
# Const item in type of Item_sum (min(a), max(a))
#
create table t1 (a int, b int, key(a, b)) engine=innodb;
insert into t1 values 
(94, 46), (31, 438), (61, 152), (78, 123), (88, 411), (122, 118), (0, 177),
(75, 42), (108, 67), (79, 349), (59, 188), (68, 206), (49, 345), (118, 380),
(111, 368), (94, 468), (56, 379), (77, 133), (29, 399), (9, 363), (23, 36),
(116, 390), (119, 368), (87, 351), (123, 411), (24, 398), (34, 202), (28, 499),
(30, 83), (5, 178), (60, 343), (4, 245), (104, 280), (106, 446), (127, 403),
(44, 307), (68, 454), (57, 135);
analyze table t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
select min(a), max(a), min(b), max(b) from t1;
min(a)	max(a)	min(b)	max(b)
0	127	36	499
drop table t1;
#
# Dual locking clauses
#
set @start_autocommit= @@global.autocommit;
set @@global.autocommit= 0;
CREATE USER test@localhost;
grant CREATE, SELECT, UPDATE on *.* to test@localhost;
CREATE USER test2@localhost;
grant CREATE, SELECT, UPDATE on *.* to test2@localhost;
connection default;
CREATE TABLE t1 ( a INT, b INT) ENGINE=InnoDB;
INSERT INTO t1 VALUES (1, 1);
INSERT INTO t1 VALUES (2, 2);
INSERT INTO t1 VALUES (3, 3);
CREATE TABLE t2 ( a INT) ENGINE=InnoDB;
INSERT INTO t2 VALUES (10);
INSERT INTO t2 VALUES (20);
INSERT INTO t2 VALUES (30);
ALTER TABLE t1 ADD PRIMARY KEY (a);
# Locking row with a=2
connect  con1,localhost,test,,test;
BEGIN;
SELECT * FROM t1 WHERE a=2 FOR UPDATE ;
a	b
2	2
connect  con2,localhost,test2,,test;
BEGIN;
SET SESSION innodb_lock_wait_timeout=1;
SELECT * FROM t1, t2 FOR SHARE OF t1 FOR UPDATE OF t2;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
connection default;
disconnect con1;
disconnect con2;
DROP TABLE t1, t2;
DROP USER test@localhost;
DROP USER test2@localhost;
set @@global.autocommit= @start_autocommit;
create table t1(a int, b int);
insert into t1 values(1,1);
insert into t1 values(3,3);
insert into t1 values(2,2);
insert into t1 values(5,5);
insert into t1 values(4,4);
insert into t1 values(10,10);
insert into t1 values(6,6);
insert into t1 values(8,8);
insert into t1 values(7,7);
insert into t1 values(9,9);
create table t2(a int, b int, primary key(a), key idx(b));
insert into t2 values(1,1);
insert into t2 values(2,2);
insert into t2 values(3,3);
insert into t2 values(4,4);
insert into t2 values(5,5);
insert into t2 values(6,6);
insert into t2 values(7,7);
insert into t2 values(8,8);
insert into t2 values(9,9);
insert into t2 values(10,10);
create table t3(a int, b int, c int, d int);
insert into t3 values (1,2,3,4), (2,1,0,3), (3,3,3,3), (3,4,4,4), (3,5,4,5), (5,5,5,6);
create temporary table tmp as (select * from t3);
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
drop table tmp;
analyze table t1,t2,t3;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
test.t3	analyze	status	OK
#
# Sort under join
# join -> px_merge_sort -> px_send -> sort -> scan
select * from t2 join t1 on t2.a = t1.a where t2.a > 1;
a	b	a	b
10	10	10	10
2	2	2	2
3	3	3	3
4	4	4	4
5	5	5	5
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree select * from t2 join t1 on t2.a = t1.a where t2.a > 1 order by t1.a;
EXPLAIN
-> Nested loop inner join  (cost=4.75 rows=10)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Sort: t1.a  (cost=1.25 rows=10)
                -> Filter: ((t1.a > 1) and (t1.a is not null))  (cost=1.25 rows=10)
                    -> Parallel table scan on t1  (cost=1.25 rows=10)
    -> Single-row index lookup on t2 using PRIMARY (a=t1.a)  (cost=0.26 rows=1)

select * from t2 join t1 on t2.a = t1.a where t2.a > 1 order by t1.a;
a	b	a	b
2	2	2	2
3	3	3	3
4	4	4	4
5	5	5	5
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
10	10	10	10
#
#  Push aggregation with sort down to parallelism.
#
explain format=tree select sql_big_result a, sum(b) from t1 group by a order by a;
EXPLAIN
-> Group aggregate: sum(t1.b)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Group aggregate: sum(t1.b)
                -> Sort: t1.a  (cost=1.25 rows=10)
                    -> Parallel table scan on t1  (cost=1.25 rows=10)

select sql_big_result a, sum(b) from t1 group by a order by a;
a	sum(b)
1	1
2	2
3	3
4	4
5	5
6	6
7	7
8	8
9	9
10	10
explain format=tree select sql_big_result a, sum(b) from t1 group by a order by NULL;
EXPLAIN
-> Group aggregate: sum(t1.b)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Group aggregate: sum(t1.b)
                -> Sort: t1.a  (cost=1.25 rows=10)
                    -> Parallel table scan on t1  (cost=1.25 rows=10)

select sql_big_result a, sum(b) from t1 group by a order by NULL;
a	sum(b)
1	1
2	2
3	3
4	4
5	5
6	6
7	7
8	8
9	9
10	10
# Sort may not be a direct child of AGG, but in its subtree.
explain format=tree select sql_big_result t1.a, sum(t1.b) from t2 join t1 on t2.a = t1.a where t2.a > 1 group by t1.a order by t1.a;
EXPLAIN
-> Group aggregate: sum(t1.b)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Group aggregate: sum(t1.b)
                -> Nested loop inner join  (cost=4.75 rows=10)
                    -> Sort: t1.a  (cost=1.25 rows=10)
                        -> Filter: ((t1.a > 1) and (t1.a is not null))  (cost=1.25 rows=10)
                            -> Parallel table scan on t1  (cost=1.25 rows=10)
                    -> Single-row index lookup on t2 using PRIMARY (a=t1.a)  (cost=0.26 rows=1)

select sql_big_result t1.a, sum(t1.b) from t2 join t1 on t2.a = t1.a where t2.a > 1 group by t1.a order by t1.a;
a	sum(t1.b)
2	2
3	3
4	4
5	5
6	6
7	7
8	8
9	9
10	10
# Multiple group fields
explain format=tree select sql_big_result a,c, sum(b) from t3 group by a,c order by a,c;
EXPLAIN
-> Group aggregate: sum(t3.b)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Group aggregate: sum(t3.b)
                -> Sort: t3.a, t3.c  (cost=8.05 rows=78)
                    -> Parallel table scan on t3  (cost=8.05 rows=78)

select sql_big_result a,c, sum(b) from t3 group by a,c order by a,c;
a	c	sum(b)
1	3	26
2	0	13
3	3	39
3	4	117
5	5	65
explain format=tree select sql_big_result a,c, sum(b) from t3 group by c,a order by c,a;
EXPLAIN
-> Group aggregate: sum(t3.b)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Group aggregate: sum(t3.b)
                -> Sort: t3.c, t3.a  (cost=8.05 rows=78)
                    -> Parallel table scan on t3  (cost=8.05 rows=78)

select sql_big_result a,c, sum(b) from t3 group by c,a order by c,a;
a	c	sum(b)
2	0	13
1	3	26
3	3	39
3	4	117
5	5	65
explain format=tree select sql_big_result a,c, sum(b) from t3 group by a,c order by c,a;
EXPLAIN
-> Sort: t3.c, t3.a
    -> Stream results
        -> Group aggregate: sum(t3.b)
            -> PX Receiver (merge sort; slice: 0; workers: 1)
                -> PX Sender (slice: 1; workers: 4)
                    -> Group aggregate: sum(t3.b)
                        -> Sort: t3.a, t3.c  (cost=8.05 rows=78)
                            -> Parallel table scan on t3  (cost=8.05 rows=78)

select sql_big_result a,c, sum(b) from t3 group by a,c order by c,a;
a	c	sum(b)
2	0	13
1	3	26
3	3	39
3	4	117
5	5	65
explain format=tree select sql_big_result a,c,d, sum(b) from t3 group by a,c,d order by a,c,d;
EXPLAIN
-> Group aggregate: sum(t3.b)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Group aggregate: sum(t3.b)
                -> Sort: t3.a, t3.c, t3.d  (cost=8.05 rows=78)
                    -> Parallel table scan on t3  (cost=8.05 rows=78)

select sql_big_result a,c,d, sum(b) from t3 group by a,c,d order by a,c,d;
a	c	d	sum(b)
1	3	4	26
2	0	3	13
3	3	3	39
3	4	4	52
3	4	5	65
5	5	6	65
explain format=tree select sql_big_result a,c,d, sum(b) from t3 group by d,a,c order by d,a,c;
EXPLAIN
-> Group aggregate: sum(t3.b)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Group aggregate: sum(t3.b)
                -> Sort: t3.d, t3.a, t3.c  (cost=8.05 rows=78)
                    -> Parallel table scan on t3  (cost=8.05 rows=78)

select sql_big_result a,c,d, sum(b) from t3 group by d,a,c order by d,a,c;
a	c	d	sum(b)
2	0	3	13
3	3	3	39
1	3	4	26
3	4	4	52
3	4	5	65
5	5	6	65
explain format=tree select sql_big_result a,c,d, sum(b) from t3 group by d,a,c order by a,c,d;
EXPLAIN
-> Sort: t3.a, t3.c, t3.d
    -> Stream results
        -> Group aggregate: sum(t3.b)
            -> PX Receiver (merge sort; slice: 0; workers: 1)
                -> PX Sender (slice: 1; workers: 4)
                    -> Group aggregate: sum(t3.b)
                        -> Sort: t3.d, t3.a, t3.c  (cost=8.05 rows=78)
                            -> Parallel table scan on t3  (cost=8.05 rows=78)

select sql_big_result a,c,d, sum(b) from t3 group by d,a,c order by a,c,d;
a	c	d	sum(b)
1	3	4	26
2	0	3	13
3	3	3	39
3	4	4	52
3	4	5	65
5	5	6	65
#
# Agg after Stream.
#
explain format=tree select sql_big_result a, sum(b) from t1 group by a order by sum(b);
EXPLAIN
-> Sort: `sum(b)`
    -> Stream results
        -> Group aggregate: sum(t1.b)
            -> PX Receiver (merge sort; slice: 0; workers: 1)
                -> PX Sender (slice: 1; workers: 4)
                    -> Group aggregate: sum(t1.b)
                        -> Sort: t1.a  (cost=1.25 rows=10)
                            -> Parallel table scan on t1  (cost=1.25 rows=10)

select sql_big_result a, sum(b) from t1 group by a order by sum(b);
a	sum(b)
1	1
2	2
3	3
4	4
5	5
6	6
7	7
8	8
9	9
10	10
drop table t1,t2,t3;
#
# Temp fields created for exchange are of type BLOB, changing exchange send mode from Item to field
#
CREATE TABLE t1 (id int NOT NULL auto_increment, st varchar(255) NOT NULL, u int(11) NOT NULL, primary key (id));
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
INSERT INTO t1 (st, u) VALUES ('a',1),('A',1),('aa',1),('AA',1),('aaa',0),('BBB',0);
# Exchange use table of sort instead of creating new tmp table
explain format=tree select if(u=1,binary st,st) s from t1 order by s;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: if((t1.u = 1),cast(t1.st as char charset binary),t1.st)  (cost=0.85 rows=6)
            -> Parallel table scan on t1  (cost=0.85 rows=6)

select if(u=1,binary st,st) s from t1 order by s;
s
A
AA
BBB
a
aa
aaa
drop table t1;
create table t1(a int, b int, c int);
insert into t1 values (1,1,1), (2,1,1), (3,3,3), (3,4,4), (4,5,5), (5,5,5);
create temporary table tmp as (select * from t1);
insert into t1 select * from tmp;
insert into tmp select * from t1;
insert into t1 select * from tmp;
insert into tmp select * from t1;
insert into t1 select * from tmp;
insert into tmp select * from t1;
insert into t1 select * from tmp;
insert into tmp select * from t1;
insert into t1 select * from tmp;
insert into tmp select * from t1;
insert into t1 select * from tmp;
drop table tmp;
alter table t1 add index idx(a,b);
analyze table t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
#
# When using the AGGREGATE operator for grouping, 
# merge sort must be used to ensure that the input of the final agg is in order.
#
explain format=tree select a, sum(b) from t1 group by a;
EXPLAIN
-> Group aggregate: sum(t1.b)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Group aggregate: sum(t1.b)
                -> Parallel index scan on t1 using idx  (cost=141.05 rows=1398)

select a, sum(b) from t1 group by a;
a	sum(b)
1	233
2	233
3	1631
4	1165
5	1165
explain format=tree select a, b, sum(c) from t1 group by a, b;
EXPLAIN
-> Group aggregate: sum(t1.c)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Group aggregate: sum(t1.c)
                -> Parallel index scan on t1 using idx  (cost=141.05 rows=1398)

select a, b, sum(c) from t1 group by a, b;
a	b	sum(c)
1	1	233
2	1	233
3	3	699
3	4	932
4	5	1165
5	5	1165
explain format=tree select a, b, sum(c) from t1 group by a, b order by a, b;
EXPLAIN
-> Group aggregate: sum(t1.c)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Group aggregate: sum(t1.c)
                -> Parallel index scan on t1 using idx  (cost=141.05 rows=1398)

select a, b, sum(c) from t1 group by a, b order by a, b;
a	b	sum(c)
1	1	233
2	1	233
3	3	699
3	4	932
4	5	1165
5	5	1165
explain format=tree select distinct a from t1 where b>2;
EXPLAIN
-> Group (no aggregates)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Group (no aggregates)
                -> Filter: (t1.b > 2)  (cost=141.05 rows=466)
                    -> Parallel index scan on t1 using idx  (cost=141.05 rows=1398)

select distinct a from t1 where b>2;
a
3
4
5
explain format=tree select distinct a, b from t1 where c>2;
EXPLAIN
-> Group (no aggregates)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Group (no aggregates)
                -> Filter: (t1.c > 2)  (cost=141.05 rows=466)
                    -> Parallel index scan on t1 using idx  (cost=141.05 rows=1398)

select distinct a, b from t1 where c>2;
a	b
3	3
3	4
4	5
5	5
# order by key
explain format=tree select a from t1 order by a;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 2)
        -> Parallel index scan on t1 using idx  (cost=141.05 rows=1398)

select a from t1 order by a;
a
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
explain format=tree select a from t1 where a<4 order by a;
EXPLAIN
-> Filter: (t1.a < 4)  (cost=188.17 rows=932)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Parallel index range scan on t1 using idx  (cost=188.17 rows=932)

select a from t1 where a<4 order by a;
a
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
explain format=tree select a from t1 where a<4 order by a desc;
EXPLAIN
-> Filter: (t1.a < 4)  (cost=188.17 rows=932)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Parallel index range scan on t1 using idx  (cost=188.17 rows=932)

select a from t1 where a<4 order by a desc;
a
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
# Deduplication has been optimized away.
explain format=tree select distinct a from t1 where a=2;
EXPLAIN
-> Group (computed in earlier step, no aggregates)
    -> Filter: (t1.a = 2)  (cost=2.18 rows=1)
        -> Index range scan on t1 using index_for_group_by(idx)  (cost=2.18 rows=1)

# Not supported
explain format=tree select count(distinct b) from t1;
EXPLAIN
-> Aggregate: count(distinct t1.b)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Parallel index scan on t1 using idx  (cost=141.05 rows=1398)

drop table t1;
#
# common-test.inc
#
CREATE TABLE t2 (
auto int not null auto_increment,
fld1 int(6) unsigned zerofill DEFAULT '000000' NOT NULL,
companynr tinyint(2) unsigned zerofill DEFAULT '00' NOT NULL,
fld3 char(30) DEFAULT '' NOT NULL,
fld4 char(35) DEFAULT '' NOT NULL,
fld5 char(35) DEFAULT '' NOT NULL,
fld6 char(4) DEFAULT '' NOT NULL,
UNIQUE fld1 (fld1),
KEY fld3 (fld3),
PRIMARY KEY (auto)
);
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
create table t3 (
period    int not null,
name      char(32) not null,
companynr int not null,
price     double(11,0),
price2     double(11,0),
key (period),
key (name)
);
Warnings:
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
create temporary table tmp select * from t3;
Warnings:
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
insert into tmp select * from t3;
insert into t3 select * from tmp;
alter table t3 add t2nr int not null auto_increment primary key first;
drop table tmp;
analyze table t2,t3;
Table	Op	Msg_type	Msg_text
test.t2	analyze	status	OK
test.t3	analyze	status	OK
explain format=tree
select fld3,period,price,price2 from t2,t3 where t2.fld1=t3.t2nr and period >= 1001 and period <= 1002 and t2.companynr = 37 order by fld3,period, price;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: t2.fld3, t3.period, t3.price
            -> Stream results  (cost=255.79 rows=24)
                -> Nested loop inner join  (cost=255.79 rows=24)
                    -> Filter: (t2.companynr = 37)  (cost=123.90 rows=120)
                        -> Parallel table scan on t2  (cost=123.90 rows=1199)
                    -> Filter: ((t2.fld1 = t3.t2nr) and (t3.period >= 1001) and (t3.period <= 1002))  (cost=1.00 rows=0)
                        -> Single-row index lookup on t3 using PRIMARY (t2nr=t2.fld1)  (cost=1.00 rows=1)

select fld3,period,price,price2 from t2,t3 where t2.fld1=t3.t2nr and period >= 1001 and period <= 1002 and t2.companynr = 37 order by fld3,period, price;
fld3	period	price	price2
admonishing	1002	28357832	8723648
analyzable	1002	28357832	8723648
annihilates	1001	5987435	234724
Antares	1002	28357832	8723648
astound	1001	5987435	234724
audiology	1001	5987435	234724
Augustine	1002	28357832	8723648
Baird	1002	28357832	8723648
bewilderingly	1001	5987435	234724
breaking	1001	5987435	234724
Conley	1001	5987435	234724
dentally	1002	28357832	8723648
dissociate	1002	28357832	8723648
elite	1001	5987435	234724
eschew	1001	5987435	234724
Eulerian	1001	5987435	234724
flanking	1001	5987435	234724
foldout	1002	28357832	8723648
funereal	1002	28357832	8723648
galling	1002	28357832	8723648
Graves	1001	5987435	234724
grazing	1001	5987435	234724
groupings	1001	5987435	234724
handgun	1001	5987435	234724
humility	1002	28357832	8723648
impulsive	1002	28357832	8723648
inch	1001	5987435	234724
intelligibility	1001	5987435	234724
jarring	1001	5987435	234724
lawgiver	1001	5987435	234724
lectured	1002	28357832	8723648
Merritt	1002	28357832	8723648
neonatal	1001	5987435	234724
offload	1002	28357832	8723648
parters	1002	28357832	8723648
pityingly	1002	28357832	8723648
puddings	1002	28357832	8723648
Punjab	1001	5987435	234724
quitter	1002	28357832	8723648
realtor	1001	5987435	234724
relaxing	1001	5987435	234724
repetitions	1001	5987435	234724
resumes	1001	5987435	234724
Romans	1002	28357832	8723648
rusting	1001	5987435	234724
scholastics	1001	5987435	234724
skulking	1002	28357832	8723648
stated	1002	28357832	8723648
suites	1002	28357832	8723648
sureties	1001	5987435	234724
testicle	1002	28357832	8723648
tinily	1002	28357832	8723648
tragedies	1001	5987435	234724
trimmings	1001	5987435	234724
vacuuming	1001	5987435	234724
ventilate	1001	5987435	234724
wallet	1001	5987435	234724
Weissmuller	1002	28357832	8723648
Wotan	1002	28357832	8723648
drop table t2,t3;
#
# found_rows()
#
create table t1 (id int);
insert into t1 values (1),(2),(3),(4);
analyze table t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
select * from t1;
id
1
2
3
4
select found_rows();
found_rows()
4
Warning	1287	FOUND_ROWS() is deprecated and will be removed in a future release. Consider using COUNT(*) instead.
Warnings:
select * from t1 order by id limit 1;
id
1
select found_rows();
found_rows()
1
Warning	1287	FOUND_ROWS() is deprecated and will be removed in a future release. Consider using COUNT(*) instead.
Warnings:
select SQL_CALC_FOUND_ROWS * from t1 order by id limit 1;
id
1
Warnings:
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
select found_rows();
found_rows()
4
Warning	1287	FOUND_ROWS() is deprecated and will be removed in a future release. Consider using COUNT(*) instead.
Warnings:
drop table t1;
#
# Special ref slice for Aggregate
#
CREATE TABLE t1 (a INT PRIMARY KEY NOT NULL, b INT);
INSERT INTO t1 VALUES (1,1),(2,2),(3,1);
CREATE TABLE t2 (a INT NOT NULL, b INT);
INSERT INTO t2 VALUES (1,1),(2,2),(3,3);
ANALYZE TABLE t1,t2;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
EXPLAIN FORMAT=TREE SELECT t1.b, COUNT(DISTINCT t1.a) FROM t1, t2 WHERE t1.a=t2.a GROUP BY t1.b;
EXPLAIN
-> Group aggregate: count(distinct t1.a)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 3)
            -> Sort: t1.b
                -> Table scan on <temporary>
                    -> Temporary table  (cost=1.60 rows=3)
                        -> Nested loop inner join  (cost=1.60 rows=3)
                            -> Parallel table scan on t2  (cost=0.55 rows=3)
                            -> Single-row index lookup on t1 using PRIMARY (a=t2.a)  (cost=0.28 rows=1)

SELECT t1.b, COUNT(DISTINCT t1.a) FROM t1, t2 WHERE t1.a=t2.a GROUP BY t1.b;
b	COUNT(DISTINCT t1.a)
1	2
2	1
EXPLAIN FORMAT=TREE SELECT t1.b, COUNT(t1.a) FROM t1, t2 WHERE t1.a=t2.a GROUP BY t1.b WITH ROLLUP;
EXPLAIN
-> Table scan on <temporary>
    -> Temporary table
        -> Group aggregate with rollup: count(t1.a)
            -> PX Receiver (merge sort; slice: 0; workers: 1)
                -> PX Sender (slice: 1; workers: 3)
                    -> Sort: t1.b
                        -> Table scan on <temporary>
                            -> Temporary table  (cost=1.60 rows=3)
                                -> Nested loop inner join  (cost=1.60 rows=3)
                                    -> Parallel table scan on t2  (cost=0.55 rows=3)
                                    -> Single-row index lookup on t1 using PRIMARY (a=t2.a)  (cost=0.28 rows=1)

SELECT t1.b, COUNT(t1.a) FROM t1, t2 WHERE t1.a=t2.a GROUP BY t1.b WITH ROLLUP;
b	COUNT(t1.a)
1	2
2	1
NULL	3
DROP TABLE t1,t2;
#
# Union
#
CREATE TABLE t1 (a INT);
INSERT INTO t1 values (1),(2),(3);
CREATE TABLE t2 (a INT);
INSERT INTO t2 values (3),(4),(5);
ANALYZE TABLE t1,t2;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
EXPLAIN FORMAT=TREE (SELECT SQL_CALC_FOUND_ROWS * FROM t1 LIMIT 1) UNION SELECT * FROM t2 LIMIT 1;
EXPLAIN
-> Limit: 1 row(s) (no early end due to SQL_CALC_FOUND_ROWS)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 1)
            -> Table scan on <union temporary>  (cost=2.50 rows=0)
                -> Union materialize with deduplication
                    -> Limit: 1 row(s)  (cost=0.55 rows=1)
                        -> Table scan on t1  (cost=0.55 rows=3)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 3)
                            -> Parallel table scan on t2  (cost=0.55 rows=3)

Warnings:
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
(SELECT SQL_CALC_FOUND_ROWS * FROM t1 LIMIT 1) UNION SELECT * FROM t2 LIMIT 1;
a
1
Warnings:
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
Warning	1287	SQL_CALC_FOUND_ROWS is deprecated and will be removed in a future release. Consider using two separate queries instead.
EXPLAIN FORMAT=TREE SELECT * FROM ((SELECT * FROM t1) UNION SELECT * FROM t2) AS t where a>RAND(1);
EXPLAIN
-> Filter: (t.a > rand(1))
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 1)
            -> Table scan on t  (cost=2.77 rows=6)
                -> Union materialize with deduplication
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 3)
                            -> Parallel table scan on t1  (cost=0.55 rows=3)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 3)
                            -> Parallel table scan on t2  (cost=0.55 rows=3)

SELECT * FROM ((SELECT * FROM t1) UNION SELECT * FROM t2) AS t where a>RAND(1);
a
1
2
3
4
5
EXPLAIN FORMAT=TREE SELECT * FROM ((SELECT * FROM t1) UNION SELECT * FROM t2) AS t ORDER BY RAND(1);
EXPLAIN
-> Sort: `rand(1)`
    -> Stream results
        -> PX Receiver (slice: 0; workers: 1)
            -> PX Sender (slice: 1; workers: 1)
                -> Table scan on t  (cost=3.18 rows=6)
                    -> Union materialize with deduplication
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 3)
                                -> Parallel table scan on t1  (cost=0.55 rows=3)
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 3)
                                -> Parallel table scan on t2  (cost=0.55 rows=3)

SELECT * FROM ((SELECT * FROM t1) UNION SELECT * FROM t2) AS t ORDER BY RAND(1);
a
1
2
3
4
5
DROP TABLE t1,t2;
# --include set_px_to_saved.inc

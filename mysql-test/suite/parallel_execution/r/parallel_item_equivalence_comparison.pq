#######################################
# pq item equivalence comparison test
#######################################
#######################################
# 1) test field::eq, testcase copy from main.ctype_latin1_de
#######################################
create table t1 (a char (20) not null, b int not null primary key auto_increment, index (a,b));
insert into t1 (a) values ('�'),('ac'),('ae'),('ad'),('�c'),('aeb');
insert into t1 (a) values ('�c'),('uc'),('ue'),('ud'),('�'),('ueb'),('uf');
insert into t1 (a) values ('�'),('oc'),('�a'),('oe'),('od'),('�c'),('oeb');
insert into t1 (a) values ('s'),('ss'),('�'),('�b'),('ssa'),('ssc'),('�a');
insert into t1 (a) values ('e�'),('u�'),('�o'),('��'),('��a'),('aeae');
insert into t1 (a) values ('q'),('a'),('u'),('o'),('�'),('�'),('a');
select a,b from t1 order by a,b;
a	b
a	35
a	40
ac	2
ad	4
ae	3
aeae	33
aeb	6
e�	28
o	37
oc	15
od	18
oe	17
oeb	20
q	34
s	21
ss	22
ssa	25
ssc	26
u	36
uc	8
ud	10
ue	9
ueb	12
uf	13
u�	29
�	1
�	11
�	14
�	23
�	38
�	39
�a	16
�a	27
�b	24
�c	5
�c	7
�c	19
�o	30
��	31
��a	32
select a,b from t1 order by upper(a),b;
a	b
a	35
a	40
ac	2
ad	4
ae	3
aeae	33
aeb	6
e�	28
o	37
oc	15
od	18
oe	17
oeb	20
q	34
s	21
ss	22
ssa	25
ssc	26
u	36
uc	8
ud	10
ue	9
ueb	12
uf	13
u�	29
�	1
�	11
�	14
�	23
�	38
�	39
�a	16
�a	27
�b	24
�c	5
�c	7
�c	19
�o	30
��	31
��a	32
drop table t1;
#######################################
# 2) test temptables which has differ table_name/path, testcase copy from main.func_math
#######################################
CREATE TABLE t(a INT, b VARCHAR(2));
INSERT INTO t VALUES (1,'a'), (2,'qw'), (1,'t'), (3,'t');
SELECT crc32(SUM(a)) FROM t;
crc32(SUM(a))
1790921346
SELECT crc32(AVG(a)) FROM t GROUP BY b;
crc32(AVG(a))
2875100430
2875100430
768278432
drop table t;
testcase copy from main.case
create table t1 (a int);
insert into t1 values(1),(2),(3),(4);
select case a when 1 then 2 when 2 then 3 else 0 end as fcase, count(*) from t1 group by fcase;
fcase	count(*)
0	2
2	1
3	1
analyze table t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
explain select case a when 1 then 2 when 2 then 3 else 0 end as fcase, count(*) from t1 group by fcase;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers); Using temporary
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Receive from (<sender1>); Using temporary
Warnings:
Note	1003	/* select#1 */ select (case `test`.`t1`.`a` when 1 then 2 when 2 then 3 else 0 end) AS `fcase`,count(0) AS `count(*)` from `test`.`t1` group by `fcase`
select case a when 1 then "one" when 2 then "two" else "nothing" end as fcase, count(*) from t1 group by fcase;
fcase	count(*)
nothing	2
one	1
two	1
drop table t1;
CREATE TABLE t (i1 INT,
d1 DOUBLE,
e2 DECIMAL(5,2));
INSERT INTO t VALUES ( 6,    6.0,  10.0/3),
( null, 9.0,  10.0/3),
( 1,    null, 10.0/3),
( 2,    2.0,  null  );
Warnings:
Note	1265	Data truncated for column 'e2' at row 1
Note	1265	Data truncated for column 'e2' at row 2
Note	1265	Data truncated for column 'e2' at row 3
SELECT IFNULL(e2,i1) nullif_c,
IF(e2 IS NULL,i1,e2) if_c,
SUM(d1) FROM t
GROUP BY e2,i1 ORDER BY nullif_c, SUM(d1);
nullif_c	if_c	SUM(d1)
2.00	2.00	2
3.33	3.33	NULL
3.33	3.33	6
3.33	3.33	9
DROP TABLE t;
#######################################
# 3) test select list (GetAccessPathsFromSelectList), testcase copy from main.subquery_mat_all
#######################################
CREATE TABLE t1 (a varchar(20)) ENGINE=INNODB;
INSERT INTO t1 VALUES ('a'), ('a'), ('a');
CREATE TABLE t2 (a INT) ENGINE=INNODB;
INSERT INTO t2 VALUES(1), (1), (1), (1);
ANALYZE TABLE t1, t2;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
SELECT
(SELECT NULL IN (NULL) FROM t1 WHERE a) =
ANY(SELECT 1 FROM t2)
FROM t1;
(SELECT NULL IN (NULL) FROM t1 WHERE a) =
ANY(SELECT 1 FROM t2)
NULL
NULL
NULL
Warnings:
Warning	1292	Truncated incorrect DOUBLE value: 'a'
Warning	1292	Truncated incorrect DOUBLE value: 'a'
Warning	1292	Truncated incorrect DOUBLE value: 'a'
DROP TABLE t1, t2;
#######################################
# 4) test MaterializePathParameters::eq with temptable, testcase copy from main.key
#######################################
CREATE TABLE t1 (a INT PRIMARY KEY AUTO_INCREMENT);
INSERT INTO t1 VALUES (), (), ();
SELECT 1 AS c1
FROM t1
ORDER BY (
SELECT 1 AS c2
FROM t1
GROUP BY GREATEST(LAST_INSERT_ID(), t1.a)
ORDER BY GREATEST(LAST_INSERT_ID(), t1.a)
LIMIT 1);
c1
1
1
1
DROP TABLE t1;
testcase copy from main.window_functions
CREATE TABLE t1(a INT);
INSERT INTO t1 VALUES(1),(1),(2),(2);
SELECT (SELECT COUNT(a) OVER (PARTITION BY a) FROM t1) FROM t1;
ERROR 21000: Subquery returns more than 1 row
DROP TABLE t1;
create table tt1(a int, b int);
insert into tt1 (a) values(1),(2);
analyze table tt1;
Table	Op	Msg_type	Msg_text
test.tt1	analyze	status	OK
explain format = tree select
(with recursive dt as
(select tt1.a as a union select a+1 from dt where a<10)
select dt1.a from dt dt1 where dt1.a=tt1.a
) as subq
from tt1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 2)
        -> Parallel table scan on tt1  (cost=*** rows=***)
-> Select #2 (subquery in projection; dependent)
    -> Single-row index lookup on dt1 using <auto_distinct_key> (a=tt1.a)
        -> Materialize recursive CTE dt with deduplication
            -> Rows fetched before execution  (cost=*** rows=***)
            -> Repeat until convergence
                -> Filter: (dt.a < 10)  (cost=*** rows=***)
                    -> Scan new records on dt  (cost=*** rows=***)

Warnings:
Note	1276	Field or reference 'test.tt1.a' of SELECT #3 was resolved in SELECT #1
Note	1276	Field or reference 'test.tt1.a' of SELECT #2 was resolved in SELECT #1
select
(with recursive dt as
(select tt1.a as a union select a+1 from dt where a<10)
select dt1.a from dt dt1 where dt1.a=tt1.a
) as subq
from tt1;
subq
1
2
drop table tt1;
#######################################
# 5) test Item_view_ref::eq, testcase copy from main.group_by_fd_no_prot
#######################################
SET @old_sql_mode = @@sql_mode;
SET @@sql_mode='ONLY_FULL_GROUP_BY';
create table t1(
a int,
b int not null,
c int not null,
d int,
unique key(b,c),
unique key(b,d)
);
insert into t1 (a,b) values(1,10),(1,20),(2,30),(2,40);
Warnings:
Warning	1364	Field 'c' doesn't have a default value
SET session debug = "+d,px_no_row_split";
explain format=tree select any_value(a), sum(b) from (select * from t1 order by a desc) as d;
EXPLAIN
-> Aggregate: sum(t1.b)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 1)
            -> Aggregate: sum(t1.b)
                -> Parallel table scan on t1  (cost=*** rows=***)

select any_value(a), sum(b) from (select * from t1 order by a desc) as d;
any_value(a)	sum(b)
1	100
SET session debug = "-d,px_no_row_split";
drop table t1;
#######################################
# 6) test Item_values_column::eq, testcase copy from main.table_value_constructor
#######################################
CREATE TABLE t0(a INT, b INT);
INSERT INTO t0 VALUES(1, 10), (2, 20);
explain format=tree SELECT * FROM t0 WHERE (a, b) IN (VALUES ROW(1, 10), ROW(2, 20));
EXPLAIN
-> Filter: <in_optimizer>((t0.a,t0.b),<exists>(select #2))  (cost=*** rows=***)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Parallel table scan on t0  (cost=*** rows=***)
    -> Select #2 (subquery in condition; dependent)
        -> Filter: ((<cache>(t0.a) = column_0) and (<cache>(t0.b) = column_1) and <is_not_null_test>(column_0) and <is_not_null_test>(column_1))
            -> Rows fetched before execution  (cost=*** rows=***)

SELECT * FROM t0 WHERE (a, b) IN (VALUES ROW(1, 10), ROW(2, 20));
a	b
1	10
2	20
drop table t0;
#######################################
# 7) test Item_ref::eq
#######################################
SET sql_mode = 'NO_ENGINE_SUBSTITUTION';
CREATE TABLE t1(
c1 DATE NOT NULL, 
c2 DATE NULL, 
c3 DATETIME, 
c4 TIMESTAMP, 
PRIMARY KEY(c1), 
UNIQUE(c2)
);
INSERT INTO t1 VALUES('0000-00-00', '0000-00-00', '2008-01-04', '2008-01-05');
INSERT INTO t1 VALUES('2007-05-25', '2007-05-25', '2007-05-26', '2007-05-26');
INSERT INTO t1 VALUES('2008-01-01', NULL        , '2008-01-02', '2008-01-03');
INSERT INTO t1 VALUES('2008-01-17', NULL        , NULL        , '2009-01-29');
INSERT INTO t1 VALUES('2009-01-29', '2009-01-29', '2009-01-29', '2009-01-29');
SELECT * FROM t1 WHERE c2 <=> NULL ORDER BY c1,c2;
c1	c2	c3	c4
2008-01-01	NULL	2008-01-02 00:00:00	2008-01-03 00:00:00
2008-01-17	NULL	NULL	2009-01-29 00:00:00
SELECT * FROM t1 WHERE c2 <=> NULL ORDER BY c1,c2 LIMIT 2;
c1	c2	c3	c4
2008-01-01	NULL	2008-01-02 00:00:00	2008-01-03 00:00:00
2008-01-17	NULL	NULL	2009-01-29 00:00:00
drop table t1;
SET sql_mode = default;
CREATE TABLE t0 (i0 INTEGER);
INSERT INTO t0 VALUES (1), (2), (3), (4), (5);
CREATE TABLE t1 (
a INTEGER,
b INTEGER,
c INTEGER,
INDEX k1 (a),
INDEX k2 (a,b)
);
INSERT INTO t1
SELECT i0 + (10 * i0) + (100 * i0),
(i0 + (10 * i0) + (100 * i0)) % 100,
(i0 + (10 * i0) + (100 * i0)) % 100
FROM t0;
INSERT INTO t1
SELECT i0 + (10 * i0) + (100 * i0),
(i0 + (10 * i0) + (100 * i0) + 1) % 100,
(i0 + (10 * i0) + (100 * i0) + 1) % 100
FROM t0;
INSERT INTO t1
SELECT i0 + (10 * i0) + (100 * i0),
(i0 + (10 * i0) + (100 * i0) + 1) % 100,
(i0 + (10 * i0) + (100 * i0) + 2) % 100
FROM t0;
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
SELECT  MAX(a) FROM t1 WHERE (b) IN (SELECT MIN(t2.b)
FROM (SELECT b from t1) AS t2 GROUP BY t2.b WITH
ROLLUP HAVING GROUPING (t2.b)=0);
MAX(a)
555
drop table t0,t1;
create table t3 (s1 int, s2 char);
create view v1 as select s1, s2 from t3;
select s2 from v1 vq1 where 2 = (select count(*) aa from v1 vq2 having vq1.s2 = aa);
s2
drop view v1;
drop table t3;

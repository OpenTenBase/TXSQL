# --include save_and_set_px.inc
#
# case from main.group_by
#
CREATE TABLE t (a INT);
INSERT INTO t VALUES (0), (1), (1), (2);
explain format=tree SELECT (SELECT SUM(t1.a) s FROM t t2 WHERE t2.a = 0 HAVING s > 3) FROM t t1;
EXPLAIN
-> Aggregate: sum(t1.a)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: sum(t1.a)
                -> Parallel table scan on t1  (cost=*** rows=***)
-> Select #2 (subquery in projection; dependent)
    -> Filter: (s > 3)
        -> Filter: (t2.a = 0)  (cost=*** rows=***)
            -> Table scan on t2  (cost=*** rows=***)

Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
SELECT (SELECT SUM(t1.a) s FROM t t2 WHERE t2.a = 0 HAVING s > 3) FROM t t1;
(SELECT SUM(t1.a) s FROM t t2 WHERE t2.a = 0 HAVING s > 3)
4
explain format=tree SELECT (SELECT AVG(t1.a) s FROM t t2 WHERE t2.a = 0 HAVING s > 0) FROM t t1;
EXPLAIN
-> Aggregate: sum(t1.a), count(t1.a)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: sum(t1.a), count(t1.a)
                -> Parallel table scan on t1  (cost=*** rows=***)
-> Select #2 (subquery in projection; dependent)
    -> Filter: (s > 0)
        -> Filter: (t2.a = 0)  (cost=*** rows=***)
            -> Table scan on t2  (cost=*** rows=***)

Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
SELECT (SELECT AVG(t1.a) s FROM t t2 WHERE t2.a = 0 HAVING s > 0) FROM t t1;
(SELECT AVG(t1.a) s FROM t t2 WHERE t2.a = 0 HAVING s > 0)
1.0000
explain format=tree SELECT (SELECT SUM(t1.a)+AVG(t1.a)+1 s FROM t t2 WHERE t2.a = 0 HAVING s > 3) FROM t t1;
EXPLAIN
-> Aggregate: sum(t1.a), count(t1.a), sum(t1.a)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: sum(t1.a), count(t1.a), sum(t1.a)
                -> Parallel table scan on t1  (cost=*** rows=***)
-> Select #2 (subquery in projection; dependent)
    -> Filter: (s > 3)
        -> Filter: (t2.a = 0)  (cost=*** rows=***)
            -> Table scan on t2  (cost=*** rows=***)

Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
SELECT (SELECT SUM(t1.a)+AVG(t1.a)+1 s FROM t t2 WHERE t2.a = 0 HAVING s > 3) FROM t t1;
(SELECT SUM(t1.a)+AVG(t1.a)+1 s FROM t t2 WHERE t2.a = 0 HAVING s > 3)
6.0000
explain format=tree SELECT (SELECT SUM(t1.a) s FROM t t2 WHERE t2.a = 0 group by t2.a HAVING s > 3) FROM t t1;
EXPLAIN
-> Aggregate: sum(t1.a)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: sum(t1.a)
                -> Parallel table scan on t1  (cost=*** rows=***)
-> Select #2 (subquery in projection; dependent)
    -> Filter: (s > 3)
        -> Group (no aggregates)
            -> Filter: (t2.a = 0)  (cost=*** rows=***)
                -> Table scan on t2  (cost=*** rows=***)

Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
SELECT (SELECT SUM(t1.a) s FROM t t2 WHERE t2.a = 0 group by t2.a HAVING s > 3) FROM t t1;
(SELECT SUM(t1.a) s FROM t t2 WHERE t2.a = 0 group by t2.a HAVING s > 3)
4
explain format=tree SELECT (SELECT SUM(t1.a) s FROM t t2 WHERE t2.a = 0 group by t1.a HAVING s > 3) FROM t t1;
EXPLAIN
-> Aggregate: sum(t1.a)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: sum(t1.a)
                -> Parallel table scan on t1  (cost=*** rows=***)
-> Select #2 (subquery in projection; dependent)
    -> Filter: (s > 3)
        -> Table scan on <temporary>
            -> Temporary table with deduplication  (cost=*** rows=***)
                -> Filter: (t2.a = 0)  (cost=*** rows=***)
                    -> Table scan on t2  (cost=*** rows=***)

Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
SELECT (SELECT SUM(t1.a) s FROM t t2 WHERE t2.a = 0 group by t1.a HAVING s > 3) FROM t t1;
(SELECT SUM(t1.a) s FROM t t2 WHERE t2.a = 0 group by t1.a HAVING s > 3)
4
explain format=tree SELECT (select (SELECT SUM(t1.a) s FROM t as t2 WHERE t2.a = 0 HAVING s > 3)+1) FROM t t1;
EXPLAIN
-> Aggregate: sum(t1.a)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: sum(t1.a)
                -> Parallel table scan on t1  (cost=*** rows=***)
-> Select #2 (subquery in projection; dependent)
    -> Rows fetched before execution  (cost=*** rows=***)
    -> Select #3 (subquery in projection; dependent)
        -> Filter: (s > 3)
            -> Filter: (t2.a = 0)  (cost=*** rows=***)
                -> Table scan on t2  (cost=*** rows=***)

Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #3 was resolved in SELECT #1
SELECT (select (SELECT SUM(t1.a) s FROM t as t2 WHERE t2.a = 0 HAVING s > 3) + 1) FROM t t1;
(select (SELECT SUM(t1.a) s FROM t as t2 WHERE t2.a = 0 HAVING s > 3) + 1)
5
DROP TABLE t;
#
# case from subquery.inc
#
CREATE TABLE t1 (a1 INT, a2 INT);
CREATE TABLE t2 (b1 INT, b2 INT);
INSERT INTO t1 VALUES (100, 200);
INSERT INTO t1 VALUES (101, 201);
INSERT INTO t2 VALUES (101, 201);
INSERT INTO t2 VALUES (103, 203);
explain format=tree SELECT ((a1) IN (SELECT sum(a1) FROM t2 WHERE b2 > 0)) FROM t1;
EXPLAIN
-> Aggregate: sum(t1.a1)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 2)
            -> Aggregate: sum(t1.a1)
                -> Parallel table scan on t1  (cost=0.45 rows=2)
-> Select #2 (subquery in projection; dependent)
    -> Limit: 1 row(s)
        -> Filter: <if>(outer_field_is_not_null, (<cache>(t1.a1) = <ref_null_helper>(sum(t1.a1))), true)
            -> Filter: (t2.b2 > 0)  (cost=0.45 rows=1)
                -> Table scan on t2  (cost=0.45 rows=2)

Warnings:
Note	1276	Field or reference 'test.t1.a1' of SELECT #2 was resolved in SELECT #1
SELECT ((a1) IN (SELECT sum(a1) FROM t2 WHERE b2 > 0)) FROM t1;
((a1) IN (SELECT sum(a1) FROM t2 WHERE b2 > 0))
0
DROP TABLE t1, t2;
# --include set_px_to_saved.inc

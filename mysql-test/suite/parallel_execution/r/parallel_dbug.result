CREATE TABLE t1(id INT);
INSERT INTO t1 VALUES (1),(2),(3),(4);
INSERT INTO t1 SELECT a.id FROM t1 a,t1 b,t1 c,t1 d;
# --include save_and_set_px.inc
EXPLAIN format=tree SELECT SUM(EXISTS(SELECT RAND() FROM t1)) FROM t1;
EXPLAIN
-> Aggregate: sum(exists(select #2))
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel table scan on t1  (cost=*** rows=***)
-> Select #2 (subquery in projection; uncacheable)
    -> Limit: 1 row(s)  (cost=*** rows=***)
        -> Table scan on t1  (cost=*** rows=***)

SET SESSION debug="d,subselect_exec_fail";
SELECT SUM(EXISTS(SELECT RAND() FROM t1)) FROM t1;
SUM(EXISTS(SELECT RAND() FROM t1))
0
# --include set_px_to_saved.inc
DROP TABLE t1;

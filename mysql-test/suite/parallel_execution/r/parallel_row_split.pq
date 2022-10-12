create table t1(a int, b int, c int, d int);
insert into t1 values(1,2,3,4);
insert into t1 values(2,3,4,5);
insert into t1 values(3,4,5,6);
insert into t1 values(4,5,6,7);
insert into t1 values(5,6,7,8);
# --include save_and_set_px.inc
EXPLAIN FORMAT = TREE select sum(b) from t1;
EXPLAIN
-> Aggregate: sum(t1.b)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: sum(t1.b)  (cost=1.25 rows=1)
                -> Parallel table scan on t1  (cost=0.75 rows=5)

select sum(b) from t1;
sum(b)
20
SET session debug = "+d,px_no_row_split";
EXPLAIN FORMAT = TREE select sum(b) from t1;
EXPLAIN
-> Aggregate: sum(t1.b)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 1)
            -> Aggregate: sum(t1.b)  (cost=1.25 rows=1)
                -> Parallel table scan on t1  (cost=0.75 rows=5)

select sum(b) from t1;
sum(b)
20
SET session debug = default;
# --include set_px_to_saved.inc
drop table t1;

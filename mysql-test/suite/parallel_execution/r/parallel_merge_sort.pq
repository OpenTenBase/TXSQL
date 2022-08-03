# --include save_and_set_px.inc
DROP TABLE IF EXISTS t1;
Warnings:
Note	1051	Unknown table 'test.t1'
CREATE TABLE t1(
id INT PRIMARY KEY AUTO_INCREMENT,
col1 VARCHAR(100),
col2 INT
)ENGINE = InnoDB;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
EXPLAIN FORMAT=TREE select * from t1 order by col2;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: t1.col2  (cost=1.25 rows=10)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 order by col2;
id	col1	col2
10	testtesttesttest	1
9	testtesttesttest	2
8	testtesttesttest	3
7	testtesttesttest	4
6	testtesttesttest	5
5	testtesttesttest	6
4	testtesttesttest	7
3	testtesttesttest	8
2	testtesttesttest	9
1	testtesttesttest	10
set session debug="+d,px_receiver_init_error1";
set session debug="+d,px_binary_heap_error1";
set session debug="+d,px_load_group_record_error1";
set session debug="+d,px_store_mq_record_error";
set session debug="+d,px_decode_error";
EXPLAIN FORMAT=TREE select * from t1 order by col2;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: t1.col2  (cost=1.25 rows=10)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 order by col2;
ERROR HY000: PX execute error of errno[3044].
set session debug="-d,px_receiver_init_error1";
select * from t1 order by col2;
ERROR HY000: PX execute error of errno[3044].
set session debug="-d,px_binary_heap_error1";
EXPLAIN FORMAT=TREE select * from t1 order by col2;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: t1.col2  (cost=1.25 rows=10)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 order by col2;
ERROR 70100: Query execution was interrupted
set session debug="-d,px_load_group_record_error1";
EXPLAIN FORMAT=TREE select * from t1 order by col2;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: t1.col2  (cost=1.25 rows=10)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 order by col2;
ERROR HY000: PX execute error of errno[3044].
set session debug="-d,px_store_mq_record_error";
EXPLAIN FORMAT=TREE select * from t1 order by col2;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: t1.col2  (cost=1.25 rows=10)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 order by col2;
ERROR HY000: PX execute error of errno[8500].
set session debug="-d,px_decode_error";
EXPLAIN FORMAT=TREE select * from t1 order by col2;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: t1.col2  (cost=1.25 rows=10)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 order by col2;
id	col1	col2
10	testtesttesttest	1
9	testtesttesttest	2
8	testtesttesttest	3
7	testtesttesttest	4
6	testtesttesttest	5
5	testtesttesttest	6
4	testtesttesttest	7
3	testtesttesttest	8
2	testtesttesttest	9
1	testtesttesttest	10
# --include set_px_to_saved.inc
DROP TABLE t1;

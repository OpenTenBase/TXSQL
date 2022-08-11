# --include save_and_set_px.inc
DROP TABLE IF EXISTS t1;
Warnings:
Note	1051	Unknown table 'test.t1'
CREATE TABLE t1(
id INT PRIMARY KEY AUTO_INCREMENT,
col1 VARCHAR(100),
col2 INT,
KEY idx_col2(col2)
)ENGINE = InnoDB;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
SET session debug = "+d,px_no_row_split";
###
### TEST ATTACH TO EXCHANGE CHANNEL OOM FOR WORKER
###
EXPLAIN FORMAT=TREE SELECT * FROM t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Parallel table scan on t1  (cost=1.25 rows=10)

SET session debug = "+d,px_sender_attach_error";
SELECT * FROM t1;
ERROR HY000: Memory allocation error:  in function PX_mq_channel::attach().
SET session debug = "-d,px_sender_attach_error";
SELECT * FROM t1;
id	col1	col2
1	testtesttesttest	10
10	testtesttesttest	1
2	testtesttesttest	9
3	testtesttesttest	8
4	testtesttesttest	7
5	testtesttesttest	6
6	testtesttesttest	5
7	testtesttesttest	4
8	testtesttesttest	3
9	testtesttesttest	2
###
### TEST CREATE PX_CODEC OOM FOR WORKER
###
EXPLAIN FORMAT=TREE SELECT * FROM t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Parallel table scan on t1  (cost=1.25 rows=10)

SET session debug = "+d,px_sender_create_codec_error";
SELECT * FROM t1;
ERROR HY000: Memory allocation error: creating codec in function PX_sender::register_to_exchange().
SET session debug = "-d,px_sender_create_codec_error";
SELECT * FROM t1;
id	col1	col2
1	testtesttesttest	10
10	testtesttesttest	1
2	testtesttesttest	9
3	testtesttesttest	8
4	testtesttesttest	7
5	testtesttesttest	6
6	testtesttesttest	5
7	testtesttesttest	4
8	testtesttesttest	3
9	testtesttesttest	2
###
### TEST INIT PX_CODEC OOM FOR WORKER
###
EXPLAIN FORMAT=TREE SELECT * FROM t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Parallel table scan on t1  (cost=1.25 rows=10)

SET session debug = "+d,px_sender_init_codec_error";
SELECT * FROM t1;
ERROR HY000: Memory allocation error:  in function PX_compact_codec::init().
SET session debug = "-d,px_sender_init_codec_error";
SELECT * FROM t1;
id	col1	col2
1	testtesttesttest	10
10	testtesttesttest	1
2	testtesttesttest	9
3	testtesttesttest	8
4	testtesttesttest	7
5	testtesttesttest	6
6	testtesttesttest	5
7	testtesttesttest	4
8	testtesttesttest	3
9	testtesttesttest	2
###
### TEST PARALLEL SCAN ERROR FOR WORKER
###
EXPLAIN FORMAT=TREE SELECT * FROM t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Parallel table scan on t1  (cost=1.25 rows=10)

SET session debug = "+d,px_sender_parallel_scan_error1";
SELECT * FROM t1;
ERROR HY000: Tablespace is missing for table `test`.`t1`.
SET session debug = "-d,px_sender_parallel_scan_error1";
SET session debug = "-d,px_no_row_split";
# --include set_px_to_saved.inc
DROP TABLE t1;

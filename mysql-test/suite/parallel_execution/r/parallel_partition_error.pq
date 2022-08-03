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
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 3)
        -> Filter: (t1.col2 < 3)  (cost=0.66 rows=2)
            -> Parallel index range scan on t1 using idx_col2  (cost=0.66 rows=2)

set session debug="+d,px_partition_error1";
set session debug="+d,px_partition_error2";
set session debug="+d,px_partition_error3";
set session debug="+d,px_partition_error4";
set session debug="+d,px_partition_error5";
###
### HA_ERR_TABLE_DEF_CHANGED in px_do_partition
###
# Fallback for ER_TABLE_DEF_CHANGED
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
EXPLAIN
-> Filter: (t1.col2 < 3)  (cost=0.66 rows=2)
    -> Index range scan on t1 using idx_col2  (cost=0.66 rows=2)

set session debug="-d,px_partition_error1";
###
### Create PX_reader oom
###
# Fallback for ER_OUT_OF_RESOURCES
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
EXPLAIN
-> Filter: (t1.col2 < 3)  (cost=0.66 rows=2)
    -> Index range scan on t1 using idx_col2  (cost=0.66 rows=2)

set session debug="-d,px_partition_error2";
###
### make partition tuple error
###
# Fallback for ER_GET_ERRNO
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
EXPLAIN
-> Filter: (t1.col2 < 3)  (cost=0.66 rows=2)
    -> Index range scan on t1 using idx_col2  (cost=0.66 rows=2)

set session debug="-d,px_partition_error3";
###
### add_scan oom
###
# Fallback for ER_GET_ERRNO
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
EXPLAIN
-> Filter: (t1.col2 < 3)  (cost=0.66 rows=2)
    -> Index range scan on t1 using idx_col2  (cost=0.66 rows=2)

# Fallback for ER_GET_ERRNO
EXPLAIN FORMAT=TREE SELECT * FROM t1;
EXPLAIN
-> Table scan on t1  (cost=1.25 rows=10)

set session debug="-d,px_partition_error4";
###
### repartitioning oom
###
# Fallback for ER_GET_ERRNO
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
EXPLAIN
-> Filter: (t1.col2 < 3)  (cost=0.66 rows=2)
    -> Index range scan on t1 using idx_col2  (cost=0.66 rows=2)

set session debug="-d,px_partition_error5";
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 3)
        -> Filter: (t1.col2 < 3)  (cost=0.66 rows=2)
            -> Parallel index range scan on t1 using idx_col2  (cost=0.66 rows=2)

###
### tablespace was discarded
###
ALTER TABLE t1 DISCARD TABLESPACE;
# Fallback for ER_TABLESPACE_DISCARDED
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
EXPLAIN
-> Filter: (t1.col2 < 3)  (cost=12.00 rows=3)
    -> Index scan on t1 using idx_col2  (cost=12.00 rows=10)

# --include set_px_to_saved.inc
DROP TABLE t1;

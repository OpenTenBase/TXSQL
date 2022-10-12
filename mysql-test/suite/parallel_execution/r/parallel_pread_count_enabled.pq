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
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
EXPLAIN FORMAT=TREE SELECT count(*) from t1;
EXPLAIN
-> Count rows in t1

SELECT count(*) from t1;
count(*)
10
set session txsql_pread_count_enabled=false;
EXPLAIN FORMAT=TREE SELECT count(*) from t1;
EXPLAIN
-> Aggregate: count(0)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: count(0)  (cost=2.25 rows=1)
                -> Parallel index scan on t1 using idx_col2  (cost=1.25 rows=10)

SELECT count(*) from t1;
count(*)
10
set session txsql_pread_count_enabled=true;
EXPLAIN FORMAT=TREE SELECT count(*) from t1;
EXPLAIN
-> Count rows in t1

SELECT count(*) from t1;
count(*)
10
DROP TABLE t1;
# --include set_px_to_saved.inc

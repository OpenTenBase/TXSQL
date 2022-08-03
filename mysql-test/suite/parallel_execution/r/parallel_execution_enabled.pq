###
### 0. Prepare Data
###
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
###
### 0. Save Initial Value
###
# --include save_and_set_px.inc
set @saved_txsql_parallel_execution_enabled = @@txsql_parallel_execution_enabled;
###
### 1. Visibility
###
# 1.1 normal user cant see this switch
SHOW VARIABLES LIKE "%txsql_parallel_execution_enabled%";
Variable_name	Value

# 1.2 only tencentroot can see/set this switch
SHOW VARIABLES LIKE "%txsql_parallel_execution_enabled%";
Variable_name	Value
txsql_parallel_execution_enabled	ON
set global txsql_parallel_execution_enabled = 0;
SHOW VARIABLES LIKE "%txsql_parallel_execution_enabled%";
Variable_name	Value
txsql_parallel_execution_enabled	OFF

###
### 2. Priority
###
set global txsql_parallel_execution_enabled = 1;
# 2.1 enable=true,  max_threads > 0
set global txsql_max_parallel_worker_threads = 8;
# expect: parallel plan
EXPLAIN FORMAT = TREE SELECT * FROM t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel table scan on t1  (cost=1.25 rows=10)


# 2.2 enable=true,  max_threads = 0
set global txsql_max_parallel_worker_threads = 0;
# expect: serial plan
EXPLAIN FORMAT = TREE SELECT * FROM t1;
EXPLAIN
-> Table scan on t1  (cost=1.25 rows=10)


set global txsql_parallel_execution_enabled = 0;
# 2.3 enable=false, max_threads > 0
set global txsql_max_parallel_worker_threads = 8;
# expect: serial plan
EXPLAIN FORMAT = TREE SELECT * FROM t1;
EXPLAIN
-> Table scan on t1  (cost=1.25 rows=10)


# 2.4 enable=false,  max_threads = 0
set global txsql_max_parallel_worker_threads = 0;
# expect: serial plan
EXPLAIN FORMAT = TREE SELECT * FROM t1;
EXPLAIN
-> Table scan on t1  (cost=1.25 rows=10)


###
### Clean Up
###
# --include set_px_to_saved.inc
set global txsql_parallel_execution_enabled = @saved_txsql_parallel_execution_enabled;
drop table t1;

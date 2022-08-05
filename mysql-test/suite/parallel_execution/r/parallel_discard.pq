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
###
### tablespace was discarded
###
ALTER TABLE t1 DISCARD TABLESPACE;
# Fallback for ER_TABLESPACE_DISCARDED
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
EXPLAIN
-> Filter: (t1.col2 < 3)  (cost=12.00 rows=3)
    -> Index scan on t1 using idx_col2  (cost=12.00 rows=10)

SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 < 3;
ERROR HY000: Tablespace has been discarded for table 't1'
DROP TABLE t1;

show variables like 'partition_table_skip_limit';
Variable_name	Value
partition_table_skip_limit	OFF
set global partition_table_skip_limit = 1;
CREATE TABLE t1 (
col1 INT NOT NULL,
col2 INT NOT NULL,
col3 INT NOT NULL,
col4 INT NOT NULL,
UNIQUE KEY (col1, col2)
) PARTITION BY HASH(col3) PARTITIONS 4;
INSERT INTO t1 VALUES (1,2,3,4),(2,3,4,5),(3,4,5,6),(4,5,6,7);
SELECT * FROM t1 ORDER BY col1;
col1	col2	col3	col4
1	2	3	4
2	3	4	5
3	4	5	6
4	5	6	7
CREATE TABLE t2 (
col1 INT NOT NULL,
col2 INT NOT NULL,
col3 INT NOT NULL,
col4 INT NOT NULL,
UNIQUE KEY (col1),
UNIQUE KEY (col3)
) PARTITION BY HASH(col1 + col3) PARTITIONS 4;
INSERT INTO t2 VALUES (1,2,3,4),(2,3,4,5),(3,4,5,6),(4,5,6,7);
SELECT * FROM t2 ORDER BY col1;
col1	col2	col3	col4
1	2	3	4
2	3	4	5
3	4	5	6
4	5	6	7
CREATE TABLE t3 (
col1 INT NOT NULL,
col2 INT NOT NULL,
col3 INT NOT NULL,
col4 INT NOT NULL,
UNIQUE KEY (col1, col2),
UNIQUE KEY (col3)
) PARTITION BY HASH(col1 + col3) PARTITIONS 4;
INSERT INTO t3 VALUES (1,2,3,4),(2,3,4,5),(3,4,5,6),(4,5,6,7);
SELECT * FROM t3 ORDER BY col1;
col1	col2	col3	col4
1	2	3	4
2	3	4	5
3	4	5	6
4	5	6	7
CREATE TABLE np_pk (id INT NOT NULL AUTO_INCREMENT, name VARCHAR(50), added DATE, PRIMARY KEY (id));
ALTER TABLE np_pk PARTITION BY HASH(TO_DAYS(added)) PARTITIONS 4;
CREATE TABLE t4 (a INT PRIMARY KEY, b INT, c INT, UNIQUE KEY(c))
PARTITION BY LIST(b) (
PARTITION p1 VALUES IN (2,3),
PARTITION p2 VALUES IN (4,5),
PARTITION p3 VALUES IN (6,7));
INSERT INTO t4 VALUES (1,2,3),(2,3,4),(3,4,5),(4,5,6),(5,6,7),(6,7,8);
SELECT * FROM t4 order by c;
a	b	c
1	2	3
2	3	4
3	4	5
4	5	6
5	6	7
6	7	8
SELECT * FROM t4 WHERE b = 5;
a	b	c
4	5	6
set global partition_table_skip_limit = 0;
DROP TABLE t1;
DROP TABLE t2;
DROP TABLE t3;
DROP TABLE t4;
DROP TABLE np_pk;

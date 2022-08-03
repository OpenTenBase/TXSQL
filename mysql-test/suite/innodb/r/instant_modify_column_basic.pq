############################################
# Test instant MODIFY COLUMN for DYNAMIC format
############################################
#
# Scenario 1:
# Create a small table, and modify all kinds of columns and verify
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=DYNAMIC;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 MODIFY COLUMN a BIGINT, ALGORITHM=INSTANT;
ERROR 0A000: ALGORITHM=INSTANT is not supported. Reason: Cannot change column type INPLACE. Try ALGORITHM=COPY/INPLACE.
ALTER TABLE t1 MODIFY COLUMN b BIGINT, ALGORITHM=INSTANT;
INSERT INTO t1(a, b) VALUES(0, 1);
SELECT * FROM t1;
a	b
1	1
2	2
3	3
4	4
5	5
6	1
ALTER TABLE t1 ADD COLUMN c3 INT NOT NULL DEFAULT 10, ALGORITHM=INSTANT;
INSERT INTO t1(a, c3) VALUES(0, 11);
SELECT * FROM t1;
a	b	c3
1	1	10
2	2	10
3	3	10
4	4	10
5	5	10
6	1	10
7	NULL	11
ALTER TABLE t1 ADD COLUMN c4 CHAR(50), ADD COLUMN c5 VARCHAR(100), ALGORITHM=INSTANT;
INSERT INTO t1(a, c4, c5) VALUES(0, 'ABCD EFGH', 'abcdefg hijklmn ');
SELECT * FROM t1;
a	b	c3	c4	c5
1	1	10	NULL	NULL
2	2	10	NULL	NULL
3	3	10	NULL	NULL
4	4	10	NULL	NULL
5	5	10	NULL	NULL
6	1	10	NULL	NULL
7	NULL	11	NULL	NULL
8	NULL	10	ABCD EFGH	abcdefg hijklmn 
ALTER TABLE t1 MODIFY COLUMN c4 CHAR(100), ALGORITHM=INSTANT;
ERROR 0A000: ALGORITHM=INSTANT is not supported. Reason: Cannot change column type INPLACE. Try ALGORITHM=COPY/INPLACE.
INSERT INTO t1(a, c4, c5) VALUES(0, 'ABCD EFGH', 'abcdefg hijklmn ');
SELECT * FROM t1;
a	b	c3	c4	c5
1	1	10	NULL	NULL
2	2	10	NULL	NULL
3	3	10	NULL	NULL
4	4	10	NULL	NULL
5	5	10	NULL	NULL
6	1	10	NULL	NULL
7	NULL	11	NULL	NULL
8	NULL	10	ABCD EFGH	abcdefg hijklmn 
9	NULL	10	ABCD EFGH	abcdefg hijklmn 
ALTER TABLE t1 ADD COLUMN m1 CHAR(50) default 'The fox jumps over', ADD COLUMN m2 VARCHAR(50) DEFAULT 'The fox jumps over the lazy dog.', ALGORITHM=inplace;
Table ID differed
SELECT 2 = instant_cols AS `Instant columns equal` FROM information_schema.innodb_tables WHERE name like '%t1%';
Instant columns equal
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN m1 CHAR(100), ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN m2 CHAR(100), ALGORITHM=INSTANT;
INSERT INTO t1(a, m1, m2) VALUES(0, 'over the lazy dog', 'The lazy dog jumps over the fox.');
SELECT count(*) = max(a) FROM t1 WHERE m1 = 'The fox jumps over';
count(*) = max(a)
1
SELECT count(*) = max(a) FROM t1 WHERE m2 like 'The fox jumps%';
count(*) = max(a)
1
SELECT m1 FROM t1 WHERE m1 = 'over the lazy dog';
m1
over the lazy dog
SELECT m2 FROM t1 WHERE m2 like '%the fox.';
m2
The lazy dog jumps over the fox.
ALTER TABLE t1 ADD COLUMN n1 BINARY(10), ADD COLUMN n2 VARBINARY(10), ALGORITHM=inplace;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
INSERT INTO t1(a, n1, n2) VALUES(0, 0x010203040506070809, 0x102030405060708090);
ALTER TABLE t1 MODIFY COLUMN n1 BINARY(100), ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN n2 VARBINARY(100), ALGORITHM=INSTANT;
SELECT count(*) = max(a) FROM t1 WHERE n1 IS NULL;
count(*) = max(a)
1
SELECT count(*) = max(a) FROM t1 WHERE n2 IS NULL;
count(*) = max(a)
1
SELECT hex(n1) FROM t1 WHERE n1 = 0x01020304050607080900;
hex(n1)
SELECT hex(n2) FROM t1 WHERE n2 = 0x102030405060708090;
hex(n2)
102030405060708090
ALTER TABLE t1 ADD COLUMN o1 BINARY(10) DEFAULT 0x11223344, ADD COLUMN o2 VARBINARY(10) DEFAULT 0x55667788, ALGORITHM=inplace;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN o1 BINARY(100), ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN o2 VARBINARY(100), ALGORITHM=INSTANT;
INSERT INTO t1(a, o1, o2) VALUES(0, 0x44332211, 0x88776655);
SELECT count(*) = max(a) FROM t1 WHERE o1 = 0x11223344000000000000;
count(*) = max(a)
NULL
SELECT count(*) = max(a) FROM t1 WHERE o2 = 0x55667788;
count(*) = max(a)
1
SELECT hex(o1) FROM t1 WHERE o1 = 0x44332211000000000000;
hex(o1)
SELECT hex(o2) FROM t1 WHERE o2 = 0x88776655;
hex(o2)
88776655
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` bigint DEFAULT NULL,
  `c3` int NOT NULL DEFAULT '10',
  `c4` char(50) DEFAULT NULL,
  `c5` varchar(100) DEFAULT NULL,
  `m1` char(100) DEFAULT NULL,
  `m2` char(100) DEFAULT NULL,
  `n1` binary(100) DEFAULT NULL,
  `n2` varbinary(100) DEFAULT NULL,
  `o1` binary(100) DEFAULT NULL,
  `o2` varbinary(100) DEFAULT NULL,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=DYNAMIC
DROP TABLE t1;
#
# Scenario 2:
# Create a small table, modify some columns instantly, along with
# virtual columns
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=DYNAMIC;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 ADD COLUMN c INT NOT NULL, ADD COLUMN d INT GENERATED ALWAYS AS ((b * 2)) VIRTUAL, ALGORITHM=copy;
Table ID differed
count(*) = 1
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN c BIGINT not null, ALGORITHM=INSTANT;
INSERT INTO t1(a, b, c) VALUES(0, 6, 20);
SELECT * FROM t1;
a	b	c	d
1	1	0	2
2	2	0	4
3	3	0	6
4	4	0	8
5	5	0	10
6	6	20	12
ALTER TABLE t1 ADD COLUMN e VARCHAR(100) DEFAULT 'ABCD EFGH', ADD COLUMN f INT GENERATED ALWAYS AS (LENGTH(e)), ALGORITHM=copy;
Table ID differed
count(*) = 1
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN e VARCHAR(150), ALGORITHM=INSTANT;
INSERT INTO t1(a, c, e) VALUES(0, 20, 'Hello'), (0, 20, 'World'), (0, 20, 'Hello World');
SELECT * FROM t1;
a	b	c	d	e	f
1	1	0	2	ABCD EFGH	9
2	2	0	4	ABCD EFGH	9
3	3	0	6	ABCD EFGH	9
4	4	0	8	ABCD EFGH	9
5	5	0	10	ABCD EFGH	9
6	6	20	12	ABCD EFGH	9
7	NULL	20	NULL	Hello	5
8	NULL	20	NULL	World	5
9	NULL	20	NULL	Hello World	11
ALTER TABLE t1 ADD COLUMN g VARCHAR(100) GENERATED ALWAYS AS (e), ADD COLUMN h BIGINT DEFAULT 10000, ADD COLUMN i BIGINT GENERATED ALWAYS AS (h * 2 + b), ALGORITHM=copy;
Table ID differed
count(*) = 1
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN g VARCHAR(150), ALGORITHM=INSTANT;
ERROR HY000: 'Changing the STORED status' is not supported for generated columns.
SELECT * FROM t1;
a	b	c	d	e	f	g	h	i
1	1	0	2	ABCD EFGH	9	ABCD EFGH	10000	20001
2	2	0	4	ABCD EFGH	9	ABCD EFGH	10000	20002
3	3	0	6	ABCD EFGH	9	ABCD EFGH	10000	20003
4	4	0	8	ABCD EFGH	9	ABCD EFGH	10000	20004
5	5	0	10	ABCD EFGH	9	ABCD EFGH	10000	20005
6	6	20	12	ABCD EFGH	9	ABCD EFGH	10000	20006
7	NULL	20	NULL	Hello	5	Hello	10000	NULL
8	NULL	20	NULL	World	5	World	10000	NULL
9	NULL	20	NULL	Hello World	11	Hello World	10000	NULL
INSERT INTO t1(a, b, c, h) VALUES(0, 7, 40, 2000), (0, 7, 40, 20000);
SELECT * FROM t1;
a	b	c	d	e	f	g	h	i
1	1	0	2	ABCD EFGH	9	ABCD EFGH	10000	20001
10	7	40	14	NULL	NULL	NULL	2000	4007
11	7	40	14	NULL	NULL	NULL	20000	40007
2	2	0	4	ABCD EFGH	9	ABCD EFGH	10000	20002
3	3	0	6	ABCD EFGH	9	ABCD EFGH	10000	20003
4	4	0	8	ABCD EFGH	9	ABCD EFGH	10000	20004
5	5	0	10	ABCD EFGH	9	ABCD EFGH	10000	20005
6	6	20	12	ABCD EFGH	9	ABCD EFGH	10000	20006
7	NULL	20	NULL	Hello	5	Hello	10000	NULL
8	NULL	20	NULL	World	5	World	10000	NULL
9	NULL	20	NULL	Hello World	11	Hello World	10000	NULL
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` int DEFAULT NULL,
  `c` bigint NOT NULL,
  `d` int GENERATED ALWAYS AS ((`b` * 2)) VIRTUAL,
  `e` varchar(150) DEFAULT NULL,
  `f` int GENERATED ALWAYS AS (length(`e`)) VIRTUAL,
  `g` varchar(100) GENERATED ALWAYS AS (`e`) VIRTUAL,
  `h` bigint DEFAULT '10000',
  `i` bigint GENERATED ALWAYS AS (((`h` * 2) + `b`)) VIRTUAL,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=DYNAMIC
DROP TABLE t1;
#
# Scenario 3:
# Create a small table, add some columns, then change
# their default values, check original default values are correct
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=DYNAMIC;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 ADD COLUMN c INT NOT NULL DEFAULT 100, ADD COLUMN d INT GENERATED ALWAYS AS ((c * 2 + b)) VIRTUAL, ADD COLUMN e VARCHAR(100) DEFAULT 'Hello world', ALGORITHM=copy;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN c BIGINT not null, ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN e VARCHAR(150), ALGORITHM=INSTANT;
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
INSERT INTO t1(a, b, c, e) VALUES(0, 6, 200, 'Good day'), (0, 7, 300, 'Good DAY');
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
ALTER TABLE t1 ALTER COLUMN c SET DEFAULT 500;
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
INSERT INTO t1(a, b) VALUES(0, 8), (0, 9);
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
8	8	500	1008	NULL
9	9	500	1009	NULL
ALTER TABLE t1 ALTER COLUMN e SET DEFAULT 'HELLO MySQL!';
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
8	8	500	1008	NULL
9	9	500	1009	NULL
INSERT INTO t1(a, b) VALUES(0, 10), (0, 20);
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
10	10	500	1010	HELLO MySQL!
11	20	500	1020	HELLO MySQL!
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
8	8	500	1008	NULL
9	9	500	1009	NULL
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` int DEFAULT NULL,
  `c` bigint NOT NULL DEFAULT '500',
  `d` int GENERATED ALWAYS AS (((`c` * 2) + `b`)) VIRTUAL,
  `e` varchar(150) DEFAULT 'HELLO MySQL!',
  PRIMARY KEY (`a`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=DYNAMIC
DROP TABLE t1;
#
# Scenario 4:
# Create a small table, add some columns instantly, then do DML
# on the table
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=DYNAMIC;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 ADD COLUMN c INT NOT NULL DEFAULT 100, ADD COLUMN d INT GENERATED ALWAYS AS ((c * 2 + b)) VIRTUAL, ADD COLUMN e VARCHAR(100) DEFAULT 'Hello world', ALGORITHM=copy;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN c BIGINT NOT NULL, ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN e VARCHAR(150), ALGORITHM=INSTANT;
UPDATE t1 SET c = 200 WHERE a > 3;
SELECT distinct(c) FROM t1;
c
100
200
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
DELETE FROM t1 WHERE c = 100;
UPDATE t1 SET c = 300;
SELECT distinct(c) FROM t1;
c
300
SELECT count(*) FROM t1;
count(*)
16
ALTER TABLE t1 ADD COLUMN t DATETIME DEFAULT CURRENT_TIMESTAMP, ALGORITHM=instant;
Table id did not change
count(*) = 1
1
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
UPDATE t1 SET e = 'Hello MySQL' WHERE a > 10;
UPDATE t1 SET e = 'Hello MySQL!!' WHERE a > 20;
SELECT distinct(e) FROM t1;
e
Hello MySQL
Hello MySQL!!
Hello world
UPDATE t1 SET c = 500 WHERE e LIKE '%world%';
SELECT c, e FROM t1 GROUP BY c, e;
c	e
300	Hello MySQL
300	Hello MySQL!!
500	Hello world
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
UPDATE t1 SET t = CURRENT_TIMESTAMP WHERE a < 50;
SELECT count(t) FROM t1 GROUP BY t;
count(t)
16
16
DELETE FROM t1 WHERE a < 50;
SELECT count(t) FROM t1 GROUP BY t;
count(t)
16
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` int DEFAULT NULL,
  `c` bigint NOT NULL,
  `d` int GENERATED ALWAYS AS (((`c` * 2) + `b`)) VIRTUAL,
  `e` varchar(150) DEFAULT NULL,
  `t` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB AUTO_INCREMENT=90 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=DYNAMIC
DROP TABLE t1;
#
# Scenario 5:
# Create a small table, add some columns and modify them instantly, then do DDL
# to build indexes
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=DYNAMIC;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 ADD COLUMN c INT NOT NULL DEFAULT 100, ADD COLUMN d INT GENERATED ALWAYS AS ((c * 2 + b)) VIRTUAL, ADD COLUMN e VARCHAR(100) DEFAULT 'Hello world', ALGORITHM=copy;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN c BIGINT NOT NULL, ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN e VARCHAR(150), ALGORITHM=INSTANT;
ALTER TABLE t1 ADD KEY(c);
EXPLAIN SELECT c FROM t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	index	NULL	c	#	NULL	#	100.00	Parallel scan (4 workers); Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	#	NULL	#	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	#	NULL	#	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`c` AS `c` from `test`.`t1`
SELECT c FROM t1 WHERE c != 100;
c
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
UPDATE t1 SET e = 'Hello MySQL' WHERE a > 30;
ALTER TABLE t1 ADD KEY(e);
EXPLAIN SELECT e FROM t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	index	NULL	e	#	NULL	#	100.00	Parallel scan (4 workers); Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	#	NULL	#	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	#	NULL	#	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`e` AS `e` from `test`.`t1`
SELECT count(e) FROM t1 WHERE e LIKE '%MySQL%';
count(e)
17
SELECT count(e) FROM t1 WHERE e LIKE '%world%';
count(e)
23
ALTER TABLE t1 DROP PRIMARY KEY, ADD PRIMARY KEY(a, c);
SELECT a, c, e FROM t1 WHERE a > 25 AND a < 40;
a	c	e
28	100	Hello world
29	100	Hello world
30	100	Hello world
31	100	Hello MySQL
32	100	Hello MySQL
33	100	Hello MySQL
34	100	Hello MySQL
35	100	Hello MySQL
36	100	Hello MySQL
37	100	Hello MySQL
38	100	Hello MySQL
39	100	Hello MySQL
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` int DEFAULT NULL,
  `c` bigint NOT NULL,
  `d` int GENERATED ALWAYS AS (((`c` * 2) + `b`)) VIRTUAL,
  `e` varchar(150) DEFAULT NULL,
  PRIMARY KEY (`a`,`c`),
  KEY `c` (`c`),
  KEY `e` (`e`)
) ENGINE=InnoDB AUTO_INCREMENT=59 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=DYNAMIC
DROP TABLE t1;
############################################
# Test instant MODIFY COLUMN for COMPACT format
############################################
#
# Scenario 1:
# Create a small table, and modify all kinds of columns and verify
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=COMPACT;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 MODIFY COLUMN a BIGINT, ALGORITHM=INSTANT;
ERROR 0A000: ALGORITHM=INSTANT is not supported. Reason: Cannot change column type INPLACE. Try ALGORITHM=COPY/INPLACE.
ALTER TABLE t1 MODIFY COLUMN b BIGINT, ALGORITHM=INSTANT;
INSERT INTO t1(a, b) VALUES(0, 1);
SELECT * FROM t1;
a	b
1	1
2	2
3	3
4	4
5	5
6	1
ALTER TABLE t1 ADD COLUMN c3 INT NOT NULL DEFAULT 10, ALGORITHM=INSTANT;
INSERT INTO t1(a, c3) VALUES(0, 11);
SELECT * FROM t1;
a	b	c3
1	1	10
2	2	10
3	3	10
4	4	10
5	5	10
6	1	10
7	NULL	11
ALTER TABLE t1 ADD COLUMN c4 CHAR(50), ADD COLUMN c5 VARCHAR(100), ALGORITHM=INSTANT;
INSERT INTO t1(a, c4, c5) VALUES(0, 'ABCD EFGH', 'abcdefg hijklmn ');
SELECT * FROM t1;
a	b	c3	c4	c5
1	1	10	NULL	NULL
2	2	10	NULL	NULL
3	3	10	NULL	NULL
4	4	10	NULL	NULL
5	5	10	NULL	NULL
6	1	10	NULL	NULL
7	NULL	11	NULL	NULL
8	NULL	10	ABCD EFGH	abcdefg hijklmn 
ALTER TABLE t1 MODIFY COLUMN c4 CHAR(100), ALGORITHM=INSTANT;
ERROR 0A000: ALGORITHM=INSTANT is not supported. Reason: Cannot change column type INPLACE. Try ALGORITHM=COPY/INPLACE.
INSERT INTO t1(a, c4, c5) VALUES(0, 'ABCD EFGH', 'abcdefg hijklmn ');
SELECT * FROM t1;
a	b	c3	c4	c5
1	1	10	NULL	NULL
2	2	10	NULL	NULL
3	3	10	NULL	NULL
4	4	10	NULL	NULL
5	5	10	NULL	NULL
6	1	10	NULL	NULL
7	NULL	11	NULL	NULL
8	NULL	10	ABCD EFGH	abcdefg hijklmn 
9	NULL	10	ABCD EFGH	abcdefg hijklmn 
ALTER TABLE t1 ADD COLUMN m1 CHAR(50) default 'The fox jumps over', ADD COLUMN m2 VARCHAR(50) DEFAULT 'The fox jumps over the lazy dog.', ALGORITHM=inplace;
Table ID differed
SELECT 2 = instant_cols AS `Instant columns equal` FROM information_schema.innodb_tables WHERE name like '%t1%';
Instant columns equal
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN m1 CHAR(100), ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN m2 CHAR(100), ALGORITHM=INSTANT;
INSERT INTO t1(a, m1, m2) VALUES(0, 'over the lazy dog', 'The lazy dog jumps over the fox.');
SELECT count(*) = max(a) FROM t1 WHERE m1 = 'The fox jumps over';
count(*) = max(a)
1
SELECT count(*) = max(a) FROM t1 WHERE m2 like 'The fox jumps%';
count(*) = max(a)
1
SELECT m1 FROM t1 WHERE m1 = 'over the lazy dog';
m1
over the lazy dog
SELECT m2 FROM t1 WHERE m2 like '%the fox.';
m2
The lazy dog jumps over the fox.
ALTER TABLE t1 ADD COLUMN n1 BINARY(10), ADD COLUMN n2 VARBINARY(10), ALGORITHM=inplace;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
INSERT INTO t1(a, n1, n2) VALUES(0, 0x010203040506070809, 0x102030405060708090);
ALTER TABLE t1 MODIFY COLUMN n1 BINARY(100), ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN n2 VARBINARY(100), ALGORITHM=INSTANT;
SELECT count(*) = max(a) FROM t1 WHERE n1 IS NULL;
count(*) = max(a)
1
SELECT count(*) = max(a) FROM t1 WHERE n2 IS NULL;
count(*) = max(a)
1
SELECT hex(n1) FROM t1 WHERE n1 = 0x01020304050607080900;
hex(n1)
SELECT hex(n2) FROM t1 WHERE n2 = 0x102030405060708090;
hex(n2)
102030405060708090
ALTER TABLE t1 ADD COLUMN o1 BINARY(10) DEFAULT 0x11223344, ADD COLUMN o2 VARBINARY(10) DEFAULT 0x55667788, ALGORITHM=inplace;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN o1 BINARY(100), ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN o2 VARBINARY(100), ALGORITHM=INSTANT;
INSERT INTO t1(a, o1, o2) VALUES(0, 0x44332211, 0x88776655);
SELECT count(*) = max(a) FROM t1 WHERE o1 = 0x11223344000000000000;
count(*) = max(a)
NULL
SELECT count(*) = max(a) FROM t1 WHERE o2 = 0x55667788;
count(*) = max(a)
1
SELECT hex(o1) FROM t1 WHERE o1 = 0x44332211000000000000;
hex(o1)
SELECT hex(o2) FROM t1 WHERE o2 = 0x88776655;
hex(o2)
88776655
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` bigint DEFAULT NULL,
  `c3` int NOT NULL DEFAULT '10',
  `c4` char(50) DEFAULT NULL,
  `c5` varchar(100) DEFAULT NULL,
  `m1` char(100) DEFAULT NULL,
  `m2` char(100) DEFAULT NULL,
  `n1` binary(100) DEFAULT NULL,
  `n2` varbinary(100) DEFAULT NULL,
  `o1` binary(100) DEFAULT NULL,
  `o2` varbinary(100) DEFAULT NULL,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=COMPACT
DROP TABLE t1;
#
# Scenario 2:
# Create a small table, modify some columns instantly, along with
# virtual columns
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=COMPACT;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 ADD COLUMN c INT NOT NULL, ADD COLUMN d INT GENERATED ALWAYS AS ((b * 2)) VIRTUAL, ALGORITHM=copy;
Table ID differed
count(*) = 1
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN c BIGINT not null, ALGORITHM=INSTANT;
INSERT INTO t1(a, b, c) VALUES(0, 6, 20);
SELECT * FROM t1;
a	b	c	d
1	1	0	2
2	2	0	4
3	3	0	6
4	4	0	8
5	5	0	10
6	6	20	12
ALTER TABLE t1 ADD COLUMN e VARCHAR(100) DEFAULT 'ABCD EFGH', ADD COLUMN f INT GENERATED ALWAYS AS (LENGTH(e)), ALGORITHM=copy;
Table ID differed
count(*) = 1
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN e VARCHAR(150), ALGORITHM=INSTANT;
INSERT INTO t1(a, c, e) VALUES(0, 20, 'Hello'), (0, 20, 'World'), (0, 20, 'Hello World');
SELECT * FROM t1;
a	b	c	d	e	f
1	1	0	2	ABCD EFGH	9
2	2	0	4	ABCD EFGH	9
3	3	0	6	ABCD EFGH	9
4	4	0	8	ABCD EFGH	9
5	5	0	10	ABCD EFGH	9
6	6	20	12	ABCD EFGH	9
7	NULL	20	NULL	Hello	5
8	NULL	20	NULL	World	5
9	NULL	20	NULL	Hello World	11
ALTER TABLE t1 ADD COLUMN g VARCHAR(100) GENERATED ALWAYS AS (e), ADD COLUMN h BIGINT DEFAULT 10000, ADD COLUMN i BIGINT GENERATED ALWAYS AS (h * 2 + b), ALGORITHM=copy;
Table ID differed
count(*) = 1
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN g VARCHAR(150), ALGORITHM=INSTANT;
ERROR HY000: 'Changing the STORED status' is not supported for generated columns.
SELECT * FROM t1;
a	b	c	d	e	f	g	h	i
1	1	0	2	ABCD EFGH	9	ABCD EFGH	10000	20001
2	2	0	4	ABCD EFGH	9	ABCD EFGH	10000	20002
3	3	0	6	ABCD EFGH	9	ABCD EFGH	10000	20003
4	4	0	8	ABCD EFGH	9	ABCD EFGH	10000	20004
5	5	0	10	ABCD EFGH	9	ABCD EFGH	10000	20005
6	6	20	12	ABCD EFGH	9	ABCD EFGH	10000	20006
7	NULL	20	NULL	Hello	5	Hello	10000	NULL
8	NULL	20	NULL	World	5	World	10000	NULL
9	NULL	20	NULL	Hello World	11	Hello World	10000	NULL
INSERT INTO t1(a, b, c, h) VALUES(0, 7, 40, 2000), (0, 7, 40, 20000);
SELECT * FROM t1;
a	b	c	d	e	f	g	h	i
1	1	0	2	ABCD EFGH	9	ABCD EFGH	10000	20001
10	7	40	14	NULL	NULL	NULL	2000	4007
11	7	40	14	NULL	NULL	NULL	20000	40007
2	2	0	4	ABCD EFGH	9	ABCD EFGH	10000	20002
3	3	0	6	ABCD EFGH	9	ABCD EFGH	10000	20003
4	4	0	8	ABCD EFGH	9	ABCD EFGH	10000	20004
5	5	0	10	ABCD EFGH	9	ABCD EFGH	10000	20005
6	6	20	12	ABCD EFGH	9	ABCD EFGH	10000	20006
7	NULL	20	NULL	Hello	5	Hello	10000	NULL
8	NULL	20	NULL	World	5	World	10000	NULL
9	NULL	20	NULL	Hello World	11	Hello World	10000	NULL
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` int DEFAULT NULL,
  `c` bigint NOT NULL,
  `d` int GENERATED ALWAYS AS ((`b` * 2)) VIRTUAL,
  `e` varchar(150) DEFAULT NULL,
  `f` int GENERATED ALWAYS AS (length(`e`)) VIRTUAL,
  `g` varchar(100) GENERATED ALWAYS AS (`e`) VIRTUAL,
  `h` bigint DEFAULT '10000',
  `i` bigint GENERATED ALWAYS AS (((`h` * 2) + `b`)) VIRTUAL,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=COMPACT
DROP TABLE t1;
#
# Scenario 3:
# Create a small table, add some columns, then change
# their default values, check original default values are correct
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=COMPACT;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 ADD COLUMN c INT NOT NULL DEFAULT 100, ADD COLUMN d INT GENERATED ALWAYS AS ((c * 2 + b)) VIRTUAL, ADD COLUMN e VARCHAR(100) DEFAULT 'Hello world', ALGORITHM=copy;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN c BIGINT not null, ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN e VARCHAR(150), ALGORITHM=INSTANT;
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
INSERT INTO t1(a, b, c, e) VALUES(0, 6, 200, 'Good day'), (0, 7, 300, 'Good DAY');
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
ALTER TABLE t1 ALTER COLUMN c SET DEFAULT 500;
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
INSERT INTO t1(a, b) VALUES(0, 8), (0, 9);
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
8	8	500	1008	NULL
9	9	500	1009	NULL
ALTER TABLE t1 ALTER COLUMN e SET DEFAULT 'HELLO MySQL!';
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
8	8	500	1008	NULL
9	9	500	1009	NULL
INSERT INTO t1(a, b) VALUES(0, 10), (0, 20);
SELECT * FROM t1;
a	b	c	d	e
1	1	100	201	Hello world
10	10	500	1010	HELLO MySQL!
11	20	500	1020	HELLO MySQL!
2	2	100	202	Hello world
3	3	100	203	Hello world
4	4	100	204	Hello world
5	5	100	205	Hello world
6	6	200	406	Good day
7	7	300	607	Good DAY
8	8	500	1008	NULL
9	9	500	1009	NULL
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` int DEFAULT NULL,
  `c` bigint NOT NULL DEFAULT '500',
  `d` int GENERATED ALWAYS AS (((`c` * 2) + `b`)) VIRTUAL,
  `e` varchar(150) DEFAULT 'HELLO MySQL!',
  PRIMARY KEY (`a`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=COMPACT
DROP TABLE t1;
#
# Scenario 4:
# Create a small table, add some columns instantly, then do DML
# on the table
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=COMPACT;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 ADD COLUMN c INT NOT NULL DEFAULT 100, ADD COLUMN d INT GENERATED ALWAYS AS ((c * 2 + b)) VIRTUAL, ADD COLUMN e VARCHAR(100) DEFAULT 'Hello world', ALGORITHM=copy;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN c BIGINT NOT NULL, ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN e VARCHAR(150), ALGORITHM=INSTANT;
UPDATE t1 SET c = 200 WHERE a > 3;
SELECT distinct(c) FROM t1;
c
100
200
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
DELETE FROM t1 WHERE c = 100;
UPDATE t1 SET c = 300;
SELECT distinct(c) FROM t1;
c
300
SELECT count(*) FROM t1;
count(*)
16
ALTER TABLE t1 ADD COLUMN t DATETIME DEFAULT CURRENT_TIMESTAMP, ALGORITHM=instant;
Table id did not change
count(*) = 1
1
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
UPDATE t1 SET e = 'Hello MySQL' WHERE a > 10;
UPDATE t1 SET e = 'Hello MySQL!!' WHERE a > 20;
SELECT distinct(e) FROM t1;
e
Hello MySQL
Hello MySQL!!
Hello world
UPDATE t1 SET c = 500 WHERE e LIKE '%world%';
SELECT c, e FROM t1 GROUP BY c, e;
c	e
300	Hello MySQL
300	Hello MySQL!!
500	Hello world
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
UPDATE t1 SET t = CURRENT_TIMESTAMP WHERE a < 50;
SELECT count(t) FROM t1 GROUP BY t;
count(t)
16
16
DELETE FROM t1 WHERE a < 50;
SELECT count(t) FROM t1 GROUP BY t;
count(t)
16
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` int DEFAULT NULL,
  `c` bigint NOT NULL,
  `d` int GENERATED ALWAYS AS (((`c` * 2) + `b`)) VIRTUAL,
  `e` varchar(150) DEFAULT NULL,
  `t` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB AUTO_INCREMENT=90 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=COMPACT
DROP TABLE t1;
#
# Scenario 5:
# Create a small table, add some columns and modify them instantly, then do DDL
# to build indexes
#
CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=COMPACT;
INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);
ALTER TABLE t1 ADD COLUMN c INT NOT NULL DEFAULT 100, ADD COLUMN d INT GENERATED ALWAYS AS ((c * 2 + b)) VIRTUAL, ADD COLUMN e VARCHAR(100) DEFAULT 'Hello world', ALGORITHM=copy;
Table ID differed
count(*) = 2
0
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
ALTER TABLE t1 MODIFY COLUMN c BIGINT NOT NULL, ALGORITHM=INSTANT;
ALTER TABLE t1 MODIFY COLUMN e VARCHAR(150), ALGORITHM=INSTANT;
ALTER TABLE t1 ADD KEY(c);
EXPLAIN SELECT c FROM t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	index	NULL	c	#	NULL	#	100.00	Parallel scan (4 workers); Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	#	NULL	#	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	#	NULL	#	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`c` AS `c` from `test`.`t1`
SELECT c FROM t1 WHERE c != 100;
c
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
INSERT INTO t1(b, c, e) SELECT b, c, e FROM t1;
UPDATE t1 SET e = 'Hello MySQL' WHERE a > 30;
ALTER TABLE t1 ADD KEY(e);
EXPLAIN SELECT e FROM t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	index	NULL	e	#	NULL	#	100.00	Parallel scan (4 workers); Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	#	NULL	#	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	#	NULL	#	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`e` AS `e` from `test`.`t1`
SELECT count(e) FROM t1 WHERE e LIKE '%MySQL%';
count(e)
17
SELECT count(e) FROM t1 WHERE e LIKE '%world%';
count(e)
23
ALTER TABLE t1 DROP PRIMARY KEY, ADD PRIMARY KEY(a, c);
SELECT a, c, e FROM t1 WHERE a > 25 AND a < 40;
a	c	e
28	100	Hello world
29	100	Hello world
30	100	Hello world
31	100	Hello MySQL
32	100	Hello MySQL
33	100	Hello MySQL
34	100	Hello MySQL
35	100	Hello MySQL
36	100	Hello MySQL
37	100	Hello MySQL
38	100	Hello MySQL
39	100	Hello MySQL
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` int DEFAULT NULL,
  `c` bigint NOT NULL,
  `d` int GENERATED ALWAYS AS (((`c` * 2) + `b`)) VIRTUAL,
  `e` varchar(150) DEFAULT NULL,
  PRIMARY KEY (`a`,`c`),
  KEY `c` (`c`),
  KEY `e` (`e`)
) ENGINE=InnoDB AUTO_INCREMENT=59 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=COMPACT
DROP TABLE t1;

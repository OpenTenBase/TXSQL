DROP DATABASE IF EXISTS dbtest_seq_sta;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_sta.sequence_000;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_sta.sequence_001;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_sta.sequence_002;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_sta.sequence_003;
CREATE DATABASE dbtest_seq_sta;
use dbtest_seq_sta;
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 1000 TDSQL_minvalue 100;
CREATE TABLE t0(a int PRIMARY KEY);
SELECT * from mysql.tdsql_sequences where db='dbtest_seq_sta' and name='sequence_000';
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
dbtest_seq_sta	sequence_000	-9223372036854775808	100	1	1000	100	0	20
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT * FROM t0 order by a;
a
100
101
102
103
104
105
106
107
108
109
commit;
DROP TDSQL_SEQUENCE sequence_000 ;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_increment by 1 start with 10;
CREATE TABLE t1(a int PRIMARY KEY);
SELECT * from mysql.tdsql_sequences where db='dbtest_seq_sta' and name='sequence_001';
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
dbtest_seq_sta	sequence_001	-9223372036854775808	10	1	9223372036854775807	1	0	20
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
SELECT * FROM t1 order by a;
a
10
11
12
13
14
15
16
17
18
19
commit;
DROP TDSQL_SEQUENCE sequence_001 ;
CREATE TDSQL_SEQUENCE sequence_002 TDSQL_increment by -1 TDSQL_MAXVALUE -10 TDSQL_minvalue -1000;
SELECT * from mysql.tdsql_sequences where db='dbtest_seq_sta' and  name='sequence_002';
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
dbtest_seq_sta	sequence_002	-9223372036854775808	-10	-1	-10	-1000	0	20
CREATE TABLE t2(a int PRIMARY KEY);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
SELECT * FROM t2 order by a;
a
-19
-18
-17
-16
-15
-14
-13
-12
-11
-10
commit;
DROP TDSQL_SEQUENCE sequence_002 ;
CREATE TDSQL_SEQUENCE sequence_003 TDSQL_increment by -10 start with -100 TDSQL_MAXVALUE -10 TDSQL_minvalue -1000 ;
SELECT * from mysql.tdsql_sequences where db='dbtest_seq_sta' and name='sequence_003';
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
dbtest_seq_sta	sequence_003	-9223372036854775808	-100	-10	-10	-1000	0	20
CREATE TABLE t3(a int PRIMARY KEY);
INSERT INTO t3 values(sequence_003.nextval);
INSERT INTO t3 values(sequence_003.nextval);
INSERT INTO t3 values(sequence_003.nextval);
INSERT INTO t3 values(sequence_003.nextval);
INSERT INTO t3 values(sequence_003.nextval);
INSERT INTO t3 values(sequence_003.nextval);
INSERT INTO t3 values(sequence_003.nextval);
INSERT INTO t3 values(sequence_003.nextval);
INSERT INTO t3 values(sequence_003.nextval);
INSERT INTO t3 values(sequence_003.nextval);
SELECT * FROM t3 order by a;
a
-190
-180
-170
-160
-150
-140
-130
-120
-110
-100
commit;
DROP TDSQL_SEQUENCE sequence_003 ;
DROP DATABASE dbtest_seq_sta ;

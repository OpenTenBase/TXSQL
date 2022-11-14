DROP DATABASE IF EXISTS dbtest_seq_cyc_bug;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_cyc_bug.sequence_000;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_cyc_bug.sequence_001;
CREATE DATABASE dbtest_seq_cyc_bug;
use dbtest_seq_cyc_bug;
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_increment by -1 TDSQL_MAXVALUE 50 TDSQL_minvalue 0 TDSQL_nocycle;
CREATE TABLE t0(a int PRIMARY KEY);
SELECT * from mysql.tdsql_sequences where db='dbtest_seq_cyc_bug' and  name='sequence_000';
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
dbtest_seq_cyc_bug	sequence_000	-9223372036854775808	50	-1	50	0	0	20
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
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
ERROR HY000: Sequence sequence_000 (step -1) in database dbtest_seq_cyc_bug is not cyclic and has reached its min/max value 0.
SELECT * FROM t0 order by a;
a
0
1
2
3
4
5
6
7
8
9
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
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
commit;
DROP TDSQL_SEQUENCE sequence_000 ;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_increment by -1 TDSQL_MAXVALUE 50 TDSQL_minvalue 0 TDSQL_cycle;
CREATE TABLE t1(a int PRIMARY KEY);
SELECT * from mysql.tdsql_sequences where db='dbtest_seq_cyc_bug' and name='sequence_001';
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
dbtest_seq_cyc_bug	sequence_001	-9223372036854775808	50	-1	50	0	1	20
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
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
ERROR 23000: Duplicate entry '50' for key 't1.PRIMARY'
SELECT * FROM t1 order by a;
a
0
1
2
3
4
5
6
7
8
9
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
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
SELECT sequence_001.nextval;
sequence_001.nextval
49
SELECT sequence_001.nextval;
sequence_001.nextval
48
SELECT sequence_001.nextval;
sequence_001.nextval
47
commit;
DROP TDSQL_SEQUENCE sequence_001 ;
DROP DATABASE dbtest_seq_cyc_bug ;

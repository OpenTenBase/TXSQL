DROP DATABASE IF EXISTS dbtest_seq_alter_max;
commit;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_max.sequence_000;
commit;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_max.sequence_001;
commit;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_max.sequence_002;
CREATE DATABASE dbtest_seq_alter_max;
use dbtest_seq_alter_max;
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_increment by 1 TDSQL_MAXVALUE 5 TDSQL_minvalue 1 TDSQL_nocache;
CREATE TABLE t0(a int PRIMARY KEY);
SELECT curval,max_value from mysql.tdsql_sequences where db='dbtest_seq_alter_max' and name='sequence_000';
curval	max_value
-9223372036854775808	5
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
ERROR HY000: Sequence sequence_000 (step 1) in database dbtest_seq_alter_max is not cyclic and has reached its min/max value 5.
SELECT * FROM t0 order by a;
a
1
2
3
4
5
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 10;
SELECT curval,max_value from mysql.tdsql_sequences where db='dbtest_seq_alter_max' and name='sequence_000';
curval	max_value
5	10
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT * FROM t0 order by a;
a
1
2
3
4
5
6
7
8
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 100;
SELECT curval,max_value from mysql.tdsql_sequences where db='dbtest_seq_alter_max' and name='sequence_000';
curval	max_value
8	100
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT * FROM t0 order by a;
a
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
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_nomaxvalue;
SELECT curval,max_value from mysql.tdsql_sequences where db='dbtest_seq_alter_max' and name='sequence_000';
curval	max_value
11	9223372036854775807
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
commit;
SELECT * FROM t0 order by a;
a
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
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 10;
ERROR HY000: Wrong sequence argument(max/min) value specified: current sequence value must be in [min,max].
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 0;
ERROR HY000: Wrong sequence argument(maxvalue and minvalue) value specified: minvalue < maxvalue must be true..
commit;
DROP TDSQL_SEQUENCE sequence_000 ;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_increment by -1 TDSQL_MAXVALUE 1000  TDSQL_minvalue -1000 TDSQL_nocache;
CREATE TABLE t1(a int PRIMARY KEY);
SELECT curval,max_value from mysql.tdsql_sequences where db='dbtest_seq_alter_max' and name='sequence_001';
curval	max_value
-9223372036854775808	1000
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
SELECT * FROM t1 order by a;
a
995
996
997
998
999
1000
commit;
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_MAXVALUE 2000;
SELECT curval,max_value from mysql.tdsql_sequences where db='dbtest_seq_alter_max' and name='sequence_001';
curval	max_value
995	2000
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
SELECT * FROM t1 order by a;
a
992
993
994
995
996
997
998
999
1000
commit;
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_nomaxvalue;
ERROR HY000: Wrong sequence argument(start) value specified: start must be in [minvalue, maxvalue].
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_MAXVALUE 500;
ERROR HY000: Wrong sequence argument(start) value specified: start must be in [minvalue, maxvalue].
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_MAXVALUE -2000;
ERROR HY000: Wrong sequence argument(maxvalue and minvalue) value specified: minvalue < maxvalue must be true..
commit;
DROP TDSQL_SEQUENCE sequence_001 ;
CREATE TDSQL_SEQUENCE sequence_002 TDSQL_increment by -1 start with -100 TDSQL_MAXVALUE 0 TDSQL_minvalue -1000 TDSQL_nocache;
CREATE TABLE t2(a int PRIMARY KEY);
SELECT curval,max_value from mysql.tdsql_sequences where db='dbtest_seq_alter_max' and name='sequence_002';
curval	max_value
-9223372036854775808	0
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
SELECT * FROM t2 order by a;
a
-102
-101
-100
commit;
ALTER TDSQL_SEQUENCE sequence_002 TDSQL_MAXVALUE 100;
SELECT curval,max_value from mysql.tdsql_sequences where db='dbtest_seq_alter_max' and name='sequence_002';
curval	max_value
-102	100
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
commit;
ALTER TDSQL_SEQUENCE sequence_002 TDSQL_nomaxvalue;
SELECT curval,max_value from mysql.tdsql_sequences where db='dbtest_seq_alter_max' and name='sequence_002';
curval	max_value
-105	-1
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
INSERT INTO t2 values(sequence_002.nextval);
SELECT * FROM t2 order by a;
a
-108
-107
-106
-105
-104
-103
-102
-101
-100
commit;
DROP TDSQL_SEQUENCE sequence_002 ;
DROP DATABASE dbtest_seq_alter_max  ;

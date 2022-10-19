DROP DATABASE IF EXISTS dbtest_seq_alter_min;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_min.sequence_001;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_min.sequence_002;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_min.sequence_003;
CREATE DATABASE dbtest_seq_alter_min;
USE dbtest_seq_alter_min;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_increment by 1 start with 1 TDSQL_MAXVALUE 1000 TDSQL_minvalue 0;
SELECT curval,min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_alter_min' and name='sequence_001';
curval	min_value
-9223372036854775808	0
CREATE TABLE t0(a int primary key);
INSERT INTO t0 values(sequence_001.nextval);
INSERT INTO t0 values(sequence_001.nextval);
INSERT INTO t0 values(sequence_001.nextval);
SELECT * FROM t0 order by a;
a
1
2
3
commit;
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_minvalue -100;
SELECT curval,min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_alter_min' and name='sequence_001';
curval	min_value
20	-100
INSERT INTO t0 values(sequence_001.nextval);
INSERT INTO t0 values(sequence_001.nextval);
INSERT INTO t0 values(sequence_001.nextval);
SELECT * FROM t0 order by a;
a
1
2
3
4
5
6
commit;
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_minvalue -1000;
SELECT curval,min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_alter_min' and name='sequence_001';
curval	min_value
20	-1000
INSERT INTO t0 values(sequence_001.nextval);
INSERT INTO t0 values(sequence_001.nextval);
INSERT INTO t0 values(sequence_001.nextval);
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
commit;
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_nominvalue;
SELECT curval,min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_alter_min' and name='sequence_001';
curval	min_value
20	1
INSERT INTO t0 values(sequence_001.nextval);
INSERT INTO t0 values(sequence_001.nextval);
INSERT INTO t0 values(sequence_001.nextval);
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
commit;
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_minvalue 10000;
ERROR HY000: Wrong sequence argument(maxvalue and minvalue) value specified: minvalue < maxvalue must be true..
commit;
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_minvalue 10;
ERROR HY000: Wrong sequence argument(start) value specified: start must be in [minvalue, maxvalue].
DROP TDSQL_SEQUENCE sequence_001;
CREATE TDSQL_SEQUENCE sequence_002 TDSQL_increment by 1 start with 0 TDSQL_MAXVALUE 1000 TDSQL_minvalue -1000;
SELECT curval,min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_alter_min' and name='sequence_002';
curval	min_value
-9223372036854775808	-1000
commit;
ALTER TDSQL_SEQUENCE sequence_002 TDSQL_nominvalue;
ERROR HY000: Wrong sequence argument(start) value specified: start must be in [minvalue, maxvalue].
DROP TDSQL_SEQUENCE sequence_002;
CREATE TDSQL_SEQUENCE sequence_003 TDSQL_increment by -1 TDSQL_MAXVALUE -1 TDSQL_minvalue -1000 TDSQL_nocache;
SELECT curval,min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_alter_min' and name='sequence_003';
curval	min_value
-9223372036854775808	-1000
CREATE TABLE t1(a int primary key);
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
SELECT * FROM t1 order by a;
a
-3
-2
-1
commit;
ALTER TDSQL_SEQUENCE sequence_003 TDSQL_minvalue -10000;
SELECT curval,min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_alter_min' and name='sequence_003';
curval	min_value
-3	-10000
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
SELECT * FROM t1 order by a;
a
-6
-5
-4
-3
-2
-1
commit;
ALTER TDSQL_SEQUENCE sequence_003 TDSQL_minvalue -10;
SELECT curval,min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_alter_min' and name='sequence_003';
curval	min_value
-6	-10
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
ERROR HY000: Sequence sequence_003 (step -1) in database dbtest_seq_alter_min is not cyclic and has reached its min/max value -10.
SELECT * FROM t1 order by a;
a
-10
-9
-8
-7
-6
-5
-4
-3
-2
-1
commit;
ALTER TDSQL_SEQUENCE sequence_003 TDSQL_nominvalue;
SELECT curval,min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_alter_min' and name='sequence_003';
curval	min_value
-10	-9223372036854775808
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
INSERT INTO t1 values(sequence_003.nextval);
SELECT * FROM t1 order by a;
a
-13
-12
-11
-10
-9
-8
-7
-6
-5
-4
-3
-2
-1
commit;
ALTER TDSQL_SEQUENCE sequence_003 TDSQL_minvalue -1;
ERROR HY000: Wrong sequence argument(maxvalue and minvalue) value specified: minvalue < maxvalue must be true..
commit;
DROP TDSQL_SEQUENCE sequence_003;
DROP DATABASE dbtest_seq_alter_min  ;

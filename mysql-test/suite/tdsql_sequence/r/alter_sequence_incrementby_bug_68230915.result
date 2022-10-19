DROP DATABASE IF EXISTS dbtest_seq_alter_inc_bug;
commit;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_inc_bug.sequence_000;
commit;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_inc_bug.sequence_001;
CREATE DATABASE dbtest_seq_alter_inc_bug;
use dbtest_seq_alter_inc_bug;
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_increment by 1 TDSQL_MAXVALUE 1000 TDSQL_minvalue 1 TDSQL_nocache;
CREATE TABLE t0(a int PRIMARY KEY);
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc_bug' and name='sequence_000';
step
1
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT * FROM t0 order by a;
a
1
2
3
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_increment by -1;
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc_bug' and name='sequence_000';
step
-1
INSERT INTO t0 values(sequence_000.nextval);
ERROR 23000: Duplicate entry '2' for key 't0.PRIMARY'
SELECT * FROM t0 order by a;
a
1
2
3
commit;
DROP TDSQL_SEQUENCE sequence_000 ;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_increment by -1 TDSQL_MAXVALUE 1000 TDSQL_minvalue 1 TDSQL_nocache;
CREATE TABLE t1(a int PRIMARY KEY);
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc_bug' and name='sequence_001';
step
-1
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
SELECT * FROM t1 order by a;
a
998
999
1000
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_increment by -2;
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc_bug' and name='sequence_001';
step
-2
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
SELECT * FROM t1 order by a;
a
992
994
996
998
999
1000
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_increment by 10;
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc_bug' and name='sequence_001';
step
10
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
ERROR HY000: Sequence sequence_001 (step 10) in database dbtest_seq_alter_inc_bug is not cyclic and has reached its min/max value 1000.
SELECT * FROM t1 order by a;
a
992
994
996
998
999
1000
1002
commit;
DROP TDSQL_SEQUENCE sequence_001 ;
DROP DATABASE dbtest_seq_alter_inc_bug  ;

DROP DATABASE IF EXISTS dbtest_seq_min_bug;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_min_bug.sequence_003;
CREATE DATABASE dbtest_seq_min_bug;
USE dbtest_seq_min_bug;
CREATE TDSQL_SEQUENCE sequence_003 TDSQL_increment by -100 start with -1  TDSQL_minvalue -201 TDSQL_nocache;
CREATE TABLE t0(a int PRIMARY KEY);
INSERT INTO t0 values(sequence_003.nextval);
INSERT INTO t0 values(sequence_003.nextval);
INSERT INTO t0 values(sequence_003.nextval);
INSERT INTO t0 values(sequence_003.nextval);
ERROR HY000: Sequence sequence_003 (step -100) in database dbtest_seq_min_bug is not cyclic and has reached its min/max value -201.
commit;
DROP TDSQL_SEQUENCE sequence_003;
DROP DATABASE dbtest_seq_min_bug ;

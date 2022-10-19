DROP DATABASE IF EXISTS dbtest_seq_max;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_max.sequence_001;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_max.sequence_002;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_max.sequence_003;
CREATE DATABASE dbtest_seq_max;
USE dbtest_seq_max;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_increment by 1 TDSQL_nomaxvalue;
CREATE TDSQL_SEQUENCE sequence_002 TDSQL_increment by -1 TDSQL_nomaxvalue;
SELECT max_value FROM  mysql.tdsql_sequences where db='dbtest_seq_max' and name='sequence_001';
max_value
9223372036854775807
SELECT max_value FROM  mysql.tdsql_sequences where db='dbtest_seq_max' and name='sequence_002';
max_value
-1
DROP TDSQL_SEQUENCE sequence_001;
DROP TDSQL_SEQUENCE sequence_002;
CREATE TDSQL_SEQUENCE sequence_003 start with 9 TDSQL_MAXVALUE 10 TDSQL_nocache;
CREATE TABLE t0(a int PRIMARY KEY);
INSERT INTO t0 values(sequence_003.nextval);
INSERT INTO t0 values(sequence_003.nextval);
INSERT INTO t0 values(sequence_003.nextval);
ERROR HY000: Sequence sequence_003 (step 1) in database dbtest_seq_max is not cyclic and has reached its min/max value 10.
commit;
DROP TDSQL_SEQUENCE sequence_003;
CREATE TDSQL_SEQUENCE sequence_004 TDSQL_increment by 1 TDSQL_MAXVALUE 99999999999999999999999999999;
ERROR 42000: Only integers allowed as number here near '99999999999999999999999999999' at line 1
CREATE TDSQL_SEQUENCE sequence_005 TDSQL_increment by 50 TDSQL_MAXVALUE '99';
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near ''99'' at line 1
DROP DATABASE dbtest_seq_max ;

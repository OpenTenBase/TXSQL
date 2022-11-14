DROP DATABASE IF EXISTS dbtest_seq_err;
CREATE DATABASE dbtest_seq_err;
USE dbtest_seq_err;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_increment by 0;
ERROR HY000: Wrong sequence argument(step) value specified: step != 0.
CREATE TDSQL_SEQUENCE sequence_002 TDSQL_increment by 1000 TDSQL_MAXVALUE 100 TDSQL_minvalue 10;
ERROR HY000: Wrong sequence argument(step) value specified: step < maxvalue-minvalue must be true..
CREATE TDSQL_SEQUENCE sequence_003 TDSQL_increment by '10';
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near ''10'' at line 1
CREATE TDSQL_SEQUENCE sequence_004 TDSQL_increment by 999999999999999999999;
ERROR 42000: Only integers allowed as number here near '999999999999999999999' at line 1
DROP DATABASE dbtest_seq_err ;

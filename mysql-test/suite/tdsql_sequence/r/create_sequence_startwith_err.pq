DROP DATABASE IF EXISTS dbtest_seq_sta_err;
CREATE DATABASE dbtest_seq_sta_err;
USE dbtest_seq_sta_err;
CREATE TDSQL_SEQUENCE sequence_001 start with 1000 TDSQL_MAXVALUE 100;
ERROR HY000: Wrong sequence argument(start) value specified: start must be in [minvalue, maxvalue].
CREATE TDSQL_SEQUENCE sequence_002 TDSQL_increment by 1 start with 1  TDSQL_minvalue 10;
ERROR HY000: Wrong sequence argument(start) value specified: start must be in [minvalue, maxvalue].
CREATE TDSQL_SEQUENCE sequence_003 TDSQL_increment by 50 start with 60 TDSQL_MAXVALUE 100 TDSQL_nocache;
ERROR HY000: Wrong sequence argument(cache) value specified: cache < (maxvalue - minvalue)/abs(step).
CREATE TDSQL_SEQUENCE sequence_004 TDSQL_increment by -1 start with 1000 TDSQL_MAXVALUE -10;
ERROR HY000: Wrong sequence argument(start) value specified: start must be in [minvalue, maxvalue].
CREATE TDSQL_SEQUENCE sequence_005 TDSQL_increment by -1 start with -1000 TDSQL_minvalue -100;
ERROR HY000: Wrong sequence argument(start) value specified: start must be in [minvalue, maxvalue].
CREATE TDSQL_SEQUENCE sequence_006 TDSQL_increment by -50 start with -2 TDSQL_MAXVALUE -1 TDSQL_minvalue -40 TDSQL_nocache;
ERROR HY000: Wrong sequence argument(step) value specified: step < maxvalue-minvalue must be true..
CREATE TDSQL_SEQUENCE sequence_007 TDSQL_increment by 50 startwith '6';
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'startwith '6'' at line 1
DROP DATABASE dbtest_seq_sta_err ;

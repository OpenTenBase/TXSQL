DROP DATABASE IF EXISTS dbtest_seq_cac_err;
CREATE DATABASE dbtest_seq_cac_err;
use dbtest_seq_cac_err;
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 1000 TDSQL_minvalue 100 TDSQL_CACHE 10000;
ERROR HY000: Wrong sequence argument(cache) value specified: cache < (maxvalue - minvalue)/abs(step).
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 1000 TDSQL_minvalue 100 TDSQL_CACHE 1;
ERROR HY000: Wrong sequence argument(CACHE) value specified: In 'CACHE N' clause, N >= 2.
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 1000 TDSQL_minvalue 100 TDSQL_CACHE 0;
ERROR HY000: Wrong sequence argument(CACHE) value specified: In 'CACHE N' clause, N >= 2.
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 1000 TDSQL_minvalue 100 TDSQL_CACHE -10;
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near '-10' at line 1
commit;
DROP DATABASE dbtest_seq_cac_err ;

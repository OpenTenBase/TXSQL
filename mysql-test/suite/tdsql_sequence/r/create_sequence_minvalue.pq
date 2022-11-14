DROP DATABASE IF EXISTS dbtest_seq_min;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_min.sequence_001;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_min.sequence_002;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_min.sequence_003;
CREATE DATABASE dbtest_seq_min;
USE dbtest_seq_min;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_increment by 1 TDSQL_nominvalue;
CREATE TDSQL_SEQUENCE sequence_002 TDSQL_increment by -1 TDSQL_nominvalue;
SELECT min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_min' and name='sequence_001';
min_value
1
SELECT min_value FROM  mysql.tdsql_sequences where db='dbtest_seq_min' and name='sequence_002';
min_value
-9223372036854775808
commit;
DROP TDSQL_SEQUENCE sequence_001;
DROP TDSQL_SEQUENCE sequence_002;
CREATE TDSQL_SEQUENCE sequence_004 TDSQL_increment by 1 TDSQL_minvalue 99999999999999999999999999999;
ERROR 42000: Only integers allowed as number here near '99999999999999999999999999999' at line 1
CREATE TDSQL_SEQUENCE sequence_005 TDSQL_increment by 50 TDSQL_minvalue '99';
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near ''99'' at line 1
DROP DATABASE dbtest_seq_min ;

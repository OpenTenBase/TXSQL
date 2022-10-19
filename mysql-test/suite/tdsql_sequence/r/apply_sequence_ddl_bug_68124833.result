DROP DATABASE IF EXISTS dbtest_seq_ddl_bug;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_ddl_bug.sequence_000;
CREATE DATABASE dbtest_seq_ddl_bug;
CREATE TABLE dbtest_seq_ddl_bug.t0(a int(64) PRIMARY KEY, b int(64) ,c char(64));
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
use dbtest_seq_ddl_bug;
CREATE TDSQL_SEQUENCE IF NOT EXISTS sequence_000 TDSQL_minvalue 1 TDSQL_nocache;
SELECT * FROM mysql.tdsql_sequences WHERE db='dbtest_seq_ddl_bug';
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
dbtest_seq_ddl_bug	sequence_000	-9223372036854775808	1	1	9223372036854775807	1	0	1
INSERT INTO dbtest_seq_ddl_bug.t0(a, b, c) values(sequence_000.nextval, 10000, '57babb2c175128a0683b902b1364e7cb');
commit;
DROP TDSQL_SEQUENCE dbtest_seq_ddl_bug.sequence_000;
INSERT INTO dbtest_seq_ddl_bug.t0(a, b, c) values(sequence_000.nextval, RAND() * 10000, SUBSTRING(MD5(RAND()),1,32));
ERROR HY000: Sequence sequence_000 in database dbtest_seq_ddl_bug not found.
commit;
DROP DATABASE dbtest_seq_ddl_bug ;

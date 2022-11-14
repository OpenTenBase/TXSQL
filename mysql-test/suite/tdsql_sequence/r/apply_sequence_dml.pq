DROP DATABASE IF EXISTS dbtest_seq_dml;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_dml.sequence_000;
CREATE DATABASE dbtest_seq_dml;
CREATE TABLE dbtest_seq_dml.t0(a int(64) PRIMARY KEY, b int(64) ,c char(64));
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
CREATE TDSQL_SEQUENCE dbtest_seq_dml.sequence_000 TDSQL_minvalue 1 TDSQL_nocache;
SELECT * FROM mysql.tdsql_sequences WHERE db='dbtest_seq_dml';
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
dbtest_seq_dml	sequence_000	-9223372036854775808	1	1	9223372036854775807	1	0	1
USE dbtest_seq_dml;
INSERT INTO dbtest_seq_dml.t0(a, b, c) values(dbtest_seq_dml.sequence_000.nextval, sequence_000.nextval, '57babb2c175128a0683b902b1364e7cb');
INSERT INTO dbtest_seq_dml.t0 set a=dbtest_seq_dml.sequence_000.nextval, b=dbtest_seq_dml.sequence_000.nextval, c='57babb2c175128a0683b902b1364e7cb';
INSERT INTO dbtest_seq_dml.t0 set a=dbtest_seq_dml.sequence_000.nextval, b=dbtest_seq_dml.sequence_000.currval, c='57babb2c175128a0683b902b1364e7cb';
UPDATE dbtest_seq_dml.t0 SET b=dbtest_seq_dml.sequence_000.nextval where b=dbtest_seq_dml.sequence_000.currval;
SELECT * FROM dbtest_seq_dml.t0 order by a;
a	b	c
1	2	57babb2c175128a0683b902b1364e7cb
3	4	57babb2c175128a0683b902b1364e7cb
5	6	57babb2c175128a0683b902b1364e7cb
SELECT sequence_000.nextval;
sequence_000.nextval
7
SELECT sequence_000.nextval;
sequence_000.nextval
8
SELECT sequence_000.nextval;
sequence_000.nextval
9
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_INCREMENT BY 10;
SELECT sequence_000.nextval;
sequence_000.nextval
19
SELECT sequence_000.nextval;
sequence_000.nextval
29
SELECT sequence_000.nextval;
sequence_000.nextval
39
INSERT INTO dbtest_seq_dml.t0(a, b, c) values(dbtest_seq_dml.sequence_000.currval, sequence_000.nextval, '57babb2c175128a0683b902b1364e7cb');
INSERT INTO dbtest_seq_dml.t0(a, b, c) values(dbtest_seq_dml.sequence_000.currval, dbtest_seq_dml.sequence_000.nextval, '57babb2c175128a0683b902b1364e7cb');
INSERT INTO dbtest_seq_dml.t0(a, b, c) values(sequence_000.currval, sequence_000.nextval, '57babb2c175128a0683b902b1364e7cb');
INSERT INTO dbtest_seq_dml.t0 SET a=dbtest_seq_dml.sequence_000.currval, b=sequence_000.currval, c='57babb2c175128a0683b902b1364e7cb';
SELECT * FROM dbtest_seq_dml.t0 ORDER BY a;
a	b	c
1	2	57babb2c175128a0683b902b1364e7cb
3	4	57babb2c175128a0683b902b1364e7cb
5	6	57babb2c175128a0683b902b1364e7cb
39	49	57babb2c175128a0683b902b1364e7cb
49	59	57babb2c175128a0683b902b1364e7cb
59	69	57babb2c175128a0683b902b1364e7cb
69	69	57babb2c175128a0683b902b1364e7cb
SELECT sequence_000.currval;
sequence_000.currval
69
SELECT sequence_000.currval;
sequence_000.currval
69
SELECT sequence_000.currval;
sequence_000.currval
69
UPDATE dbtest_seq_dml.t0 SET b=dbtest_seq_dml.sequence_000.currval;
SELECT * FROM dbtest_seq_dml.t0 ORDER BY a;
a	b	c
1	69	57babb2c175128a0683b902b1364e7cb
3	69	57babb2c175128a0683b902b1364e7cb
5	69	57babb2c175128a0683b902b1364e7cb
39	69	57babb2c175128a0683b902b1364e7cb
49	69	57babb2c175128a0683b902b1364e7cb
59	69	57babb2c175128a0683b902b1364e7cb
69	69	57babb2c175128a0683b902b1364e7cb
DELETE FROM dbtest_seq_dml.t0 WHERE a=dbtest_seq_dml.sequence_000.currval;
SELECT * FROM dbtest_seq_dml.t0 ORDER BY a;
a	b	c
1	69	57babb2c175128a0683b902b1364e7cb
3	69	57babb2c175128a0683b902b1364e7cb
5	69	57babb2c175128a0683b902b1364e7cb
39	69	57babb2c175128a0683b902b1364e7cb
49	69	57babb2c175128a0683b902b1364e7cb
59	69	57babb2c175128a0683b902b1364e7cb
INSERT INTO dbtest_seq_dml.t0 SET a=sequence_not_exist.nextval, b=RAND() * 10000, c='57babb2c175128a0683b902b1364e7cb';
ERROR HY000: Sequence sequence_not_exist in database dbtest_seq_dml not found.
INSERT INTO dbtest_seq_dml.t0 SET a=sequence_not_exist.currval, b=RAND() * 10000, c='57babb2c175128a0683b902b1364e7cb';
ERROR HY000: Sequence sequence_not_exist in database dbtest_seq_dml not found.
commit;
DROP TDSQL_SEQUENCE dbtest_seq_dml.sequence_000;
DROP DATABASE dbtest_seq_dml ;

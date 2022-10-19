DROP DATABASE IF EXISTS dbtest_seq_alter_inc;
commit;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_inc.sequence_000;
commit;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_inc.sequence_001;
CREATE DATABASE dbtest_seq_alter_inc;
use dbtest_seq_alter_inc;
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_increment by 1 TDSQL_MAXVALUE 1000 TDSQL_minvalue 1 TDSQL_nocache;
CREATE TABLE t0(a int PRIMARY KEY);
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc' and name='sequence_000';
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
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_increment by 2;
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc' and name='sequence_000';
step
2
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT * FROM t0 order by a;
a
1
2
3
5
7
9
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_increment by 10;
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc' and name='sequence_000';
step
10
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT * FROM t0 order by a;
a
1
2
3
5
7
9
19
29
39
commit;
DROP TDSQL_SEQUENCE sequence_000 ;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_increment by -1 TDSQL_MAXVALUE 1000 TDSQL_minvalue 1 TDSQL_nocache;
CREATE TABLE t1(a int PRIMARY KEY);
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc' and name='sequence_001';
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
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc' and name='sequence_001';
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
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_increment by -10;
SELECT step from mysql.tdsql_sequences where db='dbtest_seq_alter_inc' and name='sequence_001';
step
-10
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
SELECT * FROM t1 order by a;
a
962
972
982
992
994
996
998
999
1000
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_increment by 10000;
ERROR HY000: Wrong sequence argument(step) value specified: step < maxvalue-minvalue must be true..
ALTER TDSQL_SEQUENCE sequence_001 TDSQL_increment by 0;
ERROR HY000: Wrong sequence argument(step) value specified: step != 0.
commit;
DROP TDSQL_SEQUENCE sequence_001 ;
DROP DATABASE dbtest_seq_alter_inc  ;

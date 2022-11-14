DROP DATABASE IF EXISTS dbtest_seq_cac;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_cac.sequence_000;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_cac.sequence_001;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_cac.sequence_002;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_ddl.sequence_000;
CREATE DATABASE dbtest_seq_cac;
use dbtest_seq_cac;
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 1000 TDSQL_minvalue 100;
CREATE TABLE t0(a int PRIMARY KEY);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
-9223372036854775808	20
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
119	20
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
119	20
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
139	20
SELECT * FROM t0 order by a;
a
100
101
102
103
104
105
106
107
108
109
110
111
112
113
114
115
116
117
118
119
120
121
122
123
124
125
126
127
128
129
commit;
DROP TDSQL_SEQUENCE sequence_000 ;
CREATE TDSQL_SEQUENCE sequence_001 TDSQL_MAXVALUE 1000 TDSQL_minvalue 100 TDSQL_CACHE 10;
CREATE TABLE t1(a int PRIMARY KEY);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_001';
curval	n_cache
-9223372036854775808	10
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_001';
curval	n_cache
109	10
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_001';
curval	n_cache
119	10
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
INSERT INTO t1 values(sequence_001.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_001';
curval	n_cache
129	10
SELECT * FROM t1 order by a;
a
100
101
102
103
104
105
106
107
108
109
110
111
112
113
114
115
116
117
118
119
120
121
122
123
124
125
126
127
128
129
commit;
DROP TDSQL_SEQUENCE sequence_001;
CREATE TDSQL_SEQUENCE sequence_002 TDSQL_MAXVALUE 1000 TDSQL_minvalue 100 TDSQL_nocache;
CREATE TABLE t2(a int PRIMARY KEY);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
-9223372036854775808	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
100	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
101	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
102	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
103	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
104	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
105	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
106	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
107	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
108	1
INSERT INTO t2 values(sequence_002.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_002';
curval	n_cache
109	1
SELECT * FROM t2 order by a;
a
100
101
102
103
104
105
106
107
108
109
commit;
DROP TDSQL_SEQUENCE sequence_002;
DROP DATABASE dbtest_seq_cac ;

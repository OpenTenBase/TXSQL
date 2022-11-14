DROP DATABASE IF EXISTS dbtest_seq_alter_cac;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_cac.sequence_000;
CREATE DATABASE dbtest_seq_alter_cac;
use dbtest_seq_alter_cac;
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
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_nocache;
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
139	1
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
139	1
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
141	1
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
142	1
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
143	1
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
144	1
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
145	1
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_CACHE 5;
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
145	5
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
150	5
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
150	5
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
150	5
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
150	5
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
150	5
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
150	5
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
155	5
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
155	5
INSERT INTO t0 values(sequence_000.nextval);
SELECT curval,n_cache from mysql.tdsql_sequences where name='sequence_000';
curval	n_cache
155	5
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
130
131
132
133
134
135
136
137
138
139
140
141
142
143
144
145
146
147
148
149
150
151
152
153
commit;
DROP TDSQL_SEQUENCE sequence_000;
DROP DATABASE dbtest_seq_alter_cac  ;

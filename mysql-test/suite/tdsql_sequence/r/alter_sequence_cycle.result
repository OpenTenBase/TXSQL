DROP DATABASE IF EXISTS dbtest_seq_alter_cyc;
DROP TDSQL_SEQUENCE IF EXISTS dbtest_seq_alter_cyc.sequence_000;
CREATE DATABASE dbtest_seq_alter_cyc;
use dbtest_seq_alter_cyc;
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_increment by 1 TDSQL_MAXVALUE 50 TDSQL_minvalue 0 TDSQL_nocycle;
CREATE TABLE t0(id int PRIMARY KEY AUTO_INCREMENT,a int);
SELECT do_cycle from mysql.tdsql_sequences where db='dbtest_seq_alter_cyc' and name='sequence_000';
do_cycle
0
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
ERROR HY000: Sequence sequence_000 (step 1) in database dbtest_seq_alter_cyc is not cyclic and has reached its min/max value 50.
SELECT * FROM t0 order by id;
id	a
1	0
2	1
3	2
4	3
5	4
6	5
7	6
8	7
9	8
10	9
11	10
12	11
13	12
14	13
15	14
16	15
17	16
18	17
19	18
20	19
21	20
22	21
23	22
24	23
25	24
26	25
27	26
28	27
29	28
30	29
31	30
32	31
33	32
34	33
35	34
36	35
37	36
38	37
39	38
40	39
41	40
42	41
43	42
44	43
45	44
46	45
47	46
48	47
49	48
50	49
51	50
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_cycle;
SELECT do_cycle from mysql.tdsql_sequences where db='dbtest_seq_alter_cyc' and name='sequence_000';
do_cycle
1
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
SELECT * FROM t0 order by id;
id	a
1	0
2	1
3	2
4	3
5	4
6	5
7	6
8	7
9	8
10	9
11	10
12	11
13	12
14	13
15	14
16	15
17	16
18	17
19	18
20	19
21	20
22	21
23	22
24	23
25	24
26	25
27	26
28	27
29	28
30	29
31	30
32	31
33	32
34	33
35	34
36	35
37	36
38	37
39	38
40	39
41	40
42	41
43	42
44	43
45	44
46	45
47	46
48	47
49	48
50	49
51	50
52	0
53	1
54	2
55	3
56	4
57	5
58	6
59	7
60	8
61	9
62	10
63	11
64	12
65	13
66	14
67	15
68	16
69	17
70	18
71	19
72	20
73	21
74	22
75	23
76	24
77	25
78	26
79	27
80	28
81	29
82	30
83	31
84	32
85	33
86	34
87	35
88	36
89	37
90	38
91	39
92	40
93	41
94	42
95	43
96	44
97	45
98	46
99	47
100	48
101	49
102	50
103	0
104	1
105	2
106	3
107	4
108	5
109	6
110	7
111	8
112	9
113	10
114	11
115	12
116	13
117	14
118	15
119	16
120	17
121	18
122	19
123	20
124	21
125	22
126	23
127	24
128	25
129	26
130	27
131	28
132	29
133	30
134	31
135	32
136	33
137	34
138	35
139	36
140	37
141	38
142	39
143	40
144	41
145	42
146	43
147	44
148	45
149	46
150	47
151	48
152	49
153	50
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_nocycle;
SELECT curval,do_cycle from mysql.tdsql_sequences where db='dbtest_seq_alter_cyc' and name='sequence_000';
curval	do_cycle
50	0
INSERT INTO t0(a) values(sequence_000.nextval);
ERROR HY000: Sequence sequence_000 (step 1) in database dbtest_seq_alter_cyc is not cyclic and has reached its min/max value 50.
ALTER TDSQL_SEQUENCE sequence_000 TDSQL_cycle;
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
INSERT INTO t0(a) values(sequence_000.nextval);
SELECT * FROM t0 order by id desc limit 10;
id	a
158	4
157	3
156	2
155	1
154	0
153	50
152	49
151	48
150	47
149	46
commit;
DROP TDSQL_SEQUENCE sequence_000 ;
DROP DATABASE dbtest_seq_alter_cyc  ;

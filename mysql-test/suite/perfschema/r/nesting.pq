"================== con1 connected =================="
select "MARKER_BEGIN" as marker;
marker
MARKER_BEGIN
select "This is simple statement one" as payload;
payload
This is simple statement one
select "This is simple statement two" as payload;
payload
This is simple statement two
select "This is the first part of a multi query" as payload;
select "And this is the second part of a multi query" as payload;
select "With a third part to make things complete" as payload;
$$
payload
And this is the second part of a multi query
This is the first part of a multi query
With a third part to make things complete
payload
payload
select "MARKER_END" as marker;
marker
MARKER_END
select "Con1 is done with payload" as status;
status
Con1 is done with payload
"================== con1 done =================="
set @con1_tid = (select thread_id from performance_schema.threads
where processlist_user = 'user1');
select (@con1_tid is not null) as expected;
expected
1
set @marker_begin = (select event_id from performance_schema.events_statements_history_long
where sql_text like "%MARKER_BEGIN%" and thread_id = @con1_tid);
select (@marker_begin is not null) as expected;
expected
1
set @marker_end = (select end_event_id from performance_schema.events_statements_history_long
where sql_text like "%MARKER_END%" and thread_id = @con1_tid);
select (@marker_end is not null) as expected;
expected
1
show global status like "performance_schema%_lost";
Variable_name	Value
Performance_schema_accounts_lost	0
Performance_schema_cond_classes_lost	0
Performance_schema_cond_instances_lost	0
Performance_schema_digest_lost	0
Performance_schema_file_classes_lost	0
Performance_schema_file_handles_lost	0
Performance_schema_file_instances_lost	0
Performance_schema_hosts_lost	0
Performance_schema_index_stat_lost	0
Performance_schema_locker_lost	0
Performance_schema_memory_classes_lost	0
Performance_schema_metadata_lock_lost	0
Performance_schema_mutex_classes_lost	0
Performance_schema_mutex_instances_lost	0
Performance_schema_nested_statement_lost	0
Performance_schema_prepared_statements_lost	0
Performance_schema_program_lost	0
Performance_schema_rwlock_classes_lost	0
Performance_schema_rwlock_instances_lost	0
Performance_schema_session_connect_attrs_lost	0
Performance_schema_socket_classes_lost	0
Performance_schema_socket_instances_lost	0
Performance_schema_stage_classes_lost	0
Performance_schema_statement_classes_lost	0
Performance_schema_table_handles_lost	0
Performance_schema_table_instances_lost	0
Performance_schema_table_lock_stat_lost	0
Performance_schema_thread_classes_lost	0
Performance_schema_thread_instances_lost	0
Performance_schema_users_lost	0
select * from (
(
select (event_id - @marker_begin) as relative_event_id,
(end_event_id - @marker_begin) as relative_end_event_id,
event_name,
sql_text as comment,
nesting_event_type,
(nesting_event_id - @marker_begin) as relative_nesting_event_id
from performance_schema.events_statements_history_long
where (thread_id = @con1_tid)
and (@marker_begin <= event_id)
and (end_event_id <= @marker_end)
)
union
(
select (event_id - @marker_begin) as relative_event_id,
(end_event_id - @marker_begin) as relative_end_event_id,
event_name,
"(stage)" as comment,
nesting_event_type,
(nesting_event_id - @marker_begin) as relative_nesting_event_id
from performance_schema.events_stages_history_long
where (thread_id = @con1_tid)
and (@marker_begin <= event_id)
and (end_event_id <= @marker_end)
)
union
(
select (event_id - @marker_begin) as relative_event_id,
(end_event_id - @marker_begin) as relative_end_event_id,
event_name,
operation as comment,
nesting_event_type,
(nesting_event_id - @marker_begin) as relative_nesting_event_id
from performance_schema.events_waits_history_long
where (thread_id = @con1_tid)
and (@marker_begin <= event_id)
and (end_event_id <= @marker_end)
)
) all_events
order by relative_event_id asc;
relative_event_id	relative_end_event_id	event_name	comment	nesting_event_type	relative_nesting_event_id
0	16	statement/sql/select	select "MARKER_BEGIN" as marker	NULL	NULL
1	5	stage/sql/starting	(stage)	STATEMENT	0
2	2	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	1
3	3	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	1
4	4	wait/io/file/sql/query_log	write	STAGE	1
5	5	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	1
6	6	stage/sql/checking permissions	(stage)	STATEMENT	0
7	7	stage/sql/Opening tables	(stage)	STATEMENT	0
8	8	stage/sql/init	(stage)	STATEMENT	0
9	9	stage/sql/optimizing	(stage)	STATEMENT	0
10	10	stage/sql/executing	(stage)	STATEMENT	0
11	11	stage/sql/end	(stage)	STATEMENT	0
12	12	stage/sql/query end	(stage)	STATEMENT	0
13	13	stage/sql/closing tables	(stage)	STATEMENT	0
14	14	stage/sql/freeing items	(stage)	STATEMENT	0
15	16	stage/sql/cleaning up	(stage)	STATEMENT	0
16	16	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	15
17	17	idle	idle	NULL	NULL
18	34	statement/sql/select	select "This is simple statement one" as payload	NULL	NULL
19	23	stage/sql/starting	(stage)	STATEMENT	18
20	20	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	19
21	21	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	19
22	22	wait/io/file/sql/query_log	write	STAGE	19
23	23	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	19
24	24	stage/sql/checking permissions	(stage)	STATEMENT	18
25	25	stage/sql/Opening tables	(stage)	STATEMENT	18
26	26	stage/sql/init	(stage)	STATEMENT	18
27	27	stage/sql/optimizing	(stage)	STATEMENT	18
28	28	stage/sql/executing	(stage)	STATEMENT	18
29	29	stage/sql/end	(stage)	STATEMENT	18
30	30	stage/sql/query end	(stage)	STATEMENT	18
31	31	stage/sql/closing tables	(stage)	STATEMENT	18
32	32	stage/sql/freeing items	(stage)	STATEMENT	18
33	34	stage/sql/cleaning up	(stage)	STATEMENT	18
34	34	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	33
35	35	idle	idle	NULL	NULL
36	52	statement/sql/select	select "This is simple statement two" as payload	NULL	NULL
37	41	stage/sql/starting	(stage)	STATEMENT	36
38	38	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	37
39	39	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	37
40	40	wait/io/file/sql/query_log	write	STAGE	37
41	41	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	37
42	42	stage/sql/checking permissions	(stage)	STATEMENT	36
43	43	stage/sql/Opening tables	(stage)	STATEMENT	36
44	44	stage/sql/init	(stage)	STATEMENT	36
45	45	stage/sql/optimizing	(stage)	STATEMENT	36
46	46	stage/sql/executing	(stage)	STATEMENT	36
47	47	stage/sql/end	(stage)	STATEMENT	36
48	48	stage/sql/query end	(stage)	STATEMENT	36
49	49	stage/sql/closing tables	(stage)	STATEMENT	36
50	50	stage/sql/freeing items	(stage)	STATEMENT	36
51	52	stage/sql/cleaning up	(stage)	STATEMENT	36
52	52	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	51
53	53	idle	idle	NULL	NULL
54	69	statement/sql/select	select "This is the first part of a multi query" as payload;
select "And this is the second part of a multi query" as payload;
select "With a third part to make things complete" as payload	NULL	NULL
55	60	stage/sql/starting	(stage)	STATEMENT	54
56	56	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	55
57	57	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	55
58	58	wait/io/file/sql/query_log	write	STAGE	55
59	59	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	55
60	60	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	55
61	61	stage/sql/checking permissions	(stage)	STATEMENT	54
62	62	stage/sql/Opening tables	(stage)	STATEMENT	54
63	63	stage/sql/init	(stage)	STATEMENT	54
64	64	stage/sql/optimizing	(stage)	STATEMENT	54
65	65	stage/sql/executing	(stage)	STATEMENT	54
66	66	stage/sql/end	(stage)	STATEMENT	54
67	67	stage/sql/query end	(stage)	STATEMENT	54
68	68	stage/sql/closing tables	(stage)	STATEMENT	54
69	69	stage/sql/freeing items	(stage)	STATEMENT	54
70	85	statement/sql/select	select "And this is the second part of a multi query" as payload;
select "With a third part to make things complete" as payload	NULL	NULL
71	76	stage/sql/starting	(stage)	STATEMENT	70
72	72	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	71
73	73	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	71
74	74	wait/io/file/sql/query_log	write	STAGE	71
75	75	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	71
76	76	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	71
77	77	stage/sql/checking permissions	(stage)	STATEMENT	70
78	78	stage/sql/Opening tables	(stage)	STATEMENT	70
79	79	stage/sql/init	(stage)	STATEMENT	70
80	80	stage/sql/optimizing	(stage)	STATEMENT	70
81	81	stage/sql/executing	(stage)	STATEMENT	70
82	82	stage/sql/end	(stage)	STATEMENT	70
83	83	stage/sql/query end	(stage)	STATEMENT	70
84	84	stage/sql/closing tables	(stage)	STATEMENT	70
85	85	stage/sql/freeing items	(stage)	STATEMENT	70
86	102	statement/sql/select	select "With a third part to make things complete" as payload	NULL	NULL
87	91	stage/sql/starting	(stage)	STATEMENT	86
88	88	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	87
89	89	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	87
90	90	wait/io/file/sql/query_log	write	STAGE	87
91	91	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	87
92	92	stage/sql/checking permissions	(stage)	STATEMENT	86
93	93	stage/sql/Opening tables	(stage)	STATEMENT	86
94	94	stage/sql/init	(stage)	STATEMENT	86
95	95	stage/sql/optimizing	(stage)	STATEMENT	86
96	96	stage/sql/executing	(stage)	STATEMENT	86
97	97	stage/sql/end	(stage)	STATEMENT	86
98	98	stage/sql/query end	(stage)	STATEMENT	86
99	99	stage/sql/closing tables	(stage)	STATEMENT	86
100	100	stage/sql/freeing items	(stage)	STATEMENT	86
101	102	stage/sql/cleaning up	(stage)	STATEMENT	86
102	102	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	101
103	103	idle	idle	NULL	NULL
104	120	statement/sql/select	select "MARKER_END" as marker	NULL	NULL
105	109	stage/sql/starting	(stage)	STATEMENT	104
106	106	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	105
107	107	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	105
108	108	wait/io/file/sql/query_log	write	STAGE	105
109	109	wait/synch/mutex/sql/THD::LOCK_thd_data	lock	STAGE	105
110	110	stage/sql/checking permissions	(stage)	STATEMENT	104
111	111	stage/sql/Opening tables	(stage)	STATEMENT	104
112	112	stage/sql/init	(stage)	STATEMENT	104
113	113	stage/sql/optimizing	(stage)	STATEMENT	104
114	114	stage/sql/executing	(stage)	STATEMENT	104
115	115	stage/sql/end	(stage)	STATEMENT	104
116	116	stage/sql/query end	(stage)	STATEMENT	104
117	117	stage/sql/closing tables	(stage)	STATEMENT	104
118	118	stage/sql/freeing items	(stage)	STATEMENT	104
119	120	stage/sql/cleaning up	(stage)	STATEMENT	104
120	120	wait/synch/mutex/sql/THD::LOCK_thd_query	lock	STAGE	119

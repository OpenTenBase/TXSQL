# Infrastructure for test error and kill of parallel execution
show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,GROUP_REPLICATION_ADMIN,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PERSIST_RO_VARIABLES_ADMIN,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ''@'' TO 'root'@'localhost' WITH GRANT OPTION
select user();
user()
root@localhost
create database test1;
create user 'user1'@'%';
grant all on test1.* to 'user1'@'%';
create table t1(a int, b int, c int, d int);
insert into t1 values(1,2,3,4);
insert into t1 values(1,1,1,1);
insert into t1 values(2,3,4,5);
insert into t1 values(2,2,2,2);
insert into t1 values(3,4,5,6);
insert into t1 values(4,5,6,7);
insert into t1 values(5,6,7,8);
create table t2(a int, b int, c int, d int);
insert into t2 values(1,1,1,1);
insert into t2 values(2,2,2,2);
insert into t2 values(3,3,3,3);
insert into t2 values(4,4,4,4);
insert into t2 values(5,5,5,5);
create table t3(a int, b int, c int, d int, primary key(a), key idx(b));
insert into t3 values(1,1,1,1);
insert into t3 values(2,2,2,2);
insert into t3 values(3,3,3,3);
insert into t3 values(4,4,4,4);
insert into t3 values(5,5,5,5);
insert into t3 values(6,6,6,6);
insert into t3 values(7,7,7,7);
insert into t3 values(8,8,8,8);
insert into t3 values(9,9,9,9);
insert into t3 values(10,10,10,10);
create table t4(a int, b int, c int, d int);
insert into t4 values(100,100,100,100);
create table t5(a int, b int, c int, d int);
# --include save_and_set_px.inc
set txsql_parallel_table_record_threshold = 2;
set global debug = '+d, disable_debug_sync_check';
flush status;
set debug_sync='execute_in_parallel_before_kill SIGNAL waiting wait_for killed';
select a,sum(b) from t1 group by a order by a;;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
set debug_sync='execute_in_parallel_before_kill SIGNAL waiting wait_for killed';
select a,sum(b) from t4 group by a union select a,b from t2 order by a;;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
set debug_sync='execute_in_parallel_before_kill SIGNAL waiting wait_for killed';
(select a,sum(b) from t4 group by a order by a) union (select a,b from t2 order by a);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
(select a,sum(b) from t4 group by a order by a) union (select a,b from t2 order by a);
a	sum(b)
100	100
1	1
4	4
3	3
2	2
5	5
set debug_sync='execute_in_parallel_before_kill SIGNAL waiting wait_for killed';
select a,b from t4 union (select a,b from t2 order by a);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	1
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,execute_in_parallel_before_error";
select a, sum(b) from t1 group by a;
ERROR HY000: Memory allocation error:  in function px_execute_in_coordinator.
set session debug="+d,execute_in_parallel_before_error";
select a,b from t1 union select a,c from t2;
ERROR HY000: Memory allocation error:  in function px_execute_in_coordinator.
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	1
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,execute_in_parallel_before_fallback";
select a, sum(b) from t1 group by a order by a;
a	sum(b)
1	3
2	5
3	4
4	5
5	6
set session debug="+d,execute_in_parallel_before_fallback";
(select a,b from t1 order by a) union (select a,c from t2 order by a);
a	b
1	2
1	1
2	3
2	2
3	4
4	5
5	6
3	3
4	4
5	5
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	1
Txsql_parallel_stmt_fallback	2
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,execute_in_parallel_before_sleep";
select a,b,c,d from t1;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
set session debug="+d,execute_in_parallel_before_sleep";
(select a,b,c,d from t1) union (select a,b,sum(c),sum(d) from t2 group by a,b order by a,b);
a	b	c	d
1	2	3	4
2	3	4	5
1	1	1	1
2	2	2	2
3	4	5	6
4	5	6	7
5	6	7	8
4	4	4	4
3	3	3	3
5	5	5	5
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	3
Txsql_parallel_stmt_fallback	2
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set debug_sync='schedule_before_kill SIGNAL waiting wait_for killed';
select a,sum(b),avg(d) from t1 group by a order by a;;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
set debug_sync='schedule_before_kill SIGNAL waiting wait_for killed';
(select a,b,c from t2) union (select a,b,c from t1 order by a);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	5
Txsql_parallel_stmt_fallback	2
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,schedule_before_fallback";
select a, sum(b), avg(c)+avg(d) from t2 group by a order by a;
a	sum(b)	avg(c)+avg(d)
1	1	2.0000
2	2	4.0000
3	3	6.0000
4	4	8.0000
5	5	10.0000
set session debug="+d,schedule_before_fallback";
(select a,b,sum(c) from t1 group by a,b) union (select a, sum(b), avg(c)+avg(d) from t2 group by a order by a);
a	b	sum(c)
1	2	3.0000
1	1	1.0000
2	3	4.0000
2	2	2.0000
3	4	5.0000
4	5	6.0000
5	6	7.0000
1	1	2.0000
2	2	4.0000
3	3	6.0000
4	4	8.0000
5	5	10.0000
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	5
Txsql_parallel_stmt_fallback	4
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,schedule_before_sleep";
select a,b,c,d from t1 order by d;
a	b	c	d
1	1	1	1
2	2	2	2
1	2	3	4
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
set session debug="+d,schedule_before_sleep";
(select a,b,c,d from t1 order by d) union (select a,b,sum(c),sum(d) from t2 group by a,b);
a	b	c	d
1	2	3	4
2	3	4	5
1	1	1	1
3	4	5	6
2	2	2	2
4	5	6	7
5	6	7	8
4	4	4	4
3	3	3	3
5	5	5	5
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	7
Txsql_parallel_stmt_fallback	4
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set debug_sync='schedule_before_schedule_kill SIGNAL waiting wait_for killed';
select /*+no_bnl(t1,t2)*/ t1.* from t1,t2 where t1.a = t2.a;;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
set debug_sync='schedule_before_schedule_kill SIGNAL waiting wait_for killed';
(select a,b,c from t2 order by a,c) union (select a,b,c from t1 order by a,b);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	9
Txsql_parallel_stmt_fallback	4
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set global debug="+d,schedule_before_schedule_error";
select b, sum(a) from t1 group by b order by b;
b	sum(a)
1	1
2	3
3	2
4	3
5	4
6	5
set global debug="-d,schedule_before_schedule_error";
set global debug="+d,schedule_before_schedule_error";
(select b,a,sum(c) from t1 group by b,a) union (select a,b,c from t2 order by a);
b	a	sum(c)
2	1	3
1	1	1
3	2	4
2	2	2
4	3	5
5	4	6
6	5	7
3	3	3
4	4	4
5	5	5
set global debug="-d,schedule_before_schedule_error";
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	9
Txsql_parallel_stmt_fallback	6
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,schedule_before_schedule_sleep";
select a,b,c,d from t1 order by a,b,c,d;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
set session debug="+d,schedule_before_schedule_sleep";
(select a,b,c,d from t1 order by a,b,c,d) union all (select d,c,b,a from t2 order by d,c);
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
3	3	3	3
1	1	1	1
2	2	2	2
4	4	4	4
5	5	5	5
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	11
Txsql_parallel_stmt_fallback	6
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set debug_sync='schedule_one_loop_kill SIGNAL waiting wait_for killed';
select /*+no_bnl(t1,t2)*/ t1.* from t1,t2 where t1.a = t2.a order by t1.a;;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
a	b	c	d
1	2	3	4
1	1	1	1
2	3	4	5
2	2	2	2
3	4	5	6
4	5	6	7
5	6	7	8
SET DEBUG_SYNC = 'RESET';
set debug_sync='schedule_one_loop_kill SIGNAL waiting wait_for killed';
(select /*+no_bnl(t1,t2)*/ t1.* from t1,t2 where t1.a = t2.a order by t1.a) union all (select * from t3);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	11
Txsql_parallel_stmt_fallback	6
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set global debug="+d,schedule_one_loop_sleep_worker";
select a,b,c+d from t1 order by a,b;
a	b	c+d
1	1	2
1	2	7
2	2	4
2	3	9
3	4	11
4	5	13
5	6	15
set global debug="-d,schedule_one_loop_sleep_worker";
set global debug="+d,schedule_one_loop_sleep_worker";
(select a,b,c+d from t1 order by a,b) union (select a,b+c,c+d from t2);
a	b	c+d
1	2	7
1	1	2
2	3	9
2	2	4
3	4	11
5	6	15
4	5	13
2	4	4
4	8	8
1	2	2
3	6	6
5	10	10
set global debug="-d,schedule_one_loop_sleep_worker";
set session debug="+d,schedule_one_loop_sleep";
(select * from t1) union (select * from t2) union (select * from t3);
a	b	c	d
1	2	3	4
1	1	1	1
2	3	4	5
2	2	2	2
3	4	5	6
4	5	6	7
5	6	7	8
3	3	3	3
4	4	4	4
5	5	5	5
8	8	8	8
6	6	6	6
7	7	7	7
10	10	10	10
9	9	9	9
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	14
Txsql_parallel_stmt_fallback	6
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set debug_sync='run_root_sending_data_kill SIGNAL waiting wait_for killed';
select /*+no_bnl(t1,t2)*/ t1.* from t1,t3 where t1.a = t3.a order by t1.a;;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
set debug_sync='run_root_sending_data_kill SIGNAL waiting wait_for killed';
(select * from t1 where t1.a > 1) union (select * from t2 order by t2.a);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	4
Txsql_parallel_stmt_executed	14
Txsql_parallel_stmt_fallback	6
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,run_root_before_sending_data_error";
select a+b+c+d from t1 where a > 1 order by a,b,c,d;
a+b+c+d
8
14
18
22
26
set session debug="+d,run_root_before_sending_data_error";
(select a,b from t2) union (select a+b+c,d from t1 where a > 1 order by a,b,c,d);
a	b
1	1
2	2
3	3
4	4
5	5
9	5
6	2
12	6
15	7
18	8
set session debug="+d,run_root_in_sending_data_error";
select a, sum(b+c) from t1 where a > 1 group by a order by a;
ERROR HY000: PX execute error of errno[3044].
set session debug="+d,run_root_in_sending_data_error";
(select a,b from t2) union (select a, sum(b+c) from t1 where a > 1 group by a order by a);
ERROR HY000: PX execute error of errno[3044].
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	6
Txsql_parallel_stmt_executed	14
Txsql_parallel_stmt_fallback	8
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,run_root_in_sending_data_sleep";
select a+b,c+d from t1 order by a+b,c+d;
a+b	c+d
2	2
3	7
4	4
5	9
7	11
9	13
11	15
set session debug="+d,run_root_in_sending_data_sleep";
(select a+b,c+d from t1 order by a+b,c+d) union all (select d+c,b+a from t2 order by d+c);
a+b	c+d
2	2
5	9
4	4
3	7
9	13
11	15
7	11
2	2
4	4
6	6
8	8
10	10
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	6
Txsql_parallel_stmt_executed	16
Txsql_parallel_stmt_fallback	8
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set debug_sync='schedule_after_schedule_kill SIGNAL waiting wait_for killed';
(select /*+no_bnl(t1,t2)*/ t1.* from t1,t2 where t1.a = t2.a order by t1.a) union (select * from t3) union (select * from t4 order by a);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
a	b	c	d
1	2	3	4
1	1	1	1
2	3	4	5
2	2	2	2
4	5	6	7
3	4	5	6
5	6	7	8
3	3	3	3
4	4	4	4
5	5	5	5
8	8	8	8
7	7	7	7
9	9	9	9
6	6	6	6
10	10	10	10
100	100	100	100
SET DEBUG_SYNC = 'RESET';
set debug_sync='schedule_after_schedule_kill SIGNAL waiting wait_for killed';
(select * from t3) union (select * from t4 order by a) union (select /*+no_bnl(t1,t2)*/ t1.* from t1,t2 where t1.a = t2.a order by t1.a);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
a	b	c	d
1	1	1	1
3	3	3	3
2	2	2	2
5	5	5	5
4	4	4	4
7	7	7	7
6	6	6	6
8	8	8	8
9	9	9	9
10	10	10	10
100	100	100	100
1	2	3	4
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
SET DEBUG_SYNC = 'RESET';
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	6
Txsql_parallel_stmt_executed	18
Txsql_parallel_stmt_fallback	8
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,schedule_after_schedule_sleep";
select a+b,c+d from t1 order by a,b,c,d;
a+b	c+d
2	2
3	7
4	4
5	9
7	11
9	13
11	15
set session debug="+d,schedule_after_schedule_sleep";
(select a+b,c+d from t1 order by a,b,c,d) union all (select d+c,b+a from t2 order by d,c);
a+b	c+d
3	7
2	2
4	4
5	9
7	11
11	15
9	13
2	2
4	4
6	6
8	8
10	10
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	6
Txsql_parallel_stmt_executed	20
Txsql_parallel_stmt_fallback	8
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set session debug="+d,create_partial_threads_error";
select a+b,sum(c),avg(d) from t5 group by a+b;
a+b	sum(c)	avg(d)
set session debug="-d,create_partial_threads_error";
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	6
Txsql_parallel_stmt_executed	21
Txsql_parallel_stmt_fallback	8
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
select a from t5 union select b from t5 union select c from t5 union select d from t5;
a
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	6
Txsql_parallel_stmt_executed	22
Txsql_parallel_stmt_fallback	8
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
select /*+max_execution_time(2)*/ a,sum(b),sum(c) from t1 group by a;
ERROR HY000: Query execution was interrupted, maximum statement execution time exceeded
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	6
Txsql_parallel_stmt_executed	22
Txsql_parallel_stmt_fallback	8
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
SET GLOBAL debug = '+d,px_simulate_worker_timeout_kill';
select /*+max_execution_time(1000)*/ * FROM t1;
ERROR HY000: Query execution was interrupted, maximum statement execution time exceeded
SET GLOBAL debug = '-d,px_simulate_worker_timeout_kill';
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	6
Txsql_parallel_stmt_executed	23
Txsql_parallel_stmt_fallback	8
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
set global debug="+d,enter_worker_sleep";
set debug_sync='schedule_before_schedule_kill SIGNAL waiting wait_for killed';
(select a,b,d from t1) union (select a,b,sum(d) from t2 group by a,b order by a,b);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
SET DEBUG_SYNC= 'now SIGNAL killed';
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
set global debug="-d,enter_worker_sleep";
# --include set_px_to_saved.inc
flush status;
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	0
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
drop table t1;
drop table t2;
drop table t3;
drop table t4;
drop table t5;
drop database test1;
drop user user1;

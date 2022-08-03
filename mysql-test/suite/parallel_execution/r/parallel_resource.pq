# Infrastructure for test resource allocation of parallel execution
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
SET @saved_txsql_parallel_fallback_in_execution =@@txsql_parallel_fallback_in_execution;
set global txsql_parallel_fallback_in_execution = 0;
# --include save_and_set_px.inc
set global debug='+d,disable_debug_sync_check';
set global txsql_parallel_table_record_threshold = 2;
SET @saved_txsql_parallel_degree = @@txsql_parallel_degree;
set global txsql_parallel_degree = 1;
flush status;
#
# Case 1:
#   Query A uses up threads, preventing query B from executing in parallel.
#
set global txsql_max_parallel_worker_threads = 3;
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	0
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
# Query A
set debug_sync="execute_in_parallel_before_kill SIGNAL waiting wait_for killed";
select a,sum(b) from t1 group by a;;
# Query B
select a,avg(b) from t1 group by a union select a,b from t2;
a	avg(b)
1	1.0000
1	1.5000
2	2.0000
2	2.5000
3	3.0000
3	4.0000
4	4.0000
4	5.0000
5	5.0000
5	6.0000
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
SET DEBUG_SYNC= 'now SIGNAL killed';
a	sum(b)
1	3
2	5
3	4
4	5
5	6
SET DEBUG_SYNC = 'RESET';
# Expect error 0 executed 1 fallback 1
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	2
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
#
# Case 2:
#   Query A uses up threads, preventing query B from executing in parallel.
#   However, A is killed later.
#
# Query A
set debug_sync="execute_in_parallel_before_kill SIGNAL waiting wait_for timeout";
select a,sum(b) from t1 group by a union select a,b from t2;;
# Query B
select a,avg(b) from t1 group by a union select a,sum(c) from t2 group by a;
a	avg(b)
1	1.0000
1	1.5000
2	2.0000
2	2.5000
3	3.0000
3	4.0000
4	4.0000
4	5.0000
5	5.0000
5	6.0000
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id>;
ERROR 70100: Query execution was interrupted
SET DEBUG_SYNC = 'RESET';
# Expect error 1 executed 1 fallback 2
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	1
Txsql_parallel_stmt_executed	2
Txsql_parallel_stmt_fallback	1
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	0
Txsql_parallel_stmt_thread_refused	1
Txsql_parallel_threads_currently_used	0
#
# Case 3:
#   When thread limit is big enough, both query A and query B execute in parallel.
#
set global txsql_max_parallel_worker_threads = 16;
# Query A
set debug_sync="execute_in_parallel_before_kill SIGNAL waiting wait_for killed";
select a,sum(b) from t1 group by a union select a,b from t2 union select a,sum(b) from t3 group by a;;
# Query B
select a,avg(b),sum(c) from t1 group by a;
a	avg(b)	sum(c)
1	1.5000	4
2	2.5000	6
3	4.0000	5
4	5.0000	6
5	6.0000	7
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
SET DEBUG_SYNC= 'now SIGNAL killed';
a	sum(b)
1	3
2	5
3	4
4	5
5	6
1	1
2	2
3	3
4	4
5	5
6	6
7	7
8	8
9	9
10	10
SET DEBUG_SYNC = 'RESET';
# Expect error 1 executed 3 fallback 2
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	1
Txsql_parallel_stmt_executed	4
Txsql_parallel_stmt_fallback	1
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	0
Txsql_parallel_stmt_thread_refused	1
Txsql_parallel_threads_currently_used	0
set global txsql_parallel_degree = @saved_txsql_parallel_degree;
# --include set_px_to_saved.inc
set global txsql_parallel_fallback_in_execution = @saved_txsql_parallel_fallback_in_execution;
flush status;
# Expect error 0 executed 0 fallback 0
show status like '%txsql_parallel%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	0
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	0
Txsql_parallel_stmt_thread_refused	0
Txsql_parallel_threads_currently_used	0
drop table t1;
drop table t2;
drop table t3;

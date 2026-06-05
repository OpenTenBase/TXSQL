create table t1(a int, b int);
create table t2(a int not null, b int not null);
INSERT INTO t1 VALUES (1, 1), (2, NULL), (3, 3);
INSERT INTO t2 VALUES (1, 1), (2, 2), (3, 3);
ANALYZE TABLE t1,t2;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
set @saved_value=@@txsql_count_conversion_enabled;
set @@txsql_count_conversion_enabled = 0;
flush status;
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	0
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	0
EXPLAIN format=tree select count(*) from t1 where a > 1;
EXPLAIN
-> Aggregate: sum(`count(*)`)  (cost=0.65 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.65 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.65 rows=1)
            -> Aggregate: count(0)  (cost=0.65 rows=1)
                -> Filter: (t1.a > 1)  (cost=0.55 rows=1)
                    -> Parallel table scan on t1  (cost=0.55 rows=3)

select count(*) from t1 where a > 1;
count(*)
2
EXPLAIN format=tree select count(b) from t1 where a > 1;
EXPLAIN
-> Aggregate: sum(`count(b)`)  (cost=0.65 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.65 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.65 rows=1)
            -> Aggregate: count(t1.b)  (cost=0.65 rows=1)
                -> Filter: (t1.a > 1)  (cost=0.55 rows=1)
                    -> Parallel table scan on t1  (cost=0.55 rows=3)

select count(b) from t1 where a > 1;
count(b)
1
EXPLAIN format=tree select count(*) from t2 where a > 1;
EXPLAIN
-> Aggregate: sum(`count(*)`)  (cost=0.65 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.65 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.65 rows=1)
            -> Aggregate: count(0)  (cost=0.65 rows=1)
                -> Filter: (t2.a > 1)  (cost=0.55 rows=1)
                    -> Parallel table scan on t2  (cost=0.55 rows=3)

select count(*) from t2 where a > 1;
count(*)
2
EXPLAIN format=tree select count(b) from t2 where a > 1;
EXPLAIN
-> Aggregate: sum(`count(b)`)  (cost=0.65 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.65 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.65 rows=1)
            -> Aggregate: count(t2.b)  (cost=0.65 rows=1)
                -> Filter: (t2.a > 1)  (cost=0.55 rows=1)
                    -> Parallel table scan on t2  (cost=0.55 rows=3)

select count(b) from t2 where a > 1;
count(b)
2
EXPLAIN format=tree select count(t2.b) from t2 left join t1 on t1.a = t2.a;
EXPLAIN
-> Aggregate: sum(`count(t2.b)`)  (cost=2.23 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=2.23 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=2.23 rows=1)
            -> Aggregate: count(t2.b)  (cost=2.23 rows=1)
                -> Left hash join (t1.a = t2.a)  (cost=1.33 rows=9)
                    -> Parallel table scan on t2  (cost=0.55 rows=3)
                    -> Hash
                        -> Table scan on t1  (cost=0.18 rows=3)

select count(t2.b) from t2 left join t1 on t1.a = t2.a;
count(t2.b)
3
EXPLAIN format=tree select count(t2.b) from t1 left join t2 on t1.a = t2.a;
EXPLAIN
-> Aggregate: sum(`count(t2.b)`)  (cost=2.23 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=2.23 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=2.23 rows=1)
            -> Aggregate: count(t2.b)  (cost=2.23 rows=1)
                -> Left hash join (t2.a = t1.a)  (cost=1.33 rows=9)
                    -> Parallel table scan on t1  (cost=0.55 rows=3)
                    -> Hash
                        -> Table scan on t2  (cost=0.18 rows=3)

select count(t2.b) from t1 left join t2 on t1.a = t2.a;
count(t2.b)
3
EXPLAIN format=tree select count(b) from t2 group by a;
EXPLAIN
-> Table scan on <temporary>
    -> Final Aggregate using temporary table
        -> PX Receiver (slice: 0; workers: 1)
            -> PX Sender (slice: 1; workers: 3)
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> Parallel table scan on t2  (cost=0.55 rows=3)

select count(b) from t2 group by a;
count(b)
1
1
1
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	0
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	0
set @@txsql_count_conversion_enabled = 1;
EXPLAIN format=tree select count(*) from t1 where a > 1;
EXPLAIN
-> Aggregate: sum(`count(*)`)  (cost=0.65 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.65 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.65 rows=1)
            -> Aggregate: count(0)  (cost=0.65 rows=1)
                -> Filter: (t1.a > 1)  (cost=0.55 rows=1)
                    -> Parallel table scan on t1  (cost=0.55 rows=3)

select count(*) from t1 where a > 1;
count(*)
2
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	5
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	0
EXPLAIN format=tree select count(b) from t1 where a > 1;
EXPLAIN
-> Aggregate: sum(`count(b)`)  (cost=0.65 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.65 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.65 rows=1)
            -> Aggregate: count(t1.b)  (cost=0.65 rows=1)
                -> Filter: (t1.a > 1)  (cost=0.55 rows=1)
                    -> Parallel table scan on t1  (cost=0.55 rows=3)

select count(b) from t1 where a > 1;
count(b)
1
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	10
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	0
EXPLAIN format=tree select count(*) from t2 where a > 1;
EXPLAIN
-> Aggregate: sum(`count(*)`)  (cost=0.65 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.65 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.65 rows=1)
            -> Aggregate: count(0)  (cost=0.65 rows=1)
                -> Filter: (t2.a > 1)  (cost=0.55 rows=1)
                    -> Parallel table scan on t2  (cost=0.55 rows=3)

select count(*) from t2 where a > 1;
count(*)
2
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	15
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	0
EXPLAIN format=tree select count(b) from t2 where a > 1;
EXPLAIN
-> Aggregate: sum(`count(b)`)  (cost=0.65 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.65 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.65 rows=1)
            -> Aggregate: count(0) using conversion for count of not null column  (cost=0.65 rows=1)
                -> Filter: (t2.a > 1)  (cost=0.55 rows=1)
                    -> Parallel table scan on t2  (cost=0.55 rows=3)

select count(b) from t2 where a > 1;
count(b)
2
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	20
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	5
EXPLAIN format=tree select count(t2.b) from t2 left join t1 on t1.a = t2.a;
EXPLAIN
-> Aggregate: sum(`count(t2.b)`)  (cost=2.23 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=2.23 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=2.23 rows=1)
            -> Aggregate: count(0) using conversion for count of not null column  (cost=2.23 rows=1)
                -> Left hash join (t1.a = t2.a)  (cost=1.33 rows=9)
                    -> Parallel table scan on t2  (cost=0.55 rows=3)
                    -> Hash
                        -> Table scan on t1  (cost=0.18 rows=3)

select count(t2.b) from t2 left join t1 on t1.a = t2.a;
count(t2.b)
3
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	25
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	10
EXPLAIN format=tree select count(t2.b) from t1 left join t2 on t1.a = t2.a;
EXPLAIN
-> Aggregate: sum(`count(t2.b)`)  (cost=2.23 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=2.23 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=2.23 rows=1)
            -> Aggregate: count(t2.b)  (cost=2.23 rows=1)
                -> Left hash join (t2.a = t1.a)  (cost=1.33 rows=9)
                    -> Parallel table scan on t1  (cost=0.55 rows=3)
                    -> Hash
                        -> Table scan on t2  (cost=0.18 rows=3)

select count(t2.b) from t1 left join t2 on t1.a = t2.a;
count(t2.b)
3
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	30
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	10
EXPLAIN format=tree select count(b) from t2 group by a;
EXPLAIN
-> Table scan on <temporary>
    -> Final Aggregate using temporary table
        -> PX Receiver (slice: 0; workers: 1)
            -> PX Sender (slice: 1; workers: 3)
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> Parallel table scan on t2  (cost=0.55 rows=3)

select count(b) from t2 group by a;
count(b)
1
1
1
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	35
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	15
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	35
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	15
# --include save_and_set_px.inc
set @@txsql_count_conversion_enabled=0;
EXPLAIN format=tree select avg(b) from t2;
EXPLAIN
-> Aggregate: px_sum_switcher(avg(t2.b))  (cost=0.85 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.85 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.85 rows=1)
            -> Aggregate: px_sum_switcher(avg(t2.b))  (cost=0.85 rows=1)
                -> Parallel table scan on t2  (cost=0.55 rows=3)

select avg(b) from t2;
avg(b)
2.0000
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	0
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	0
set @@txsql_count_conversion_enabled=1;
EXPLAIN format=tree select avg(b) from t2;
EXPLAIN
-> Aggregate: px_sum_switcher(avg(t2.b))  (cost=0.85 rows=1)
    -> PX Receiver (slice: 0; workers: 1)  (cost=0.85 rows=1)
        -> PX Sender (slice: 1; workers: 3)  (cost=0.85 rows=1)
            -> Aggregate: px_sum_switcher(avg(t2.b))  (cost=0.85 rows=1)
                -> Parallel table scan on t2  (cost=0.55 rows=3)

select avg(b) from t2;
avg(b)
2.0000
# --include set_px_to_saved.inc
show status like "%total_count_function%";
Variable_name	Value
Txsql_total_count_function	35
show status like "%implicit_convert_to_count_star%";
Variable_name	Value
Txsql_implicit_convert_to_count_star	15
set @@txsql_count_conversion_enabled=@saved_value;
DROP TABLE t1,t2;

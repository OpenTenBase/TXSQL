set @saved_value = @@txsql_convert_view_to_cte_enabled;
create table t1 (a int, b int);
insert into t1 values (1,2), (1,3), (2,4), (2,5), (3,10);
create view simple_view as
select * from t1;
create view complex_view as
select b from t1 group by b;
create view complex_view_2 as
select b+1 from t1 group by b+1;
create view complex_view_3 as
select b from t1 group by b;
set @@txsql_convert_view_to_cte_enabled = 1;
explain format=tree select * from complex_view as d_1, complex_view_3 as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize CTE txsql cte if needed (query plan printed elsewhere)  (cost=4.83..6.88 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize CTE txsql cte if needed  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from complex_view as d_1, complex_view_3 as d_2;
b	b
10	10
10	2
10	3
10	4
10	5
2	10
2	2
2	3
2	4
2	5
3	10
3	2
3	3
3	4
3	5
4	10
4	2
4	3
4	4
4	5
5	10
5	2
5	3
5	4
5	5
set @@txsql_convert_view_to_cte_enabled = 1;
explain format=tree select * from complex_view as d_1, complex_view_2 as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize  (cost=4.83..6.88 rows=5)
            -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                    -> PX Receiver (slice: 0; workers: 1)  (cost=0.75 rows=5)
                        -> PX Sender (slice: 1; workers: 4)  (cost=0.75 rows=5)
                            -> Parallel table scan on t1  (cost=0.75 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from complex_view as d_1, complex_view_2 as d_2;
b	b+1
10	11
10	3
10	4
10	5
10	6
2	11
2	3
2	4
2	5
2	6
3	11
3	3
3	4
3	5
3	6
4	11
4	3
4	4
4	5
4	6
5	11
5	3
5	4
5	5
5	6
create view uncacheable_view as
select b,rand() from t1 group by b;
set @@txsql_convert_view_to_cte_enabled = 0;
explain format=tree select * from uncacheable_view as d_1, uncacheable_view as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize  (cost=4.83..6.88 rows=5)
            -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                    -> PX Receiver (slice: 0; workers: 1)  (cost=0.75 rows=5)
                        -> PX Sender (slice: 1; workers: 4)  (cost=0.75 rows=5)
                            -> Parallel table scan on t1  (cost=0.75 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

set @@txsql_convert_view_to_cte_enabled = 1;
explain format=tree select * from uncacheable_view as d_1, uncacheable_view as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize CTE txsql cte if needed (query plan printed elsewhere)  (cost=4.83..6.88 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize CTE txsql cte if needed  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

drop view uncacheable_view;
set @@txsql_convert_view_to_cte_enabled = 0;
explain format=tree select * from simple_view;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)  (cost=0.75 rows=5)
    -> PX Sender (slice: 1; workers: 4)  (cost=0.75 rows=5)
        -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from simple_view;
a	b
1	2
1	3
2	4
2	5
3	10
explain format=tree select * from complex_view;
EXPLAIN
-> Table scan on complex_view  (cost=0.51..2.56 rows=5)
    -> Materialize  (cost=4.83..6.88 rows=5)
        -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
            -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                -> PX Receiver (slice: 0; workers: 1)  (cost=0.75 rows=5)
                    -> PX Sender (slice: 1; workers: 4)  (cost=0.75 rows=5)
                        -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from complex_view;
b
10
2
3
4
5
set @@txsql_convert_view_to_cte_enabled = 1;
explain format=tree select * from simple_view;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)  (cost=0.75 rows=5)
    -> PX Sender (slice: 1; workers: 4)  (cost=0.75 rows=5)
        -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from simple_view;
a	b
1	2
1	3
2	4
2	5
3	10
explain format=tree select * from complex_view;
EXPLAIN
-> Table scan on complex_view  (cost=0.51..2.56 rows=5)
    -> Materialize  (cost=4.83..6.88 rows=5)
        -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
            -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                -> PX Receiver (slice: 0; workers: 1)  (cost=0.75 rows=5)
                    -> PX Sender (slice: 1; workers: 4)  (cost=0.75 rows=5)
                        -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from complex_view;
b
10
2
3
4
5
set @@txsql_convert_view_to_cte_enabled = 0;
explain format=tree select * from simple_view as d_1, simple_view as d_2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)  (cost=3.50 rows=25)
    -> PX Sender (slice: 1; workers: 4)  (cost=3.50 rows=25)
        -> Inner hash join (no condition)  (cost=3.50 rows=25)
            -> Parallel table scan on t1  (cost=0.15 rows=5)
            -> Hash
                -> Table scan on t1  (cost=0.75 rows=5)

select * from simple_view as d_1, simple_view as d_2;
a	b	a	b
1	2	1	2
1	2	1	3
1	2	2	4
1	2	2	5
1	2	3	10
1	3	1	2
1	3	1	3
1	3	2	4
1	3	2	5
1	3	3	10
2	4	1	2
2	4	1	3
2	4	2	4
2	4	2	5
2	4	3	10
2	5	1	2
2	5	1	3
2	5	2	4
2	5	2	5
2	5	3	10
3	10	1	2
3	10	1	3
3	10	2	4
3	10	2	5
3	10	3	10
explain format=tree select * from complex_view as d_1, complex_view as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize  (cost=4.83..6.88 rows=5)
            -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                    -> PX Receiver (slice: 0; workers: 1)  (cost=0.75 rows=5)
                        -> PX Sender (slice: 1; workers: 4)  (cost=0.75 rows=5)
                            -> Parallel table scan on t1  (cost=0.75 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from complex_view as d_1, complex_view as d_2;
b	b
10	10
10	2
10	3
10	4
10	5
2	10
2	2
2	3
2	4
2	5
3	10
3	2
3	3
3	4
3	5
4	10
4	2
4	3
4	4
4	5
5	10
5	2
5	3
5	4
5	5
set @@txsql_convert_view_to_cte_enabled = 1;
explain format=tree select * from simple_view as d_1, simple_view as d_2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)  (cost=3.50 rows=25)
    -> PX Sender (slice: 1; workers: 4)  (cost=3.50 rows=25)
        -> Inner hash join (no condition)  (cost=3.50 rows=25)
            -> Parallel table scan on t1  (cost=0.15 rows=5)
            -> Hash
                -> Table scan on t1  (cost=0.75 rows=5)

select * from simple_view as d_1, simple_view as d_2;
a	b	a	b
1	2	1	2
1	2	1	3
1	2	2	4
1	2	2	5
1	2	3	10
1	3	1	2
1	3	1	3
1	3	2	4
1	3	2	5
1	3	3	10
2	4	1	2
2	4	1	3
2	4	2	4
2	4	2	5
2	4	3	10
2	5	1	2
2	5	1	3
2	5	2	4
2	5	2	5
2	5	3	10
3	10	1	2
3	10	1	3
3	10	2	4
3	10	2	5
3	10	3	10
explain format=tree select * from complex_view as d_1, complex_view as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize CTE txsql cte if needed (query plan printed elsewhere)  (cost=4.83..6.88 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize CTE txsql cte if needed  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from complex_view as d_1, complex_view as d_2;
b	b
10	10
10	2
10	3
10	4
10	5
2	10
2	2
2	3
2	4
2	5
3	10
3	2
3	3
3	4
3	5
4	10
4	2
4	3
4	4
4	5
5	10
5	2
5	3
5	4
5	5
set @@txsql_convert_view_to_cte_enabled = 0;
explain format=tree select * from (select * from t1) as d_1, (select * from t1) as d_2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)  (cost=3.50 rows=25)
    -> PX Sender (slice: 1; workers: 4)  (cost=3.50 rows=25)
        -> Inner hash join (no condition)  (cost=3.50 rows=25)
            -> Parallel table scan on t1  (cost=0.15 rows=5)
            -> Hash
                -> Table scan on t1  (cost=0.75 rows=5)

select * from (select * from t1) as d_1, (select * from t1) as d_2;
a	b	a	b
1	2	1	2
1	2	1	3
1	2	2	4
1	2	2	5
1	2	3	10
1	3	1	2
1	3	1	3
1	3	2	4
1	3	2	5
1	3	3	10
2	4	1	2
2	4	1	3
2	4	2	4
2	4	2	5
2	4	3	10
2	5	1	2
2	5	1	3
2	5	2	4
2	5	2	5
2	5	3	10
3	10	1	2
3	10	1	3
3	10	2	4
3	10	2	5
3	10	3	10
explain format=tree select * from (select b from t1 group by b) as d_1, (select b from t1 group by b) as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize  (cost=4.83..6.88 rows=5)
            -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                    -> PX Receiver (slice: 0; workers: 1)  (cost=0.75 rows=5)
                        -> PX Sender (slice: 1; workers: 4)  (cost=0.75 rows=5)
                            -> Parallel table scan on t1  (cost=0.75 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from (select b from t1 group by b) as d_1, (select b from t1 group by b) as d_2;
b	b
10	10
10	2
10	3
10	4
10	5
2	10
2	2
2	3
2	4
2	5
3	10
3	2
3	3
3	4
3	5
4	10
4	2
4	3
4	4
4	5
5	10
5	2
5	3
5	4
5	5
set @@txsql_convert_view_to_cte_enabled = 1;
explain format=tree select * from (select * from t1) as d_1, (select * from t1) as d_2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)  (cost=3.50 rows=25)
    -> PX Sender (slice: 1; workers: 4)  (cost=3.50 rows=25)
        -> Inner hash join (no condition)  (cost=3.50 rows=25)
            -> Parallel table scan on t1  (cost=0.15 rows=5)
            -> Hash
                -> Table scan on t1  (cost=0.75 rows=5)

select * from (select * from t1) as d_1, (select * from t1) as d_2;
a	b	a	b
1	2	1	2
1	2	1	3
1	2	2	4
1	2	2	5
1	2	3	10
1	3	1	2
1	3	1	3
1	3	2	4
1	3	2	5
1	3	3	10
2	4	1	2
2	4	1	3
2	4	2	4
2	4	2	5
2	4	3	10
2	5	1	2
2	5	1	3
2	5	2	4
2	5	2	5
2	5	3	10
3	10	1	2
3	10	1	3
3	10	2	4
3	10	2	5
3	10	3	10
explain format=tree select * from (select b from t1 group by b) as d_1, (select b from t1 group by b) as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize  (cost=4.83..6.88 rows=5)
            -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                    -> PX Receiver (slice: 0; workers: 1)  (cost=0.75 rows=5)
                        -> PX Sender (slice: 1; workers: 4)  (cost=0.75 rows=5)
                            -> Parallel table scan on t1  (cost=0.75 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

select * from (select b from t1 group by b) as d_1, (select b from t1 group by b) as d_2;
b	b
10	10
10	2
10	3
10	4
10	5
2	10
2	2
2	3
2	4
2	5
3	10
3	2
3	3
3	4
3	5
4	10
4	2
4	3
4	4
4	5
5	10
5	2
5	3
5	4
5	5
set @@txsql_convert_view_to_cte_enabled = 0;
explain format=tree
with simple_view as (select * from t1)
select * from simple_view as d_1, simple_view as d_2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)  (cost=3.50 rows=25)
    -> PX Sender (slice: 1; workers: 4)  (cost=3.50 rows=25)
        -> Inner hash join (no condition)  (cost=3.50 rows=25)
            -> Parallel table scan on t1  (cost=0.15 rows=5)
            -> Hash
                -> Table scan on t1  (cost=0.75 rows=5)

with complex_view as (select * from t1)
select * from simple_view as d_1, simple_view as d_2;
a	b	a	b
3	10	1	2
2	5	1	2
2	4	1	2
1	3	1	2
1	2	1	2
3	10	1	3
2	5	1	3
2	4	1	3
1	3	1	3
1	2	1	3
3	10	2	5
2	5	2	5
2	4	2	5
1	3	2	5
1	2	2	5
3	10	3	10
2	5	3	10
2	4	3	10
1	3	3	10
1	2	3	10
3	10	2	4
2	5	2	4
2	4	2	4
1	3	2	4
1	2	2	4
explain format=tree
with simple_view as (select b from t1 group by b)
select * from simple_view as d_1, simple_view as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize CTE simple_view if needed (query plan printed elsewhere)  (cost=4.83..6.88 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize CTE simple_view if needed  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

with simple_view as (select b from t1 group by b)
select * from simple_view as d_1, simple_view as d_2;
b	b
10	3
4	3
2	3
5	3
3	3
10	5
4	5
2	5
5	5
3	5
10	2
4	2
2	2
5	2
3	2
10	4
4	4
2	4
5	4
3	4
10	10
4	10
2	10
5	10
3	10
set @@txsql_convert_view_to_cte_enabled = 1;
explain format=tree
with simple_view as (select * from t1)
select * from simple_view as d_1, simple_view as d_2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)  (cost=3.50 rows=25)
    -> PX Sender (slice: 1; workers: 4)  (cost=3.50 rows=25)
        -> Inner hash join (no condition)  (cost=3.50 rows=25)
            -> Parallel table scan on t1  (cost=0.15 rows=5)
            -> Hash
                -> Table scan on t1  (cost=0.75 rows=5)

with complex_view as (select * from t1)
select * from simple_view as d_1, simple_view as d_2;
a	b	a	b
3	10	1	2
2	5	1	2
2	4	1	2
1	3	1	2
1	2	1	2
3	10	1	3
2	5	1	3
2	4	1	3
1	3	1	3
1	2	1	3
3	10	2	5
2	5	2	5
2	4	2	5
1	3	2	5
1	2	2	5
3	10	2	4
2	5	2	4
2	4	2	4
1	3	2	4
1	2	2	4
3	10	3	10
2	5	3	10
2	4	3	10
1	3	3	10
1	2	3	10
explain format=tree
with simple_view as (select b from t1 group by b)
select * from simple_view as d_1, simple_view as d_2;
EXPLAIN
-> Inner hash join (no condition)  (cost=11.94 rows=25)
    -> Table scan on d_2  (cost=0.51..2.56 rows=5)
        -> Materialize CTE simple_view if needed (query plan printed elsewhere)  (cost=4.83..6.88 rows=5)
    -> Hash
        -> Table scan on d_1  (cost=0.51..2.56 rows=5)
            -> Materialize CTE simple_view if needed  (cost=4.83..6.88 rows=5)
                -> Table scan on <temporary>  (cost=0.51..2.56 rows=5)
                    -> Temporary table with deduplication  (cost=1.76..3.81 rows=5)
                        -> PX Receiver (slice: 2; workers: 1)  (cost=0.75 rows=5)
                            -> PX Sender (slice: 3; workers: 4)  (cost=0.75 rows=5)
                                -> Parallel table scan on t1  (cost=0.75 rows=5)

with simple_view as (select b from t1 group by b)
select * from simple_view as d_1, simple_view as d_2;
b	b
10	2
4	2
5	2
3	2
2	2
10	3
4	3
5	3
3	3
2	3
10	5
4	5
5	5
3	5
2	5
10	4
4	4
5	4
3	4
2	4
10	10
4	10
5	10
3	10
2	10
# Load data ...
# Create indexes ...
# Analyze tables ...
Table	Op	Msg_type	Msg_text
test.customer	analyze	status	OK
test.lineitem	analyze	status	OK
test.nation	analyze	status	OK
test.orders	analyze	status	OK
test.part	analyze	status	OK
test.partsupp	analyze	status	OK
test.region	analyze	status	OK
test.supplier	analyze	status	OK
create view revenue0 (supplier_no, total_revenue) as
select
l_suppkey,
sum(l_extendedprice * (1 - l_discount))
from
lineitem
where
l_shipdate >= '1995-06-01'
		and l_shipdate < date_add('1995-06-01', interval '90' day)
group by
l_suppkey;
set @@txsql_convert_view_to_cte_enabled = 0;
explain format=tree select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
revenue0
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
revenue0
)
order by
s_suppkey;
EXPLAIN
-> Sort: supplier.s_suppkey
    -> Stream results  (cost=# rows=10)
        -> Nested loop inner join  (cost=# rows=10)
            -> Filter: (revenue0.total_revenue = (select #2))  (cost=#..13.30 rows=10)
                -> Table scan on revenue0  (cost=#..2.50 rows=0)
                    -> Materialize  (cost=#..2.50 rows=0)
                        -> Table scan on <temporary>
                            -> Final Aggregate using temporary table
                                -> PX Receiver (slice: 0; workers: 1)
                                    -> PX Sender (slice: 1; workers: 2)
                                        -> Table scan on <temporary>
                                            -> Aggregate using temporary table
                                                -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=# rows=96)
                -> Select #2 (subquery in condition; run only once)
                    -> Aggregate: max(revenue0.total_revenue)  (cost=#..2.50 rows=1)
                        -> Table scan on revenue0  (cost=#..2.50 rows=0)
                            -> Materialize  (cost=#..2.50 rows=0)
                                -> Table scan on <temporary>
                                    -> Final Aggregate using temporary table
                                        -> PX Receiver (slice: 2; workers: 1)
                                            -> PX Sender (slice: 3; workers: 2)
                                                -> Table scan on <temporary>
                                                    -> Aggregate using temporary table
                                                        -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=# rows=96)
            -> Single-row index lookup on supplier using PRIMARY (s_suppkey=revenue0.supplier_no)  (cost=# rows=1)

explain analyze select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
revenue0
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
revenue0
)
order by
s_suppkey;
EXPLAIN
-> Sort: supplier.s_suppkey  (actual time=X...)
    -> Stream results  (cost=16.66 rows=10) (actual time=X...)
        -> Nested loop inner join  (cost=16.66 rows=10) (actual time=X...)
            -> Filter: (revenue0.total_revenue = (select #2))  (cost=1.39..13.30 rows=10) (actual time=X...)
                -> Table scan on revenue0  (cost=2.50..2.50 rows=0) (actual time=X...)
                    -> Materialize  (cost=2.50..2.50 rows=0) (actual time=X...)
                        -> Table scan on <temporary>  (actual time=X...)
                            -> Final Aggregate using temporary table  (actual time=X...)
                                -> PX Receiver (slice: 0; workers: 1)  (actual time=X...)
                                    -> PX Sender (slice: 1; workers: 2)  (actual time=X...)
                                        -> Table scan on <temporary>  (actual time=X...)
                                            -> Aggregate using temporary table  (actual time=X...)
                                                -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=43.46 rows=96) (actual time=X...)
                -> Select #2 (subquery in condition; run only once)
                    -> Aggregate: max(revenue0.total_revenue)  (cost=2.50..2.50 rows=1) (actual time=X...)
                        -> Table scan on revenue0  (cost=2.50..2.50 rows=0) (actual time=X...)
                            -> Materialize  (cost=2.50..2.50 rows=0) (actual time=X...)
                                -> Table scan on <temporary>  (actual time=X...)
                                    -> Final Aggregate using temporary table  (actual time=X...)
                                        -> PX Receiver (slice: 2; workers: 1)  (actual time=X...)
                                            -> PX Sender (slice: 3; workers: 2)  (actual time=X...)
                                                -> Table scan on <temporary>  (actual time=X...)
                                                    -> Aggregate using temporary table  (actual time=X...)
                                                        -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=43.46 rows=96) (actual time=X...)
            -> Single-row index lookup on supplier using PRIMARY (s_suppkey=revenue0.supplier_no)  (cost=0.26 rows=1) (actual time=X...)

select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
revenue0
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
revenue0
)
order by
s_suppkey;
s_suppkey	s_name	s_address	s_phone	total_revenue
46	Supplier#000000046	e0URUXfDOYMdKe16Z5h5StMRbzGmTs,D2cjap	34-748-308-3215	128609.5280
set @@txsql_convert_view_to_cte_enabled = 1;
explain format=tree select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
revenue0
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
revenue0
)
order by
s_suppkey;
EXPLAIN
-> Sort: supplier.s_suppkey
    -> Stream results  (cost=# rows=10)
        -> Nested loop inner join  (cost=# rows=10)
            -> Filter: (revenue0.total_revenue = (select #2))  (cost=#..13.30 rows=10)
                -> Table scan on revenue0  (cost=#..2.50 rows=0)
                    -> Materialize CTE txsql cte if needed  (cost=#..2.50 rows=0)
                        -> Table scan on <temporary>
                            -> Final Aggregate using temporary table
                                -> PX Receiver (slice: 0; workers: 1)
                                    -> PX Sender (slice: 1; workers: 2)
                                        -> Table scan on <temporary>
                                            -> Aggregate using temporary table
                                                -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=# rows=96)
                -> Select #2 (subquery in condition; run only once)
                    -> Aggregate: max(revenue0.total_revenue)  (cost=#..2.50 rows=1)
                        -> Table scan on revenue0  (cost=#..2.50 rows=0)
                            -> Materialize CTE txsql cte if needed (query plan printed elsewhere)  (cost=#..2.50 rows=0)
            -> Single-row index lookup on supplier using PRIMARY (s_suppkey=revenue0.supplier_no)  (cost=# rows=1)

explain analyze select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
revenue0
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
revenue0
)
order by
s_suppkey;
EXPLAIN
-> Sort: supplier.s_suppkey  (actual time=X...)
    -> Stream results  (cost=16.66 rows=10) (actual time=X...)
        -> Nested loop inner join  (cost=16.66 rows=10) (actual time=X...)
            -> Filter: (revenue0.total_revenue = (select #2))  (cost=1.39..13.30 rows=10) (actual time=X...)
                -> Table scan on revenue0  (cost=2.50..2.50 rows=0) (actual time=X...)
                    -> Materialize CTE txsql cte if needed  (cost=2.50..2.50 rows=0) (actual time=X...)
                        -> Table scan on <temporary>  (actual time=X...)
                            -> Final Aggregate using temporary table  (actual time=X...)
                                -> PX Receiver (slice: 0; workers: 1)  (actual time=X...)
                                    -> PX Sender (slice: 1; workers: 2)  (actual time=X...)
                                        -> Table scan on <temporary>  (actual time=X...)
                                            -> Aggregate using temporary table  (actual time=X...)
                                                -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=43.46 rows=96) (actual time=X...)
                -> Select #2 (subquery in condition; run only once)
                    -> Aggregate: max(revenue0.total_revenue)  (cost=2.50..2.50 rows=1) (actual time=X...)
                        -> Table scan on revenue0  (cost=2.50..2.50 rows=0) (actual time=X...)
                            -> Materialize CTE txsql cte if needed  (cost=2.50..2.50 rows=0) (actual time=X...)
                                -> Table scan on <temporary>  (never executed)
                                    -> Final Aggregate using temporary table  (never executed)
                                        -> PX Receiver (slice: 0; workers: 0)  (never executed)
                                            -> PX Sender (slice: 0; workers: 0)  (never executed)
                                                -> Table scan on <temporary>  (never executed)
                                                    -> Aggregate using temporary table  (never executed)
                                                        -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=43.46 rows=96) (never executed)
            -> Single-row index lookup on supplier using PRIMARY (s_suppkey=revenue0.supplier_no)  (cost=0.26 rows=1) (actual time=X...)

select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
revenue0
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
revenue0
)
order by
s_suppkey;
s_suppkey	s_name	s_address	s_phone	total_revenue
46	Supplier#000000046	e0URUXfDOYMdKe16Z5h5StMRbzGmTs,D2cjap	34-748-308-3215	128609.5280
set @@txsql_convert_view_to_cte_enabled = 0;
explain format=tree with cte_revenue0 (supplier_no, total_revenue) as (
select
l_suppkey,
sum(l_extendedprice * (1 - l_discount))
from
lineitem
where
l_shipdate >= '1995-06-01'
		and l_shipdate < date_add('1995-06-01', interval '90' day)
group by
l_suppkey)
select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
cte_revenue0 as t1
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
cte_revenue0 as t2
)
order by
s_suppkey;
EXPLAIN
-> Sort: supplier.s_suppkey
    -> Stream results  (cost=# rows=10)
        -> Nested loop inner join  (cost=# rows=10)
            -> Filter: (t1.total_revenue = (select #3))  (cost=#..13.30 rows=10)
                -> Table scan on t1  (cost=#..2.50 rows=0)
                    -> Materialize CTE cte_revenue0 if needed  (cost=#..2.50 rows=0)
                        -> Table scan on <temporary>
                            -> Final Aggregate using temporary table
                                -> PX Receiver (slice: 2; workers: 1)
                                    -> PX Sender (slice: 3; workers: 2)
                                        -> Table scan on <temporary>
                                            -> Aggregate using temporary table
                                                -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=# rows=96)
                -> Select #3 (subquery in condition; run only once)
                    -> Aggregate: max(t2.total_revenue)  (cost=#..2.50 rows=1)
                        -> Table scan on t2  (cost=#..2.50 rows=0)
                            -> Materialize CTE cte_revenue0 if needed (query plan printed elsewhere)  (cost=#..2.50 rows=0)
            -> Single-row index lookup on supplier using PRIMARY (s_suppkey=t1.supplier_no)  (cost=# rows=1)

explain analyze with cte_revenue0 (supplier_no, total_revenue) as (
select
l_suppkey,
sum(l_extendedprice * (1 - l_discount))
from
lineitem
where
l_shipdate >= '1995-06-01'
		and l_shipdate < date_add('1995-06-01', interval '90' day)
group by
l_suppkey)
select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
cte_revenue0 as t1
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
cte_revenue0 as t2
)
order by
s_suppkey;
EXPLAIN
-> Sort: supplier.s_suppkey  (actual time=X...)
    -> Stream results  (cost=16.66 rows=10) (actual time=X...)
        -> Nested loop inner join  (cost=16.66 rows=10) (actual time=X...)
            -> Filter: (t1.total_revenue = (select #3))  (cost=1.39..13.30 rows=10) (actual time=X...)
                -> Table scan on t1  (cost=2.50..2.50 rows=0) (actual time=X...)
                    -> Materialize CTE cte_revenue0 if needed  (cost=2.50..2.50 rows=0) (actual time=X...)
                        -> Table scan on <temporary>  (never executed)
                            -> Final Aggregate using temporary table  (never executed)
                                -> PX Receiver (slice: 0; workers: 0)  (never executed)
                                    -> PX Sender (slice: 0; workers: 0)  (never executed)
                                        -> Table scan on <temporary>  (never executed)
                                            -> Aggregate using temporary table  (never executed)
                                                -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=43.46 rows=96) (never executed)
                -> Select #3 (subquery in condition; run only once)
                    -> Aggregate: max(t2.total_revenue)  (cost=2.50..2.50 rows=1) (actual time=X...)
                        -> Table scan on t2  (cost=2.50..2.50 rows=0) (actual time=X...)
                            -> Materialize CTE cte_revenue0 if needed  (cost=2.50..2.50 rows=0) (actual time=X...)
                                -> Table scan on <temporary>  (actual time=X...)
                                    -> Final Aggregate using temporary table  (actual time=X...)
                                        -> PX Receiver (slice: 0; workers: 1)  (actual time=X...)
                                            -> PX Sender (slice: 1; workers: 2)  (actual time=X...)
                                                -> Table scan on <temporary>  (actual time=X...)
                                                    -> Aggregate using temporary table  (actual time=X...)
                                                        -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=43.46 rows=96) (actual time=X...)
            -> Single-row index lookup on supplier using PRIMARY (s_suppkey=t1.supplier_no)  (cost=0.26 rows=1) (actual time=X...)

with cte_revenue0 (supplier_no, total_revenue) as (
select
l_suppkey,
sum(l_extendedprice * (1 - l_discount))
from
lineitem
where
l_shipdate >= '1995-06-01'
		and l_shipdate < date_add('1995-06-01', interval '90' day)
group by
l_suppkey)
select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
cte_revenue0 as t1
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
cte_revenue0 as t2
)
order by
s_suppkey;
s_suppkey	s_name	s_address	s_phone	total_revenue
46	Supplier#000000046	e0URUXfDOYMdKe16Z5h5StMRbzGmTs,D2cjap	34-748-308-3215	128609.5280
set @@txsql_convert_view_to_cte_enabled = 1;
explain format=tree with cte_revenue0 (supplier_no, total_revenue) as (
select
l_suppkey,
sum(l_extendedprice * (1 - l_discount))
from
lineitem
where
l_shipdate >= '1995-06-01'
		and l_shipdate < date_add('1995-06-01', interval '90' day)
group by
l_suppkey)
select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
cte_revenue0 as t1
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
cte_revenue0 as t2
)
order by
s_suppkey;
EXPLAIN
-> Sort: supplier.s_suppkey
    -> Stream results  (cost=# rows=10)
        -> Nested loop inner join  (cost=# rows=10)
            -> Filter: (t1.total_revenue = (select #3))  (cost=#..13.30 rows=10)
                -> Table scan on t1  (cost=#..2.50 rows=0)
                    -> Materialize CTE cte_revenue0 if needed  (cost=#..2.50 rows=0)
                        -> Table scan on <temporary>
                            -> Final Aggregate using temporary table
                                -> PX Receiver (slice: 2; workers: 1)
                                    -> PX Sender (slice: 3; workers: 2)
                                        -> Table scan on <temporary>
                                            -> Aggregate using temporary table
                                                -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=# rows=96)
                -> Select #3 (subquery in condition; run only once)
                    -> Aggregate: max(t2.total_revenue)  (cost=#..2.50 rows=1)
                        -> Table scan on t2  (cost=#..2.50 rows=0)
                            -> Materialize CTE cte_revenue0 if needed (query plan printed elsewhere)  (cost=#..2.50 rows=0)
            -> Single-row index lookup on supplier using PRIMARY (s_suppkey=t1.supplier_no)  (cost=# rows=1)

explain analyze with cte_revenue0 (supplier_no, total_revenue) as (
select
l_suppkey,
sum(l_extendedprice * (1 - l_discount))
from
lineitem
where
l_shipdate >= '1995-06-01'
		and l_shipdate < date_add('1995-06-01', interval '90' day)
group by
l_suppkey)
select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
cte_revenue0 as t1
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
cte_revenue0 as t2
)
order by
s_suppkey;
EXPLAIN
-> Sort: supplier.s_suppkey  (actual time=X...)
    -> Stream results  (cost=16.66 rows=10) (actual time=X...)
        -> Nested loop inner join  (cost=16.66 rows=10) (actual time=X...)
            -> Filter: (t1.total_revenue = (select #3))  (cost=1.39..13.30 rows=10) (actual time=X...)
                -> Table scan on t1  (cost=2.50..2.50 rows=0) (actual time=X...)
                    -> Materialize CTE cte_revenue0 if needed  (cost=2.50..2.50 rows=0) (actual time=X...)
                        -> Table scan on <temporary>  (never executed)
                            -> Final Aggregate using temporary table  (never executed)
                                -> PX Receiver (slice: 0; workers: 0)  (never executed)
                                    -> PX Sender (slice: 0; workers: 0)  (never executed)
                                        -> Table scan on <temporary>  (never executed)
                                            -> Aggregate using temporary table  (never executed)
                                                -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=43.46 rows=96) (never executed)
                -> Select #3 (subquery in condition; run only once)
                    -> Aggregate: max(t2.total_revenue)  (cost=2.50..2.50 rows=1) (actual time=X...)
                        -> Table scan on t2  (cost=2.50..2.50 rows=0) (actual time=X...)
                            -> Materialize CTE cte_revenue0 if needed  (cost=2.50..2.50 rows=0) (actual time=X...)
                                -> Table scan on <temporary>  (actual time=X...)
                                    -> Final Aggregate using temporary table  (actual time=X...)
                                        -> PX Receiver (slice: 0; workers: 1)  (actual time=X...)
                                            -> PX Sender (slice: 1; workers: 2)  (actual time=X...)
                                                -> Table scan on <temporary>  (actual time=X...)
                                                    -> Aggregate using temporary table  (actual time=X...)
                                                        -> Parallel index range scan on lineitem using i_l_shipdate over ('1995-06-01' <= l_shipdate < '1995-08-30'), with index condition: ((lineitem.l_shipdate >= DATE'1995-06-01') and (lineitem.l_shipdate < <cache>(('1995-06-01' + interval '90' day))))  (cost=43.46 rows=96) (actual time=X...)
            -> Single-row index lookup on supplier using PRIMARY (s_suppkey=t1.supplier_no)  (cost=0.26 rows=1) (actual time=X...)

with cte_revenue0 (supplier_no, total_revenue) as (
select
l_suppkey,
sum(l_extendedprice * (1 - l_discount))
from
lineitem
where
l_shipdate >= '1995-06-01'
		and l_shipdate < date_add('1995-06-01', interval '90' day)
group by
l_suppkey)
select
s_suppkey,
s_name,
s_address,
s_phone,
total_revenue
from
supplier,
cte_revenue0 as t1
where
s_suppkey = supplier_no
and total_revenue = (
select
max(total_revenue)
from
cte_revenue0 as t2
)
order by
s_suppkey;
s_suppkey	s_name	s_address	s_phone	total_revenue
46	Supplier#000000046	e0URUXfDOYMdKe16Z5h5StMRbzGmTs,D2cjap	34-748-308-3215	128609.5280
drop table t1;
drop view simple_view;
drop view complex_view;
drop view complex_view_2;
drop view complex_view_3;
drop view revenue0;
#
# Test end
#
# Cleanup, drop testing tables ...
START TRANSACTION;
drop TABLE lineitem;
drop TABLE orders;
drop TABLE partsupp;
drop TABLE part;
drop TABLE customer;
drop TABLE supplier;
drop TABLE nation;
drop TABLE region;
COMMIT;
set @@txsql_convert_view_to_cte_enabled = @saved_value;

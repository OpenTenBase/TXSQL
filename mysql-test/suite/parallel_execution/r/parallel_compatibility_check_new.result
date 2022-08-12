# --include save_and_set_px.inc
#
# Create TABLE with fields of compatible types.
#
create table t1(id int, col1 int, col2 int, primary key(id), key idx(col2));
insert into t1 values(1,1,1);
insert into t1 values(2,2,2);
insert into t1 values(3,3,3);
insert into t1 values(4,4,4);
insert into t1 values(5,5,5);
insert into t1 values(6,6,6);
insert into t1 values(7,7,7);
insert into t1 values(8,8,8);
insert into t1 values(9,9,9);
insert into t1 values(10,10,10);
create table t2(id int, col1 int, col2 int, primary key(id), key idx(col2));
insert into t2 values(1,1,1);
insert into t2 values(2,2,2);
insert into t2 values(3,3,3);
insert into t2 values(4,4,4);
insert into t2 values(5,5,5);
insert into t2 values(6,6,6);
insert into t2 values(7,7,7);
insert into t2 values(8,8,8);
insert into t2 values(9,9,9);
insert into t2 values(10,10,10);
analyze table t1,t2;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
#
# Single Query block
#
# Single table
explain format=tree select col1+RAND(1) from t1 order by col1;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: t1.col1  (cost=1.25 rows=10)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select col1+RAND(1) from t1 order by col1;
col1+RAND(1)
1.4054035371219773
2.8716141803857074
3.141860321296249
4.0944590960577685
5.046714547133739
6.950195478229034
7.6108337804776
8.203582498434542
9.185411181473556
10.3163084427978
explain format=tree select * from t1 order by col1+RAND(1);
EXPLAIN
-> Sort: `(t1.col1 + rand(1))`
    -> Stream results  (cost=1.25 rows=10)
        -> PX Receiver (slice: 0; workers: 1)
            -> PX Sender (slice: 1; workers: 4)
                -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 order by col1+RAND(1);
id	col1	col2
1	1	1
2	2	2
3	3	3
4	4	4
5	5	5
6	6	6
7	7	7
8	8	8
9	9	9
10	10	10
explain format=tree select sum(col1+RAND(1)) from t1;
EXPLAIN
-> Aggregate: sum((t1.col1 + rand(1)))
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select sum(col1+RAND(1)) from t1;
sum(col1+RAND(1))
58.82638306340797
explain format=tree select sum(col1)+RAND(1) from t1;
EXPLAIN
-> Aggregate: sum(t1.col1)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select sum(col1)+RAND(1) from t1;
sum(col1)+RAND(1)
55.40540353712198
explain format=tree select * from t1 where RAND(1)>1;
EXPLAIN
-> Filter: (rand(1) > 1)  (cost=1.25 rows=10)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 where RAND(1)>1;
id	col1	col2
explain format=tree select sum(col1) + RAND(1) from t1 group by col2 order by col2;
EXPLAIN
-> Group aggregate: sum(t1.col1)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel index scan on t1 using idx  (cost=1.25 rows=10)

select sum(col1) + RAND(1) from t1 group by col2 order by col2;
sum(col1) + RAND(1)
1.4054035371219773
2.8716141803857074
3.141860321296249
4.0944590960577685
5.046714547133739
6.950195478229034
7.6108337804776
8.203582498434542
9.185411181473556
10.3163084427978
explain format=tree select sum(col1) + RAND(1) from t1 group by col2 order by sum(col1);
EXPLAIN
-> Sort: `sum(t1.col1)`
    -> Stream results
        -> Group aggregate: sum(t1.col1), sum(t1.col1)
            -> PX Receiver (merge sort; slice: 0; workers: 1)
                -> PX Sender (slice: 1; workers: 4)
                    -> Parallel index scan on t1 using idx  (cost=1.25 rows=10)

select sum(col1) + RAND(1) from t1 group by col2 order by sum(col1);
sum(col1) + RAND(1)
1.4054035371219773
2.8716141803857074
3.141860321296249
4.0944590960577685
5.046714547133739
6.950195478229034
7.6108337804776
8.203582498434542
9.185411181473556
10.3163084427978
explain format=tree select sum(col1) from t1 group by col2 having RAND(1) > 1 order by sum(col1);
EXPLAIN
-> Sort: `sum(col1)`
    -> Filter: (rand(1) > 1)
        -> Stream results
            -> Group aggregate: sum(t1.col1)
                -> PX Receiver (merge sort; slice: 0; workers: 1)
                    -> PX Sender (slice: 1; workers: 4)
                        -> Group aggregate: sum(t1.col1)
                            -> Parallel index scan on t1 using idx  (cost=1.25 rows=10)

select sum(col1) from t1 group by col2 having RAND(1) > 1 order by sum(col1);
sum(col1)
# Multiple tables
explain format=tree select t1.col1, RAND(1) from t1 join t2 on t1.col2=t2.col2 order by t1.col1;
EXPLAIN
-> Nested loop inner join  (cost=4.75 rows=10)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Sort: t1.col1  (cost=1.25 rows=10)
                -> Filter: (t1.col2 is not null)  (cost=1.25 rows=10)
                    -> Parallel table scan on t1  (cost=1.25 rows=10)
    -> Index lookup on t2 using idx (col2=t1.col2)  (cost=0.26 rows=1)

select t1.col1, RAND(1) from t1 join t2 on t1.col2=t2.col2 order by t1.col1;
col1	RAND(1)
1	0.40540353712197724
2	0.8716141803857071
3	0.1418603212962489
4	0.09445909605776807
5	0.04671454713373868
6	0.9501954782290342
7	0.6108337804776
8	0.2035824984345422
9	0.18541118147355615
10	0.31630844279779907
explain format=tree select t1.col1, RAND(1) from t1 join t2 on t1.id=t2.id order by t1.col1;
EXPLAIN
-> Nested loop inner join  (cost=4.75 rows=10)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Sort: t1.col1  (cost=1.25 rows=10)
                -> Parallel table scan on t1  (cost=1.25 rows=10)
    -> Single-row index lookup on t2 using PRIMARY (id=t1.id)  (cost=0.26 rows=1)

select t1.col1, RAND(1) from t1 join t2 on t1.id=t2.id order by t1.col1;
col1	RAND(1)
1	0.40540353712197724
2	0.8716141803857071
3	0.1418603212962489
4	0.09445909605776807
5	0.04671454713373868
6	0.9501954782290342
7	0.6108337804776
8	0.2035824984345422
9	0.18541118147355615
10	0.31630844279779907
# Hash Join
explain format=tree select * from t1 join t2 join t1 as t3 where t1.id = t2.id and t2.col1 = t3.col1;
EXPLAIN
-> Nested loop inner join  (cost=12.75 rows=10)
    -> Inner hash join (t2.col1 = t3.col1)  (cost=11.50 rows=10)
        -> Table scan on t2  (cost=0.04 rows=10)
        -> Hash
            -> PX Receiver (slice: 0; workers: 1)
                -> PX Sender (slice: 1; workers: 4)
                    -> Parallel table scan on t3  (cost=1.25 rows=10)
    -> Single-row index lookup on t1 using PRIMARY (id=t2.id)  (cost=0.03 rows=1)

select * from t1 join t2 join t1 as t3 where t1.id = t2.id and t2.col1 = t3.col1;
id	col1	col2	id	col1	col2	id	col1	col2
1	1	1	1	1	1	1	1	1
10	10	10	10	10	10	10	10	10
2	2	2	2	2	2	2	2	2
3	3	3	3	3	3	3	3	3
4	4	4	4	4	4	4	4	4
5	5	5	5	5	5	5	5	5
6	6	6	6	6	6	6	6	6
7	7	7	7	7	7	7	7	7
8	8	8	8	8	8	8	8	8
9	9	9	9	9	9	9	9	9
explain format=tree select * from t1 straight_join t2 join t1 as t3 where t1.id = t2.id and t2.col1 = t3.col1;
EXPLAIN
-> Inner hash join (t3.col1 = t2.col1)  (cost=15.00 rows=10)
    -> Table scan on t3  (cost=0.04 rows=10)
    -> Hash
        -> PX Receiver (slice: 0; workers: 1)
            -> PX Sender (slice: 1; workers: 4)
                -> Nested loop inner join  (cost=4.75 rows=10)
                    -> Parallel table scan on t1  (cost=1.25 rows=10)
                    -> Single-row index lookup on t2 using PRIMARY (id=t1.id)  (cost=0.26 rows=1)

select * from t1 straight_join t2 join t1 as t3 where t1.id = t2.id and t2.col1 = t3.col1;
id	col1	col2	id	col1	col2	id	col1	col2
1	1	1	1	1	1	1	1	1
10	10	10	10	10	10	10	10	10
2	2	2	2	2	2	2	2	2
3	3	3	3	3	3	3	3	3
4	4	4	4	4	4	4	4	4
5	5	5	5	5	5	5	5	5
6	6	6	6	6	6	6	6	6
7	7	7	7	7	7	7	7	7
8	8	8	8	8	8	8	8	8
9	9	9	9	9	9	9	9	9
# LIMIT
explain format=tree select * from t1 limit 1;
EXPLAIN
-> Limit: 1 row(s)  (cost=1.25 rows=1)
    -> Table scan on t1  (cost=1.25 rows=10)

select * from t1 limit 1;
id	col1	col2
1	1	1
explain format=tree select * from t1 order by col1 limit 1;
EXPLAIN
-> Limit: 1 row(s)  (cost=1.25 rows=1)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Sort: t1.col1, limit input to 1 row(s) per chunk  (cost=1.25 rows=10)
                -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 order by col1 limit 1;
id	col1	col2
1	1	1
explain format=tree select col2 from t1 order by col2 limit 1;
EXPLAIN
-> Limit: 1 row(s)  (cost=0.03 rows=1)
    -> Index scan on t1 using idx  (cost=0.03 rows=1)

select col2 from t1 order by col2 limit 1;
col2
1
explain format=tree select distinct col1 from t1 limit 1;
EXPLAIN
-> Limit: 1 row(s)  (cost=1.25 rows=1)
    -> Table scan on <temporary>
        -> Temporary table with deduplication  (cost=1.25 rows=10)
            -> Limit table size: 1 unique row(s)
                -> Table scan on t1  (cost=1.25 rows=10)

select distinct col1 from t1 limit 1;
col1
1
#
# Union
#
explain format=tree select * from t1 union select * from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=1.25 rows=10)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=1.25 rows=10)

select * from t1 union select * from t2;
id	col1	col2
1	1	1
10	10	10
2	2	2
3	3	3
4	4	4
5	5	5
6	6	6
7	7	7
8	8	8
9	9	9
explain format=tree select * from t1 union all select * from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=1.25 rows=10)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=1.25 rows=10)
            -> Stream results  (cost=1.25 rows=10)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=1.25 rows=10)

select * from t1 union all select * from t2;
id	col1	col2
1	1	1
1	1	1
10	10	10
10	10	10
2	2	2
2	2	2
3	3	3
3	3	3
4	4	4
4	4	4
5	5	5
5	5	5
6	6	6
6	6	6
7	7	7
7	7	7
8	8	8
8	8	8
9	9	9
9	9	9
explain format=tree select * from t1 union (select * from t2 where id > RAND(1));
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=1.25 rows=10)
                -> Filter: (t2.id > rand(1))  (cost=0.58 rows=3)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=0.58 rows=10)

select * from t1 union all (select * from t2 where id > RAND(1));
id	col1	col2
1	1	1
1	1	1
10	10	10
10	10	10
2	2	2
2	2	2
3	3	3
3	3	3
4	4	4
4	4	4
5	5	5
5	5	5
6	6	6
6	6	6
7	7	7
7	7	7
8	8	8
8	8	8
9	9	9
9	9	9
explain format=tree (select * from t1 where id > RAND(1)) union (select * from t2 where id > RAND(1));
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> Filter: (t1.id > rand(1))  (cost=0.58 rows=3)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 4)
                            -> Parallel table scan on t1  (cost=0.58 rows=10)
                -> Filter: (t2.id > rand(1))  (cost=0.58 rows=3)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=0.58 rows=10)

(select * from t1 where id > RAND(1)) union all (select * from t2 where id > RAND(1));
id	col1	col2
1	1	1
1	1	1
10	10	10
10	10	10
2	2	2
2	2	2
3	3	3
3	3	3
4	4	4
4	4	4
5	5	5
5	5	5
6	6	6
6	6	6
7	7	7
7	7	7
8	8	8
8	8	8
9	9	9
9	9	9
explain format=tree select * from t1 union all (select * from t2 where id > RAND(1));
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=1.25 rows=10)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=1.25 rows=10)
            -> Stream results  (cost=0.58 rows=3)
                -> Filter: (t2.id > rand(1))  (cost=0.58 rows=3)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=0.58 rows=10)

select * from t1 union all (select * from t2 where id > RAND(1));
id	col1	col2
1	1	1
1	1	1
10	10	10
10	10	10
2	2	2
2	2	2
3	3	3
3	3	3
4	4	4
4	4	4
5	5	5
5	5	5
6	6	6
6	6	6
7	7	7
7	7	7
8	8	8
8	8	8
9	9	9
9	9	9
explain format=tree (select * from t1 where id > RAND(1)) union all (select * from t2 where id > RAND(1));
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.58 rows=3)
                -> Filter: (t1.id > rand(1))  (cost=0.58 rows=3)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 4)
                            -> Parallel table scan on t1  (cost=0.58 rows=10)
            -> Stream results  (cost=0.58 rows=3)
                -> Filter: (t2.id > rand(1))  (cost=0.58 rows=3)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=0.58 rows=10)

(select * from t1 where id > RAND(1)) union all (select * from t2 where id > RAND(1));
id	col1	col2
1	1	1
1	1	1
10	10	10
10	10	10
2	2	2
2	2	2
3	3	3
3	3	3
4	4	4
4	4	4
5	5	5
5	5	5
6	6	6
6	6	6
7	7	7
7	7	7
8	8	8
8	8	8
9	9	9
9	9	9
explain format=tree (SELECT 1 as a) UNION (SELECT 1) ORDER BY (SELECT a+0);
EXPLAIN
-> Sort: (a + 0)
    -> Table scan on <union temporary>  (cost=2.50 rows=0)
        -> Union materialize with deduplication
            -> Rows fetched before execution  (cost=0.00 rows=1)
            -> Rows fetched before execution  (cost=0.00 rows=1)

Warnings:
Note	1276	Field or reference 'a' of SELECT #3 was resolved in SELECT #1
Note	1249	Select 3 was reduced during optimization
(SELECT 1 as a) UNION (SELECT 1) ORDER BY (SELECT a+0);
a
1
# UNION and UNION ALL in one statement
explain format=tree select * from t1 union select * from t2 union all select * from t1;
EXPLAIN
-> Append
    -> Table scan on <union temporary>  (cost=2.50 rows=0)
        -> Union materialize with deduplication
            -> Table scan on t1  (cost=1.25 rows=10)
            -> Table scan on t2  (cost=1.25 rows=10)
    -> Stream results  (cost=1.25 rows=10)
        -> Table scan on t1  (cost=1.25 rows=10)

select * from t1 union select * from t2 union all select * from t1;
id	col1	col2
1	1	1
1	1	1
10	10	10
10	10	10
2	2	2
2	2	2
3	3	3
3	3	3
4	4	4
4	4	4
5	5	5
5	5	5
6	6	6
6	6	6
7	7	7
7	7	7
8	8	8
8	8	8
9	9	9
9	9	9
# LIMIT
explain format=tree select * from t1 union select * from t2 limit 2;
EXPLAIN
-> Limit: 2 row(s)
    -> Table scan on <union temporary>  (cost=2.50 rows=0)
        -> Union materialize with deduplication
            -> Limit table size: 2 unique row(s)
                -> Table scan on t1  (cost=1.25 rows=10)
            -> Limit table size: 2 unique row(s)
                -> Table scan on t2  (cost=1.25 rows=10)

select * from t1 union select * from t2 limit 2;
id	col1	col2
1	1	1
2	2	2
explain format=tree select * from t1 union all select * from t2 limit 2;
EXPLAIN
-> Limit: 2 row(s)
    -> Append
        -> Stream results  (cost=1.25 rows=10)
            -> Table scan on t1  (cost=1.25 rows=10)
        -> Stream results  (cost=1.25 rows=10)
            -> Table scan on t2  (cost=1.25 rows=10)

select * from t1 union all select * from t2 limit 2;
id	col1	col2
1	1	1
2	2	2
#
# Subselect
#
set @old_optimizer_switch = @@optimizer_switch;
set optimizer_switch='semijoin=on,materialization=on,loosescan=off,firstmatch=off,duplicateweedout=off,index_condition_pushdown=off,mrr=off';
explain format=tree select * from t1 where id in (select col1 from t2);
EXPLAIN
-> Nested loop inner join  (cost=3.25 rows=10)
    -> Filter: (`<subquery2>`.col1 is not null)  (cost=2.00 rows=10)
        -> Table scan on <subquery2>  (cost=2.00 rows=10)
            -> Materialize with deduplication  (cost=1.25 rows=10)
                -> PX Receiver (slice: 0; workers: 1)
                    -> PX Sender (slice: 1; workers: 4)
                        -> Filter: (t2.col1 is not null)  (cost=1.25 rows=10)
                            -> Parallel table scan on t2  (cost=1.25 rows=10)
    -> Single-row index lookup on t1 using PRIMARY (id=`<subquery2>`.col1)  (cost=0.35 rows=1)

select * from t1 where id in (select col1 from t2);
id	col1	col2
1	1	1
10	10	10
2	2	2
3	3	3
4	4	4
5	5	5
6	6	6
7	7	7
8	8	8
9	9	9
explain format=tree select * from t1 where id in (select col1 from t2 where col1 > RAND(1));
EXPLAIN
-> Nested loop inner join  (cost=1.18 rows=3)
    -> Filter: (`<subquery2>`.col1 is not null)  (cost=0.63 rows=3)
        -> Table scan on <subquery2>  (cost=0.63 rows=3)
            -> Materialize with deduplication  (cost=0.58 rows=3)
                -> Filter: (t2.col1 is not null)  (cost=0.58 rows=3)
                    -> Filter: (t2.col1 > rand(1))  (cost=0.58 rows=3)
                        -> PX Receiver (slice: 0; workers: 1)
                            -> PX Sender (slice: 1; workers: 4)
                                -> Parallel table scan on t2  (cost=0.58 rows=10)
    -> Single-row index lookup on t1 using PRIMARY (id=`<subquery2>`.col1)  (cost=0.35 rows=1)

select * from t1 where id in (select col1 from t2 where col1 > RAND(1));
id	col1	col2
1	1	1
10	10	10
2	2	2
3	3	3
4	4	4
5	5	5
6	6	6
7	7	7
8	8	8
9	9	9
explain format=tree select * from t1 where col1 in (select col1 from t2 where col1 > RAND(1));
EXPLAIN
-> Nested loop inner join  (cost=5.58 rows=33)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Filter: (t1.col1 is not null)  (cost=1.25 rows=10)
                -> Parallel table scan on t1  (cost=1.25 rows=10)
    -> Single-row index lookup on <subquery2> using <auto_distinct_key> (col1=t1.col1)
        -> Materialize with deduplication  (cost=0.58 rows=3)
            -> Filter: (t2.col1 is not null)  (cost=0.58 rows=3)
                -> Filter: (t2.col1 > rand(1))  (cost=0.58 rows=3)
                    -> Table scan on t2  (cost=0.58 rows=10)

select * from t1 where col1 in (select col1 from t2 where col1 > RAND(1));
id	col1	col2
1	1	1
10	10	10
2	2	2
3	3	3
4	4	4
5	5	5
6	6	6
7	7	7
8	8	8
9	9	9
explain format=tree SELECT SUM(EXISTS(SELECT RAND() FROM t1)) FROM t1;
EXPLAIN
-> Aggregate: sum(exists(select #2))
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel index scan on t1 using idx  (cost=1.25 rows=10)
-> Select #2 (subquery in projection; uncacheable)
    -> Limit: 1 row(s)  (cost=1.25 rows=1)
        -> Index scan on t1 using idx  (cost=1.25 rows=10)

SELECT SUM(EXISTS(SELECT RAND() FROM t1)) FROM t1;
SUM(EXISTS(SELECT RAND() FROM t1))
10
explain format=tree SELECT SUM(EXISTS(SELECT 1 FROM t1)) FROM t1;
EXPLAIN
-> Aggregate: sum(exists(select #2))
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: sum(exists(select #2))
                -> Parallel index scan on t1 using idx  (cost=1.25 rows=10)

SELECT SUM(EXISTS(SELECT 1 FROM t1)) FROM t1;
SUM(EXISTS(SELECT 1 FROM t1))
10
set optimizer_switch = @old_optimizer_switch;
explain format=tree select * from t1 order by (select col1 from t2 limit 1);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel table scan on t1  (cost=1.25 rows=10)
-> Select #2 (subquery in projection; run only once)
    -> Limit: 1 row(s)  (cost=1.25 rows=1)
        -> Table scan on t2  (cost=1.25 rows=10)

select * from t1 order by (select col1 from t2 limit 1);
id	col1	col2
1	1	1
10	10	10
2	2	2
3	3	3
4	4	4
5	5	5
6	6	6
7	7	7
8	8	8
9	9	9
explain format=tree select * from t1 order by (select col1 from t2 where col1 = t1.col1);
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Sort: (select #2)  (cost=1.25 rows=10)
            -> Parallel table scan on t1  (cost=1.25 rows=10)
-> Select #2 (subquery in projection; dependent)
    -> Filter: (t2.col1 = t1.col1)  (cost=0.35 rows=1)
        -> Table scan on t2  (cost=0.35 rows=10)

Warnings:
Note	1276	Field or reference 'test.t1.col1' of SELECT #2 was resolved in SELECT #1
select * from t1 order by (select col1 from t2 where col1 = t1.col1);
id	col1	col2
1	1	1
2	2	2
3	3	3
4	4	4
5	5	5
6	6	6
7	7	7
8	8	8
9	9	9
10	10	10
#
# Derived table
#
explain format=tree select * from (select distinct col1 from t1) as t where t.col1>2;
EXPLAIN
-> Table scan on t  (cost=2.84 rows=3)
    -> Materialize  (cost=1.25 rows=3)
        -> Table scan on <temporary>
            -> Temporary table with deduplication  (cost=1.25 rows=3)
                -> PX Receiver (slice: 0; workers: 1)
                    -> PX Sender (slice: 1; workers: 4)
                        -> Filter: (t1.col1 > 2)  (cost=1.25 rows=3)
                            -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from (select distinct col1 from t1) as t where t.col1>2;
col1
10
3
4
5
6
7
8
9
explain format=tree select * from (select sum(col2) as a from t1 group by col1) as t where t.a>2;
EXPLAIN
-> Table scan on t  (cost=3.62 rows=10)
    -> Materialize
        -> Filter: (sum(t1.col2) > 2)
            -> Table scan on <temporary>
                -> Final Aggregate using temporary table
                    -> PX Receiver (slice: 0; workers: 1)
                        -> PX Sender (slice: 1; workers: 4)
                            -> Table scan on <temporary>
                                -> Aggregate using temporary table
                                    -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from (select sum(col2) as a from t1 group by col1) as t where t.a>2;
a
10
3
4
5
6
7
8
9
explain format=tree select * from t1 join (select distinct col1 from t1) as t where t.col1>2 and t1.id=t.col1;
EXPLAIN
-> Nested loop inner join  (cost=3.89 rows=3)
    -> Filter: (t.col1 is not null)  (cost=2.84 rows=3)
        -> Table scan on t  (cost=2.84 rows=3)
            -> Materialize  (cost=1.25 rows=3)
                -> Table scan on <temporary>
                    -> Temporary table with deduplication  (cost=1.25 rows=3)
                        -> PX Receiver (slice: 0; workers: 1)
                            -> PX Sender (slice: 1; workers: 4)
                                -> Filter: (t1.col1 > 2)  (cost=1.25 rows=3)
                                    -> Parallel table scan on t1  (cost=1.25 rows=10)
    -> Single-row index lookup on t1 using PRIMARY (id=t.col1)  (cost=0.28 rows=1)

select * from t1 join (select distinct col1 from t1) as t where t.col1>2 and t1.id=t.col1;
id	col1	col2	col1
10	10	10	10
3	3	3	3
4	4	4	4
5	5	5	5
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
explain format=tree select * from t1 straight_join (select distinct col1 from t1) as t where t.col1>2 and t1.id=t.col1;
EXPLAIN
-> Inner hash join (t.col1 = t1.id)  (cost=7.12 rows=11)
    -> Table scan on t  (cost=0.26 rows=3)
        -> Materialize  (cost=1.25 rows=3)
            -> Table scan on <temporary>
                -> Temporary table with deduplication  (cost=1.25 rows=3)
                    -> Filter: (t1.col1 > 2)  (cost=1.25 rows=3)
                        -> Table scan on t1  (cost=1.25 rows=10)
    -> Hash
        -> PX Receiver (slice: 0; workers: 1)
            -> PX Sender (slice: 1; workers: 4)
                -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 straight_join (select distinct col1 from t1) as t where t.col1>2 and t1.id=t.col1;
id	col1	col2	col1
10	10	10	10
3	3	3	3
4	4	4	4
5	5	5	5
6	6	6	6
7	7	7	7
8	8	8	8
9	9	9	9
# Aggregate(stream or temptable) below Materialize can NOT be split now.
explain format=tree select * from (select sum(col2) as a from t1 group by col1) as t;
EXPLAIN
-> Table scan on t  (cost=3.62 rows=10)
    -> Materialize
        -> Table scan on <temporary>
            -> Final Aggregate using temporary table
                -> PX Receiver (slice: 0; workers: 1)
                    -> PX Sender (slice: 1; workers: 4)
                        -> Table scan on <temporary>
                            -> Aggregate using temporary table
                                -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from (select sum(col2) as a from t1 group by col1) as t;
a
1
10
2
3
4
5
6
7
8
9
# LIMIT
explain format=tree select * from (select distinct col1 from t1) as t where t.col1>2 order by t.col1 limit 1;
EXPLAIN
-> Limit: 1 row(s)  (cost=1.25 rows=1)
    -> Sort: t.col1, limit input to 1 row(s) per chunk  (cost=1.25 rows=3)
        -> Table scan on t  (cost=2.84 rows=3)
            -> Materialize  (cost=1.25 rows=3)
                -> Table scan on <temporary>
                    -> Temporary table with deduplication  (cost=1.25 rows=3)
                        -> PX Receiver (slice: 0; workers: 1)
                            -> PX Sender (slice: 1; workers: 4)
                                -> Filter: (t1.col1 > 2)  (cost=1.25 rows=3)
                                    -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from (select distinct col1 from t1) as t where t.col1>2 order by t.col1 limit 1;
col1
3
explain format=tree select * from t1 join (select distinct col1 from t1) as t where t.col1>2 and t1.id=t.col1 order by t.col1 limit 1;
EXPLAIN
-> Limit: 1 row(s)  (cost=2.33 rows=1)
    -> Nested loop inner join  (cost=2.33 rows=3)
        -> Sort: t.col1  (cost=1.25 rows=3)
            -> Filter: (t.col1 is not null)  (cost=2.84 rows=3)
                -> Table scan on t  (cost=2.84 rows=3)
                    -> Materialize  (cost=1.25 rows=3)
                        -> Table scan on <temporary>
                            -> Temporary table with deduplication  (cost=1.25 rows=3)
                                -> PX Receiver (slice: 0; workers: 1)
                                    -> PX Sender (slice: 1; workers: 4)
                                        -> Filter: (t1.col1 > 2)  (cost=1.25 rows=3)
                                            -> Parallel table scan on t1  (cost=1.25 rows=10)
        -> Single-row index lookup on t1 using PRIMARY (id=t.col1)  (cost=0.28 rows=1)

select * from t1 join (select distinct col1 from t1) as t where t.col1>2 and t1.id=t.col1 order by t.col1 limit 1;
id	col1	col2	col1
3	3	3	3
explain format=tree select * from t1 straight_join (select distinct col1 from t1) as t where t.col1>2 and t1.id=t.col1 order by t.col1 limit 1;
EXPLAIN
-> Limit: 1 row(s)
    -> Sort: t.col1, limit input to 1 row(s) per chunk
        -> Stream results  (cost=7.12 rows=11)
            -> Inner hash join (t.col1 = t1.id)  (cost=7.12 rows=11)
                -> Table scan on t  (cost=0.26 rows=3)
                    -> Materialize  (cost=1.25 rows=3)
                        -> Table scan on <temporary>
                            -> Temporary table with deduplication  (cost=1.25 rows=3)
                                -> Filter: (t1.col1 > 2)  (cost=1.25 rows=3)
                                    -> Table scan on t1  (cost=1.25 rows=10)
                -> Hash
                    -> PX Receiver (slice: 0; workers: 1)
                        -> PX Sender (slice: 1; workers: 4)
                            -> Parallel table scan on t1  (cost=1.25 rows=10)

select * from t1 straight_join (select distinct col1 from t1) as t where t.col1>2 and t1.id=t.col1 order by t.col1 limit 1;
id	col1	col2	col1
3	3	3	3
drop table t1,t2;
#
# Create TABLE with fields of incompatible types.
#
create table t1 (a mediumtext, b int);
create table t2 (a varchar(10), b int);
insert into t1 values ('a', 1),('b', 2);
insert into t2 values ('a', 1),('b', 2);
analyze table t1,t2;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
explain format=tree select * from t1;
EXPLAIN
-> Table scan on t1  (cost=0.45 rows=2)

select * from t1;
a	b
a	1
b	2
explain format=tree select b from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 2)
        -> Parallel table scan on t1  (cost=0.45 rows=2)

select b from t1;
b
1
2
explain format=tree select a from t2 where length(hex(a)) >1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 2)
        -> Filter: (length(hex(t2.a)) > 1)  (cost=0.45 rows=2)
            -> Parallel table scan on t2  (cost=0.45 rows=2)

select a from t2 where length(hex(a)) >1;
a
a
b
explain format=tree select sum(length(a)) from t1;
EXPLAIN
-> Aggregate: sum(length(t1.a))
    -> Table scan on t1  (cost=0.45 rows=2)

select sum(length(a)) from t1;
sum(length(a))
2
explain format=tree select * from t1 join t2 on t1.b=t2.b;
EXPLAIN
-> Inner hash join (t2.b = t1.b)  (cost=1.12 rows=2)
    -> Table scan on t2  (cost=0.18 rows=2)
    -> Hash
        -> Table scan on t1  (cost=0.45 rows=2)

select * from t1 join t2 on t1.b=t2.b;
a	b	a	b
a	1	a	1
b	2	b	2
explain format=tree select * from t1 union all select * from t2;
EXPLAIN
-> Append
    -> Stream results  (cost=0.45 rows=2)
        -> Table scan on t1  (cost=0.45 rows=2)
    -> Stream results  (cost=0.45 rows=2)
        -> Table scan on t2  (cost=0.45 rows=2)

select * from t1 union all select * from t2;
a	b
a	1
a	1
b	2
b	2
explain format=tree select 1 union all select 2;
EXPLAIN
-> Append
    -> Stream results  (cost=0.00 rows=1)
        -> Rows fetched before execution  (cost=0.00 rows=1)
    -> Stream results  (cost=0.00 rows=1)
        -> Rows fetched before execution  (cost=0.00 rows=1)

select 1 union all select 2;
1
1
2
explain format=tree select 1 union select 2;
EXPLAIN
-> Table scan on <union temporary>  (cost=2.50 rows=0)
    -> Union materialize with deduplication
        -> Rows fetched before execution  (cost=0.00 rows=1)
        -> Rows fetched before execution  (cost=0.00 rows=1)

select 1 union select 2;
1
1
2
explain format=tree select * from t1 union all select * from t1;
EXPLAIN
-> Append
    -> Stream results  (cost=0.45 rows=2)
        -> Table scan on t1  (cost=0.45 rows=2)
    -> Stream results  (cost=0.45 rows=2)
        -> Table scan on t1  (cost=0.45 rows=2)

select * from t1 union all select * from t1;
a	b
a	1
a	1
b	2
b	2
explain format=tree select * from t1 union select * from t1;
EXPLAIN
-> Table scan on <union temporary>  (cost=2.50 rows=0)
    -> Union materialize with deduplication
        -> Table scan on t1  (cost=0.45 rows=2)
        -> Table scan on t1  (cost=0.45 rows=2)

select * from t1 union select * from t1;
a	b
a	1
b	2
drop table t1,t2;
#
# Choose first non-const table
#
CREATE TABLE t1 (a INT PRIMARY KEY, b INT);
CREATE TABLE t2 (a INT, b INT, KEY a_idx(a));
CREATE TABLE t3 (a INT PRIMARY KEY, b INT);
CREATE TABLE t4 (a INT, b INT, KEY a_idx(a));
INSERT INTO t1 VALUES (1, 1);
INSERT INTO t1 VALUES (2, 2);
INSERT INTO t1 VALUES (3, 3);
INSERT INTO t2 VALUES (1, 1);
INSERT INTO t2 VALUES (1, 2);
INSERT INTO t2 VALUES (1, 3);
INSERT INTO t3 VALUES (1, 1);
INSERT INTO t3 VALUES (2, 2);
INSERT INTO t3 VALUES (3, 3);
INSERT INTO t4 VALUES (1, 1);
INSERT INTO t4 VALUES (2, 2);
INSERT INTO t4 VALUES (3, 3);
ANALYZE TABLE t1, t2, t3, t4;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
test.t3	analyze	status	OK
test.t4	analyze	status	OK
# Inner table in left join can't be the parallel table
EXPLAIN FORMAT=TREE SELECT * FROM t1 LEFT JOIN t2 USING(a) WHERE a=1;
EXPLAIN
-> Nested loop left join  (cost=0.80 rows=3)
    -> Rows fetched before execution  (cost=0.00 rows=1)
    -> Index lookup on t2 using a_idx (a=1)  (cost=0.80 rows=3)

SELECT * FROM t1 LEFT JOIN t2 USING(a) WHERE a=1;
a	b	b
1	1	1
1	1	2
1	1	3
EXPLAIN FORMAT=TREE SELECT * FROM t1 LEFT JOIN t3 USING(a) LEFT JOIN t2 USING(a) WHERE a=1;
EXPLAIN
-> Nested loop left join  (cost=0.80 rows=3)
    -> Rows fetched before execution  (cost=0.00 rows=1)
    -> Index lookup on t2 using a_idx (a=1)  (cost=0.80 rows=3)

SELECT * FROM t1 LEFT JOIN t3 USING(a) LEFT JOIN t2 USING(a) WHERE a=1;
a	b	b	b
1	1	1	1
1	1	1	2
1	1	1	3
EXPLAIN FORMAT=TREE SELECT * FROM t1 LEFT JOIN t2 USING(a) LEFT JOIN t3 USING(a) WHERE a=1;
EXPLAIN
-> Nested loop left join  (cost=1.35 rows=3)
    -> Nested loop left join  (cost=0.80 rows=3)
        -> Rows fetched before execution  (cost=0.00 rows=1)
        -> Index lookup on t2 using a_idx (a=1)  (cost=0.80 rows=3)
    -> Constant row from t3  (cost=0.12 rows=1)

SELECT * FROM t1 LEFT JOIN t2 USING(a) LEFT JOIN t3 USING(a) WHERE a=1;
a	b	b	b
1	1	1	1
1	1	2	1
1	1	3	1
EXPLAIN FORMAT=TREE SELECT * FROM t1 LEFT JOIN (SELECT DISTINCT b FROM t2) AS t USING(b) WHERE t1.a=1;
EXPLAIN
-> Nested loop left join  (cost=0.55 rows=3)
    -> Rows fetched before execution  (cost=0.00 rows=1)
    -> Index lookup on t using <auto_key0> (b='1')
        -> Materialize  (cost=0.55 rows=3)
            -> Table scan on <temporary>
                -> Temporary table with deduplication  (cost=0.55 rows=3)
                    -> Table scan on t2  (cost=0.55 rows=3)

SELECT * FROM t1 LEFT JOIN (SELECT DISTINCT b FROM t2) AS t USING(b) WHERE t1.a=1;
b	a
1	1
EXPLAIN FORMAT=TREE SELECT /*+JOIN_ORDER(t1,t4)*/ * FROM t1 LEFT JOIN t4 USING(a) WHERE a=1;
EXPLAIN
-> Nested loop left join  (cost=0.35 rows=1)
    -> Rows fetched before execution  (cost=0.00 rows=1)
    -> Index lookup on t4 using a_idx (a=1)  (cost=0.35 rows=1)

SELECT /*+JOIN_ORDER(t1,t4)*/ * FROM t1 LEFT JOIN t4 USING(a) WHERE a=1;
a	b	b
1	1	1
DROP TABLE t1, t2, t3, t4;
#
# Window functions
#
CREATE TABLE t1 (a INT, b INT, c INT, KEY b_idx(b));
INSERT INTO t1 VALUES (1,1,1),(2,1,1),(3,1,1),(4,2,2),(5,2,2),(6,2,2);
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
EXPLAIN FORMAT=TREE SELECT SUM(AVG(a)) OVER () FROM t1;
EXPLAIN
-> Aggregate: avg(t1.a)
    -> Table scan on t1  (cost=0.85 rows=6)

SELECT SUM(AVG(a)) OVER () FROM t1;
SUM(AVG(a)) OVER ()
3.5000
EXPLAIN FORMAT=TREE SELECT SUM(a), SUM(AVG(a)) OVER () FROM t1 GROUP BY b;
EXPLAIN
-> Window aggregate with buffering: sum(avg(t1.a)) OVER () 
    -> Table scan on <temporary>
        -> Aggregate using temporary table
            -> Table scan on t1  (cost=0.85 rows=6)

SELECT SUM(a), SUM(AVG(a)) OVER () FROM t1 GROUP BY b;
SUM(a)	SUM(AVG(a)) OVER ()
15	7.0000
6	7.0000
EXPLAIN FORMAT=TREE SELECT SUM(a), SUM(AVG(a)) OVER () FROM t1 GROUP BY c;
EXPLAIN
-> Window aggregate with buffering: sum(avg(t1.a)) OVER () 
    -> Table scan on <temporary>
        -> Aggregate using temporary table
            -> Table scan on t1  (cost=0.85 rows=6)

SELECT SUM(a), SUM(AVG(a)) OVER () FROM t1 GROUP BY c;
SUM(a)	SUM(AVG(a)) OVER ()
15	7.0000
6	7.0000
DROP TABLE t1;
#
# MATCH functions
#
CREATE TABLE wp(
FTS_DOC_ID bigint unsigned PRIMARY KEY,
title VARCHAR(255) NOT NULL DEFAULT '',
text MEDIUMTEXT NOT NULL ) ENGINE=InnoDB;
INSERT INTO wp (FTS_DOC_ID, title, text) VALUES
(1, 'MySQL Tutorial','DBMS stands for DataBase ...'),
(2, 'How To Use MySQL Well','After you went through a ...');
CREATE FULLTEXT INDEX idx ON wp(title, text);
explain format=tree SELECT FTS_DOC_ID, MATCH(title, text) AGAINST ('database') FROM wp;
EXPLAIN
-> Index scan on wp using PRIMARY  (cost=0.45 rows=2)

SELECT FTS_DOC_ID, MATCH(title, text) AGAINST ('database') FROM wp;
FTS_DOC_ID	MATCH(title, text) AGAINST ('database')
1	0.0906190574169159
2	0
drop table wp;
#
# Sub-plan of Limit Access Path is parallel unsafe
#
SET @old_optimizer_switch = @@optimizer_switch;
SET optimizer_switch='semijoin=off';
CREATE TABLE t SELECT 4096 c;
EXPLAIN FORMAT=tree
SELECT c FROM t WHERE 1 IN
(SELECT t.c = page_size FROM information_schema.innodb_cmpmem);
EXPLAIN
-> Filter: <in_optimizer>(1,<exists>(select #2))  (cost=0.35 rows=1)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 1)
            -> Parallel table scan on t  (cost=0.35 rows=1)
    -> Select #2 (subquery in condition; dependent)
        -> Limit: 1 row(s)
            -> Filter: (<cache>(1) = (t.c = innodb_cmpmem.page_size))
                -> Table scan on innodb_cmpmem  (cost=0.00 rows=0)
                    -> Fill information schema table innodb_cmpmem

Warnings:
Note	1276	Field or reference 'test.t.c' of SELECT #2 was resolved in SELECT #1
SELECT c FROM t WHERE 1 IN
(SELECT t.c = page_size FROM information_schema.innodb_cmpmem);
c
4096
set optimizer_switch = @old_optimizer_switch;
DROP TABLE t;
# --include set_px_to_saved.inc

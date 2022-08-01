# --include save_and_set_px.inc
#
# Create tables
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
# Set cost threshold to 0.
#
SET SESSION txsql_parallel_cost_threshold=0;
explain format=tree select col1 from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel table scan on t1  (cost=1.25 rows=10)

explain format=tree select t1.col1 from t1 join t2 on t1.col2=t2.col2 order by t1.col1;
EXPLAIN
-> Nested loop inner join  (cost=4.75 rows=10)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Sort: t1.col1  (cost=1.25 rows=10)
                -> Filter: (t1.col2 is not null)  (cost=1.25 rows=10)
                    -> Parallel table scan on t1  (cost=1.25 rows=10)
    -> Index lookup on t2 using idx (col2=t1.col2)  (cost=0.26 rows=1)

# UNION
explain format=tree (select * from t1 where id > 2) union all (select * from t2 where id > 3);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=1.86 rows=8)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.id > 2)  (cost=1.86 rows=8)
                            -> Parallel index range scan on t1 using PRIMARY  (cost=1.86 rows=8)
            -> Stream results  (cost=1.66 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.id > 3)  (cost=1.66 rows=7)
                            -> Parallel index range scan on t2 using PRIMARY  (cost=1.66 rows=7)

# Subquery
set @old_optimizer_switch = @@optimizer_switch;
set optimizer_switch='semijoin=on,materialization=on,loosescan=off,firstmatch=off,duplicateweedout=off,index_condition_pushdown=off,mrr=off';
explain format=tree select * from t1 where col1 in (select col1 from t2 where col1 > 2);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=5.58 rows=33)
            -> Filter: (t1.col1 is not null)  (cost=1.25 rows=10)
                -> Parallel table scan on t1  (cost=1.25 rows=10)
            -> Single-row index lookup on <subquery2> using <auto_distinct_key> (col1=t1.col1)
                -> Materialize with deduplication  (cost=1.25 rows=3)
                    -> Filter: (t2.col1 is not null)  (cost=1.25 rows=3)
                        -> Filter: (t2.col1 > 2)  (cost=1.25 rows=3)
                            -> Table scan on t2  (cost=1.25 rows=10)

set optimizer_switch = @old_optimizer_switch;
# Derived table
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

#
# Set cost threshold to 5.
#
SET SESSION txsql_parallel_cost_threshold=5;
explain format=tree select col1 from t1;
EXPLAIN
-> Table scan on t1  (cost=1.25 rows=10)

explain format=tree select t1.col1 from t1 join t2 on t1.col2=t2.col2 order by t1.col1;
EXPLAIN
-> Nested loop inner join  (cost=4.75 rows=10)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Sort: t1.col1  (cost=1.25 rows=10)
                -> Filter: (t1.col2 is not null)  (cost=1.25 rows=10)
                    -> Parallel table scan on t1  (cost=1.25 rows=10)
    -> Index lookup on t2 using idx (col2=t1.col2)  (cost=0.26 rows=1)

# UNION
explain format=tree (select * from t1 where id > 2) union all (select * from t2 where id > 3);
EXPLAIN
-> Append
    -> Stream results  (cost=1.86 rows=8)
        -> Filter: (t1.id > 2)  (cost=1.86 rows=8)
            -> Index range scan on t1 using PRIMARY  (cost=1.86 rows=8)
    -> Stream results  (cost=1.66 rows=7)
        -> Filter: (t2.id > 3)  (cost=1.66 rows=7)
            -> Index range scan on t2 using PRIMARY  (cost=1.66 rows=7)

# Subquery
set @old_optimizer_switch = @@optimizer_switch;
set optimizer_switch='semijoin=on,materialization=on,loosescan=off,firstmatch=off,duplicateweedout=off,index_condition_pushdown=off,mrr=off';
explain format=tree select * from t1 where col1 in (select col1 from t2 where col1 > 2);
EXPLAIN
-> Nested loop inner join  (cost=5.58 rows=33)
    -> Filter: (t1.col1 is not null)  (cost=1.25 rows=10)
        -> Table scan on t1  (cost=1.25 rows=10)
    -> Single-row index lookup on <subquery2> using <auto_distinct_key> (col1=t1.col1)
        -> Materialize with deduplication  (cost=1.25 rows=3)
            -> Filter: (t2.col1 is not null)  (cost=1.25 rows=3)
                -> Filter: (t2.col1 > 2)  (cost=1.25 rows=3)
                    -> Table scan on t2  (cost=1.25 rows=10)

set optimizer_switch = @old_optimizer_switch;
# Derived table
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

#
# Set cost threshold to 10.
#
SET SESSION txsql_parallel_cost_threshold=10;
explain format=tree select col1 from t1;
EXPLAIN
-> Table scan on t1  (cost=1.25 rows=10)

explain format=tree select t1.col1 from t1 join t2 on t1.col2=t2.col2 order by t1.col1;
EXPLAIN
-> Nested loop inner join  (cost=4.75 rows=10)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Sort: t1.col1  (cost=1.25 rows=10)
                -> Filter: (t1.col2 is not null)  (cost=1.25 rows=10)
                    -> Parallel table scan on t1  (cost=1.25 rows=10)
    -> Index lookup on t2 using idx (col2=t1.col2)  (cost=0.26 rows=1)

# UNION
explain format=tree (select * from t1 where id > 2) union all (select * from t2 where id > 3);
EXPLAIN
-> Append
    -> Stream results  (cost=1.86 rows=8)
        -> Filter: (t1.id > 2)  (cost=1.86 rows=8)
            -> Index range scan on t1 using PRIMARY  (cost=1.86 rows=8)
    -> Stream results  (cost=1.66 rows=7)
        -> Filter: (t2.id > 3)  (cost=1.66 rows=7)
            -> Index range scan on t2 using PRIMARY  (cost=1.66 rows=7)

# Subquery
set @old_optimizer_switch = @@optimizer_switch;
set optimizer_switch='semijoin=on,materialization=on,loosescan=off,firstmatch=off,duplicateweedout=off,index_condition_pushdown=off,mrr=off';
explain format=tree select * from t1 where col1 in (select col1 from t2 where col1 > 2);
EXPLAIN
-> Nested loop inner join  (cost=5.58 rows=33)
    -> Filter: (t1.col1 is not null)  (cost=1.25 rows=10)
        -> Table scan on t1  (cost=1.25 rows=10)
    -> Single-row index lookup on <subquery2> using <auto_distinct_key> (col1=t1.col1)
        -> Materialize with deduplication  (cost=1.25 rows=3)
            -> Filter: (t2.col1 is not null)  (cost=1.25 rows=3)
                -> Filter: (t2.col1 > 2)  (cost=1.25 rows=3)
                    -> Table scan on t2  (cost=1.25 rows=10)

set optimizer_switch = @old_optimizer_switch;
# Derived table
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
        -> Table scan on t1  (cost=1.25 rows=10)

#
# Set cost threshold to 15.
#
SET SESSION txsql_parallel_cost_threshold=15;
explain format=tree select col1 from t1;
EXPLAIN
-> Table scan on t1  (cost=1.25 rows=10)

explain format=tree select t1.col1 from t1 join t2 on t1.col2=t2.col2 order by t1.col1;
EXPLAIN
-> Nested loop inner join  (cost=4.75 rows=10)
    -> Sort: t1.col1  (cost=1.25 rows=10)
        -> Filter: (t1.col2 is not null)  (cost=1.25 rows=10)
            -> Table scan on t1  (cost=1.25 rows=10)
    -> Index lookup on t2 using idx (col2=t1.col2)  (cost=0.26 rows=1)

# UNION
explain format=tree (select * from t1 where id > 2) union all (select * from t2 where id > 3);
EXPLAIN
-> Append
    -> Stream results  (cost=1.86 rows=8)
        -> Filter: (t1.id > 2)  (cost=1.86 rows=8)
            -> Index range scan on t1 using PRIMARY  (cost=1.86 rows=8)
    -> Stream results  (cost=1.66 rows=7)
        -> Filter: (t2.id > 3)  (cost=1.66 rows=7)
            -> Index range scan on t2 using PRIMARY  (cost=1.66 rows=7)

# Subquery
set @old_optimizer_switch = @@optimizer_switch;
set optimizer_switch='semijoin=on,materialization=on,loosescan=off,firstmatch=off,duplicateweedout=off,index_condition_pushdown=off,mrr=off';
explain format=tree select * from t1 where col1 in (select col1 from t2 where col1 > 2);
EXPLAIN
-> Nested loop inner join  (cost=5.58 rows=33)
    -> Filter: (t1.col1 is not null)  (cost=1.25 rows=10)
        -> Table scan on t1  (cost=1.25 rows=10)
    -> Single-row index lookup on <subquery2> using <auto_distinct_key> (col1=t1.col1)
        -> Materialize with deduplication  (cost=1.25 rows=3)
            -> Filter: (t2.col1 is not null)  (cost=1.25 rows=3)
                -> Filter: (t2.col1 > 2)  (cost=1.25 rows=3)
                    -> Table scan on t2  (cost=1.25 rows=10)

set optimizer_switch = @old_optimizer_switch;
# Derived table
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
        -> Table scan on t1  (cost=1.25 rows=10)

drop table t1,t2;
# --include set_px_to_saved.inc

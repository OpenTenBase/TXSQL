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
create table t5(a int, b int, c int, d int, primary key(a));
insert into t5 values(1,1,1,1);
insert into t5 values(2,2,2,2);
insert into t5 values(3,3,3,3);
insert into t5 values(4,4,4,4);
create table t6(a int, b int, c int, d int, primary key(a), foreign key (d) references t3(a));
insert into t6 values(1,1,1,1);
insert into t6 values(2,2,2,2);
insert into t6 values(3,3,3,3);
insert into t6 values(4,4,4,4);
analyze table t1,t2,t3,t4,t5,t6;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
test.t3	analyze	status	OK
test.t4	analyze	status	OK
test.t5	analyze	status	OK
test.t6	analyze	status	OK
# --include save_and_set_px.inc
# 1.1. test the PARALLEL hint
# 1.1.1 test the PARALLEL(n)
explain select /*+PARALLEL(0)*/ * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select /*+ PARALLEL(0) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
explain format=tree select /*+PARALLEL(0)*/ * from t1 union select * from t2;
EXPLAIN
-> Table scan on <union temporary>  (cost=2.50 rows=0)
    -> Union materialize with deduplication
        -> Table scan on t1  (cost=0.95 rows=7)
        -> Table scan on t2  (cost=0.75 rows=5)

explain format=tree select /*+PARALLEL(1)*/ * from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Parallel table scan on t1  (cost=0.95 rows=7)

explain format=tree select /*+PARALLEL(1)*/a,avg(c) from t1 group by a union select a,b from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 1)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 1)
                        -> Parallel table scan on t2  (cost=0.75 rows=5)

explain format=tree select /*+PARALLEL(10)*/ * from t1;
EXPLAIN
-> Table scan on t1  (cost=0.95 rows=7)

explain format=tree select /*+PARALLEL(10)*/ a,sum(b) from t1 group by a union select a,b from t2;
EXPLAIN
-> Table scan on <union temporary>  (cost=2.50 rows=0)
    -> Union materialize with deduplication
        -> Table scan on <temporary>
            -> Aggregate using temporary table
                -> Table scan on t1  (cost=0.95 rows=7)
        -> Table scan on t2  (cost=0.75 rows=5)

# 1.1.2 test the PARALLEL(table)
explain format=tree select /*+NO_BNL(t1,t2) PARALLEL(t2)*/ t1.a, t2.a from t1,t2 where t1.a = t2.a;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=5.50 rows=5)
            -> Parallel table scan on t2  (cost=0.75 rows=5)
            -> Filter: (t1.a = t2.a)  (cost=0.27 rows=1)
                -> Table scan on t1  (cost=0.27 rows=7)

explain format=tree select /*+NO_BNL(t1,t2) PARALLEL(t2)*/ t1.a, t2.a from t1,t2 where t1.b = t2.b union select t3.a,t3.b from t3;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=5.50 rows=5)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
                            -> Filter: (t1.b = t2.b)  (cost=0.27 rows=1)
                                -> Table scan on t1  (cost=0.27 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel index scan on t3 using idx  (cost=1.25 rows=10)

explain format=tree select /*+PARALLEL(t1) PARALLEL(t2)*/ t1.* from t1,t2;
EXPLAIN
-> Inner hash join (no condition)  (cost=4.50 rows=35)
    -> Table scan on t1  (cost=0.19 rows=7)
    -> Hash
        -> PX Receiver (slice: 0; workers: 1)
            -> PX Sender (slice: 1; workers: 4)
                -> Parallel table scan on t2  (cost=0.75 rows=5)

Warnings:
Warning	3126	Hint PARALLEL(`t2`@``,) is ignored as conflicting/duplicated
Warning	8504	parallel hint has no effect, not the chosen table.
explain format=tree select /*+PARALLEL(t1)*/ t1.* from t1,t2 union select /*+PARALLEL(t3)*/ * from t3;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> Inner hash join (no condition)  (cost=4.50 rows=35)
                    -> Table scan on t1  (cost=0.19 rows=7)
                    -> Hash
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Parallel table scan on t2  (cost=0.75 rows=5)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t3  (cost=1.25 rows=10)

Warnings:
Warning	8504	parallel hint has no effect, not the chosen table.
explain format=tree select /*+JOIN_ORDER(t5,t3) PARALLEL(t3)*/ t5.a, t3.a from t5, t3 where t5.a = t3.b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=2.05 rows=4)
            -> Parallel index scan on t5 using PRIMARY  (cost=0.65 rows=4)
            -> Index lookup on t3 using idx (b=t5.a)  (cost=0.28 rows=1)

Warnings:
Warning	8504	parallel hint has no effect, not the chosen table.
explain format=tree select /*+JOIN_ORDER(t5,t3) PARALLEL(t3)*/ t5.a, t3.a from t5, t3 where t5.a = t3.b union select /*+PARALLEL(t1)*/ t1.a,t1.b from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=2.05 rows=4)
                            -> Parallel index scan on t5 using PRIMARY  (cost=0.65 rows=4)
                            -> Index lookup on t3 using idx (b=t5.a)  (cost=0.28 rows=1)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)

Warnings:
Warning	8504	parallel hint has no effect, not the chosen table.
# 1.1.3 test the PARALLEL(table n)
explain format=tree select /*+NO_BNL(t1,t2) PARALLEL(t2 2)*/ t1.a+t2.b from t1,t2 where t1.a = t2.b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 2)
        -> Nested loop inner join  (cost=5.50 rows=5)
            -> Parallel table scan on t2  (cost=0.75 rows=5)
            -> Filter: (t1.a = t2.b)  (cost=0.27 rows=1)
                -> Table scan on t1  (cost=0.27 rows=7)

explain format=tree select /*+NO_BNL(t1,t2) PARALLEL(t2 2)*/ t1.a+t2.b from t1,t2 where t1.b = t2.a union select /*+PARALLEL(t4 2)*/ t4.a+t4.b from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 2)
                        -> Nested loop inner join  (cost=5.50 rows=5)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
                            -> Filter: (t1.b = t2.a)  (cost=0.27 rows=1)
                                -> Table scan on t1  (cost=0.27 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)

Warnings:
Warning	3126	Hint PARALLEL(`t4`@``,2) is ignored as conflicting/duplicated
explain format=tree select /*+NO_BNL(t1,t2) PARALLEL(t2 2)*/ t1.a+t2.b from t1,t2 where t1.b = t2.a union select /*+PARALLEL(t4 4)*/ t4.a+t4.b from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 2)
                        -> Nested loop inner join  (cost=5.50 rows=5)
                            -> Parallel table scan on t2  (cost=0.75 rows=5)
                            -> Filter: (t1.b = t2.a)  (cost=0.27 rows=1)
                                -> Table scan on t1  (cost=0.27 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 1)
                        -> Parallel table scan on t4  (cost=0.35 rows=1)

Warnings:
Warning	3126	Hint PARALLEL(`t4`@``,4) is ignored as conflicting/duplicated
# 1.1.4 error(qb test)
explain select /*+ PARALLEL(5) PARALLEL(12)*/ * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (5 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>)
Warnings:
Warning	3126	Hint PARALLEL(12) is ignored as conflicting/duplicated
Note	1003	/* select#1 */ select /*+ PARALLEL(5) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
explain select /*+ PARALLEL(5) PARALLEL(t2 12)*/ * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (5 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>)
Warnings:
Warning	3126	Hint PARALLEL(`t2`@``,12) is ignored as conflicting/duplicated
Note	1003	/* select#1 */ select /*+ PARALLEL(5) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
explain select * from (select /*+ PARALLEL(t2 5) PARALLEL(12)*/t1.a as aa, t1.b as bb from t1) as v where v.bb = 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	14.29	Parallel scan (4 workers); Using where
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Receive from (<sender1>)
Warnings:
Warning	3126	Hint PARALLEL(12) is ignored as conflicting/duplicated
Warning	3128	Unresolved name `t2`@`select#2` for PARALLEL hint
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `aa`,`test`.`t1`.`b` AS `bb` from `test`.`t1` where (`test`.`t1`.`b` = 1)
explain select * from (select /*+ PARALLEL(t1 5) PARALLEL(t2 12)*/t1.a as aa, t1.b as bb from t1) as v where v.bb = 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	14.29	Parallel scan (5 workers); Using where
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Receive from (<sender1>)
Warnings:
Warning	3126	Hint PARALLEL(`t2`@``,12) is ignored as conflicting/duplicated
Note	1003	/* select#1 */ select /*+ PARALLEL(5) PARALLEL(`t1`@`select#2`) */ `test`.`t1`.`a` AS `aa`,`test`.`t1`.`b` AS `bb` from `test`.`t1` where (`test`.`t1`.`b` = 1)
explain select /*+ PARALLEL(5) PARALLEL(t1@qb1 12)*/ * from (select /*+ qb_name(qb1)*/t1.a as aa, t1.b as bb from t1) as v where v.bb = 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	14.29	Parallel scan (5 workers); Using where
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Receive from (<sender1>)
Warnings:
Warning	3126	Hint PARALLEL(`t1`@`qb1`,12) is ignored as conflicting/duplicated
Note	1003	/* select#1 */ select /*+ PARALLEL(5) */ `test`.`t1`.`a` AS `aa`,`test`.`t1`.`b` AS `bb` from `test`.`t1` where (`test`.`t1`.`b` = 1)
explain select /*+ PARALLEL(t1@qb1 5) PARALLEL(12)*/ * from (select /*+ qb_name(qb1)*/t1.a as aa, t1.b as bb from t1) as v where v.bb = 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	14.29	Parallel scan (5 workers); Using where
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Receive from (<sender1>)
Warnings:
Warning	3126	Hint PARALLEL(12) is ignored as conflicting/duplicated
Note	1003	/* select#1 */ select /*+ PARALLEL(5) PARALLEL(`t1`@`qb1`) */ `test`.`t1`.`a` AS `aa`,`test`.`t1`.`b` AS `bb` from `test`.`t1` where (`test`.`t1`.`b` = 1)
explain select /*+qb_name(qb1) parallel(t1@qb1 4)*/ * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select /*+ QB_NAME(`qb1`) PARALLEL(4) PARALLEL(`t1`@`qb1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
explain format=tree select /*+qb_name(qb1) parallel(t1@qb1 4)*/ * from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel table scan on t1  (cost=0.95 rows=7)

explain select /*+qb_name(qb1) parallel(t1@qb2 4)*/ * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>)
Warnings:
Warning	3127	Query block name `qb2` is not found for PARALLEL hint
Note	1003	/* select#1 */ select /*+ QB_NAME(`qb1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
explain select /*+qb_name(qb1) parallel(t2@qb1 4)*/ * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>)
Warnings:
Warning	3128	Unresolved name `t2`@`qb1` for PARALLEL hint
Note	1003	/* select#1 */ select /*+ QB_NAME(`qb1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
explain select /*+QB_NAME(qb1) JOIN_ORDER(t5@qb1,t3@qb1) PARALLEL(t3@qb1 4)*/ t5.a, t3.a from t5, t3 where t5.a = t3.b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t5	NULL	index	PRIMARY	PRIMARY	4	NULL	4	100.00	Parallel scan (4 workers); Using index
1	SIMPLE	t3	NULL	ref	idx	idx	5	test.t5.a	1	100.00	Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>)
Warnings:
Warning	8504	parallel hint has no effect, not the chosen table.
Note	1003	/* select#1 */ select /*+ QB_NAME(`qb1`) JOIN_ORDER(@`qb1` `t5`@`qb1`,`t3`@`qb1`) PARALLEL(`t3`@`qb1`) */ `test`.`t5`.`a` AS `a`,`test`.`t3`.`a` AS `a` from `test`.`t5` join `test`.`t3` where (`test`.`t3`.`b` = `test`.`t5`.`a`)
explain format=tree select /*+QB_NAME(qb1) JOIN_ORDER(t5@qb1,t3@qb1) PARALLEL(t3@qb1 4)*/ t5.a, t3.a from t5, t3 where t5.a = t3.b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=2.05 rows=4)
            -> Parallel index scan on t5 using PRIMARY  (cost=0.65 rows=4)
            -> Index lookup on t3 using idx (b=t5.a)  (cost=0.28 rows=1)

Warnings:
Warning	8504	parallel hint has no effect, not the chosen table.
explain select /*+parallel(t1@qb1)*/ * from (select /*+qb_name(qb1)*/ * from t1) as v where v.a = 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	14.29	Parallel scan (4 workers); Using where
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select /*+ PARALLEL(`t1`@`qb1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` where (`test`.`t1`.`a` = 1)
explain format=tree select /*+parallel(t1@qb1)*/ * from (select /*+qb_name(qb1)*/ * from t1) as v where v.a = 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Filter: (t1.a = 1)  (cost=0.95 rows=1)
            -> Parallel table scan on t1  (cost=0.95 rows=7)

explain select /*+PARALLEL(t3@qb1 4)*/ * from (select /*+QB_NAME(qb1) JOIN_ORDER(t6,t3) */ t6.a as aa, t3.a, t3.b as bb from t6, t3 where t6.a = t3.b) as v where v.aa+v.bb = 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t6	NULL	index	PRIMARY	d	5	NULL	4	100.00	Parallel scan (4 workers); Using index
1	SIMPLE	t3	NULL	ref	idx	idx	5	test.t6.a	1	100.00	Using where; Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select /*+ JOIN_ORDER(@`qb1` `t6`,`t3`) PARALLEL(`t3`@`qb1`) */ `test`.`t6`.`a` AS `aa`,`test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `bb` from `test`.`t6` join `test`.`t3` where ((`test`.`t3`.`b` = `test`.`t6`.`a`) and ((`test`.`t6`.`a` + `test`.`t3`.`b`) = 1))
explain format=tree select /*+PARALLEL(t3@qb1 4)*/ * from (select /*+QB_NAME(qb1) JOIN_ORDER(t6,t3) */ t6.a as aa, t3.a, t3.b as bb from t6, t3 where t6.a = t3.b) as v where v.aa+v.bb = 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=2.05 rows=4)
            -> Parallel index scan on t6 using d  (cost=0.65 rows=4)
            -> Filter: ((t6.a + t3.b) = 1)  (cost=0.28 rows=1)
                -> Index lookup on t3 using idx (b=t6.a)  (cost=0.28 rows=1)

# 1.2. test the NO_PARALLEL hint
# 1.2.1 NO_PARALLEL(t)
explain select /*+NO_PARALLEL(t1)*/ * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select /*+ NO_PARALLEL(`t1`@`select#1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
explain format=tree select /*+NO_PARALLEL(t1)*/ * from t1;
EXPLAIN
-> Table scan on t1  (cost=0.95 rows=7)

explain format=tree select /*+NO_PARALLEL(t1) PARALLEL(t1)*/ * from t1;
EXPLAIN
-> Table scan on t1  (cost=0.95 rows=7)

Warnings:
Warning	3126	Hint PARALLEL(`t1`@``,) is ignored as conflicting/duplicated
explain format=tree select /*+NO_PARALLEL(t2)*/ * from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel table scan on t1  (cost=0.95 rows=7)

Warnings:
Warning	3128	Unresolved name `t2`@`select#1` for NO_PARALLEL hint
explain select /*+NO_PARALLEL(t1)*/ * from t1 union select /*+parallel(4)*/ * from t2;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	NULL
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Parallel scan (4 workers)
2	UNION	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Send to (<receiver2>)
2	UNION	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Receive from (<sender2>)
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select /*+ PARALLEL(4) NO_PARALLEL(`t1`@`select#1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2`
explain format=tree select /*+NO_PARALLEL(t1)*/ * from t1 union select /*+parallel(4)*/ * from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> Table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t2  (cost=0.75 rows=5)

# 1.2.2 multiple NO_PARALLEL(t) in query block
explain format=tree select /*+NO_BNL(t1,t2) NO_PARALLEL(t2) NO_PARALLEL(t1)*/ t1.a, t2.a from t1,t2 where t1.a = t2.b;
EXPLAIN
-> Nested loop inner join  (cost=5.50 rows=5)
    -> Table scan on t2  (cost=0.75 rows=5)
    -> Filter: (t1.a = t2.b)  (cost=0.27 rows=1)
        -> Table scan on t1  (cost=0.27 rows=7)

explain format=tree select /*+JOIN_ORDER(t6,t3) NO_PARALLEL(t3) NO_PARALLEL(t6)*/ t6.a, t3.a from t6, t3 where t6.a = t3.b;
EXPLAIN
-> Nested loop inner join  (cost=2.05 rows=4)
    -> Index scan on t6 using d  (cost=0.65 rows=4)
    -> Index lookup on t3 using idx (b=t6.a)  (cost=0.28 rows=1)

# 1.2.3 others
explain select /*+NO_PARALLEL(t1, 4)*/ * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>)
Warnings:
Warning	1064	Optimizer hint syntax error near ', 4)*/ * from t1' at line 1
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
explain select /*+NO_PARALLEL(t2)*/ * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>)
Warnings:
Warning	3128	Unresolved name `t2`@`select#1` for NO_PARALLEL hint
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
# 1.3 set parallel degree to 0
SET txsql_parallel_degree=0;
explain format=tree select * from t1;
EXPLAIN
-> Table scan on t1  (cost=0.95 rows=7)

explain format=tree select /*+ PARALLEL(4) */ * from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel table scan on t1  (cost=0.95 rows=7)

explain format=tree select /*+ PARALLEL(t1 4) */ * from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel table scan on t1  (cost=0.95 rows=7)

explain format=tree select /*+ PARALLEL(t5 4) */ * from t5 straight_join t6 using(a);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=2.05 rows=4)
            -> Parallel table scan on t5  (cost=0.65 rows=4)
            -> Single-row index lookup on t6 using PRIMARY (a=t5.a)  (cost=0.28 rows=1)

explain format=tree select /*+ PARALLEL(t6 4) */ * from t5 straight_join t6 using(a);
EXPLAIN
-> Nested loop inner join  (cost=2.05 rows=4)
    -> Table scan on t5  (cost=0.65 rows=4)
    -> Single-row index lookup on t6 using PRIMARY (a=t5.a)  (cost=0.28 rows=1)

Warnings:
Warning	8504	parallel hint has no effect, not the chosen table.
# --include set_px_to_saved.inc
drop table t1;
drop table t2;
drop table t6;
drop table t3;
drop table t4;
drop table t5;

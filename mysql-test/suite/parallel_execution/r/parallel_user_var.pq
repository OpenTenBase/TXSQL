# --include save_and_set_px.inc
drop table if exists t1, t9 ;
create table t1 (
a int, b varchar(30),
primary key(a)
);
insert into t1 values (1,'one');
insert into t1 values (2,'two');
insert into t1 values (3,'three');
insert into t1 values (4,'four');
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
set @arg00=1 ;
explain format=tree select @arg00, b from t1 where a > 1 ;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 3)
        -> Filter: (t1.a > 1)  (cost=0.86 rows=3)
            -> Parallel index range scan on t1 using PRIMARY over (1 < a)  (cost=0.86 rows=3)

select @arg00, b from t1 where a > 1 ;
@arg00	b
1	four
1	three
1	two
set @arg00=2 ;
explain format=tree select a, b FROM t1 outer_table where
a = (select a from t1 where a = @arg00 and b = outer_table.b) and b='two' ;
EXPLAIN
-> Filter: ((outer_table.b = 'two') and (outer_table.a = (select #2)))  (cost=0.65 rows=1)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel table scan on outer_table  (cost=0.65 rows=4)
    -> Select #2 (subquery in condition; dependent)
        -> Filter: (('two' = outer_table.b))  (cost=0.00..0.00 rows=1)
            -> Rows fetched before execution  (cost=0.00..0.00 rows=1)

Warnings:
Note	1276	Field or reference 'test.outer_table.b' of SELECT #2 was resolved in SELECT #1
select a, b FROM t1 outer_table where
a = (select a from t1 where a = @arg00 and b = outer_table.b) and b='two' ;
a	b
2	two
# --include set_px_to_saved.inc
DROP table t1;

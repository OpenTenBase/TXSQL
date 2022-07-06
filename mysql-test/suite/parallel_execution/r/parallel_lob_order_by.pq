create table pt2(j json);
prepare ps2 from 'insert into pt2 values (?)';
set @b='123';
set @e='"123"';
set @g=null;
execute ps2 using @b;
execute ps2 using @e;
execute ps2 using @g;
set session txsql_parallel_lob_enabled=1;
show variables like '%txsql_parallel_lob_enabled%';
Variable_name	Value
txsql_parallel_lob_enabled	ON
explain format=tree select * from pt2 order by j;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 3)
        -> Sort: pt2.j  (cost=0.55 rows=3)
            -> Parallel table scan on pt2  (cost=0.55 rows=3)

select * from pt2 order by j;
j
NULL
123
"123"
set session txsql_parallel_lob_enabled=0;
show variables like '%txsql_parallel_lob_enabled%';
Variable_name	Value
txsql_parallel_lob_enabled	OFF
drop table pt2;

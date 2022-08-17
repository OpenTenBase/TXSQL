# --include save_and_set_px.inc
show variables like 'txsql_optimizer_context_max_mem_size';
Variable_name	Value
txsql_optimizer_context_max_mem_size	8388608
set global txsql_max_parallel_worker_threads = 8;

# 1) test optimization context memory limit of handler::stats

create table t1(a int, b int, c int, d int);
insert into t1 values(1,2,3,4);
insert into t1 values(1,1,1,1);
insert into t1 values(2,3,4,5);
insert into t1 values(2,2,2,2);
insert into t1 values(3,4,5,6);
insert into t1 values(4,5,6,7);
insert into t1 values(5,6,7,8);
flush status;
SELECT * FROM t1 order by a,b;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	1
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
show status like '%txsql_max_optimizer_context%';
Variable_name	Value
Txsql_max_optimizer_context_memory_exceeded	0
SET txsql_optimizer_context_max_mem_size = 1;
show variables like 'txsql_optimizer_context_max_mem_size';
Variable_name	Value
txsql_optimizer_context_max_mem_size	1
SELECT * FROM t1 order by a,b;
ERROR HY000: Optimization context cache isn't enabled due to memory limitation.
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	1
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
show status like '%txsql_max_optimizer_context%';
Variable_name	Value
Txsql_max_optimizer_context_memory_exceeded	1
DROP TABLE t1;
SET txsql_optimizer_context_max_mem_size = default;

# 2) test optimization context memory limit of index dive

CREATE TABLE t2 (
auto int not null auto_increment,
fld1 int(6) unsigned zerofill DEFAULT '000000' NOT NULL,
companynr tinyint(2) unsigned zerofill DEFAULT '00' NOT NULL,
fld3 char(30) DEFAULT '' NOT NULL,
fld4 char(35) DEFAULT '' NOT NULL,
fld5 char(35) DEFAULT '' NOT NULL,
fld6 char(4) DEFAULT '' NOT NULL,
UNIQUE fld1 (fld1),
KEY fld3 (fld3),
PRIMARY KEY (auto)
);
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
create table t3 (
period    int not null,
name      char(32) not null,
companynr int not null,
price     double(11,0),
price2     double(11,0),
key (period),
key (name)
);
Warnings:
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
Warnings:
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
flush status;
show variables like 'txsql_optimizer_context_max_mem_size';
Variable_name	Value
txsql_optimizer_context_max_mem_size	8388608
select t2.fld3,companynr from t2 where companynr = 57+1 order by fld3;
fld3	companynr
select name,count(*) from t3 where name >= "extramarital" and price <= 39654943 group by name;
name	count(*)
Iranizes	4181
extramarital	4181
gazer	4181
gems	4181
spates	4181
tucked	4181
violinist	4181
select t2.fld3,count(*) from t2,t3 where t2.fld1=158402 and t3.name=t2.fld3 group by t3.name;
fld3	count(*)
spates	4181
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	3
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
show status like '%txsql_max_optimizer_context%';
Variable_name	Value
Txsql_max_optimizer_context_memory_exceeded	0
SET txsql_optimizer_context_max_mem_size = 1;
show variables like 'txsql_optimizer_context_max_mem_size';
Variable_name	Value
txsql_optimizer_context_max_mem_size	1
select t2.fld3,companynr from t2 where companynr = 57+1 order by fld3;
ERROR HY000: Optimization context cache isn't enabled due to memory limitation.
select name,count(*) from t3 where name >= "extramarital" and price <= 39654943 group by name;
ERROR HY000: Optimization context cache isn't enabled due to memory limitation.
select t2.fld3,count(*) from t2,t3 where t2.fld1=158402 and t3.name=t2.fld3 group by t3.name;
ERROR HY000: Optimization context cache isn't enabled due to memory limitation.
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	3
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
show status like '%txsql_max_optimizer_context%';
Variable_name	Value
Txsql_max_optimizer_context_memory_exceeded	3
SET txsql_optimizer_context_max_mem_size = default;
drop table t2,t3;

# 3) test Txsql_parallel_stmt_thread_refused

SET global txsql_max_parallel_worker_threads = 1;
show variables like 'txsql_max_parallel_worker_threads';
Variable_name	Value
txsql_max_parallel_worker_threads	1
CREATE TABLE t1 (
f1 varchar(16) PRIMARY KEY,
f2 varchar(16)
);
INSERT INTO t1 VALUES ('bla','blah');
INSERT INTO t1 VALUES ('blc','blad');
INSERT INTO t1 VALUES ('bld','blad');
flush status;
SELECT /*+PARALLEL(0)*/ * FROM t1;
f1	f2
bla	blah
blc	blad
bld	blad
SELECT * FROM t1;
f1	f2
bla	blah
blc	blad
bld	blad
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	0
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0

# 4) test Txsql_parallel_stmt_hint_executed

SET global txsql_max_parallel_worker_threads = 0;
show variables like 'txsql_max_parallel_worker_threads';
Variable_name	Value
txsql_max_parallel_worker_threads	0
# test counting the PARALLEL(n)
SELECT /*+PARALLEL(1)*/ * FROM t1;
f1	f2
bla	blah
blc	blad
bld	blad
# test counting the PARALLEL(table)
select /*+PARALLEL(t1)*/ * FROM t1;
f1	f2
bla	blah
blc	blad
bld	blad
Warnings:
Warning	8504	parallel hint has no effect, not the chosen table.
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	0
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
# test counting the hint of qb
select /*+ PARALLEL(t1@qb1 5)*/ * from (select /*+ qb_name(qb1)*/ t1.f1 as aa, t1.f2 as bb from t1) as v where v.aa > 'a';
aa	bb
bla	blah
blc	blad
bld	blad
select /*+ PARALLEL(t1@qb1 5) PARALLEL(12)*/ * from (select /*+ qb_name(qb1)*/ t1.f1 as aa, t1.f2 as bb from t1) as v where v.aa > 'a';
aa	bb
bla	blah
blc	blad
bld	blad
Warnings:
Warning	3126	Hint PARALLEL(12) is ignored as conflicting/duplicated
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	0
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
SET global txsql_max_parallel_worker_threads = 2;
show variables like 'txsql_max_parallel_worker_threads';
Variable_name	Value
txsql_max_parallel_worker_threads	2
SELECT /*+PARALLEL(1)*/ * FROM t1;
f1	f2
bla	blah
blc	blad
bld	blad
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	1
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	1
Txsql_parallel_stmt_thread_refused	0
drop table t1;

# 5) test optimization context memory limit for explain

SET global txsql_max_parallel_worker_threads = 8;
SET txsql_optimizer_context_max_mem_size = default;
create table t1(a int, b int, c int, d int);
insert into t1 values(1,2,3,4);
insert into t1 values(1,1,1,1);
insert into t1 values(2,3,4,5);
insert into t1 values(2,2,2,2);
insert into t1 values(3,4,5,6);
insert into t1 values(4,5,6,7);
insert into t1 values(5,6,7,8);
flush status;
SET txsql_optimizer_context_max_mem_size = default;
explain select * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
SET txsql_optimizer_context_max_mem_size = 1;
show variables like 'txsql_optimizer_context_max_mem_size';
Variable_name	Value
txsql_optimizer_context_max_mem_size	1
explain select * from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	NULL
Warnings:
Warning	3170	Memory capacity of 1 bytes for 'ER_PX_CAPACITY_EXCEEDED_IN_OPTIMIZER_CONTEXT' exceeded. Optimization context cache isn't enabled due to memory limitation.
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	0
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
show status like '%txsql_max_optimizer_context%';
Variable_name	Value
Txsql_max_optimizer_context_memory_exceeded	0

# 6) test max parallel worker limit for explain

flush status;
SET txsql_optimizer_context_max_mem_size = default;
show variables like 'txsql_max_parallel_worker_threads';
Variable_name	Value
txsql_max_parallel_worker_threads	8
explain select * from t1 order by a,b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers); Using filesort
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>); Using merge sort
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` order by `test`.`t1`.`a`,`test`.`t1`.`b`
explain select /*+PARALLEL(4)*/ * from t1 order by a,b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers); Using filesort
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>); Using merge sort
Warnings:
Note	1003	/* select#1 */ select /*+ PARALLEL(4) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` order by `test`.`t1`.`a`,`test`.`t1`.`b`
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	0
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
show status like '%txsql_max_optimizer_context%';
Variable_name	Value
Txsql_max_optimizer_context_memory_exceeded	0
select /*+PARALLEL(4)*/ * from t1 order by a,b;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
select * from t1 order by a,b;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	2
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	1
Txsql_parallel_stmt_thread_refused	0
show status like '%txsql_max_optimizer_context%';
Variable_name	Value
Txsql_max_optimizer_context_memory_exceeded	0
SET global txsql_max_parallel_worker_threads = 1;
flush status;
show variables like 'txsql_max_parallel_worker_threads';
Variable_name	Value
txsql_max_parallel_worker_threads	1
explain select * from t1 order by a,b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Using filesort
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` order by `test`.`t1`.`a`,`test`.`t1`.`b`
explain select /*+PARALLEL(4)*/ * from t1 order by a,b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Using filesort
Warnings:
Note	1003	/* select#1 */ select /*+ PARALLEL(4) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` order by `test`.`t1`.`a`,`test`.`t1`.`b`
select /*+PARALLEL(4)*/ * from t1 order by a,b;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
select * from t1 order by a,b;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	0
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
show status like '%txsql_max_optimizer_context%';
Variable_name	Value
Txsql_max_optimizer_context_memory_exceeded	0
SET global txsql_max_parallel_worker_threads = 8;

# 7) test max DOP limit for explain

flush status;
SET txsql_parallel_degree=0;
show variables like 'txsql_max_parallel_worker_threads';
Variable_name	Value
txsql_max_parallel_worker_threads	8
explain select * from t1 order by a,b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Using filesort
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` order by `test`.`t1`.`a`,`test`.`t1`.`b`
explain select /*+PARALLEL(4)*/ * from t1 order by a,b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers); Using filesort
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>); Using merge sort
Warnings:
Note	1003	/* select#1 */ select /*+ PARALLEL(4) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` order by `test`.`t1`.`a`,`test`.`t1`.`b`
select /*+PARALLEL(4)*/ * from t1 order by a,b;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
select * from t1 order by a,b;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	1
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	1
Txsql_parallel_stmt_thread_refused	0
SET txsql_parallel_degree=4;
explain select /*+PARALLEL(4)*/ * from t1 order by a,b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers); Using filesort
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>); Using merge sort
Warnings:
Note	1003	/* select#1 */ select /*+ PARALLEL(4) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` order by `test`.`t1`.`a`,`test`.`t1`.`b`
select /*+PARALLEL(4)*/ * from t1 order by a,b;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
select * from t1 order by a,b;
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
3	4	5	6
4	5	6	7
5	6	7	8
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	3
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	2
Txsql_parallel_stmt_thread_refused	0
show status like '%txsql_max_optimizer_context%';
Variable_name	Value
Txsql_max_optimizer_context_memory_exceeded	0
DROP TABLE t1;
# --include set_px_to_saved.inc

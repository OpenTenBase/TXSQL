# --include save_and_set_px.inc
show variables like 'txsql_optimizer_context_max_mem_size';
Variable_name	Value
txsql_optimizer_context_max_mem_size	8388608

# 1) test optimization context memory limit of handler::stats

CREATE TABLE t1 (
f1 varchar(16) PRIMARY KEY,
f2 varchar(16)
);
CREATE TABLE t2 (
f1 varchar(16) PRIMARY KEY,
f3 varchar(16)
);
INSERT INTO t1 VALUES ('bla','blah');
INSERT INTO t1 VALUES ('blc','blad');
INSERT INTO t1 VALUES ('bld','blad');
INSERT INTO t2 VALUES ('bla','sheep');
INSERT INTO t2 VALUES ('blc','sheep');
INSERT INTO t2 VALUES ('bld','sheep');
SELECT * FROM t1 LEFT JOIN t2 USING(f1) WHERE f1='bla';
f1	f2	f3
bla	blah	sheep
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	4
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	0
Txsql_parallel_stmt_thread_refused	0
SET txsql_optimizer_context_max_mem_size = 1;
show variables like 'txsql_optimizer_context_max_mem_size';
Variable_name	Value
txsql_optimizer_context_max_mem_size	1
SELECT * FROM t1 LEFT JOIN t2 USING(f1) WHERE f1='bla';
ERROR HY000: Optimization context cache isn't enabled due to memory limitation.
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	4
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	1
Txsql_parallel_stmt_thread_refused	0
DROP TABLE t1,t2;
SET txsql_optimizer_context_max_mem_size = @saved_optimizer_context_max_mem_size;

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
Txsql_parallel_stmt_executed	7
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	1
Txsql_parallel_stmt_thread_refused	0
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
Txsql_parallel_stmt_executed	7
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	4
Txsql_parallel_stmt_thread_refused	0
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
SELECT /*+PARALLEL(0)*/ * FROM t1;
f1	f2
bla	blah
blc	blad
bld	blad
SELECT /*+PARALLEL(1)*/ * FROM t1;
f1	f2
bla	blah
blc	blad
bld	blad
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	8
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_memory_refused	4
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
Warning	8024	parallel hint has no effect, not the chosen table.
Warnings:
bla	blah
blc	blad
bld	blad
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	8
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	1
Txsql_parallel_stmt_memory_refused	4
Txsql_parallel_stmt_thread_refused	0
# test counting the hint of qb
select /*+ PARALLEL(t1@qb1 5)*/ * from (select /*+ qb_name(qb1)*/ t1.f1 as aa, t1.f2 as bb from t1) as v where v.aa > 'a';
aa	bb
bla	blah
blc	blad
bld	blad
select /*+ PARALLEL(t1@qb1 5) PARALLEL(12)*/ * from (select /*+ qb_name(qb1)*/ t1.f1 as aa, t1.f2 as bb from t1) as v where v.aa > 'a';
aa	bb
Warning	3126	Hint PARALLEL(12) is ignored as conflicting/duplicated
Warnings:
bla	blah
blc	blad
bld	blad
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	8
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	3
Txsql_parallel_stmt_memory_refused	4
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
Txsql_parallel_stmt_executed	9
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	3
Txsql_parallel_stmt_memory_refused	4
Txsql_parallel_stmt_thread_refused	0
drop table t1;
# --include set_px_to_saved.inc

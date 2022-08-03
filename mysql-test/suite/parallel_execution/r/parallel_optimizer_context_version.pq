###############################################
# pq optimization context version test
###############################################
# 1) test copying outline version
# 2) test copying optimizer cost version
# 3) test copying rewriter plugin version

# 1) test copying outline version

SET @cdb_opt_outline_saved = @@global.cdb_opt_outline_enabled;
CREATE TABLE t1 (
f1 varchar(16) collate latin1_swedish_ci PRIMARY KEY,
f2 varchar(16) collate latin1_swedish_ci
);
CREATE TABLE t2 (
f1 varchar(16) collate latin1_swedish_ci PRIMARY KEY,
f3 varchar(16) collate latin1_swedish_ci
);
INSERT INTO t1 VALUES ('bla','blah');
INSERT INTO t1 VALUES ('blc','blad');
INSERT INTO t1 VALUES ('bld','blad');
INSERT INTO t2 VALUES ('bla','sheep');
INSERT INTO t2 VALUES ('blc','sheep');
INSERT INTO t2 VALUES ('bld','sheep');
set global cdb_opt_outline_enabled = on;
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
SELECT * FROM t1 LEFT JOIN t2 on t1.f1=t2.f1;
f1	f2	f1	f3
bla	blah	bla	sheep
blc	blad	blc	sheep
bld	blad	bld	sheep
outline "SELECT * FROM t1 LEFT JOIN t2 on t1.f1=t2.f1" set outline_info "OUTLINE:SELECT * FROM t1 use index(primary) LEFT JOIN t2 on t1.f1=t2.f1";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	8fd045e290fdd76ff2cb44892f4f1f8a115651dbee171f5b5443a35d9e5409d8	select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`f1` = `t2`.`f1`)))	select * from (`test`.`t1` USE INDEX (PRIMARY) left join `test`.`t2` on((`t1`.`f1` = `t2`.`f1`)))
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`f1` = `t2`.`f1`)))	select * from (`test`.`t1` USE INDEX (PRIMARY) left join `test`.`t2` on((`t1`.`f1` = `t2`.`f1`)))	0
truncate table mysql.outline;
outline flush;
SET @@global.cdb_opt_outline_enabled = @cdb_opt_outline_saved;

# 2) test copying optimizer cost version

SELECT * FROM t1 LEFT JOIN t2 on t1.f1=t2.f1;
f1	f2	f1	f3
bla	blah	bla	sheep
blc	blad	blc	sheep
bld	blad	bld	sheep
DROP TABLE t2;
DROP TABLE t1;
SELECT * FROM mysql.server_cost;
cost_name	cost_value	last_update	comment	default_value
disk_temptable_create_cost	NULL	#	NULL	20
disk_temptable_row_cost	NULL	#	NULL	0.5
key_compare_cost	NULL	#	NULL	0.05
memory_temptable_create_cost	NULL	#	NULL	1
memory_temptable_row_cost	NULL	#	NULL	0.1
row_evaluate_cost	NULL	#	NULL	0.1
FLUSH OPTIMIZER_COSTS;

# 3) test copying rewriter plugin version

CREATE TABLE t1 ( c1 INTEGER primary key, c2 INTEGER );
CREATE TABLE t2 ( c1 INTEGER primary key, c2 INTEGER );
INSERT INTO t1 VALUES (1, 2), (2, 0), (3, 1), (4, 3), (5, 4), (6, 1), (7, 2);
INSERT INTO t2 VALUES (1, 1), (3, 3), (4, 0), (6, 3), (9, 1), (8, 10);
# Query rewrite plugin was installed.
SELECT * FROM t1 JOIN t2 ON ( t1.c1 = t2.c1 AND t2.c2 > 2 );
c1	c2	c1	c2
3	1	3	3
6	1	6	3
SET sql_mode = 'PIPES_AS_CONCAT';
INSERT INTO query_rewrite.rewrite_rules( pattern, replacement  )
VALUES ( 'SELECT * FROM test.t1 JOIN test.t2 ON ' ||
'( test.t1.c1 = test.t2.c1 AND test.t1.c2 > ? )',
'SELECT * FROM test.t1 JOIN test.t2 ON ' ||
'( test.t1.c1 = test.t2.c1 AND test.t2.c2 > ? )' );
CALL query_rewrite.flush_rewrite_rules();
SELECT * FROM query_rewrite.rewrite_rules;
id	pattern	pattern_database	replacement	enabled	message
1	SELECT * FROM test.t1 JOIN test.t2 ON ( test.t1.c1 = test.t2.c1 AND test.t1.c2 > ? )	NULL	SELECT * FROM test.t1 JOIN test.t2 ON ( test.t1.c1 = test.t2.c1 AND test.t2.c2 > ? )	YES	NULL
SET sql_mode = DEFAULT;
DROP TABLE t1;
DROP TABLE t2;
Warnings:
Warning	1620	Plugin is busy and will be uninstalled on shutdown
# Query rewrite plugin was queued for uninstalling.

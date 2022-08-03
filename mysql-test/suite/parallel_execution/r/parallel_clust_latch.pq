create table t2 (a int unique);
insert into t2 (a) values (1);
insert into t2 (a) values (2000);
begin;
insert into t2 (a) values (1024);
insert into t2 (a) values (24);
# --include save_and_set_px.inc
EXPLAIN select * from t2 force index(a);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	NULL	a	5	NULL	4	100.00	Parallel scan (4 workers); Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a` from `test`.`t2` FORCE INDEX (`a`)
select * from t2 force index(a);
a
1
2000
# --include set_px_to_saved.inc
rollback;
drop table t2;

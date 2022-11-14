set @saved_innodb_backquery_enable = @@global.innodb_backquery_enable;
set @saved_innodb_backquery_window = @@global.innodb_backquery_window;
set global innodb_backquery_enable = ON;
create table t1(id int primary key, c1 varchar(20));
insert into t1 values(1,'aaa'),(2,'bbb'),(3,'ccc'),(4,'ddd'),(5,'eee');
create table t2(id int primary key, c1 varchar(20));
insert into t2 values(1,'adaa'),(2,'bdbb'),(3,'cdcc'),(4,'ddcd'),(5,'eefe');
create table externally_stored_field_table(id int, c1 text) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
insert into externally_stored_field_table values(1,repeat('a',60000)),(2,'aaaa');
basic test
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
length(c1)
4
different table with different timestamp
c1	id
aaa	1
abcdefg	4
bbb	2
ccc	3
c1	id
aaa	1
bbb	2
ccc	3
ddd	4
eee	5
c1	id
aaa	1
bbb	2
ccc	3
ddd	4
eee	5
c1	id
aaa	1
bbb	2
ccc	3
ddd	4
eee	5
id	c1
1	adaa
2	bdbb
3	cdcc
4	ddcd
5	eefe
c1	id	c1
aaa	1	newabcd
bbb	2	bdbb
ccc	3	cdcc
ddd	4	ddcd
eee	5	eefe
c1	id	c1
aaa	1	newabcd
abcdefg	4	ddcd
bbb	2	bdbb
ccc	3	cdcc
c1	id	c1
aaa	1	adaa
abcdefg	4	ddcd
bbb	2	bdbb
ccc	3	cdcc
c1	id	c1	t3c1
aaa	1	adaa	adaa
abcdefg	4	ddcd	ddcd
bbb	2	bdbb	bdbb
ccc	3	cdcc	cdcc
test grammar
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'as of timestamp '12345'' at line 1
test create select
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
test insert select
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
test explain
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t7	NULL	index	NULL	c1	83	NULL	5	100.00	Parallel scan (4 workers); Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	5	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t7`.`id` AS `id`,`test`.`t7`.`c1` AS `c1` from `test`.`t7`
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t7	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select `test`.`t7`.`id` AS `id`,`test`.`t7`.`c1` AS `c1` from `test`.`t7`
test update select
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
id	c1
1	ggg
2	ggg
3	ggg
4	ggg
5	eee
id	c1
1	ddd
2	ddd
3	ddd
4	ddd
5	ddd
test same table with different timestamps
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
id	c1
1	aaa
2	bbb
3	ccc
4	ddd
5	eee
test isolation level
Variable_name	Value
transaction_isolation	REPEATABLE-READ
tx_isolation	REPEATABLE-READ
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
id	c1
1	aaa
2	bbb
3	ccc
4	abcdefg
test alter table
id	c1	c2
1	aaa	NULL
2	bbb	NULL
3	ccc	NULL
4	abcdefg	NULL
id	c1	c2
1	aaa	NULL
2	bbb	NULL
3	ccc	NULL
4	ddd	NULL
5	eee	NULL
id	c1	c2
1	aaa	NULL
2	bbb	NULL
3	ccc	NULL
4	abcdefg	NULL
test locking clause
ERROR 42000: This version of MySQL doesn't yet support 'Using locking clause in combination with backquery'
ERROR 42000: This version of MySQL doesn't yet support 'Using locking clause in combination with backquery'
ERROR 42000: This version of MySQL doesn't yet support 'Using locking clause in combination with backquery'
current data in partition table
id	name	part
1	a	1
2	a	1
3	bbb	2
4	jak	2
5	jone	343
history data in partition table
id	name	part
data in partition table after delete
id	name	part
1	a	1
2	a	1
4	jak	2
data in partition table before delete
id	name	part
1	a	1
2	a	1
3	bbb	2
4	jak	2
5	jone	343
test externally stored field
length(c1)
4
length(c1)
60000
test backquery window
Variable_name	Value
innodb_backquery_window	3
cleanup
select @saved_innodb_backquery_window;
@saved_innodb_backquery_window
86400
set global innodb_backquery_window=@saved_innodb_backquery_window;
set global innodb_backquery_enable=@saved_innodb_backquery_enable;
drop table if exists t1;
drop table if exists t2;
drop table if exists t3;
drop table if exists t4;
drop table if exists t5;
drop table if exists t6;
drop table if exists t7;
drop table if exists partition_table;
drop table if exists externally_stored_field_table;

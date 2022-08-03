################################################
# Test instant MODIFY COLUMN IN SECONDARY INDEX
################################################
create table t1 (c1 int primary key, c2 int, c3 varchar(30), unique key idx_c2 (c2));
insert into t1 values(0, 1, 'xxx');
insert into t1 values(1, 2147483646, 'aaa');
insert into t1 values(2, 2147483647, 'bbb');
insert into t1 values(3, 2147483648, 'ccc');
ERROR 22003: Out of range value for column 'c2' at row 1
select * from t1 where c2 <= 2147483648;
c1	c2	c3
0	1	xxx
1	2147483646	aaa
2	2147483647	bbb
alter table t1 modify c2 bigint default null, algorithm=instant;
select * from t1 where c2 <= 2147483648;
c1	c2	c3
0	1	xxx
1	2147483646	aaa
2	2147483647	bbb
insert into t1 values(3, 2147483648, 'ccc');
insert into t1 values(4, 2147483649, 'ddd');
select * from t1 where c2 < 2147483648;
c1	c2	c3
0	1	xxx
1	2147483646	aaa
2	2147483647	bbb
select * from t1 where c2 = 2147483648;
c1	c2	c3
3	2147483648	ccc
select * from t1 where c2 > 2147483648;
c1	c2	c3
4	2147483649	ddd
insert into t1 values(5, 2147483645, 'eee');
select * from t1 order by c2;
c1	c2	c3
0	1	xxx
5	2147483645	eee
1	2147483646	aaa
2	2147483647	bbb
3	2147483648	ccc
4	2147483649	ddd
delete from t1 where c2 = 1;
select * from t1 order by c2;
c1	c2	c3
5	2147483645	eee
1	2147483646	aaa
2	2147483647	bbb
3	2147483648	ccc
4	2147483649	ddd
update t1 set c2=2147483651 where c2 = 2147483647;
select * from t1 order by c2;
c1	c2	c3
5	2147483645	eee
1	2147483646	aaa
3	2147483648	ccc
4	2147483649	ddd
2	2147483651	bbb
check table t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
drop table t1;
################################################
# Test instant MODIFY COLUMN IN PRIMARY KEY
################################################
create table t1 (c1 int auto_increment primary key, c2 int, c3 varchar(30));
insert into t1 values(1, 0,  'xxx');
insert into t1 values(2147483646, 1, 'aaa');
insert into t1 values(2147483647, 2, 'bbb');
insert into t1 values(2147483648, 3, 'ccc');
ERROR 22003: Out of range value for column 'c1' at row 1
select * from t1 where c1 <= 2147483648;
c1	c2	c3
1	0	xxx
2147483646	1	aaa
2147483647	2	bbb
alter table t1 modify c1 bigint, algorithm=instant;
select * from t1 where c1 <= 2147483648;
c1	c2	c3
1	0	xxx
2147483646	1	aaa
2147483647	2	bbb
insert into t1 values(2147483648, 3, 'ccc');
insert into t1 values(2147483649, 4, 'ddd');
select * from t1 where c1 < 2147483648;
c1	c2	c3
1	0	xxx
2147483646	1	aaa
2147483647	2	bbb
select * from t1 where c1 = 2147483648;
c1	c2	c3
2147483648	3	ccc
select * from t1 where c1 > 2147483648;
c1	c2	c3
2147483649	4	ddd
insert into t1 values(2147483645, 5, 'eee');
select * from t1 order by c1;
c1	c2	c3
1	0	xxx
2147483645	5	eee
2147483646	1	aaa
2147483647	2	bbb
2147483648	3	ccc
2147483649	4	ddd
delete from t1 where c1 = 1;
select * from t1 order by c1;
c1	c2	c3
2147483645	5	eee
2147483646	1	aaa
2147483647	2	bbb
2147483648	3	ccc
2147483649	4	ddd
update t1 set c1=2147483651 where c1 = 2147483647;
select * from t1 order by c1;
c1	c2	c3
2147483645	5	eee
2147483646	1	aaa
2147483648	3	ccc
2147483649	4	ddd
2147483651	2	bbb
check table t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
drop table t1;
#
# Test for partition table
#
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a char(150), b varchar(150),
c char(150), d varchar(150), e int) DEFAULT CHARACTER SET utf8mb4
PARTITION BY HASH(id) PARTITIONS 3;
INSERT INTO t1 VALUES(1, repeat('a', 130), repeat('b', 130), repeat('c', 130), repeat('d', 130), 10000);
alter table t1 modify a char(251), modify b varchar(251), algorithm=instant;
alter table t1 modify id bigint auto_increment, algorithm=instant;
INSERT INTO t1 VALUES(2, repeat('a', 200), repeat('b', 130), repeat('c', 130), repeat('d', 130), -1);
INSERT INTO t1 VALUES(3, repeat('a', 200), repeat('b', 200), repeat('c', 130), repeat('d', 130), 0);
alter table t1 modify c varchar(251), modify d char(251), algorithm=instant;
INSERT INTO t1 VALUES(4, repeat('a', 200), repeat('b', 200), repeat('c', 200), repeat('d', 130), -2147483648);
INSERT INTO t1 VALUES(5, repeat('a', 200), repeat('b', 200), repeat('c', 200), repeat('d', 200), 2147483647);
alter table t1 modify e bigint, algorithm=instant;
INSERT INTO t1 VALUES(6, repeat('a', 200), repeat('b', 200), repeat('c', 200), repeat('d', 200), 1023);
select * from t1;
id	a	b	c	d	e
1	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	10000
2	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	-1
3	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	0
4	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	-2147483648
5	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	2147483647
6	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	1023
update t1 set e=e+1;
select * from t1;
id	a	b	c	d	e
1	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	10001
2	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	0
3	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	1
4	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	-2147483647
5	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	2147483648
6	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd	1024
update t1 set d='xx';
check table t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
select pos, mtype, prtype, len from information_schema.innodb_instant_modified_cols;
POS	MTYPE	PRTYPE	LEN
1	6	1283	4
2	13	16711934	600
3	12	16715791	600
4	13	16711934	600
5	12	16715791	600
6	6	1027	4
drop table t1;
#
# Test for table with 2 level btree
#
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a
char(150), b varchar(150), c char(150), d varchar(150), e int);
CREATE PROCEDURE populate_t1()
begin
declare i int default 1;
declare has_error int default 0;
DECLARE CONTINUE HANDLER FOR 1062 SET has_error = 1;
while (i <= 1000) DO
insert into t1(a,b,c,d,e)
VALUES(REPEAT('a',150), REPEAT('b',150), REPEAT('c',150), REPEAT('d',150), i);
set i = i + 1;
end while;
end|
call populate_t1();
select count(*) from t1;
count(*)
1000
alter table t1 modify a char(251), modify b varchar(251), algorithm=instant;
alter table t1 modify id bigint auto_increment, algorithm=instant;
INSERT INTO t1 (a,b,c,d,e) VALUES(repeat('a', 200), repeat('b', 130), repeat('c', 130), repeat('d', 130), -1);
INSERT INTO t1 (a,b,c,d,e) VALUES(repeat('a', 200), repeat('b', 200), repeat('c', 130), repeat('d', 130), 0);
alter table t1 modify c varchar(251), modify d char(251), algorithm=instant;
INSERT INTO t1 (a,b,c,d,e) VALUES(repeat('a', 200), repeat('b', 200), repeat('c', 200), repeat('d', 130), -2147483648);
INSERT INTO t1 (a,b,c,d,e) VALUES(repeat('a', 200), repeat('b', 200), repeat('c', 200), repeat('d', 200), 2147483647);
alter table t1 modify e bigint, algorithm=instant;
INSERT INTO t1 (a,b,c,d,e) VALUES(repeat('a', 200), repeat('b', 200), repeat('c', 200), repeat('d', 200), 1023);
select count(*) from t1;
count(*)
1005
update t1 set e=e+1;
select count(*) from t1;
count(*)
1005
update t1 set d='xx';
check table t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
select pos, mtype, prtype, len from information_schema.innodb_instant_modified_cols;
POS	MTYPE	PRTYPE	LEN
1	6	1283	4
2	13	16711934	600
3	12	16715791	600
4	13	16711934	600
5	12	16715791	600
6	6	1027	4
drop procedure populate_t1;
drop table t1;

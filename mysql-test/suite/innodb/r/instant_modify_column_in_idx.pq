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
create table t2 (id int not null AUTO_INCREMENT,c1_char char(10) CHARACTER SET latin1 default 'abc', c2_varchar varchar(10) CHARACTER SET utf8 not null, c3_binary binary(10),      c4_varbinary varbinary(10) not null default b'111',c5_tint tinyint default 0, c6_sint smallint,c7_mint mediumint signed not null,c8_int int unsigned unique,c9_bint bigint,PRIMARY KEY (id),KEY t1_key (c9_bint));
Warnings:
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
alter table t2 modify c8_int bigint unsigned unique, algorithm=instant;
ERROR 0A000: ALGORITHM=INSTANT is not supported. Reason: Need to rebuild the table to change column type. Try ALGORITHM=COPY/INPLACE.
alter table t2 modify c8_int bigint unsigned, algorithm=instant;
drop table t2;
create table t1(id int,c1 int not null unique);
insert into t1 values(1,1),(2,2);
alter table t1 modify column c1 bigint not null unique, algorithm=instant;
ERROR 0A000: ALGORITHM=INSTANT is not supported. Reason: Need to rebuild the table to change column type. Try ALGORITHM=COPY/INPLACE.
alter table t1 modify column c1 bigint not null, algorithm=instant;
insert into t1 values(1,1);
ERROR 23000: Duplicate entry '1' for key 't1.c1'
select * from t1;
id	c1
1	1
2	2
drop table t1;
create table t1 (id int not null AUTO_INCREMENT, c1_char char(10) CHARACTER SET latin1 default 'abc', c2_varchar varchar(10) CHARACTER SET utf8 not null, c3_binary binary(10), c4_varbinary varbinary(10) not null default b'111', c5_tint tinyint default 0, c6_sint smallint, c7_mint mediumint signed not null, c8_int int unsigned, c9_bint bigint, PRIMARY KEY (id)) partition by range(id) (partition p0 values less than (100),partition p1 values less than (200));
Warnings:
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
alter table t1 add partition (partition p2 values less than (1500));
replace into t1 set id=952,c1_char='dddddd',c2_varchar='ggg',c7_mint=9;
alter table t1 modify c7_mint bigint signed not null,modify c4_varbinary binary(20) not null default b'111', modify id bigint not null AUTO_INCREMENT, algorithm=instant;
alter table t1 add partition (partition p3 values less than Maxvalue);
checksum table t1;
Table	Checksum
test.t1	2469892167
drop table t1;
alter table t1 modify id bigint not null AUTO_INCREMENT, algorithm=instant;
replace into t1 set id=2147483597,c1_char='dddddd',c2_varchar='ggg',c7_mint=9;
replace into t1 set id=2147483646,c1_char='dddddd',c2_varchar='ggg',c7_mint=9;
insert into t1 value (2147483648, 'ff', '11', '22', '33', '48', -26263, 2685513, 994392385, -5203789411292461781);
insert into t1 value (2147483697, '++', '..', '--', 'AA', '97', -26214, 2685562, 994392434, -5203789411292461732);
update t1 set c1_char='fgfgffgggg', c3_binary='c3', c5_tint=10 where id>=1 and id<=51;
update t1 set c1_char='fgfgffgggg', c3_binary='c3', c5_tint=10 where id>=2147483699 and id<=2147483749;
replace into t1 set id=2147483606,c1_char='dddddd',c2_varchar='ggg',c7_mint=9;
insert into t1 set c2_varchar='ffff',c7_mint=-8,c9_bint=-320.68e+10,c5_tint=NULL;
insert into t1 set id=2147483647,c2_varchar='j',c7_mint=16;
alter table t1 modify c8_int bigint unsigned, algorithm=instant;
replace into t1 set id=2147483651,c1_char='dddddd',c2_varchar='ggg',c7_mint=9;
replace into t1 set id=2147483700,c1_char='dddddd',c2_varchar='ggg',c7_mint=9;
alter table t1 modify c1_char varchar(20) CHARACTER SET latin1 default 'fgte', algorithm=instant;
drop table t1;
create table t5 (c1 varchar(5), c2 varchar(5), c3 varchar(5), c4 varchar(5), c5 varchar(5),primary key(c1),unique key(c2), key(c3), key(c4));
alter table t5 modify c1 char(7),modify c2 char(7), algorithm=instant;
drop table t5;
create table t5 (c1 varchar(5), c2 varchar(5), c3 varchar(5), c4 varchar(5), c5 varchar(5),primary key(c1),unique key(c2), key(c3), key(c4));
alter table t5 modify c2 char(7),modify c3 char(7), algorithm=instant;
drop table t5;
create table t5 (c1 varchar(5), c2 varchar(5), c3 varchar(5), c4 varchar(5), c5 varchar(5),primary key(c1),unique key(c2), key(c3), key(c4));
alter table t5 modify c1 char(7),modify c3 char(7), algorithm=instant;
drop table t5;
create table t1(id int not null primary key,c1 varchar(10),c2 varchar(10) default 'abc')default character set='latin1';
insert into t1(id,c1) values(1,'ddd');
alter table t1 modify column id bigint,algorithm=instant;
select * from t1;
id	c1	c2
1	ddd	abc
checksum table t1;
Table	Checksum
test.t1	3322152231
begin;
update t1 set c2='eee';
rollback;
select * from t1;
id	c1	c2
1	ddd	abc
checksum table t1;
Table	Checksum
test.t1	3322152231
drop table t1;
CREATE TABLE sbtest1 (id int(11) NOT NULL AUTO_INCREMENT, k int(11) NOT NULL DEFAULT '0', c char(120) NOT NULL DEFAULT '', pad char(60) NOT NULL DEFAULT '', PRIMARY KEY (id), KEY k_1 (k),key index_2(k, c)) DEFAULT CHARSET=latin1;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
insert into sbtest1 values(1,34,'ffdfff-jjjjjj-kdksjsjd','ggggggggggggg');
alter table sbtest1 add column i int,algorithm=instant;
alter table sbtest1 add column j char(30) default 'abc',algorithm=instant;
alter table sbtest1 modify column c varchar(240) not null default '',algorithm=instant;
alter table sbtest1 modify column id bigint NOT NULL AUTO_INCREMENT,algorithm=instant;
select * from sbtest1;
id	k	c	pad	i	j
1	34	ffdfff-jjjjjj-kdksjsjd	ggggggggggggg	NULL	abc
checksum table sbtest1;
Table	Checksum
test.sbtest1	1100557903
begin;
update sbtest1 set j='aaaaaaaaaaa' where id=1;
rollback;
select * from sbtest1;
id	k	c	pad	i	j
1	34	ffdfff-jjjjjj-kdksjsjd	ggggggggggggg	NULL	abc
checksum table sbtest1;
Table	Checksum
test.sbtest1	1100557903
drop table sbtest1;

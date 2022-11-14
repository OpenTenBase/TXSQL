test1
CREATE TABLE `t1` (
`id` int(11) NOT NULL AUTO_INCREMENT,
`k` int(11) NOT NULL DEFAULT '0',
`c` char(120) NOT NULL DEFAULT '',
`pad` char(60) NOT NULL DEFAULT '',
PRIMARY KEY (`id`),
KEY `k_1` (`k`)
) ENGINE=InnoDB AUTO_INCREMENT=455115 DEFAULT CHARSET=latin1;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
insert into t1 values(455119,19982,'45710173166-55970366418-46110319583-32666806665-59107758310-03100962915-56996749429-13143329136-70160501983-35446519008','58059266282-12236046963-36829385701-89632835700-73658374282');
alter table t1 modify column c varchar(240) not null default '',algorithm=instant;
checksum table t1;
Table	Checksum
test.t1	2766999341
begin;
delete from t1 where id=455119;
insert into t1 values(455119,19982,'45710173166-55970366418-46110319583-32666806665-59107758310-03100962915-56996749429-13143329136-70160501983-35446519008','58059266282-12236046963-36829385701-89632835700-73658374282');
rollback;
select * from t1;
id	k	c	pad
455119	19982	45710173166-55970366418-46110319583-32666806665-59107758310-03100962915-56996749429-13143329136-70160501983-35446519008	58059266282-12236046963-36829385701-89632835700-73658374282
checksum table t1;
Table	Checksum
test.t1	2766999341
drop table t1;
test2
create table t1(id int, c1 int, c2 int);
insert into t1 values(1, -2147483648, 2147483647);
insert into t1 values(2, 9981, -1);
select * from t1;
id	c1	c2
1	-2147483648	2147483647
2	9981	-1
alter table t1 modify column id bigint, modify column c1 bigint, modify column c2 bigint,algorithm=instant;
select * from t1;
id	c1	c2
1	-2147483648	2147483647
2	9981	-1
checksum table t1;
Table	Checksum
test.t1	1273896120
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2
1	-2147483648	2147483647
2	9981	-1
checksum table t1;
Table	Checksum
test.t1	1273896120
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2
1	-2147483648	2147483647
2	9981	-1
checksum table t1;
Table	Checksum
test.t1	1273896120
drop table t1;
test3
create table t1(id int, c1 mediumint, c2 mediumint);
insert into t1 values(1, -8388608, 8388607);
insert into t1 values(2, 9981, -2);
select * from t1;
id	c1	c2
1	-8388608	8388607
2	9981	-2
alter table t1 modify column id bigint, modify column c1 bigint, modify column c2 bigint,algorithm=instant;
select * from t1;
id	c1	c2
1	-8388608	8388607
2	9981	-2
checksum table t1;
Table	Checksum
test.t1	189069453
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2
1	-8388608	8388607
2	9981	-2
checksum table t1;
Table	Checksum
test.t1	189069453
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2
1	-8388608	8388607
2	9981	-2
checksum table t1;
Table	Checksum
test.t1	189069453
drop table t1;
test4
create table t1(id int, c1 smallint, c2 smallint);
insert into t1 values(1, -32768, 32767);
insert into t1 values(2, 9981, -9981);
select * from t1;
id	c1	c2
1	-32768	32767
2	9981	-9981
alter table t1 modify column id bigint, modify column c1 bigint, modify column c2 bigint,algorithm=instant;
select * from t1;
id	c1	c2
1	-32768	32767
2	9981	-9981
checksum table t1;
Table	Checksum
test.t1	3546258974
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2
1	-32768	32767
2	9981	-9981
checksum table t1;
Table	Checksum
test.t1	3546258974
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2
1	-32768	32767
2	9981	-9981
checksum table t1;
Table	Checksum
test.t1	3546258974
drop table t1;
test5
create table t1(id int, c1 tinyint, c2 tinyint);
insert into t1 values(1, -128, 127);
insert into t1 values(2, 98, -105);
select * from t1;
id	c1	c2
1	-128	127
2	98	-105
alter table t1 modify column id bigint, modify column c1 bigint, modify column c2 bigint,algorithm=instant;
select * from t1;
id	c1	c2
1	-128	127
2	98	-105
checksum table t1;
Table	Checksum
test.t1	328623460
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2
1	-128	127
2	98	-105
checksum table t1;
Table	Checksum
test.t1	328623460
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2
1	-128	127
2	98	-105
checksum table t1;
Table	Checksum
test.t1	328623460
drop table t1;
test6
create table t1(id int, c1 tinyint, c2 tinyint);
insert into t1 values(1, -128, 127);
insert into t1 values(2, 98, -105);
select * from t1;
id	c1	c2
1	-128	127
2	98	-105
alter table t1 modify column id bigint, modify column c1 mediumint, modify column c2 mediumint,algorithm=instant;
select * from t1;
id	c1	c2
1	-128	127
2	98	-105
checksum table t1;
Table	Checksum
test.t1	3009154580
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2
1	-128	127
2	98	-105
checksum table t1;
Table	Checksum
test.t1	3009154580
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2
1	-128	127
2	98	-105
checksum table t1;
Table	Checksum
test.t1	3009154580
drop table t1;
test7
create table t1(id int, c1 tinyint unsigned, c2 tinyint unsigned);
insert into t1 values(1, 255, 127);
insert into t1 values(2, 98, 128);
select * from t1;
id	c1	c2
1	255	127
2	98	128
alter table t1 modify column id bigint, modify column c1 int unsigned, modify column c2 int unsigned,algorithm=instant;
select * from t1;
id	c1	c2
1	255	127
2	98	128
checksum table t1;
Table	Checksum
test.t1	2471765203
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2
1	255	127
2	98	128
checksum table t1;
Table	Checksum
test.t1	2471765203
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2
1	255	127
2	98	128
checksum table t1;
Table	Checksum
test.t1	2471765203
drop table t1;
test8
create table t1(id int, c1 int zerofill, c2 int zerofill);
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
insert into t1 values(1, 2147483648, 2147483647);
insert into t1 values(2, 4294967295, 32392);
select * from t1;
id	c1	c2
1	2147483648	2147483647
2	4294967295	0000032392
alter table t1 modify column id bigint, modify column c1 bigint zerofill, modify column c2 bigint zerofill,algorithm=instant;
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
insert into t1 values(9, 18446744073709551615, 4294967299);
select * from t1;
id	c1	c2
1	00000000002147483648	00000000002147483647
2	00000000004294967295	00000000000000032392
9	18446744073709551615	00000000004294967299
checksum table t1;
Table	Checksum
test.t1	1017808750
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2
1	00000000002147483648	00000000002147483647
2	00000000004294967295	00000000000000032392
9	18446744073709551615	00000000004294967299
checksum table t1;
Table	Checksum
test.t1	1017808750
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2
1	00000000002147483648	00000000002147483647
2	00000000004294967295	00000000000000032392
9	18446744073709551615	00000000004294967299
checksum table t1;
Table	Checksum
test.t1	1017808750
begin;
update t1 set id=10 where id=9;
rollback;
select * from t1;
id	c1	c2
1	00000000002147483648	00000000002147483647
2	00000000004294967295	00000000000000032392
9	18446744073709551615	00000000004294967299
checksum table t1;
Table	Checksum
test.t1	1017808750
drop table t1;
test9
create table t1(id int, c1 char(10), c2 varchar(10), c3 char(10)) default charset=latin1;
insert into t1 values(1, 'column11', 'column12', 'column13');
insert into t1 values(2, 'column21', 'column22', 'column23');
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
alter table t1 modify column id bigint, modify column c1 varchar(20), modify column c2 char(20), modify column c3 char(20), algorithm=instant;
insert into t1 values(8, 'column8111111', 'column82222222222222', 'column83333333');
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	3802302165
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	3802302165
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	3802302165
begin;
update t1 set id=10 where id=8;
rollback;
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	3802302165
drop table t1;
test10
create table t1(id int, c1 char(10), c2 varchar(10), c3 char(10)) default charset=utf8;
Warnings:
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
insert into t1 values(1, 'column11', 'column12', 'column13');
insert into t1 values(2, 'column21', 'column22', 'column23');
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
alter table t1 modify column id bigint, modify column c1 varchar(20), modify column c2 char(20), modify column c3 char(20), algorithm=instant;
insert into t1 values(8, 'column8111111', 'column82222222222222', 'column83333333');
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	2496892777
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	2496892777
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	2496892777
begin;
update t1 set id=10 where id=8;
rollback;
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	2496892777
drop table t1;
test11
create table t1(id int, c1 char(10), c2 varchar(10), c3 char(10)) default charset=utf8mb4;
insert into t1 values(1, 'column11', 'column12', 'column13');
insert into t1 values(2, 'column21', 'column22', 'column23');
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
alter table t1 modify column id bigint, modify column c1 varchar(20), modify column c2 char(20), modify column c3 char(20), algorithm=instant;
insert into t1 values(8, 'column8111111', 'column82222222222222', 'column83333333');
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	23808844
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	23808844
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	23808844
begin;
update t1 set id=10 where id=8;
rollback;
select * from t1;
id	c1	c2	c3
1	column11	column12	column13
2	column21	column22	column23
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	23808844
drop table t1;
test12
create table t1(id int, c1 binary(10), c2 varbinary(10), c3 binary(10));
insert into t1 values(1, 'column11', 'column12', 'column13');
insert into t1 values(2, 'column21', 'column22', 'column23');
select * from t1;
id	c1	c2	c3
1	column11  	column12	column13  
2	column21  	column22	column23  
alter table t1 modify column id bigint, modify column c1 varbinary(20), modify column c2 binary(20), modify column c3 binary(20), algorithm=instant;
insert into t1 values(8, 'column8111111', 'column82222222222222', 'column83333333');
select * from t1;
id	c1	c2	c3
1	column11  	column12            	column13            
2	column21  	column22            	column23            
8	column8111111	column82222222222222	column83333333      
checksum table t1;
Table	Checksum
test.t1	2246994574
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2	c3
1	column11  	column12            	column13            
2	column21  	column22            	column23            
8	column8111111	column82222222222222	column83333333      
checksum table t1;
Table	Checksum
test.t1	2246994574
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2	c3
1	column11  	column12            	column13            
2	column21  	column22            	column23            
8	column8111111	column82222222222222	column83333333      
checksum table t1;
Table	Checksum
test.t1	2246994574
begin;
update t1 set id=10 where id=8;
rollback;
select * from t1;
id	c1	c2	c3
1	column11  	column12            	column13            
2	column21  	column22            	column23            
8	column8111111	column82222222222222	column83333333      
checksum table t1;
Table	Checksum
test.t1	2246994574
drop table t1;
test13
set session character_set_client=utf8mb4;
set session character_set_connection=utf8mb4;
set session character_set_results=utf8mb4;
create table t1(id int, c1 char(10), c2 varchar(10), c3 char(10)) ENGINE=InnoDB default charset=utf8mb4;
insert into t1 values(1, '测试用例11', '测试用例12', '你好你好你好你好你好');
insert into t1 values(2, '测试用例21', '测试用例22', '测试用例233333');
select * from t1;
id	c1	c2	c3
1	测试用例11	测试用例12	你好你好你好你好你好
2	测试用例21	测试用例22	测试用例233333
alter table t1 modify column id bigint, modify column c1 varchar(20), modify column c2 char(20), modify column c3 char(20), algorithm=instant;
insert into t1 values(8, 'column8111111', 'column82222222222222', 'column83333333');
select * from t1;
id	c1	c2	c3
1	测试用例11	测试用例12	你好你好你好你好你好
2	测试用例21	测试用例22	测试用例233333
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	625812743
begin;
update t1 set id=3 where id=1;
rollback;
select * from t1;
id	c1	c2	c3
1	测试用例11	测试用例12	你好你好你好你好你好
2	测试用例21	测试用例22	测试用例233333
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	625812743
begin;
update t1 set id=4 where id=2;
rollback;
select * from t1;
id	c1	c2	c3
1	测试用例11	测试用例12	你好你好你好你好你好
2	测试用例21	测试用例22	测试用例233333
8	column8111111	column82222222222222	column83333333
checksum table t1;
Table	Checksum
test.t1	625812743
drop table t1;
test14
CREATE TABLE `sbtest7` (
`id` int(11) NOT NULL AUTO_INCREMENT,
`k` int(11) NOT NULL DEFAULT '0',
`c` char(120) NOT NULL DEFAULT '',
`pad` char(60) NOT NULL DEFAULT '',
PRIMARY KEY (`id`),
KEY `k_1` (`k`)
) ENGINE=InnoDB AUTO_INCREMENT=455115 DEFAULT CHARSET=latin1;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
insert into sbtest7 values(455119,19982,'45710173166-55970366418-46110319583-32666806665-59107758310-03100962915-56996749429-13143329136-70160501983-35446519008','58059266282-12236046963-36829385701-89632835700-73658374282');
alter table sbtest7 modify column c varchar(240) not null default '',algorithm=instant;
select * from sbtest7;
id	k	c	pad
455119	19982	45710173166-55970366418-46110319583-32666806665-59107758310-03100962915-56996749429-13143329136-70160501983-35446519008	58059266282-12236046963-36829385701-89632835700-73658374282
checksum table sbtest7;
Table	Checksum
test.sbtest7	2766999341
begin;
replace into sbtest7 values(455119,19982,'45710173166-55970366418-46110319583-32666806665-59107758310-03100962915-56996749429-13143329136-70160501983-35446519887777777777777','58059266282-12236046963-36829385701-89632835700-73658374282');
update sbtest7 set c = "abcdefg" where id=455119;
update sbtest7 set c = "45710173166-55970366418-46110319583-32666806665-59107758310-03100962915-56996749429-13143329136-70160501983-35446519008999999999999999";
rollback;
select * from sbtest7;
id	k	c	pad
455119	19982	45710173166-55970366418-46110319583-32666806665-59107758310-03100962915-56996749429-13143329136-70160501983-35446519008	58059266282-12236046963-36829385701-89632835700-73658374282
checksum table sbtest7;
Table	Checksum
test.sbtest7	2766999341
drop table sbtest7;
test15
set session character_set_client=utf8mb4;
set session character_set_connection=utf8mb4;
set session character_set_results=utf8mb4;
CREATE TABLE `t1` (
`id` int(11) NOT NULL AUTO_INCREMENT,
`k` int(11) NOT NULL DEFAULT '0',
`c` char(120) NOT NULL DEFAULT '',
`pad` char(60) NOT NULL DEFAULT '',
PRIMARY KEY (`id`),
KEY `k_1` (`k`)
) ENGINE=InnoDB AUTO_INCREMENT=455115 DEFAULT CHARSET=utf8mb4;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
insert into t1 values(455119,19982,'测试测试测试测试测试测试abcdefg测试','58059266282-测试');
alter table t1 modify column c varchar(240) not null default '',algorithm=instant;
checksum table t1;
Table	Checksum
test.t1	4030504194
begin;
update t1 set c = "测试测试测试测试测试测试abcdefg测试ddd测试";
rollback;
select * from t1;
id	k	c	pad
455119	19982	测试测试测试测试测试测试abcdefg测试	58059266282-测试
checksum table t1;
Table	Checksum
test.t1	4030504194
drop table t1;
test16
set session character_set_client=utf8mb4;
set session character_set_connection=utf8mb4;
set session character_set_results=utf8mb4;
CREATE TABLE `t1` (
`id` int NOT NULL AUTO_INCREMENT,
`k` int NOT NULL DEFAULT '0',
`c` char(120) BINARY NOT NULL DEFAULT '',
`pad` char(60) BINARY NOT NULL DEFAULT '',
PRIMARY KEY (`id`),
KEY `k_1` (`k`)
) ENGINE=InnoDB AUTO_INCREMENT=455115 DEFAULT CHARSET=utf8mb4;
Warnings:
Warning	1287	'BINARY as attribute of a type' is deprecated and will be removed in a future release. Please use a CHARACTER SET clause with _bin collation instead
Warning	1287	'BINARY as attribute of a type' is deprecated and will be removed in a future release. Please use a CHARACTER SET clause with _bin collation instead
insert into t1 values(455119,19982,'测试ab测试','58059266282-测试');
alter table t1 modify column c varchar(240) BINARY not null default '',algorithm=instant;
Warnings:
Warning	1287	'BINARY as attribute of a type' is deprecated and will be removed in a future release. Please use a CHARACTER SET clause with _bin collation instead
checksum table t1;
Table	Checksum
test.t1	2835087634
begin;
update t1 set c = "测试测试测试测试测试测试abcdefg测试ddd测试";
rollback;
select * from t1;
id	k	c	pad
455119	19982	测试ab测试	58059266282-测试
checksum table t1;
Table	Checksum
test.t1	2835087634
drop table t1;
create table t1(
c1 char(100),
c2 char(100),
c3 char(100),
c4 char(100),
c5 char(100),
c6 char(100),
c7 char(100),
c8 char(100),
c9 char(100),
c10 char(100),
c11 char(100),
c12 char(100),
c13 char(100),
c14 char(100),
c15 char(100),
c16 char(100),
c17 char(100),
c18 char(100),
c19 char(100),
c20 char(100),
c21 char(100),
c22 char(100),
c23 char(100),
c24 char(100),
c25 char(100),
c26 char(100),
c27 char(100),
c28 char(100),
c29 char(100),
c30 char(100),
c31 char(100),
c32 char(100),
c33 char(100),
c34 char(100),
c35 char(100),
c36 char(100),
c37 char(100),
c38 char(100),
c39 char(100),
c40 char(100),
c41 char(100),
c42 char(100),
c43 char(100),
c44 char(100),
c45 char(100),
c46 char(100),
c47 char(100),
c48 char(100),
c49 char(100),
c50 char(100),
c51 char(100),
c52 char(100),
c53 char(100),
c54 char(100),
c55 char(100),
c56 char(100),
c57 char(100),
c58 char(100),
c59 char(100),
c60 char(100),
c61 char(100),
c62 char(100),
c63 char(100),
c64 char(100),
c65 char(100),
c66 char(100),
c67 char(100),
c68 char(100),
c69 char(100),
c70 char(100),
c71 char(100),
c72 char(100),
c73 char(100),
c74 char(100),
c75 char(100),
c76 char(100),
c77 char(100),
c78 char(100),
c79 char(100),
c80 char(100),
c81 char(130)
) engine=innodb, default character set=utf8mb4;
insert into t1 values(
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('册', 130)
);
alter table t1 modify column c81 char(150),algorithm=instant;
checksum table t1;
Table	Checksum
test.t1	3265722400
begin;
update t1 set c81=repeat('策',150);
rollback;
checksum table t1;
Table	Checksum
test.t1	3265722400
drop table t1;
create table t1(
c1 char(100),
c2 char(100),
c3 char(100),
c4 char(100),
c5 char(100),
c6 char(100),
c7 char(100),
c8 char(100),
c9 char(100),
c10 char(100),
c11 char(100),
c12 char(100),
c13 char(100),
c14 char(100),
c15 char(100),
c16 char(100),
c17 char(100),
c18 char(100),
c19 char(100),
c20 char(100),
c21 char(100),
c22 char(100),
c23 char(100),
c24 char(100),
c25 char(100),
c26 char(100),
c27 char(100),
c28 char(100),
c29 char(100),
c30 char(100),
c31 char(100),
c32 char(100),
c33 char(100),
c34 char(100),
c35 char(100),
c36 char(100),
c37 char(100),
c38 char(100),
c39 char(100),
c40 char(100),
c41 char(100),
c42 char(100),
c43 char(100),
c44 char(100),
c45 char(100),
c46 char(100),
c47 char(100),
c48 char(100),
c49 char(100),
c50 char(100),
c51 char(100),
c52 char(100),
c53 char(100),
c54 char(100),
c55 char(100),
c56 char(100),
c57 char(100),
c58 char(100),
c59 char(100),
c60 char(100),
c61 char(100),
c62 char(100),
c63 char(100),
c64 char(100),
c65 char(100),
c66 char(100),
c67 char(100),
c68 char(100),
c69 char(100),
c70 char(100),
c71 char(100),
c72 char(100),
c73 char(100),
c74 char(100),
c75 char(100),
c76 char(100),
c77 char(100),
c78 char(100),
c79 char(100),
c80 char(100),
c81 char(150),
c82 char(150)
) engine=innodb, default character set=utf8mb4;
insert into t1 values(
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('册', 150),
repeat('侧', 150)
);
alter table t1 modify column c81 char(160), modify column c82 char(160),algorithm=instant;
checksum table t1;
Table	Checksum
test.t1	1555586046
begin;
update t1 set c81=repeat('策',150);
rollback;
checksum table t1;
Table	Checksum
test.t1	1555586046
drop table t1;
create table t1(
c1 char(100),
c2 char(100),
c3 char(100),
c4 char(100),
c5 char(100),
c6 char(100),
c7 char(100),
c8 char(100),
c9 char(100),
c10 char(100),
c11 char(100),
c12 char(100),
c13 char(100),
c14 char(100),
c15 char(100),
c16 char(100),
c17 char(100),
c18 char(100),
c19 char(100),
c20 char(100),
c21 char(100),
c22 char(100),
c23 char(100),
c24 char(100),
c25 char(100),
c26 char(100),
c27 char(100),
c28 char(100),
c29 char(100),
c30 char(100),
c31 char(100),
c32 char(100),
c33 char(100),
c34 char(100),
c35 char(100),
c36 char(100),
c37 char(100),
c38 char(100),
c39 char(100),
c40 char(100),
c41 char(100),
c42 char(100),
c43 char(100),
c44 char(100),
c45 char(100),
c46 char(100),
c47 char(100),
c48 char(100),
c49 char(100),
c50 char(100),
c51 char(100),
c52 char(100),
c53 char(100),
c54 char(100),
c55 char(100),
c56 char(100),
c57 char(100),
c58 char(100),
c59 char(100),
c60 char(100),
c61 char(100),
c62 char(100),
c63 char(100),
c64 char(100),
c65 char(100),
c66 char(100),
c67 char(100),
c68 char(100),
c69 char(100),
c70 char(100),
c71 char(100),
c72 char(100),
c73 char(100),
c74 char(100),
c75 char(100),
c76 char(100),
c77 char(100),
c78 char(100),
c79 char(100),
c80 char(100),
c81 char(150),
c82 char(150)
) engine=innodb, default character set=utf8mb4;
insert into t1 values(
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('册', 150),
repeat('侧', 150)
);
alter table t1 modify column c81 varchar(160), modify column c82 varchar(160),algorithm=instant;
checksum table t1;
Table	Checksum
test.t1	3150160946
begin;
update t1 set c81=repeat('策',150);
rollback;
checksum table t1;
Table	Checksum
test.t1	3150160946
drop table t1;
create table t1(
c1 char(100),
c2 char(100),
c3 char(100),
c4 char(100),
c5 char(100),
c6 char(100),
c7 char(100),
c8 char(100),
c9 char(100),
c10 char(100),
c11 char(100),
c12 char(100),
c13 char(100),
c14 char(100),
c15 char(100),
c16 char(100),
c17 char(100),
c18 char(100),
c19 char(100),
c20 char(100),
c21 char(100),
c22 char(100),
c23 char(100),
c24 char(100),
c25 char(100),
c26 char(100),
c27 char(100),
c28 char(100),
c29 char(100),
c30 char(100),
c31 char(100),
c32 char(100),
c33 char(100),
c34 char(100),
c35 char(100),
c36 char(100),
c37 char(100),
c38 char(100),
c39 char(100),
c40 char(100),
c41 char(100),
c42 char(100),
c43 char(100),
c44 char(100),
c45 char(100),
c46 char(100),
c47 char(100),
c48 char(100),
c49 char(100),
c50 char(100),
c51 char(100),
c52 char(100),
c53 char(100),
c54 char(100),
c55 char(100),
c56 char(100),
c57 char(100),
c58 char(100),
c59 char(100),
c60 char(100),
c61 char(100),
c62 char(100),
c63 char(100),
c64 char(100),
c65 char(100),
c66 char(100),
c67 char(100),
c68 char(100),
c69 char(100),
c70 char(100),
c71 char(100),
c72 char(100),
c73 char(100),
c74 char(100),
c75 char(100),
c76 char(100),
c77 char(100),
c78 char(100),
c79 char(100),
c80 char(100),
c81 char(130)
) engine=innodb, default character set=utf8mb4;
insert into t1 values(
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('册', 10)
);
alter table t1 modify column c81 char(200),algorithm=instant;
checksum table t1;
Table	Checksum
test.t1	33768929
begin;
update t1 set c81=repeat('策',180);
rollback;
checksum table t1;
Table	Checksum
test.t1	33768929
drop table t1;
create table t1(
c1 char(100),
c2 char(100),
c3 char(100),
c4 char(100),
c5 char(100),
c6 char(100),
c7 char(100),
c8 char(100),
c9 char(100),
c10 char(100),
c11 char(100),
c12 char(100),
c13 char(100),
c14 char(100),
c15 char(100),
c16 char(100),
c17 char(100),
c18 char(100),
c19 char(100),
c20 char(100),
c21 char(100),
c22 char(100),
c23 char(100),
c24 char(100),
c25 char(100),
c26 char(100),
c27 char(100),
c28 char(100),
c29 char(100),
c30 char(100),
c31 char(100),
c32 char(100),
c33 char(100),
c34 char(100),
c35 char(100),
c36 char(100),
c37 char(100),
c38 char(100),
c39 char(100),
c40 char(100),
c41 char(100),
c42 char(100),
c43 char(100),
c44 char(100),
c45 char(100),
c46 char(100),
c47 char(100),
c48 char(100),
c49 char(100),
c50 char(100),
c51 char(100),
c52 char(100),
c53 char(100),
c54 char(100),
c55 char(100),
c56 char(100),
c57 char(100),
c58 char(100),
c59 char(100),
c60 char(100),
c61 char(100),
c62 char(100),
c63 char(100),
c64 char(100),
c65 char(100),
c66 char(100),
c67 char(100),
c68 char(100),
c69 char(100),
c70 char(100),
c71 char(100),
c72 char(100),
c73 char(100),
c74 char(100),
c75 char(100),
c76 char(100),
c77 char(100),
c78 char(100),
c79 char(100),
c80 char(100),
c81 char(10)
) engine=innodb, default character set=utf8mb4;
insert into t1 values(
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('测',100),
repeat('册', 10)
);
alter table t1 modify column c81 char(200),algorithm=instant;
checksum table t1;
Table	Checksum
test.t1	33768929
begin;
update t1 set c81=repeat('策',180);
rollback;
checksum table t1;
Table	Checksum
test.t1	33768929
drop table t1;
create table t1(id int not null primary key,c1 varchar(2000), c2 varchar(2000), c3 varchar(2000),c4 varchar(2500)) engine=innodb, default character set=latin1, row_format=compact;
insert into t1 values(1, repeat('a',2000),repeat('b', 2000),repeat('c',2000),repeat('d',2500));
alter table t1 modify column c4 varchar(3000);
checksum table t1;
Table	Checksum
test.t1	1381998132
begin;
update t1 set c4 = repeat('e', 3000);
rollback;
checksum table t1;
Table	Checksum
test.t1	1381998132
drop table t1;
create table t1(id int not null primary key,c1 varchar(2000), c2 varchar(2000), c3 char(100),c4 varchar(2000)) engine=innodb, default character set=latin1, row_format=compact;
insert into t1 values(1, repeat('a',2000),repeat('b', 2000),repeat('c',100),repeat('d',2000));
alter table t1 modify column c3 varchar(3000);
checksum table t1;
Table	Checksum
test.t1	3045417034
begin;
update t1 set c3 = repeat('e', 3000);
rollback;
checksum table t1;
Table	Checksum
test.t1	3045417034
drop table t1;

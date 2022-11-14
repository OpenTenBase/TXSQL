DROP TABLE IF EXISTS t1;
SET NAMES utf8mb4 COLLATE utf8mb4_0900_as_ci;
SET @test_character_set= 'utf8mb4';
SET @test_collation= 'utf8mb4_0900_as_ci';
SET @safe_character_set_server= @@character_set_server;
SET @safe_collation_server= @@collation_server;
SET @safe_character_set_client= @@character_set_client;
SET @safe_character_set_results= @@character_set_results;
SET character_set_server= @test_character_set;
SET collation_server= @test_collation;
CREATE DATABASE d1;
USE d1;
CREATE TABLE t1 (c CHAR(10), KEY(c));
SHOW FULL COLUMNS FROM t1;
Field	Type	Collation	Null	Key	Default	Extra	Privileges	Comment
c	char(10)	utf8mb4_0900_as_ci	YES	MUL	NULL			
INSERT INTO t1 VALUES ('aaa'),('aaaa'),('aaaaa');
SELECT c as want3results FROM t1 WHERE c LIKE 'aaa%';
want3results
aaa
aaaa
aaaaa
DROP TABLE t1;
CREATE TABLE t1 (c1 varchar(15), KEY c1 (c1(2)));
SHOW FULL COLUMNS FROM t1;
Field	Type	Collation	Null	Key	Default	Extra	Privileges	Comment
c1	varchar(15)	utf8mb4_0900_as_ci	YES	MUL	NULL			
INSERT INTO t1 VALUES ('location'),('loberge'),('lotre'),('boabab');
SELECT c1 as want3results from t1 where c1 like 'l%';
want3results
loberge
location
lotre
SELECT c1 as want3results from t1 where c1 like 'lo%';
want3results
loberge
location
lotre
SELECT c1 as want1result  from t1 where c1 like 'loc%';
want1result
location
SELECT c1 as want1result  from t1 where c1 like 'loca%';
want1result
location
SELECT c1 as want1result  from t1 where c1 like 'locat%';
want1result
location
SELECT c1 as want1result  from t1 where c1 like 'locati%';
want1result
location
SELECT c1 as want1result  from t1 where c1 like 'locatio%';
want1result
location
SELECT c1 as want1result  from t1 where c1 like 'location%';
want1result
location
DROP TABLE t1;
create table t1 (a set('a') not null);
insert ignore into t1 values (),();
Warnings:
Warning	1364	Field 'a' doesn't have a default value
select cast(a as char(1)) from t1;
cast(a as char(1))


select a sounds like a from t1;
a sounds like a
1
1
select 1 from t1 order by cast(a as char(1));
1
1
1
drop table t1;
set names utf8;
Warnings:
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
create table t1 (
name varchar(10),
level smallint unsigned);
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `name` varchar(10) COLLATE utf8mb4_0900_as_ci DEFAULT NULL,
  `level` smallint unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_as_ci
insert into t1 values ('string',1);
select concat(name,space(level)), concat(name, repeat(' ',level)) from t1;
concat(name,space(level))	concat(name, repeat(' ',level))
string 	string 
drop table t1;
DROP DATABASE d1;
USE test;
SET character_set_server= @safe_character_set_server;
SET collation_server= @safe_collation_server;
SET character_set_client= @safe_character_set_client;
SET character_set_results= @safe_character_set_results;
SET NAMES utf8mb4 COLLATE utf8mb4_0900_as_ci;
create table t1 select repeat('a',4000) a;
delete from t1;
insert into t1 values ('a'), ('a '), ('a\t');
select collation(a),hex(a) from t1 order by a;
collation(a)	hex(a)
utf8mb4_0900_as_ci	61
utf8mb4_0900_as_ci	6109
utf8mb4_0900_as_ci	6120
drop table t1;
create table t1 engine=innodb select repeat('a',50) as c1;
alter table t1 add index(c1(5));
insert into t1 values ('abcdefg'),('abcde100'),('abcde110'),('abcde111');
select collation(c1) from t1 limit 1;
collation(c1)
utf8mb4_0900_as_ci
select c1 from t1 where c1 like 'abcdef%' order by c1;
c1
abcdefg
select c1 from t1 where c1 like 'abcde1%' order by c1;
c1
abcde100
abcde110
abcde111
select c1 from t1 where c1 like 'abcde11%' order by c1;
c1
abcde110
abcde111
select c1 from t1 where c1 like 'abcde111%' order by c1;
c1
abcde111
drop table t1;
select @@collation_connection;
@@collation_connection
utf8mb4_0900_as_ci
create table t1 ROW_FORMAT=DYNAMIC select repeat('a',50) as c1 ;
insert into t1 values('abcdef');
insert into t1 values('_bcdef');
insert into t1 values('a_cdef');
insert into t1 values('ab_def');
insert into t1 values('abc_ef');
insert into t1 values('abcd_f');
insert into t1 values('abcde_');
select c1 as c1u from t1 where c1 like 'ab\_def';
c1u
ab_def
select c1 as c2h from t1 where c1 like 'ab#_def' escape '#';
c2h
ab_def
drop table t1;
drop table if exists t1;
create table t1 select repeat('a',10) as c1;
delete from t1;
insert into t1 values (0x20),(0x21),(0x22),(0x23),(0x24),(0x25),(0x26),(0x27),(0x28),(0x29),(0x2A),(0x2B),(0x2C),(0x2D),(0x2E),(0x2F);
insert into t1 values (0x30),(0x31),(0x32),(0x33),(0x34),(0x35),(0x36),(0x37),(0x38),(0x39),(0x3A),(0x3B),(0x3C),(0x3D),(0x3E),(0x3F);
insert into t1 values (0x40),(0x41),(0x42),(0x43),(0x44),(0x45),(0x46),(0x47),(0x48),(0x49),(0x4A),(0x4B),(0x4C),(0x4D),(0x4E),(0x4F);
insert into t1 values (0x50),(0x51),(0x52),(0x53),(0x54),(0x55),(0x56),(0x57),(0x58),(0x59),(0x5A),(0x5B),(0x5C),(0x5D),(0x5E),(0x5F);
insert into t1 values (0x60),(0x61),(0x62),(0x63),(0x64),(0x65),(0x66),(0x67),(0x68),(0x69),(0x6A),(0x6B),(0x6C),(0x6D),(0x6E),(0x6F);
insert into t1 values (0x70),(0x71),(0x72),(0x73),(0x74),(0x75),(0x76),(0x77),(0x78),(0x79),(0x7A),(0x7B),(0x7C),(0x7D),(0x7E),(0x7F);
SELECT HEX(cx), cy
FROM (SELECT GROUP_CONCAT(c1 ORDER BY binary c1 SEPARATOR '') AS cx,
GROUP_CONCAT(HEX(c1) ORDER BY BINARY c1) AS cy
FROM t1
GROUP BY c1
) AS dt;
HEX(cx)	cy
7F	7F
20	20
5F	5F
2D	2D
2C	2C
3B	3B
3A	3A
21	21
3F	3F
2E	2E
27	27
22	22
28	28
29	29
5B	5B
5D	5D
7B	7B
7D	7D
40	40
2A	2A
2F	2F
5C	5C
26	26
23	23
25	25
60	60
5E	5E
2B	2B
3C	3C
3D	3D
3E	3E
7C	7C
7E	7E
24	24
30	30
31	31
32	32
33	33
34	34
35	35
36	36
37	37
38	38
39	39
4161	41,61
4262	42,62
4363	43,63
4464	44,64
4565	45,65
4666	46,66
4767	47,67
4868	48,68
4969	49,69
4A6A	4A,6A
4B6B	4B,6B
4C6C	4C,6C
4D6D	4D,6D
4E6E	4E,6E
4F6F	4F,6F
5070	50,70
5171	51,71
5272	52,72
5373	53,73
5474	54,74
5575	55,75
5676	56,76
5777	57,77
5878	58,78
5979	59,79
5A7A	5A,7A
Warnings:
Warning	1287	'BINARY expr' is deprecated and will be removed in a future release. Please use CAST instead
Warning	1287	'BINARY expr' is deprecated and will be removed in a future release. Please use CAST instead
drop table t1;
SELECT HEX(WEIGHT_STRING('aA'));
HEX(WEIGHT_STRING('aA'))
1C471C47000000200020
SELECT HEX(WEIGHT_STRING(CAST(_utf32 x'337F' AS CHAR)));
HEX(WEIGHT_STRING(CAST(_utf32 x'337F' AS CHAR)))
FB40E82AFB40DF0FFB40CF1AFB40F93E00000020002000200020
SELECT HEX(WEIGHT_STRING(CAST(_utf32 x'FDFA' AS CHAR)));
HEX(WEIGHT_STRING(CAST(_utf32 x'FDFA' AS CHAR)))
2364239C23C50209230B239C239C23B1000000200020002000200020002000200020
select @@collation_connection;
@@collation_connection
utf8mb4_0900_as_ci
select hex(weight_string('a'));
hex(weight_string('a'))
1C4700000020
select hex(weight_string('A'));
hex(weight_string('A'))
1C4700000020
select hex(weight_string('abc'));
hex(weight_string('abc'))
1C471C601C7A0000002000200020
select hex(weight_string('abc' as char(2)));
hex(weight_string('abc' as char(2)))
1C471C60000000200020
select hex(weight_string('abc' as char(3)));
hex(weight_string('abc' as char(3)))
1C471C601C7A0000002000200020
select hex(weight_string('abc' as char(5)));
hex(weight_string('abc' as char(5)))
1C471C601C7A0000002000200020
select hex(weight_string('abc', 1, 2, 0xC0));
hex(weight_string('abc', 1, 2, 0xC0))
1C
select hex(weight_string('abc', 2, 2, 0xC0));
hex(weight_string('abc', 2, 2, 0xC0))
1C47
select hex(weight_string('abc', 3, 2, 0xC0));
hex(weight_string('abc', 3, 2, 0xC0))
1C471C
select hex(weight_string('abc', 4, 2, 0xC0));
hex(weight_string('abc', 4, 2, 0xC0))
1C471C60
select hex(weight_string('abc', 5, 2, 0xC0));
hex(weight_string('abc', 5, 2, 0xC0))
1C471C6000
select hex(weight_string('abc',25, 2, 0xC0));
hex(weight_string('abc',25, 2, 0xC0))
1C471C60000000200020000000000000000000000000000000
select hex(weight_string('abc', 1, 3, 0xC0));
hex(weight_string('abc', 1, 3, 0xC0))
1C
select hex(weight_string('abc', 2, 3, 0xC0));
hex(weight_string('abc', 2, 3, 0xC0))
1C47
select hex(weight_string('abc', 3, 3, 0xC0));
hex(weight_string('abc', 3, 3, 0xC0))
1C471C
select hex(weight_string('abc', 4, 3, 0xC0));
hex(weight_string('abc', 4, 3, 0xC0))
1C471C60
select hex(weight_string('abc', 5, 3, 0xC0));
hex(weight_string('abc', 5, 3, 0xC0))
1C471C601C
select hex(weight_string('abc',25, 3, 0xC0));
hex(weight_string('abc',25, 3, 0xC0))
1C471C601C7A00000020002000200000000000000000000000
select hex(weight_string('abc', 1, 4, 0xC0));
hex(weight_string('abc', 1, 4, 0xC0))
1C
select hex(weight_string('abc', 2, 4, 0xC0));
hex(weight_string('abc', 2, 4, 0xC0))
1C47
select hex(weight_string('abc', 3, 4, 0xC0));
hex(weight_string('abc', 3, 4, 0xC0))
1C471C
select hex(weight_string('abc', 4, 4, 0xC0));
hex(weight_string('abc', 4, 4, 0xC0))
1C471C60
select hex(weight_string('abc', 5, 4, 0xC0));
hex(weight_string('abc', 5, 4, 0xC0))
1C471C601C
select hex(weight_string('abc',25, 4, 0xC0));
hex(weight_string('abc',25, 4, 0xC0))
1C471C601C7A00000020002000200000000000000000000000
select @@collation_connection;
@@collation_connection
utf8mb4_0900_as_ci
select hex(weight_string(cast(_latin1 0x80 as char)));
hex(weight_string(cast(_latin1 0x80 as char)))
1C2A00000020
select hex(weight_string(cast(_latin1 0x808080 as char)));
hex(weight_string(cast(_latin1 0x808080 as char)))
1C2A1C2A1C2A0000002000200020
select hex(weight_string(cast(_latin1 0x808080 as char) as char(2)));
hex(weight_string(cast(_latin1 0x808080 as char) as char(2)))
1C2A1C2A000000200020
select hex(weight_string(cast(_latin1 0x808080 as char) as char(3)));
hex(weight_string(cast(_latin1 0x808080 as char) as char(3)))
1C2A1C2A1C2A0000002000200020
select hex(weight_string(cast(_latin1 0x808080 as char) as char(5)));
hex(weight_string(cast(_latin1 0x808080 as char) as char(5)))
1C2A1C2A1C2A0000002000200020
select hex(weight_string(cast(_latin1 0x808080 as char), 1, 2, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 1, 2, 0xC0))
1C
select hex(weight_string(cast(_latin1 0x808080 as char), 2, 2, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 2, 2, 0xC0))
1C2A
select hex(weight_string(cast(_latin1 0x808080 as char), 3, 2, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 3, 2, 0xC0))
1C2A1C
select hex(weight_string(cast(_latin1 0x808080 as char), 4, 2, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 4, 2, 0xC0))
1C2A1C2A
select hex(weight_string(cast(_latin1 0x808080 as char), 5, 2, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 5, 2, 0xC0))
1C2A1C2A00
select hex(weight_string(cast(_latin1 0x808080 as char),25, 2, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char),25, 2, 0xC0))
1C2A1C2A000000200020000000000000000000000000000000
select hex(weight_string(cast(_latin1 0x808080 as char), 1, 3, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 1, 3, 0xC0))
1C
select hex(weight_string(cast(_latin1 0x808080 as char), 2, 3, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 2, 3, 0xC0))
1C2A
select hex(weight_string(cast(_latin1 0x808080 as char), 3, 3, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 3, 3, 0xC0))
1C2A1C
select hex(weight_string(cast(_latin1 0x808080 as char), 4, 3, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 4, 3, 0xC0))
1C2A1C2A
select hex(weight_string(cast(_latin1 0x808080 as char), 5, 3, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 5, 3, 0xC0))
1C2A1C2A1C
select hex(weight_string(cast(_latin1 0x808080 as char),25, 3, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char),25, 3, 0xC0))
1C2A1C2A1C2A00000020002000200000000000000000000000
select hex(weight_string(cast(_latin1 0x808080 as char), 1, 4, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 1, 4, 0xC0))
1C
select hex(weight_string(cast(_latin1 0x808080 as char), 2, 4, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 2, 4, 0xC0))
1C2A
select hex(weight_string(cast(_latin1 0x808080 as char), 3, 4, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 3, 4, 0xC0))
1C2A1C
select hex(weight_string(cast(_latin1 0x808080 as char), 4, 4, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 4, 4, 0xC0))
1C2A1C2A
select hex(weight_string(cast(_latin1 0x808080 as char), 5, 4, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char), 5, 4, 0xC0))
1C2A1C2A1C
select hex(weight_string(cast(_latin1 0x808080 as char),25, 4, 0xC0));
hex(weight_string(cast(_latin1 0x808080 as char),25, 4, 0xC0))
1C2A1C2A1C2A00000020002000200000000000000000000000
CREATE TABLE t1 AS SELECT repeat('a', 10) as c LIMIT 0;
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `c` varchar(10) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
INSERT INTO t1 VALUES (_utf32 0x0180),(_utf32 0x023A);
INSERT INTO t1 VALUES (_utf32 0x023B),(_utf32 0x023C);
INSERT INTO t1 VALUES (_utf32 0x023D),(_utf32 0x023E);
INSERT INTO t1 VALUES (_utf32 0x0241),(_utf32 0x0242);
INSERT INTO t1 VALUES (_utf32 0x0243),(_utf32 0x0244);
INSERT INTO t1 VALUES (_utf32 0x0245),(_utf32 0x0246);
INSERT INTO t1 VALUES (_utf32 0x0247),(_utf32 0x0248);
INSERT INTO t1 VALUES (_utf32 0x0249),(_utf32 0x024A);
INSERT INTO t1 VALUES (_utf32 0x024B),(_utf32 0x024C);
INSERT INTO t1 VALUES (_utf32 0x024D),(_utf32 0x024E);
INSERT INTO t1 VALUES (_utf32 0x024F),(_utf32 0x026B);
INSERT INTO t1 VALUES (_utf32 0x027D),(_utf32 0x0289);
INSERT INTO t1 VALUES (_utf32 0x028C);
INSERT INTO t1 VALUES (_utf32 0x037B), (_utf32 0x037C);
INSERT INTO t1 VALUES (_utf32 0x037D), (_utf32 0x03FD);
INSERT INTO t1 VALUES (_utf32 0x03FE), (_utf32 0x03FF);
INSERT INTO t1 VALUES (_utf32 0x04C0), (_utf32 0x04CF);
INSERT INTO t1 VALUES (_utf32 0x04F6), (_utf32 0x04F7);
INSERT INTO t1 VALUES (_utf32 0x04FA), (_utf32 0x04FB);
INSERT INTO t1 VALUES (_utf32 0x04FC), (_utf32 0x04FD);
INSERT INTO t1 VALUES (_utf32 0x04FE), (_utf32 0x04FF);
INSERT INTO t1 VALUES (_utf32 0x0510), (_utf32 0x0511);
INSERT INTO t1 VALUES (_utf32 0x0512), (_utf32 0x0513);
INSERT INTO t1 VALUES (_utf32 0x10A0), (_utf32 0x10A1);
INSERT INTO t1 VALUES (_utf32 0x10A2), (_utf32 0x10A3);
INSERT INTO t1 VALUES (_utf32 0x10A4), (_utf32 0x10A5);
INSERT INTO t1 VALUES (_utf32 0x10A6), (_utf32 0x10A7);
INSERT INTO t1 VALUES (_utf32 0x2D00), (_utf32 0x2D01);
INSERT INTO t1 VALUES (_utf32 0x2D02), (_utf32 0x2D03);
INSERT INTO t1 VALUES (_utf32 0x2D04), (_utf32 0x2D05);
INSERT INTO t1 VALUES (_utf32 0x2D06), (_utf32 0x2D07);
INSERT INTO t1 VALUES (_utf32 0x1D7D);
INSERT INTO t1 VALUES (_utf32 0x2132),(_utf32 0x214E);
INSERT INTO t1 VALUES (_utf32 0x2183),(_utf32 0x2184);
INSERT INTO t1 VALUES (_utf32 0x2C80), (_utf32 0x2C81);
INSERT INTO t1 VALUES (_utf32 0x2C82), (_utf32 0x2C83);
INSERT INTO t1 VALUES (_utf32 0x2C84), (_utf32 0x2C85);
INSERT INTO t1 VALUES (_utf32 0x2C86), (_utf32 0x2C87);
INSERT INTO t1 VALUES (_utf32 0x2C88), (_utf32 0x2C89);
INSERT INTO t1 VALUES (_utf32 0x2C8A), (_utf32 0x2C8B);
INSERT INTO t1 VALUES (_utf32 0x2C8C), (_utf32 0x2C8D);
INSERT INTO t1 VALUES (_utf32 0x2C8E), (_utf32 0x2C8F);
INSERT INTO t1 VALUES (_utf32 0x2C60), (_utf32 0x2C61);
INSERT INTO t1 VALUES (_utf32 0x2C62), (_utf32 0x2C63);
INSERT INTO t1 VALUES (_utf32 0x2C64), (_utf32 0x2C65);
INSERT INTO t1 VALUES (_utf32 0x2C66), (_utf32 0x2C67);
INSERT INTO t1 VALUES (_utf32 0x2C68), (_utf32 0x2C69);
INSERT INTO t1 VALUES (_utf32 0x2C6A), (_utf32 0x2C6B);
INSERT INTO t1 VALUES (_utf32 0x2C6C), (_utf32 0x2C75);
INSERT INTO t1 VALUES (_utf32 0x2C76);
INSERT INTO t1 VALUES (_utf32 0x2C00), (_utf32 0x2C01);
INSERT INTO t1 VALUES (_utf32 0x2C02), (_utf32 0x2C03);
INSERT INTO t1 VALUES (_utf32 0x2C04), (_utf32 0x2C05);
INSERT INTO t1 VALUES (_utf32 0x2C06), (_utf32 0x2C07);
INSERT INTO t1 VALUES (_utf32 0x2C30), (_utf32 0x2C31);
INSERT INTO t1 VALUES (_utf32 0x2C32), (_utf32 0x2C33);
INSERT INTO t1 VALUES (_utf32 0x2C34), (_utf32 0x2C35);
INSERT INTO t1 VALUES (_utf32 0x2C36), (_utf32 0x2C37);
INSERT INTO t1 VALUES (_utf32 0x10400), (_utf32 0x10401);
INSERT INTO t1 VALUES (_utf32 0x10402), (_utf32 0x10403);
INSERT INTO t1 VALUES (_utf32 0x10404), (_utf32 0x10405);
INSERT INTO t1 VALUES (_utf32 0x10406), (_utf32 0x10407);
INSERT INTO t1 VALUES (_utf32 0x10428), (_utf32 0x10429);
INSERT INTO t1 VALUES (_utf32 0x1042A), (_utf32 0x1042B);
INSERT INTO t1 VALUES (_utf32 0x1042C), (_utf32 0x1042D);
INSERT INTO t1 VALUES (_utf32 0x1042E), (_utf32 0x1042F);
INSERT INTO t1 VALUES (_utf32 0x0370);
INSERT INTO t1 VALUES (_utf32 0x0371);
INSERT INTO t1 VALUES (_utf32 0x0372);
INSERT INTO t1 VALUES (_utf32 0x0373);
INSERT INTO t1 VALUES (_utf32 0x0514);
INSERT INTO t1 VALUES (_utf32 0x0515);
INSERT INTO t1 VALUES (_utf32 0x0516);
INSERT INTO t1 VALUES (_utf32 0x0517);
INSERT INTO t1 VALUES (_utf32 0xA640);
INSERT INTO t1 VALUES (_utf32 0xA641);
INSERT INTO t1 VALUES (_utf32 0xA642);
INSERT INTO t1 VALUES (_utf32 0xA643);
INSERT INTO t1 VALUES (_utf32 0xA722);
INSERT INTO t1 VALUES (_utf32 0xA723);
INSERT INTO t1 VALUES (_utf32 0xA724);
INSERT INTO t1 VALUES (_utf32 0xA725);
INSERT INTO t1 VALUES (_utf32 0xA726);
INSERT INTO t1 VALUES (_utf32 0xA727);
INSERT INTO t1 VALUES (_utf32 0xA728);
INSERT INTO t1 VALUES (_utf32 0xA729);
INSERT INTO t1 VALUES (_utf32 0xA72A);
INSERT INTO t1 VALUES (_utf32 0xA72B);
INSERT INTO t1 VALUES (_utf32 0x2CEB);
INSERT INTO t1 VALUES (_utf32 0x2CEC);
INSERT INTO t1 VALUES (_utf32 0x2CED);
INSERT INTO t1 VALUES (_utf32 0x2CEE);
INSERT INTO t1 VALUES (_utf32 0x1FA01);
INSERT INTO t1 VALUES (_utf32 0x1FB01);
INSERT INTO t1 VALUES (_utf32 0x1FC01);
INSERT INTO t1 VALUES (_utf32 0x1FD01);
INSERT INTO t1 VALUES (_utf32 0x1F603);
INSERT INTO t1 VALUES (_utf32 0x1F604);
INSERT INTO t1 VALUES (_utf32 0x1F648);
INSERT INTO t1 VALUES (_utf32 0x1F64F);
INSERT INTO t1 VALUES (_utf32 0x2B759);
INSERT INTO t1 VALUES (_utf32 0x2B760);
INSERT INTO t1 VALUES (_utf32 0x2B761);
INSERT INTO t1 VALUES (_utf32 0x2B762);
INSERT INTO t1 VALUES (_utf32 0x16F00);
INSERT INTO t1 VALUES (_utf32 0x16F01);
INSERT INTO t1 VALUES (_utf32 0x08A2);
INSERT INTO t1 VALUES (_utf32 0x08A3);
INSERT INTO t1 VALUES (_utf32 0xA794);
INSERT INTO t1 VALUES (_utf32 0xA795);
INSERT INTO t1 VALUES (_utf32 0xA796);
INSERT INTO t1 VALUES (_utf32 0xA797);
INSERT INTO t1 VALUES (_utf32 0xAB37);
INSERT INTO t1 VALUES (_utf32 0xAB38);
INSERT INTO t1 VALUES (_utf32 0x10600);
INSERT INTO t1 VALUES (_utf32 0x10601);
INSERT INTO t1 VALUES (_utf32 0x10602);
INSERT INTO t1 VALUES (_utf32 0x10603);
INSERT INTO t1 VALUES (_utf32 0x1F800);
INSERT INTO t1 VALUES (_utf32 0x1F801);
INSERT INTO t1 VALUES (_utf32 0x1F802);
INSERT INTO t1 VALUES (_utf32 0x1F803);
INSERT INTO t1 VALUES (_utf32 0x10C92);
INSERT INTO t1 VALUES (_utf32 0x10C93);
INSERT INTO t1 VALUES (_utf32 0x10C94);
INSERT INTO t1 VALUES (_utf32 0x10C95);
INSERT INTO t1 VALUES (_utf32 0x2B836);
INSERT INTO t1 VALUES (_utf32 0x2B837);
INSERT INTO t1 VALUES (_utf32 0x2B838);
INSERT INTO t1 VALUES (_utf32 0x2B839);
SELECT hex(c), hex(lower(c)), hex(upper(c)), hex(weight_string(c)), c
FROM t1 ORDER BY c, BINARY c;
hex(c)	hex(lower(c))	hex(upper(c))	hex(weight_string(c))	c
F09F9883	F09F9883	F09F9883	15FE00000020	😃
F09F9884	F09F9884	F09F9884	15FF00000020	😄
F09F9988	F09F9988	F09F9988	164300000020	🙈
F09F998F	F09F998F	F09F998F	164A00000020	🙏
F09FA080	F09FA080	F09FA080	17AB00000020	🠀
F09FA081	F09FA081	F09FA081	17AC00000020	🠁
F09FA082	F09FA082	F09FA082	17AD00000020	🠂
F09FA083	F09FA083	F09FA083	17AE00000020	🠃
C8BA		C8BA	1C4C00000020	Ⱥ
E2B1A5	E2B1A5	C8BA	1C4C00000020	ⱥ
C680	C680	C983	1C6800000020	ƀ
C983	C680	C983	1C6800000020	Ƀ
EA9E96	EA9E97	EA9E96	1C6F00000020	Ꞗ
EA9E97	EA9E97	EA9E96	1C6F00000020	ꞗ
C8BB	C8BC	C8BB	1C7F00000020	Ȼ
C8BC	C8BC	C8BB	1C7F00000020	ȼ
EA9E94	EA9E94	EA9E94	1C8400000020	ꞔ
E28683	E28684	E28683	1C8D00000020	Ↄ
E28684	E28684	E28683	1C8D00000020	ↄ
C986	C987	C986	1CB100000020	Ɇ
C987	C987	C986	1CB100000020	ɇ
E284B2	E2858E	E284B2	1CF200000020	Ⅎ
E2858E	E2858E	E284B2	1CF200000020	ⅎ
EA9E95	EA9E95	EA9E95	1D2400000020	ꞕ
E2B1A7	E2B1A8	E2B1A7	1D2900000020	Ⱨ
E2B1A8	E2B1A8	E2B1A7	1D2900000020	ⱨ
E2B1B5	E2B1B6	E2B1B5	1D2A00000020	Ⱶ
E2B1B6	E2B1B6	E2B1B5	1D2A00000020	ⱶ
EA9CA6	EA9CA7	EA9CA6	1D2B00000020	Ꜧ
EA9CA7	EA9CA7	EA9CA6	1D2B00000020	ꜧ
C988	C989	C988	1D5500000020	Ɉ
C989	C989	C988	1D5500000020	ɉ
E2B1A9	E2B1AA	E2B1A9	1D6F00000020	Ⱪ
E2B1AA	E2B1AA	E2B1A9	1D6F00000020	ⱪ
C8BD	C69A	C8BD	1D8200000020	Ƚ
E2B1A0	E2B1A1	E2B1A0	1D8600000020	Ⱡ
E2B1A1	E2B1A1	E2B1A0	1D8600000020	ⱡ
C9AB	C9AB		1D8700000020	ɫ
E2B1A2	C9AB	E2B1A2	1D8700000020	Ɫ
EAACB8	EAACB8	EAACB8	1D8B00000020	ꬸ
EAACB7	EAACB7	EAACB7	1D9100000020	ꬷ
E1B5BD	E1B5BD	E2B1A3	1E1100000020	ᵽ
E2B1A3	E1B5BD	E2B1A3	1E1100000020	Ᵽ
C98A	C98B	C98A	1E2B00000020	Ɋ
C98B	C98B	C98A	1E2B00000020	ɋ
C98C	C98D	C98C	1E3F00000020	Ɍ
C98D	C98D	C98C	1E3F00000020	ɍ
C9BD	C9BD		1E5700000020	ɽ
E2B1A4	C9BD	E2B1A4	1E5700000020	Ɽ
EA9CA8	EA9CA9	EA9CA8	1E951F21000000200020	Ꜩ
EA9CA9	EA9CA9	EA9CA8	1E951F21000000200020	ꜩ
C8BE		C8BE	1E9E00000020	Ⱦ
E2B1A6	E2B1A6	C8BE	1E9E00000020	ⱦ
C984	CA89	C984	1EC000000020	Ʉ
CA89	CA89	C984	1EC000000020	ʉ
C985	CA8C	C985	1EF100000020	Ʌ
CA8C	CA8C	C985	1EF100000020	ʌ
C98E	C98F	C98E	1F1300000020	Ɏ
C98F	C98F	C98E	1F1300000020	ɏ
E2B1AB	E2B1AC	E2B1AB	1F3C00000020	Ⱬ
E2B1AC	E2B1AC	E2B1AB	1F3C00000020	ⱬ
EA9CAA	EA9CAB	EA9CAA	1F6600000020	Ꜫ
EA9CAB	EA9CAB	EA9CAA	1F6600000020	ꜫ
C981	C982	C981	1F7900000020	Ɂ
C982	C982	C981	1F7900000020	ɂ
EA9CA2	EA9CA3	EA9CA2	1F8100000020	Ꜣ
EA9CA3	EA9CA3	EA9CA2	1F8100000020	ꜣ
EA9CA4	EA9CA5	EA9CA4	1F8C00000020	Ꜥ
EA9CA5	EA9CA5	EA9CA4	1F8C00000020	ꜥ
CDB0	CDB1	CDB0	1FC300000020	Ͱ
CDB1	CDB1	CDB0	1FC300000020	ͱ
CDBC	CDBC	CFBE	1FD800000020	ͼ
CFBE	CDBC	CFBE	1FD800000020	Ͼ
CDBB	CDBB	CFBD	1FD900000020	ͻ
CFBD	CDBB	CFBD	1FD900000020	Ͻ
CDBD	CDBD	CFBF	1FDA00000020	ͽ
CFBF	CDBD	CFBF	1FDA00000020	Ͽ
CDB2	CDB3	CDB2	1FE400000020	Ͳ
CDB3	CDB3	CDB2	1FE400000020	ͳ
E2B280	E2B281	E2B280	1FE600000020	Ⲁ
E2B281	E2B281	E2B280	1FE600000020	ⲁ
E2B282	E2B283	E2B282	1FE700000020	Ⲃ
E2B283	E2B283	E2B282	1FE700000020	ⲃ
E2B284	E2B285	E2B284	1FE800000020	Ⲅ
E2B285	E2B285	E2B284	1FE800000020	ⲅ
E2B286	E2B287	E2B286	1FE900000020	Ⲇ
E2B287	E2B287	E2B286	1FE900000020	ⲇ
E2B288	E2B289	E2B288	1FEA00000020	Ⲉ
E2B289	E2B289	E2B288	1FEA00000020	ⲉ
E2B28A	E2B28B	E2B28A	1FEC00000020	Ⲋ
E2B28B	E2B28B	E2B28A	1FEC00000020	ⲋ
E2B28C	E2B28D	E2B28C	1FED00000020	Ⲍ
E2B28D	E2B28D	E2B28C	1FED00000020	ⲍ
E2B28E	E2B28F	E2B28E	1FEE00000020	Ⲏ
E2B28F	E2B28F	E2B28E	1FEE00000020	ⲏ
E2B3AB	E2B3AC	E2B3AB	200600000020	Ⳬ
E2B3AC	E2B3AC	E2B3AB	200600000020	ⳬ
E2B3AD	E2B3AE	E2B3AD	201600000020	Ⳮ
E2B3AE	E2B3AE	E2B3AD	201600000020	ⳮ
D3BA	D3BB	D3BA	203E00000020	Ӻ
D3BB	D3BB	D3BA	203E00000020	ӻ
D3B6	D3B7	D3B6	204600000020	Ӷ
D3B7	D3B7	D3B6	204600000020	ӷ
EA9980	EA9981	EA9980	207000000020	Ꙁ
EA9981	EA9981	EA9980	207000000020	ꙁ
D490	D491	D490	207200000020	Ԑ
D491	D491	D490	207200000020	ԑ
EA9982	EA9983	EA9982	207300000020	Ꙃ
EA9983	EA9983	EA9982	207300000020	ꙃ
D492	D493	D492	20BA00000020	Ԓ
D493	D493	D492	20BA00000020	ԓ
D494	D495	D494	20C200000020	Ԕ
D495	D495	D494	20C200000020	ԕ
D496	D497	D496	210400000020	Ԗ
D497	D497	D496	210400000020	ԗ
D3BC	D3BD	D3BC	213600000020	Ӽ
D3BD	D3BD	D3BC	213600000020	ӽ
D3BE	D3BF	D3BE	213A00000020	Ӿ
D3BF	D3BF	D3BE	213A00000020	ӿ
D380	D38F	D380	21E100000020	Ӏ
D38F	D38F	D380	21E100000020	ӏ
E2B080	E2B0B0	E2B080	21E500000020	Ⰰ
E2B0B0	E2B0B0	E2B080	21E500000020	ⰰ
E2B081	E2B0B1	E2B081	21E600000020	Ⰱ
E2B0B1	E2B0B1	E2B081	21E600000020	ⰱ
E2B082	E2B0B2	E2B082	21E700000020	Ⰲ
E2B0B2	E2B0B2	E2B082	21E700000020	ⰲ
E2B083	E2B0B3	E2B083	21E800000020	Ⰳ
E2B0B3	E2B0B3	E2B083	21E800000020	ⰳ
E2B084	E2B0B4	E2B084	21E900000020	Ⰴ
E2B0B4	E2B0B4	E2B084	21E900000020	ⰴ
E2B085	E2B0B5	E2B085	21EA00000020	Ⰵ
E2B0B5	E2B0B5	E2B085	21EA00000020	ⰵ
E2B086	E2B0B6	E2B086	21EB00000020	Ⰶ
E2B0B6	E2B0B6	E2B086	21EB00000020	ⰶ
E2B087	E2B0B7	E2B087	21EC00000020	Ⰷ
E2B0B7	E2B0B7	E2B087	21EC00000020	ⰷ
E182A0	E2B480	E182A0	223B00000020	Ⴀ
E2B480	E2B480	E182A0	223B00000020	ⴀ
E182A1	E2B481	E182A1	223D00000020	Ⴁ
E2B481	E2B481	E182A1	223D00000020	ⴁ
E182A2	E2B482	E182A2	223F00000020	Ⴂ
E2B482	E2B482	E182A2	223F00000020	ⴂ
E182A3	E2B483	E182A3	224100000020	Ⴃ
E2B483	E2B483	E182A3	224100000020	ⴃ
E182A4	E2B484	E182A4	224300000020	Ⴄ
E2B484	E2B484	E182A4	224300000020	ⴄ
E182A5	E2B485	E182A5	224500000020	Ⴅ
E2B485	E2B485	E182A5	224500000020	ⴅ
E182A6	E2B486	E182A6	224700000020	Ⴆ
E2B486	E2B486	E182A6	224700000020	ⴆ
E182A7	E2B487	E182A7	224B00000020	Ⴇ
E2B487	E2B487	E182A7	224B00000020	ⴇ
E0A2A2	E0A2A2	E0A2A2	232B00000020	ࢢ
E0A2A3	E0A2A3	E0A2A3	236D00000020	ࢣ
F090B292	F090B392	F090B292	371200000020	𐲒
F090B293	F090B393	F090B293	371300000020	𐲓
F090B294	F090B394	F090B294	371400000020	𐲔
F090B295	F090B395	F090B295	371500000020	𐲕
F096BC80	F096BC80	F096BC80	427A00000020	𖼀
F096BC81	F096BC81	F096BC81	427B00000020	𖼁
F0909080	F09090A8	F0909080	445200000020	𐐀
F09090A8	F09090A8	F0909080	445200000020	𐐨
F0909081	F09090A9	F0909081	445300000020	𐐁
F09090A9	F09090A9	F0909081	445300000020	𐐩
F0909082	F09090AA	F0909082	445400000020	𐐂
F09090AA	F09090AA	F0909082	445400000020	𐐪
F0909083	F09090AB	F0909083	445500000020	𐐃
F09090AB	F09090AB	F0909083	445500000020	𐐫
F0909084	F09090AC	F0909084	445600000020	𐐄
F09090AC	F09090AC	F0909084	445600000020	𐐬
F0909085	F09090AD	F0909085	445700000020	𐐅
F09090AD	F09090AD	F0909085	445700000020	𐐭
F0909086	F09090AE	F0909086	445800000020	𐐆
F09090AE	F09090AE	F0909086	445800000020	𐐮
F0909087	F09090AF	F0909087	445900000020	𐐇
F09090AF	F09090AF	F0909087	445900000020	𐐯
F0909880	F0909880	F0909880	46BA00000020	𐘀
F0909881	F0909881	F0909881	46BB00000020	𐘁
F0909882	F0909882	F0909882	46BC00000020	𐘂
F0909883	F0909883	F0909883	46BD00000020	𐘃
F0AB9D99	F0AB9D99	F0AB9D99	FB85B75900000020	𫝙
F0AB9DA0	F0AB9DA0	F0AB9DA0	FB85B76000000020	𫝠
F0AB9DA1	F0AB9DA1	F0AB9DA1	FB85B76100000020	𫝡
F0AB9DA2	F0AB9DA2	F0AB9DA2	FB85B76200000020	𫝢
F0ABA0B6	F0ABA0B6	F0ABA0B6	FB85B83600000020	𫠶
F0ABA0B7	F0ABA0B7	F0ABA0B7	FB85B83700000020	𫠷
F0ABA0B8	F0ABA0B8	F0ABA0B8	FB85B83800000020	𫠸
F0ABA0B9	F0ABA0B9	F0ABA0B9	FB85B83900000020	𫠹
F09FA881	F09FA881	F09FA881	FBC3FA0100000020	🨁
F09FAC81	F09FAC81	F09FAC81	FBC3FB0100000020	🬁
F09FB081	F09FB081	F09FB081	FBC3FC0100000020	🰁
F09FB481	F09FB481	F09FB481	FBC3FD0100000020	🴁
Warnings:
Warning	1287	'BINARY expr' is deprecated and will be removed in a future release. Please use CAST instead
INSERT INTO t1 VALUES ('a');
INSERT INTO t1 VALUES (concat(_utf32 0x61, _utf32 0xFFFF));
INSERT INTO t1 VALUES (concat(_utf32 0x61, _utf32 0x10FFFF));
INSERT INTO t1 VALUES (concat(_utf32 0x61, _utf32 0x10400));
SELECT hex(c), hex(weight_string(c)) FROM t1 WHERE c LIKE 'a%' ORDER BY c;
hex(c)	hex(weight_string(c))
61	1C4700000020
61F0909080	1C474452000000200020
61EFBFBF	1C47FBC1FFFF000000200020
61F48FBFBF	1C47FBE1FFFF000000200020
SELECT hex(c), hex(weight_string(c)), c FROM t1 WHERE c LIKE _utf32 0x10400 ORDER BY c, BINARY c;
hex(c)	hex(weight_string(c))	c
F0909080	445200000020	𐐀
F09090A8	445200000020	𐐨
Warnings:
Warning	1287	'BINARY expr' is deprecated and will be removed in a future release. Please use CAST instead
SELECT hex(c), hex(weight_string(c)), c FROM t1 WHERE c LIKE _utf32 0x10428 ORDER BY c, BINARY c;
hex(c)	hex(weight_string(c))	c
F0909080	445200000020	𐐀
F09090A8	445200000020	𐐨
Warnings:
Warning	1287	'BINARY expr' is deprecated and will be removed in a future release. Please use CAST instead
ALTER TABLE t1 ADD KEY(c);
EXPLAIN SELECT hex(c) FROM t1 WHERE c LIKE 'a%' ORDER BY c;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	range	c	c	43	NULL	4	100.00	Parallel scan (4 workers); Using where; Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>); Using merge sort
Warnings:
Note	1003	/* select#1 */ select hex(`test`.`t1`.`c`) AS `hex(c)` from `test`.`t1` where (`test`.`t1`.`c` like 'a%') order by `test`.`t1`.`c`
SELECT hex(c), hex(weight_string(c)) FROM t1 WHERE c LIKE 'a%' ORDER BY c;
hex(c)	hex(weight_string(c))
61	1C4700000020
61F0909080	1C474452000000200020
61EFBFBF	1C47FBC1FFFF000000200020
61F48FBFBF	1C47FBE1FFFF000000200020
SELECT hex(c), hex(weight_string(c)), c FROM t1 WHERE c LIKE _utf32 0x10400 ORDER BY c, BINARY c;
hex(c)	hex(weight_string(c))	c
F0909080	445200000020	𐐀
F09090A8	445200000020	𐐨
Warnings:
Warning	1287	'BINARY expr' is deprecated and will be removed in a future release. Please use CAST instead
SELECT hex(c), hex(weight_string(c)), c FROM t1 WHERE c LIKE _utf32 0x10428 ORDER BY c, BINARY c;
hex(c)	hex(weight_string(c))	c
F0909080	445200000020	𐐀
F09090A8	445200000020	𐐨
Warnings:
Warning	1287	'BINARY expr' is deprecated and will be removed in a future release. Please use CAST instead
DROP TABLE t1;
SELECT is_ipv4(inet_ntoa('1'));
is_ipv4(inet_ntoa('1'))
1
SELECT is_ipv6(inet_ntoa('1'));
is_ipv6(inet_ntoa('1'))
0
SELECT HEX(inet6_aton(inet_ntoa('1')));
HEX(inet6_aton(inet_ntoa('1')))
00000001
SELECT inet6_ntoa(inet_ntoa('1'));
inet6_ntoa(inet_ntoa('1'))
NULL
#
# Bug#14040277 UNINITIALIZED VALUE REFERENCED IN STR_TO_IPV6
#
SELECT inet6_aton(soundex('a'));
inet6_aton(soundex('a'))
NULL
#
# Bug#19047425 UNINITIALISED VALUE IN STR_TO_IPV6
#
do is_ipv4_mapped(inet6_aton(convert(_ascii "a:" using utf8mb4)));
CREATE TABLE t1 (c VARCHAR(10) CHARACTER SET utf8mb4);
INSERT INTO t1 VALUES (_utf8mb4 0xF09090A7), (_utf8mb4 0xEA8B93), (_utf8mb4 0xC4BC), (_utf8mb4 0xC6AD), (_utf8mb4 0xF090918F), (_utf8mb4 0xEAAD8B);
SELECT HEX(ANY_VALUE(c)), COUNT(c) FROM t1 GROUP BY c COLLATE utf8mb4_0900_as_ci;
HEX(ANY_VALUE(c))	COUNT(c)
C4BC	1
C6AD	1
EA8B93	1
EAAD8B	1
F09090A7	2
DROP TABLE t1;
SET NAMES utf8mb4;
CREATE TABLE t1 (c1 CHAR(10) COLLATE utf8mb4_0900_as_ci);
insert into t1 values ('A'),('a');
insert into t1 values ('B'),('b');
insert into t1 values ('C'),('c');
insert into t1 values ('D'),('d');
insert into t1 values ('E'),('e');
insert into t1 values ('F'),('f');
insert into t1 values ('G'),('g');
insert into t1 values ('H'),('h');
insert into t1 values ('I'),('i');
insert into t1 values ('J'),('j');
insert into t1 values ('K'),('k');
insert into t1 values ('L'),('l');
insert into t1 values ('M'),('m');
insert into t1 values ('N'),('n');
insert into t1 values ('O'),('o');
insert into t1 values ('P'),('p');
insert into t1 values ('Q'),('q');
insert into t1 values ('R'),('r');
insert into t1 values ('S'),('s');
insert into t1 values ('T'),('t');
insert into t1 values ('U'),('u');
insert into t1 values ('V'),('v');
insert into t1 values ('W'),('w');
insert into t1 values ('X'),('x');
insert into t1 values ('Y'),('y');
insert into t1 values ('Z'),('z');
insert into t1 values (_ucs2 0x00e0),(_ucs2 0x00c0);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00e1),(_ucs2 0x00c1);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00e2),(_ucs2 0x00c2);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00e3),(_ucs2 0x00c3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00e4),(_ucs2 0x00c4);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00e5),(_ucs2 0x00c5);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00e6),(_ucs2 0x00c6);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00e7),(_ucs2 0x00c7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00e8),(_ucs2 0x00c8);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00e9),(_ucs2 0x00c9);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00ea),(_ucs2 0x00ca);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00eb),(_ucs2 0x00cb);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00ec),(_ucs2 0x00cc);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00ed),(_ucs2 0x00cd);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00ee),(_ucs2 0x00ce);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00ef),(_ucs2 0x00cf);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f0),(_ucs2 0x00d0);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f1),(_ucs2 0x00d1);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f2),(_ucs2 0x00d2);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f3),(_ucs2 0x00d3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f4),(_ucs2 0x00d4);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f5),(_ucs2 0x00d5);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f6),(_ucs2 0x00d6);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f7),(_ucs2 0x00d7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f8),(_ucs2 0x00d8);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00f9),(_ucs2 0x00d9);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00fa),(_ucs2 0x00da);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00fb),(_ucs2 0x00db);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00fc),(_ucs2 0x00dc);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00fd),(_ucs2 0x00dd);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00fe),(_ucs2 0x00de);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x00ff),(_ucs2 0x00df);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0100),(_ucs2 0x0101),(_ucs2 0x0102),(_ucs2 0x0103);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0104),(_ucs2 0x0105),(_ucs2 0x0106),(_ucs2 0x0107);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0108),(_ucs2 0x0109),(_ucs2 0x010a),(_ucs2 0x010b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x010c),(_ucs2 0x010d),(_ucs2 0x010e),(_ucs2 0x010f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0110),(_ucs2 0x0111),(_ucs2 0x0112),(_ucs2 0x0113);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0114),(_ucs2 0x0115),(_ucs2 0x0116),(_ucs2 0x0117);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0118),(_ucs2 0x0119),(_ucs2 0x011a),(_ucs2 0x011b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x011c),(_ucs2 0x011d),(_ucs2 0x011e),(_ucs2 0x011f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0120),(_ucs2 0x0121),(_ucs2 0x0122),(_ucs2 0x0123);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0124),(_ucs2 0x0125),(_ucs2 0x0126),(_ucs2 0x0127);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0128),(_ucs2 0x0129),(_ucs2 0x012a),(_ucs2 0x012b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x012c),(_ucs2 0x012d),(_ucs2 0x012e),(_ucs2 0x012f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0130),(_ucs2 0x0131),(_ucs2 0x0132),(_ucs2 0x0133);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0134),(_ucs2 0x0135),(_ucs2 0x0136),(_ucs2 0x0137);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0138),(_ucs2 0x0139),(_ucs2 0x013a),(_ucs2 0x013b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x013c),(_ucs2 0x013d),(_ucs2 0x013e),(_ucs2 0x013f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0140),(_ucs2 0x0141),(_ucs2 0x0142),(_ucs2 0x0143);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0144),(_ucs2 0x0145),(_ucs2 0x0146),(_ucs2 0x0147);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0148),(_ucs2 0x0149),(_ucs2 0x014a),(_ucs2 0x014b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x014c),(_ucs2 0x014d),(_ucs2 0x014e),(_ucs2 0x014f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0150),(_ucs2 0x0151),(_ucs2 0x0152),(_ucs2 0x0153);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0154),(_ucs2 0x0155),(_ucs2 0x0156),(_ucs2 0x0157);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0158),(_ucs2 0x0159),(_ucs2 0x015a),(_ucs2 0x015b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x015c),(_ucs2 0x015d),(_ucs2 0x015e),(_ucs2 0x015f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0160),(_ucs2 0x0161),(_ucs2 0x0162),(_ucs2 0x0163);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0164),(_ucs2 0x0165),(_ucs2 0x0166),(_ucs2 0x0167);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0168),(_ucs2 0x0169),(_ucs2 0x016a),(_ucs2 0x016b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x016c),(_ucs2 0x016d),(_ucs2 0x016e),(_ucs2 0x016f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0170),(_ucs2 0x0171),(_ucs2 0x0172),(_ucs2 0x0173);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0174),(_ucs2 0x0175),(_ucs2 0x0176),(_ucs2 0x0177);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0178),(_ucs2 0x0179),(_ucs2 0x017a),(_ucs2 0x017b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x017c),(_ucs2 0x017d),(_ucs2 0x017e),(_ucs2 0x017f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0180),(_ucs2 0x0181),(_ucs2 0x0182),(_ucs2 0x0183);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0184),(_ucs2 0x0185),(_ucs2 0x0186),(_ucs2 0x0187);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0188),(_ucs2 0x0189),(_ucs2 0x018a),(_ucs2 0x018b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x018c),(_ucs2 0x018d),(_ucs2 0x018e),(_ucs2 0x018f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0190),(_ucs2 0x0191),(_ucs2 0x0192),(_ucs2 0x0193);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0194),(_ucs2 0x0195),(_ucs2 0x0196),(_ucs2 0x0197);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0198),(_ucs2 0x0199),(_ucs2 0x019a),(_ucs2 0x019b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x019c),(_ucs2 0x019d),(_ucs2 0x019e),(_ucs2 0x019f);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01a0),(_ucs2 0x01a1),(_ucs2 0x01a2),(_ucs2 0x01a3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01a4),(_ucs2 0x01a5),(_ucs2 0x01a6),(_ucs2 0x01a7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01a8),(_ucs2 0x01a9),(_ucs2 0x01aa),(_ucs2 0x01ab);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01ac),(_ucs2 0x01ad),(_ucs2 0x01ae),(_ucs2 0x01af);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01b0),(_ucs2 0x01b1),(_ucs2 0x01b2),(_ucs2 0x01b3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01b4),(_ucs2 0x01b5),(_ucs2 0x01b6),(_ucs2 0x01b7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01b8),(_ucs2 0x01b9),(_ucs2 0x01ba),(_ucs2 0x01bb);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01bc),(_ucs2 0x01bd),(_ucs2 0x01be),(_ucs2 0x01bf);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01c0),(_ucs2 0x01c1),(_ucs2 0x01c2),(_ucs2 0x01c3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01c4),(_ucs2 0x01c5),(_ucs2 0x01c6),(_ucs2 0x01c7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01c8),(_ucs2 0x01c9),(_ucs2 0x01ca),(_ucs2 0x01cb);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01cc),(_ucs2 0x01cd),(_ucs2 0x01ce),(_ucs2 0x01cf);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01d0),(_ucs2 0x01d1),(_ucs2 0x01d2),(_ucs2 0x01d3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01d4),(_ucs2 0x01d5),(_ucs2 0x01d6),(_ucs2 0x01d7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01d8),(_ucs2 0x01d9),(_ucs2 0x01da),(_ucs2 0x01db);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01dc),(_ucs2 0x01dd),(_ucs2 0x01de),(_ucs2 0x01df);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01e0),(_ucs2 0x01e1),(_ucs2 0x01e2),(_ucs2 0x01e3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01e4),(_ucs2 0x01e5),(_ucs2 0x01e6),(_ucs2 0x01e7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01e8),(_ucs2 0x01e9),(_ucs2 0x01ea),(_ucs2 0x01eb);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01ec),(_ucs2 0x01ed),(_ucs2 0x01ee),(_ucs2 0x01ef);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01f0),(_ucs2 0x01f1),(_ucs2 0x01f2),(_ucs2 0x01f3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01f4),(_ucs2 0x01f5),(_ucs2 0x01f6),(_ucs2 0x01f7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01f8),(_ucs2 0x01f9),(_ucs2 0x01fa),(_ucs2 0x01fb);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x01fc),(_ucs2 0x01fd),(_ucs2 0x01fe),(_ucs2 0x01ff);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EA0),(_ucs2 0x1EA1),(_ucs2 0x1EA2),(_ucs2 0x1EA3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EA4),(_ucs2 0x1EA5),(_ucs2 0x1EA6),(_ucs2 0x1EA7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EA8),(_ucs2 0x1EA9),(_ucs2 0x1EAA),(_ucs2 0x1EAB);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EAC),(_ucs2 0x1EAD),(_ucs2 0x1EAE),(_ucs2 0x1EAF);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EB0),(_ucs2 0x1EB1),(_ucs2 0x1EB2),(_ucs2 0x1EB3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EB4),(_ucs2 0x1EB5),(_ucs2 0x1EB6),(_ucs2 0x1EB7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EB8),(_ucs2 0x1EB9),(_ucs2 0x1EBA),(_ucs2 0x1EBB);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EBC),(_ucs2 0x1EBD),(_ucs2 0x1EBE),(_ucs2 0x1EBF);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EC0),(_ucs2 0x1EC1),(_ucs2 0x1EC2),(_ucs2 0x1EC3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EC4),(_ucs2 0x1EC5),(_ucs2 0x1EC6),(_ucs2 0x1EC7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EC8),(_ucs2 0x1EC9),(_ucs2 0x1ECA),(_ucs2 0x1ECB);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1ECC),(_ucs2 0x1ECD),(_ucs2 0x1ECE),(_ucs2 0x1ECF);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1ED0),(_ucs2 0x1ED1),(_ucs2 0x1ED2),(_ucs2 0x1ED3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1ED4),(_ucs2 0x1ED5),(_ucs2 0x1ED6),(_ucs2 0x1ED7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1ED8),(_ucs2 0x1ED9),(_ucs2 0x1EDA),(_ucs2 0x1EDB);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EDC),(_ucs2 0x1EDD),(_ucs2 0x1EDE),(_ucs2 0x1EDF);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EE0),(_ucs2 0x1EE1),(_ucs2 0x1EE2),(_ucs2 0x1EE3);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EE4),(_ucs2 0x1EE5),(_ucs2 0x1EE6),(_ucs2 0x1EE7);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EE8),(_ucs2 0x1EE9),(_ucs2 0x1EEA),(_ucs2 0x1EEB);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EEC),(_ucs2 0x1EED),(_ucs2 0x1EEE),(_ucs2 0x1EEF);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
INSERT INTO t1 VALUES (_ucs2 0x1EF0),(_ucs2 0x1EF1);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values ('AA'),('Aa'),('aa'),('aA');
insert into t1 values ('AE'),('Ae'),('ae'),('aE');
insert into t1 values ('CH'),('Ch'),('ch'),('cH');
insert into t1 values ('DZ'),('Dz'),('dz'),('dZ');
insert into t1 values ('DŽ'),('Dž'),('dž'),('dŽ');
insert into t1 values ('IJ'),('Ij'),('ij'),('iJ');
insert into t1 values ('LJ'),('Lj'),('lj'),('lJ');
insert into t1 values ('LL'),('Ll'),('ll'),('lL');
insert into t1 values ('NJ'),('Nj'),('nj'),('nJ');
insert into t1 values ('OE'),('Oe'),('oe'),('oE');
insert into t1 values ('SS'),('Ss'),('ss'),('sS');
insert into t1 values ('RR'),('Rr'),('rr'),('rR');
insert into t1 values (_ucs2 0x0218), (_ucs2 0x0219), (_ucs2 0x021a), (_ucs2 0x021b);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0d96), (_ucs2 0x0da4), (_ucs2 0x0da5);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values (_ucs2 0x0064017e), (_ucs2 0x0044017e), (_ucs2 0x0044017d);
Warnings:
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
Warning	1287	'ucs2' is deprecated and will be removed in a future release. Please use utf8mb4 instead
insert into t1 values ('CS'),('Cs'),('cs'),('cS');
insert into t1 values ('DZS'),('DZs'),('Dzs'),('DzS');
insert into t1 values ('dZS'),('dZs'),('dzs'),('dzS');
insert into t1 values ('GY'),('Gy'),('gy'),('gY');
insert into t1 values ('LY'),('Ly'),('ly'),('lY');
insert into t1 values ('NY'),('Ny'),('ny'),('nY');
insert into t1 values ('SZ'),('Sz'),('sz'),('sZ');
insert into t1 values ('TY'),('Ty'),('ty'),('tY');
insert into t1 values ('ZS'),('Zs'),('zs'),('zS');
insert into t1 values ('RR'),('Rr'),('rr'),('rR');
insert into t1 values ('ccs'),('Ccs'),('CCS'),('cCS');
insert into t1 values ('ddz'),('Ddz'),('DDZ'),('dDZ');
insert into t1 values ('ddzs'),('Ddzs'),('DDZS'),('dDZS');
insert into t1 values ('ggy'),('Ggy'),('GGY'),('gGY');
insert into t1 values ('lly'),('Lly'),('LLY'),('lLY');
insert into t1 values ('nny'),('Nny'),('NNY'),('nNY');
insert into t1 values ('ssz'),('Ssz'),('SSZ'),('sSZ');
insert into t1 values ('tty'),('Tty'),('TTY'),('tTY');
insert into t1 values ('zzs'),('Zzs'),('ZZS'),('zZS');
insert into t1 values ('UE'),('Ue'),('ue'),('uE');
SELECT c1, HEX(WEIGHT_STRING(c1)) FROM t1 ORDER BY c1, HEX(c1) COLLATE utf8mb4_0900_as_ci;
c1	HEX(WEIGHT_STRING(c1))
÷	061800000020
×	061900000020
A	1C4700000020
a	1C4700000020
Á	1C47000000200024
á	1C47000000200024
À	1C47000000200025
à	1C47000000200025
Ă	1C47000000200026
ă	1C47000000200026
Ắ	1C470000002000260024
ắ	1C470000002000260024
Ằ	1C470000002000260025
ằ	1C470000002000260025
Ẵ	1C47000000200026002D
ẵ	1C47000000200026002D
Ẳ	1C47000000200026003B
ẳ	1C47000000200026003B
Â	1C47000000200027
â	1C47000000200027
Ấ	1C470000002000270024
ấ	1C470000002000270024
Ầ	1C470000002000270025
ầ	1C470000002000270025
Ẫ	1C47000000200027002D
ẫ	1C47000000200027002D
Ẩ	1C47000000200027003B
ẩ	1C47000000200027003B
Ǎ	1C47000000200028
ǎ	1C47000000200028
Å	1C47000000200029
å	1C47000000200029
Ǻ	1C470000002000290024
ǻ	1C470000002000290024
Ä	1C4700000020002B
ä	1C4700000020002B
Ǟ	1C4700000020002B0032
ǟ	1C4700000020002B0032
Ã	1C4700000020002D
ã	1C4700000020002D
Ǡ	1C4700000020002E0032
ǡ	1C4700000020002E0032
Ą	1C47000000200031
ą	1C47000000200031
Ā	1C47000000200032
ā	1C47000000200032
Ả	1C4700000020003B
ả	1C4700000020003B
Ạ	1C47000000200042
ạ	1C47000000200042
Ặ	1C470000002000420026
ặ	1C470000002000420026
Ậ	1C470000002000420027
ậ	1C470000002000420027
AA	1C471C47000000200020
Aa	1C471C47000000200020
aA	1C471C47000000200020
aa	1C471C47000000200020
AE	1C471CAA000000200020
Ae	1C471CAA000000200020
aE	1C471CAA000000200020
ae	1C471CAA000000200020
Æ	1C471CAA0000002001100020
æ	1C471CAA0000002001100020
Ǽ	1C471CAA00000020011000200024
ǽ	1C471CAA00000020011000200024
Ǣ	1C471CAA00000020011000200032
ǣ	1C471CAA00000020011000200032
B	1C6000000020
b	1C6000000020
ƀ	1C6800000020
Ɓ	1C7100000020
Ƃ	1C7500000020
ƃ	1C7500000020
C	1C7A00000020
c	1C7A00000020
Ć	1C7A000000200024
ć	1C7A000000200024
Ĉ	1C7A000000200027
ĉ	1C7A000000200027
Č	1C7A000000200028
č	1C7A000000200028
Ċ	1C7A00000020002E
ċ	1C7A00000020002E
Ç	1C7A000000200030
ç	1C7A000000200030
CCS	1C7A1C7A1E710000002000200020
Ccs	1C7A1C7A1E710000002000200020
cCS	1C7A1C7A1E710000002000200020
ccs	1C7A1C7A1E710000002000200020
CH	1C7A1D18000000200020
Ch	1C7A1D18000000200020
cH	1C7A1D18000000200020
ch	1C7A1D18000000200020
CS	1C7A1E71000000200020
Cs	1C7A1E71000000200020
cS	1C7A1E71000000200020
cs	1C7A1E71000000200020
Ƈ	1C8500000020
ƈ	1C8500000020
D	1C8F00000020
d	1C8F00000020
Ď	1C8F000000200028
ď	1C8F000000200028
Đ	1C8F000000200039
đ	1C8F000000200039
Ð	1C8F000000200110
ð	1C8F000000200110
DDZ	1C8F1C8F1F210000002000200020
Ddz	1C8F1C8F1F210000002000200020
dDZ	1C8F1C8F1F210000002000200020
ddz	1C8F1C8F1F210000002000200020
DDZS	1C8F1C8F1F211E7100000020002000200020
Ddzs	1C8F1C8F1F211E7100000020002000200020
dDZS	1C8F1C8F1F211E7100000020002000200020
ddzs	1C8F1C8F1F211E7100000020002000200020
DZ	1C8F1F21000000200020
Dz	1C8F1F21000000200020
dZ	1C8F1F21000000200020
dz	1C8F1F21000000200020
Ǳ	1C8F1F21000000200020
ǲ	1C8F1F21000000200020
ǳ	1C8F1F21000000200020
DŽ	1C8F1F210000002000200028
DŽ	1C8F1F210000002000200028
Dž	1C8F1F210000002000200028
Dž	1C8F1F210000002000200028
dŽ	1C8F1F210000002000200028
dž	1C8F1F210000002000200028
dž	1C8F1F210000002000200028
Ǆ	1C8F1F210000002000200028
ǅ	1C8F1F210000002000200028
ǆ	1C8F1F210000002000200028
DZS	1C8F1F211E710000002000200020
DZs	1C8F1F211E710000002000200020
DzS	1C8F1F211E710000002000200020
Dzs	1C8F1F211E710000002000200020
dZS	1C8F1F211E710000002000200020
dZs	1C8F1F211E710000002000200020
dzS	1C8F1F211E710000002000200020
dzs	1C8F1F211E710000002000200020
Ɖ	1C9700000020
Ɗ	1C9B00000020
Ƌ	1CA000000020
ƌ	1CA000000020
E	1CAA00000020
e	1CAA00000020
É	1CAA000000200024
é	1CAA000000200024
È	1CAA000000200025
è	1CAA000000200025
Ĕ	1CAA000000200026
ĕ	1CAA000000200026
Ê	1CAA000000200027
ê	1CAA000000200027
Ế	1CAA0000002000270024
ế	1CAA0000002000270024
Ề	1CAA0000002000270025
ề	1CAA0000002000270025
Ễ	1CAA000000200027002D
ễ	1CAA000000200027002D
Ể	1CAA000000200027003B
ể	1CAA000000200027003B
Ě	1CAA000000200028
ě	1CAA000000200028
Ë	1CAA00000020002B
ë	1CAA00000020002B
Ẽ	1CAA00000020002D
ẽ	1CAA00000020002D
Ė	1CAA00000020002E
ė	1CAA00000020002E
Ę	1CAA000000200031
ę	1CAA000000200031
Ē	1CAA000000200032
ē	1CAA000000200032
Ẻ	1CAA00000020003B
ẻ	1CAA00000020003B
Ẹ	1CAA000000200042
ẹ	1CAA000000200042
Ệ	1CAA0000002000420027
ệ	1CAA0000002000420027
Ǝ	1CB800000020
ǝ	1CB800000020
Ə	1CBD00000020
Ɛ	1CC200000020
F	1CE500000020
f	1CE500000020
Ƒ	1CEE00000020
ƒ	1CEE00000020
G	1CF400000020
g	1CF400000020
Ǵ	1CF4000000200024
ǵ	1CF4000000200024
Ğ	1CF4000000200026
ğ	1CF4000000200026
Ĝ	1CF4000000200027
ĝ	1CF4000000200027
Ǧ	1CF4000000200028
ǧ	1CF4000000200028
Ġ	1CF400000020002E
ġ	1CF400000020002E
Ģ	1CF4000000200030
ģ	1CF4000000200030
GGY	1CF41CF41F0B0000002000200020
Ggy	1CF41CF41F0B0000002000200020
gGY	1CF41CF41F0B0000002000200020
ggy	1CF41CF41F0B0000002000200020
GY	1CF41F0B000000200020
Gy	1CF41F0B000000200020
gY	1CF41F0B000000200020
gy	1CF41F0B000000200020
Ǥ	1D0100000020
ǥ	1D0100000020
Ɠ	1D0600000020
Ɣ	1D1000000020
Ƣ	1D1400000020
ƣ	1D1400000020
H	1D1800000020
h	1D1800000020
Ĥ	1D18000000200027
ĥ	1D18000000200027
Ħ	1D18000000200039
ħ	1D18000000200039
ƕ	1D2000000020
Ƕ	1D2000000020
I	1D3200000020
i	1D3200000020
Í	1D32000000200024
í	1D32000000200024
Ì	1D32000000200025
ì	1D32000000200025
Ĭ	1D32000000200026
ĭ	1D32000000200026
Î	1D32000000200027
î	1D32000000200027
Ǐ	1D32000000200028
ǐ	1D32000000200028
Ï	1D3200000020002B
ï	1D3200000020002B
Ĩ	1D3200000020002D
ĩ	1D3200000020002D
İ	1D3200000020002E
Į	1D32000000200031
į	1D32000000200031
Ī	1D32000000200032
ī	1D32000000200032
Ỉ	1D3200000020003B
ỉ	1D3200000020003B
Ị	1D32000000200042
ị	1D32000000200042
IJ	1D321D4C000000200020
Ij	1D321D4C000000200020
iJ	1D321D4C000000200020
ij	1D321D4C000000200020
Ĳ	1D321D4C000000200020
ĳ	1D321D4C000000200020
ı	1D3600000020
Ɨ	1D4100000020
Ɩ	1D4700000020
J	1D4C00000020
j	1D4C00000020
Ĵ	1D4C000000200027
ĵ	1D4C000000200027
ǰ	1D4C000000200028
K	1D6500000020
k	1D6500000020
Ǩ	1D65000000200028
ǩ	1D65000000200028
Ķ	1D65000000200030
ķ	1D65000000200030
Ƙ	1D6B00000020
ƙ	1D6B00000020
L	1D7700000020
l	1D7700000020
Ĺ	1D77000000200024
ĺ	1D77000000200024
Ľ	1D77000000200028
ľ	1D77000000200028
Ļ	1D77000000200030
ļ	1D77000000200030
Ł	1D77000000200039
ł	1D77000000200039
Ŀ	1D77000000200110
ŀ	1D77000000200110
LJ	1D771D4C000000200020
Lj	1D771D4C000000200020
lJ	1D771D4C000000200020
lj	1D771D4C000000200020
Ǉ	1D771D4C000000200020
ǈ	1D771D4C000000200020
ǉ	1D771D4C000000200020
LL	1D771D77000000200020
Ll	1D771D77000000200020
lL	1D771D77000000200020
ll	1D771D77000000200020
LLY	1D771D771F0B0000002000200020
Lly	1D771D771F0B0000002000200020
lLY	1D771D771F0B0000002000200020
lly	1D771D771F0B0000002000200020
LY	1D771F0B000000200020
Ly	1D771F0B000000200020
lY	1D771F0B000000200020
ly	1D771F0B000000200020
ƚ	1D8200000020
ƛ	1DA200000020
M	1DAA00000020
m	1DAA00000020
N	1DB900000020
n	1DB900000020
Ń	1DB9000000200024
ń	1DB9000000200024
Ǹ	1DB9000000200025
ǹ	1DB9000000200025
Ň	1DB9000000200028
ň	1DB9000000200028
Ñ	1DB900000020002D
ñ	1DB900000020002D
Ņ	1DB9000000200030
ņ	1DB9000000200030
NJ	1DB91D4C000000200020
Nj	1DB91D4C000000200020
nJ	1DB91D4C000000200020
nj	1DB91D4C000000200020
Ǌ	1DB91D4C000000200020
ǋ	1DB91D4C000000200020
ǌ	1DB91D4C000000200020
NNY	1DB91DB91F0B0000002000200020
Nny	1DB91DB91F0B0000002000200020
nNY	1DB91DB91F0B0000002000200020
nny	1DB91DB91F0B0000002000200020
NY	1DB91F0B000000200020
Ny	1DB91F0B000000200020
nY	1DB91F0B000000200020
ny	1DB91F0B000000200020
Ɲ	1DC400000020
ƞ	1DC800000020
Ŋ	1DD800000020
ŋ	1DD800000020
O	1DDD00000020
o	1DDD00000020
Ó	1DDD000000200024
ó	1DDD000000200024
Ò	1DDD000000200025
ò	1DDD000000200025
Ŏ	1DDD000000200026
ŏ	1DDD000000200026
Ô	1DDD000000200027
ô	1DDD000000200027
Ố	1DDD0000002000270024
ố	1DDD0000002000270024
Ồ	1DDD0000002000270025
ồ	1DDD0000002000270025
Ỗ	1DDD000000200027002D
ỗ	1DDD000000200027002D
Ổ	1DDD000000200027003B
ổ	1DDD000000200027003B
Ǒ	1DDD000000200028
ǒ	1DDD000000200028
Ö	1DDD00000020002B
ö	1DDD00000020002B
Ő	1DDD00000020002C
ő	1DDD00000020002C
Õ	1DDD00000020002D
õ	1DDD00000020002D
Ø	1DDD00000020002F
ø	1DDD00000020002F
Ǿ	1DDD00000020002F0024
ǿ	1DDD00000020002F0024
Ǫ	1DDD000000200031
ǫ	1DDD000000200031
Ǭ	1DDD0000002000310032
ǭ	1DDD0000002000310032
Ō	1DDD000000200032
ō	1DDD000000200032
Ỏ	1DDD00000020003B
ỏ	1DDD00000020003B
Ơ	1DDD00000020003F
ơ	1DDD00000020003F
Ớ	1DDD00000020003F0024
ớ	1DDD00000020003F0024
Ờ	1DDD00000020003F0025
ờ	1DDD00000020003F0025
Ỡ	1DDD00000020003F002D
ỡ	1DDD00000020003F002D
Ở	1DDD00000020003F003B
ở	1DDD00000020003F003B
Ợ	1DDD00000020003F0042
ợ	1DDD00000020003F0042
Ọ	1DDD000000200042
ọ	1DDD000000200042
Ộ	1DDD0000002000420027
ộ	1DDD0000002000420027
OE	1DDD1CAA000000200020
Oe	1DDD1CAA000000200020
oE	1DDD1CAA000000200020
oe	1DDD1CAA000000200020
Œ	1DDD1CAA0000002001100020
œ	1DDD1CAA0000002001100020
Ɔ	1DF000000020
Ɵ	1DFD00000020
P	1E0C00000020
p	1E0C00000020
Ƥ	1E1500000020
ƥ	1E1500000020
Q	1E2100000020
q	1E2100000020
ĸ	1E2F00000020
R	1E3300000020
r	1E3300000020
Ŕ	1E33000000200024
ŕ	1E33000000200024
Ř	1E33000000200028
ř	1E33000000200028
Ŗ	1E33000000200030
ŗ	1E33000000200030
RR	1E331E33000000200020
RR	1E331E33000000200020
Rr	1E331E33000000200020
Rr	1E331E33000000200020
rR	1E331E33000000200020
rR	1E331E33000000200020
rr	1E331E33000000200020
rr	1E331E33000000200020
Ʀ	1E3800000020
S	1E7100000020
s	1E7100000020
Ś	1E71000000200024
ś	1E71000000200024
Ŝ	1E71000000200027
ŝ	1E71000000200027
Š	1E71000000200028
š	1E71000000200028
Ş	1E71000000200030
ş	1E71000000200030
Ș	1E71000000200045
ș	1E71000000200045
ſ	1E71000000200111
SS	1E711E71000000200020
Ss	1E711E71000000200020
sS	1E711E71000000200020
ss	1E711E71000000200020
ß	1E711E710000002001100020
SSZ	1E711E711F210000002000200020
Ssz	1E711E711F210000002000200020
sSZ	1E711E711F210000002000200020
ssz	1E711E711F210000002000200020
SZ	1E711F21000000200020
Sz	1E711F21000000200020
sZ	1E711F21000000200020
sz	1E711F21000000200020
Ʃ	1E8200000020
ƪ	1E8800000020
T	1E9500000020
t	1E9500000020
Ť	1E95000000200028
ť	1E95000000200028
Ţ	1E95000000200030
ţ	1E95000000200030
Ț	1E95000000200045
ț	1E95000000200045
ƾ	1E951E71000000200020
TTY	1E951E951F0B0000002000200020
Tty	1E951E951F0B0000002000200020
tTY	1E951E951F0B0000002000200020
tty	1E951E951F0B0000002000200020
TY	1E951F0B000000200020
Ty	1E951F0B000000200020
tY	1E951F0B000000200020
ty	1E951F0B000000200020
Ŧ	1E9A00000020
ŧ	1E9A00000020
ƫ	1EA000000020
Ƭ	1EA400000020
ƭ	1EA400000020
Ʈ	1EA800000020
U	1EB500000020
u	1EB500000020
Ú	1EB5000000200024
ú	1EB5000000200024
Ù	1EB5000000200025
ù	1EB5000000200025
Ŭ	1EB5000000200026
ŭ	1EB5000000200026
Û	1EB5000000200027
û	1EB5000000200027
Ǔ	1EB5000000200028
ǔ	1EB5000000200028
Ů	1EB5000000200029
ů	1EB5000000200029
Ü	1EB500000020002B
ü	1EB500000020002B
Ǘ	1EB500000020002B0024
ǘ	1EB500000020002B0024
Ǜ	1EB500000020002B0025
ǜ	1EB500000020002B0025
Ǚ	1EB500000020002B0028
ǚ	1EB500000020002B0028
Ǖ	1EB500000020002B0032
ǖ	1EB500000020002B0032
Ű	1EB500000020002C
ű	1EB500000020002C
Ũ	1EB500000020002D
ũ	1EB500000020002D
Ų	1EB5000000200031
ų	1EB5000000200031
Ū	1EB5000000200032
ū	1EB5000000200032
Ủ	1EB500000020003B
ủ	1EB500000020003B
Ư	1EB500000020003F
ư	1EB500000020003F
Ứ	1EB500000020003F0024
ứ	1EB500000020003F0024
Ừ	1EB500000020003F0025
ừ	1EB500000020003F0025
Ữ	1EB500000020003F002D
ữ	1EB500000020003F002D
Ử	1EB500000020003F003B
ử	1EB500000020003F003B
Ự	1EB500000020003F0042
ự	1EB500000020003F0042
Ụ	1EB5000000200042
ụ	1EB5000000200042
UE	1EB51CAA000000200020
Ue	1EB51CAA000000200020
uE	1EB51CAA000000200020
ue	1EB51CAA000000200020
Ɯ	1ED400000020
Ʊ	1EDE00000020
V	1EE300000020
v	1EE300000020
Ʋ	1EEA00000020
W	1EF500000020
w	1EF500000020
Ŵ	1EF5000000200027
ŵ	1EF5000000200027
X	1EFF00000020
x	1EFF00000020
Y	1F0B00000020
y	1F0B00000020
Ý	1F0B000000200024
ý	1F0B000000200024
Ŷ	1F0B000000200027
ŷ	1F0B000000200027
ÿ	1F0B00000020002B
Ÿ	1F0B00000020002B
Ƴ	1F1700000020
ƴ	1F1700000020
Z	1F2100000020
z	1F2100000020
Ź	1F21000000200024
ź	1F21000000200024
Ž	1F21000000200028
ž	1F21000000200028
Ż	1F2100000020002E
ż	1F2100000020002E
ZS	1F211E71000000200020
Zs	1F211E71000000200020
zS	1F211E71000000200020
zs	1F211E71000000200020
ƍ	1F211EF5000000200020
ZZS	1F211F211E710000002000200020
Zzs	1F211F211E710000002000200020
zZS	1F211F211E710000002000200020
zzs	1F211F211E710000002000200020
Ƶ	1F2600000020
ƶ	1F2600000020
Ʒ	1F3E00000020
Ǯ	1F3E000000200028
ǯ	1F3E000000200028
Ƹ	1F4300000020
ƹ	1F4300000020
ƺ	1F4800000020
Þ	1F5000000020
þ	1F5000000020
ƿ	1F5600000020
Ƿ	1F5600000020
ƻ	1F6200000020
Ƨ	1F6900000020
ƨ	1F6900000020
Ƽ	1F6D00000020
ƽ	1F6D00000020
Ƅ	1F7100000020
ƅ	1F7100000020
ŉ	1F7E1DB9000000200020
ǀ	1F9900000020
ǁ	1F9D00000020
ǂ	1FA100000020
ǃ	1FA500000020
ඖ	28E100000020
ඤ	28EC00000020
ඥ	28ED00000020
DROP TABLE t1;

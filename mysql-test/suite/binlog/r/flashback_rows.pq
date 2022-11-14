SET  binlog_row_image=full;
# BINLOG_FORMAT = ROW
# BINLOG_ROW_IMAGE = FULL
#
# Preparatory cleanup.
#
DROP TABLE IF EXISTS t1;
DROP TABLE IF EXISTS t2;
#
# We need a fixed timestamp to avoid varying results.
#
SET timestamp=1000000000;
CREATE TABLE t1 (
c01 bigint unsigned primary key,
c02 tinyint,
c03 smallint,
c04 mediumint unsigned,
c05 int,
c06 bigint,
c07 char(10),
c08 varchar(20),
c09 varbinary(20),
c10 datetime,
c11 timestamp(3),
c12 float,
c13 decimal(10,3),
c14 TEXT,
c15 BLOB,
c16 JSON
) ENGINE=InnoDB;
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `c01` bigint unsigned NOT NULL,
  `c02` tinyint DEFAULT NULL,
  `c03` smallint DEFAULT NULL,
  `c04` mediumint unsigned DEFAULT NULL,
  `c05` int DEFAULT NULL,
  `c06` bigint DEFAULT NULL,
  `c07` char(10) DEFAULT NULL,
  `c08` varchar(20) DEFAULT NULL,
  `c09` varbinary(20) DEFAULT NULL,
  `c10` datetime DEFAULT NULL,
  `c11` timestamp(3) NULL DEFAULT NULL,
  `c12` float DEFAULT NULL,
  `c13` decimal(10,3) DEFAULT NULL,
  `c14` text,
  `c15` blob,
  `c16` json DEFAULT NULL,
  PRIMARY KEY (`c01`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
# Delete all existing binary logs.
#
RESET MASTER;
# checksum #100
checksum table t1;
Table	Checksum
test.t1	0
select * from t1;
c01	c02	c03	c04	c05	c06	c07	c08	c09	c10	c11	c12	c13	c14	c15	c16
# < CASE 1.1 >
# Insert to t1  (master-bin.000001)
#
insert into t1 values(1,1,1,1,1,1,'a','aa','aaa', '2021-06-17 00:00:01','2021-06-17 00:00:01', 1.0,1.0, repeat('a', 10),repeat('a', 10),'["a"]');
insert into t1 values(2,2,2,2,2,1,'b','bb',NULL, '2021-06-17 00:00:01','2021-06-17 00:00:01', 200.1,200.0, repeat('b', 10),repeat('b', 20),'["b"]');
insert into t1 values(3,3,3,16777215,3,1,'c','xxx','ccc', '2021-06-17 00:00:02','2021-06-17 00:00:02.101', 3.00,3.00, repeat('c', 2000),repeat('c', 10),'["c"]');
insert into t1 values(4,4,-4,4,4,4,'d','d d','xxx', '2021-06-16 23:59:59','2021-06-17 00:00:02.101', 4.0,4.0, repeat('d', 10),repeat('d', 10),'["d"]');
insert into t1 values(5,5,5,5,5,5,'e','d d',unhex(6565), '2021-06-17 00:00:02','2021-06-17 00:00:02', 5.0,5.0, repeat('e', 10),repeat('e', 10),'["e"]');
# < CASE 1.2 >
# Test update t1
#
update t1 set c11='2021-06-16 23:59:59.101' where c01 = 1;
update t1 set c11='2021-06-16 01:01:01.101' where c01 = 1;
update t1 set c11='2021-06-16 23:59:59' where c01 != 1;
# checksum #101
checksum table t1;
Table	Checksum
test.t1	1641211074
select * from t1;
c01	c02	c03	c04	c05	c06	c07	c08	c09	c10	c11	c12	c13	c14	c15	c16
1	1	1	1	1	1	a	aa	aaa	2021-06-17 00:00:01	2021-06-16 01:01:01.101	1	1.000	aaaaaaaaaa	aaaaaaaaaa	["a"]
2	2	2	2	2	1	b	bb	NULL	2021-06-17 00:00:01	2021-06-16 23:59:59.000	200.1	200.000	bbbbbbbbbb	bbbbbbbbbbbbbbbbbbbb	["b"]
3	3	3	16777215	3	1	c	xxx	ccc	2021-06-17 00:00:02	2021-06-16 23:59:59.000	3	3.000	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	cccccccccc	["c"]
4	4	-4	4	4	4	d	d d	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	4	4.000	dddddddddd	dddddddddd	["d"]
5	5	5	5	5	5	e	d d	ee	2021-06-17 00:00:02	2021-06-16 23:59:59.000	5	5.000	eeeeeeeeee	eeeeeeeeee	["e"]
FLUSH LOGS;
# < CASE 2.1 >
# Test transaction  (master-bin.000002)
#
BEGIN;
insert into t1 values(6,6,6,6,6,1,'f','ff','xxx', '2021-06-16 23:59:59','2021-06-16 23:59:59', 6.0,6.0, '','','{}'),
(700,7,700,700,700,700,'g','g,"g','xxx', '2021-06-16 23:59:59','2021-06-16 23:59:59', 7.0,7.0, '','','{}'),
(800,8,800,800,800,800,'h','h,"h','yyy', '2021-06-14 23:59:59','2021-06-14 23:59:59', 8.0,8.0, '','',NULL);
DELETE FROM t1 where c01 = 2;
COMMIT;
# < CASE 2.2 >
# Test delete from t1 and log_bin_compress
#
DELETE FROM t1 where c01 = 1;
DELETE FROM t1 where c01 in (3, 4);
# checksum #102
checksum table t1;
Table	Checksum
test.t1	545639661
select * from t1;
c01	c02	c03	c04	c05	c06	c07	c08	c09	c10	c11	c12	c13	c14	c15	c16
5	5	5	5	5	5	e	d d	ee	2021-06-17 00:00:02	2021-06-16 23:59:59.000	5	5.000	eeeeeeeeee	eeeeeeeeee	["e"]
6	6	6	6	6	1	f	ff	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	6	6.000			{}
700	7	700	700	700	700	g	g,"g	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	7	7.000			{}
800	8	800	800	800	800	h	h,"h	yyy	2021-06-14 23:59:59	2021-06-14 23:59:59.000	8	8.000			NULL
FLUSH LOGS;
# < CASE 3 >
# Alter t1 (master-bin.000003)
#
CREATE TABLE t2 (
c01 bigint unsigned primary key,
c02 varchar(20)
) ENGINE=InnoDB charset=utf8mb4;
BEGIN;
update t1 set c12=50.1,c13=50.1,c15='x中文x',c16='["j", "s", "o", "n"]' where c01=5;
replace into t2 values(100, 'xxx'),(200, 'xxx'),(300, 'ccc');
COMMIT;
# checksum #103
checksum table t1;
Table	Checksum
test.t1	768951662
select * from t1;
c01	c02	c03	c04	c05	c06	c07	c08	c09	c10	c11	c12	c13	c14	c15	c16
5	5	5	5	5	5	e	d d	ee	2021-06-17 00:00:02	2021-06-16 23:59:59.000	50.1	50.100	eeeeeeeeee	x中文x	["j", "s", "o", "n"]
6	6	6	6	6	1	f	ff	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	6	6.000			{}
700	7	700	700	700	700	g	g,"g	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	7	7.000			{}
800	8	800	800	800	800	h	h,"h	yyy	2021-06-14 23:59:59	2021-06-14 23:59:59.000	8	8.000			NULL
select * from t2;
c01	c02
100	xxx
200	xxx
300	ccc
FLUSH LOGS;
# < CASE 4 >
# Alter t1 (master-bin.000004)
#
update t1 set c12=50.1,c13=50.1,c15='yyyy',c16='{"type":"json"}' where c01=5;
alter table t1 modify c01 bigint unsigned auto_increment;
alter table t2 add index idx_c02(c02);
update t2 set c02='' where c01=300;
FLUSH LOGS;
#
# < CASE 1 >
# binlog rows filter and print it
#
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
BEGIN
/*!*/;
### INSERT INTO `test`.`t1`
### SET
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623859201.000
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623859201.000
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
### SET
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623859199.101
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623859199.101
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
### SET
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623776461.101
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
### INSERT INTO `test`.`t1`
### SET
###   @1=2
###   @2=2
###   @3=2
###   @4=2
###   @5=2
###   @6=1
###   @7='b'
###   @8='bb'
###   @9=NULL
###   @10='2021-06-17 00:00:01'
###   @11=1623859201.000
###   @12=200.1               
###   @13=200.000
###   @14='bbbbbbbbbb'
###   @15='bbbbbbbbbbbbbbbbbbbb'
###   @16='["b"]'
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=2
###   @2=2
###   @3=2
###   @4=2
###   @5=2
###   @6=1
###   @7='b'
###   @8='bb'
###   @9=NULL
###   @10='2021-06-17 00:00:01'
###   @11=1623859201.000
###   @12=200.1               
###   @13=200.000
###   @14='bbbbbbbbbb'
###   @15='bbbbbbbbbbbbbbbbbbbb'
###   @16='["b"]'
### SET
###   @1=2
###   @2=2
###   @3=2
###   @4=2
###   @5=2
###   @6=1
###   @7='b'
###   @8='bb'
###   @9=NULL
###   @10='2021-06-17 00:00:01'
###   @11=1623859199.000
###   @12=200.1               
###   @13=200.000
###   @14='bbbbbbbbbb'
###   @15='bbbbbbbbbbbbbbbbbbbb'
###   @16='["b"]'
COMMIT/*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
### INSERT INTO `test`.`t1`
### SET
###   @1=3
###   @2=3
###   @3=3
###   @4=-1 (16777215)
###   @5=3
###   @6=1
###   @7='c'
###   @8='xxx'
###   @9='ccc'
###   @10='2021-06-17 00:00:02'
###   @11=1623859202.101
###   @12=3                   
###   @13=3.000
###   @14='cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc'
###   @15='cccccccccc'
###   @16='["c"]'
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=3
###   @2=3
###   @3=3
###   @4=-1 (16777215)
###   @5=3
###   @6=1
###   @7='c'
###   @8='xxx'
###   @9='ccc'
###   @10='2021-06-17 00:00:02'
###   @11=1623859202.101
###   @12=3                   
###   @13=3.000
###   @14='cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc'
###   @15='cccccccccc'
###   @16='["c"]'
### SET
###   @1=3
###   @2=3
###   @3=3
###   @4=-1 (16777215)
###   @5=3
###   @6=1
###   @7='c'
###   @8='xxx'
###   @9='ccc'
###   @10='2021-06-17 00:00:02'
###   @11=1623859199.000
###   @12=3                   
###   @13=3.000
###   @14='cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc'
###   @15='cccccccccc'
###   @16='["c"]'
COMMIT/*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
### INSERT INTO `test`.`t1`
### SET
###   @1=5
###   @2=5
###   @3=5
###   @4=5
###   @5=5
###   @6=5
###   @7='e'
###   @8='d d'
###   @9='ee'
###   @10='2021-06-17 00:00:02'
###   @11=1623859202.000
###   @12=5                   
###   @13=5.000
###   @14='eeeeeeeeee'
###   @15='eeeeeeeeee'
###   @16='["e"]'
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
COMMIT/*!*/;
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=5
###   @2=5
###   @3=5
###   @4=5
###   @5=5
###   @6=5
###   @7='e'
###   @8='d d'
###   @9='ee'
###   @10='2021-06-17 00:00:02'
###   @11=1623859202.000
###   @12=5                   
###   @13=5.000
###   @14='eeeeeeeeee'
###   @15='eeeeeeeeee'
###   @16='["e"]'
### SET
###   @1=5
###   @2=5
###   @3=5
###   @4=5
###   @5=5
###   @6=5
###   @7='e'
###   @8='d d'
###   @9='ee'
###   @10='2021-06-17 00:00:02'
###   @11=1623859199.000
###   @12=5                   
###   @13=5.000
###   @14='eeeeeeeeee'
###   @15='eeeeeeeeee'
###   @16='["e"]'
COMMIT/*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
#
# flashback rows filter and print it
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623776461.101
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
### SET
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623859199.101
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623859199.101
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
### SET
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623859201.000
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t1`
### WHERE
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623859201.000
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
BEGIN/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=2
###   @2=2
###   @3=2
###   @4=2
###   @5=2
###   @6=1
###   @7='b'
###   @8='bb'
###   @9=NULL
###   @10='2021-06-17 00:00:01'
###   @11=1623859199.000
###   @12=200.1               
###   @13=200.000
###   @14='bbbbbbbbbb'
###   @15='bbbbbbbbbbbbbbbbbbbb'
###   @16='["b"]'
### SET
###   @1=2
###   @2=2
###   @3=2
###   @4=2
###   @5=2
###   @6=1
###   @7='b'
###   @8='bb'
###   @9=NULL
###   @10='2021-06-17 00:00:01'
###   @11=1623859201.000
###   @12=200.1               
###   @13=200.000
###   @14='bbbbbbbbbb'
###   @15='bbbbbbbbbbbbbbbbbbbb'
###   @16='["b"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t1`
### WHERE
###   @1=2
###   @2=2
###   @3=2
###   @4=2
###   @5=2
###   @6=1
###   @7='b'
###   @8='bb'
###   @9=NULL
###   @10='2021-06-17 00:00:01'
###   @11=1623859201.000
###   @12=200.1               
###   @13=200.000
###   @14='bbbbbbbbbb'
###   @15='bbbbbbbbbbbbbbbbbbbb'
###   @16='["b"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
BEGIN/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=3
###   @2=3
###   @3=3
###   @4=-1 (16777215)
###   @5=3
###   @6=1
###   @7='c'
###   @8='xxx'
###   @9='ccc'
###   @10='2021-06-17 00:00:02'
###   @11=1623859199.000
###   @12=3                   
###   @13=3.000
###   @14='cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc'
###   @15='cccccccccc'
###   @16='["c"]'
### SET
###   @1=3
###   @2=3
###   @3=3
###   @4=-1 (16777215)
###   @5=3
###   @6=1
###   @7='c'
###   @8='xxx'
###   @9='ccc'
###   @10='2021-06-17 00:00:02'
###   @11=1623859202.101
###   @12=3                   
###   @13=3.000
###   @14='cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc'
###   @15='cccccccccc'
###   @16='["c"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t1`
### WHERE
###   @1=3
###   @2=3
###   @3=3
###   @4=-1 (16777215)
###   @5=3
###   @6=1
###   @7='c'
###   @8='xxx'
###   @9='ccc'
###   @10='2021-06-17 00:00:02'
###   @11=1623859202.101
###   @12=3                   
###   @13=3.000
###   @14='cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc'
###   @15='cccccccccc'
###   @16='["c"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
BEGIN/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=4
###   @2=4
###   @3=-4 (65532)
###   @4=4
###   @5=4
###   @6=4
###   @7='d'
###   @8='d d'
###   @9='xxx'
###   @10='2021-06-16 23:59:59'
###   @11=1623859199.000
###   @12=4                   
###   @13=4.000
###   @14='dddddddddd'
###   @15='dddddddddd'
###   @16='["d"]'
### SET
###   @1=4
###   @2=4
###   @3=-4 (65532)
###   @4=4
###   @5=4
###   @6=4
###   @7='d'
###   @8='d d'
###   @9='xxx'
###   @10='2021-06-16 23:59:59'
###   @11=1623859202.101
###   @12=4                   
###   @13=4.000
###   @14='dddddddddd'
###   @15='dddddddddd'
###   @16='["d"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t1`
### WHERE
###   @1=4
###   @2=4
###   @3=-4 (65532)
###   @4=4
###   @5=4
###   @6=4
###   @7='d'
###   @8='d d'
###   @9='xxx'
###   @10='2021-06-16 23:59:59'
###   @11=1623859202.101
###   @12=4                   
###   @13=4.000
###   @14='dddddddddd'
###   @15='dddddddddd'
###   @16='["d"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
BEGIN/*!*/;
### UPDATE `test`.`t1`
### WHERE
###   @1=5
###   @2=5
###   @3=5
###   @4=5
###   @5=5
###   @6=5
###   @7='e'
###   @8='d d'
###   @9='ee'
###   @10='2021-06-17 00:00:02'
###   @11=1623859199.000
###   @12=5                   
###   @13=5.000
###   @14='eeeeeeeeee'
###   @15='eeeeeeeeee'
###   @16='["e"]'
### SET
###   @1=5
###   @2=5
###   @3=5
###   @4=5
###   @5=5
###   @6=5
###   @7='e'
###   @8='d d'
###   @9='ee'
###   @10='2021-06-17 00:00:02'
###   @11=1623859202.000
###   @12=5                   
###   @13=5.000
###   @14='eeeeeeeeee'
###   @15='eeeeeeeeee'
###   @16='["e"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t1`
### WHERE
###   @1=5
###   @2=5
###   @3=5
###   @4=5
###   @5=5
###   @6=5
###   @7='e'
###   @8='d d'
###   @9='ee'
###   @10='2021-06-17 00:00:02'
###   @11=1623859202.000
###   @12=5                   
###   @13=5.000
###   @14='eeeeeeeeee'
###   @15='eeeeeeeeee'
###   @16='["e"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
#
# < CASE 2 >
# Test --filter-lines-terminated-by and --filter-fields-enclosed-by
#
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
# [empty]
BEGIN/*!*/;
### INSERT INTO `test`.`t1`
### SET
###   @1=4
###   @2=4
###   @3=-4 (65532)
###   @4=4
###   @5=4
###   @6=4
###   @7='d'
###   @8='d d'
###   @9='xxx'
###   @10='2021-06-16 23:59:59'
###   @11=1623859199.000
###   @12=4                   
###   @13=4.000
###   @14='dddddddddd'
###   @15='dddddddddd'
###   @16='["d"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t1`
### WHERE
###   @1=700
###   @2=7
###   @3=700
###   @4=700
###   @5=700
###   @6=700
###   @7='g'
###   @8='g,"g'
###   @9='xxx'
###   @10='2021-06-16 23:59:59'
###   @11=1623859199.000
###   @12=7                   
###   @13=7.000
###   @14=''
###   @15=''
###   @16='{}'
### DELETE FROM `test`.`t1`
### WHERE
###   @1=6
###   @2=6
###   @3=6
###   @4=6
###   @5=6
###   @6=1
###   @7='f'
###   @8='ff'
###   @9='xxx'
###   @10='2021-06-16 23:59:59'
###   @11=1623859199.000
###   @12=6                   
###   @13=6.000
###   @14=''
###   @15=''
###   @16='{}'
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
# [empty]
BEGIN/*!*/;
### INSERT INTO `test`.`t1`
### SET
###   @1=3
###   @2=3
###   @3=3
###   @4=-1 (16777215)
###   @5=3
###   @6=1
###   @7='c'
###   @8='xxx'
###   @9='ccc'
###   @10='2021-06-17 00:00:02'
###   @11=1623859199.000
###   @12=3                   
###   @13=3.000
###   @14='cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc'
###   @15='cccccccccc'
###   @16='["c"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### INSERT INTO `test`.`t1`
### SET
###   @1=1
###   @2=1
###   @3=1
###   @4=1
###   @5=1
###   @6=1
###   @7='a'
###   @8='aa'
###   @9='aaa'
###   @10='2021-06-17 00:00:01'
###   @11=1623776461.101
###   @12=1                   
###   @13=1.000
###   @14='aaaaaaaaaa'
###   @15='aaaaaaaaaa'
###   @16='["a"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t1`
### WHERE
###   @1=800
###   @2=8
###   @3=800
###   @4=800
###   @5=800
###   @6=800
###   @7='h'
###   @8='h,"h'
###   @9='yyy'
###   @10='2021-06-14 23:59:59'
###   @11=1623686399.000
###   @12=8                   
###   @13=8.000
###   @14=''
###   @15=''
###   @16=NULL
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
#
# Test transaction and log_bin_compress and float
#
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
# [empty]
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### INSERT INTO `test`.`t1`
### SET
###   @1=2
###   @2=2
###   @3=2
###   @4=2
###   @5=2
###   @6=1
###   @7='b'
###   @8='bb'
###   @9=NULL
###   @10='2021-06-17 00:00:01'
###   @11=1623859199.000
###   @12=200.1               
###   @13=200.000
###   @14='bbbbbbbbbb'
###   @15='bbbbbbbbbbbbbbbbbbbb'
###   @16='["b"]'
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
#
# < CASE 3 >
# Test t1 ignoring t2 DDL and Test --conv-event-update-to-write
#
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;

DELIMITER /*!*/;
# [empty]
BEGIN/*!*/;
### INSERT INTO `test`.`t1`
### SET
###   @1=5
###   @2=5
###   @3=5
###   @4=5
###   @5=5
###   @6=5
###   @7='e'
###   @8='d d'
###   @9='ee'
###   @10='2021-06-17 00:00:02'
###   @11=1623859199.000
###   @12=5                   
###   @13=5.000
###   @14='eeeeeeeeee'
###   @15='eeeeeeeeee'
###   @16='["e"]'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
use `test`/*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
# ignore query_log_event
# CREATE TABLE t2 (# c01 bigint unsigned primary key,# c02 varchar(20)# ) ENGINE=InnoDB charset=utf8mb4 
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
#
# < CASE 4 >
# Test t1 has DDL(add auto_increment)
#
[Cannot handler this query event]
#
# Test t2 has DDL(add index) and Test --flashback-tables-ignore
#
#
# < CASE 4 >
# Flashback & Check Result
#
# checksum #103
checksum table t1;
Table	Checksum
test.t1	768951662
select * from t1;
c01	c02	c03	c04	c05	c06	c07	c08	c09	c10	c11	c12	c13	c14	c15	c16
5	5	5	5	5	5	e	d d	ee	2021-06-17 00:00:02	2021-06-16 23:59:59.000	50.1	50.100	eeeeeeeeee	x中文x	["j", "s", "o", "n"]
6	6	6	6	6	1	f	ff	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	6	6.000			{}
700	7	700	700	700	700	g	g,"g	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	7	7.000			{}
800	8	800	800	800	800	h	h,"h	yyy	2021-06-14 23:59:59	2021-06-14 23:59:59.000	8	8.000			NULL
select * from t2;
c01	c02
100	xxx
200	xxx
300	
# < CASE 3 >
# Flashback & Check Result
#
# checksum #102
checksum table t1;
Table	Checksum
test.t1	545639661
select * from t1;
c01	c02	c03	c04	c05	c06	c07	c08	c09	c10	c11	c12	c13	c14	c15	c16
5	5	5	5	5	5	e	d d	ee	2021-06-17 00:00:02	2021-06-16 23:59:59.000	5	5.000	eeeeeeeeee	eeeeeeeeee	["e"]
6	6	6	6	6	1	f	ff	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	6	6.000			{}
700	7	700	700	700	700	g	g,"g	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	7	7.000			{}
800	8	800	800	800	800	h	h,"h	yyy	2021-06-14 23:59:59	2021-06-14 23:59:59.000	8	8.000			NULL
#
# < CASE 2 >
# Flashback & Check Result
#
# checksum #101
checksum table t1;
Table	Checksum
test.t1	1641211074
select * from t1;
c01	c02	c03	c04	c05	c06	c07	c08	c09	c10	c11	c12	c13	c14	c15	c16
1	1	1	1	1	1	a	aa	aaa	2021-06-17 00:00:01	2021-06-16 01:01:01.101	1	1.000	aaaaaaaaaa	aaaaaaaaaa	["a"]
2	2	2	2	2	1	b	bb	NULL	2021-06-17 00:00:01	2021-06-16 23:59:59.000	200.1	200.000	bbbbbbbbbb	bbbbbbbbbbbbbbbbbbbb	["b"]
3	3	3	16777215	3	1	c	xxx	ccc	2021-06-17 00:00:02	2021-06-16 23:59:59.000	3	3.000	cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc	cccccccccc	["c"]
4	4	-4	4	4	4	d	d d	xxx	2021-06-16 23:59:59	2021-06-16 23:59:59.000	4	4.000	dddddddddd	dddddddddd	["d"]
5	5	5	5	5	5	e	d d	ee	2021-06-17 00:00:02	2021-06-16 23:59:59.000	5	5.000	eeeeeeeeee	eeeeeeeeee	["e"]
#
# < CASE 1 >
# Flashback & Check Result
#
# checksum #100
checksum table t1;
Table	Checksum
test.t1	0
select * from t1;
c01	c02	c03	c04	c05	c06	c07	c08	c09	c10	c11	c12	c13	c14	c15	c16
# < CASE 5 > Test data type
# Delete all existing binary logs.
#
CREATE TABLE t3 (
c01 bigint unsigned primary key,
c02 tinyint,
c03 smallint,
c04 mediumint unsigned,
c05 int,
c06 bigint,
c07 char(10),
c08 varchar(20),
c09 binary(10),
c10 varbinary(20),
c11 date,
c12 time,
c13 datetime,
c14 timestamp(3),
c15 year,
c16 float,
c17 double,
c18 decimal(10,3),
c19 TINYTEXT,
c20 MEDIUMTEXT,
c21 TEXT,
c22 LONGTEXT,
c23 TINYBLOB,
C24 MEDIUMBLOB,
c25 BLOB,
c26 LONGBLOB,
c27 ENUM('Low', 'Medium', 'High') NOT NULL,
c28 BIT(8),
c29 JSON
) ENGINE=InnoDB;
RESET MASTER;
insert into t3 values(0,0,0,0,0,0, '', '', 'a', 'aa', '2021-06-28', '11:11:11', '2021-06-28 11:11:11', '2021-06-28 11:11:11', '2021','0.0','100.0','100.000', 'aaaa', 'aaaaa', 'aaaaaa', 'aaaaaaa', 'aaaa', 'aaaaa', 'aaaaaa', 'aaaaaaa',1,b'00000000','{}');
insert into t3 values(9223372036854775807,127,-32768,8388607,2147483647,-9223372036854775808, 'b', 'bb', 'b', 'bb', '2021-06-28', '11:11:11', '2021-06-28 11:11:11', '2021-06-28 11:11:11', '2069','0.11','100.01','100.001', 'bbbb', 'bbbbb', 'bbbbbb', 'bbbbbbb', '', '', '', '',1,b'00000010','{}');
FLUSH LOGS;
# test datatype min
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t3`
### WHERE
###   @1=0
###   @2=0
###   @3=0
###   @4=0
###   @5=0
###   @6=0
###   @7=''
###   @8=''
###   @9='a'
###   @10='aa'
###   @11='2021:06:28'
###   @12='11:11:11'
###   @13='2021-06-28 11:11:11'
###   @14=1624849871.000
###   @15=2021
###   @16=0                   
###   @17=100
###   @18=100.000
###   @19='aaaa'
###   @20='aaaaa'
###   @21='aaaaaa'
###   @22='aaaaaaa'
###   @23='aaaa'
###   @24='aaaaa'
###   @25='aaaaaa'
###   @26='aaaaaaa'
###   @27=1
###   @28=b'00000000'
###   @29='{}'
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# test datatype max
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
BEGIN/*!*/;
### DELETE FROM `test`.`t3`
### WHERE
###   @1=9223372036854775807
###   @2=127
###   @3=-32768 (32768)
###   @4=8388607
###   @5=2147483647
###   @6=-9223372036854775808 (9223372036854775808)
###   @7='b'
###   @8='bb'
###   @9='b'
###   @10='bb'
###   @11='2021:06:28'
###   @12='11:11:11'
###   @13='2021-06-28 11:11:11'
###   @14=1624849871.000
###   @15=2069
###   @16=0.11                
###   @17=100.01000000000000512
###   @18=100.001
###   @19='bbbb'
###   @20='bbbbb'
###   @21='bbbbbb'
###   @22='bbbbbbb'
###   @23=''
###   @24=''
###   @25=''
###   @26=''
###   @27=1
###   @28=b'00000010'
###   @29='{}'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# test datatype misc1
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
BEGIN/*!*/;
### DELETE FROM `test`.`t3`
### WHERE
###   @1=9223372036854775807
###   @2=127
###   @3=-32768 (32768)
###   @4=8388607
###   @5=2147483647
###   @6=-9223372036854775808 (9223372036854775808)
###   @7='b'
###   @8='bb'
###   @9='b'
###   @10='bb'
###   @11='2021:06:28'
###   @12='11:11:11'
###   @13='2021-06-28 11:11:11'
###   @14=1624849871.000
###   @15=2069
###   @16=0.11                
###   @17=100.01000000000000512
###   @18=100.001
###   @19='bbbb'
###   @20='bbbbb'
###   @21='bbbbbb'
###   @22='bbbbbbb'
###   @23=''
###   @24=''
###   @25=''
###   @26=''
###   @27=1
###   @28=b'00000010'
###   @29='{}'
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t3`
### WHERE
###   @1=0
###   @2=0
###   @3=0
###   @4=0
###   @5=0
###   @6=0
###   @7=''
###   @8=''
###   @9='a'
###   @10='aa'
###   @11='2021:06:28'
###   @12='11:11:11'
###   @13='2021-06-28 11:11:11'
###   @14=1624849871.000
###   @15=2021
###   @16=0                   
###   @17=100
###   @18=100.000
###   @19='aaaa'
###   @20='aaaaa'
###   @21='aaaaaa'
###   @22='aaaaaaa'
###   @23='aaaa'
###   @24='aaaaa'
###   @25='aaaaaa'
###   @26='aaaaaaa'
###   @27=1
###   @28=b'00000000'
###   @29='{}'
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
# test datatype misc2
# The proper term is pseudo_replica_mode, but we use this compatibility alias
# to make the statement usable on server versions 8.0.24 and older.
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
ROLLBACK/*!*/;
# [empty]
BEGIN/*!*/;
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
BEGIN/*!*/;
### DELETE FROM `test`.`t3`
### WHERE
###   @1=0
###   @2=0
###   @3=0
###   @4=0
###   @5=0
###   @6=0
###   @7=''
###   @8=''
###   @9='a'
###   @10='aa'
###   @11='2021:06:28'
###   @12='11:11:11'
###   @13='2021-06-28 11:11:11'
###   @14=1624849871.000
###   @15=2021
###   @16=0                   
###   @17=100
###   @18=100.000
###   @19='aaaa'
###   @20='aaaaa'
###   @21='aaaaaa'
###   @22='aaaaaaa'
###   @23='aaaa'
###   @24='aaaaa'
###   @25='aaaaaa'
###   @26='aaaaaaa'
###   @27=1
###   @28=b'00000000'
###   @29='{}'
SET TIMESTAMP=1000000000/*!*/;
/*!\C utf8mb4 *//*!*/;
COMMIT
/*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
DROP TABLE t1;
DROP TABLE t2;
DROP TABLE t3;

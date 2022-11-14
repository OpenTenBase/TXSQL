show variables like 'cdb_convert_memory_to_innodb';
Variable_name	Value
cdb_convert_memory_to_innodb	OFF
### Startup status of Cdb_cnvt_memory_to_innodb_counts
include/assert.inc [Cdb_cnvt_memory_to_innodb_counts = 0]
create database testdb;
use testdb;
CREATE TABLE `t1` (
`a` int NOT NULL AUTO_INCREMENT,
`b` varchar(10) DEFAULT NULL,
`c` char(254) DEFAULT NULL,
PRIMARY KEY (`a`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1;
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` varchar(10) DEFAULT NULL,
  `c` char(254) DEFAULT NULL,
  PRIMARY KEY (`a`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1
CREATE TABLE `t2` (
`a` int NOT NULL AUTO_INCREMENT,
`b` varchar(10) DEFAULT NULL,
`c` char(254) DEFAULT NULL,
PRIMARY KEY (`a`)
) ENGINE=INNODB DEFAULT CHARSET=latin1;
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` varchar(10) DEFAULT NULL,
  `c` char(254) DEFAULT NULL,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
alter table t2 ENGINE=MEMORY;
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` varchar(10) DEFAULT NULL,
  `c` char(254) DEFAULT NULL,
  PRIMARY KEY (`a`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1
drop table t1,t2;
set global cdb_convert_memory_to_innodb = on;
show variables like 'cdb_convert_memory_to_innodb';
Variable_name	Value
cdb_convert_memory_to_innodb	ON
use testdb;
CREATE TABLE `t1` (
`a` int NOT NULL AUTO_INCREMENT,
`b` varchar(10) DEFAULT NULL,
`c` char(254) DEFAULT NULL,
PRIMARY KEY (`a`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1;
include/assert.inc [Cdb_cnvt_memory_to_innodb_counts = 1]
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` varchar(10) DEFAULT NULL,
  `c` char(254) DEFAULT NULL,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
CREATE TABLE `t2` (
`a` int NOT NULL AUTO_INCREMENT,
`b` varchar(10) DEFAULT NULL,
`c` char(254) DEFAULT NULL,
PRIMARY KEY (`a`)
) ENGINE=INNODB DEFAULT CHARSET=latin1;
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` varchar(10) DEFAULT NULL,
  `c` char(254) DEFAULT NULL,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
alter table t2 ENGINE=MEMORY;
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `a` int NOT NULL AUTO_INCREMENT,
  `b` varchar(10) DEFAULT NULL,
  `c` char(254) DEFAULT NULL,
  PRIMARY KEY (`a`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
drop table t1,t2;
drop database testdb;
set global cdb_convert_memory_to_innodb = off;

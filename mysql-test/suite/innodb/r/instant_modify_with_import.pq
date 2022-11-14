call mtr.add_suppression("Export is not supported for instant modified table");
create database import_test;
use import_test;
create table t1(id int,c1 int) engine=innodb;
insert into t1 values(1,1),(2,2);
create table t2 like t1;
# EXPORT
FLUSH TABLE t1 FOR EXPORT;
# Copy cfg/ibd files into temp location
UNLOCK TABLES;
DROP TABLE t1;
ALTER TABLE t2 RENAME to t1;
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `id` int DEFAULT NULL,
  `c1` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
# IMPORT
ALTER TABLE t1 DISCARD TABLESPACE;
# Copy cfg/ibd files from temp location
ALTER TABLE t1 IMPORT TABLESPACE;
SELECT * FROM t1;
id	c1
1	1
2	2
alter table t1 modify column c1 bigint,algorithm=instant;
# EXPORT instant modified table
FLUSH TABLE t1 FOR EXPORT;
ERROR HY000: Table storage engine for 'InnoDB' doesn't have this option
drop table t1;
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, c1 int) DEFAULT CHARACTER SET utf8mb4
PARTITION BY HASH(id) PARTITIONS 3;
alter table t1 modify column c1 bigint,algorithm=instant;
FLUSH TABLE t1 FOR EXPORT;
ERROR HY000: Table storage engine for 'InnoDB' doesn't have this option
drop table t1;
create table t1(id int)engine=innodb;
alter table t1 add column c1 bigint,algorithm=instant;
create table t2(id int, c1 int)engine=innodb;
alter table t2 modify column c1 bigint,algorithm=instant;
# EXPORT
FLUSH TABLE t1 FOR EXPORT;
# Copy cfg/ibd files into temp location
UNLOCK TABLES;
drop table t1;
ALTER TABLE t2 RENAME to t1;
SHOW CREATE TABLE t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `id` int DEFAULT NULL,
  `c1` bigint DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
select name, pos, version, mtype, prtype, len from information_schema.innodb_instant_modified_cols;
NAME	POS	VERSION	MTYPE	PRTYPE	LEN
c1	2	1	6	1027	4
# IMPORT
ALTER TABLE t1 DISCARD TABLESPACE;
# Copy cfg/ibd files from temp location
ALTER TABLE t1 IMPORT TABLESPACE;
ERROR HY000: Schema mismatch (Import is not supported for instant modified table)
drop table t1;
drop database import_test;

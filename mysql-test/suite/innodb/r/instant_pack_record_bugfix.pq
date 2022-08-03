set global cdb_instant_modify_column_enabled=on;
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a char(10), b varchar(10));
INSERT INTO t1 VALUES(1, repeat('ax', 4), repeat('b', 8));
INSERT INTO t1 VALUES(2, repeat('xa', 4), repeat('b', 8));
alter table t1 modify b char(20), algorithm=instant;
checksum table t1;
Table	Checksum
test.t1	2018981089
drop table t1;
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a char(10), b varchar(10));
INSERT INTO t1 VALUES(1, repeat('ax', 4), repeat('b', 8));
INSERT INTO t1 VALUES(2, repeat('xa', 4), repeat('b', 8));
alter table t1 modify b char(20), algorithm=copy;
checksum table t1;
Table	Checksum
test.t1	2018981089
drop table t1;
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a char(10));
INSERT INTO t1 VALUES(1, repeat('ax', 4));
INSERT INTO t1 VALUES(2, repeat('xa', 4));
alter table t1 add b varchar(20), algorithm=instant;
INSERT INTO t1 VALUES(3, repeat('ac', 4), repeat('b', 8));
checksum table t1;
Table	Checksum
test.t1	1901372453
drop table t1;
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a char(10));
INSERT INTO t1 VALUES(1, repeat('ax', 4));
INSERT INTO t1 VALUES(2, repeat('xa', 4));
alter table t1 add b varchar(20), algorithm=copy;
INSERT INTO t1 VALUES(3, repeat('ac', 4), repeat('b', 8));
checksum table t1;
Table	Checksum
test.t1	1901372453
drop table t1;
set global cdb_instant_modify_column_enabled=off;

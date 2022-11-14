CREATE TABLE t2(a INT, b INT, c char(255) character set utf32, d char(255)
character set utf32, e char(255) character set utf32, f char(255) character
set utf32, KEY(b)) engine=myisam;
set global binlog_write_threshold=5000;
select @@binlog_write_threshold;
@@binlog_write_threshold
5000
insert into t2 values
(1,1,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(2,2,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(3,3,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(4,4,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),
(1,1,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(2,2,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(3,3,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(4,4,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255));
ERROR HY000: binlog write threshold(5000) exceeded, the transaction has 8387 (bytes) binlogs, it is aborted.
select count(*) from t2;
count(*)
3
set global binlog_write_threshold=0;
drop table t2;
set global binlog_write_threshold=5000;
CREATE TABLE t2(a INT, b INT, c char(255) character set utf32, d char(255)
character set utf32, e char(255) character set utf32, f char(255) character
set utf32, KEY(b)) engine=innodb;
begin;
insert into t2 values 
(1,1,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(2,2,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255));
ERROR HY000: binlog write threshold(5000) exceeded, the transaction has 8387 (bytes) binlogs, it is aborted.
insert into t2 values 
(3,3,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(4,4,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),
(1,1,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(2,2,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(3,3,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255)),(4,4,repeat('c',255),repeat('d',255),repeat('e',255),repeat('f',255));
ERROR HY000: binlog write threshold(5000) exceeded, the transaction has 8387 (bytes) binlogs, it is aborted.
commit;
select count(*) from t2;
count(*)
0
drop table t2;
set global binlog_write_threshold=0;
set global binlog_write_threshold = 5;
create table t2 (a int, b int) engine = innodb;
ERROR HY000: binlog write threshold(5) exceeded, the transaction has 130 (bytes) binlogs, it is aborted.
set global binlog_write_threshold = 0;
create table t2 (a int, b int) engine = innodb;
set global binlog_write_threshold = 5;
drop table t2;
ERROR HY000: binlog write threshold(5) exceeded, the transaction has 123 (bytes) binlogs, it is aborted.
set global binlog_write_threshold = 0;
drop table t2;

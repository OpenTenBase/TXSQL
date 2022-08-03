Verbose statements from : write-partial-row.binlog
select replace(txt,'\r', '') as stmt from raw_binlog_rows where txt like '###%';
stmt
###   @1=1
###   @1=1
###   @1=2
###   @1=2
###   @1=3
###   @1=4
###   @1=4
###   @2=1
###   @2=2
###   @2=25769803786
###   @2=3
###   @2=4
###   @3=''
###   @3=1
###   @3=2
###   @3=3
###   @3=4
###   @3=40
###   @4=0
###   @5=0
### DELETE FROM `test`.`ba`
### INSERT INTO `mysql`.`ndb_apply_status`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### SET
### SET
### SET
### SET
### SET
### SET
### WHERE
drop table raw_binlog_rows;
Verbose statements from : write-full-row.binlog
select replace(txt,'\r', '') as stmt from raw_binlog_rows where txt like '###%';
stmt
###   @1=1
###   @1=2
###   @1=2
###   @1=2
###   @1=3
###   @1=4
###   @1=4
###   @2=1
###   @2=2
###   @2=25769803786
###   @2=3
###   @2=4
###   @2=4
###   @3=''
###   @3=1
###   @3=2
###   @3=3
###   @3=4
###   @3=40
###   @4=0
###   @5=0
### DELETE FROM `test`.`ba`
### INSERT INTO `mysql`.`ndb_apply_status`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### SET
### SET
### SET
### SET
### SET
### SET
### WHERE
drop table raw_binlog_rows;
Verbose statements from : update-partial-row.binlog
select replace(txt,'\r', '') as stmt from raw_binlog_rows where txt like '###%';
stmt
###   @1=1
###   @1=2
###   @1=2
###   @1=3
###   @1=3
###   @1=4
###   @1=4
###   @1=4
###   @2=1
###   @2=2
###   @2=25769803786
###   @2=3
###   @2=4
###   @3=''
###   @3=1
###   @3=2
###   @3=3
###   @3=4
###   @3=4
###   @3=40
###   @4=0
###   @5=0
### DELETE FROM `test`.`ba`
### INSERT INTO `mysql`.`ndb_apply_status`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### SET
### SET
### SET
### SET
### SET
### SET
### UPDATE `test`.`ba`
### WHERE
### WHERE
drop table raw_binlog_rows;
Verbose statements from : update-full-row.binlog
select replace(txt,'\r', '') as stmt from raw_binlog_rows where txt like '###%';
stmt
###   @1=1
###   @1=2
###   @1=2
###   @1=3
###   @1=4
###   @1=4
###   @1=4
###   @1=4
###   @2=1
###   @2=2
###   @2=25769803786
###   @2=3
###   @2=4
###   @2=4
###   @2=4
###   @3=''
###   @3=1
###   @3=2
###   @3=3
###   @3=4
###   @3=4
###   @3=40
###   @4=0
###   @5=0
### DELETE FROM `test`.`ba`
### INSERT INTO `mysql`.`ndb_apply_status`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### INSERT INTO `test`.`ba`
### SET
### SET
### SET
### SET
### SET
### SET
### UPDATE `test`.`ba`
### WHERE
### WHERE
drop table raw_binlog_rows;
SET @@SESSION.BINLOG_ROWS_QUERY_LOG_EVENTS = 1;
CREATE TABLE t1 (a VARCHAR(50));
INSERT INTO t1 VALUES ("
  GRANT ALL ON *.* TO 'batman'/*!*/;
  ")|
DROP TABLE t1;

#
# Preparatory cleanup.
#
DROP TABLE IF EXISTS t1;
#
# We need a fixed timestamp to avoid varying results.
#
SET timestamp=1000000000;
#
# Delete all existing binary logs.
#
RESET MASTER;
CREATE TABLE t1 (
c01 tinyint,
c02 smallint primary key,
c03 mediumint,
c04 int,
c05 bigint,
c06 char(10),
c07 varchar(20),
c08 TEXT
) ENGINE=InnoDB;
#
# Insert data to t1
#
INSERT INTO t1 VALUES(0,0,0,0,0,'','','');
INSERT INTO t1 VALUES(1,2,3,4,5, "abc", "abcdefg", "abcedfghijklmnopqrstuvwxyz");
INSERT INTO t1 VALUES(127, 32767, 8388607, 2147483647, 9223372036854775807, repeat('a', 10), repeat('a', 20), repeat('a', 255));
CREATE TABLE test(col1 int, col2 int primary key);
#
# Update t1
#
UPDATE t1 SET c01=100 WHERE c02=0 OR c03=3;
ALTER TABLE test ADD COLUMN col3 INT;
#
# Clear t1
#
DELETE FROM t1;
DROP TABLE test;
SET BINLOG_FORMAT = STATEMENT;
CREATE TABLE test2(a INT);
INSERT INTO test2 VALUES(1);
DROP TABLE test2;
FLUSH LOGS;
SET BINLOG_FORMAT = ROW;
#
# Show mysqlbinlog result without -B
#
/*!50530 SET @@SESSION.PSEUDO_SLAVE_MODE=1*/;
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Start: binlog v 4, server v #.##.## created 010909  4:46:40 at startup
ROLLBACK/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Previous-GTIDs
# [empty]
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=0	sequence_number=1	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
use `test`/*!*/;
SET TIMESTAMP=1000000000/*!*/;
SET @@session.pseudo_thread_id=#/*!*/;
SET @@session.foreign_key_checks=1, @@session.sql_auto_is_null=0, @@session.unique_checks=1, @@session.autocommit=1/*!*/;
SET @@session.sql_mode=X/*!*/;
SET @@session.auto_increment_increment=1, @@session.auto_increment_offset=1/*!*/;
/*!\C utf8mb4 *//*!*/;
SET @@session.character_set_client=X,@@session.collation_connection=X,@@session.collation_server=X/*!*/;
SET @@session.lc_time_names=0/*!*/;
SET @@session.collation_database=DEFAULT/*!*/;
/*!80011 SET @@session.default_collation_for_utf8mb4=255*//*!*/;
/*!80013 SET @@session.sql_require_primary_key=0*//*!*/;
CREATE TABLE t1 (
c01 tinyint,
c02 smallint primary key,
c03 mediumint,
c04 int,
c05 bigint,
c06 char(10),
c07 varchar(20),
c08 TEXT
) ENGINE=InnoDB
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=1	sequence_number=2	rbr_only=yes	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Write_rows: table id # flags: STMT_END_F
### INSERT INTO `test`.`t1`
### SET
###   @1=0 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=0 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=0 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=0 /* INT meta=0 nullable=1 is_null=0 */
###   @5=0 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=2	sequence_number=3	rbr_only=yes	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Write_rows: table id # flags: STMT_END_F
### INSERT INTO `test`.`t1`
### SET
###   @1=1 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=2 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=3 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=4 /* INT meta=0 nullable=1 is_null=0 */
###   @5=5 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='abc' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='abcdefg' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='abcedfghijklmnopqrstuvwxyz' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=3	sequence_number=4	rbr_only=yes	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Write_rows: table id # flags: STMT_END_F
### INSERT INTO `test`.`t1`
### SET
###   @1=127 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=32767 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=8388607 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=2147483647 /* INT meta=0 nullable=1 is_null=0 */
###   @5=9223372036854775807 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='aaaaaaaaaa' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='aaaaaaaaaaaaaaaaaaaa' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=4	sequence_number=5	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
/*!80013 SET @@session.sql_require_primary_key=0*//*!*/;
CREATE TABLE test(col1 int, col2 int primary key)
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=5	sequence_number=6	rbr_only=yes	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Update_rows: table id # flags: STMT_END_F
### UPDATE `test`.`t1`
### WHERE
###   @1=0 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=0 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=0 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=0 /* INT meta=0 nullable=1 is_null=0 */
###   @5=0 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### SET
###   @1=100 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=0 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=0 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=0 /* INT meta=0 nullable=1 is_null=0 */
###   @5=0 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### UPDATE `test`.`t1`
### WHERE
###   @1=1 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=2 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=3 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=4 /* INT meta=0 nullable=1 is_null=0 */
###   @5=5 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='abc' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='abcdefg' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='abcedfghijklmnopqrstuvwxyz' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### SET
###   @1=100 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=2 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=3 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=4 /* INT meta=0 nullable=1 is_null=0 */
###   @5=5 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='abc' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='abcdefg' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='abcedfghijklmnopqrstuvwxyz' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=6	sequence_number=7	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
/*!80013 SET @@session.sql_require_primary_key=0*//*!*/;
ALTER TABLE test ADD COLUMN col3 INT
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=7	sequence_number=8	rbr_only=yes	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
/*!50718 SET TRANSACTION ISOLATION LEVEL READ COMMITTED*//*!*/;
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Delete_rows: table id # flags: STMT_END_F
### DELETE FROM `test`.`t1`
### WHERE
###   @1=100 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=0 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=0 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=0 /* INT meta=0 nullable=1 is_null=0 */
###   @5=0 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### DELETE FROM `test`.`t1`
### WHERE
###   @1=100 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=2 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=3 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=4 /* INT meta=0 nullable=1 is_null=0 */
###   @5=5 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='abc' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='abcdefg' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='abcedfghijklmnopqrstuvwxyz' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### DELETE FROM `test`.`t1`
### WHERE
###   @1=127 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=32767 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=8388607 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=2147483647 /* INT meta=0 nullable=1 is_null=0 */
###   @5=9223372036854775807 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='aaaaaaaaaa' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='aaaaaaaaaaaaaaaaaaaa' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=8	sequence_number=9	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
DROP TABLE `test` /* generated by server */
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=9	sequence_number=10	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
/*!80013 SET @@session.sql_require_primary_key=0*//*!*/;
CREATE TABLE test2(a INT)
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=10	sequence_number=11	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
INSERT INTO test2 VALUES(1)
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=11	sequence_number=12	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
# original_commit_timestamp=X (X-X-X X:X:X.X GMT)
# immediate_commit_timestamp=X (X-X-X X:X:X.X GMT)
/*!80001 SET @@session.original_commit_timestamp=X*//*!*/;
/*!80014 SET @@session.original_server_version=X*//*!*/;
/*!80014 SET @@session.immediate_server_version=X*//*!*/;
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
DROP TABLE `test2` /* generated by server */
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Rotate to binlog.000002  pos: 4
SET @@SESSION.GTID_NEXT= 'AUTOMATIC' /* added by mysqlbinlog */ /*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
/*!50530 SET @@SESSION.PSEUDO_SLAVE_MODE=0*/;
#
# Show mysqlbinlog result with -B
#
/*!50530 SET @@SESSION.PSEUDO_SLAVE_MODE=1*/;
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Start: binlog v 4, server v #.##.## created 010909  4:46:40 at startup
ROLLBACK/*!*/;
/*!50616 SET @@SESSION.GTID_NEXT='AUTOMATIC'*//*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Previous-GTIDs
# [empty]
# at #
# at #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
# at #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
# at #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Table_map: `test`.`t1` mapped to number #
# at #
# at #
# at #
# at #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Rotate to binlog.000002  pos: 4
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
BEGIN/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
/*!80013 SET @@session.sql_require_primary_key=0*//*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
BEGIN/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Write_rows: table id # flags: STMT_END_F
### INSERT INTO `test`.`t1`
### SET
###   @1=127 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=32767 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=8388607 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=2147483647 /* INT meta=0 nullable=1 is_null=0 */
###   @5=9223372036854775807 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='aaaaaaaaaa' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='aaaaaaaaaaaaaaaaaaaa' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### INSERT INTO `test`.`t1`
### SET
###   @1=100 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=2 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=3 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=4 /* INT meta=0 nullable=1 is_null=0 */
###   @5=5 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='abc' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='abcdefg' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='abcedfghijklmnopqrstuvwxyz' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### INSERT INTO `test`.`t1`
### SET
###   @1=100 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=0 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=0 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=0 /* INT meta=0 nullable=1 is_null=0 */
###   @5=0 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
/*!80013 SET @@session.sql_require_primary_key=0*//*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
BEGIN/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Update_rows: table id # flags: STMT_END_F
### UPDATE `test`.`t1`
### WHERE
###   @1=100 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=2 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=3 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=4 /* INT meta=0 nullable=1 is_null=0 */
###   @5=5 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='abc' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='abcdefg' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='abcedfghijklmnopqrstuvwxyz' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### SET
###   @1=1 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=2 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=3 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=4 /* INT meta=0 nullable=1 is_null=0 */
###   @5=5 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='abc' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='abcdefg' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='abcedfghijklmnopqrstuvwxyz' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### UPDATE `test`.`t1`
### WHERE
###   @1=100 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=0 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=0 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=0 /* INT meta=0 nullable=1 is_null=0 */
###   @5=0 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
### SET
###   @1=0 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=0 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=0 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=0 /* INT meta=0 nullable=1 is_null=0 */
###   @5=0 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
SET TIMESTAMP=1000000000/*!*/;
/*!80013 SET @@session.sql_require_primary_key=0*//*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
BEGIN/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Delete_rows: table id # flags: STMT_END_F
### DELETE FROM `test`.`t1`
### WHERE
###   @1=127 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=32767 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=8388607 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=2147483647 /* INT meta=0 nullable=1 is_null=0 */
###   @5=9223372036854775807 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='aaaaaaaaaa' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='aaaaaaaaaaaaaaaaaaaa' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
BEGIN/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Delete_rows: table id # flags: STMT_END_F
### DELETE FROM `test`.`t1`
### WHERE
###   @1=1 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=2 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=3 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=4 /* INT meta=0 nullable=1 is_null=0 */
###   @5=5 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='abc' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='abcdefg' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='abcedfghijklmnopqrstuvwxyz' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
BEGIN/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Delete_rows: table id # flags: STMT_END_F
### DELETE FROM `test`.`t1`
### WHERE
###   @1=0 /* TINYINT meta=0 nullable=1 is_null=0 */
###   @2=0 /* SHORTINT meta=0 nullable=0 is_null=0 */
###   @3=0 /* MEDIUMINT meta=0 nullable=1 is_null=0 */
###   @4=0 /* INT meta=0 nullable=1 is_null=0 */
###   @5=0 /* LONGINT meta=0 nullable=1 is_null=0 */
###   @6='' /* STRING(40) meta=65064 nullable=1 is_null=0 */
###   @7='' /* VARSTRING(80) meta=80 nullable=1 is_null=0 */
###   @8='' /* BLOB/TEXT meta=2 nullable=1 is_null=0 */
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
COMMIT
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0	Xid = #
use `test`/*!*/;
SET TIMESTAMP=1000000000/*!*/;
SET @@session.pseudo_thread_id=#/*!*/;
SET @@session.foreign_key_checks=1, @@session.sql_auto_is_null=0, @@session.unique_checks=1, @@session.autocommit=1/*!*/;
SET @@session.sql_mode=X/*!*/;
SET @@session.auto_increment_increment=1, @@session.auto_increment_offset=1/*!*/;
/*!\C utf8mb4 *//*!*/;
SET @@session.character_set_client=X,@@session.collation_connection=X,@@session.collation_server=X/*!*/;
SET @@session.lc_time_names=0/*!*/;
SET @@session.collation_database=DEFAULT/*!*/;
/*!80011 SET @@session.default_collation_for_utf8mb4=255*//*!*/;
/*!80013 SET @@session.sql_require_primary_key=0*//*!*/;
COMMIT /* added by mysqlbinlog *//*!*/;
DELIMITER ;
# End of log file
/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;
/*!50530 SET @@SESSION.PSEUDO_SLAVE_MODE=0*/;
#
# Insert data to t1
#
TRUNCATE TABLE t1;
INSERT INTO t1 VALUES(0,0,0,0,0,'','','');
INSERT INTO t1 VALUES(1,2,3,4,5, "abc", "abcdefg", "abcedfghijklmnopqrstuvwxyz");
INSERT INTO t1 VALUES(127, 32767, 8388607, 2147483647, 9223372036854775807, repeat('a', 10), repeat('a', 20), repeat('a', 60));
#
# Delete all existing binary logs.
#
RESET MASTER;
SELECT * FROM t1;
c01	c02	c03	c04	c05	c06	c07	c08
0	0	0	0	0			
1	2	3	4	5	abc	abcdefg	abcedfghijklmnopqrstuvwxyz
127	32767	8388607	2147483647	9223372036854775807	aaaaaaaaaa	aaaaaaaaaaaaaaaaaaaa	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
#
# Operate some data
#
UPDATE t1 SET c01=20;
UPDATE t1 SET c04=200;
UPDATE t1 SET c03=2000;
DELETE FROM t1;
FLUSH LOGS;
#
# Flashback & Check the result
#
SELECT * FROM t1;
c01	c02	c03	c04	c05	c06	c07	c08
0	0	0	0	0			
1	2	3	4	5	abc	abcdefg	abcedfghijklmnopqrstuvwxyz
127	32767	8388607	2147483647	9223372036854775807	aaaaaaaaaa	aaaaaaaaaaaaaaaaaaaa	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
RESET MASTER;
#
# UPDATE multi-rows in one event
#
BEGIN;
UPDATE t1 SET c01=10 WHERE c01=0;
UPDATE t1 SET c01=20 WHERE c01=10;
UPDATE t1 SET c01=30 WHERE c01=20;
COMMIT;
FLUSH LOGS;
#
# Flashback & Check the result
#
SELECT * FROM t1;
c01	c02	c03	c04	c05	c06	c07	c08
0	0	0	0	0			
1	2	3	4	5	abc	abcdefg	abcedfghijklmnopqrstuvwxyz
127	32767	8388607	2147483647	9223372036854775807	aaaaaaaaaa	aaaaaaaaaaaaaaaaaaaa	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
DROP TABLE t1;
#
# Flashback --table=t1 test 
#
CREATE TABLE T1(col1 int primary key, col2 int);
CREATE TABLE T2(col1 int primary key, col2 int);
INSERT INTO T1 VALUES(1,1),(2,2),(3,3);
INSERT INTO T2 VALUES(1,1),(2,2),(3,3);
RESET MASTER;
SELECT *FROM T1;
col1	col2
1	1
2	2
3	3
SELECT *FROM T2;
col1	col2
1	1
2	2
3	3
DELETE FROM T1;
DELETE FROM T2;
FLUSH LOGS;
SELECT *FROM T1;
col1	col2
1	1
2	2
3	3
SELECT *FROM T2;
col1	col2
DROP TABLE T1,T2;
#
# Self-referencing foreign keys
#
CREATE TABLE t1 (a INT PRIMARY KEY, b INT, FOREIGN KEY my_fk(b) REFERENCES t1(a)) ENGINE=InnoDB;
BEGIN;
INSERT INTO t1 VALUES (1, NULL);
INSERT INTO t1 VALUES (2, 1), (3, 2), (4, 3);
COMMIT;
SELECT * FROM t1;
a	b
1	NULL
2	1
3	2
4	3
RESET MASTER;
DELETE FROM t1 ORDER BY a DESC;
FLUSH LOGS;
#
# Flashback & Check the result
#
SELECT * FROM t1;
a	b
1	NULL
2	1
3	2
4	3
DROP TABLE t1;

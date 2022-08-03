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
c02 smallint,
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
#
# Update t1
#
UPDATE t1 SET c01=100 WHERE c02=0 OR c03=3;
#
# Clear t1
#
DELETE FROM t1;
FLUSH LOGS;
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
c02 smallint,
c03 mediumint,
c04 int,
c05 bigint,
c06 char(10),
c07 varchar(20),
c08 TEXT
) ENGINE=InnoDB
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=1	sequence_number=2	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
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
INSERT INTO t1 VALUES(0,0,0,0,0,'','','')
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=2	sequence_number=3	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
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
INSERT INTO t1 VALUES(1,2,3,4,5, "abc", "abcdefg", "abcedfghijklmnopqrstuvwxyz")
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=3	sequence_number=4	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
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
INSERT INTO t1 VALUES(127, 32767, 8388607, 2147483647, 9223372036854775807, repeat('a', 10), repeat('a', 20), repeat('a', 255))
/*!*/;
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
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
BEGIN
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Query	thread_id=#	exec_time=#	error_code=0
SET TIMESTAMP=1000000000/*!*/;
UPDATE t1 SET c01=100 WHERE c02=0 OR c03=3
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Anonymous_GTID	last_committed=5	sequence_number=6	rbr_only=no	original_committed_timestamp=X	immediate_commit_timestamp=X	transaction_length=X
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
DELETE FROM t1
/*!*/;
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Xid = #
COMMIT/*!*/;
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
# at #
# at #
# at #
# at #
#010909  4:46:40 server id 1  end_log_pos # CRC32 XXX 	Rotate to binlog.000002  pos: 4
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
UPDATE t1 SET c02=200;
UPDATE t1 SET c03=2000;
DELETE FROM t1;
FLUSH LOGS;
#
# Flashback & Check the result
#
SELECT * FROM t1;
c01	c02	c03	c04	c05	c06	c07	c08
RESET MASTER;
#
# UPDATE multi-rows in one event
#
BEGIN;
UPDATE t1 SET c01=10 WHERE c01=0;
UPDATE t1 SET c01=20 WHERE c01=10;
COMMIT;
FLUSH LOGS;
#
# Flashback & Check the result
#
SELECT * FROM t1;
c01	c02	c03	c04	c05	c06	c07	c08
DROP TABLE t1;
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
DROP TABLE t1;

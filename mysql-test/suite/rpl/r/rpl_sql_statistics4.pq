include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
CREATE TABLE `t1` (
`c1` int(11),
`c2` int(11)
) ENGINE=InnoDB;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
insert into t1 values(10,10),(20,20), (30, 30), (40, 40);
update t1 set c1=0;
SELECT * FROM  information_schema.CDB_SQL_STATISTICS;
DIGEST_MD5	564adfcfc4c214c58326dd1da050bb6eda3f1e3fd0ee5d1effcf024c3dab7123
AVER_AFFECTED_ROWS	4
DIGEST_TEXT	UPDATE `t1` SET `c1` = ?
EXECUTE_COUNT	1
FIRST_UPDATE_TIMESTAMP	#
LAST_ACCESS_TIMESTAMP	#
LAST_AFFECTED_ROWS	4
LAST_UPDATE_TIMESTAMP	#
SQL_COMMAND	UPDATE
STMT_BINLOG_FORMAT_IF_POSSIBLE	TRUE
TOTAL_AFFECTED_ROWS	4
update t1 set c1= 10;
SELECT * FROM  information_schema.CDB_SQL_STATISTICS;
DIGEST_MD5	564adfcfc4c214c58326dd1da050bb6eda3f1e3fd0ee5d1effcf024c3dab7123
AVER_AFFECTED_ROWS	4
DIGEST_TEXT	UPDATE `t1` SET `c1` = ?
EXECUTE_COUNT	2
FIRST_UPDATE_TIMESTAMP	#
LAST_ACCESS_TIMESTAMP	#
LAST_AFFECTED_ROWS	4
LAST_UPDATE_TIMESTAMP	#
SQL_COMMAND	UPDATE
STMT_BINLOG_FORMAT_IF_POSSIBLE	TRUE
TOTAL_AFFECTED_ROWS	8
set global cdb_optimize_large_trans_binlog=OFF;
update t1 set c1= 0;
SELECT * FROM  information_schema.CDB_SQL_STATISTICS;
DIGEST_MD5	564adfcfc4c214c58326dd1da050bb6eda3f1e3fd0ee5d1effcf024c3dab7123
AVER_AFFECTED_ROWS	4
DIGEST_TEXT	UPDATE `t1` SET `c1` = ?
EXECUTE_COUNT	3
FIRST_UPDATE_TIMESTAMP	#
LAST_ACCESS_TIMESTAMP	#
LAST_AFFECTED_ROWS	4
LAST_UPDATE_TIMESTAMP	#
SQL_COMMAND	UPDATE
STMT_BINLOG_FORMAT_IF_POSSIBLE	TRUE
TOTAL_AFFECTED_ROWS	12
set global cdb_optimize_large_trans_binlog=ON;
set global cdb_sql_statistics_info_threshold=0;
SELECT * FROM  information_schema.CDB_SQL_STATISTICS;
set global cdb_sql_statistics_info_threshold=10000;
select * from t1;
c1	c2
0	10
0	20
0	30
0	40
include/show_binlog_events.inc
Log_name	Pos	Event_type	Server_id	End_log_pos	Info
master-bin.000001	#	Query	#	#	use `test`; CREATE TABLE `t1` (
`c1` int(11),
`c2` int(11)
) ENGINE=InnoDB
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Table_map	#	#	table_id: # (test.t1)
master-bin.000001	#	Write_rows	#	#	table_id: # flags: STMT_END_F
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Table_map	#	#	table_id: # (test.t1)
master-bin.000001	#	Update_rows	#	#	table_id: # flags: STMT_END_F
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Query	#	#	use `test`; update t1 set c1= 10
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
master-bin.000001	#	Query	#	#	BEGIN
master-bin.000001	#	Table_map	#	#	table_id: # (test.t1)
master-bin.000001	#	Update_rows	#	#	table_id: # flags: STMT_END_F
master-bin.000001	#	Xid	#	#	COMMIT /* XID */
include/sync_slave_sql_with_master.inc
select * from t1;
c1	c2
0	10
0	20
0	30
0	40
drop table t1;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

include/assert.inc [Cdb_lock_nowait_counts = 0]
include/assert.inc [Cdb_skip_locked_counts = 0]
SET SESSION innodb_lock_wait_timeout=1;
SET SESSION innodb_lock_wait_timeout=1;
CREATE TABLE t1(
seat_id    INT,
pos        POINT NOT NULL,
state      INT,
PRIMARY KEY(seat_id)
) ENGINE=InnoDB;
INSERT INTO t1 VALUES
(1,ST_PointFromText('POINT(1 0)'),0),
(3,ST_PointFromText('POINT(2 0)'),0),
(4,ST_PointFromText('POINT(1 1)'),0),
(6,ST_PointFromText('POINT(2 1)'),0);
BEGIN;
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 AND seat_id >= 1 LIMIT 2 FOR SHARE;
BEGIN;
INSERT INTO t1 VALUES(2,ST_PointFromText('POINT(2 1)'),0);
SELECT seat_id, state, ST_AsText(pos) FROM t1 WHERE state = 0
FOR UPDATE NOWAIT;
SELECT seat_id, state, ST_AsText(pos) FROM t1 WHERE state = 0
FOR UPDATE SKIP LOCKED;
SET @g = ST_GeomFromText('POLYGON((0 0,0 3,3 3,0 3,0 0))');
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 AND MBRWithin(pos, @g) LIMIT 2 FOR UPDATE NOWAIT;
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 AND MBRWithin(pos, @g) LIMIT 2 FOR UPDATE SKIP LOCKED;
COMMIT;
COMMIT;
DROP TABLE t1;
include/assert.inc [Cdb_lock_nowait_counts = 2]
include/assert.inc [Cdb_skip_locked_counts = 4]
include/assert.inc [Cdb_show_detail_processlist_counts = 0]
show detail processlist;
include/assert.inc [Cdb_show_detail_processlist_counts = 1]
include/assert.inc [Cdb_show_threadpool_counts = 0]
show threadpool status;
include/assert.inc [Cdb_show_threadpool_counts = 1]
include/assert.inc [Cdb_show_enhanced_full_processlist_counts = 0]
show full processlist;
include/assert.inc [Cdb_show_enhanced_full_processlist_counts = 1]
set global cdb_sql_filter = "+SELECT,-1,1,value";
create user 'xx'@'%';
grant all on test.* to 'xx'@'%';
create table test(id int, value1 int, value2 int, value3 int);
insert into test values(1,1,1,1);
begin;
select * from test for update;
select value1 from test lock in share mode;;
select value2 from test lock in share mode;
commit;
include/assert.inc [Cdb_sql_filter_rejected_counts = 1]
drop table test;
drop user 'xx'@'%';
include/assert.inc [Cdb_parse_ddl_counts = 0]
parser create table aa(b int);
include/assert.inc [Cdb_parse_ddl_counts = 1]
include/assert.inc [Innodb_cdb_check_index_counts = 0]
create table t1(id int,c1 int);
check index from t1;
include/assert.inc [Innodb_cdb_check_index_counts = 1]
drop table t1;
use test;
## ALTER TABLE
create table t1(a int, b int, c int, d int);
include/assert.inc [cdb_instant_ddl_counts = 0]
alter table t1 add column (e int), algorithm=instant;
alter table t1 add column (f int), algorithm=instant;
include/assert.inc [cdb_instant_ddl_counts = 2]
DROP TABLE t1;
CREATE RESOURCE GROUP res_grp1 TYPE=USER VCPU=1,3-4 THREAD_PRIORITY=1 IOPS_LIMIT=1000 MEMORY_LIMIT=30000000 THREAD_IOPS_MAX_PERCENT=50 THREAD_MEMORY_MAX_PERCENT=50 ENABLE;
ALTER RESOURCE GROUP res_grp1 THREAD_PRIORITY=-1 IOPS_LIMIT=2000 ENABLE;
SET RESOURCE GROUP res_grp1;
SHOW RESOURCE GROUP STATUS;
Resource_group_name	Type	Enabled	Vcpu_ids	Thread_priority	Iops_limit	Memory_limit	Thread_iops_max_percent	Thread_memory_max_percent	Current_iops	Current_memory	Sql_Timeout	Timeout_action
res_grp1	User	1	#	#	#	#	#	#	#	#	0	NONE
SYS_default	System	1	#	#	#	#	#	#	#	#	0	
USR_default	User	1	#	#	#	#	#	#	#	#	0	
SET RESOURCE GROUP USR_default;
DROP RESOURCE GROUP res_grp1;
show status like "cdb%resource_group_counts";
Variable_name	Value
Cdb_alter_resource_group_counts	1
Cdb_create_resource_group_counts	1
Cdb_drop_resource_group_counts	1
Cdb_set_resource_group_counts	2
Cdb_show_resource_group_counts	1
reset master;
include/master-slave.inc
[connection master]
include/assert.inc [Cdb_transmit_buffer_pool_counts = 0]
set @old_snapshot_pct = @@innodb_buffer_pool_snapshot_pct;
SET GLOBAL innodb_buffer_pool_snapshot_pct = 100;
set @old_transmit_enabled = @@innodb_buffer_pool_transmit_enabled;
set @old_recover_now = @@innodb_buffer_pool_recover_now;
set @old_recover_pct = @@innodb_buffer_pool_recover_pct;
SET GLOBAL innodb_buffer_pool_recover_pct = 100;
CREATE TABLE ib_bp_test
(a INT AUTO_INCREMENT, b VARCHAR(64), c TEXT, PRIMARY KEY (a), KEY (b, c(128)))
ENGINE=INNODB;
INSERT INTO ib_bp_test (b, c) VALUES (REPEAT('b', 64), REPEAT('c', 256));
INSERT INTO ib_bp_test (b, c) VALUES (REPEAT('B', 64), REPEAT('C', 256));
include/assert.inc [Innodb_cdb_snapshot_buffer_pool_counts = 0]
SET GLOBAL innodb_buffer_pool_snapshot_now = ON;
include/assert.inc [Innodb_cdb_snapshot_buffer_pool_counts = 1]
include/assert.inc [Innodb_cdb_recover_buffer_pool_counts = 0]
SET GLOBAL innodb_buffer_pool_transmit_enabled = ON;
stop slave;
start slave;
include/assert.inc [Cdb_transmit_buffer_pool_counts = 1]
SET GLOBAL innodb_buffer_pool_recover_now = ON;
include/assert.inc [Innodb_cdb_recover_buffer_pool_counts = 1]
SET GLOBAL innodb_buffer_pool_transmit_enabled = @old_transmit_enabled;
SET GLOBAL innodb_buffer_pool_recover_now = @old_recover_now;
SET GLOBAL innodb_buffer_pool_recover_pct = @old_recover_pct;
stop slave;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
start slave;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
SET GLOBAL innodb_buffer_pool_snapshot_pct = @old_snapshot_pct;
drop table ib_bp_test;
set global tencent_root_whitelist=concat(@@tencent_root_whitelist,"");
include/assert.inc [Cdb_tencentroot_set_counts = 1]
include/assert.inc [Cdb_show_tencentroot_counts = 4]
use test;
create table t(id INT);
show global status like "Cdb_returning_counts";
Variable_name	Value
Cdb_returning_counts	0
include/assert.inc [Cdb_returning_counts = 0]
insert into t (id) values (1) returning *;
id
1
replace into t (id) values (2) returning *;
id
2
delete from t returning *;
id
1
2
include/assert.inc [Cdb_returning_counts = 2]
drop table t;
include/assert.inc [Cdb_forbid_mysql_write_counts = 0]
SET GLOBAL cdb_forbid_mysql_write=on;
use mysql;
create table tbl_fail(id int);
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
use mysql;
create table tbl(c1 int primary key, c2 int);
insert into tbl values(1,1);
update tbl set c2 = 2 where c1 = 1;
replace into tbl values(1, 3);
delete from tbl where c1 = 1;
SET GLOBAL cdb_forbid_mysql_write=off;
commit;
include/assert.inc [Cdb_forbid_mysql_write_counts = 1]
drop table tbl;
include/assert.inc [Cdb_forbid_system_database_write_counts = 0]
SET GLOBAL cdb_forbid_system_database_write=ON;
use sys;
create table tbl(c1 int primary key, c2 int);
insert into tbl values(1,1);
update tbl set c2 = 2 where c1 = 1;
replace into tbl values(1, 3);
delete from tbl where c1 = 1;
use sys;
create table tbl_fail(id int);
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
SET GLOBAL cdb_forbid_system_database_write=OFF;
commit;
include/assert.inc [Cdb_forbid_system_database_write_counts = 1]
drop table tbl;
include/assert.inc [Cdb_working_mode_counts = 0]
set global cdb_working_mode_default= READONLY;
set global cdb_workmode_life_interval=100000000;
set global cdb_working_mode_enabled= ON;
use test;
create table tbl(a int, b int, c int);
insert into tbl values(1,2,3);
select * from tbl;
a	b	c
1	2	3
insert into tbl values(2);
ERROR HY000: The MySQL server is running with the root option so it cannot execute this statement
set global cdb_working_mode_default= READWRITE;
set global cdb_workmode_life_interval=100000000;
set global cdb_working_mode_enabled= OFF;
commit;
include/assert.inc [Cdb_working_mode_counts = 1]
drop table tbl;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

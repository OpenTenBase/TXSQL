SET @old_global_isolation= @@global.transaction_isolation;
SET @old_innodb_use_trx_lock= @@global.innodb_use_trx_lock;
SET @old_innodb_lock_wait_timeout= @@global.innodb_lock_wait_timeout;
SET GLOBAL TRANSACTION ISOLATION LEVEL READ COMMITTED;
SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED;
SET GLOBAL INNODB_USE_TRX_LOCK = ON;
SET GLOBAL innodb_lock_wait_timeout = 1;
SET SESSION innodb_lock_wait_timeout = 1;
#
#Case 1:
#This case is derived from virtual_debug.test
#It is used to test online DDL with DML using trx lock.
#During DDL, we need to build old versions.
#
CREATE TABLE t (a INT, b INT);
INSERT INTO t VALUES (11, 3);
INSERT INTO t VALUES (18, 1);
SET DEBUG_SYNC = 'innodb_inplace_alter_table_enter SIGNAL start_create WAIT_FOR go_ahead';
CREATE INDEX idx ON t(b);;
SET DEBUG_SYNC = 'now WAIT_FOR start_create';
update t set a=0 where a = 11;
SET DEBUG_SYNC = 'now SIGNAL go_ahead';
SELECT b FROM t;
b
1
3
SHOW CREATE TABLE t;
Table	Create Table
t	CREATE TABLE `t` (
  `a` int DEFAULT NULL,
  `b` int DEFAULT NULL,
  KEY `idx` (`b`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
SELECT * FROM t;
a	b
0	3
18	1
DROP TABLE t;
#
#Case 2:
#This case is used to test semi-consistent read for trx lock,
#which is derived from innodb-semi-consistent.test
#
drop table if exists t1;
create table t1(a int not null) engine=innodb DEFAULT CHARSET=latin1;
insert into t1 values (1),(2),(3),(4),(5),(6),(7);
begin;
update t1 set a=10 where a=5;
begin;
select * from t1 where a=2 for update;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
select * from t1 where a=2 limit 1 for update;
a
2
update t1 set a=11 where a=6;
update t1 set a=12 where a=2;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
update t1 set a=13 where a=1;
commit;
update t1 set a=14 where a=1;
commit;
select * from t1;
a
10
11
13
2
3
4
7
set autocommit=0;
delete from t1;
insert into t1 values (1),(2),(3),(4),(5),(6),(7);
create table t2 as select 1 as col;
commit;
select * from t1 where a=5 lock in share mode;
a
5
set autocommit=0;
update t1 straight_join t2 set a=10 where a=5;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
commit;
commit;
flush status;
update t1 straight_join t2 set a=10 where a=5;
select * from t1 where a=2 for update;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
select * from t1 where a=2 limit 1 for update;
a
2
update t1 straight_join t2 set a=11 where a=6;
update t1 straight_join t2 set a=12 where a=2;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
update t1 straight_join t2 set a=13 where a=1;
commit;
update t1 straight_join t2 set a=14 where a=1;
commit;
drop table t1, t2;
#
#Case 3:
#This case is used to test semi-consistent read and lock upgrade for trx lock,
#which is derived from unsafe_binlog_innodb.test
#
create table t1(a int not null, b int, primary key(a));
insert into t1 values(1,1),(2,2),(3,1),(4,2),(5,1),(6,2),(7,3);
set autocommit = 0;
select * from t1 lock in share mode;
a	b
1	1
2	2
3	1
4	2
5	1
6	2
7	3
update t1 set b = 5 where b = 1;
set autocommit = 0;
select * from t1 where a = 2 and b = 2 for update;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
commit;
commit;
set autocommit = 0;
update t1 set b = 5 where b = 1;
select * from t1 where a = 7 and b = 3 for update;
a	b
7	3
commit;
commit;
drop table t1;
#
#Case 4:
#This case is used to test secondary index lock for trx lock,
#which is derived from lock_impl_to_expl.test
#
CREATE TABLE t1(
id INT NOT NULL,
c1 INT NOT NULL,
c2 INT NOT NULL,
PRIMARY KEY (id DESC),
UNIQUE KEY(c1)
) Engine=InnoDB;
INSERT INTO t1 (id,c1,c2) VALUES (0,0,0),(1,1,1),(3,3,3);
BEGIN;
SELECT 1 FROM t1 WHERE c1=1 FOR SHARE;
1
1
SET autocommit = 1;
UPDATE t1 SET c2 = c2 + 1 WHERE id = 1;
SELECT * FROM t1 WHERE c2=1 FOR SHARE;
id	c1	c2
DELETE FROM t1 WHERE id=1;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
COMMIT;
ALTER TABLE t1 ADD INDEX idx(c2);
BEGIN;
SELECT * FROM t1 WHERE c1=1 FOR SHARE;
id	c1	c2
1	1	2
SET autocommit = 1;
UPDATE t1 SET c2 = c2 + 1 WHERE id = 1;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
SELECT 1 FROM t1 WHERE c2=1 FOR UPDATE;
1
COMMIT;
drop table t1;
#
#Case 5:
#This case is used to test transaction isolation for trx lock,
#when select twice in the same transaction.
#
CREATE TABLE t1 (c1 INT , c2 CHAR(10), PRIMARY KEY (c1)) ENGINE = InnoDB;
INSERT INTO t1 VALUES(0, "0");
INSERT INTO t1 VALUES(1, "1");
INSERT INTO t1 VALUES(2, "2");
INSERT INTO t1 VALUES(3, "3");
UPDATE t1 SET c1 = c1 + 100;
BEGIN;
SELECT * FROM t1;
c1	c2
100	0
101	1
102	2
103	3
BEGIN;
SELECT * FROM t1;
c1	c2
100	0
101	1
102	2
103	3
UPDATE t1 SET c1 = c1 + 100;
SELECT * FROM t1;
c1	c2
200	0
201	1
202	2
203	3
COMMIT;
SELECT * FROM t1;
c1	c2
200	0
201	1
202	2
203	3
COMMIT;
DROP TABLE t1;
#
#Case 6:
#This case is used to test data_lock_waits and innodb_trx for trx lock.
#
CREATE TABLE t1(
id INT PRIMARY KEY
) Engine=InnoDB;
INSERT INTO t1 VALUES (1);
#Case 6.1:
BEGIN;
SELECT * FROM t1 WHERE id=1 FOR SHARE;
id
1
BEGIN;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL C1_will_wait';
SELECT * FROM t1 WHERE id=1 FOR UPDATE;
CREATE TABLE data_lock_wait_t (
id INT PRIMARY KEY AUTO_INCREMENT,
lock_id VARCHAR(200) NOT NULL
) Engine=InnoDB;
CREATE TABLE innodb_trx_t (
id INT PRIMARY KEY AUTO_INCREMENT,
lock_id VARCHAR(200) NOT NULL
) Engine=InnoDB;
SET DEBUG_SYNC=
'now WAIT_FOR C1_will_wait';
SELECT COUNT(*) FROM performance_schema.data_lock_waits;
COUNT(*)
1
SELECT COUNT(*) FROM information_schema.innodb_trx where trx_state = "LOCK WAIT";
COUNT(*)
1
INSERT INTO data_lock_wait_t (lock_id)
SELECT REQUESTING_ENGINE_LOCK_ID
FROM performance_schema.data_lock_waits;
INSERT INTO innodb_trx_t (lock_id)
SELECT trx_requested_lock_id
FROM information_schema.innodb_trx where trx_state = "LOCK WAIT";
SELECT COUNT(data_lock_wait_t.id)
FROM data_lock_wait_t
JOIN innodb_trx_t dlw
ON(dlw.lock_id = data_lock_wait_t.lock_id)
ORDER BY data_lock_wait_t.id;
COUNT(data_lock_wait_t.id)
1
ROLLBACK;
DROP TABLE t1, data_lock_wait_t, innodb_trx_t;
#
#Case 7:
#Test different transaction isolation levels for trx
#lock with skip lock no wait.
#
SET SESSION innodb_lock_wait_timeout=1;
#
# Test READ UNCOMMITTED
#
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
SET SESSION TRANSACTION ISOLATION LEVEL READ UNCOMMITTED;
BEGIN;
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 AND seat_id >= 1 LIMIT 2 FOR SHARE;
seat_id	state	ST_AsText(pos)
1	0	POINT(1 0)
3	0	POINT(2 0)
SET SESSION TRANSACTION ISOLATION LEVEL READ UNCOMMITTED;
BEGIN;
INSERT INTO t1 VALUES(2,ST_PointFromText('POINT(2 1)'),0);
SELECT seat_id, state, ST_AsText(pos) FROM t1 WHERE state = 0
FOR UPDATE NOWAIT;
SELECT seat_id, state, ST_AsText(pos) FROM t1 WHERE state = 0
FOR UPDATE SKIP LOCKED;
seat_id	state	ST_AsText(pos)
2	0	POINT(2 1)
4	0	POINT(1 1)
6	0	POINT(2 1)
SET @g = ST_GeomFromText('POLYGON((0 0,0 3,3 3,0 3,0 0))');
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 AND MBRWithin(pos, @g) LIMIT 2 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 AND MBRWithin(pos, @g) LIMIT 2 FOR UPDATE SKIP LOCKED;
seat_id	state	ST_AsText(pos)
2	0	POINT(2 1)
4	0	POINT(1 1)
COMMIT;
COMMIT;
DROP TABLE t1;
#
# Test READ COMMITTED
#
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
SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED;
BEGIN;
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 AND seat_id >= 1 LIMIT 2 FOR SHARE;
seat_id	state	ST_AsText(pos)
1	0	POINT(1 0)
3	0	POINT(2 0)
SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED;
BEGIN;
INSERT INTO t1 VALUES(2,ST_PointFromText('POINT(2 1)'),0);
SELECT seat_id, state, ST_AsText(pos) FROM t1 WHERE state = 0
FOR UPDATE NOWAIT;
SELECT seat_id, state, ST_AsText(pos) FROM t1 WHERE state = 0
FOR UPDATE SKIP LOCKED;
seat_id	state	ST_AsText(pos)
2	0	POINT(2 1)
4	0	POINT(1 1)
6	0	POINT(2 1)
SET @g = ST_GeomFromText('POLYGON((0 0,0 3,3 3,0 3,0 0))');
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 AND MBRWithin(pos, @g) LIMIT 2 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 AND MBRWithin(pos, @g) LIMIT 2 FOR UPDATE SKIP LOCKED;
seat_id	state	ST_AsText(pos)
2	0	POINT(2 1)
4	0	POINT(1 1)
COMMIT;
COMMIT;
DROP TABLE t1;
#
#Case 8:
#Test two deadlock on lock upgrade scenarios for trx lock.
#
CREATE TABLE t(
id INT NOT NULL PRIMARY KEY
) ENGINE=InnoDB;
INSERT INTO t (id) VALUES (1), (2);
#Case 8.1:
BEGIN;
SELECT id FROM t WHERE id=1 FOR SHARE;
id
1
SELECT id FROM t WHERE id=2 FOR UPDATE;
id
2
SET DEBUG_SYNC=
'lock_wait_will_wait SIGNAL waiter_will_wait';
SELECT id FROM t WHERE id = 1 FOR UPDATE;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_will_wait';
SELECT * FROM t FOR UPDATE;
id
1
2
COMMIT;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
#Case 8.2:
BEGIN;
SELECT id FROM t WHERE id=1 FOR UPDATE;
id
1
SELECT id FROM t WHERE id=2 FOR UPDATE;
id
2
SET DEBUG_SYNC=
'lock_wait_will_wait SIGNAL waiter_will_wait';
SELECT id FROM t WHERE id = 1 FOR UPDATE;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_will_wait';
SELECT * FROM t FOR UPDATE;
id
1
2
COMMIT;
id
1
DROP TABLE t;
#
#Case 9:
#Some basic concurrent conflict and deadlock detection tests for trx lock.
#
CREATE TABLE t1(
id INT NOT NULL PRIMARY KEY, val int
) ENGINE=InnoDB;
INSERT INTO t1 VALUES (1, 1), (2, 2);
#Case 9.1:
set autocommit = 0;
select * from t1 where id = 1 lock in share mode;
id	val
1	1
update t1 set val = val + 1 where id = 1;
set autocommit = 0;
select * from t1 where id = 1 lock in share mode;;
COMMIT;
id	val
1	2
COMMIT;
#Case 9.2:
select * from t1 where id = 1 lock in share mode;
id	val
1	2
update t1 set val = val + 1 where id = 1;
select * from t1 where id = 1 lock in share mode;;
set autocommit = 0;
update t1 set val = val + 1 where id = 1;;
COMMIT;
id	val
1	3
COMMIT;
COMMIT;
#Case 9.3:
select * from t1 where id = 1 lock in share mode;
id	val
1	4
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL CON2_will_wait';
update t1 set val = val + 1 where id = 1;;
SET DEBUG_SYNC=
'now WAIT_FOR CON2_will_wait';
update t1 set val = val + 1 where id = 1;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
COMMIT;
#Case 9.4:
select * from t1 where id = 1 lock in share mode;
id	val
1	5
select * from t1 where id = 1 lock in share mode;
id	val
1	5
select * from t1 where id = 1 lock in share mode;
id	val
1	5
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL CON1_will_wait';
update t1 set val = val + 1 where id = 1;;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL CON2_will_wait WAIT_FOR CON1_will_wait';
update t1 set val = val + 1 where id = 1;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
commit;
SET DEBUG_SYNC=
'now WAIT_FOR CON2_will_wait';
update t1 set val = val + 1 where id = 1;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
commit;
rollback;
select * from t1;
id	val
1	5
2	2
SET DEBUG_SYNC= 'RESET';
#Case 9.5:
BEGIN;
update t1 set val = val + 1 where id = 1;
BEGIN;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_wait';
update t1 set val = val + 1 where id = 1;;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_wait';
COMMIT;
COMMIT;
#Case 9.6:
BEGIN;
update t1 set val = val + 1 where id = 1;
BEGIN;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_wait';
select * from t1 where id = 1 for share;;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_wait';
COMMIT;
id	val
1	8
COMMIT;
#Case 9.7:
BEGIN;
select * from t1 where id = 1 for share;
id	val
1	8
BEGIN;
select * from t1 where id = 1 for share;
id	val
1	8
BEGIN;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_wait';
update t1 set val = val + 1 where id = 1;;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_wait';
COMMIT;
COMMIT;
COMMIT;
#Case 9.8:
BEGIN;
select * from t1 where id = 1 for share;
id	val
1	9
BEGIN;
select * from t1 where id = 1 for share;
id	val
1	9
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_wait';
update t1 set val = val + 1 where id = 1;;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_wait';
COMMIT;
COMMIT;
#Case 9.9:
BEGIN;
select * from t1 where id = 1 for update;
id	val
1	10
BEGIN;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_wait';
update t1 set val = val + 1 where id = 1;;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_wait';
update t1 set val = val + 1 where id = 1;
COMMIT;
COMMIT;
#Case 9.10:
BEGIN;
select * from t1 where id = 1 lock in share mode;
id	val
1	12
BEGIN;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_wait';
update t1 set val = val + 1 where id = 1;;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_wait';
update t1 set val = val + 1 where id = 1;
COMMIT;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
COMMIT;
#Case 9.11:
DROP TABLE t1;
CREATE TABLE t1 (a INT PRIMARY KEY, b INT NOT NULL) ENGINE=InnoDB;
INSERT INTO t1 VALUES(3,1);
BEGIN;
INSERT INTO t1 VALUES(3,2);
ERROR 23000: Duplicate entry '3' for key 't1.PRIMARY'
BEGIN;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_wait';
SELECT * FROM t1 FOR UPDATE;;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_wait';
INSERT INTO t1 VALUES(3,3) ON DUPLICATE KEY UPDATE b=b+10;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
COMMIT;
a	b
3	1
COMMIT;
#Case 9.12:
set autocommit = 0;
delete from t1;
COMMIT;
insert into t1 values(1, 100);
set autocommit = 0;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_will_wait';
insert into t1 values(1, 100);;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_will_wait';
commit;
ERROR 23000: Duplicate entry '1' for key 't1.PRIMARY'
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL holder_will_wait';
select * from t1 where a = 1 for update;;
SET DEBUG_SYNC=
'now WAIT_FOR holder_will_wait';
select * from t1 where a = 1 for update;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
COMMIT;
a	b
1	100
COMMIT;
#Case 9.13:
delete from t1;
insert into t1 values(1, 100);
select * from t1 where a = 1 for share;
a	b
1	100
set autocommit = 0;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_will_wait';
select * from t1 where a = 1 for update;
set autocommit = 0;
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter2_will_wait WAIT_FOR waiter_will_wait';
select * from t1 where a = 1 for update;
SET DEBUG_SYNC=
'now WAIT_FOR waiter2_will_wait';
COMMIT;
a	b
1	100
COMMIT;
a	b
1	100
COMMIT;
#Case 9.14:
select * from t1 where a = 1 for share;
a	b
1	100
set autocommit = 0;
select * from t1 where a = 1 for share;
a	b
1	100
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL waiter_will_wait';
select * from t1 where a = 1 for update;
SET DEBUG_SYNC=
'now WAIT_FOR waiter_will_wait';
COMMIT;
a	b
1	100
COMMIT;
Case 9.15
DROP TABLE t1;
CREATE TABLE t1(a blob,b blob,c blob,d blob,e blob,f blob,g blob,
h blob,i blob,j blob,k blob,l blob,m blob,n blob, o blob,p blob,q blob,r blob,s blob,t blob,u blob) ENGINE=InnoDB ROW_FORMAT=COMPRESSED;
SET @a = repeat('a', 767);
SET @b = repeat('b', 767);
SET @c = repeat('c', 767);
SET @d = repeat('d', 767);
SET @e = repeat('e', 767);
INSERT INTO t1 VALUES (@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a,@a);
UPDATE t1 SET a=@b,b=@b,c=@b,d=@b,e=@b,f=@b,g=@b,h=@b,i=@b,j=@b,
k=@b,l=@b,m=@b,n=@b,o=@b,p=@b,q=@b,r=@b,s=@b,t=@b,u=@b;
DROP TABLE t1;
#
#Case 10:
#This case is derived from innodb-lock-inherit-read_commited.test
#to test Bug #21025880	DUPLICATE UK VALUES IN READ-COMMITTED for trx lock.
#
CREATE TABLE t1 (
a INT NOT NULL,
b INT NOT NULL,
PRIMARY KEY(b),
UNIQUE KEY(a))
ENGINE=INNODB;
SET @old_innodb_stats_auto_recalc = @@innodb_stats_auto_recalc;
SET GLOBAL innodb_stats_auto_recalc = OFF;
SET GLOBAL innodb_purge_stop_now = ON;
INSERT INTO t1 VALUES (1,1),(2,2);
DELETE FROM t1;
SET debug_sync = 'row_ins_sec_index_entry_dup_locks_created SIGNAL
  con1_locks_done WAIT_FOR con1_go';
SET debug_sync = 'ha_commit_trans_after_acquire_commit_lock SIGNAL
  con1_insert_done WAIT_FOR con1_finish';
REPLACE INTO t1 VALUES (1,2);
SET debug_sync = 'now WAIT_FOR con1_locks_done';
SET debug_sync = 'lock_wait_suspend_thread_enter SIGNAL con2_blocked
  WAIT_FOR con2_go';
SET debug_sync = 'ha_commit_trans_after_acquire_commit_lock SIGNAL
  con2_insert_done WAIT_FOR con2_finish';
SET debug_sync = 'ib_after_row_insert SIGNAL con2_insert_done';
REPLACE INTO t1 VALUES (1,3);
SET debug_sync = 'now WAIT_FOR con2_blocked';
SET GLOBAL innodb_purge_run_now=ON;
SET debug_sync = 'now SIGNAL con2_go WAIT_FOR con2_insert_done';
SET debug_sync = 'now SIGNAL con1_go WAIT_FOR con1_insert_done';
SET debug_sync = 'now SIGNAL con1_finish';
SET debug_sync = 'now SIGNAL con2_finish';
SELECT * FROM t1;
a	b
1	2
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
DROP TABLE t1;
SET GLOBAL innodb_stats_auto_recalc = @old_innodb_stats_auto_recalc;
SET DEBUG_SYNC= 'RESET';
CREATE TABLE t1 (
a INT NOT NULL,
b INT NOT NULL,
PRIMARY KEY(b),
UNIQUE KEY(a))
ENGINE=INNODB;
SET @old_innodb_stats_auto_recalc = @@innodb_stats_auto_recalc;
SET GLOBAL innodb_stats_auto_recalc = OFF;
SET GLOBAL innodb_purge_stop_now = ON;
INSERT INTO t1 VALUES (1,1),(2,2);
DELETE FROM t1;
SET debug_sync = 'row_ins_sec_index_entry_dup_locks_created SIGNAL
  con1_locks_done WAIT_FOR con1_go';
SET debug_sync = 'ha_commit_trans_after_acquire_commit_lock SIGNAL
  con1_insert_done WAIT_FOR con1_finish';
INSERT INTO t1 values (1,2) ON DUPLICATE KEY UPDATE a=2;
SET debug_sync = 'now WAIT_FOR con1_locks_done';
SET debug_sync = 'lock_wait_suspend_thread_enter SIGNAL con2_blocked
  WAIT_FOR con2_go';
SET debug_sync = 'ha_commit_trans_after_acquire_commit_lock SIGNAL
  con2_insert_done WAIT_FOR con2_finish';
SET debug_sync = 'ib_after_row_insert SIGNAL con2_insert_done';
REPLACE INTO t1 VALUES (1,3);
SET debug_sync = 'now WAIT_FOR con2_blocked';
SET GLOBAL innodb_purge_run_now=ON;
SET debug_sync = 'now SIGNAL con2_go WAIT_FOR con2_insert_done';
SET debug_sync = 'now SIGNAL con1_go WAIT_FOR con1_insert_done';
SET debug_sync = 'now SIGNAL con1_finish';
SET debug_sync = 'now SIGNAL con2_finish';
SELECT * FROM t1;
a	b
1	2
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
DROP TABLE t1;
SET GLOBAL innodb_stats_auto_recalc = @old_innodb_stats_auto_recalc;
SET DEBUG_SYNC= 'RESET';
CREATE TABLE t1 (
a INT NOT NULL,
b INT NOT NULL,
PRIMARY KEY(b),
UNIQUE KEY(a))
ENGINE=INNODB;
SET @old_innodb_stats_auto_recalc = @@innodb_stats_auto_recalc;
SET GLOBAL innodb_stats_auto_recalc = OFF;
SET GLOBAL innodb_purge_stop_now = ON;
INSERT INTO t1 VALUES (1,1),(2,2);
DELETE FROM t1;
SET debug_sync = 'row_ins_sec_index_entry_dup_locks_created SIGNAL
  con1_locks_done WAIT_FOR con1_go';
SET debug_sync = 'ha_commit_trans_after_acquire_commit_lock SIGNAL
  con1_insert_done WAIT_FOR con1_finish';
INSERT INTO t1 VALUES (1,2);
SET debug_sync = 'now WAIT_FOR con1_locks_done';
SET debug_sync = 'lock_wait_suspend_thread_enter SIGNAL con2_blocked
  WAIT_FOR con2_go';
SET debug_sync = 'ha_commit_trans_after_acquire_commit_lock SIGNAL
  con2_insert_done WAIT_FOR con2_finish';
SET debug_sync = 'ib_after_row_insert SIGNAL con2_insert_done';
REPLACE INTO t1 VALUES (1,3);
SET debug_sync = 'now WAIT_FOR con2_blocked';
SET GLOBAL innodb_purge_run_now=ON;
SET debug_sync = 'now SIGNAL con2_go WAIT_FOR con2_insert_done';
SET debug_sync = 'now SIGNAL con1_go WAIT_FOR con1_insert_done';
SET debug_sync = 'now SIGNAL con1_finish';
SET debug_sync = 'now SIGNAL con2_finish';
SELECT * FROM t1;
a	b
1	2
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
DROP TABLE t1;
SET GLOBAL innodb_stats_auto_recalc = @old_innodb_stats_auto_recalc;
SET DEBUG_SYNC= 'RESET';
#
#Case 11:
#This case is derived from skip_locked_nowait.test
#to test skip lock and no wait for trx lock.
#
SET SESSION innodb_lock_wait_timeout=1;
# Case 11.1: Test primary index
CREATE TABLE t1(
seat_id    INT,
state      INT,
PRIMARY KEY(seat_id)
) ENGINE=InnoDB;
INSERT INTO t1 VALUES(1,0), (2,0), (3,0), (4,0);
BEGIN;
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR SHARE;
seat_id	state
1	0
2	0
BEGIN;
SELECT * FROM t1 WHERE state = 0 LIMIT 2 LOCK IN SHARE MODE;
seat_id	state
1	0
2	0
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR SHARE;
seat_id	state
1	0
2	0
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR SHARE NOWAIT;
seat_id	state
1	0
2	0
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR SHARE SKIP LOCKED;
seat_id	state
1	0
2	0
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR UPDATE;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR UPDATE SKIP LOCKED;
seat_id	state
3	0
4	0
SELECT * FROM t1 WHERE seat_id > 0 LIMIT 2 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE seat_id > 0 LIMIT 2 FOR UPDATE SKIP LOCKED;
seat_id	state
3	0
4	0
COMMIT;
select count(*) from information_schema.innodb_trx;
count(*)
1
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR UPDATE;
seat_id	state
1	0
2	0
SELECT * FROM t1 WHERE state = 0 LIMIT 2 LOCK IN SHARE MODE;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR SHARE;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR SHARE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR SHARE SKIP LOCKED;
seat_id	state
3	0
4	0
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR UPDATE;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE state = 0 LIMIT 2 FOR UPDATE SKIP LOCKED;
seat_id	state
3	0
4	0
SELECT * FROM t1 WHERE seat_id > 0 LIMIT 2 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE seat_id > 0 LIMIT 2 FOR UPDATE SKIP LOCKED;
seat_id	state
3	0
4	0
COMMIT;
COMMIT;
DROP TABLE t1;
# Case 11.2: Test primary index & secondary index
CREATE TABLE t1(
seat_id    INT,
row_id     INT,
state      INT,
PRIMARY KEY(seat_id),
KEY(row_id)
) ENGINE=InnoDB;
INSERT INTO t1 VALUES(1,1,0), (2,1,0), (3,2,0), (4,2,0);
# Test secondary key
BEGIN;
SELECT * FROM t1 WHERE state = 0 AND row_id = 1 LIMIT 1 FOR UPDATE NOWAIT;
seat_id	row_id	state
1	1	0
BEGIN;
SELECT * FROM t1 WHERE state = 0 AND row_id = 1 LIMIT 1 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE state = 0 AND row_id = 1 LIMIT 1 FOR UPDATE SKIP LOCKED;
seat_id	row_id	state
2	1	0
SELECT * FROM t1 WHERE state = 0 AND row_id > 0 LIMIT 1 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE state = 0 AND row_id > 0 LIMIT 1 FOR UPDATE SKIP LOCKED;
seat_id	row_id	state
2	1	0
SELECT * FROM t1 WHERE state = 0 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE state = 0 FOR UPDATE SKIP LOCKED;
seat_id	row_id	state
2	1	0
3	2	0
4	2	0
COMMIT;
COMMIT;
BEGIN;
SELECT * FROM t1 WHERE seat_id = 1 FOR UPDATE NOWAIT;
seat_id	row_id	state
1	1	0
BEGIN;
SELECT * FROM t1 WHERE state = 0 AND row_id = 1 LIMIT 1 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE state = 0 AND row_id = 1 LIMIT 1 FOR UPDATE SKIP LOCKED;
seat_id	row_id	state
2	1	0
SELECT * FROM t1 WHERE state = 0 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT * FROM t1 WHERE state = 0 FOR UPDATE SKIP LOCKED;
seat_id	row_id	state
2	1	0
3	2	0
4	2	0
COMMIT;
COMMIT;
DROP TABLE t1;
# Case 11.3: Test primary index & spatial index
CREATE TABLE t1(
seat_id    INT,
pos        POINT NOT NULL SRID 0,
state      INT,
PRIMARY KEY(seat_id),
SPATIAL KEY(pos)
) ENGINE=InnoDB;
INSERT INTO t1 VALUES
(1,ST_PointFromText('POINT(1 0)'),0),
(2,ST_PointFromText('POINT(1 1)'),0),
(3,ST_PointFromText('POINT(2 0)'),0),
(4,ST_PointFromText('POINT(2 1)'),0),
(5,ST_PointFromText('POINT(3 0)'),0),
(6,ST_PointFromText('POINT(3 1)'),0);
BEGIN;
SET @g = ST_GeomFromText('POLYGON((0 0,0 2,2 2,0 2,0 0))');
SELECT seat_id, state, ST_AsText(pos) FROM t1 FORCE INDEX (pos)
WHERE state = 0 AND MBRWithin(pos, @g) FOR UPDATE NOWAIT;
seat_id	state	ST_AsText(pos)
2	0	POINT(1 1)
BEGIN;
SET @g = ST_GeomFromText('POLYGON((0 0,0 4,4 4,0 4,0 0))');
SELECT seat_id, state, ST_AsText(pos) FROM t1 FORCE INDEX (pos)
WHERE state = 0 AND MBRWithin(pos, @g) FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT seat_id, state, ST_AsText(pos) FROM t1 FORCE INDEX (pos)
WHERE state = 0 AND MBRWithin(pos, @g)  FOR UPDATE SKIP LOCKED;
seat_id	state	ST_AsText(pos)
6	0	POINT(3 1)
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 FOR UPDATE SKIP LOCKED;
seat_id	state	ST_AsText(pos)
5	0	POINT(3 0)
6	0	POINT(3 1)
COMMIT;
COMMIT;
SET @g = ST_GeomFromText('POLYGON((0 0,0 3,3 3,0 3,0 0))');
SELECT seat_id, state, ST_AsText(pos) FROM t1 FORCE INDEX (pos)
WHERE state = 0 AND MBRWithin(pos, @g) FOR UPDATE;
seat_id	state	ST_AsText(pos)
2	0	POINT(1 1)
4	0	POINT(2 1)
BEGIN;
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE seat_id = 4 FOR UPDATE NOWAIT;
seat_id	state	ST_AsText(pos)
4	0	POINT(2 1)
SELECT seat_id, state, ST_AsText(pos) FROM t1 FORCE INDEX (pos)
WHERE state = 0 AND MBRWithin(pos, @g) FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT seat_id, state, ST_AsText(pos) FROM t1 FORCE INDEX (pos)
WHERE state = 0 AND MBRWithin(pos, @g) FOR UPDATE SKIP LOCKED;
seat_id	state	ST_AsText(pos)
2	0	POINT(1 1)
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 FOR UPDATE NOWAIT;
ERROR HY000: Statement aborted because lock(s) could not be acquired immediately and NOWAIT is set.
SELECT seat_id, state, ST_AsText(pos) FROM t1
WHERE state = 0 FOR UPDATE SKIP LOCKED;
seat_id	state	ST_AsText(pos)
1	0	POINT(1 0)
2	0	POINT(1 1)
3	0	POINT(2 0)
5	0	POINT(3 0)
6	0	POINT(3 1)
COMMIT;
DROP TABLE t1;
SET @@global.innodb_lock_wait_timeout= @old_innodb_lock_wait_timeout;
SET @@global.innodb_use_trx_lock= @old_innodb_use_trx_lock;
SET @@global.transaction_isolation= @old_global_isolation;

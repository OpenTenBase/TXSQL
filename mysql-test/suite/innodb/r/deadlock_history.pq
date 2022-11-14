SET @old_deadlock_histroy_size = @@global.innodb_txsql_deadlock_history_size;
SET @old_deadlock_detect = @@global.innodb_deadlock_detect;
# 1. Historical deadlock cannot be opened when deadlock detect is turned off
DROP TABLE IF EXISTS t1;
Warnings:
Note	1051	Unknown table 'test.t1'
create table t1 (a int primary key ,b int);
insert into t1 values (1,2),(2,3),(3,4),(4,5),(5,6),(6,7),(7,8),(8,9),(9,10);
SET GLOBAL innodb_deadlock_detect = OFF;
SET GLOBAL innodb_txsql_deadlock_history_size = 2;
ERROR 42000: Variable 'innodb_txsql_deadlock_history_size' can't be set to the value of '2'
# 2. Test innodb_txsql_deadlock_history_size = 0
SET GLOBAL innodb_deadlock_detect = ON;
SET GLOBAL innodb_txsql_deadlock_history_size = 0;
begin;
select * from t1 where a = 1 for update;
a	b
1	2
begin;
select * from t1 where a = 2 for update;
a	b
2	3
select * from t1 where a = 2 for update;
select * from t1 where a = 1 for update;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
commit;
a	b
2	3
commit;
select * from information_schema.txsql_deadlock_history;
GROUP_ID	LOOP_ID	THREAD_ID	TRX_ID	LOCK_TYPE	LOCK_MODE	INDEX_NAME	TABLE_NAME	QUERY	LOCK_DATA	LOCK_WAIT_DATA	HOST	USER	TRX_START_TIME	DETECTION_TIME
# 3. Test record lock not gap
SET GLOBAL innodb_txsql_deadlock_history_size = 6;
begin;
select * from t1 where a = 1 for update;
a	b
1	2
begin;
select * from t1 where a = 2 for update;
a	b
2	3
select * from t1 where a = 2 for update;
select * from t1 where a = 1 for update;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
commit;
a	b
2	3
commit;
select * from information_schema.txsql_deadlock_history;
GROUP_ID	LOOP_ID	THREAD_ID	TRX_ID	LOCK_TYPE	LOCK_MODE	INDEX_NAME	TABLE_NAME	QUERY	LOCK_DATA	LOCK_WAIT_DATA	HOST	USER	TRX_START_TIME	DETECTION_TIME
1	0	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 2 for update	1,	2,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
1	1	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 1 for update	2,	1,	localhost	tencentroot	<TRX_START_TIME>	<DETECTION_TIME>
# 4. Test record lock gap
SET GLOBAL innodb_txsql_deadlock_history_size = 6;
begin;
select * from t1 where a < 3 for update;
a	b
1	2
2	3
begin;
select * from t1 where a > 5 for update;
a	b
6	7
7	8
8	9
9	10
select * from t1 where a < 8 for update;
select * from t1 where a > 1 for update;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
commit;
a	b
1	2
2	3
3	4
4	5
5	6
6	7
7	8
commit;
select * from information_schema.txsql_deadlock_history;
GROUP_ID	LOOP_ID	THREAD_ID	TRX_ID	LOCK_TYPE	LOCK_MODE	INDEX_NAME	TABLE_NAME	QUERY	LOCK_DATA	LOCK_WAIT_DATA	HOST	USER	TRX_START_TIME	DETECTION_TIME
1	0	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 2 for update	1,	2,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
1	1	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 1 for update	2,	1,	localhost	tencentroot	<TRX_START_TIME>	<DETECTION_TIME>
2	0	<THREAD_ID>	<TRX_ID>	RECORD	X	PRIMARY	test/t1	select * from t1 where a < 8 for update	1,2,3,4,5,	6,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
2	1	<THREAD_ID>	<TRX_ID>	RECORD	X	PRIMARY	test/t1	select * from t1 where a > 1 for update	supermum,6,7,8,9,	2,	localhost	tencentroot	<TRX_START_TIME>	<DETECTION_TIME>
# 5. Test exceeds innodb_txsql_deadlock_history_size
begin;
select * from t1 where a = 1 for update;
a	b
1	2
begin;
select * from t1 where a = 2 for update;
a	b
2	3
begin;
select * from t1 where a = 3 for update;
a	b
3	4
select * from t1 where a = 2 for update;
select * from t1 where a = 3 for update;
select * from t1 where a = 1 for update;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
commit;
a	b
3	4
commit;
a	b
2	3
commit;
select * from information_schema.txsql_deadlock_history;
GROUP_ID	LOOP_ID	THREAD_ID	TRX_ID	LOCK_TYPE	LOCK_MODE	INDEX_NAME	TABLE_NAME	QUERY	LOCK_DATA	LOCK_WAIT_DATA	HOST	USER	TRX_START_TIME	DETECTION_TIME
1	1	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 1 for update	2,	1,	localhost	tencentroot	<TRX_START_TIME>	<DETECTION_TIME>
2	0	<THREAD_ID>	<TRX_ID>	RECORD	X	PRIMARY	test/t1	select * from t1 where a < 8 for update	1,2,3,4,5,	6,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
2	1	<THREAD_ID>	<TRX_ID>	RECORD	X	PRIMARY	test/t1	select * from t1 where a > 1 for update	supermum,6,7,8,9,	2,	localhost	tencentroot	<TRX_START_TIME>	<DETECTION_TIME>
3	0	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 3 for update	2,	3,	localhost	tencentroot	<TRX_START_TIME>	<DETECTION_TIME>
3	1	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 1 for update	3,	1,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
3	2	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 2 for update	1,	2,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
# 6. Test resize the innodb_txsql_deadlock_history_size
SET GLOBAL innodb_txsql_deadlock_history_size = 2;
select * from information_schema.txsql_deadlock_history;
GROUP_ID	LOOP_ID	THREAD_ID	TRX_ID	LOCK_TYPE	LOCK_MODE	INDEX_NAME	TABLE_NAME	QUERY	LOCK_DATA	LOCK_WAIT_DATA	HOST	USER	TRX_START_TIME	DETECTION_TIME
3	1	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 1 for update	3,	1,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
3	2	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 2 for update	1,	2,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
SET GLOBAL innodb_txsql_deadlock_history_size = 6;
begin;
select * from t1 where a = 1 for update;
a	b
1	2
begin;
select * from t1 where a = 2 for update;
a	b
2	3
select * from t1 where a = 2 for update;
select * from t1 where a = 1 for update;
ERROR 40001: Deadlock found when trying to get lock; try restarting transaction
commit;
a	b
2	3
commit;
select * from information_schema.txsql_deadlock_history;
GROUP_ID	LOOP_ID	THREAD_ID	TRX_ID	LOCK_TYPE	LOCK_MODE	INDEX_NAME	TABLE_NAME	QUERY	LOCK_DATA	LOCK_WAIT_DATA	HOST	USER	TRX_START_TIME	DETECTION_TIME
3	1	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 1 for update	3,	1,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
3	2	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 2 for update	1,	2,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
4	0	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 2 for update	1,	2,	localhost	root	<TRX_START_TIME>	<DETECTION_TIME>
4	1	<THREAD_ID>	<TRX_ID>	RECORD	X,REC_NOT_GAP	PRIMARY	test/t1	select * from t1 where a = 1 for update	2,	1,	localhost	tencentroot	<TRX_START_TIME>	<DETECTION_TIME>
# 7. Test turn off deadlock detect
SET GLOBAL innodb_deadlock_detect = OFF;
select * from information_schema.txsql_deadlock_history;
GROUP_ID	LOOP_ID	THREAD_ID	TRX_ID	LOCK_TYPE	LOCK_MODE	INDEX_NAME	TABLE_NAME	QUERY	LOCK_DATA	LOCK_WAIT_DATA	HOST	USER	TRX_START_TIME	DETECTION_TIME
set global innodb_deadlock_detect = @old_deadlock_detect;
set global innodb_txsql_deadlock_history_size = @old_deadlock_histroy_size;
DROP TABLE t1;

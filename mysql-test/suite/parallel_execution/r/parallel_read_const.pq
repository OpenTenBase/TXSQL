show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,READ_MASK,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ``@`` TO `root`@`localhost` WITH GRANT OPTION
select user();
user()
root@localhost
create database test1;
create user 'user1'@'%';
grant all on *.* to 'user1'@'%';
CREATE TABLE t1 (
f1 varchar(16) collate latin1_swedish_ci PRIMARY KEY,
f2 varchar(16) collate latin1_swedish_ci
);
CREATE TABLE t2 (
f1 varchar(16) collate latin1_swedish_ci,
f3 varchar(16) collate latin1_swedish_ci, key(f1));
INSERT INTO t1 VALUES ('bla','blah');
INSERT INTO t1 VALUES ('blc','blad');
INSERT INTO t1 VALUES ('bld','blad');
INSERT INTO t2 VALUES ('bla','sheep');
INSERT INTO t2 VALUES ('blc','sheep');
INSERT INTO t2 VALUES ('bld','sheep');
INSERT INTO t2 VALUES ('bla','sheep2');
INSERT INTO t2 VALUES ('bla','sheep3');
ANALYZE TABLE t1,t2;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
# --include save_and_set_px.inc
explain format=tree SELECT * FROM t1 JOIN t2 USING(f1) WHERE f1='bla';
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel index lookup on t2 using f1 (f1='bla')  (cost=0.80 rows=3)

explain SELECT * FROM t1 JOIN t2 USING(f1) WHERE f1='bla';
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ref	f1	f1	19	const	3	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	3	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	3	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select 'bla' AS `f1`,'blah' AS `f2`,`test`.`t2`.`f3` AS `f3` from `test`.`t1` join `test`.`t2` where (`test`.`t2`.`f1` = 'bla')
SELECT * FROM t1 JOIN t2 USING(f1) WHERE f1='bla';
f1	f2	f3
bla	blah	sheep
bla	blah	sheep2
bla	blah	sheep3
# --include set_px_to_saved.inc
drop table t1,t2;
drop database test1;
drop user user1;

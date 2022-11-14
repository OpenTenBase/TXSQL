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
DROP TABLE IF EXISTS t1;
Warnings:
Note	1051	Unknown table 'test.t1'
CREATE TABLE t1(
id INT PRIMARY KEY AUTO_INCREMENT,
col1 VARCHAR(100),
col2 INT,
KEY idx_col2(col2)
)ENGINE = InnoDB;
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
CREATE TABLE t2 (
col_varchar_255_latin1_key varchar(255) DEFAULT NULL,
pk int(11) NOT NULL AUTO_INCREMENT,
KEY (pk DESC),
KEY k3 (col_varchar_255_latin1_key DESC)
);
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
INSERT INTO t2(col_varchar_255_latin1_key, pk) VALUES
('l',4), ('something',3), ('ycyoybhug',2), ('l',5), ('my',1),
('l',4),('l',4),('l',4),('l',4),('l',4),('l',4),
('l',4),('l',4),('l',4),('l',4),('l',4),('l',4),
('l',4),('l',4),('l',4),('l',4),('l',4),('l',4),
('l',4),('l',4),('l',4),('l',4),('l',4),('l',4);
ANALYZE TABLE t2;
Table	Op	Msg_type	Msg_text
test.t2	analyze	status	OK
# --include save_and_set_px.inc
EXPLAIN FORMAT=TREE SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 >= 5 AND col2 < 8 order by col2 DESC;
EXPLAIN
-> Filter: ((t1.col2 >= 5) and (t1.col2 < 8))  (cost=0.86 rows=3)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel covering index range scan on t1 using idx_col2 over (5 <= col2 < 8) (reverse)  (cost=0.86 rows=3)

SELECT id, col2 FROM t1 FORCE INDEX(idx_col2) WHERE col2 >= 5 AND col2 < 8 order by col2 DESC;
id	col2
4	7
5	6
6	5
explain FORMAT=tree select pk from t2 where pk <> 4 order by pk;
EXPLAIN
-> Filter: (t2.pk <> 4)  (cost=1.06 rows=4)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel covering index range scan on t2 using pk over (pk < 4) OR (4 < pk) (reverse)  (cost=1.06 rows=4)

select pk from t2 where pk <> 4 order by pk;
pk
1
2
3
5
# --include set_px_to_saved.inc
drop table t1;
drop table t2;
drop database test1;
drop user user1;

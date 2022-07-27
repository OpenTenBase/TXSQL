# Infrastructure for Enhanced Prepare once
show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,GROUP_REPLICATION_ADMIN,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PERSIST_RO_VARIABLES_ADMIN,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ''@'' TO 'root'@'localhost' WITH GRANT OPTION
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
# --include save_and_set_px.inc
set optimizer_switch='mrr=on,mrr_cost_based=off';
explain format=tree select * from t1 force index(idx_col2) where col2 > 4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel index range scan on t1 using idx_col2, with index condition: (t1.col2 > 4)  (cost=2.35 rows=6)

select * from t1 force index(idx_col2) where col2 > 4;
id	col1	col2
1	testtesttesttest	10
2	testtesttesttest	9
3	testtesttesttest	8
4	testtesttesttest	7
5	testtesttesttest	6
6	testtesttesttest	5
# --include set_px_to_saved.inc
drop table t1;
drop database test1;
drop user user1;

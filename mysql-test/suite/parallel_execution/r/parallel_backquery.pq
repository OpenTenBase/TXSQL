show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ``@`` TO `root`@`localhost` WITH GRANT OPTION
select user();
user()
root@localhost
create database test1;
create user 'user1'@'%';
grant all on *.* to 'user1'@'%';
create table t1(id int,c1 int);
insert into t1 values(1,1),(2,2),(3,3),(4,4),(5,5);
analyze table t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
# --include save_and_set_px.inc
set @saved_innodb_backquery_enable = @@global.innodb_backquery_enable;
set global innodb_backquery_enable=ON;
delete from t1 where id=5;
explain format=tree select * from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel table scan on t1  (cost=0.65 rows=4)

select * from t1;
id	c1
1	1
2	2
3	3
4	4
## select * from t1 as of timestamp 'time1'
EXPLAIN
-> Table scan on t1  (cost=0.65 rows=4)

id	c1
1	1
2	2
3	3
4	4
5	5
## date_sub
EXPLAIN
-> Table scan on t1  (cost=0.65 rows=4)

id	c1
1	1
2	2
3	3
4	4
# --include set_px_to_saved.inc
set global innodb_backquery_enable=@saved_innodb_backquery_enable;
drop table if exists t1;
drop database test1;
drop user user1;

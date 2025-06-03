create user 'test_user1'@'localhost';
create user 'test_user2'@'localhost';
show grants for 'test_user1'@'localhost';
Grants for test_user1@localhost
GRANT USAGE ON *.* TO `test_user1`@`localhost`
show grants for 'test_user2'@'localhost';
Grants for test_user2@localhost
GRANT USAGE ON *.* TO `test_user2`@`localhost`
## should fail
grant system_variables_admin on *.* to 'test_user1'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
grant session_variables_admin on *.* to 'test_user1'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
grant system_variables_admin on *.* to 'test_user2'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
grant session_variables_admin on *.* to 'test_user2'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
set session sql_log_bin=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
grant all privileges on *.* to 'test_user1'@'localhost';
show grants for 'test_user1'@'localhost';
Grants for test_user1@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `test_user1`@`localhost`
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,READ_MASK,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `test_user1`@`localhost`
flush privileges;
## should success
grant system_variables_admin on *.* to 'test_user1'@'localhost';
grant session_variables_admin on *.* to 'test_user1'@'localhost';
grant system_variables_admin on *.* to 'test_user2'@'localhost';
grant session_variables_admin on *.* to 'test_user2'@'localhost';
set session sql_log_bin=OFF;
set session sql_log_bin=ON;
revoke system_variables_admin on *.* from 'test_user1'@'localhost';
revoke session_variables_admin on *.* from 'test_user1'@'localhost';
revoke system_variables_admin on *.* from 'test_user2'@'localhost';
revoke session_variables_admin on *.* from 'test_user2'@'localhost';
show grants for 'test_user1'@'localhost';
Grants for test_user1@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `test_user1`@`localhost`
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,READ_MASK,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `test_user1`@`localhost`
show grants for 'test_user2'@'localhost';
Grants for test_user2@localhost
GRANT USAGE ON *.* TO `test_user2`@`localhost`
flush privileges;
## should success, because test_user1 has UPDATE privilege on mysql.*
grant system_variables_admin on *.* to 'test_user1'@'localhost';
grant session_variables_admin on *.* to 'test_user1'@'localhost';
grant system_variables_admin on *.* to 'test_user2'@'localhost';
grant session_variables_admin on *.* to 'test_user2'@'localhost';
revoke system_variables_admin on *.* from 'test_user1'@'localhost';
revoke session_variables_admin on *.* from 'test_user1'@'localhost';
revoke system_variables_admin on *.* from 'test_user2'@'localhost';
revoke session_variables_admin on *.* from 'test_user2'@'localhost';
show grants for 'test_user1'@'localhost';
Grants for test_user1@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `test_user1`@`localhost`
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,READ_MASK,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `test_user1`@`localhost`
show grants for 'test_user2'@'localhost';
Grants for test_user2@localhost
GRANT USAGE ON *.* TO `test_user2`@`localhost`
flush privileges;
grant system_variables_admin on *.* to 'test_user1'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
grant session_variables_admin on *.* to 'test_user1'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
grant system_variables_admin on *.* to 'test_user2'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
grant session_variables_admin on *.* to 'test_user2'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
revoke update on *.* from 'test_user1'@'localhost';
show grants for 'test_user1'@'localhost';
Grants for test_user1@localhost
GRANT SELECT, INSERT, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `test_user1`@`localhost`
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,READ_MASK,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `test_user1`@`localhost`
flush privileges;
## should fail because no UPDATE privilege on mysql.*
grant system_variables_admin on *.* to 'test_user1'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
grant session_variables_admin on *.* to 'test_user1'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
grant system_variables_admin on *.* to 'test_user2'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
grant session_variables_admin on *.* to 'test_user2'@'localhost';
ERROR 42000: Access denied; you need (at least one of) the GRANT OPTION privilege(s) for this operation
revoke system_variables_admin on *.* from 'test_user1'@'localhost';
revoke session_variables_admin on *.* from 'test_user1'@'localhost';
revoke system_variables_admin on *.* from 'test_user2'@'localhost';
revoke session_variables_admin on *.* from 'test_user2'@'localhost';
grant system_variables_admin on *.* to 'test_user1'@'localhost' with grant option;
grant session_variables_admin on *.* to 'test_user1'@'localhost' with grant option;
show grants for 'test_user1'@'localhost';
Grants for test_user1@localhost
GRANT SELECT, INSERT, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `test_user1`@`localhost`
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,READ_MASK,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `test_user1`@`localhost`
GRANT SESSION_VARIABLES_ADMIN,SYSTEM_VARIABLES_ADMIN ON *.* TO `test_user1`@`localhost` WITH GRANT OPTION
show grants for 'test_user2'@'localhost';
Grants for test_user2@localhost
GRANT USAGE ON *.* TO `test_user2`@`localhost`
flush privileges;
## should success because test_user1 has specific privileges
grant system_variables_admin on *.* to 'test_user2'@'localhost';
grant session_variables_admin on *.* to 'test_user2'@'localhost';
show grants for 'test_user2'@'localhost';
Grants for test_user2@localhost
GRANT USAGE ON *.* TO `test_user2`@`localhost`
GRANT SESSION_VARIABLES_ADMIN,SYSTEM_VARIABLES_ADMIN ON *.* TO `test_user2`@`localhost`
revoke system_variables_admin on *.* from 'test_user2'@'localhost';
revoke session_variables_admin on *.* from 'test_user2'@'localhost';
grant update on mysql.* to 'test_user1'@'localhost';
show grants for 'test_user2'@'localhost';
Grants for test_user2@localhost
GRANT USAGE ON *.* TO `test_user2`@`localhost`
flush privileges;
grant system_variables_admin on *.* to 'test_user2'@'localhost';
grant session_variables_admin on *.* to 'test_user2'@'localhost';
show grants for 'test_user2'@'localhost';
Grants for test_user2@localhost
GRANT USAGE ON *.* TO `test_user2`@`localhost`
GRANT SESSION_VARIABLES_ADMIN,SYSTEM_VARIABLES_ADMIN ON *.* TO `test_user2`@`localhost`
drop user 'test_user1'@'localhost';
drop user 'test_user2'@'localhost';

include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
[connection slave3]
# Set up and connect replication channel.
CHANGE MASTER TO master_host='127.0.0.1',master_port=MASTER_PORT,master_user='root';
Warnings:
Warning	1287	'CHANGE MASTER' is deprecated and will be removed in a future release. Please use CHANGE REPLICATION SOURCE instead
Warning	1287	'MASTER_HOST' is deprecated and will be removed in a future release. Please use SOURCE_HOST instead
Warning	1287	'MASTER_PORT' is deprecated and will be removed in a future release. Please use SOURCE_PORT instead
Warning	1287	'MASTER_USER' is deprecated and will be removed in a future release. Please use SOURCE_USER instead
Note	1759	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	1760	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
START SLAVE IO_THREAD;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
include/wait_for_slave_io_to_start.inc
[connection tencentroot]
# Slave (Server_id 2) reports default cdb_role.
# Slave (Server_id 3) reports explicitly configured cdb_role.
SHOW SLAVE HOSTS;
Server_id	Host	Port	Master_id	Slave_UUID	Role
2	127.0.0.1	SLAVE_PORT	1	SLAVE_UUID	CDB_ROLE_UNKNOWN
3	localhost	SLAVE_PORT	1	SLAVE_UUID	CDB_ROLE_SLAVE
Warning	1287	SLAVE_PORT
Warnings:
[connection slave3]
set global cdb_role = 3;
[connection tencentroot]
# Slave (Server_id 3) does not report reconfigured cdb_role without reconnecting.
SHOW SLAVE HOSTS;
Server_id	Host	Port	Master_id	Slave_UUID	Role
2	127.0.0.1	SLAVE_PORT	1	SLAVE_UUID	CDB_ROLE_UNKNOWN
3	localhost	SLAVE_PORT	1	SLAVE_UUID	CDB_ROLE_SLAVE
Warning	1287	SLAVE_PORT
Warnings:
[connection slave3]
STOP SLAVE IO_THREAD;
Warnings:
Warning	1287	'STOP SLAVE' is deprecated and will be removed in a future release. Please use STOP REPLICA instead
set global cdb_role = 1;
START SLAVE IO_THREAD;
Warnings:
Warning	1287	'START SLAVE' is deprecated and will be removed in a future release. Please use START REPLICA instead
[connection tencentroot]
# Slave (Server_id 3) reports reconfigured cdb_role after reconnecting.
SHOW SLAVE HOSTS;
Server_id	Host	Port	Master_id	Slave_UUID	Role
2	127.0.0.1	SLAVE_PORT	1	SLAVE_UUID	CDB_ROLE_UNKNOWN
3	localhost	SLAVE_PORT	1	SLAVE_UUID	CDB_ROLE_MASTER
Warning	1287	SLAVE_PORT
Warnings:
[connection slave3]
include/stop_slave_io.inc
[connection master]
include/rpl_end.inc

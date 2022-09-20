include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
set @old_snapshot_pct = @@innodb_buffer_pool_snapshot_pct;
SET GLOBAL innodb_buffer_pool_snapshot_pct = 100;
SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_transmit_status";
Variable_name	Value
Innodb_buffer_pool_transmit_status	ib_bp_info transmition not started
set @old_transmit_enabled = @@innodb_buffer_pool_transmit_enabled;
set @old_transmit_interval = @@innodb_buffer_pool_transmit_interval;
set @old_recover_after_transmit = @@innodb_buffer_pool_recover_after_transmit;
set @old_recover_pct = @@innodb_buffer_pool_recover_pct;
SET GLOBAL innodb_buffer_pool_recover_pct = 100;
SET GLOBAL innodb_buffer_pool_transmit_enabled = OFF;
# Check STOP REPLICA && START REPLICA when slave is on and enabled is off
STOP REPLICA;
START REPLICA;
SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_transmit_status";
Variable_name	Value
Innodb_buffer_pool_transmit_status	ib_bp_info transmition not started
# Check START REPLICA && STOP REPLICA when slave is off and enabled is off
STOP REPLICA;
SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_transmit_status";
Variable_name	Value
Innodb_buffer_pool_transmit_status	ib_bp_info transmition not started
START REPLICA;
STOP REPLICA;
SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_transmit_status";
Variable_name	Value
Innodb_buffer_pool_transmit_status	ib_bp_info transmition not started
# Set slave is on and enabled is on
SET GLOBAL innodb_buffer_pool_transmit_enabled = ON;
START REPLICA;
SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_transmit_status";
Variable_name	Value
Innodb_buffer_pool_transmit_status	slave_transmit thread for channel '': connected to master 'root@127.0.0.1:MASTER_PORT'
# Check STOP REPLICA && START REPLICA when slave is on and enabled is on
STOP REPLICA;
START REPLICA;
SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_transmit_status";
Variable_name	Value
Innodb_buffer_pool_transmit_status	slave_transmit thread for channel '': connected to master 'root@127.0.0.1:MASTER_PORT'
# check START REPLICA && STOP REPLICA when slave is off and enabled is on
STOP REPLICA;
SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_transmit_status";
Variable_name	Value
Innodb_buffer_pool_transmit_status	slave_transmit thread exited
START REPLICA;
STOP REPLICA;
SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_transmit_status";
Variable_name	Value
Innodb_buffer_pool_transmit_status	slave_transmit thread exited
START REPLICA;
CREATE TABLE ib_bp_test
(a INT AUTO_INCREMENT, b VARCHAR(64), c TEXT, PRIMARY KEY (a), KEY (b, c(128)))
ENGINE=INNODB;
SET GLOBAL innodb_buffer_pool_snapshot_now = ON;
SET GLOBAL innodb_buffer_pool_transmit_enabled = ON;
STOP REPLICA;
START REPLICA;
SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_transmit_status";
Variable_name	Value
Innodb_buffer_pool_transmit_status	slave_transmit thread for channel '': connected to master 'root@127.0.0.1:MASTER_PORT'
SET GLOBAL innodb_buffer_pool_transmit_interval = 5;

SHOW STATUS WHERE Variable_name = "innodb_buffer_pool_recover_status";
Variable_name	Value
Innodb_buffer_pool_recover_status	Recovering of buffer pool not started
SET GLOBAL innodb_buffer_pool_recover_after_transmit = ON;
SET GLOBAL innodb_buffer_pool_snapshot_now = ON;
SET GLOBAL innodb_buffer_pool_transmit_enabled = @old_transmit_enabled;
SET GLOBAL innodb_buffer_pool_transmit_interval = @old_transmit_interval;
SET GLOBAL innodb_buffer_pool_recover_after_transmit = @old_recover_after_transmit;
SET GLOBAL innodb_buffer_pool_recover_pct = @old_recover_pct;
SET GLOBAL innodb_buffer_pool_snapshot_pct = @old_snapshot_pct;
DROP TABLE ib_bp_test;
include/rpl_end.inc

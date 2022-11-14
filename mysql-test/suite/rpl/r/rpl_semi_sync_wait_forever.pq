include/rpl_init.inc [topology=1->2, 1->3]
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection server_1]
CALL mtr.add_suppression("Semi-sync master failed on net_flush().*");
CALL mtr.add_suppression('Timeout waiting for reply of binlog');
# Test forever on and off
include/install_semisync_source.inc
Warnings:
Note	1287	'rpl_semi_sync_master' is deprecated and will be removed in a future release. Please use rpl_semi_sync_source instead
SET GLOBAL rpl_semi_sync_master_enabled= OFF;
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
SET GLOBAL rpl_semi_sync_master_enabled= OFF;
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
Warnings:
Warning	168	Set wait forever is forbidden when semi-sync replication is disabled.
SET GLOBAL rpl_semi_sync_master_enabled= ON;
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
SET GLOBAL rpl_semi_sync_master_enabled= ON;
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
Warnings:
Warning	168	Set wait forever is forbidden when semi-sync replication is disabled.
# Start slave : server_2
[connection server_2]
include/install_semisync_replica.inc
Warnings:
Note	1287	'rpl_semi_sync_slave' is deprecated and will be removed in a future release. Please use rpl_semi_sync_replica instead
# Enable semisync on master
[connection server_1]
SET GLOBAL rpl_semi_sync_master_enabled= ON;
# Change master variables
SET GLOBAL rpl_semi_sync_master_timeout= 2000;
SET GLOBAL rpl_semi_sync_master_wait_no_slave= OFF;
include/rpl_sync.inc
# Verify ack_receiver thread is created
include/assert.inc [ack receiver thread is created;]
CREATE TABLE t1(c1 INT);
include/rpl_sync.inc
# Verify semisync replication works well
[connection server_1]
include/assert.inc [Semisync master is on]
# Test with wait for slave count = 1
SET GLOBAL rpl_semi_sync_master_wait_for_slave_count = 1;
# Test 1.1 : timeout without forever enabled
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
# Let server_2 wait before sending ack
[connection server_2]
SET GLOBAL debug = '+d,rpl_semisync_before_send_ack';
# Server_1 send INSERT (1)
[connection server_1]
INSERT INTO t1 VALUES(1);;
# Server_2 ack after sleeping 5s
[connection server_2]
SET DEBUG_SYNC = 'now SIGNAL continue';
# Server_1 wait timeout
[connection server_1]
# Check wait time
# Test 1.2 : no timeout with forever enabled
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
SELECT @@rpl_semi_sync_master_wait_forever;
@@rpl_semi_sync_master_wait_forever
1
# Server_1 send INSERT (2)
[connection server_1]
INSERT INTO t1 VALUES(2);;
# Server_2 ack after sleeping 5s
[connection server_2]
SET DEBUG_SYNC = 'now SIGNAL continue';
# Server_1 wait timeout
[connection server_1]
# Check wait time
# Test 1.3 : wakeup when switch forever off
# with rpl_semi_sync_master_wait_no_slave= ON
[connection server_1]
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
SELECT @@rpl_semi_sync_master_wait_forever;
@@rpl_semi_sync_master_wait_forever
1
# Server_1 send INSERT (3)
INSERT INTO t1 VALUES(3);;
# Server_2 ack after sleeping 5s
[connection server_2]
#  Switch forever off
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	OFF
# Server_1 wakeup
[connection server_1]
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	OFF
# Server_2 ack
[connection server_2]
SET DEBUG_SYNC = 'now SIGNAL continue';
# Check wait time
[connection server_1]
# Semi sync enabled again
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	ON
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
Warnings:
Warning	168	Set wait forever is forbidden when semi-sync replication is disabled.
# Test 1.4 : wakeup when switch forever off
# with rpl_semi_sync_master_wait_no_slave= ON
[connection server_1]
SET GLOBAL rpl_semi_sync_master_wait_no_slave= ON;
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
SELECT @@rpl_semi_sync_master_wait_forever;
@@rpl_semi_sync_master_wait_forever
1
# Server_1 send INSERT (4)
INSERT INTO t1 VALUES(4);;
# Server_2 ack after sleeping 5s
[connection server_2]
#  Switch forever off
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	OFF
# Server_1 wakeup
[connection server_1]
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	OFF
# Server_2 ack
[connection server_2]
SET DEBUG_SYNC = 'now SIGNAL continue';
SET GLOBAL debug = '-d,rpl_semisync_before_send_ack';
# Check wait time
[connection server_1]
# Semi sync still disabled
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	ON
SET GLOBAL rpl_semi_sync_master_wait_no_slave= OFF;
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
# Enable semi sync again
SET GLOBAL rpl_semi_sync_master_enabled= OFF;
SET GLOBAL rpl_semi_sync_master_enabled= ON;
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	ON
# Test 1.5 : wait again when timeout if switch forever on
[connection server_1]
SELECT @@rpl_semi_sync_master_wait_forever;
@@rpl_semi_sync_master_wait_forever
0
SET GLOBAL debug = '+d,rpl_semisync_wait_timeout';
[connection server_2]
SET GLOBAL debug = '+d,rpl_semisync_before_send_ack';
# Server_1 send INSERT (5)
[connection server_1]
INSERT INTO t1 VALUES(5);;
# Con1 switch forever on and wait for timeout
SET DEBUG_SYNC = 'now WAIT_FOR timeout';
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
SET DEBUG_SYNC = 'now SIGNAL continue';
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	ON
# Server_2 ack after sleeping 5s
[connection server_2]
SET DEBUG_SYNC = 'now SIGNAL continue';
SET GLOBAL debug = '-d,rpl_semisync_before_send_ack';
# Server_1 wakeup
[connection server_1]
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	ON
SET GLOBAL debug = '-d,rpl_semisync_wait_timeout';
# Check wait time
# Test with wait for slave count = 2
SET GLOBAL rpl_semi_sync_master_wait_for_slave_count = 2;
[connection server_3]
include/install_semisync_replica.inc
Warnings:
Note	1287	'rpl_semi_sync_slave' is deprecated and will be removed in a future release. Please use rpl_semi_sync_replica instead
[connection server_1]
# Test 2.1 : timeout without forever enabled
# Let server_2 wait before sending ack
[connection server_2]
SET @original_debug = @@GLOBAL.debug;
SET GLOBAL debug = '+d,rpl_semisync_before_send_ack';
# Server_1 send INSERT (21)
[connection server_1]
INSERT INTO t1 VALUES(21);;
# Make sure server_1 receives ack from server_3
[connection server_3]
# Server_2 ack after sleeping 5s
[connection server_2]
SET DEBUG_SYNC = 'now SIGNAL continue';
# Server_1 wait timeout
[connection server_1]
# Check wait time
# Test 2.2 : no timeout with forever enabled
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
SELECT @@rpl_semi_sync_master_wait_forever;
@@rpl_semi_sync_master_wait_forever
1
# Server_1 send INSERT(22)
[connection server_1]
INSERT INTO t1 VALUES(22);;
# Make sure server_1 receives ack from server_3
[connection server_3]
# Server_2 ack after sleeping 5s
[connection server_2]
SET DEBUG_SYNC = 'now SIGNAL continue';
SET GLOBAL debug = '-d,rpl_semisync_before_send_ack';
# Server_1 wait timeout
[connection server_1]
# Check wait time
# Test 2.3: wakeup when switch forever off
# with rpl_semi_sync_master_wait_no_slave=OFF
# Let server_2 wait before sending ack
[connection server_2]
SET GLOBAL debug = '+d,rpl_semisync_before_send_ack';
# Server_1 send INSERT(23)
[connection default]
INSERT INTO t1 VALUES(23);
# Make sure server_1 receives ack (23) from server_3
[connection server_3]
# Server_1 send INSERT(24)
[connection server_1]
INSERT INTO t1 VALUES(24);
# Make sure server_1 receives ack (24) from server_3
[connection server_3]
# Server_2 ack after sleeping 5s
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	OFF
[connection default]
[connection server_1]
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	OFF
# Server_3 send ack for INSERT (23)
[connection server_2]
SET DEBUG_SYNC = 'now SIGNAL continue';
# Server_3 send ack for INSERT (24)
SET DEBUG_SYNC = 'now SIGNAL continue';
SET GLOBAL debug = '-d,rpl_semisync_before_send_ack';
[connection server_1]
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	ON
# Check wait time
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
# Test 2.4: wakeup when switch forever off
# with rpl_semi_sync_master_wait_no_slave=OFF
SET GLOBAL rpl_semi_sync_master_wait_no_slave= ON;
# Let server_2 wait before sending ack
[connection server_2]
SET GLOBAL debug = '+d,rpl_semisync_before_send_ack';
# Server_1 send INSERT(25)
[connection default]
INSERT INTO t1 VALUES(25);
# Make sure server_1 receives ack (25) from server_3
[connection server_3]
# Server_1 send INSERT(26)
[connection server_1]
INSERT INTO t1 VALUES(26);
# Make sure server_1 receives ack (26) from server_3
[connection server_3]
# Server_2 ack after sleeping 5s
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	OFF
[connection default]
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	OFF
[connection server_1]
# Server_3 send ack for INSERT (25)
[connection server_2]
SET DEBUG_SYNC = 'now SIGNAL continue';
# Server_3 send ack for INSERT (26)
SET DEBUG_SYNC = 'now SIGNAL continue';
SET GLOBAL debug = '-d,rpl_semisync_before_send_ack';
[connection server_1]
SELECT @@rpl_semi_sync_master_enabled;
@@rpl_semi_sync_master_enabled
1
SHOW STATUS like 'rpl_semi_sync_master_status';
Variable_name	Value
Rpl_semi_sync_master_status	ON
# Check wait time
SET GLOBAL rpl_semi_sync_master_wait_no_slave= OFF;
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
SET GLOBAL rpl_semi_sync_master_wait_forever= OFF;
# Cleanup
[connection server_1]
DROP TABLE t1;
include/rpl_sync.inc
[connection server_2]
include/uninstall_semisync_replica.inc
[connection server_3]
include/uninstall_semisync_replica.inc
[connection server_1]
SET GLOBAL rpl_semi_sync_master_enabled= OFF;
include/uninstall_semisync_source.inc
include/rpl_end.inc

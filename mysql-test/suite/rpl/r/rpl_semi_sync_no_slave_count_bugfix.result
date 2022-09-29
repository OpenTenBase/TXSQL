#
# prepare
#
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
CALL mtr.add_suppression('SEMISYNC: Forced shutdown. Some updates might not be replicated');
CALL mtr.add_suppression('of the transaction may not be synchronized to slave when server shutdown');
# Enable semisync on master
[connection server_1]
include/install_semisync_source.inc
Warnings:
Note	1287	'rpl_semi_sync_master' is deprecated and will be removed in a future release. Please use rpl_semi_sync_source instead
SET GLOBAL rpl_semi_sync_master_enabled= ON;
SET GLOBAL rpl_semi_sync_master_wait_forever= ON;
# Change master variables
SET GLOBAL rpl_semi_sync_master_timeout= 20000000;
SET GLOBAL rpl_semi_sync_master_wait_no_slave= ON;
SET GLOBAL rpl_semi_sync_master_wait_for_slave_count = 2;
SET GLOBAL rpl_semi_sync_master_trace_level = 255;
# Start slave : server_2
[connection server_2]
include/install_semisync_replica.inc
Warnings:
Note	1287	'rpl_semi_sync_slave' is deprecated and will be removed in a future release. Please use rpl_semi_sync_replica instead
SET GLOBAL rpl_semi_sync_slave_trace_level = 255;
show variables like "slave_net_timeout";
Variable_name	Value
slave_net_timeout	8
# Start slave : server_3
[connection server_3]
include/install_semisync_replica.inc
Warnings:
Note	1287	'rpl_semi_sync_slave' is deprecated and will be removed in a future release. Please use rpl_semi_sync_replica instead
SET GLOBAL rpl_semi_sync_slave_trace_level = 255;
show variables like "slave_net_timeout";
Variable_name	Value
slave_net_timeout	8
[connection server_1]
include/rpl_sync.inc
# Verify ack_receiver thread is created
include/assert.inc [ack receiver thread is created;]
CREATE TABLE t1(c1 INT);
include/rpl_sync.inc
# Verify semisync replication works well
[connection server_1]
include/assert.inc [Semisync master is on]
#
# Run
#
# shutdown slave : server_2
[connection server_2]
include/stop_slave_io.inc
include/wait_for_slave_io_to_stop.inc
# Server2 io_thread is down
# shutdown slave : server_3
[connection server_3]
include/stop_slave_io.inc
include/wait_for_slave_io_to_stop.inc
# Server3 io_thread is down
#
# sleep 6s wait master send heartbeat 
# shutdown slave : server_1
[connection server_1]
insert into t1 values(1);
# Server1 is down
[connection server_1]
ERROR HY000: The binlog(file: master-bin.000001, end_pos: <pos>) of the transaction may not be synchronized to slave when server shutdown.

# restart server 1.
include/rpl_start_server.inc [server_number=1]
[connection server_1]
# restart server 2 io_thread.
[connection server_2]
include/start_slave_io.inc
# restart server 3 io_thread.
[connection server_3]
include/start_slave_io.inc
#
# Cleanup
#
[connection server_1]
DROP TABLE t1;
include/rpl_sync.inc
[connection server_2]
include/uninstall_semisync_replica.inc
[connection server_3]
include/uninstall_semisync_replica.inc
[connection server_1]
include/uninstall_semisync_source.inc
include/rpl_end.inc

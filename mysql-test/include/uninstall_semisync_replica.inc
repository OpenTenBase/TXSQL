# Uninstall semisync slave plugin
# Usage:
# [--let $use_old_semisync_plugin_names = 1|0]
# --source include/uninstall_semisync_replica.inc
#
# Parameters:
#
#   $use_old_semisync_plugin_names
#     By default uses the new-named plugin, semisync_source.  If this is
#     set to 1, uses the old-named plugin, semisync_master

--let $include_filename= uninstall_semisync_replica.inc
--source include/begin_include_file.inc

--let $_iss_replica = replica 
if ($use_old_semisync_plugin_names) {
  --let $_iss_replica = slave 
}

--disable_query_log

--source include/stop_slave_io.inc

--disable_warnings
eval UNINSTALL PLUGIN rpl_semi_sync_$_iss_replica;
--enable_warnings

--source include/start_slave_io.inc

--source include/end_include_file.inc

# Uninstall semisync master plugin
# Usage:
# [--let $use_old_semisync_plugin_names = 1|0]
# --source include/uninstall_semisync_source.inc
#
# Parameters:
#
#   $use_old_semisync_plugin_names
#     By default uses the new-named plugin, semisync_source.  If this is
#     set to 1, uses the old-named plugin, semisync_master

--let $include_filename= uninstall_semisync_source.inc
--source include/begin_include_file.inc

--let $_ss_source = source
--let $_ss_var= Rpl_semi_sync_source_clients
if ($use_old_semisync_plugin_names) {
  --let $_ss_source = master
  --let $_ss_var= Rpl_semi_sync_master_clients
}

--disable_query_log
--disable_warnings

# wait till semisync clients are '0' before uninstalling
# semisync master plugin.
--let $status_var=$_ss_var
--let $status_var_value= 0
--source include/wait_for_status_var.inc

eval UNINSTALL PLUGIN rpl_semi_sync_$_ss_source;

--source include/end_include_file.inc

# ==== Purpose ====
#
# Wait until rpl_semi_sync_source_status equals to OFF
#
# ==== Usage ====
#
# [--let $use_old_semisync_plugin_names = 1]
# --source include/wait_for_semisync_source_status_off.inc
#
# ==== Parameters ====
#
# $use_old_semisync_plugin_names
#   By default uses the new-named plugin, semisync_source.  If this is
#   set to 1, uses the old-named plugin, semisync_master

--let $status_var= rpl_semi_sync_source_status
if ($use_old_semisync_plugin_names) {
  --let $status_var= rpl_semi_sync_master_status
}

--let $status_var_value= OFF
--source include/wait_for_status_var.inc

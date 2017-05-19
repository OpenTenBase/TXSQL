#
# An auxaliary file for asserting semisync master on
#
# assert source status is on
# Usage:
# [--let $use_old_semisync_plugin_names = 1]
# --source include/install_semisync_source.inc
#
# $use_old_semisync_plugin_names
#   By default uses the new-named plugin, semisync_source.  If this is
#   set to 1, uses the old-named plugin, semisync_master

--let $var_name = rpl_semi_sync_source_status
if ($use_old_semisync_plugin_names) {
  --let $var_name = rpl_semi_sync_master_status
}
--let $semisync_status= query_get_value(SHOW STATUS LIKE '$var_name', Value, 1)
--let $assert_text= Semisync master is on
--let $assert_cond= "$semisync_status" = "ON"
--source include/assert.inc

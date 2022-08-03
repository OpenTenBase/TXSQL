set global cdb_audit_action_enabled = on;
set global audit_cdb_config_json = 'mysql_test_dir/t/plugin_audit_cdb.json';
set global audit_cdb_audit_reload_config = on;
show status like "%audit%";
Variable_name	Value
audit_cdb_ignore_actions	#
audit_cdb_ignore_push_queue_counts	#
audit_cdb_log_file_name	#
audit_cdb_log_file_pos	#
audit_cdb_truncat_counts	#
set global cdb_audit_action_enabled = off;
select sleep(4);
sleep(4)
0
# restart

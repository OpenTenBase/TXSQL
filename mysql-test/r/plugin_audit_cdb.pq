install plugin audit_cdb soname 'audit_cdb.so';
set global cdb_audit_action_enabled = on;
set global cdb_tencentroot_audit_action_enabled = on;
set global audit_cdb_config_json = 'mysql_test_dir/t/plugin_audit_cdb.json';
set global audit_cdb_audit_reload_config = on;
Found pos file
Found log file
set global audit_cdb_tencentroot_audit_dir=@@global.tmpdir;
Found pos file
Found log file
set global audit_cdb_log_file_max_size = 8*1024*1024;
set global audit_cdb_rotate_file_count = 2;
tencentroot.0
tencentroot.pos_file
set global audit_cdb_log_file_max_size = 134217728;
set global audit_cdb_rotate_file_count = 10;
set global cdb_audit_action_enabled = off;
set global cdb_tencentroot_audit_action_enabled = off;
select sleep(4);
sleep(4)
0
uninstall plugin audit_cdb;
Warnings:
Warning	1620	Plugin is busy and will be uninstalled on shutdown
# restart

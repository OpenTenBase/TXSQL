set global cdb_audit_action_enabled = on;
set global cdb_tencentroot_audit_action_enabled = on;
set global audit_cdb_tencentroot_audit_dir = "mysql_test_dir/audit_tmp/";
set global audit_cdb_audit_dir = "mysql_test_dir/audit_tmp/";
show variables like "AUDIT_CDB_AUDIT_RELOAD_CONFIG";
Variable_name	Value
audit_cdb_audit_reload_config	ON
set global audit_cdb_config_json = "mysql_test_dir/t/xxxxxx.json";
set global audit_cdb_audit_reload_config = on;
show variables like "AUDIT_CDB_AUDIT_RELOAD_CONFIG";
Variable_name	Value
audit_cdb_audit_reload_config	ON
set global audit_cdb_config_json = "mysql_test_dir/t/plugin_audit_cdb_file_check.json";
set global audit_cdb_audit_reload_config = on;
show variables like "AUDIT_CDB_AUDIT_RELOAD_CONFIG";
Variable_name	Value
audit_cdb_audit_reload_config	OFF
select sleep(5);
sleep(5)
0
set global audit_cdb_audit_reload_config = on;
mysql_test_dir/audit_tmp/my_audit_log_file.0
set global audit_cdb_audit_reload_config = on;
select sleep(5);
sleep(5)
0
use test;
create table `test_sql_type` (id INT PRIMARY KEY, number INT);
insert into test_sql_type (id, number) values (1, 1);
select * from test_sql_type;
id	number
1	1
update test_sql_type set number = 2 where id = 1;
alter table test_sql_type drop column number;
replace into test_sql_type (id) values(2);
delete from test_sql_type where id = 1;
prepare t from "select * from test_sql_type";
EXECUTE t;
id
2
DEALLOCATE PREPARE t;
set global audit_cdb_audit_reload_config = on;
drop table test_sql_type;
select sleep(5);
sleep(5)
0
set global audit_cdb_tencentroot_audit_dir = "mysql_test_dir";
set global audit_cdb_audit_dir = "mysql_test_dir";
set global audit_cdb_config_json = "mysql_test_dir/t/xxxxxx.json";
set global audit_cdb_audit_reload_config = on;
set global audit_cdb_config_json = "/usr/local/json/audit_config.json";

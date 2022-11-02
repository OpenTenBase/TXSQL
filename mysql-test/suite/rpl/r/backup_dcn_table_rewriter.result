include/rpl_init.inc [topology=1->2,1->3 ]
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
include/rpl_connect.inc [creating master]
include/rpl_connect.inc [creating slave_1]
include/rpl_connect.inc [creating slave_2]
include/rpl_stop_slaves.inc
include/rpl_change_topology.inc [new topology=1->3->2]
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
include/rpl_start_slaves.inc
create table t(id int primary key);
set session tx_isolation='REPEATABLE-READ';
set session binlog_format='STATEMENT';
set session tdsql_allow_access_internal_table=on;
insert into query_rewrite.table_rewrite_rules values('test','t@1','t');
include/sync_slave_sql_with_master.inc
select * from query_rewrite.table_rewrite_rules;
db	table_name	table_name_new
test	t@1	t
select * from `t@1`;
id
include/sync_slave_sql_with_master.inc
select * from query_rewrite.table_rewrite_rules;
db	table_name	table_name_new
test	t@1	t
select * from `t@1`;
id
drop table t;
delete from query_rewrite.table_rewrite_rules;
include/rpl_end.inc

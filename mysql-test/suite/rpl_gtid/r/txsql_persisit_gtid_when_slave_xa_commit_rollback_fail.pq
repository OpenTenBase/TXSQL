include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
select @@global.gtid_mode;
@@global.gtid_mode
OFF_PERMISSIVE
set global gtid_mode = 'off_permissive';
set global gtid_mode = 'on_permissive';
set global gtid_mode = 'ON';
set gtid_next='automatic';
call mtr.add_suppression("The slave coordinator and worker threads are stopped, possibly leaving data in inconsistent state");
call mtr.add_suppression("Slave I/O for channel '': Master command COM_REGISTER_SLAVE failed: failed registering on master, reconnecting to try *");
call mtr.add_suppression("Unsafe statement written to the binary log using statement format *");
CREATE TABLE `t` (
`id` int NOT NULL PRIMARY KEY AUTO_INCREMENT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
include/sync_slave_sql_with_master.inc
show create table t;
Table	Create Table
t	CREATE TABLE `t` (
  `id` int NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci
xa begin 'xa_id';
insert into t values(0);
xa end 'xa_id';
xa prepare 'xa_id';
xa commit  'xa_id' ;
include/sync_slave_sql_with_master.inc
"select slave,get one line"
select * from t;
id
1
set global debug="+d,xa_commit_rollback_wait10sec";
xa begin 'xa_id';
insert into t values(0);
xa end 'xa_id';
xa prepare 'xa_id';
xa commit  'xa_id' ;
set sql_log_bin=0;
xa commit 'xa_id';
include/sync_slave_sql_with_master.inc
"select slave,get two lines"
select * from t;
id
1
2
"master gtid == slave gtid:,so it is fixed issue_141"
set global debug="+d,xa_commit_rollback_wait10sec";
xa begin 'xa_id';
insert into t values(0);
xa end 'xa_id';
xa prepare 'xa_id';
xa rollback  'xa_id' ;
set sql_log_bin=0;
xa rollback 'xa_id';
include/sync_slave_sql_with_master.inc
"because rollback,so select slave,get two lines"
select * from t;
id
1
2
"master gtid == slave gtid:,so it is fixed issue_141"
set global debug='';
drop table t;
include/rpl_end.inc
set global gtid_mode = 'on_permissive';
set global gtid_mode = 'OFF_PERMISSIVE';

include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
create database `测试数据库aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`;
use `测试数据库aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`;
create table `测试表aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`(id int);
insert into `测试表aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa` values(1),(2),(3);
select * from `测试表aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`;
id
1
2
3
create database `测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测`;
use `测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测`;
create table `测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表`(id int);
insert into `测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表` values(1);
SET @@SESSION.debug = "+d,simulate_table_map_max_table_id";
table_id: 18446744073709551615 (测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测.测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表)
SET @@SESSION.debug = "-d,simulate_table_map_max_table_id";
include/sync_slave_sql_with_master.inc
use `测试数据库aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`;
select * from `测试表aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`;
id
1
2
3
use `测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测`;
select * from `测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表测试数据表`;
id
1
drop database `测试数据库aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`;
drop database `测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测试数据库测`;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

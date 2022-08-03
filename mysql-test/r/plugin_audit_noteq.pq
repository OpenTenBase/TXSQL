set global cdb_audit_action_enabled = on;
set global audit_cdb_config_json = 'mysql_test_dir/t/plugin_audit_noteq.json';
set global audit_cdb_audit_reload_config = on;
CREATE USER test1 IDENTIFIED BY 'test';
GRANT ALL ON testdb.* TO 'test1'@'%';
CREATE USER test2 IDENTIFIED BY 'test';
GRANT ALL ON testdb.* TO 'test2'@'%';
CREATE USER test3 IDENTIFIED BY 'test';
GRANT ALL ON testdb.* TO 'test3'@'%';
FLUSH PRIVILEGES;
create database testdb;
use testdb;
create table abcd(a int);
insert into abcd values(1);
update abcd set a =111 where a <5;
select * from abcd;
a
111
drop table abcd;
use testdb;
create table abcd(a int);
insert into abcd values(1);
update abcd set a =111 where a <5;
drop table abcd;
use testdb;
create table abcd(a int);
insert into abcd values(1);
update abcd set a =111 where a <5;
drop table abcd;
use testdb;
create table abcd(a int);
insert into abcd values(1);
update abcd set a =111 where a <5;
select * from abcd;
a
111
drop table abcd;
set global cdb_audit_action_enabled = off;
REVOKE ALL ON testdb.* FROM 'test1'@'%';
DROP USER test1@'%';
REVOKE ALL ON testdb.* FROM 'test2'@'%';
DROP USER test2@'%';
REVOKE ALL ON testdb.* FROM 'test3'@'%';
DROP USER test3@'%';
drop database testdb;
select sleep(4);
sleep(4)
0
uninstall plugin audit_cdb;
Warnings:
Warning	1620	Plugin is busy and will be uninstalled on shutdown
# restart

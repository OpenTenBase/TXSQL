#connect user=root
#1.1 check max_connections for non tencent users.
set global max_connections=2;
#connect con_root_1
#reserved connections test
#1.2 check processlist for non tencent users.
#non tencent connection
show processlist;
Id	User	Host	db	Command	Time	State	Info
#####	event_scheduler	localhost	NULL	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	show processlist
#####	root	localhost	test	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	NULL
select id, user, command from information_schema.processlist order by user;
id	user	command
#####	event_scheduler	#####
#####	root	#####
#####	root	#####
#####	root	#####
#1.3 check show processlist for tencent users.
show processlist;
Id	User	Host	db	Command	Time	State	Info
#####	event_scheduler	localhost	NULL	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	show processlist
#####	root	localhost	test	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	NULL
select id, user, command from information_schema.processlist order by user;
id	user	command
#####	event_scheduler	#####
#####	root	#####
#####	root	#####
#####	root	#####
#####	root	#####
#####	root	#####
#####	root	#####
#1.4 kill operation for non tencent users.
kill ##;
kill ##;
ERROR HY###: Unknown thread id: ##
kill ##;
kill ##;
select id, user, command from information_schema.processlist order by user;
id	user	command
#####	event_scheduler	#####
#####	root	#####
#####	root	#####
#####	root	#####
#1.5 check set privileges for non tencent users.
set global tencent_root_whitelist='localhost';
ERROR HY000: Unknown system variable 'tencent_root_whitelist'
set global tencent_root_embedded=OFF;
ERROR HY000: Unknown system variable 'tencent_root_embedded'
set global tencent_root_whitelist='localhost;127.0.0.1';
set global tencent_root_whitelist= 'localhost;127.0.0.1;::1';
SET GLOBAL max_connections = 151;
#
# only root have write access to mysql schema
#
use mysql;
select host from db limit 0;
host
create table tbl(c1 int primary key, c2 int);
insert into tbl values(1,1);
update tbl set c2 = 2 where c1 = 1;
replace into tbl values(1, 3);
delete from tbl where c1 = 1;
alter table tbl add column c3 int;
create index idx_c2 on tbl(c2);
drop index idx_c2 on tbl;
create view v1 as select * from tbl;
drop view v1;
create view test.v1 as select * from tbl;
drop view test.v1;
create trigger tri_tbl before insert on tbl for each row set @sum = @sum + 1;
ERROR HY000: Triggers can not be created on system tables
drop trigger tri_tbl;
ERROR HY000: Trigger does not exist
repair table tbl;
Table	Op	Msg_type	Msg_text
mysql.tbl	repair	note	The storage engine for the table doesn't support repair
optimize table tbl;
Table	Op	Msg_type	Msg_text
mysql.tbl	optimize	note	Table does not support optimize, doing recreate + analyze instead
mysql.tbl	optimize	status	OK
create table test.temp_table like tbl;
insert into test.temp_table select * from mysql.tbl;
drop table test.temp_table;
lock tables tbl write;
unlock tables;
check table tbl quick;
Table	Op	Msg_type	Msg_text
mysql.tbl	check	status	OK
flush table tbl;
analyze table tbl;
Table	Op	Msg_type	Msg_text
mysql.tbl	analyze	status	OK
checksum table tbl;
Table	Checksum
mysql.tbl	0
handler tbl open;
handler tbl read first;
c1	c2	c3
handler tbl close;
create user 'u1'@'localhost' identified by '';
grant select on mysql.tbl to 'u1'@'localhost';
revoke select on mysql.tbl from 'u1'@'localhost';
revoke all, grant option from 'u1'@'localhost';
drop user 'u1'@'localhost';
#
# even root has no write access to mysql schema
#
use mysql;
select host from db limit 0;
host
create table tbl_fail(id int);
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
drop table tbl_fail;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
insert into tbl values(1, 1, 1);
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
update tbl set c2 = 2 where c1 = 1;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
replace into tbl values(1, 3, 3);
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
delete from tbl where c1 = 1;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
alter table tbl drop column c3;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
rename table tbl to temp_tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
drop table tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
create index idx_c2 on tbl(c2);
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
drop index idx_c2 on tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
create view v1 as select * from tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
drop view v1;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
create view test.v1 as select * from tbl;
drop view test.v1;
create trigger tri_tbl before insert on tbl for each row set @sum = @sum + 1;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
drop trigger tri_tbl;
ERROR HY000: Trigger does not exist
repair table tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
optimize table tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
create table test.temp_table like tbl;
insert into test.temp_table select * from mysql.tbl;
drop table test.temp_table;
lock tables tbl write;
unlock tables;
check table tbl quick;
Table	Op	Msg_type	Msg_text
mysql.tbl	check	status	OK
flush table tbl;
analyze table tbl;
Table	Op	Msg_type	Msg_text
mysql.tbl	analyze	status	OK
checksum table tbl;
Table	Checksum
mysql.tbl	0
handler tbl open;
handler tbl read first;
c1	c2	c3
handler tbl close;
create user 'u1'@'localhost' identified by '';
grant select on mysql.tbl to 'u1'@'localhost';
revoke select on mysql.tbl from 'u1'@'localhost';
revoke all, grant option from 'u1'@'localhost';
drop user 'u1'@'localhost';
drop database mysql;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
#clean up
use mysql;
rename table tbl to temp_tbl;
drop table temp_tbl;
SET GLOBAL cdb_forbid_system_database_write=ON;
#
# even root has no write access to sys schema
#
use sys;
create table tbl(c1 int primary key, c2 int);
insert into tbl values(1,1);
update tbl set c2 = 2 where c1 = 1;
replace into tbl values(1, 3);
delete from tbl where c1 = 1;
alter table tbl add column c3 int;
create index idx_c2 on tbl(c2);
drop index idx_c2 on tbl;
use test;
drop database sys;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
use sys;
select host from host_summary limit 0;
host
create table tbl_fail(id int);;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
drop table bl_fail;;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
insert into tbl values(1, 1, 1);
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
update tbl set c2 = 2 where c1 = 1;;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
replace into tbl values(1, 3, 3);
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
delete from tbl where c1 = 1;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
alter table tbl drop column c3;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
rename table tbl to temp_tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
drop table tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
create index idx_c2 on tbl(c2);
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
drop index idx_c2 on tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
create view v1 as select * from tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
drop view v1;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
create trigger tri_tbl before insert on tbl for each row set @sum = @sum + 1;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
drop trigger tri_tbl;
ERROR HY000: Trigger does not exist
repair table tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
optimize table tbl;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'sys'
check table host_summary quick;
Table	Op	Msg_type	Msg_text
sys.host_summary	check	status	OK
flush table host_summary;
analyze table host_summary;
Table	Op	Msg_type	Msg_text
sys.host_summary	analyze	Error	'sys.host_summary' is not BASE TABLE
sys.host_summary	analyze	status	Operation failed
checksum table host_summary;
Table	Checksum
sys.host_summary	NULL
Warnings:
Error	1347	'sys.host_summary' is not BASE TABLE
use sys;
drop table tbl;
SET GLOBAL cdb_forbid_system_database_write=0;

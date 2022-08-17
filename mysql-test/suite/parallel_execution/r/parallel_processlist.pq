# Infrastructure for test parallel execution frame
show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,GROUP_REPLICATION_ADMIN,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PERSIST_RO_VARIABLES_ADMIN,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ''@'' TO 'root'@'localhost' WITH GRANT OPTION
select user();
user()
root@localhost
create database test1;
create user 'user1'@'%';
grant all on test1.* to 'user1'@'%';
create table t1(a int, b int, c int, d int);
insert into t1 values(1,2,3,4);
insert into t1 values(2,3,4,5);
create table t2(a int, b int, c int, d int);
insert into t2 values(1,1,1,1);
insert into t2 values(2,2,2,2);
create table t3(a int, b int, c int, d int);
# --include save_and_set_px.inc
flush status;
set global debug="+d,enter_worker_sleep";
(select /*+no_bnl(t1,t2)*/ t1.* from t1,t2 where t1.a = t2.a order by t1.a) union (select * from t3) union (select * from t2 order by a);;
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
set global debug="-d,enter_worker_sleep";
set global debug="+d,execute_in_parallel_before_sleep";
(select a+b,c+d from t1 order by a+b,c+d) union all (select d+c,b+a from t2 order by d+c);;
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	root	localhost	test	COMMAND	TIME	STATE	INFO
a+b	c+d
2	2
3	7
4	4
5	9
set global debug="-d,execute_in_parallel_before_sleep";
set global debug="+d,run_root_in_sending_data_sleep";
(select a,b from t2) union (select a, sum(b+c) from t1 where a > 1 group by a order by a);;
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
a	b
1	1
2	2
2	7
set global debug="-d,run_root_in_sending_data_sleep";
set global debug="+d,schedule_before_sleep";
(select /*+no_bnl(t1,t2)*/ t1.* from t1,t2 where t1.a = t2.a order by t1.a) union (select * from t3) union (select * from t2 order by a);;
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
set global debug="-d,schedule_before_sleep";
set global debug="+d,schedule_before_schedule_sleep";
(select a+b,c+d from t1) union (select d+c,b+a from t2 order by d+c);;
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
a+b	c+d
2	2
3	7
4	4
5	9
set global debug="-d,schedule_before_schedule_sleep";
set global debug="+d,schedule_one_loop_sleep";
(select a,b from t2) union (select a, sum(b+c) from t1 where a > 1 group by a order by a);;
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
a	b
1	1
2	2
2	7
set global debug="-d,schedule_one_loop_sleep";
set debug_sync='schedule_before_schedule_kill SIGNAL waiting wait_for killed';
select /*+no_bnl(t1,t2)*/ t1.* from t1,t2 where t1.a = t2.a;;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id> + 1;
Warnings:
Warning	8508	Killing worker is not allowed, please kill query/connection <id>
SET DEBUG_SYNC= 'now SIGNAL killed';
a	b	c	d
1	2	3	4
2	3	4	5
SET DEBUG_SYNC = 'RESET';
set debug_sync='schedule_before_schedule_kill SIGNAL waiting wait_for killed';
(select a,sum(b) from t3 group by a order by a) union (select a,b from t2 order by a);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL CONNECTION <id> + 1;
Warnings:
Warning	8508	Killing worker is not allowed, please kill query/connection <id>
SET DEBUG_SYNC= 'now SIGNAL killed';
a	sum(b)
1	1
2	2
SET DEBUG_SYNC = 'RESET';
set debug_sync='schedule_one_loop_kill SIGNAL waiting wait_for killed';
select a,b from t1 union (select a,b from t2 order by a);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id> + 1;
Warnings:
Warning	8508	Killing worker is not allowed, please kill query/connection <id>
SET DEBUG_SYNC= 'now SIGNAL killed';
a	b
1	1
1	2
2	2
2	3
SET DEBUG_SYNC = 'RESET';
set debug_sync='schedule_one_loop_kill SIGNAL waiting wait_for killed';
(select a,b,c,d from t1) union (select a,b,sum(c),sum(d) from t2 group by a,b order by a,b);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL CONNECTION <id> + 1;
Warnings:
Warning	8508	Killing worker is not allowed, please kill query/connection <id>
SET DEBUG_SYNC= 'now SIGNAL killed';
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
SET DEBUG_SYNC = 'RESET';
set debug_sync='run_root_sending_data_kill SIGNAL waiting wait_for killed';
(select a,b,sum(c) from t1 group by a,b) union (select a, sum(b), avg(c)+avg(d) from t2 group by a order by a);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL QUERY <id> + 1;
Warnings:
Warning	8<id>8	Killing worker is not allowed, please kill query/connection <id>
SET DEBUG_SYNC= 'now SIGNAL killed';
a	b	sum(c)
1	1	2.0000
1	2	3.0000
2	2	4.0000
2	3	4.0000
SET DEBUG_SYNC = 'RESET';
set debug_sync='run_root_sending_data_kill SIGNAL waiting wait_for killed';
(select a,b,c,d from t1 order by d) union (select a,b,sum(c),sum(d) from t2 group by a,b);;
SET DEBUG_SYNC= 'now WAIT_FOR waiting';
KILL CONNECTION <id> + 1;
Warnings:
Warning	8508	Killing worker is not allowed, please kill query/connection <id>
SET DEBUG_SYNC= 'now SIGNAL killed';
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
2	3	4	5
SET DEBUG_SYNC = 'RESET';
create user user101@localhost, user102@localhost,
user103@localhost, user104@localhost;
grant ALL on *.* to user101@localhost;
grant ALL on *.* to user102@localhost;
grant SELECT on *.* to user103@localhost;
grant SELECT on *.* to user104@localhost;
create table t4(a int, b int, c int, d int);
insert into t4 values(1,1,1,1);
set global debug="+d,enter_worker_sleep";
select * from t4 union select * from t4;;
select * from t4 union select * from t4;;
select * from t4 union select * from t4;;
root see all.
show processlist;
Id	User	Host	db	Command	Time	State	Info
#	event_scheduler	localhost	NULL	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	user101	localhost	test	COMMAND	TIME	STATE	INFO
#	user102	localhost	test	COMMAND	TIME	STATE	INFO
#	user103	localhost	test	COMMAND	TIME	STATE	INFO
#	user104	localhost	test	COMMAND	TIME	STATE	INFO
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	user101	localhost	test	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	user103	localhost	test	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
user with root see all
show processlist;
Id	User	Host	db	Command	Time	State	Info
#	event_scheduler	localhost	NULL	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	user101	localhost	test	COMMAND	TIME	STATE	INFO
#	user102	localhost	test	COMMAND	TIME	STATE	INFO
#	user103	localhost	test	COMMAND	TIME	STATE	INFO
#	user104	localhost	test	COMMAND	TIME	STATE	INFO
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	user101	localhost	test	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	user103	localhost	test	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
user with no root see only himself
show processlist;
Id	User	Host	db	Command	Time	State	Info
#	user104	localhost	test	COMMAND	TIME	STATE	INFO
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
tencent root see everything.
show processlist;
Id	User	Host	db	Command	Time	State	Info
#	event_scheduler	localhost	NULL	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	tencentroot	localhost	test	COMMAND	TIME	STATE	INFO
#	tencentroot	localhost	test	COMMAND	TIME	STATE	INFO
#	user101	localhost	test	COMMAND	TIME	STATE	INFO
#	user102	localhost	test	COMMAND	TIME	STATE	INFO
#	user103	localhost	test	COMMAND	TIME	STATE	INFO
#	user104	localhost	test	COMMAND	TIME	STATE	INFO
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	tencentroot	localhost	test	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	user101	localhost	test	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	user103	localhost	test	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
root can not see tencent root.
show processlist;
Id	User	Host	db	Command	Time	State	Info
#	event_scheduler	localhost	NULL	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	root	localhost	test	COMMAND	TIME	STATE	INFO
#	user101	localhost	test	COMMAND	TIME	STATE	INFO
#	user102	localhost	test	COMMAND	TIME	STATE	INFO
#	user103	localhost	test	COMMAND	TIME	STATE	INFO
#	user104	localhost	test	COMMAND	TIME	STATE	INFO
show parallel processlist;
Id	User	Host	db	Command	Time	State	Info
#	user101	localhost	test	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	user103	localhost	test	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
#	 	 	 	COMMAND	TIME	STATE	INFO
set global debug="-d,enter_worker_sleep";
# --include set_px_to_saved.inc
drop table t1;
drop table t2;
drop table t3;
drop table t4;
drop database test1;
drop user user1;
drop user user101@localhost;
drop user user102@localhost;
drop user user103@localhost;
drop user user104@localhost;

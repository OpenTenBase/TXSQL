#
# Test for lock hangup
#
Create database and tables
create database tdb1;
use tdb1;
create table t1 (f1 int);
create table t2 (f1 int);
set global innodb_lock_wait_timeout=300;
#Start transaction and Lock t1 to block session
begin;
select * from t1 for update;
f1
#start a client
#exec mysql command
#Sleep 10 for mysql client send query
#there should be a insert sql blocked
show processlist;
Id	User	Host	db	Command	Time	State	Info
##	event_scheduler	localhost	NULL	Daemon	##	Waiting on empty queue	NULL
##	root	localhost	tdb1	Sleep	##		NULL
##	root	localhost	tdb1	Query	##	update	insert into t1 values (1)
##	root	localhost	test	Query	##	init	show processlist
#kill mysql client
#Sleep 3 second for block query quit
#There should not be a insert sql blocked
show processlist;
Id	User	Host	db	Command	Time	State	Info
##	event_scheduler	localhost	NULL	Daemon	##	Waiting on empty queue	NULL
##	root	localhost	tdb1	Sleep	##		NULL
##	root	localhost	test	Query	##	init	show processlist
Cleanup
rollback;
drop database tdb1;
set global innodb_lock_wait_timeout=default;

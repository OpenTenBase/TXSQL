create table t(id int primary key, name varchar(10), key k(name));
create database kz;
create sequence seq;
create user 'tdsqlsys_test'@'localhost';
grant all on *.* to 'tdsqlsys_test'@'localhost';
create user 'normal_user'@'localhost';
grant all on *.* to 'normal_user'@'localhost';
insert into t values(1, 'kz');
insert into t values(2, 'wj');
set global tdsql_forbid_write=1;
insert into t values(3, 'kyx');
ERROR HY000: Can not execute the statement, disk may be full.
update t set name='kyx' where id=1;
ERROR HY000: Can not execute the statement, disk may be full.
create table t1(id int primary key);
ERROR HY000: Can not execute the statement, disk may be full.
insert into t values(3, 'kyx');
drop database kz;
rename table t to t1;
drop sequence seq;
delete from t1 where id=1;
analyze table t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
truncate table t1;
drop table t1;
drop user 'tdsqlsys_test'@'localhost';
drop user 'normal_user'@'localhost';
set global tdsql_forbid_write=0;

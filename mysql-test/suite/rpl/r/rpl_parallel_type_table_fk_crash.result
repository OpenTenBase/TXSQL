include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
create table t1(id int not null primary key, c1 int,key k_c1(c1)) engine=innodb;
create table t2(id int not null primary key, t1_c1 int, foreign key(t1_c1) references t1(c1))engine = innodb;
insert into t1 values(1,1),(2,2);
include/sync_slave_sql_with_master.inc
drop table t2;
drop table t1;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

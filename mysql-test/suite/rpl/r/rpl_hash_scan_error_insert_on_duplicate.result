include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
CREATE TABLE `t1` (`id` int DEFAULT NULL, `test` varchar(255) DEFAULT 'hallo', UNIQUE KEY `id` (`id`)) ENGINE=InnoDB;
INSERT INTO `t1` VALUES (1,'foo'),(2,'test'),(3,'lorem');
insert into t1 values(1, 'test'), (1, 'foo'), (1, 'bar')  as new_t1 on duplicate key update test=new_t1.test;
insert into t1 values(1, 'test'), (1, 'foo') as new_t1 on duplicate key update test=new_t1.test;
insert into t1 values(1, 'test'), (1, 'foo'), (1, 'bar'), (1, 'foo'), (1, 'bar1'), (1, 'foo'), (1, 'bar2'), (1, 'foo'), (1, 'bar3'), (1, 'foo'), (1, 'bar4') as new_t1 on duplicate key update test=new_t1.test;
include/sync_slave_sql_with_master.inc
select * from t1;
id	test
1	bar4
2	test
3	lorem
insert into t1 values(1, 'bar'), (1, 'foo'), (1, 'bar'), (1, 'foo'), (1, 'bar'), (1, 'foo'), (1, 'bar'), (1, 'foo'), (1, 'bar'), (1, 'foo'), (1, 'bar') as new_t1 on duplicate key update test=new_t1.test;
include/sync_slave_sql_with_master.inc
select * from t1;
id	test
1	bar
2	test
3	lorem
drop table t1;
include/sync_slave_sql_with_master.inc
include/rpl_end.inc

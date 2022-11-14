xa begin 'test1';
xa end 'test1';
xa commit 'test1' one phase;
xa begin 'test1';
xa end 'test1';
xa rollback 'test1' force;
ERROR XAE05: xa rollback with force,the xid[test1] must empty
xa rollback '' force;
CREATE TABLE `t` (
`id` int NOT NULL PRIMARY KEY AUTO_INCREMENT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
xa begin 'xa_id';
insert into t values(0);
insert into t values(0);
xa rollback '' force;
"after rollback when active state,can't find data"
select * from t;
id
xa begin 'xa_id';
insert into t values(0);
insert into t values(0);
xa end 'xa_id';
xa rollback '' force;
"after rollback when idle state,can't find data"
select * from t;
id
xa begin 'xa_id_con1';
insert into t values(0);
insert into t values(0);
"connection con1,just insert,can read two num rows(not commit) "
select * from t;
id
5
6
"connection default,con1 has not commited,so can't read "
select * from t;
id
xa rollback '' force;
xa rollback '' force;
"connection con1,default rollback dont' affect con1, so can read two num rows(not commit) "
select * from t;
id
5
6
xa end 'xa_id_con1';
xa prepare 'xa_id_con1';
xa commit 'xa_id_con1';
"connection con1,after commit , so can read two num rows "
select * from t;
id
5
6
"connection default, con1 had committed , so can read two num rows "
select * from t;
id
5
6
"connection con1, con1 had committed , so can read two num rows "
select * from t;
id
5
6
begin;
insert into t values(0);
insert into t values(0);
xa rollback '' force;
"begin normal transaction,the xa rollback '' force have not meaning,can read 4 rows"
select * from t;
id
5
6
7
8
commit;
"after  normal transaction commit,the con1 can read 4 rows"
select * from t;
id
5
6
7
8
xa begin 'xa_id_default';
insert into t values(0);
insert into t values(0);
xa end 'xa_id_default';
xa prepare 'xa_id_default';
select * from t;
id
5
6
7
8
"con1 just can read 4 rows"
select * from t;
id
5
6
7
8
xa recover;
formatID	gtrid_length	bqual_length	data
1	13	0	xa_id_default
xa recover;
formatID	gtrid_length	bqual_length	data
1	13	0	xa_id_default
in mysql8.0.30, when xa_detach_on_prepare=true, xa rollback '' force can not rollback a prepared xa transaction
xa rollback '' force;
xa rollback 'xa_id_default';
select * from t;
id
5
6
7
8
"con1 just can read 4 rows"
xa recover;
formatID	gtrid_length	bqual_length	data
drop table t;

create table t1 (threadpool int, b int);
create table threadpool(id int, c1 int);
insert into t1 values (1,1);
insert into threadpool values (1,2);
select * from t1;
threadpool	b
1	1
select threadpool from t1;
threadpool
1
select * from threadpool;
id	c1
1	2
create procedure p1()
begin
declare threadpool int;
select max(t1.threadpool) into threadpool from t1;
end|
create procedure p2()
begin
threadpool:loop
leave threadpool;
end loop threadpool;
end |
call p1();
call p2();
drop procedure p1;
drop procedure p2;
drop table t1;
drop table threadpool;
create table tencentroot(id int, c1 int);
create table t1(tencentroot int, c1 int);
insert into t1 values (1,1);
insert into tencentroot values (1,2);
create procedure p1()
begin
declare tencentroot int;
select max(t1.tencentroot) into tencentroot from t1;
end|
call p1();
create procedure p2()
begin
tencentroot:loop
leave tencentroot;
end loop tencentroot;
end |
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near ':loop
leave tencentroot;
end loop tencentroot;
end' at line 3
create procedure p2()
begin
`tencentroot`:loop
leave `tencentroot`;
end loop `tencentroot`;
end |
call p2();
drop procedure p1;
drop procedure p2;
drop table t1;
drop table tencentroot;

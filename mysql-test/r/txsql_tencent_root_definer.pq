# CREATE VIEW
create table t1 (id int, name varchar(100));
insert into t1 values (1, 'test');
create view v1 as select * from t1;
show create view v1;
View	Create View	character_set_client	collation_connection
v1	CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v1` AS select `t1`.`id` AS `id`,`t1`.`name` AS `name` from `t1`	utf8mb4	utf8mb4_0900_ai_ci
select * from v1;
id	name
1	test
create view v2 as select * from v1;
select * from v2;
id	name
1	test
drop view v2;
create definer='root'@'localhost' view v3 as select  * from v1;
select * from v3;
id	name
1	test
drop view v3;
drop view v1;
create definer=root view v5 as select * from t1;
Warnings:
Note	1449	The user specified as a definer ('root'@'%') does not exist
show create view v5;
View	Create View	character_set_client	collation_connection
v5	CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`%` SQL SECURITY DEFINER VIEW `v5` AS select `t1`.`id` AS `id`,`t1`.`name` AS `name` from `t1`	utf8mb4	utf8mb4_0900_ai_ci
Warnings:
Note	1449	The user specified as a definer ('root'@'%') does not exist
select * from v5;
ERROR HY000: The user specified as a definer ('root'@'%') does not exist
drop view v5;
# CREATE EVENT
create event e1
on schedule at current_timestamp + interval 1 hour
do update t1 set id =  id + 1;
drop event e1;
create definer=root event e3
on schedule at current_timestamp + interval 1 hour
do update t1 set id =  id + 1;
Warnings:
Note	1449	The user specified as a definer ('root'@'%') does not exist
drop event e3;
# CREATE FUNCTION
create function f1(s char(20))
returns char(50) deterministic
return concat('Hello, ',s,'!');
show create function f1;
Function	sql_mode	Create Function	character_set_client	collation_connection	Database Collation
f1	ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION	CREATE DEFINER=`root`@`localhost` FUNCTION `f1`(s char(20)) RETURNS char(50) CHARSET utf8mb4
    DETERMINISTIC
return concat('Hello, ',s,'!')	utf8mb4	utf8mb4_0900_ai_ci	utf8mb4_0900_ai_ci
select f1('world');
f1('world')
Hello, world!
drop function f1;
create definer=root function f3(s char(20))
returns char(50) deterministic
return concat('Hello, ',s,'!');
Warnings:
Note	1449	The user specified as a definer ('root'@'%') does not exist
show create function f3;
Function	sql_mode	Create Function	character_set_client	collation_connection	Database Collation
f3	ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION	CREATE DEFINER=`root`@`%` FUNCTION `f3`(s char(20)) RETURNS char(50) CHARSET utf8mb4
    DETERMINISTIC
return concat('Hello, ',s,'!')	utf8mb4	utf8mb4_0900_ai_ci	utf8mb4_0900_ai_ci
select f3('world');
ERROR HY000: The user specified as a definer ('root'@'%') does not exist
drop function f3;
# CREATE PROCEDURE
create procedure p1 (out param1 int)
begin
select count(*) into param1 from t1;
end|
show create procedure p1;
Procedure	sql_mode	Create Procedure	character_set_client	collation_connection	Database Collation
p1	ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION	CREATE DEFINER=`root`@`localhost` PROCEDURE `p1`(out param1 int)
begin
select count(*) into param1 from t1;
end	utf8mb4	utf8mb4_0900_ai_ci	utf8mb4_0900_ai_ci
call p1(@a);
select @a;
@a
1
drop procedure p1;
create definer=root procedure p3 (out param1 int)
begin
select count(*) into param1 from t1;
end|
Warnings:
Note	1449	The user specified as a definer ('root'@'%') does not exist
show create procedure p3;
Procedure	sql_mode	Create Procedure	character_set_client	collation_connection	Database Collation
p3	ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION	CREATE DEFINER=`root`@`%` PROCEDURE `p3`(out param1 int)
begin
select count(*) into param1 from t1;
end	utf8mb4	utf8mb4_0900_ai_ci	utf8mb4_0900_ai_ci
call p3(@a);
ERROR HY000: The user specified as a definer ('root'@'%') does not exist
drop procedure p3;
# CREATE TRIGGER
create trigger tg1 before insert on t1
for each row set @sum = @sum + 1;
set @sum = 0;
insert into t1 values (2, 'test');
select @sum as 'Total amount inserted';
Total amount inserted
1
drop trigger tg1;
create definer=root trigger tg3 before insert on t1
for each row set @sum = @sum + 1;
Warnings:
Note	1449	The user specified as a definer ('root'@'%') does not exist
insert into t1 values (3, 'test');
ERROR HY000: The user specified as a definer ('root'@'%') does not exist
select @sum as 'Total amount inserted';
Total amount inserted
1
drop trigger tg3;
drop table t1;

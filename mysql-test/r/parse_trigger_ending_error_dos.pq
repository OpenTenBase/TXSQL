DROP TABLE IF EXISTS t1;
create table t1 (c1 int);
CREATE
TRIGGER `tg1` BEFORE INSERT ON `t1` FOR EACH ROW
LABELTRI: BEGIN
SET new.c1 = 2;
END
;
//
CREATE
TRIGGER `tg2` BEFORE UPDATE ON `t1` FOR EACH ROW
LABELTRI: BEGIN
SET new.c1 = 2;
END
;
*//
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near '*' at line 1
show triggers;
Trigger	Event	Table	Statement	Timing	Created	sql_mode	Definer	character_set_client	collation_connection	Database Collation
tg1	INSERT	t1	LABELTRI: BEGIN
SET new.c1 = 2;
END	#	#	#	root@localhost	utf8mb4	utf8mb4_0900_ai_ci	utf8mb4_0900_ai_ci
tg2	UPDATE	t1	LABELTRI: BEGIN
SET new.c1 = 2;
END	#	#	#	root@localhost	utf8mb4	utf8mb4_0900_ai_ci	utf8mb4_0900_ai_ci
drop trigger tg1;
drop trigger tg2;
drop table t1;

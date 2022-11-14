# ------------------------------------------------------------
# Create a table with 3 columns. [id, c1, c2]
# ------------------------------------------------------------
CREATE TABLE rec_t1(id INT PRIMARY KEY, c1 CHAR(10), c2 CHAR(10), c3 int, key idx_c1(c1), key idx_c3(c3)) ROW_FORMAT=dynamic;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	7	0	0
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	13	16711934	0
c2	2	13	16711934	0
c3	3	6	1027	0
id	0	6	1283	0
INSERT INTO rec_t1 VALUES (1, "r1c1", "r1c2", 1);
insert into rec_t1 values (2, "r2c1", "r2c2", 2);
SELECT * FROM rec_t1;
id	c1	c2	c3
1	r1c1	r1c2	1
2	r2c1	r2c2	2
# INSTANT ADD a column
Alter table rec_t1 add column c4 char(10), algorithm=instant;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	8	0	1
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	13	16711934	0
c2	2	13	16711934	0
c3	3	6	1027	0
c4	4	13	16711934	1
id	0	6	1283	0
INSERT INTO rec_t1 VALUES (3, "r3c1", "r3c2", 3, "r3c4");
SELECT * FROM rec_t1;
id	c1	c2	c3	c4
1	r1c1	r1c2	1	NULL
2	r2c1	r2c2	2	NULL
3	r3c1	r3c2	3	r3c4
# INSTANT DROP a column
Alter table rec_t1 drop column c2 , algorithm=instant;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	7	0	2
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	13	16711934	0
c3	2	6	1027	0
c4	3	13	16711934	1
id	0	6	1283	0
INSERT INTO rec_t1 VALUES (4, "r4c1", 4, "r4c4");
SELECT * FROM rec_t1;
id	c1	c3	c4
1	r1c1	1	NULL
2	r2c1	2	NULL
3	r3c1	3	r3c4
4	r4c1	4	r4c4
# add a index on instant added column
alter table rec_t1 add index idx_c4(c4);
# INSTANT MODIFY some columns
alter table rec_t1 modify column c1 varchar(20), algorithm=instant;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	7	0	3
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	12	16711695	0
c3	2	6	1027	0
c4	3	13	16711934	1
id	0	6	1283	0
insert into rec_t1 values (5, "r5c1c1c1c1c1",5, "r5c4");
select * from rec_t1;
id	c1	c3	c4
1	r1c1	1	NULL
2	r2c1	2	NULL
3	r3c1	3	r3c4
4	r4c1	4	r4c4
5	r5c1c1c1c1c1	5	r5c4
alter table rec_t1 modify column id bigint,algorithm=instant;
insert into rec_t1 values(2147483650, "r6c1", 99, "r6c4");
alter table rec_t1 modify column c3 bigint,algorithm=instant;
insert into rec_t1 values (2147493650, "r7c1", 99, "r7c4");
select * from rec_t1;
id	c1	c3	c4
1	r1c1	1	NULL
2	r2c1	2	NULL
2147483650	r6c1	99	r6c4
2147493650	r7c1	99	r7c4
3	r3c1	3	r3c4
4	r4c1	4	r4c4
5	r5c1c1c1c1c1	5	r5c4
# Make sure nothing gets flushed on disk
SET GLOBAL innodb_log_checkpoint_now = ON;
SET GLOBAL innodb_page_cleaner_disabled_debug = 1;
SET GLOBAL innodb_dict_stats_disabled_debug = 1;
SET GLOBAL innodb_master_thread_disabled_debug = 1;
UPDATE rec_t1 SET c1="r0c0";
UPDATE rec_t1 SET c3=2147503650 WHERE id=1;
update rec_t1 set id= 9981 where id=2;
INSERT INTO rec_t1 VALUES (8, "r8c1c1c1c1c1", 8, "r8c4");
DELETE FROM rec_t1 WHERE id=3;
UPDATE rec_t1 SET c3=9958 WHERE id=4;
UPDATE rec_t1 set c4='dd' where id=5;
select * from rec_t1;
id	c1	c3	c4
1	r0c0	2147503650	NULL
2147483650	r0c0	99	r6c4
2147493650	r0c0	99	r7c4
4	r0c0	9958	r4c4
5	r0c0	5	dd
8	r8c1c1c1c1c1	8	r8c4
9981	r0c0	2	NULL
checksum table rec_t1;
Table	Checksum
test.rec_t1	2250058910
select c1 from rec_t1;
c1
r0c0
r0c0
r0c0
r0c0
r0c0
r0c0
r8c1c1c1c1c1
select c3 from rec_t1;
c3
2
2147503650
5
8
99
99
9958
select c4 from rec_t1;
c4
NULL
NULL
dd
r4c4
r6c4
r7c4
r8c4
# Kill and restart
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	7	0	5
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	12	16711695	0
c3	2	6	1032	0
c4	3	13	16711934	1
id	0	6	1288	0
SELECT * FROM rec_t1;
id	c1	c3	c4
1	r0c0	2147503650	NULL
2147483650	r0c0	99	r6c4
2147493650	r0c0	99	r7c4
4	r0c0	9958	r4c4
5	r0c0	5	dd
8	r8c1c1c1c1c1	8	r8c4
9981	r0c0	2	NULL
checksum table rec_t1;
Table	Checksum
test.rec_t1	2250058910
select c1 from rec_t1;
c1
r0c0
r0c0
r0c0
r0c0
r0c0
r0c0
r8c1c1c1c1c1
select c3 from rec_t1;
c3
2
2147503650
5
8
99
99
9958
select c4 from rec_t1;
c4
NULL
NULL
dd
r4c4
r6c4
r7c4
r8c4
###########
# CLEANUP #
###########
DROP TABLE rec_t1;

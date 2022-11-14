# ------------------------------------------------------------
# Create a table with 3 columns. [id, c1, c2]
# ------------------------------------------------------------
CREATE TABLE rec_t1(id INT PRIMARY KEY, c1 CHAR(10), c2 CHAR(10)) ROW_FORMAT=dynamic;
CREATE TABLE rec_t2(id INT PRIMARY KEY, c1 CHAR(10), c2 CHAR(10)) ROW_FORMAT=compact;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	6	0	0
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	13	16711934	0
c2	2	13	16711934	0
id	0	6	1283	0
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t2	6	0	0
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	13	16711934	0
c2	2	13	16711934	0
id	0	6	1283	0
INSERT INTO rec_t1 VALUES (1, "r1c1", "r1c2");
insert into rec_t1 values (2, "r2c1", "r2c2");
SELECT * FROM rec_t1;
id	c1	c2
1	r1c1	r1c2
2	r2c1	r2c2
INSERT INTO rec_t2 VALUES (1, "r1c1", "r1c2");
insert into rec_t2 values (2, "r2c1", "r2c2");
select * from rec_t1;
id	c1	c2
1	r1c1	r1c2
2	r2c1	r2c2
# INSTANT ADD a column
Alter table rec_t1 add column c3 char(10), algorithm=instant;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	7	0	1
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	13	16711934	0
c2	2	13	16711934	0
c3	3	13	16711934	1
id	0	6	1283	0
INSERT INTO rec_t1 VALUES (3, "r3c1", "r3c2", "r3c3");
SELECT * FROM rec_t1;
id	c1	c2	c3
1	r1c1	r1c2	NULL
2	r2c1	r2c2	NULL
3	r3c1	r3c2	r3c3
alter table rec_t2 add column c3 char(10), algorithm=instant;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t2	7	0	1
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	13	16711934	0
c2	2	13	16711934	0
c3	3	13	16711934	1
id	0	6	1283	0
INSERT INTO rec_t2 VALUES (3, "r3c1", "r3c2", "r3c3");
SELECT * FROM rec_t2;
id	c1	c2	c3
1	r1c1	r1c2	NULL
2	r2c1	r2c2	NULL
3	r3c1	r3c2	r3c3
# INSTANT DROP a column
Alter table rec_t1 drop column c2 , algorithm=instant;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	6	0	2
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	13	16711934	0
c3	2	13	16711934	1
id	0	6	1283	0
INSERT INTO rec_t1 VALUES (4, "r4c1", "r4c3");
SELECT * FROM rec_t1;
id	c1	c3
1	r1c1	NULL
2	r2c1	NULL
3	r3c1	r3c3
4	r4c1	r4c3
Alter table rec_t2 drop column c2 , algorithm=instant;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t2	6	0	2
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	13	16711934	0
c3	2	13	16711934	1
id	0	6	1283	0
INSERT INTO rec_t2 VALUES (4, "r4c1", "r4c3");
SELECT * FROM rec_t2;
id	c1	c3
1	r1c1	NULL
2	r2c1	NULL
3	r3c1	r3c3
4	r4c1	r4c3
# INSTANT MODIFY a column
alter table rec_t1 modify column c1 varchar(20), algorithm=instant;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	6	0	3
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	12	16711695	0
c3	2	13	16711934	1
id	0	6	1283	0
insert into rec_t1 values (5, "r5c1c1c1c1c1", "r5c3");
select * from rec_t1;
id	c1	c3
1	r1c1	NULL
2	r2c1	NULL
3	r3c1	r3c3
4	r4c1	r4c3
5	r5c1c1c1c1c1	r5c3
alter table rec_t2 modify column c1 varchar(20), algorithm=instant;
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t2	6	0	3
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	12	16711695	0
c3	2	13	16711934	1
id	0	6	1283	0
insert into rec_t2 values (5, "r5c1c1c1c1c1", "r5c3");
select * from rec_t2;
id	c1	c3
1	r1c1	NULL
2	r2c1	NULL
3	r3c1	r3c3
4	r4c1	r4c3
5	r5c1c1c1c1c1	r5c3
# Make sure nothing gets flushed on disk
SET GLOBAL innodb_log_checkpoint_now = ON;
SET GLOBAL innodb_page_cleaner_disabled_debug = 1;
SET GLOBAL innodb_dict_stats_disabled_debug = 1;
SET GLOBAL innodb_master_thread_disabled_debug = 1;
# Make inplace update
UPDATE rec_t1 SET c1="r0c0";
# Make not-inplace update
UPDATE rec_t1 SET c3="r1c3" WHERE id=1;
# Insert a new record
INSERT INTO rec_t1 VALUES (6, "r6c1c1c1c1c1", "r6c3");
# Delete a record
DELETE FROM rec_t1 WHERE id=2;
select * from rec_t1;
id	c1	c3
1	r0c0	r1c3
3	r0c0	r3c3
4	r0c0	r4c3
5	r0c0	r5c3
6	r6c1c1c1c1c1	r6c3
checksum table rec_t1;
Table	Checksum
test.rec_t1	692983471
select * from rec_t2;
id	c1	c3
1	r1c1	NULL
2	r2c1	NULL
3	r3c1	r3c3
4	r4c1	r4c3
5	r5c1c1c1c1c1	r5c3
checksum table rec_t2;
Table	Checksum
test.rec_t2	3169136861
# Kill and restart
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t1	6	0	3
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	12	16711695	0
c3	2	13	16711934	1
id	0	6	1283	0
SELECT * FROM rec_t1;
id	c1	c3
1	r0c0	r1c3
3	r0c0	r3c3
4	r0c0	r4c3
5	r0c0	r5c3
6	r6c1c1c1c1c1	r6c3
checksum table rec_t1;
Table	Checksum
test.rec_t1	692983471
# Metadata from INFORMATION_SCHEMA.TABLES
NAME	N_COLS	INSTANT_COLS	TOTAL_ROW_VERSIONS
test/rec_t2	6	0	3
# Metadata from INFORMATION_SCHEMA.COLUMNS
NAME	POS	MTYPE	PRTYPE	HAS_DEFAULT
c1	1	12	16711695	0
c3	2	13	16711934	1
id	0	6	1283	0
SELECT * FROM rec_t2;
id	c1	c3
1	r1c1	NULL
2	r2c1	NULL
3	r3c1	r3c3
4	r4c1	r4c3
5	r5c1c1c1c1c1	r5c3
checksum table rec_t2;
Table	Checksum
test.rec_t2	3169136861
###########
# CLEANUP #
###########
DROP TABLE rec_t1;
drop table rec_t2;

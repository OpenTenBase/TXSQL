#
# This test upgrades a 5.7 database with tables having instant column
#
# Stop the running server
# Restart the server with the unzipped datadir
# restart: --datadir=DATADIR --innodb_page_size=16k --log-error=MYSQLD_LOG_UPGRADE
SHOW TABLES;
Tables_in_test
t1
SELECT * FROM t1;
a	b	c1
1	1	NULL
2	2	NULL
3	3	NULL
4	4	NULL
5	5	NULL
select * from information_schema.INNODB_TABLES WHERE NAME='test/t1';
TABLE_ID	NAME	FLAG	N_COLS	SPACE	ROW_FORMAT	ZIP_PAGE_SIZE	SPACE_TYPE	INSTANT_COLS
1061	test/t1	33	6	23	Dynamic	0	Single	2
SELECT * FROM information_schema.innodb_columns sc
INNER JOIN information_schema.innodb_tables st
ON sc.TABLE_ID=st.TABLE_ID
WHERE st.NAME='test/t1' ORDER BY sc.pos, sc.NAME;
TABLE_ID	NAME	POS	MTYPE	PRTYPE	LEN	HAS_DEFAULT	DEFAULT_VALUE	TABLE_ID	NAME	FLAG	N_COLS	SPACE	ROW_FORMAT	ZIP_PAGE_SIZE	SPACE_TYPE	INSTANT_COLS
1061	a	0	6	1283	4	0	NULL	1061	test/t1	33	6	23	Dynamic	0	Single	2
1061	b	1	6	1027	4	0	NULL	1061	test/t1	33	6	23	Dynamic	0	Single	2
1061	c1	2	6	1027	4	1	NULL	1061	test/t1	33	6	23	Dynamic	0	Single	2
ALTER TABLE t1 ADD COLUMN c2 INT, ALGORITHM=INSTANT;
SELECT * FROM t1;
a	b	c1	c2
1	1	NULL	NULL
2	2	NULL	NULL
3	3	NULL	NULL
4	4	NULL	NULL
5	5	NULL	NULL
select * from information_schema.INNODB_TABLES WHERE NAME='test/t1';
TABLE_ID	NAME	FLAG	N_COLS	SPACE	ROW_FORMAT	ZIP_PAGE_SIZE	SPACE_TYPE	INSTANT_COLS
1061	test/t1	33	7	23	Dynamic	0	Single	2
SELECT * FROM information_schema.innodb_columns sc
INNER JOIN information_schema.innodb_tables st
ON sc.TABLE_ID=st.TABLE_ID
WHERE st.NAME='test/t1' ORDER BY sc.pos, sc.NAME;
TABLE_ID	NAME	POS	MTYPE	PRTYPE	LEN	HAS_DEFAULT	DEFAULT_VALUE	TABLE_ID	NAME	FLAG	N_COLS	SPACE	ROW_FORMAT	ZIP_PAGE_SIZE	SPACE_TYPE	INSTANT_COLS
1061	a	0	6	1283	4	0	NULL	1061	test/t1	33	7	23	Dynamic	0	Single	2
1061	b	1	6	1027	4	0	NULL	1061	test/t1	33	7	23	Dynamic	0	Single	2
1061	c1	2	6	1027	4	1	NULL	1061	test/t1	33	7	23	Dynamic	0	Single	2
1061	c2	3	6	1027	4	1	NULL	1061	test/t1	33	7	23	Dynamic	0	Single	2
# Shutdown server
# Restart for checking.
# restart: --datadir=DATADIR --innodb_page_size=16k --log-error=MYSQLD_LOG_UPGRADE
SELECT * FROM t1;
a	b	c1	c2
1	1	NULL	NULL
2	2	NULL	NULL
3	3	NULL	NULL
4	4	NULL	NULL
5	5	NULL	NULL
select * from information_schema.INNODB_TABLES WHERE NAME='test/t1';
TABLE_ID	NAME	FLAG	N_COLS	SPACE	ROW_FORMAT	ZIP_PAGE_SIZE	SPACE_TYPE	INSTANT_COLS
1061	test/t1	33	7	23	Dynamic	0	Single	2
SELECT * FROM information_schema.innodb_columns sc
INNER JOIN information_schema.innodb_tables st
ON sc.TABLE_ID=st.TABLE_ID
WHERE st.NAME='test/t1' ORDER BY sc.pos, sc.NAME;
TABLE_ID	NAME	POS	MTYPE	PRTYPE	LEN	HAS_DEFAULT	DEFAULT_VALUE	TABLE_ID	NAME	FLAG	N_COLS	SPACE	ROW_FORMAT	ZIP_PAGE_SIZE	SPACE_TYPE	INSTANT_COLS
1061	a	0	6	1283	4	0	NULL	1061	test/t1	33	7	23	Dynamic	0	Single	2
1061	b	1	6	1027	4	0	NULL	1061	test/t1	33	7	23	Dynamic	0	Single	2
1061	c1	2	6	1027	4	1	NULL	1061	test/t1	33	7	23	Dynamic	0	Single	2
1061	c2	3	6	1027	4	1	NULL	1061	test/t1	33	7	23	Dynamic	0	Single	2
# Cleanup
# Shutdown server
# Restart the server
# restart

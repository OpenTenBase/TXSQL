#
# modify generated cols
#
CREATE TABLE t1 (a INT, b INT GENERATED ALWAYS AS (a), c CHAR(10), d CHAR(20), e CHAR(10) GENERATED ALWAYS AS (c), g INT);
INSERT INTO t1 VALUES(10, DEFAULT, "aa", "bb", DEFAULT, 20);
INSERT INTO t1 VALUES(11, DEFAULT, "jj", "kk", DEFAULT, 21);
select * from t1;
a	b	c	d	e	g
10	10	aa	bb	aa	20
11	11	jj	kk	jj	21
alter table t1 modify g bigint, algorithm=instant;
select * from t1;
a	b	c	d	e	g
10	10	aa	bb	aa	20
11	11	jj	kk	jj	21
alter table t1 modify a bigint, algorithm=instant;
select * from t1;
a	b	c	d	e	g
10	10	aa	bb	aa	20
11	11	jj	kk	jj	21
update t1 set g=2;
select * from t1;
a	b	c	d	e	g
10	10	aa	bb	aa	2
11	11	jj	kk	jj	2
alter table t1 modify b bigint, algorithm=instant;
ERROR HY000: 'Changing the STORED status' is not supported for generated columns.
alter table t1 modify e varchar(20), algorithm=instant;
ERROR HY000: 'Changing the STORED status' is not supported for generated columns.
alter table t1 modify c varchar(20), algorithm=instant;
INSERT INTO t1 VALUES(12, DEFAULT, "jj", "kk", DEFAULT, 21);
select * from t1;
a	b	c	d	e	g
10	10	aa	bb	aa	2
11	11	jj	kk	jj	2
12	12	jj	kk	jj	21
update t1 set g=22;
select * from t1;
a	b	c	d	e	g
10	10	aa	bb	aa	22
11	11	jj	kk	jj	22
12	12	jj	kk	jj	22
update t1 set a=2;
select * from t1;
a	b	c	d	e	g
2	2	aa	bb	aa	22
2	2	jj	kk	jj	22
2	2	jj	kk	jj	22
update t1 set c=2;
select * from t1;
a	b	c	d	e	g
2	2	2	bb	2	22
2	2	2	kk	2	22
2	2	2	kk	2	22
checksum table t1;
Table	Checksum
test.t1	122533898
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
3
drop table t1;

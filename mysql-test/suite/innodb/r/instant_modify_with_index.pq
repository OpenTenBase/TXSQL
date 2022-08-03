#
# test add index on instant modified column  - char
#
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a char(10), b varchar(10));
INSERT INTO t1 VALUES(1, repeat('ax', 4), repeat('b', 8));
INSERT INTO t1 VALUES(2, repeat('xa', 4), repeat('b', 8));
INSERT INTO t1 VALUES(3, repeat('ac', 4), repeat('b', 8));
alter table t1 modify a char(20), algorithm=instant;
INSERT INTO t1 VALUES(4, repeat('ab', 8), repeat('b', 8));
INSERT INTO t1 VALUES(5, repeat('a', 16), repeat('b', 8));
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
1
select * from t1;
id	a	b
1	axaxaxax	bbbbbbbb
2	xaxaxaxa	bbbbbbbb
3	acacacac	bbbbbbbb
4	abababababababab	bbbbbbbb
5	aaaaaaaaaaaaaaaa	bbbbbbbb
alter table t1 add index idx(a);
INSERT INTO t1 VALUES(6, repeat('x', 16), repeat('x', 8));
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
1
select * from t1;
id	a	b
1	axaxaxax	bbbbbbbb
2	xaxaxaxa	bbbbbbbb
3	acacacac	bbbbbbbb
4	abababababababab	bbbbbbbb
5	aaaaaaaaaaaaaaaa	bbbbbbbb
6	xxxxxxxxxxxxxxxx	xxxxxxxx
alter table t1 modify b char(20), algorithm=instant;
INSERT INTO t1 VALUES(7, repeat('y', 16), repeat('y', 16));
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
2
select * from t1;
id	a	b
1	axaxaxax	bbbbbbbb
2	xaxaxaxa	bbbbbbbb
3	acacacac	bbbbbbbb
4	abababababababab	bbbbbbbb
5	aaaaaaaaaaaaaaaa	bbbbbbbb
6	xxxxxxxxxxxxxxxx	xxxxxxxx
7	yyyyyyyyyyyyyyyy	yyyyyyyyyyyyyyyy
select a from t1 force index(idx);
a
aaaaaaaaaaaaaaaa
abababababababab
acacacac
axaxaxax
xaxaxaxa
xxxxxxxxxxxxxxxx
yyyyyyyyyyyyyyyy
select a from t1 order by a;
a
aaaaaaaaaaaaaaaa
abababababababab
acacacac
axaxaxax
xaxaxaxa
xxxxxxxxxxxxxxxx
yyyyyyyyyyyyyyyy
checksum table t1;
Table	Checksum
test.t1	2064074107
drop table t1;
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
0
#
# test add index on instant modified column  - varchar
#
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a varchar(10), b varchar(10));
INSERT INTO t1 VALUES(1, repeat('ax', 4), repeat('b', 8));
INSERT INTO t1 VALUES(2, repeat('xa', 4), repeat('b', 8));
INSERT INTO t1 VALUES(3, repeat('ac', 4), repeat('b', 8));
alter table t1 modify a char(20), algorithm=instant;
INSERT INTO t1 VALUES(4, repeat('ab', 8), repeat('b', 8));
INSERT INTO t1 VALUES(5, repeat('a', 16), repeat('b', 8));
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
1
select * from t1;
id	a	b
1	axaxaxax	bbbbbbbb
2	xaxaxaxa	bbbbbbbb
3	acacacac	bbbbbbbb
4	abababababababab	bbbbbbbb
5	aaaaaaaaaaaaaaaa	bbbbbbbb
alter table t1 add index idx(a);
INSERT INTO t1 VALUES(6, repeat('x', 16), repeat('x', 8));
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
1
select * from t1;
id	a	b
1	axaxaxax	bbbbbbbb
2	xaxaxaxa	bbbbbbbb
3	acacacac	bbbbbbbb
4	abababababababab	bbbbbbbb
5	aaaaaaaaaaaaaaaa	bbbbbbbb
6	xxxxxxxxxxxxxxxx	xxxxxxxx
alter table t1 modify b char(20), algorithm=instant;
INSERT INTO t1 VALUES(7, repeat('y', 16), repeat('y', 16));
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
2
select * from t1;
id	a	b
1	axaxaxax	bbbbbbbb
2	xaxaxaxa	bbbbbbbb
3	acacacac	bbbbbbbb
4	abababababababab	bbbbbbbb
5	aaaaaaaaaaaaaaaa	bbbbbbbb
6	xxxxxxxxxxxxxxxx	xxxxxxxx
7	yyyyyyyyyyyyyyyy	yyyyyyyyyyyyyyyy
select a from t1 force index(idx);
a
aaaaaaaaaaaaaaaa
abababababababab
acacacac
axaxaxax
xaxaxaxa
xxxxxxxxxxxxxxxx
yyyyyyyyyyyyyyyy
select a from t1 order by a;
a
aaaaaaaaaaaaaaaa
abababababababab
acacacac
axaxaxax
xaxaxaxa
xxxxxxxxxxxxxxxx
yyyyyyyyyyyyyyyy
checksum table t1;
Table	Checksum
test.t1	2064074107
drop table t1;
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
0
#
# test add index on instant modified column  - int
#
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a int);
INSERT INTO t1(a) VALUES(5);
INSERT INTO t1(a) VALUES(1);
INSERT INTO t1(a) VALUES(1000);
INSERT INTO t1(a) VALUES(-1);
INSERT INTO t1(a) VALUES(0);
INSERT INTO t1(a) VALUES(-100);
alter table t1 modify a bigint, algorithm=instant;
SELECT * FROM t1;
id	a
1	5
2	1
3	1000
4	-1
5	0
6	-100
INSERT INTO t1(a) VALUES(2);
INSERT INTO t1(a) VALUES(pow(2, 40));
INSERT INTO t1(a) VALUES(pow(2, 60));
alter table t1 add key idx(a);
INSERT INTO t1(a) VALUES(10);
select * from t1 where a=-1;
id	a
4	-1
select * from t1 where a=5;
id	a
1	5
select a from t1 force index(idx);
a
-1
-100
0
1
10
1000
1099511627776
1152921504606846976
2
5
checksum table t1;
Table	Checksum
test.t1	1978878417
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
1
drop table t1;
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a mediumint);
INSERT INTO t1(a) VALUES(5);
INSERT INTO t1(a) VALUES(1);
INSERT INTO t1(a) VALUES(1000);
INSERT INTO t1(a) VALUES(-1);
INSERT INTO t1(a) VALUES(0);
INSERT INTO t1(a) VALUES(-100);
alter table t1 modify a bigint, algorithm=instant;
SELECT * FROM t1;
id	a
1	5
2	1
3	1000
4	-1
5	0
6	-100
INSERT INTO t1(a) VALUES(2);
INSERT INTO t1(a) VALUES(pow(2, 40));
INSERT INTO t1(a) VALUES(pow(2, 60));
alter table t1 add key idx(a);
INSERT INTO t1(a) VALUES(10);
select * from t1 where a=-1;
id	a
4	-1
select * from t1 where a=5;
id	a
1	5
select a from t1 force index(idx);
a
-1
-100
0
1
10
1000
1099511627776
1152921504606846976
2
5
checksum table t1;
Table	Checksum
test.t1	1978878417
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
1
drop table t1;
CREATE TABLE t1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, a tinyint);
INSERT INTO t1(a) VALUES(5);
INSERT INTO t1(a) VALUES(1);
INSERT INTO t1(a) VALUES(10);
INSERT INTO t1(a) VALUES(-1);
INSERT INTO t1(a) VALUES(0);
INSERT INTO t1(a) VALUES(-10);
alter table t1 modify a int, algorithm=instant;
SELECT * FROM t1;
id	a
1	5
2	1
3	10
4	-1
5	0
6	-10
INSERT INTO t1(a) VALUES(2);
INSERT INTO t1(a) VALUES(pow(2, 30));
INSERT INTO t1(a) VALUES(pow(2, 20));
alter table t1 add key idx(a);
INSERT INTO t1(a) VALUES(10);
select * from t1 where a=-1;
id	a
4	-1
select * from t1 where a=5;
id	a
1	5
select a from t1 force index(idx);
a
-1
-10
0
1
10
10
1048576
1073741824
2
5
checksum table t1;
Table	Checksum
test.t1	2038431323
select count(*) from information_schema.innodb_instant_modified_cols;
count(*)
1
drop table t1;

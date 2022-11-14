create database d1;
drop table if exists mysql.tdsql_sequences;
drop database d1;
CREATE TABLE mysql.tdsql_sequences(
db varchar(128) not null,
name varchar(128) COLLATE utf8_bin not null,
curval bigint not null, start bigint not null,
step int not null,
max_value bigint not null,
min_value bigint not null,
do_cycle bool not null, n_cache int unsigned not null,
primary key(db,name)) engine=innodb, charset=utf8 tablespace=mysql;
Warnings:
Warning	3778	'utf8mb3_bin' is a collation of the deprecated character set UTF8MB3. Please consider using UTF8MB4 with an appropriate collation instead.
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
create database d2;
drop table if exists mysql.tdsql_sequences;
drop database d2;
create database d3;
drop database d3;
create database d3;
use d3;
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_increment by 1 TDSQL_MAXVALUE 50 TDSQL_minvalue 0 TDSQL_nocycle;
CREATE TABLE t0(a int PRIMARY KEY);
SELECT * from mysql.tdsql_sequences where db='d3' and name='sequence_000';
db	name	curval	start	step	max_value	min_value	do_cycle	n_cache
d3	sequence_000	-9223372036854775808	0	1	50	0	0	20
INSERT INTO t0 values(sequence_000.nextval);
INSERT INTO t0 values(sequence_000.nextval);
commit;
DROP TDSQL_SEQUENCE sequence_000 ;
CREATE TABLE if not exists mysql.tdsql_sequences(
db varchar(128) not null,
name varchar(128) COLLATE utf8_bin not null,
curval bigint not null, start bigint not null,
step int not null,
max_value bigint not null,
min_value bigint not null,
do_cycle bool not null, n_cache int unsigned not null,
primary key(db,name)) engine=innodb, charset=utf8 tablespace=mysql;
Warnings:
Warning	3778	'utf8mb3_bin' is a collation of the deprecated character set UTF8MB3. Please consider using UTF8MB4 with an appropriate collation instead.
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
Note	1050	Table 'tdsql_sequences' already exists
drop database d3;
drop table if exists mysql.tdsql_sequences;
create database d3;
drop database d3;
CREATE TABLE if not exists mysql.tdsql_sequences(
db varchar(128) not null,
name varchar(128) COLLATE utf8_bin not null,
curval bigint not null, start bigint not null,
step int not null,
max_value bigint not null,
min_value bigint not null,
do_cycle bool not null, n_cache int unsigned not null,
primary key(db,name)) engine=innodb, charset=utf8 tablespace=mysql;
Warnings:
Warning	3778	'utf8mb3_bin' is a collation of the deprecated character set UTF8MB3. Please consider using UTF8MB4 with an appropriate collation instead.
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.

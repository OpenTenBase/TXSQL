create table t1(id int not null primary key, c1 varchar(10)) default character set=latin1;
alter table t1 modify column c1 char(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 varchar(10)) default character set=utf8mb3;
Warnings:
Warning	1287	'utf8mb3' is deprecated and will be removed in a future release. Please use utf8mb4 instead
alter table t1 modify column c1 char(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 varchar(10)) default character set=binary;
alter table t1 modify column c1 char(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 varchar(10)) default character set=utf8mb4;
alter table t1 modify column c1 char(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 char(10)) default character set=latin1;
alter table t1 modify column c1 varchar(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 char(10)) default character set=utf8mb3;
Warnings:
Warning	1287	'utf8mb3' is deprecated and will be removed in a future release. Please use utf8mb4 instead
alter table t1 modify column c1 varchar(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 char(10)) default character set=binary;
alter table t1 modify column c1 varchar(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 char(10)) default character set=utf8mb4;
alter table t1 modify column c1 varchar(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 binary(10)) default character set=latin1;
alter table t1 modify column c1 varbinary(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 binary(10)) default character set=utf8mb3;
Warnings:
Warning	1287	'utf8mb3' is deprecated and will be removed in a future release. Please use utf8mb4 instead
alter table t1 modify column c1 varbinary(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 binary(10)) default character set=binary;
alter table t1 modify column c1 varbinary(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 binary(10)) default character set=utf8mb4;
alter table t1 modify column c1 varbinary(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 binary(10)) default character set=gbk;
alter table t1 modify column c1 varbinary(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 binary(10)) default character set=utf16le;
alter table t1 modify column c1 varbinary(20),algorithm=instant;
drop table t1;
create table t1(id int not null primary key, c1 char(10)) default character set=gbk;
alter table t1 modify column c1 varchar(20),algorithm=instant;
ERROR 0A000: ALGORITHM=INSTANT is not supported. Reason: Need to rebuild the table to change column type. Try ALGORITHM=COPY/INPLACE.
drop table t1;
create table t1(id int not null primary key, c1 varchar(10)) default character set=utf16le;
alter table t1 modify column c1 char(20),algorithm=instant;
ERROR 0A000: ALGORITHM=INSTANT is not supported. Reason: Need to rebuild the table to change column type. Try ALGORITHM=COPY/INPLACE.
drop table t1;

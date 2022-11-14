CREATE TABLE `sbtest1` (
`id` int NOT NULL default '0',
`k` int NOT NULL DEFAULT '0',
`c` char(120) NOT NULL DEFAULT '',
`pad` char(60) NOT NULL DEFAULT '',
key k_1(k));
alter table sbtest1 modify column id bigint not null default '0',algorithm=instant;
select name,pos,version,mtype,prtype,len from information_schema.innodb_instant_modified_cols;
NAME	POS	VERSION	MTYPE	PRTYPE	LEN
id	1	1	6	1283	4
alter table sbtest1 drop index k_1;
select name,pos,version,mtype,prtype,len from information_schema.innodb_instant_modified_cols;
NAME	POS	VERSION	MTYPE	PRTYPE	LEN
id	1	1	6	1283	4
drop table sbtest1;

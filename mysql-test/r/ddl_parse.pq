create database testdb;
use testdb;
PARSER alter table aa add b int after c ,add d int;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	ALTER TABLE	ADD_COLUMN,COLUMN_ORDER	
PARSER alter table aa add b int after c ,add xxxxxxxxxxxxd int,add xx1xxxxxxxxxxd int,add x1xxxxxxxxxxxd int,add xxx1xxxxxxxxxd int,add x2xxxxxxxxxxxd int,add xx2xxxxxxxxxxd int,add x2xxxxxxxxxxxd int,add xxx2xxxxxxxxxd int      ,add x3xxxxxxxxxxxd int,add xx31xxxxxxxxxxd int,add x1x3xxxxxxxxxxd int,add xxx3xxxxxxxxxd int,add x2xxx3xxxxxxxxd int,add xx2xxx3xxxxxxxd int,add x2xxxxx3xxxxxxd int,add xxx2xxxx3xxxxxd int
,add x4xxxxxxxxxxxd int,add xx4xxxxxxxxxxd int,add x1xx4xxxxxxxxxd int,add xxx1xx4xxxxxxxd int,add x2xxxx4xxxxxxxd int,add xx2xxxxx4xxxxxd int,add x2xxxxxxx4xxxxd int,add xxx2xxxxxx4xxxd int      ,add x3xxxxxxxxx4xxd int,add xx31xxxxxxxxx4xd int,add x1x3xxxxxxxxxx4d int,add xxx3xxxxxxxxxzd int,add x2xxx3xxxxxxxxzxd int,add xx2xxx3xxxxxzxxd int,add x2xxxxxz3xxxxxxd int,add xxx2xxxx3zxxxxxd int;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	ALTER TABLE	ADD_COLUMN,COLUMN_ORDER	
PARSER alter table aa add index aain(b);
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	ALTER TABLE	ADD_INDEX	
PARSER alter table aa drop primary key;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	ALTER TABLE	DROP_INDEX	
PARSER alter table aa drop aa;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	ALTER TABLE	DROP_COLUMN	
parser ALTER TABLE t1 ADD PARTITION (PARTITION p3 VALUES LESS THAN (2002));
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	t1	ALTER TABLE	ADD_PARTITION	
PARSER alter table aa  DROP PRIMARY KEY;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	ALTER TABLE	DROP_INDEX	
parser create table aa(b int);
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	CREATE_TABLE		InnoDB
parser create tablespace aa add datafile 'fdsfs' USE LOGFILE GROUP dd ENGINE =innodb;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
		UNKNOWN DDL TYPE		
parser create database db1;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
		CREATE_DB		
parser create index aa on t1(b);
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	t1	CREATE_INDEX		
parser truncate table tt;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	tt	TRUNCATE_TABLE		
parser drop table a,cc.b,ee,f,dd.aaa,ddddddf.fdfssdafds,ewrwfdsf.afdsafdsaf,fdsfsafsafe.jkjkjkjk,dddd1ddf.fdfssdafds,ewrw2fdsf.afds3afdsaf,fdsfsa4fsafe.jk3jkjkjk,ddddddf.fdfs3sdafds,ewrwfdsf.afdsaf3dsaf,fdsfsafsafe.jkjk3jkjk,dddd1ddf.fdfss3dafds,ewrw2fdsf.afd3s3afdsaf,fds2fsa4fsafe.jkj2kjkjk    ,dddd1ddf.fdfssd4afds,ewrw2fdsf.afds43afdsaf,fds4fsa4fsafe.jk34jkjkjk,ddddddf.fdfs34sdafds,ewrwfdsf.afdsaf43dsaf,fdsfsafsafe.jkjk34jkjk,dddd1ddf.fdf4ss3dafds,ewrw2fdsf.afd3s43afdsaf,fds24fsa4fsafe.jkj42kjDDD    ,dddd1ddf.fdfssd45afds,ewrw2fdsf.afds43afd5saf,fds4fsa4fsafe.jk354jkjkjk,ddddddf.fdfs534sdafds,ewrwfdsf.afdsaf543dsaf,fdsfsafsafe.jkjk345jkjk,dddd1ddf.fdf4s5s3dafds,ewrw2fdsf.afd3s43a5fdsaf,fds24fsa4fsafe.jkj42kj5kjkAA;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	a	DROP_TABLE		
cc	b	DROP_TABLE		
testdb	ee	DROP_TABLE		
testdb	f	DROP_TABLE		
dd	aaa	DROP_TABLE		
ddddddf	fdfssdafds	DROP_TABLE		
ewrwfdsf	afdsafdsaf	DROP_TABLE		
fdsfsafsafe	jkjkjkjk	DROP_TABLE		
dddd1ddf	fdfssdafds	DROP_TABLE		
ewrw2fdsf	afds3afdsaf	DROP_TABLE		
fdsfsa4fsafe	jk3jkjkjk	DROP_TABLE		
ddddddf	fdfs3sdafds	DROP_TABLE		
ewrwfdsf	afdsaf3dsaf	DROP_TABLE		
fdsfsafsafe	jkjk3jkjk	DROP_TABLE		
dddd1ddf	fdfss3dafds	DROP_TABLE		
ewrw2fdsf	afd3s3afdsaf	DROP_TABLE		
fds2fsa4fsafe	jkj2kjkjk	DROP_TABLE		
dddd1ddf	fdfssd4afds	DROP_TABLE		
ewrw2fdsf	afds43afdsaf	DROP_TABLE		
fds4fsa4fsafe	jk34jkjkjk	DROP_TABLE		
ddddddf	fdfs34sdafds	DROP_TABLE		
ewrwfdsf	afdsaf43dsaf	DROP_TABLE		
fdsfsafsafe	jkjk34jkjk	DROP_TABLE		
dddd1ddf	fdf4ss3dafds	DROP_TABLE		
ewrw2fdsf	afd3s43afdsaf	DROP_TABLE		
fds24fsa4fsafe	jkj42kjDDD	DROP_TABLE		
dddd1ddf	fdfssd45afds	DROP_TABLE		
ewrw2fdsf	afds43afd5saf	DROP_TABLE		
fds4fsa4fsafe	jk354jkjkjk	DROP_TABLE		
ddddddf	fdfs534sdafds	DROP_TABLE		
ewrwfdsf	afdsaf543dsaf	DROP_TABLE		
fdsfsafsafe	jkjk345jkjk	DROP_TABLE		
dddd1ddf	fdf4s5s3dafds	DROP_TABLE		
ewrw2fdsf	afd3s43a5fdsaf	DROP_TABLE		
fds24fsa4fsafe	jkj42kj5kjkAA	DROP_TABLE		
parser drop  index aa on t1;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	t1	DROP_INDEX		
parser drop view cc;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	cc	DROP_VIEW		
parser create table aa(b int);
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	CREATE_TABLE		InnoDB
parser create table aa(b int) ENGINE =innodb;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	CREATE_TABLE		InnoDB
parser create table aa(b int) ENGINE =myisam;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	CREATE_TABLE		OTHER
PARSER alter table aa add b int after c ,drop d ;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	aa	ALTER TABLE	ADD_COLUMN,DROP_COLUMN,COLUMN_ORDER	
parser drop  index aa on t1;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	t1	DROP_INDEX		
parser drop view cc;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	cc	DROP_VIEW		
parser ANALYZE TABLE t1;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	t1	ANALYZE_TABLE		
parser optimize TABLE t1;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	t1	OPTIMIZE_TABLE		
parser create table at like b;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	at	CREATE_TABLE		InnoDB
testdb	b	CREATE_TABLE		InnoDB
parser create table x1 as select *from t1;
DB_NAME	TABLE_NAME	DDL_TYPE	SPECIFIC_TYPE	STORAGE_TYPE
testdb	x1	CREATE_TABLE		InnoDB
testdb	t1	CREATE_TABLE		InnoDB
drop database testdb;

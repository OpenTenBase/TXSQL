CREATE TABLE `t1` (`id` int NOT NULL AUTO_INCREMENT, `k` int NOT NULL DEFAULT '0',
`c` char(120) NOT NULL DEFAULT '', `pad` text, 
PRIMARY KEY (`id`), KEY `k_1` (`k`)) ENGINE=InnoDB;
alter table t1 drop column pad;
alter table t1 add column pad text, add column i int,  add column j char(250) default 'abc', add column n int default 1,algorithm=instant;
SET @old_sort_buf_size= @@sort_buffer_size;
SET GLOBAL sort_buffer_size=4096;
Warnings:
Warning	1292	Truncated incorrect sort_buffer_size value: '4096'
alter table t1 add FULLTEXT INDEX fullkey(pad,j);
Warnings:
Warning	124	InnoDB rebuilding table to add column FTS_DOC_ID
SET GLOBAL sort_buffer_size=@old_sort_buf_size;
drop table t1;

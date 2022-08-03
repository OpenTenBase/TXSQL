SET @saved_innodb_stats_persistent = @@GLOBAL.innodb_stats_persistent;
SET @saved_persistent_sample_pages = @@GLOBAL.innodb_stats_persistent_sample_pages;
SET @saved_skip_adjustment_for_primary_key = @@innodb_stats_skip_adjustment_for_primary_key;
create table t(c1 int primary key, c2 text(8000)) engine=innodb;
insert into t value(10000, repeat('a',8000));
# Insert 9999 big records. All leaf pages will have one row except for the first.
set global innodb_stats_persistent=on;
set global innodb_stats_persistent_sample_pages=20;
SET GLOBAL innodb_stats_skip_adjustment_for_primary_key=ON;
analyze table t;
Table	Op	Msg_type	Msg_text
test.t	analyze	status	OK
select TABLE_ROWS from information_schema.tables where table_name ='t';
TABLE_ROWS
9998
SET GLOBAL innodb_stats_skip_adjustment_for_primary_key=OFF;
analyze table t;
Table	Op	Msg_type	Msg_text
test.t	analyze	status	OK
select TABLE_ROWS from information_schema.tables where table_name ='t';
TABLE_ROWS
0
drop table t;
SET GLOBAL innodb_stats_persistent = @saved_innodb_stats_persistent;
SET GLOBAL innodb_stats_persistent_sample_pages = @saved_persistent_sample_pages;
SET GLOBAL innodb_stats_skip_adjustment_for_primary_key = @saved_skip_adjustment_for_primary_key;

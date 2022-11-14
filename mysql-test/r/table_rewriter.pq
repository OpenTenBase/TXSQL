# shut server down
# Server is down
# create bootstrap file
# Restart the server against DDIR
create test user no previlige on query_rerite
select PLUGIN_NAME, PLUGIN_STATUS, PLUGIN_LIBRARY,PLUGIN_DESCRIPTION from information_schema.plugins where plugin_name='table_rewriter';
PLUGIN_NAME	PLUGIN_STATUS	PLUGIN_LIBRARY	PLUGIN_DESCRIPTION
Table_rewriter	ACTIVE	table_rewriter.so	A table name rewrite plugin that rewrites table name in the  parse tree.
create table t1(a int primary key);
insert into t1 values(1),(2),(3);
create user 'test'@'localhost';
grant all on test.* to 'test'@'localhost';
set tdsql_allow_access_internal_table = on;
select load_table_rewriter_rules();
load_table_rewriter_rules()
NULL
set @a=@@tx_isolation, tx_isolation='REPEATABLE-READ' , @b= @@binlog_format, binlog_format='STATEMENT';
insert ignore query_rewrite.table_rewrite_rules values('test', 't1@1', 't1');
select * from query_rewrite.table_rewrite_rules;
db	table_name	table_name_new
test	t1@1	t1
select * from `t1@1` as t1 where t1.a=1;
a
1
insert into `t1@1` values(4),(5),(6);
update `t1@1` set a=7 where a=6;
set @cmd="insert into `t1@1` values(8),(9),(10);";
prepare x from @cmd;
execute x;
drop prepare x;
delete from query_rewrite.table_rewrite_rules where db='test' and table_name='t1#2';
select * from query_rewrite.table_rewrite_rules;
db	table_name	table_name_new
test	t1@1	t1
set binlog_format=@b, tx_isolation=@a;
# shut server down
# Server is down
# close the test connection
# delete mysqld log
# delete datadir

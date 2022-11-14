create table t1 (a int, b varchar(100), primary key(a)) engine=innodb;
insert into t1 values (1, "hello");
insert into t1 values (2, "hello, lucas");
show variables like 'resultset_metadata';
Variable_name	Value
resultset_metadata	FULL
select * from  t1;
a	b
1	hello
2	hello, lucas
set session resultset_metadata = 'ALIAS_NAME';
select * from t1;
a	b
1	hello
2	hello, lucas
select a as alias_a, b as alias_b from t1 as alias_t1;
alias_a	alias_b
1	hello
2	hello, lucas
select a from t1;
a
1
2
set session resultset_metadata = "COLUMN_NAME_ONLY";
select * from t1;
a	b
1	hello
2	hello, lucas
select a as alias_a, b as alias_b from t1 as alias_t1;
alias_a	alias_b
1	hello
2	hello, lucas
select a from t1;
a
1
2
set session resultset_metadata = "FULL";
drop table t1;

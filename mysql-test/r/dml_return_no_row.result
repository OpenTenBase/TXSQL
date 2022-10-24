create table t1 (a int, b int) engine=innodb;
insert into t1 values (1,2),(2,3), (3,4);
delete from t1 where 0 txsql_returning *;
a	b
delete from t1 where a = 2 txsql_returning *;
a	b
2	3
delete from t1 where a = 12 txsql_returning *;
a	b
update t1 set b = b + 1 where 0 txsql_returning *;
a	b
update t1 set b = b + 1 where 1 txsql_returning *;
a	b
1	3
3	5
update t1 set b = b + 1 where a = 3 txsql_returning *;
a	b
3	6
update t1 set b = b + 1 where a = 13 txsql_returning *;
a	b
drop table t1;
create table t2 (id varchar(36), value int, primary key(`id`))
PARTITION BY LIST(murmurHashCodeAndMod(id,64)) (
PARTITION p4 VALUES IN (4),
PARTITION p10 VALUES IN (10)
);
delete from t2 where (id = '1') txsql_returning *;
id	value
delete from t2 where (id like '%1%') txsql_returning *;
id	value
update t2 set value = value + 1 where (id = '1') txsql_returning *;
id	value
update t2 set value = value + 1 where (id like '%1%') txsql_returning *;
id	value
drop table t2;

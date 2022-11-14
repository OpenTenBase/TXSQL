create table t(id int primary key, c1 int, c2 varchar(1024));
select group_concat(t.c2 order by t.c1 separator',') as test from t;
test
1,1,22,22,3333,44444444,555555555,6666666666,6666666666,8888888888888888,99999999999999999999999999999999,9999999999999999999999999999999999999999999999999999999999999999
set group_concat_max_len=10;
select group_concat(t.c2 order by t.c1 separator',') as test from t;
test
1,1,22,22,
Warnings:
Warning	1260	Row 5 was cut by GROUP_CONCAT()
drop table t;
create table t(id int primary key, c1 int, c2 varchar(1010)) charset=latin1;
select group_concat(t.c2 order by t.c1 separator',') as test from t;
test
1,1,22,22,
Warnings:
Warning	1260	Row 5 was cut by GROUP_CONCAT()
drop table t;

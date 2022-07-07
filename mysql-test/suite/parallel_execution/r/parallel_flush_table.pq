### 
### Test parallel query while flush tables
###
drop table if exists t1;
# --> connect default
create table t1 (a int);
insert into t1 values(1),(2),(3);
select * from t1;
a
2
1
3
# sending lock table t1 read ...
lock table t1 read;
# --> connect con1
# sending flush table t1 ...
flush table t1;
# Let flush table sync in.
select * from t1;
a
1
2
3
#--> connection con2
select * from t1;
# --> connection default
# con2 cannot do select while flushing
select * from t1;
a
1
2
3
unlock tables;
drop table t1;

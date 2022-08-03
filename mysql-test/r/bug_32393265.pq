#
# WL#8084: subquery as udf arguement cause instance crash
#

#Prepare test enviroment
create database tdb;
use tdb;
create table t1(f1 int);
insert into t1 values (1);
insert into t1 values (2);
CREATE FUNCTION func1 RETURNS INTEGER SONAME "udf_example.so";
select func1((select f1 from t1));
ERROR 21000: Subquery returns more than 1 row
#Test for window function
select f1 , rank() over(order by f1) from t1;
f1	rank() over(order by f1)
1	1
2	2
#Cleanup
drop database tdb;
drop function func1;

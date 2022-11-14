show global variables like '%_isolation';
Variable_name	Value
transaction_isolation	REPEATABLE-READ
tx_isolation	REPEATABLE-READ
show variables like '%_isolation';
Variable_name	Value
transaction_isolation	REPEATABLE-READ
tx_isolation	REPEATABLE-READ
set global tx_isolation='READ-COMMITTED';
select @@global.transaction_isolation;
@@global.transaction_isolation
READ-COMMITTED
set global tx_isolation='READ-UNCOMMITTED';
select @@global.transaction_isolation;
@@global.transaction_isolation
READ-UNCOMMITTED
set global tx_isolation='SERIALIZABLE';
select @@global.transaction_isolation;
@@global.transaction_isolation
SERIALIZABLE
set tx_isolation='READ-COMMITTED';
select @@transaction_isolation;
@@transaction_isolation
READ-COMMITTED
set tx_isolation='READ-UNCOMMITTED';
select @@transaction_isolation;
@@transaction_isolation
READ-UNCOMMITTED
set tx_isolation='SERIALIZABLE';
select @@transaction_isolation;
@@transaction_isolation
SERIALIZABLE
create table t (id int primary key);
set global transaction_isolation='READ-UNCOMMITTED';
select @@global.transaction_isolation;
@@global.transaction_isolation
READ-UNCOMMITTED
begin;
insert into t values(1);
"can read uncommited"
select *from t;
id
1
rollback;
"can not read "
select *from t;
id
drop table t;
SET GLOBAL transaction_isolation = 'REPEATABLE-READ';

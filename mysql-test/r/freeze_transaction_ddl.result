create user test@localhost;
grant CREATE, INSERT, SELECT, DROP, SYSTEM_VARIABLES_ADMIN, SESSION_VARIABLES_ADMIN on *.* to test@localhost;
use test;
create table t (id int key);
insert into t values(1);
#con1 start a transaction
use test;
begin;
#con2 set global freeze_transaction_enable=ON
set global freeze_transaction_enable=ON;
#con1 trying to DML and DDL
insert into t values(100);
set lock_wait_timeout=1;
delete from t where id = 1;
create table t1 (id int key);
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
#con1 trying to DDL is blocked
set lock_wait_timeout=10;
create table t1 (id int key);
#con2 set global freeze_transaction_enable=OFF and notify con1
set global freeze_transaction_enable=OFF;

set global freeze_wait_timeout_sec=1;
set global freeze_transaction_enable = ON;
begin;
ERROR HY000: The transaction is frozen and needs to be restarted.
set global freeze_transaction_enable = OFF;
# Disconnect and cleanup.
set global freeze_wait_timeout_sec=60;
drop table t;
drop table t1;
drop user 'test'@'localhost';

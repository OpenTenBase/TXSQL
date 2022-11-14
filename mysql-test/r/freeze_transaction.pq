create user test@localhost;
grant CREATE, INSERT, SELECT, DROP, SYSTEM_VARIABLES_ADMIN, SESSION_VARIABLES_ADMIN on *.* to test@localhost;
use test;
create table t (id int key);
insert into t values(1);
#con1 start a new transaction
use test;
begin;
#con2 start a new transaction
use test;
begin;
insert into t values(2);
#conn3 implicit commit transaction
use test;
insert into t values(3);
#freeze transaction
set global freeze_transaction_enable=ON;
#the transactions of con1 and con2 are waiting to be completed
show unfrozen processlist;
Id	status
#con5 start a new transaction blocking
begin;
#con6 start a new implicit transaction and blocking
use test;
select *from t;
#con1 complete the transaction
insert into t values(11);
commit;
#con2 complete the transaction
commit;
#End of freeze
set global freeze_transaction_enable=OFF;
#con5 report an error
ERROR HY000: The transaction is frozen and needs to be restarted.
#con6 report an error
ERROR HY000: The transaction is frozen and needs to be restarted.
#The number of transactions waiting to be completed is 0
show unfrozen processlist;
Id	status
# Disconnect and cleanup.
drop table t;
drop user 'test'@'localhost';

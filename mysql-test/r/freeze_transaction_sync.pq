SET DEBUG_SYNC= 'RESET';
create user test@localhost;
grant CREATE, INSERT, SELECT, DROP, SYSTEM_VARIABLES_ADMIN, SESSION_VARIABLES_ADMIN on *.* to test@localhost;
use test;
create table t (id int key);
insert into t values(1);
#select-autocommit just skip the freeze check
use test;
SET DEBUG_SYNC= 'skip_freeze_check WAIT_FOR freeze_switch_on_1';
select *from t;
#insert-autocomit just skip the freeze check
use test;
SET DEBUG_SYNC= 'skip_freeze_check WAIT_FOR freeze_switch_on_2';
insert into t values(2);
#transaction started just skip the freeze check
use test;
SET DEBUG_SYNC= 'skip_freeze_check WAIT_FOR freeze_switch_on_3';
begin;
#con4 set freeze_transaction_enable on and list currently active transaction
set global freeze_transaction_enable=ON;
show unfrozen processlist;
Id	status
SET DEBUG_SYNC= 'now SIGNAL freeze_switch_on_1';
SET DEBUG_SYNC= 'now SIGNAL freeze_switch_on_2';
SET DEBUG_SYNC= 'now SIGNAL freeze_switch_on_3';
#con4 set freeze_transaction_enable success
show unfrozen processlist;
Id	status
set global freeze_transaction_enable=OFF;
ERROR HY000: The transaction is frozen and needs to be restarted.
ERROR HY000: The transaction is frozen and needs to be restarted.
ERROR HY000: The transaction is frozen and needs to be restarted.
#con1 prepare to miss signal
set global freeze_transaction_enable=ON;
use test;
SET DEBUG_SYNC= 'missing_signal_notify WAIT_FOR freeze_switch_off';
#con2 miss signal
begin;
set global freeze_transaction_enable=OFF;
SET DEBUG_SYNC= 'now SIGNAL freeze_switch_off';
#con2 missing signal,but an error is reported through double check
ERROR HY000: The transaction is frozen and needs to be restarted.
#test lock instance for backup failed
SET DEBUG_SYNC= 'alter_opened_table WAIT_FOR can_alter';
alter table t1 add column c5 int;
set lock_wait_timeout=1;
set global freeze_transaction_enable=ON;
ERROR HY000: Lock wait timeout exceeded; try restarting transaction
show global variables like "freeze_transaction_enable";
Variable_name	Value
freeze_transaction_enable	OFF
SET DEBUG_SYNC= 'now SIGNAL can_alter';
set global freeze_transaction_enable=ON;
show global variables like "freeze_transaction_enable";
Variable_name	Value
freeze_transaction_enable	ON
set global freeze_transaction_enable=ON;
set global freeze_transaction_enable=ON;
set global freeze_transaction_enable=OFF;
set global freeze_transaction_enable=OFF;
# Disconnect and cleanup.
drop table t;
drop user 'test'@'localhost';
SET DEBUG_SYNC= 'RESET';

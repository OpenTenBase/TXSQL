create user user1;
create user user2;
grant all privileges on *.* to user1;
grant all privileges on *.* to user2;
set @@global.debug= '+d, disable_gtid_background_persister';
create table t1(id int, c1 varchar(4000));
insert into t1 values(1, 'ccddee');
set autocommit=off;
insert into t1 values(2, repeat('a',4000));
set debug_sync='update_session_variable_after_lock_sysvar wait_for LOCK_thd_data_locked';
set autocommit=on;
set debug_sync='pfs_variable_before_lock_thd_sysvar signal LOCK_thd_data_locked';
select distinct(thread_id) from performance_schema.variables_by_thread;
thread_id
#
#
#
set @@global.debug= '-d, disable_gtid_background_persister';
drop user user1;
drop user user2;
drop table t1;

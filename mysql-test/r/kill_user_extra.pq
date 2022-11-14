create user test11@localhost identified by "test11";
create user test22@localhost identified by "test22";
grant Process on *.* to test11@localhost;
grant Process on *.* to test22@localhost;
flush privileges;
kill 111;
ERROR HY000: You are not owner of thread 111
set global cdb_kill_user_extra='test22@local%';
show variables like 'cdb_kill_user_extra';
Variable_name	Value
cdb_kill_user_extra	test22@local%
kill 111;
set global cdb_kill_user_extra='test22@127.0.0.%';
show variables like 'cdb_kill_user_extra';
Variable_name	Value
cdb_kill_user_extra	test22@127.0.0.%
kill 111;
delete from mysql.user where user='test22' or user='test11';

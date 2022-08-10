select 1+1;
1+1
2
select 1+2;
1+2
3
SHOW GLOBAL VARIABLES LIKE 'thread_handling';
Variable_name	Value
thread_handling	no-threads
select @@session.thread_handling;
ERROR HY000: Variable 'thread_handling' is a GLOBAL variable
set GLOBAL thread_handling='one-thread';
ERROR 42000: Variable 'thread_handling' can't be set to the value of 'one-thread'

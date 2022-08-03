SET @start_global_value = @@global.extra_max_connections;
select @@global.extra_max_connections;
@@global.extra_max_connections
300
select @@session.extra_max_connections;
ERROR HY000: Variable 'extra_max_connections' is a GLOBAL variable
show global variables like 'extra_max_connections';
Variable_name	Value
extra_max_connections	300
show session variables like 'extra_max_connections';
Variable_name	Value
extra_max_connections	300
select * from performance_schema.global_variables where variable_name='extra_max_connections';
VARIABLE_NAME	VARIABLE_VALUE
extra_max_connections	300
select * from performance_schema.session_variables where variable_name='extra_max_connections';
VARIABLE_NAME	VARIABLE_VALUE
extra_max_connections	300
set global extra_max_connections=200;
select @@global.extra_max_connections;
@@global.extra_max_connections
200
set session extra_max_connections=200;
ERROR HY000: Variable 'extra_max_connections' is a GLOBAL variable and should be set with SET GLOBAL
set global extra_max_connections=1.1;
ERROR 42000: Incorrect argument type to variable 'extra_max_connections'
set global extra_max_connections=1e1;
ERROR 42000: Incorrect argument type to variable 'extra_max_connections'
set global extra_max_connections="foo";
ERROR 42000: Incorrect argument type to variable 'extra_max_connections'
set global extra_max_connections=180;
select @@global.extra_max_connections;
@@global.extra_max_connections
180
set global extra_max_connections=cast(-1 as unsigned int);
Warnings:
Warning	1292	Truncated incorrect extra_max_connections value: '18446744073709551615'
select @@global.extra_max_connections;
@@global.extra_max_connections
100000
SET @@global.extra_max_connections = @start_global_value;

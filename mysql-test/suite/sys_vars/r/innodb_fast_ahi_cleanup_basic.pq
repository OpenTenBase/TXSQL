SET @start_global_value = @@global.innodb_fast_ahi_cleanup_for_drop_table;
SET @@global.innodb_fast_ahi_cleanup_for_drop_table = default;
SELECT @@global.innodb_fast_ahi_cleanup_for_drop_table;
@@global.innodb_fast_ahi_cleanup_for_drop_table
0
SET @@global.innodb_fast_ahi_cleanup_for_drop_table = @start_global_value;
Valid values are 'ON' and 'OFF'
select @@global.innodb_fast_ahi_cleanup_for_drop_table in (0, 1);
@@global.innodb_fast_ahi_cleanup_for_drop_table in (0, 1)
1
select @@global.innodb_fast_ahi_cleanup_for_drop_table;
@@global.innodb_fast_ahi_cleanup_for_drop_table
1
select @@session.innodb_fast_ahi_cleanup_for_drop_table;
ERROR HY000: Variable 'innodb_fast_ahi_cleanup_for_drop_table' is a GLOBAL variable
show global variables like 'innodb_fast_ahi_cleanup_for_drop_table';
Variable_name	Value
innodb_fast_ahi_cleanup_for_drop_table	ON
show session variables like 'innodb_fast_ahi_cleanup_for_drop_table';
Variable_name	Value
innodb_fast_ahi_cleanup_for_drop_table	ON
select * from performance_schema.global_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	ON
select * from performance_schema.session_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	ON
set global innodb_fast_ahi_cleanup_for_drop_table='OFF';
select @@global.innodb_fast_ahi_cleanup_for_drop_table;
@@global.innodb_fast_ahi_cleanup_for_drop_table
0
select * from performance_schema.global_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	OFF
select * from performance_schema.session_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	OFF
set @@global.innodb_fast_ahi_cleanup_for_drop_table=1;
select @@global.innodb_fast_ahi_cleanup_for_drop_table;
@@global.innodb_fast_ahi_cleanup_for_drop_table
1
select * from performance_schema.global_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	ON
select * from performance_schema.session_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	ON
set global innodb_fast_ahi_cleanup_for_drop_table=0;
select @@global.innodb_fast_ahi_cleanup_for_drop_table;
@@global.innodb_fast_ahi_cleanup_for_drop_table
0
select * from performance_schema.global_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	OFF
select * from performance_schema.session_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	OFF
set @@global.innodb_fast_ahi_cleanup_for_drop_table='ON';
select @@global.innodb_fast_ahi_cleanup_for_drop_table;
@@global.innodb_fast_ahi_cleanup_for_drop_table
1
select * from performance_schema.global_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	ON
select * from performance_schema.session_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	ON
set session innodb_fast_ahi_cleanup_for_drop_table='OFF';
ERROR HY000: Variable 'innodb_fast_ahi_cleanup_for_drop_table' is a GLOBAL variable and should be set with SET GLOBAL
set @@session.innodb_fast_ahi_cleanup_for_drop_table='ON';
ERROR HY000: Variable 'innodb_fast_ahi_cleanup_for_drop_table' is a GLOBAL variable and should be set with SET GLOBAL
set global innodb_fast_ahi_cleanup_for_drop_table=1.1;
ERROR 42000: Incorrect argument type to variable 'innodb_fast_ahi_cleanup_for_drop_table'
set global innodb_fast_ahi_cleanup_for_drop_table=1e1;
ERROR 42000: Incorrect argument type to variable 'innodb_fast_ahi_cleanup_for_drop_table'
set global innodb_fast_ahi_cleanup_for_drop_table=2;
ERROR 42000: Variable 'innodb_fast_ahi_cleanup_for_drop_table' can't be set to the value of '2'
NOTE: The following should fail with ER_WRONG_VALUE_FOR_VAR (BUG#50643)
set global innodb_fast_ahi_cleanup_for_drop_table=-3;
select @@global.innodb_fast_ahi_cleanup_for_drop_table;
@@global.innodb_fast_ahi_cleanup_for_drop_table
1
select * from performance_schema.global_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	ON
select * from performance_schema.session_variables where variable_name='innodb_fast_ahi_cleanup_for_drop_table';
VARIABLE_NAME	VARIABLE_VALUE
innodb_fast_ahi_cleanup_for_drop_table	ON
set global innodb_fast_ahi_cleanup_for_drop_table='AUTO';
ERROR 42000: Variable 'innodb_fast_ahi_cleanup_for_drop_table' can't be set to the value of 'AUTO'
SET @@global.innodb_fast_ahi_cleanup_for_drop_table = @start_global_value;
SELECT @@global.innodb_fast_ahi_cleanup_for_drop_table;
@@global.innodb_fast_ahi_cleanup_for_drop_table
1

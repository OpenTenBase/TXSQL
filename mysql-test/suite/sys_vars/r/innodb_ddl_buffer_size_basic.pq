SELECT @@global.innodb_ddl_buffer_size;
@@global.innodb_ddl_buffer_size
104857600
SELECT @@session.innodb_ddl_buffer_size;
@@session.innodb_ddl_buffer_size
104857600
SHOW GLOBAL VARIABLES LIKE  'innodb_ddl_buffer_size';
Variable_name	Value
innodb_ddl_buffer_size	104857600
SHOW SESSION VARIABLES LIKE 'innodb_ddl_buffer_size';
Variable_name	Value
innodb_ddl_buffer_size	104857600
SELECT * FROM performance_schema.global_variables
WHERE variable_name='innodb_ddl_buffer_size';
VARIABLE_NAME	VARIABLE_VALUE
innodb_ddl_buffer_size	104857600
SELECT * FROM performance_schema.session_variables
WHERE variable_name='innodb_ddl_buffer_size';
VARIABLE_NAME	VARIABLE_VALUE
innodb_ddl_buffer_size	104857600
set global innodb_ddl_buffer_size=10485759;
Warnings:
Warning	1292	Truncated incorrect innodb_ddl_buffer_size value: '10485759'
set session innodb_ddl_buffer_size=10485759;
Warnings:
Warning	1292	Truncated incorrect innodb_ddl_buffer_size value: '10485759'
set global innodb_ddl_buffer_size=10485760;
set session innodb_ddl_buffer_size=10485760;
SELECT * FROM performance_schema.global_variables
WHERE variable_name='innodb_ddl_buffer_size';
VARIABLE_NAME	VARIABLE_VALUE
innodb_ddl_buffer_size	10485760
SELECT * FROM performance_schema.session_variables
WHERE variable_name='innodb_ddl_buffer_size';
VARIABLE_NAME	VARIABLE_VALUE
innodb_ddl_buffer_size	10485760
set global innodb_ddl_buffer_size=4294967297;
Warnings:
Warning	1292	Truncated incorrect innodb_ddl_buffer_size value: '4294967297'
set session innodb_ddl_buffer_size=4294967297;
Warnings:
Warning	1292	Truncated incorrect innodb_ddl_buffer_size value: '4294967297'
set global innodb_ddl_buffer_size=4294967295;
set session innodb_ddl_buffer_size=4294967295;
SELECT * FROM performance_schema.global_variables
WHERE variable_name='innodb_ddl_buffer_size';
VARIABLE_NAME	VARIABLE_VALUE
innodb_ddl_buffer_size	4294967295
SELECT * FROM performance_schema.session_variables
WHERE variable_name='innodb_ddl_buffer_size';
VARIABLE_NAME	VARIABLE_VALUE
innodb_ddl_buffer_size	4294967295
set global innodb_ddl_buffer_size=104857600;
set session innodb_ddl_buffer_size=104857600;
SELECT * FROM performance_schema.global_variables
WHERE variable_name='innodb_ddl_buffer_size';
VARIABLE_NAME	VARIABLE_VALUE
innodb_ddl_buffer_size	104857600
SELECT * FROM performance_schema.session_variables
WHERE variable_name='innodb_ddl_buffer_size';
VARIABLE_NAME	VARIABLE_VALUE
innodb_ddl_buffer_size	104857600

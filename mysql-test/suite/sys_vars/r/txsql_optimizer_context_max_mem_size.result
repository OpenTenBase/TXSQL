SET @start_global_value = @@global.txsql_optimizer_context_max_mem_size;
SELECT @start_global_value;
@start_global_value
8388608
SET @start_session_value = @@session.txsql_optimizer_context_max_mem_size;
SELECT @start_session_value;
@start_session_value
8388608
SET @@global.txsql_optimizer_context_max_mem_size = 100;
SET @@global.txsql_optimizer_context_max_mem_size = DEFAULT;
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
8388608
SET @@session.txsql_optimizer_context_max_mem_size = 200;
SET @@session.txsql_optimizer_context_max_mem_size = DEFAULT;
SELECT @@session.txsql_optimizer_context_max_mem_size;
@@session.txsql_optimizer_context_max_mem_size
8388608
SET @@global.txsql_optimizer_context_max_mem_size = DEFAULT;
SELECT @@global.txsql_optimizer_context_max_mem_size = 8388608;
@@global.txsql_optimizer_context_max_mem_size = 8388608
1
SET @@session.txsql_optimizer_context_max_mem_size = DEFAULT;
SELECT @@session.txsql_optimizer_context_max_mem_size = 8388608;
@@session.txsql_optimizer_context_max_mem_size = 8388608
1
SET @@global.txsql_optimizer_context_max_mem_size = 0;
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
0
SET @@global.txsql_optimizer_context_max_mem_size = 100;
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
100
SET @@global.txsql_optimizer_context_max_mem_size = 18446744073709551615;
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
18446744073709551615
SET @@session.txsql_optimizer_context_max_mem_size = 0;
SELECT @@session.txsql_optimizer_context_max_mem_size;
@@session.txsql_optimizer_context_max_mem_size
0
SET @@session.txsql_optimizer_context_max_mem_size = 100;
SELECT @@session.txsql_optimizer_context_max_mem_size;
@@session.txsql_optimizer_context_max_mem_size
100
SET @@session.txsql_optimizer_context_max_mem_size = 18446744073709551615;
SELECT @@session.txsql_optimizer_context_max_mem_size;
@@session.txsql_optimizer_context_max_mem_size
18446744073709551615
SET @@global.txsql_optimizer_context_max_mem_size = -1024;
Warnings:
Warning	1292	Truncated incorrect txsql_optimizer_context_max_mem_size value: '-1024'
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
0
SET @@global.txsql_optimizer_context_max_mem_size = 18446744073709551616;
ERROR 42000: Incorrect argument type to variable 'txsql_optimizer_context_max_mem_size'
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
0
SET @@global.txsql_optimizer_context_max_mem_size = 65530.34;
ERROR 42000: Incorrect argument type to variable 'txsql_optimizer_context_max_mem_size'
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
0
SET @@global.txsql_optimizer_context_max_mem_size = test;
ERROR 42000: Incorrect argument type to variable 'txsql_optimizer_context_max_mem_size'
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
0
SET @@session.txsql_optimizer_context_max_mem_size = -2;
Warnings:
Warning	1292	Truncated incorrect txsql_optimizer_context_max_mem_size value: '-2'
SELECT @@session.txsql_optimizer_context_max_mem_size;
@@session.txsql_optimizer_context_max_mem_size
0
SET @@session.txsql_optimizer_context_max_mem_size = 18446744073709551616;
ERROR 42000: Incorrect argument type to variable 'txsql_optimizer_context_max_mem_size'
SELECT @@session.txsql_optimizer_context_max_mem_size;
@@session.txsql_optimizer_context_max_mem_size
0
SET @@session.txsql_optimizer_context_max_mem_size = 65530.34;
ERROR 42000: Incorrect argument type to variable 'txsql_optimizer_context_max_mem_size'
SELECT @@session.txsql_optimizer_context_max_mem_size;
@@session.txsql_optimizer_context_max_mem_size
0
SET @@session.txsql_optimizer_context_max_mem_size = test;
ERROR 42000: Incorrect argument type to variable 'txsql_optimizer_context_max_mem_size'
SELECT @@session.txsql_optimizer_context_max_mem_size;
@@session.txsql_optimizer_context_max_mem_size
0
SELECT @@global.txsql_optimizer_context_max_mem_size = VARIABLE_VALUE 
FROM performance_schema.global_variables 
WHERE VARIABLE_NAME='txsql_optimizer_context_max_mem_size';
@@global.txsql_optimizer_context_max_mem_size = VARIABLE_VALUE
1
SELECT @@session.txsql_optimizer_context_max_mem_size = VARIABLE_VALUE 
FROM performance_schema.session_variables 
WHERE VARIABLE_NAME='txsql_optimizer_context_max_mem_size';
@@session.txsql_optimizer_context_max_mem_size = VARIABLE_VALUE
1
SET @@global.txsql_optimizer_context_max_mem_size = TRUE;
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
1
SET @@global.txsql_optimizer_context_max_mem_size = FALSE;
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
0
SET @@global.txsql_optimizer_context_max_mem_size = 10;
SELECT @@txsql_optimizer_context_max_mem_size = @@global.txsql_optimizer_context_max_mem_size;
@@txsql_optimizer_context_max_mem_size = @@global.txsql_optimizer_context_max_mem_size
0
SET @@txsql_optimizer_context_max_mem_size = 100;
SELECT @@txsql_optimizer_context_max_mem_size = @@local.txsql_optimizer_context_max_mem_size;
@@txsql_optimizer_context_max_mem_size = @@local.txsql_optimizer_context_max_mem_size
1
SELECT @@local.txsql_optimizer_context_max_mem_size = @@session.txsql_optimizer_context_max_mem_size;
@@local.txsql_optimizer_context_max_mem_size = @@session.txsql_optimizer_context_max_mem_size
1
SET txsql_optimizer_context_max_mem_size = 1;
SELECT @@txsql_optimizer_context_max_mem_size;
@@txsql_optimizer_context_max_mem_size
1
SELECT local.txsql_optimizer_context_max_mem_size;
ERROR 42S02: Unknown table 'local' in field list
SELECT session.txsql_optimizer_context_max_mem_size;
ERROR 42S02: Unknown table 'session' in field list
SELECT txsql_optimizer_context_max_mem_size = @@session.txsql_optimizer_context_max_mem_size;
ERROR 42S22: Unknown column 'txsql_optimizer_context_max_mem_size' in 'field list'
SET @@global.txsql_optimizer_context_max_mem_size = @start_global_value;
SELECT @@global.txsql_optimizer_context_max_mem_size;
@@global.txsql_optimizer_context_max_mem_size
8388608
SET @@session.txsql_optimizer_context_max_mem_size = @start_session_value;
SELECT @@session.txsql_optimizer_context_max_mem_size;
@@session.txsql_optimizer_context_max_mem_size
8388608

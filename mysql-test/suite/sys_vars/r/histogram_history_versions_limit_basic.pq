#
# Variable name : histogram_history_versions_limit
# Scope         : Global
#
SET @start_global_value = @@global.histogram_history_versions_limit;
SELECT @start_global_value;
@start_global_value
5

SET @@global.histogram_history_versions_limit = 16;
SET @@global.histogram_history_versions_limit = DEFAULT;
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
5

SET @@global.histogram_history_versions_limit = DEFAULT;
SELECT @@global.histogram_history_versions_limit = 5;
@@global.histogram_history_versions_limit = 5
1

SET @@global.histogram_history_versions_limit = 1;
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
1
SET @@global.histogram_history_versions_limit = 25;
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
25
SET @@global.histogram_history_versions_limit = 50;
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
50
SET @@global.histogram_history_versions_limit = 75;
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
75
SET @@global.histogram_history_versions_limit = 100;
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
100

SET @@histogram_history_versions_limit = 2;
ERROR HY000: Variable 'histogram_history_versions_limit' is a GLOBAL variable and should be set with SET GLOBAL
SET @@session.histogram_history_versions_limit = 3;
ERROR HY000: Variable 'histogram_history_versions_limit' is a GLOBAL variable and should be set with SET GLOBAL
SET @@local.histogram_history_versions_limit = 4;
ERROR HY000: Variable 'histogram_history_versions_limit' is a GLOBAL variable and should be set with SET GLOBAL

SET @@global.histogram_history_versions_limit = -1;
Warnings:
Warning	1292	Truncated incorrect histogram_history_versions_limit value: '-1'
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
1
SET @@global.histogram_history_versions_limit = -2147483648;
Warnings:
Warning	1292	Truncated incorrect histogram_history_versions_limit value: '-2147483648'
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
1
SET @@global.histogram_history_versions_limit = 2147483649*2;
Warnings:
Warning	1292	Truncated incorrect histogram_history_versions_limit value: '4294967298'
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
100
SET @@global.histogram_history_versions_limit = 65530.34;
ERROR 42000: Incorrect argument type to variable 'histogram_history_versions_limit'
SET @@global.histogram_history_versions_limit = '125';
ERROR 42000: Incorrect argument type to variable 'histogram_history_versions_limit'
SET @@global.histogram_history_versions_limit = 7483649.56;
ERROR 42000: Incorrect argument type to variable 'histogram_history_versions_limit'
SET @@global.histogram_history_versions_limit = 1G;
ERROR 42000: Incorrect argument type to variable 'histogram_history_versions_limit'

SET @@global.histogram_history_versions_limit = 12;
SELECT @@global.histogram_history_versions_limit = VARIABLE_VALUE
FROM performance_schema.global_variables
WHERE VARIABLE_NAME='histogram_history_versions_limit';
@@global.histogram_history_versions_limit = VARIABLE_VALUE
1

SELECT count(VARIABLE_VALUE)
FROM performance_schema.session_variables
WHERE VARIABLE_NAME='histogram_history_versions_limit';
count(VARIABLE_VALUE)
1

SET @@global.histogram_history_versions_limit = TRUE;
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
1
SET @@global.histogram_history_versions_limit = FALSE;
Warnings:
Warning	1292	Truncated incorrect histogram_history_versions_limit value: '0'
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
1

SET @@global.histogram_history_versions_limit = 19;
SELECT @@histogram_history_versions_limit = @@global.histogram_history_versions_limit;
@@histogram_history_versions_limit = @@global.histogram_history_versions_limit
1

SET histogram_history_versions_limit = 2048;
ERROR HY000: Variable 'histogram_history_versions_limit' is a GLOBAL variable and should be set with SET GLOBAL
SELECT histogram_history_versions_limit;
ERROR 42S22: Unknown column 'histogram_history_versions_limit' in 'field list'
SELECT @@histogram_history_versions_limit;
@@histogram_history_versions_limit
19
SET global histogram_history_versions_limit = 20;
SET @@global.histogram_history_versions_limit = @start_global_value;
SELECT @@global.histogram_history_versions_limit;
@@global.histogram_history_versions_limit
5

#
# Variable name : histogram_statistics_concurrency
# Scope         : Global
#
SET @start_global_value = @@global.histogram_statistics_concurrency;
SELECT @start_global_value;
@start_global_value
10

SET @@global.histogram_statistics_concurrency = 16;
SET @@global.histogram_statistics_concurrency = DEFAULT;
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
10

SET @@global.histogram_statistics_concurrency = DEFAULT;
SELECT @@global.histogram_statistics_concurrency = 10;
@@global.histogram_statistics_concurrency = 10
1

SET @@global.histogram_statistics_concurrency = 1;
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
1
SET @@global.histogram_statistics_concurrency = 25;
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
25
SET @@global.histogram_statistics_concurrency = 50;
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
50
SET @@global.histogram_statistics_concurrency = 75;
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
75
SET @@global.histogram_statistics_concurrency = 100;
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
100

SET @@histogram_statistics_concurrency = 2;
ERROR HY000: Variable 'histogram_statistics_concurrency' is a GLOBAL variable and should be set with SET GLOBAL
SET @@session.histogram_statistics_concurrency = 3;
ERROR HY000: Variable 'histogram_statistics_concurrency' is a GLOBAL variable and should be set with SET GLOBAL
SET @@local.histogram_statistics_concurrency = 4;
ERROR HY000: Variable 'histogram_statistics_concurrency' is a GLOBAL variable and should be set with SET GLOBAL

SET @@global.histogram_statistics_concurrency = -1;
Warnings:
Warning	1292	Truncated incorrect histogram_statistics_concurrency value: '-1'
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
1
SET @@global.histogram_statistics_concurrency = -2147483648;
Warnings:
Warning	1292	Truncated incorrect histogram_statistics_concurrency value: '-2147483648'
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
1
SET @@global.histogram_statistics_concurrency = 2147483649*2;
Warnings:
Warning	1292	Truncated incorrect histogram_statistics_concurrency value: '4294967298'
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
1000
SET @@global.histogram_statistics_concurrency = 65530.34;
ERROR 42000: Incorrect argument type to variable 'histogram_statistics_concurrency'
SET @@global.histogram_statistics_concurrency = '125';
ERROR 42000: Incorrect argument type to variable 'histogram_statistics_concurrency'
SET @@global.histogram_statistics_concurrency = 7483649.56;
ERROR 42000: Incorrect argument type to variable 'histogram_statistics_concurrency'
SET @@global.histogram_statistics_concurrency = 1G;
ERROR 42000: Incorrect argument type to variable 'histogram_statistics_concurrency'

SET @@global.histogram_statistics_concurrency = 12;
SELECT @@global.histogram_statistics_concurrency = VARIABLE_VALUE
FROM performance_schema.global_variables
WHERE VARIABLE_NAME='histogram_statistics_concurrency';
@@global.histogram_statistics_concurrency = VARIABLE_VALUE
1

SELECT count(VARIABLE_VALUE)
FROM performance_schema.session_variables
WHERE VARIABLE_NAME='histogram_statistics_concurrency';
count(VARIABLE_VALUE)
1

SET @@global.histogram_statistics_concurrency = TRUE;
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
1
SET @@global.histogram_statistics_concurrency = FALSE;
Warnings:
Warning	1292	Truncated incorrect histogram_statistics_concurrency value: '0'
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
1

SET @@global.histogram_statistics_concurrency = 19;
SELECT @@histogram_statistics_concurrency = @@global.histogram_statistics_concurrency;
@@histogram_statistics_concurrency = @@global.histogram_statistics_concurrency
1

SET histogram_statistics_concurrency = 2048;
ERROR HY000: Variable 'histogram_statistics_concurrency' is a GLOBAL variable and should be set with SET GLOBAL
SELECT histogram_statistics_concurrency;
ERROR 42S22: Unknown column 'histogram_statistics_concurrency' in 'field list'
SELECT @@histogram_statistics_concurrency;
@@histogram_statistics_concurrency
19
SET global histogram_statistics_concurrency = 20;
SET @@global.histogram_statistics_concurrency = @start_global_value;
SELECT @@global.histogram_statistics_concurrency;
@@global.histogram_statistics_concurrency
10

#
# Variable name : auto_stats_interval_duration
# Scope         : Global
#
SET @start_global_value = @@global.auto_stats_interval_duration;
SELECT @start_global_value;
@start_global_value
1

SET @@global.auto_stats_interval_duration = 16;
SET @@global.auto_stats_interval_duration = DEFAULT;
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
1

SET @@global.auto_stats_interval_duration = DEFAULT;
SELECT @@global.auto_stats_interval_duration = 1;
@@global.auto_stats_interval_duration = 1
1

SET @@global.auto_stats_interval_duration = 1;
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
1
SET @@global.auto_stats_interval_duration = 2;
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
2
SET @@global.auto_stats_interval_duration = 5;
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
5
SET @@global.auto_stats_interval_duration = 7;
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
7
SET @@global.auto_stats_interval_duration = 10;
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
10

SET @@auto_stats_interval_duration = 2;
ERROR HY000: Variable 'auto_stats_interval_duration' is a GLOBAL variable and should be set with SET GLOBAL
SET @@session.auto_stats_interval_duration = 3;
ERROR HY000: Variable 'auto_stats_interval_duration' is a GLOBAL variable and should be set with SET GLOBAL
SET @@local.auto_stats_interval_duration = 4;
ERROR HY000: Variable 'auto_stats_interval_duration' is a GLOBAL variable and should be set with SET GLOBAL

SET @@global.auto_stats_interval_duration = -1;
Warnings:
Warning	1292	Truncated incorrect auto_stats_interval_duration value: '-1'
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
1
SET @@global.auto_stats_interval_duration = -2147483648;
Warnings:
Warning	1292	Truncated incorrect auto_stats_interval_duration value: '-2147483648'
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
1
SET @@global.auto_stats_interval_duration = 2147483649*2;
Warnings:
Warning	1292	Truncated incorrect auto_stats_interval_duration value: '4294967298'
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
23
SET @@global.auto_stats_interval_duration = 65530.34;
ERROR 42000: Incorrect argument type to variable 'auto_stats_interval_duration'
SET @@global.auto_stats_interval_duration = '125';
ERROR 42000: Incorrect argument type to variable 'auto_stats_interval_duration'
SET @@global.auto_stats_interval_duration = 7483649.56;
ERROR 42000: Incorrect argument type to variable 'auto_stats_interval_duration'
SET @@global.auto_stats_interval_duration = 1G;
ERROR 42000: Incorrect argument type to variable 'auto_stats_interval_duration'

SET @@global.auto_stats_interval_duration = 12;
SELECT @@global.auto_stats_interval_duration = VARIABLE_VALUE
FROM performance_schema.global_variables
WHERE VARIABLE_NAME='auto_stats_interval_duration';
@@global.auto_stats_interval_duration = VARIABLE_VALUE
1

SELECT count(VARIABLE_VALUE)
FROM performance_schema.session_variables
WHERE VARIABLE_NAME='auto_stats_interval_duration';
count(VARIABLE_VALUE)
1

SET @@global.auto_stats_interval_duration = TRUE;
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
1
SET @@global.auto_stats_interval_duration = FALSE;
Warnings:
Warning	1292	Truncated incorrect auto_stats_interval_duration value: '0'
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
1

SET @@global.auto_stats_interval_duration = 19;
SELECT @@auto_stats_interval_duration = @@global.auto_stats_interval_duration;
@@auto_stats_interval_duration = @@global.auto_stats_interval_duration
1

SET auto_stats_interval_duration = 20;
ERROR HY000: Variable 'auto_stats_interval_duration' is a GLOBAL variable and should be set with SET GLOBAL
SELECT auto_stats_interval_duration;
ERROR 42S22: Unknown column 'auto_stats_interval_duration' in 'field list'
SELECT @@auto_stats_interval_duration;
@@auto_stats_interval_duration
19
SET global auto_stats_interval_duration = 20;
SET @@global.auto_stats_interval_duration = @start_global_value;
SELECT @@global.auto_stats_interval_duration;
@@global.auto_stats_interval_duration
1

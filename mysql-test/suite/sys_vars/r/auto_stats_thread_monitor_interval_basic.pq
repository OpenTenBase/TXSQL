#
# Variable name : auto_stats_thread_monitor_interval
# Scope         : Global
#
SET @start_global_value = @@global.auto_stats_thread_monitor_interval;
SELECT @start_global_value;
@start_global_value
10

SET @@global.auto_stats_thread_monitor_interval = 16;
SET @@global.auto_stats_thread_monitor_interval = DEFAULT;
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
10

SET @@global.auto_stats_thread_monitor_interval = DEFAULT;
SELECT @@global.auto_stats_thread_monitor_interval = 10;
@@global.auto_stats_thread_monitor_interval = 10
1

SET @@global.auto_stats_thread_monitor_interval = 1;
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
1
SET @@global.auto_stats_thread_monitor_interval = 25;
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
25
SET @@global.auto_stats_thread_monitor_interval = 50;
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
50
SET @@global.auto_stats_thread_monitor_interval = 75;
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
75
SET @@global.auto_stats_thread_monitor_interval = 100;
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
100

SET @@auto_stats_thread_monitor_interval = 2;
ERROR HY000: Variable 'auto_stats_thread_monitor_interval' is a GLOBAL variable and should be set with SET GLOBAL
SET @@session.auto_stats_thread_monitor_interval = 3;
ERROR HY000: Variable 'auto_stats_thread_monitor_interval' is a GLOBAL variable and should be set with SET GLOBAL
SET @@local.auto_stats_thread_monitor_interval = 4;
ERROR HY000: Variable 'auto_stats_thread_monitor_interval' is a GLOBAL variable and should be set with SET GLOBAL

SET @@global.auto_stats_thread_monitor_interval = -1;
Warnings:
Warning	1292	Truncated incorrect auto_stats_thread_monitor_interval value: '-1'
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
1
SET @@global.auto_stats_thread_monitor_interval = -2147483648;
Warnings:
Warning	1292	Truncated incorrect auto_stats_thread_monitor_interval value: '-2147483648'
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
1
SET @@global.auto_stats_thread_monitor_interval = 2147483649*2;
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
4294967298
SET @@global.auto_stats_thread_monitor_interval = 65530.34;
ERROR 42000: Incorrect argument type to variable 'auto_stats_thread_monitor_interval'
SET @@global.auto_stats_thread_monitor_interval = '125';
ERROR 42000: Incorrect argument type to variable 'auto_stats_thread_monitor_interval'
SET @@global.auto_stats_thread_monitor_interval = 7483649.56;
ERROR 42000: Incorrect argument type to variable 'auto_stats_thread_monitor_interval'
SET @@global.auto_stats_thread_monitor_interval = 1G;
ERROR 42000: Incorrect argument type to variable 'auto_stats_thread_monitor_interval'

SET @@global.auto_stats_thread_monitor_interval = 12;
SELECT @@global.auto_stats_thread_monitor_interval = VARIABLE_VALUE
FROM performance_schema.global_variables
WHERE VARIABLE_NAME='auto_stats_thread_monitor_interval';
@@global.auto_stats_thread_monitor_interval = VARIABLE_VALUE
1

SELECT count(VARIABLE_VALUE)
FROM performance_schema.session_variables
WHERE VARIABLE_NAME='auto_stats_thread_monitor_interval';
count(VARIABLE_VALUE)
1

SET @@global.auto_stats_thread_monitor_interval = TRUE;
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
1
SET @@global.auto_stats_thread_monitor_interval = FALSE;
Warnings:
Warning	1292	Truncated incorrect auto_stats_thread_monitor_interval value: '0'
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
1

SET @@global.auto_stats_thread_monitor_interval = 19;
SELECT @@auto_stats_thread_monitor_interval = @@global.auto_stats_thread_monitor_interval;
@@auto_stats_thread_monitor_interval = @@global.auto_stats_thread_monitor_interval
1

SET auto_stats_thread_monitor_interval = 2048;
ERROR HY000: Variable 'auto_stats_thread_monitor_interval' is a GLOBAL variable and should be set with SET GLOBAL
SELECT auto_stats_thread_monitor_interval;
ERROR 42S22: Unknown column 'auto_stats_thread_monitor_interval' in 'field list'
SELECT @@auto_stats_thread_monitor_interval;
@@auto_stats_thread_monitor_interval
19
SET global auto_stats_thread_monitor_interval = 20;
SET @@global.auto_stats_thread_monitor_interval = @start_global_value;
SELECT @@global.auto_stats_thread_monitor_interval;
@@global.auto_stats_thread_monitor_interval
10

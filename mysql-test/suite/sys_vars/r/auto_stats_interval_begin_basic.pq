SELECT @@global.auto_stats_interval_begin INTO @old_begin;
# NULL: forbidden
SET @@global.auto_stats_interval_begin="";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of ''
# NULL: forbidden
SET @@global.auto_stats_interval_begin=NULL;
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of 'NULL'
# client-range: forbidden
SET @@global.auto_stats_interval_begin="1234";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '1234'
SELECT @@global.auto_stats_interval_begin;
@@global.auto_stats_interval_begin
03:00
# various legal values
SET @@global.auto_stats_interval_begin="01:00";
SET @@global.auto_stats_interval_begin="01:59";
SET @@global.auto_stats_interval_begin="23:59";
# lists containing illegal values. warning should flag first problem.
SET @@global.auto_stats_interval_begin="00:60";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '00:60'
SHOW WARNINGS;
Level	Code	Message
Error	1231	Variable 'auto_stats_interval_begin' can't be set to the value of '00:60'
SET @@global.auto_stats_interval_begin="01:-1";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '01:-1'
SHOW WARNINGS;
Level	Code	Message
Error	1231	Variable 'auto_stats_interval_begin' can't be set to the value of '01:-1'
SET @@global.auto_stats_interval_begin="01:99";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '01:99'
SHOW WARNINGS;
Level	Code	Message
Error	1231	Variable 'auto_stats_interval_begin' can't be set to the value of '01:99'
SET @@global.auto_stats_interval_begin="00:+1";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '00:+1'
SHOW WARNINGS;
Level	Code	Message
Error	1231	Variable 'auto_stats_interval_begin' can't be set to the value of '00:+1'
SET @@global.auto_stats_interval_begin="0:012";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '0:012'
SHOW WARNINGS;
Level	Code	Message
Error	1231	Variable 'auto_stats_interval_begin' can't be set to the value of '0:012'
SET @@global.auto_stats_interval_begin="00:60";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '00:60'
SHOW WARNINGS;
Level	Code	Message
Error	1231	Variable 'auto_stats_interval_begin' can't be set to the value of '00:60'
SET @@global.auto_stats_interval_begin=",";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of ','
SHOW WARNINGS;
Level	Code	Message
Error	1231	Variable 'auto_stats_interval_begin' can't be set to the value of ','
SET @@global.auto_stats_interval_begin="10000 10001";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '10000 10001'
SHOW WARNINGS;
Level	Code	Message
Error	1231	Variable 'auto_stats_interval_begin' can't be set to the value of '10000 10001'
SET @@global.auto_stats_interval_begin='10000.123';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '10000.123'
SET @@global.auto_stats_interval_begin='MY-10000#$%';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of 'MY-10000#$%'
SET @@global.auto_stats_interval_begin='MY-31';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of 'MY-31'
SET SESSION auto_stats_interval_begin='01:00';
ERROR HY000: Variable 'auto_stats_interval_begin' is a GLOBAL variable and should be set with SET GLOBAL
SELECT @@auto_stats_interval_begin;
@@auto_stats_interval_begin
23:59
SET @@auto_stats_interval_begin='01:00';
ERROR HY000: Variable 'auto_stats_interval_begin' is a GLOBAL variable and should be set with SET GLOBAL
SELECT @@auto_stats_interval_begin;
@@auto_stats_interval_begin
23:59
SET GLOBAL auto_stats_interval_begin="01:00";
SHOW VARIABLES LIKE 'auto%stats%';
Variable_name	Value
auto_stats_interval_begin	01:00
auto_stats_interval_duration	1
auto_stats_node_selection	DEDICATED
auto_stats_thread_monitor_interval	10
SELECT * FROM performance_schema.global_variables
WHERE VARIABLE_NAME = 'auto_stats_interval_begin';
VARIABLE_NAME	VARIABLE_VALUE
auto_stats_interval_begin	01:00
SELECT * FROM performance_schema.variables_info
WHERE VARIABLE_NAME = 'auto_stats_interval_begin';
VARIABLE_NAME	VARIABLE_SOURCE	VARIABLE_PATH	MIN_VALUE	MAX_VALUE	SET_TIME	SET_USER	SET_HOST
auto_stats_interval_begin	DYNAMIC		0	0	#	root	localhost
SET GLOBAL auto_stats_interval_begin=10000;
ERROR 42000: Incorrect argument type to variable 'auto_stats_interval_begin'
SELECT @@global.auto_stats_interval_begin;
@@global.auto_stats_interval_begin
01:00
SET GLOBAL auto_stats_interval_begin=`01:01`;
SELECT @@global.auto_stats_interval_begin;
@@global.auto_stats_interval_begin
01:01
SET GLOBAL auto_stats_interval_begin=12345.124;
ERROR 42000: Incorrect argument type to variable 'auto_stats_interval_begin'
SET GLOBAL auto_stats_interval_begin='12345.124';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '12345.124'
SET GLOBAL auto_stats_interval_begin='12345e5';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '12345e5'
SET GLOBAL auto_stats_interval_begin=abcd;
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of 'abcd'
SET GLOBAL auto_stats_interval_begin='abcd';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of 'abcd'
SET GLOBAL auto_stats_interval_begin=',';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of ','
SET GLOBAL auto_stats_interval_begin=',10000,';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of ',10000,'
SET GLOBAL auto_stats_interval_begin='10000,';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '10000,'
SET GLOBAL auto_stats_interval_begin=',10000';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of ',10000'
SET GLOBAL auto_stats_interval_begin='10000,,10001';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '10000,,10001'
SET GLOBAL auto_stats_interval_begin="'10000'";
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of ''10000''
SET GLOBAL auto_stats_interval_begin=CONCAT("'",CONCAT('10000',"'"));
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of ''10000''
SET GLOBAL auto_stats_interval_begin='0000000000000010000';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of '0000000000000010000'
SET GLOBAL auto_stats_interval_begin='MY-010000';
ERROR 42000: Variable 'auto_stats_interval_begin' can't be set to the value of 'MY-010000'
SELECT @@global.auto_stats_interval_begin;
@@global.auto_stats_interval_begin
01:01
SET @@global.auto_stats_interval_begin=@old_begin;

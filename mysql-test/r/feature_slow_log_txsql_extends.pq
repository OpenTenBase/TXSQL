show variables like "%log_profile_in_slow_log%";
Variable_name	Value
log_profile_in_slow_log	ON
show variables like "long_query_time";
Variable_name	Value
long_query_time	2.000000
show variables like "slow_query_log";
Variable_name	Value
slow_query_log	ON
show variables like "log_output";
Variable_name	Value
log_output	FILE
SET @save_sqlf=@@global.slow_query_log_file;
SET GLOBAL slow_query_log_file= '.../slow_log_extends.log';
select sleep(3);
sleep(3)
0
# Look for Errnos in slow log. There should be two,
# both SET and CREATE will be logged as slow.
[ PASS ] found profile data in slow log.
SET @@global.slow_query_log_file=@save_sqlf;

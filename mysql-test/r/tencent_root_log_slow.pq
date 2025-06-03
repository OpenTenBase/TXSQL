#
# root slow log testing.
#
#test for root slow query log
set global log_output='TABLE';
set long_query_time=1;
#cdb_root_slow_log_enabled turned off
set global cdb_root_slow_log_enabled=off;
select 'test1 should not be logged', sleep(2);
test1 should not be logged	sleep(2)
test1 should not be logged	0
TRUNCATE mysql.slow_log;
TRUNCATE mysql.general_log;
#cdb_root_slow_log_enabled turned on
set global cdb_root_slow_log_enabled=on;
select 'test2 should be logged', sleep(2);
test2 should be logged	sleep(2)
test2 should be logged	0
TRUNCATE mysql.slow_log;
TRUNCATE mysql.general_log;
#test for root slow query log
set long_query_time=1;
select 'test3 should be logged', sleep(2);
test3 should be logged	sleep(2)
test3 should be logged	0
TRUNCATE mysql.slow_log;
TRUNCATE mysql.general_log;
set global log_output=default;
TRUNCATE mysql.general_log;

set @saved_innodb_backquery_enable = @@global.innodb_backquery_enable;
set @saved_innodb_backquery_window = @@global.innodb_backquery_window;
set @saved_innodb_backquery_history_limit = @@global.innodb_backquery_history_limit;
set @saved_innodb_backquery_trackpoint_create_interval = @@global.innodb_backquery_trackpoint_create_interval;
set @saved_innodb_backquery_trackpoint_clean_interval = @@global.innodb_backquery_trackpoint_clean_interval;
show variables like '%backquery%';
Variable_name	Value
innodb_backquery_enable	OFF
innodb_backquery_history_limit	8000000
innodb_backquery_trackpoint_clean_interval	1
innodb_backquery_trackpoint_create_interval	1
innodb_backquery_window	86400
set global innodb_backquery_enable=ON;
set global innodb_backquery_enable=OFF;
set global innodb_backquery_history_limit=10;
set global innodb_backquery_trackpoint_create_interval=3;
set global innodb_backquery_trackpoint_clean_interval=2;
set global innodb_backquery_window=100;
set sql_mode='strict_all_tables';
set global innodb_backquery_history_limit=0;
ERROR 42000: Variable 'innodb_backquery_history_limit' can't be set to the value of '0'
set global innodb_backquery_trackpoint_create_interval=0;
ERROR 42000: Variable 'innodb_backquery_trackpoint_create_interval' can't be set to the value of '0'
set global innodb_backquery_trackpoint_clean_interval=0;
ERROR 42000: Variable 'innodb_backquery_trackpoint_clean_interval' can't be set to the value of '0'
set global innodb_backquery_window=0;
ERROR 42000: Variable 'innodb_backquery_window' can't be set to the value of '0'
set global innodb_backquery_window=2592001;
ERROR 42000: Variable 'innodb_backquery_window' can't be set to the value of '2592001'
set global innodb_backquery_enable=@saved_innodb_backquery_enable;
set global innodb_backquery_history_limit=@saved_innodb_backquery_history_limit;
set global innodb_backquery_trackpoint_create_interval=@saved_innodb_backquery_trackpoint_create_interval;
set global innodb_backquery_trackpoint_clean_interval=@saved_innodb_backquery_trackpoint_clean_interval;
set global innodb_backquery_window=@saved_innodb_backquery_window;
show variables like '%backquery%';
Variable_name	Value
innodb_backquery_enable	OFF
innodb_backquery_history_limit	8000000
innodb_backquery_trackpoint_clean_interval	1
innodb_backquery_trackpoint_create_interval	1
innodb_backquery_window	86400

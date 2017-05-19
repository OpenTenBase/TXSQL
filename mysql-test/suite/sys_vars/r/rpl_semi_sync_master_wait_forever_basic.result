INSTALL PLUGIN rpl_semi_sync_master SONAME 'semisync_master.so';
Warnings:
Note	1287	'rpl_semi_sync_master' is deprecated and will be removed in a future release. Please use rpl_semi_sync_source instead
select @@global.rpl_semi_sync_master_wait_forever;
@@global.rpl_semi_sync_master_wait_forever
0
SET @start_global_value = @@global.rpl_semi_sync_master_wait_forever;
select @@global.rpl_semi_sync_master_wait_forever in (0,1);
@@global.rpl_semi_sync_master_wait_forever in (0,1)
1
select @@session.rpl_semi_sync_master_wait_forever;
ERROR HY000: Variable 'rpl_semi_sync_master_wait_forever' is a GLOBAL variable
show global variables like 'rpl_semi_sync_master_wait_forever';
Variable_name	Value
rpl_semi_sync_master_wait_forever	OFF
show session variables like 'rpl_semi_sync_master_wait_forever';
Variable_name	Value
rpl_semi_sync_master_wait_forever	OFF
select * from performance_schema.global_variables where variable_name='rpl_semi_sync_master_wait_forever';
VARIABLE_NAME	VARIABLE_VALUE
rpl_semi_sync_master_wait_forever	OFF
select * from performance_schema.session_variables where variable_name='rpl_semi_sync_master_wait_forever';
VARIABLE_NAME	VARIABLE_VALUE
rpl_semi_sync_master_wait_forever	OFF
set global rpl_semi_sync_master_wait_forever=0;
set session rpl_semi_sync_master_wait_forever=0;
ERROR HY000: Variable 'rpl_semi_sync_master_wait_forever' is a GLOBAL variable and should be set with SET GLOBAL
select @@global.rpl_semi_sync_master_wait_forever;
@@global.rpl_semi_sync_master_wait_forever
0
select @@session.rpl_semi_sync_master_wait_forever;
ERROR HY000: Variable 'rpl_semi_sync_master_wait_forever' is a GLOBAL variable
show global variables like 'rpl_semi_sync_master_wait_forever';
Variable_name	Value
rpl_semi_sync_master_wait_forever	OFF
show session variables like 'rpl_semi_sync_master_wait_forever';
Variable_name	Value
rpl_semi_sync_master_wait_forever	OFF
select * from performance_schema.global_variables where variable_name='rpl_semi_sync_master_wait_forever';
VARIABLE_NAME	VARIABLE_VALUE
rpl_semi_sync_master_wait_forever	OFF
select * from performance_schema.session_variables where variable_name='rpl_semi_sync_master_wait_forever';
VARIABLE_NAME	VARIABLE_VALUE
rpl_semi_sync_master_wait_forever	OFF
set global rpl_semi_sync_master_wait_forever=1;
Warnings:
Warning	168	Set wait forever is forbidden when semi-sync replication is disabled.
set session rpl_semi_sync_master_wait_forever=1;
ERROR HY000: Variable 'rpl_semi_sync_master_wait_forever' is a GLOBAL variable and should be set with SET GLOBAL
select @@global.rpl_semi_sync_master_wait_forever;
@@global.rpl_semi_sync_master_wait_forever
1
select @@session.rpl_semi_sync_master_wait_forever;
ERROR HY000: Variable 'rpl_semi_sync_master_wait_forever' is a GLOBAL variable
show global variables like 'rpl_semi_sync_master_wait_forever';
Variable_name	Value
rpl_semi_sync_master_wait_forever	ON
show session variables like 'rpl_semi_sync_master_wait_forever';
Variable_name	Value
rpl_semi_sync_master_wait_forever	ON
select * from performance_schema.global_variables where variable_name='rpl_semi_sync_master_wait_forever';
VARIABLE_NAME	VARIABLE_VALUE
rpl_semi_sync_master_wait_forever	ON
select * from performance_schema.session_variables where variable_name='rpl_semi_sync_master_wait_forever';
VARIABLE_NAME	VARIABLE_VALUE
rpl_semi_sync_master_wait_forever	ON
set global rpl_semi_sync_master_wait_forever=1.1;
ERROR 42000: Incorrect argument type to variable 'rpl_semi_sync_master_wait_forever'
set global rpl_semi_sync_master_wait_forever=1e1;
ERROR 42000: Incorrect argument type to variable 'rpl_semi_sync_master_wait_forever'
set global rpl_semi_sync_master_wait_forever="some text";
ERROR 42000: Variable 'rpl_semi_sync_master_wait_forever' can't be set to the value of 'some text'
SET @@global.rpl_semi_sync_master_wait_forever = @start_global_value;
Warnings:
Warning	168	Set wait forever is forbidden when semi-sync replication is disabled.
select @@global.rpl_semi_sync_master_wait_forever;
@@global.rpl_semi_sync_master_wait_forever
0
UNINSTALL PLUGIN rpl_semi_sync_master;

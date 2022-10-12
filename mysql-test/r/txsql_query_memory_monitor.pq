set global cdb_query_memory_log_enabled=1;
set global cdb_query_memory_profile_enabled=1;
show variables like 'cdb_enable_resource_statistics';
Variable_name	Value
cdb_enable_resource_statistics	ON
show variables like '%query_memory%';
Variable_name	Value
cdb_query_memory_log_enabled	ON
cdb_query_memory_log_threshold	1073741824
cdb_query_memory_profile_enabled	ON
cdb_query_memory_profile_max_stage_num	100
cdb_query_memory_stack_enabled	OFF
set global cdb_enable_resource_statistics=0;
show variables like 'cdb_enable_resource_statistics';
Variable_name	Value
cdb_enable_resource_statistics	OFF
show variables like '%query_memory%';
Variable_name	Value
cdb_query_memory_log_enabled	OFF
cdb_query_memory_log_threshold	1073741824
cdb_query_memory_profile_enabled	OFF
cdb_query_memory_profile_max_stage_num	100
cdb_query_memory_stack_enabled	OFF
set global cdb_query_memory_log_enabled=1;
ERROR HY000: cdb_query_memory_log_enabled requires cdb_enable_resource_statistics and cdb_enable_precise_memory_usage to be ON.
set global cdb_query_memory_profile_enabled=1;
ERROR HY000: cdb_query_memory_profile_enabled requires cdb_memory_log_enabled to be ON.
set global cdb_enable_resource_statistics=1;
set global cdb_query_memory_profile_enabled=1;
ERROR HY000: cdb_query_memory_profile_enabled requires cdb_memory_log_enabled to be ON.
set global cdb_query_memory_log_enabled=1;
show variables like 'cdb_enable_resource_statistics';
Variable_name	Value
cdb_enable_resource_statistics	ON
show variables like '%query_memory%';
Variable_name	Value
cdb_query_memory_log_enabled	ON
cdb_query_memory_log_threshold	1073741824
cdb_query_memory_profile_enabled	OFF
cdb_query_memory_profile_max_stage_num	100
cdb_query_memory_stack_enabled	OFF
set global cdb_query_memory_profile_enabled=1;
show variables like 'cdb_enable_resource_statistics';
Variable_name	Value
cdb_enable_resource_statistics	ON
show variables like '%query_memory%';
Variable_name	Value
cdb_query_memory_log_enabled	ON
cdb_query_memory_log_threshold	1073741824
cdb_query_memory_profile_enabled	ON
cdb_query_memory_profile_max_stage_num	100
cdb_query_memory_stack_enabled	OFF
set @cdb_query_memory_log_threshold_old = @@cdb_query_memory_log_threshold;
set @cdb_query_memory_profile_max_stage_num_old = @@cdb_query_memory_profile_max_stage_num;
set global cdb_query_memory_log_threshold = 1;
set global cdb_query_memory_profile_max_stage_num=3;
set global cdb_query_memory_stack_enabled = 1;
SET DEBUG='+d, check_query_memory_profile_stages';
create database test233;
use test233;
create table t1(id int);
insert into t1 values(1);
drop database test233;
set global cdb_query_memory_stack_enabled = 0;
SET DEBUG='-d, check_query_memory_profile_stages';
set global cdb_query_memory_profile_max_stage_num = @cdb_query_memory_profile_max_stage_num_old;
set global cdb_query_memory_log_threshold = @cdb_query_memory_log_threshold_old;
set global cdb_query_memory_log_enabled=0;
show variables like 'cdb_enable_resource_statistics';
Variable_name	Value
cdb_enable_resource_statistics	ON
show variables like '%query_memory%';
Variable_name	Value
cdb_query_memory_log_enabled	OFF
cdb_query_memory_log_threshold	1073741824
cdb_query_memory_profile_enabled	OFF
cdb_query_memory_profile_max_stage_num	100
cdb_query_memory_stack_enabled	OFF
set global cdb_query_memory_log_enabled=1;

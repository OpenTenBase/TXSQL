create user 'tdsqlsys_test'@'localhost';
grant all on *.* to 'tdsqlsys_test'@'localhost';
call mtr.add_suppression(".*option 'tdsql_buffer_pool_optimize_pct': value.*");
show variables like 'innodb_buffer_pool_size';
Variable_name	Value
innodb_buffer_pool_size	25165824
show variables like 'tdsql_buffer_pool_optimize_pct';
Variable_name	Value
tdsql_buffer_pool_optimize_pct	0.800000
set global tdsql_buffer_pool_optimize_pct=0.5;
ERROR HY000: Variable 'tdsql_buffer_pool_optimize_pct' is a read only variable
# Restart server with a tdsql_buffer_pool_optimize_pct.
show variables like 'innodb_buffer_pool_size';
Variable_name	Value
innodb_buffer_pool_size	25165824
show variables like 'tdsql_buffer_pool_optimize_pct';
Variable_name	Value
tdsql_buffer_pool_optimize_pct	0.500000
# Restart server with a little tdsql_buffer_pool_optimize_pct.
show variables like 'innodb_buffer_pool_size';
Variable_name	Value
innodb_buffer_pool_size	25165824
show variables like 'tdsql_buffer_pool_optimize_pct';
Variable_name	Value
tdsql_buffer_pool_optimize_pct	0.100000
# Restart server with a big tdsql_buffer_pool_optimize_pct.
show variables like 'innodb_buffer_pool_size';
Variable_name	Value
innodb_buffer_pool_size	25165824
show variables like 'tdsql_buffer_pool_optimize_pct';
Variable_name	Value
tdsql_buffer_pool_optimize_pct	1.000000
drop user 'tdsqlsys_test'@'localhost';

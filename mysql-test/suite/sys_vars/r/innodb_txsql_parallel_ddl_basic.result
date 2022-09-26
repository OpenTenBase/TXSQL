SELECT @@global.innodb_txsql_parallel_ddl;
@@global.innodb_txsql_parallel_ddl
0
SELECT @@session.innodb_txsql_parallel_ddl;
@@session.innodb_txsql_parallel_ddl
0
SHOW GLOBAL VARIABLES LIKE  'innodb_txsql_parallel_ddl';
Variable_name	Value
innodb_txsql_parallel_ddl	OFF
SHOW SESSION VARIABLES LIKE 'innodb_txsql_parallel_ddl';
Variable_name	Value
innodb_txsql_parallel_ddl	OFF
SELECT * FROM performance_schema.global_variables
WHERE variable_name='innodb_txsql_parallel_ddl';
VARIABLE_NAME	VARIABLE_VALUE
innodb_txsql_parallel_ddl	OFF
SELECT * FROM performance_schema.session_variables
WHERE variable_name='innodb_txsql_parallel_ddl';
VARIABLE_NAME	VARIABLE_VALUE
innodb_txsql_parallel_ddl	OFF
set global innodb_txsql_parallel_ddl=ON;
set session innodb_txsql_parallel_ddl=ON;
set global innodb_txsql_parallel_ddl=OFF;
set session innodb_txsql_parallel_ddl=OFF;

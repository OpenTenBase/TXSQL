SET GLOBAL cdb_role= 'CDB_ROLE_UNKNOWN';
SELECT COUNT(@@GLOBAL.cdb_role);
COUNT(@@GLOBAL.cdb_role)
1
SELECT COUNT(@@SESSION.cdb_role);
ERROR HY000: Variable 'cdb_role' is a GLOBAL variable
SELECT VARIABLE_NAME FROM performance_schema.global_variables WHERE VARIABLE_NAME='cdb_role';
VARIABLE_NAME
cdb_role
SELECT VARIABLE_NAME FROM performance_schema.session_variables WHERE variable_name='cdb_role';
VARIABLE_NAME
cdb_role
SET GLOBAL cdb_role= 'CDB_ROLE_UNKNOWN';
include/assert.inc ['cdb_role is a dynamic variable']
SET GLOBAL cdb_role= 'CDB_ROLE_UNKNOWN';
include/assert.inc ['cdb_role should be CDB_ROLE_UNKNOWN']
SET GLOBAL cdb_role= 0;
include/assert.inc ['cdb_role should be CDB_ROLE_UNKNOWN']
SET GLOBAL cdb_role= 'CDB_ROLE_MASTER';
include/assert.inc ['cdb_role should be CDB_ROLE_MASTER']
SET GLOBAL cdb_role= 1;
include/assert.inc ['cdb_role should be CDB_ROLE_MASTER']
SET GLOBAL cdb_role= 'CDB_ROLE_SLAVE';
include/assert.inc ['cdb_role should be CDB_ROLE_SLAVE']
SET GLOBAL cdb_role= 2;
include/assert.inc ['cdb_role should be CDB_ROLE_SLAVE']
SET GLOBAL cdb_role= 'CDB_ROLE_RO';
include/assert.inc ['cdb_role should be CDB_ROLE_RO']
SET GLOBAL cdb_role= 3;
include/assert.inc ['cdb_role should be CDB_ROLE_RO']
SET GLOBAL cdb_role= DEFAULT;
include/assert.inc ['cdb_role should be CDB_ROLE_UNKNOWN']
SET GLOBAL cdb_role= NULL;
ERROR 42000: Variable 'cdb_role' can't be set to the value of 'NULL'
SET GLOBAL cdb_role= '';
ERROR 42000: Variable 'cdb_role' can't be set to the value of ''
SET GLOBAL cdb_role= 1.0;
ERROR 42000: Incorrect argument type to variable 'cdb_role'
SET GLOBAL cdb_role= 'GARBAGE';
ERROR 42000: Variable 'cdb_role' can't be set to the value of 'GARBAGE'
SET GLOBAL cdb_role= ON;
ERROR 42000: Variable 'cdb_role' can't be set to the value of 'ON'
SET GLOBAL cdb_role= OFF;
ERROR 42000: Variable 'cdb_role' can't be set to the value of 'OFF'
SET GLOBAL cdb_role= 4;
ERROR 42000: Variable 'cdb_role' can't be set to the value of '4'
Expect value still set to "CDB_ROLE_UNKNOWN"
SELECT @@global.cdb_role;
@@global.cdb_role
CDB_ROLE_UNKNOWN

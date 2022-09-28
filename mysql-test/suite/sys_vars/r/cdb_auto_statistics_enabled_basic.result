#
# Variable name : cdb_auto_statistics_enabled
# Scope         : Global
#
SET @global_saved_tmp =  @@global.cdb_auto_statistics_enabled;

# Global - default
SELECT @@global.cdb_auto_statistics_enabled;
@@global.cdb_auto_statistics_enabled
0
SELECT @@session.cdb_auto_statistics_enabled;
ERROR HY000: Variable 'cdb_auto_statistics_enabled' is a GLOBAL variable

SET @@global.cdb_auto_statistics_enabled= 'true';
ERROR 42000: Variable 'cdb_auto_statistics_enabled' can't be set to the value of 'true'

SET @@global.cdb_auto_statistics_enabled= 100;
ERROR 42000: Variable 'cdb_auto_statistics_enabled' can't be set to the value of '100'

SET @@global.cdb_auto_statistics_enabled= WRONGVALUE;
ERROR 42000: Variable 'cdb_auto_statistics_enabled' can't be set to the value of 'WRONGVALUE'

# via performance_schema.global_variables
SELECT * FROM performance_schema.global_variables WHERE VARIABLE_NAME LIKE 'cdb_auto_statistics_enabled';
VARIABLE_NAME	VARIABLE_VALUE
cdb_auto_statistics_enabled	OFF
SET @@global.cdb_auto_statistics_enabled = 0;
SELECT @@global.cdb_auto_statistics_enabled;
@@global.cdb_auto_statistics_enabled
0
SET @@global.cdb_auto_statistics_enabled = TrUe;
SELECT @@global.cdb_auto_statistics_enabled;
@@global.cdb_auto_statistics_enabled
1
SET @@global.cdb_auto_statistics_enabled = FaLsE;
SELECT @@global.cdb_auto_statistics_enabled;
@@global.cdb_auto_statistics_enabled
0

# Variables' values in a new session.
# Global - expect 0
SELECT @@global.cdb_auto_statistics_enabled;
@@global.cdb_auto_statistics_enabled
0

# Switching to the default connection.
SELECT @@global.cdb_auto_statistics_enabled;
@@global.cdb_auto_statistics_enabled
0

# Test if DEFAULT is working as expected.
SET @@global.cdb_auto_statistics_enabled = DEFAULT;

# Global - expect 0
SELECT @@global.cdb_auto_statistics_enabled;
@@global.cdb_auto_statistics_enabled
0

# Restoring the original values.
SET @@global.cdb_auto_statistics_enabled = @global_saved_tmp;
# End of tests.

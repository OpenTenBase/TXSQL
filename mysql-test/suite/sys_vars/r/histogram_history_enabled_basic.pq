#
# Variable name : histogram_history_enabled
# Scope         : Global
#
# Global - default
SELECT @@global.histogram_history_enabled;
@@global.histogram_history_enabled
0
SELECT @@session.histogram_history_enabled;
ERROR HY000: Variable 'histogram_history_enabled' is a GLOBAL variable

SET @@global.histogram_history_enabled= 'true';
ERROR 42000: Variable 'histogram_history_enabled' can't be set to the value of 'true'

SET @@global.histogram_history_enabled= 100;
ERROR 42000: Variable 'histogram_history_enabled' can't be set to the value of '100'

SET @@global.histogram_history_enabled= WRONGVALUE;
ERROR 42000: Variable 'histogram_history_enabled' can't be set to the value of 'WRONGVALUE'

# via performance_schema.global_variables
SELECT * FROM performance_schema.global_variables WHERE VARIABLE_NAME LIKE 'histogram_history_enabled';
VARIABLE_NAME	VARIABLE_VALUE
histogram_history_enabled	OFF
SET @global_saved_tmp =  @@global.histogram_history_enabled;

# Altering global variable's value
SET @@global.histogram_history_enabled = 0;
SELECT @@global.histogram_history_enabled;
@@global.histogram_history_enabled
0
SET @@global.histogram_history_enabled = TrUe;
SELECT @@global.histogram_history_enabled;
@@global.histogram_history_enabled
1
SET @@global.histogram_history_enabled = FaLsE;
SELECT @@global.histogram_history_enabled;
@@global.histogram_history_enabled
0

# Variables' values in a new session.
# Global - expect 0
SELECT @@global.histogram_history_enabled;
@@global.histogram_history_enabled
0

# Switching to the default connection.
SELECT @@global.histogram_history_enabled;
@@global.histogram_history_enabled
0

# Test if DEFAULT is working as expected.
SET @@global.histogram_history_enabled = DEFAULT;

# Global - expect 0
SELECT @@global.histogram_history_enabled;
@@global.histogram_history_enabled
0

# Restoring the original values.
SET @@global.histogram_history_enabled = @global_saved_tmp;
# End of tests.

SELECT @@session.show_old_temporals;
@@session.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SHOW SESSION VARIABLES LIKE 'show_old_temporals';
Variable_name	Value
show_old_temporals	OFF
SELECT * FROM performance_schema.session_variables WHERE variable_name='show_old_temporals';
VARIABLE_NAME	VARIABLE_VALUE
show_old_temporals	OFF
#Also can be set as a global variable.
SELECT @@global.show_old_temporals;
@@global.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SHOW GLOBAL VARIABLES LIKE 'show_old_temporals';
Variable_name	Value
show_old_temporals	OFF
SELECT * FROM performance_schema.global_variables WHERE variable_name='show_old_temporals';
VARIABLE_NAME	VARIABLE_VALUE
show_old_temporals	OFF
#Test setting the variable to various values.
#Boolean values.
SET SESSION show_old_temporals= ON;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@session.show_old_temporals;
@@session.show_old_temporals
1
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SET SESSION show_old_temporals= OFF;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@session.show_old_temporals;
@@session.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SET SESSION show_old_temporals= 1;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@session.show_old_temporals;
@@session.show_old_temporals
1
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SET SESSION show_old_temporals= 0;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@session.show_old_temporals;
@@session.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
#Test for DEFAULT value.
SET SESSION show_old_temporals= DEFAULT;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@session.show_old_temporals;
@@session.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
#Test for invalid value.
SET SESSION show_old_temporals= 2;
ERROR 42000: Variable 'show_old_temporals' can't be set to the value of '2'
#Test for setting the session variable
#using different syntax.
SET show_old_temporals= 1;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
#Fetches the current session value for the variable.
SELECT @@show_old_temporals;
@@show_old_temporals
1
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
#cleanup
SET SESSION show_old_temporals= DEFAULT;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
#Test setting the global value of the variable.
#Boolean values.
SET GLOBAL show_old_temporals= ON;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@global.show_old_temporals;
@@global.show_old_temporals
1
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SET GLOBAL show_old_temporals= OFF;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@global.show_old_temporals;
@@global.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SET GLOBAL show_old_temporals= 1;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@global.show_old_temporals;
@@global.show_old_temporals
1
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SET GLOBAL show_old_temporals= 0;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@session.show_old_temporals;
@@session.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
#Test for DEFAULT value.
SET GLOBAL show_old_temporals= DEFAULT;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@global.show_old_temporals;
@@global.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
#Test for invalid value.
SET GLOBAL show_old_temporals= 2;
ERROR 42000: Variable 'show_old_temporals' can't be set to the value of '2'
CREATE USER foo, bar;
GRANT SESSION_VARIABLES_ADMIN ON *.*  TO bar;
SELECT @@session.show_old_temporals;
@@session.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SET @@session.show_old_temporals = 0;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@session.show_old_temporals;
@@session.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SET @@session.show_old_temporals = 0;
Warnings:
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
SELECT @@session.show_old_temporals;
@@session.show_old_temporals
0
Warning	1287	'@@show_old_temporals' is deprecated and will be removed in a future release.
Warnings:
SET @@global.show_old_temporals = 0;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
DROP USER foo, bar;

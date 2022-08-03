SET @start_value = @@global.cdb_enable_key_rotate;
SELECT @start_value;
@start_value
1
'#---------------------FN_DYNVARS_004_01-------------------------#'
SET @@global.cdb_enable_key_rotate = DEFAULT;
SELECT @@global.cdb_enable_key_rotate = TRUE;
@@global.cdb_enable_key_rotate = TRUE
1
'#--------------------FN_DYNVARS_004_02------------------------#'
SET @@global.cdb_enable_key_rotate = ON;
SELECT @@global.cdb_enable_key_rotate;
@@global.cdb_enable_key_rotate
1
SET @@global.cdb_enable_key_rotate = OFF;
SELECT @@global.cdb_enable_key_rotate;
@@global.cdb_enable_key_rotate
0
'#--------------------FN_DYNVARS_004_03-------------------------#'
SET @@global.cdb_enable_key_rotate = 2;
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of '2'
SET @@global.cdb_enable_key_rotate = -1;
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of '-1'
SET @@global.cdb_enable_key_rotate = TRUEF;
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of 'TRUEF'
SET @@global.cdb_enable_key_rotate = TRUE_F;
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of 'TRUE_F'
SET @@global.cdb_enable_key_rotate = FALSE0;
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of 'FALSE0'
SET @@global.cdb_enable_key_rotate = OON;
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of 'OON'
SET @@global.cdb_enable_key_rotate = ONN;
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of 'ONN'
SET @@global.cdb_enable_key_rotate = OOFF;
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of 'OOFF'
SET @@global.cdb_enable_key_rotate = 0FF;
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of '0FF'
SET @@global.cdb_enable_key_rotate = ' ';
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of ' '
SET @@global.cdb_enable_key_rotate = " ";
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of ' '
SET @@global.cdb_enable_key_rotate = '';
ERROR 42000: Variable 'cdb_enable_key_rotate' can't be set to the value of ''
'#-------------------FN_DYNVARS_004_04----------------------------#'
SET @@session.cdb_enable_key_rotate = OFF;
ERROR HY000: Variable 'cdb_enable_key_rotate' is a GLOBAL variable and should be set with SET GLOBAL
SELECT @@session.cdb_enable_key_rotate;
ERROR HY000: Variable 'cdb_enable_key_rotate' is a GLOBAL variable
'#----------------------FN_DYNVARS_004_05------------------------#'
SELECT IF(@@global.cdb_enable_key_rotate, "ON", "OFF") = VARIABLE_VALUE 
FROM performance_schema.global_variables 
WHERE VARIABLE_NAME='cdb_enable_key_rotate';
IF(@@global.cdb_enable_key_rotate, "ON", "OFF") = VARIABLE_VALUE
1
'#---------------------FN_DYNVARS_004_06----------------------#'
SET @@global.cdb_enable_key_rotate = 0;
SELECT @@global.cdb_enable_key_rotate;
@@global.cdb_enable_key_rotate
0
SET @@global.cdb_enable_key_rotate = 1;
SELECT @@global.cdb_enable_key_rotate;
@@global.cdb_enable_key_rotate
1
'#---------------------FN_DYNVARS_004_07----------------------#'
SET @@global.cdb_enable_key_rotate = TRUE;
SELECT @@global.cdb_enable_key_rotate;
@@global.cdb_enable_key_rotate
1
SET @@global.cdb_enable_key_rotate = FALSE;
SELECT @@global.cdb_enable_key_rotate;
@@global.cdb_enable_key_rotate
0
'#---------------------FN_DYNVARS_004_08----------------------#'
SET @@global.cdb_enable_key_rotate = ON;
SELECT @@cdb_enable_key_rotate = @@global.cdb_enable_key_rotate;
@@cdb_enable_key_rotate = @@global.cdb_enable_key_rotate
1
'#-----------------------FN_DYNVARS_004_09---------------------------#'
SET cdb_enable_key_rotate = ON;
ERROR HY000: Variable 'cdb_enable_key_rotate' is a GLOBAL variable and should be set with SET GLOBAL
SET local.cdb_enable_key_rotate = OFF;
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'local.cdb_enable_key_rotate = OFF' at line 1
SELECT local.cdb_enable_key_rotate;
ERROR 42S02: Unknown table 'local' in field list
SET global.cdb_enable_key_rotate = ON;
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'global.cdb_enable_key_rotate = ON' at line 1
SELECT global.cdb_enable_key_rotate;
ERROR 42S02: Unknown table 'global' in field list
SELECT cdb_enable_key_rotate = @@session.cdb_enable_key_rotate;
ERROR 42S22: Unknown column 'cdb_enable_key_rotate' in 'field list'
SET @@global.cdb_enable_key_rotate = @start_value;
SELECT @@global.cdb_enable_key_rotate;
@@global.cdb_enable_key_rotate
1

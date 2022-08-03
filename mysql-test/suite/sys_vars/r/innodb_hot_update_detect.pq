SELECT  @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
0
SET GLOBAL innodb_hot_update_detect = true;
SELECT @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
1
SET GLOBAL innodb_hot_update_detect = false;
SELECT @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
0
SET GLOBAL innodb_hot_update_detect = on;
SELECT @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
1
SET GLOBAL innodb_hot_update_detect = off;
SELECT @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
0
SET GLOBAL innodb_hot_update_detect = 'abck';
ERROR 42000: Variable 'innodb_hot_update_detect' can't be set to the value of 'abck'
SELECT @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
0
SET GLOBAL innodb_hot_update_detect = 0;
SELECT @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
0
SET GLOBAL innodb_hot_update_detect = 1;
SELECT @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
1
SET GLOBAL innodb_hot_update_detect = 2;
ERROR 42000: Variable 'innodb_hot_update_detect' can't be set to the value of '2'
SELECT @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
1
SET GLOBAL innodb_hot_update_detect = 123;
ERROR 42000: Variable 'innodb_hot_update_detect' can't be set to the value of '123'
SELECT @@global.innodb_hot_update_detect;
@@global.innodb_hot_update_detect
1
SET GLOBAL innodb_hot_update_detect = default;

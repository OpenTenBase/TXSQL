SELECT  @@global.innodb_hot_update_wait_timeout;
@@global.innodb_hot_update_wait_timeout
1000000
SET GLOBAL innodb_hot_update_wait_timeout = 1000000;
SELECT @@global.innodb_hot_update_wait_timeout;
@@global.innodb_hot_update_wait_timeout
1000000
SET GLOBAL innodb_hot_update_wait_timeout = 1000;
SELECT @@global.innodb_hot_update_wait_timeout;
@@global.innodb_hot_update_wait_timeout
1000
SET GLOBAL innodb_hot_update_wait_timeout = 'abck';
ERROR 42000: Incorrect argument type to variable 'innodb_hot_update_wait_timeout'
SELECT @@global.innodb_hot_update_wait_timeout;
@@global.innodb_hot_update_wait_timeout
1000
SET GLOBAL innodb_hot_update_wait_timeout = 'foobar';
ERROR 42000: Incorrect argument type to variable 'innodb_hot_update_wait_timeout'
SELECT @@global.innodb_hot_update_wait_timeout;
@@global.innodb_hot_update_wait_timeout
1000
SET GLOBAL innodb_hot_update_wait_timeout = 0;
Warnings:
Warning	1292	Truncated incorrect innodb_hot_update_wait_timeout value: '0'
SELECT @@global.innodb_hot_update_wait_timeout;
@@global.innodb_hot_update_wait_timeout
1000
SET GLOBAL innodb_hot_update_wait_timeout = 100000000000;
Warnings:
Warning	1292	Truncated incorrect innodb_hot_update_wait_timeout value: '100000000000'
SELECT @@global.innodb_hot_update_wait_timeout;
@@global.innodb_hot_update_wait_timeout
100000000
SET GLOBAL innodb_hot_update_wait_timeout = default;

SELECT  @@global.innodb_max_concurrent_hot_update;
@@global.innodb_max_concurrent_hot_update
1
SET GLOBAL innodb_max_concurrent_hot_update = 16;
SELECT @@global.innodb_max_concurrent_hot_update;
@@global.innodb_max_concurrent_hot_update
16
SET GLOBAL innodb_max_concurrent_hot_update = 32;
SELECT @@global.innodb_max_concurrent_hot_update;
@@global.innodb_max_concurrent_hot_update
32
SET GLOBAL innodb_max_concurrent_hot_update = 'abck';
ERROR 42000: Incorrect argument type to variable 'innodb_max_concurrent_hot_update'
SELECT @@global.innodb_max_concurrent_hot_update;
@@global.innodb_max_concurrent_hot_update
32
SET GLOBAL innodb_max_concurrent_hot_update = 'foobar';
ERROR 42000: Incorrect argument type to variable 'innodb_max_concurrent_hot_update'
SELECT @@global.innodb_max_concurrent_hot_update;
@@global.innodb_max_concurrent_hot_update
32
SET GLOBAL innodb_max_concurrent_hot_update = 0;
Warnings:
Warning	1292	Truncated incorrect innodb_max_concurrent_hot_update value: '0'
SELECT @@global.innodb_max_concurrent_hot_update;
@@global.innodb_max_concurrent_hot_update
1
SET GLOBAL innodb_max_concurrent_hot_update = 100;
Warnings:
Warning	1292	Truncated incorrect innodb_max_concurrent_hot_update value: '100'
SELECT @@global.innodb_max_concurrent_hot_update;
@@global.innodb_max_concurrent_hot_update
32
SET GLOBAL innodb_max_concurrent_hot_update = default;

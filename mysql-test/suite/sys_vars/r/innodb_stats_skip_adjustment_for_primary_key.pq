SELECT @@innodb_stats_skip_adjustment_for_primary_key;
@@innodb_stats_skip_adjustment_for_primary_key
0
SET GLOBAL innodb_stats_skip_adjustment_for_primary_key=1;
SELECT @@innodb_stats_skip_adjustment_for_primary_key;
@@innodb_stats_skip_adjustment_for_primary_key
1
SET SESSION innodb_stats_skip_adjustment_for_primary_key=1;
ERROR HY000: Variable 'innodb_stats_skip_adjustment_for_primary_key' is a GLOBAL variable and should be set with SET GLOBAL
SET GLOBAL innodb_stats_skip_adjustment_for_primary_key=100;
ERROR 42000: Variable 'innodb_stats_skip_adjustment_for_primary_key' can't be set to the value of '100'
SET GLOBAL innodb_stats_skip_adjustment_for_primary_key=foo;
ERROR 42000: Variable 'innodb_stats_skip_adjustment_for_primary_key' can't be set to the value of 'foo'
SET GLOBAL innodb_stats_skip_adjustment_for_primary_key=OFF ;
SELECT @@innodb_stats_skip_adjustment_for_primary_key;
@@innodb_stats_skip_adjustment_for_primary_key
0
SET GLOBAL innodb_stats_skip_adjustment_for_primary_key=ON ;
SELECT @@innodb_stats_skip_adjustment_for_primary_key;
@@innodb_stats_skip_adjustment_for_primary_key
1
SET GLOBAL innodb_stats_skip_adjustment_for_primary_key=Default ;
SELECT @@innodb_stats_skip_adjustment_for_primary_key;
@@innodb_stats_skip_adjustment_for_primary_key
0

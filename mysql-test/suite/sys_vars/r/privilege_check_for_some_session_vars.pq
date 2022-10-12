call mtr.add_suppression(".*has been updated.*");
create user 'test_user1'@'localhost';
create user 'test_user2'@'localhost';
create user 'test_user3'@'localhost';
create user 'test_user4'@'localhost';
create user 'test_user5'@'localhost';
grant all privileges on *.* to 'test_user2'@'localhost';
grant all privileges on test.* to 'test_user3'@'localhost';
grant super on *.* to 'test_user4'@'localhost';
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
grant session_variables_admin on *.* to 'test_user5'@'localhost';
flush privileges;
set global cdb_session_var_privilege_check_enabled=ON;
show variables like 'cdb_session_var_privilege_check_enabled';
Variable_name	Value
cdb_session_var_privilege_check_enabled	ON
select user();
user()
root@localhost
set global innodb_strict_mode=ON;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
1
set global innodb_strict_mode=OFF;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global sql_mode='ALLOW_INVALID_DATES';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
ALLOW_INVALID_DATES
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
NO_AUTO_VALUE_ON_ZERO
set global unique_checks=OFF;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
0
set global unique_checks=ON;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
1
set global binlog_row_image='FULL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
FULL
set global binlog_row_image='MINIMAL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
MINIMAL
set session innodb_strict_mode=ON;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
0
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
select user();
user()
tencentroot@localhost
set global innodb_strict_mode=ON;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
1
set global innodb_strict_mode=OFF;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global sql_mode='ALLOW_INVALID_DATES';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
ALLOW_INVALID_DATES
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
NO_AUTO_VALUE_ON_ZERO
set global unique_checks=OFF;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
0
set global unique_checks=ON;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
1
set global binlog_row_image='FULL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
FULL
set global binlog_row_image='MINIMAL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
MINIMAL
set session innodb_strict_mode=ON;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
0
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
select user();
user()
test_user1@localhost
set global innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global sql_mode='ALLOW_INVALID_DATES';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global unique_checks=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global unique_checks=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global binlog_row_image='FULL';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global binlog_row_image='minimal';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set session innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session sql_mode='ALLOW_INVALID_DATES';
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
set session unique_checks=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session unique_checks=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session binlog_row_image='FULL';
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session binlog_row_image='minimal';
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
select user();
user()
test_user2@localhost
set global innodb_strict_mode=ON;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
1
set global innodb_strict_mode=OFF;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global sql_mode='ALLOW_INVALID_DATES';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
ALLOW_INVALID_DATES
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
NO_AUTO_VALUE_ON_ZERO
set global unique_checks=OFF;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
0
set global unique_checks=ON;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
1
set global binlog_row_image='FULL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
FULL
set global binlog_row_image='MINIMAL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
MINIMAL
set session innodb_strict_mode=ON;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
0
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
select user();
user()
test_user3@localhost
set global innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global sql_mode='ALLOW_INVALID_DATES';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global unique_checks=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global unique_checks=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global binlog_row_image='FULL';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global binlog_row_image='minimal';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set session innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session sql_mode='ALLOW_INVALID_DATES';
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
set session unique_checks=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session unique_checks=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session binlog_row_image='FULL';
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
set session binlog_row_image='minimal';
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
select user();
user()
test_user4@localhost
set global innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global sql_mode='ALLOW_INVALID_DATES';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
ALLOW_INVALID_DATES
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
NO_AUTO_VALUE_ON_ZERO
set global unique_checks=OFF;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
0
set global unique_checks=ON;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
1
set global binlog_row_image='FULL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
FULL
set global binlog_row_image='MINIMAL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
MINIMAL
set session innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
select user();
user()
test_user5@localhost
set session innodb_strict_mode=ON;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
0
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
set global cdb_session_var_privilege_check_enabled=OFF;
show variables like 'cdb_session_var_privilege_check_enabled';
Variable_name	Value
cdb_session_var_privilege_check_enabled	OFF
select user();
user()
root@localhost
set global innodb_strict_mode=ON;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
1
set global innodb_strict_mode=OFF;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global sql_mode='ALLOW_INVALID_DATES';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
ALLOW_INVALID_DATES
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
NO_AUTO_VALUE_ON_ZERO
set global unique_checks=OFF;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
0
set global unique_checks=ON;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
1
set global binlog_row_image='FULL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
FULL
set global binlog_row_image='MINIMAL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
MINIMAL
set session innodb_strict_mode=ON;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
0
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
select user();
user()
tencentroot@localhost
set global innodb_strict_mode=ON;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
1
set global innodb_strict_mode=OFF;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global sql_mode='ALLOW_INVALID_DATES';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
ALLOW_INVALID_DATES
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
NO_AUTO_VALUE_ON_ZERO
set global unique_checks=OFF;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
0
set global unique_checks=ON;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
1
set global binlog_row_image='FULL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
FULL
set global binlog_row_image='MINIMAL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
MINIMAL
set session innodb_strict_mode=ON;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
0
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
select user();
user()
test_user1@localhost
set global innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global sql_mode='ALLOW_INVALID_DATES';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global unique_checks=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global unique_checks=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global binlog_row_image='FULL';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global binlog_row_image='MINIMAL';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set session innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
select user();
user()
test_user2@localhost
set global innodb_strict_mode=ON;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
1
set global innodb_strict_mode=OFF;
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global sql_mode='ALLOW_INVALID_DATES';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
ALLOW_INVALID_DATES
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
NO_AUTO_VALUE_ON_ZERO
set global unique_checks=OFF;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
0
set global unique_checks=ON;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
1
set global binlog_row_image='FULL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
FULL
set global binlog_row_image='MINIMAL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
MINIMAL
set session innodb_strict_mode=ON;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
0
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
select user();
user()
test_user3@localhost
set global innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global sql_mode='ALLOW_INVALID_DATES';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global unique_checks=OFF;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global unique_checks=ON;
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global binlog_row_image='FULL';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global binlog_row_image='MINIMAL';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set session innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
ERROR 42000: Access denied; you need (at least one of) the SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
select user();
user()
test_user4@localhost
set global innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@GLOBAL.innodb_strict_mode;
@@GLOBAL.innodb_strict_mode
0
set global sql_mode='ALLOW_INVALID_DATES';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
ALLOW_INVALID_DATES
set global sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@GLOBAL.sql_mode;
@@GLOBAL.sql_mode
NO_AUTO_VALUE_ON_ZERO
set global unique_checks=OFF;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
0
set global unique_checks=ON;
SELECT @@GLOBAL.unique_checks;
@@GLOBAL.unique_checks
1
set global binlog_row_image='FULL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
FULL
set global binlog_row_image='MINIMAL';
SELECT @@GLOBAL.binlog_row_image;
@@GLOBAL.binlog_row_image
MINIMAL
set session innodb_strict_mode=ON;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
ERROR 42000: Access denied; you need (at least one of) the SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN privilege(s) for this operation
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
show variables like 'innodb_autoinc_lock_mode';
Variable_name	Value
innodb_autoinc_lock_mode	2
set global binlog_format=row;
set global binlog_format=statement;
set global binlog_format=mixed;
set binlog_format=statement;
set binlog_format=mixed;
SET GLOBAL binlog_format=ROW;
select user();
user()
test_user5@localhost
set session innodb_strict_mode=ON;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
1
set session innodb_strict_mode=OFF;
SELECT @@SESSION.innodb_strict_mode;
@@SESSION.innodb_strict_mode
0
set session sql_mode='ALLOW_INVALID_DATES';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
ALLOW_INVALID_DATES
set session sql_mode='NO_AUTO_VALUE_ON_ZERO';
SELECT @@SESSION.sql_mode;
@@SESSION.sql_mode
NO_AUTO_VALUE_ON_ZERO
set session unique_checks=OFF;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
0
set session unique_checks=ON;
SELECT @@SESSION.unique_checks;
@@SESSION.unique_checks
1
set session binlog_row_image='FULL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
FULL
set session binlog_row_image='MINIMAL';
SELECT @@SESSION.binlog_row_image;
@@SESSION.binlog_row_image
MINIMAL
SET GLOBAL innodb_strict_mode=1;
SET GLOBAL sql_mode=default;
SET GLOBAL unique_checks=1;
SET GLOBAL binlog_row_image=FULL;
drop user 'test_user1'@'localhost';
drop user 'test_user2'@'localhost';
drop user 'test_user3'@'localhost';
drop user 'test_user4'@'localhost';
drop user 'test_user5'@'localhost';

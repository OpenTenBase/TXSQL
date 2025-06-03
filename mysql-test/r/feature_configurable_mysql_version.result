select @mysql_version_is_equal;
@mysql_version_is_equal
1
create user 'xx1'@'%';
set session version = '5.7.12';
ERROR HY000: Variable 'version' is a GLOBAL variable and should be set with SET GLOBAL
set global version = '5.7.12';
ERROR 42000: Access denied; you need (at least one of) the SUPER or SYSTEM_VARIABLES_ADMIN privilege(s) for this operation
set global version = '5.6.34';
select version();
version()
5.6.34
show variables like 'version';
Variable_name	Value
version	5.6.34
select @mysql_version_is_equal;
@mysql_version_is_equal
1
select version();
version()
5.6.34
show variables like 'version';
Variable_name	Value
version	5.6.34
set global version = '5.7.32333333333333333333333333333333333333333333333333333334';
ERROR HY000: The length specified for version is too long.
set global version = '5.7.3233333333333333333333333333333333333333333333333333333';
select version();
version()
5.7.3233333333333333333333333333333333333333333333333333333
show variables like 'version';
Variable_name	Value
version	5.7.3233333333333333333333333333333333333333333333333333333
select version();
version()
5.7.3233333333333333333333333333333333333333333333333333333
show variables like 'version';
Variable_name	Value
version	5.7.3233333333333333333333333333333333333333333333333333333
set global version = '5.7.233';
select version();
version()
5.7.233
show variables like 'version';
Variable_name	Value
version	5.7.233
set global version = default;
select @mysql_version_is_equal;
@mysql_version_is_equal
1
set global version = null;
ERROR 42000: Variable 'version' can't be set to the value of 'NULL'
drop user 'xx1'@'%';
select 1;
1
1

create user 'dev'@'localhost' identified by 'dev';
grant all on *.* to 'dev'@'localhost' with grant option;
revoke FILE,SHUTDOWN,SUPER on *.* from 'dev'@'localhost';
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
flush privileges;
select user();
user()
dev@localhost
reset master;
ERROR 42000: Access denied; you need (at least one of) the SUPER privilege(s) for this operation
REVOKE ALL PRIVILEGES on *.* FROM 'dev'@'localhost';
reset master;
drop user 'dev'@'localhost';

create user 'firewalltest'@'localhost';
grant all on mysql.* to 'firewalltest'@'localhost';

use test;
select cdb_normalize_statement("select * from t1") into @whitelist_rule;
select @whitelist_rule;
insert into mysql.cdb_firewall_whitelist values ('firewalltest@localhost', @whitelist_rule);
select * from information_schema.cdb_firewall_whitelist;

create table t1(id int);
create table t2(id int);

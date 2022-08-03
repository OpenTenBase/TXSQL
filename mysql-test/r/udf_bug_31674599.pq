call mtr.add_suppression("Invalid row in mysql.func table");
#
#Test null or empty name field value
use mysql;
insert into func values ('', 2, 'not_exist.so', 'function');
# restart
use mysql;
delete from func;
insert into func values ('not_exist', 2, '', 'function');
# restart
delete from func;

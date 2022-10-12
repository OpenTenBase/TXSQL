include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
call mtr.add_suppression("not in pt filter white list");
call mtr.add_suppression("PT add NULL user.");
call mtr.add_suppression("PT remove NULL user.");
show variables like '%\_pt\_%';
Variable_name	Value
cdb_pt_curr_white_list_user	
cdb_pt_filter	ON
cdb_pt_table_checksum	OFF
cdb_pt_white_list_add_user	
cdb_pt_white_list_rm_user	
cdb_pt_white_list_user	
set global cdb_pt_filter='off';
set global cdb_pt_filter='on';
set global cdb_pt_filter=default;
set binlog_format=row;
set cdb_pt_table_checksum=on;
ERROR HY000: Cannot set cdb_pt_table_checksum: It will cause transfer binlog events in mess state since BINLOG_FORMAT != STATEMENT.
set cdb_pt_table_checksum=off;
set global cdb_pt_table_checksum=on;
ERROR HY000: Variable 'cdb_pt_table_checksum' is a SESSION variable and can't be used with SET GLOBAL
set binlog_format=default;
set global cdb_pt_white_list_add_user='rpl1';
show variables like 'cdb_pt_curr_white_list_user';
Variable_name	Value
cdb_pt_curr_white_list_user	rpl1
set global cdb_pt_white_list_rm_user='rpl1';
show variables like 'cdb_pt_curr_white_list_user';
Variable_name	Value
cdb_pt_curr_white_list_user	
set global cdb_pt_white_list_rm_user='rpl_noexist';
set global cdb_pt_white_list_add_user=null;
set global cdb_pt_white_list_add_user=''

#remove null user;
set global cdb_pt_white_list_rm_user='';
set global cdb_pt_white_list_rm_user=null;
set global cdb_pt_white_list_rm_user=default;
show variables like '%cdb\_pt\_%';
Variable_name	Value
cdb_pt_curr_white_list_user	
cdb_pt_filter	ON
cdb_pt_table_checksum	OFF
cdb_pt_white_list_add_user	
cdb_pt_white_list_rm_user	
cdb_pt_white_list_user	
include/rpl_end.inc

include/master-slave.inc
Warnings:
Note	####	Sending passwords in plain text without SSL/TLS is extremely insecure.
Note	####	Storing MySQL user name or password information in the master info repository is not secure and is therefore not recommended. Please consider using the USER and PASSWORD connection options for START SLAVE; see the 'START SLAVE Syntax' in the MySQL Manual for more information.
[connection master]
#Test for create temporary table
create database tdb_pt;
set binlog_format='statement';
set cdb_pt_table_checksum=true;
use tdb_pt;
create temporary table t1(f1 int);
ERROR HY000: Can not create temporary table when cdb_pt_table_checksum turned on.
#clean up
drop database tdb_pt;
include/rpl_end.inc

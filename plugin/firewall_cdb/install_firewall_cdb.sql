/* Copyright (c) 2019, Tencent and/or its affiliates. All rights reserved.
   CDB audit for txsql 8.0 
*/

/*
  DB name 'mysql' and TABLE name 'cdb_firewall_users' will be used in firewall_table_service.h
  Add new table should check firewall_table_service.h

  Column name 'USERHOST', 'MODE', 'DIGEST' used in firewall_table_service.cc,
  in constructor of Whitelist_cursor and User_Mode_Cursor.
  If new column need to be added, column name should be uppercase in order to consistent with
  existing column name.
*/
create table if not exists mysql.cdb_firewall_users(
  -- in my_hostname.h HOSTNAME_LENGTH = 255, in mysql_com.h USERNAME_LENGTH = 32*3 = 96
  -- so MAX_USERHOST = 255 + 96 + 1 = 352, nearest (2^n-1) = 511 
  USERHOST varchar(511) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  MODE char(10) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  primary key(USERHOST) 
) engine=innodb default charset=utf8mb4;

create table if not exists mysql.cdb_firewall_whitelist(
  USERHOST varchar(511) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  DIGEST text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL
) engine=innodb default charset=utf8mb4;

INSTALL PLUGIN FIREWALL_CDB SONAME 'firewall_cdb.so';
INSTALL PLUGIN CDB_FIREWALL_USERS SONAME 'firewall_cdb.so';
INSTALL PLUGIN CDB_FIREWALL_WHITELIST SONAME 'firewall_cdb.so';
CREATE FUNCTION load_cdb_firewall_userhost_mode RETURNS STRING
SONAME 'firewall_cdb.so';
CREATE FUNCTION load_cdb_firewall_whitelist RETURNS STRING
SONAME 'firewall_cdb.so';
CREATE FUNCTION flush_cdb_firewall_whitelist RETURNS STRING
SONAME 'firewall_cdb.so';
CREATE FUNCTION reload_cdb_firewall_user_rules RETURNS STRING
SONAME 'firewall_cdb.so';
CREATE FUNCTION cdb_firewall_flush_status RETURNS STRING
SONAME 'firewall_cdb.so';
CREATE FUNCTION cdb_normalize_statement RETURNS STRING
SONAME 'firewall_cdb.so';
CREATE FUNCTION set_cdb_firewall_userhost_mode RETURNS STRING
SONAME 'firewall_cdb.so';

DROP PROCEDURE IF EXISTS mysql.sp_reload_cdb_firewall_user_rules;
delimiter //
CREATE PROCEDURE mysql.sp_reload_cdb_firewall_user_rules(IN username varchar(127), IN hostname varchar(255))
BEGIN
  DECLARE userhost varchar(511);
  select concat(username, '@', hostname) into userhost;
  delete from mysql.cdb_firewall_whitelist where USERHOST = userhost;
  select reload_cdb_firewall_user_rules(userhost);
END //
delimiter ;

DROP PROCEDURE IF EXISTS mysql.sp_set_cdb_firewall_mode;
delimiter //
CREATE PROCEDURE mysql.sp_set_cdb_firewall_mode(IN username varchar(127), IN hostname varchar(255), IN mode char(10))
BEGIN
  DECLARE user_cnt int;
  DECLARE userhost varchar(511);
  select concat(username, '@', hostname) into userhost;
  select count(*) from mysql.user where user = username into user_cnt;
  IF user_cnt = 0 THEN
    select "Warning: 'No such user in mysql.user";
    SIGNAL SQLSTATE '01000' SET MESSAGE_TEXT = 'No such user in mysql.user';
  END IF;
  IF mode NOT IN ("PROTECT", "DETECT", "OFF", "RECORDING") THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'mode must be in PROTECT, DETECT, OFF, RECORDING';
  END IF;
  replace into mysql.cdb_firewall_users (USERHOST, MODE) values (userhost, mode);
  select set_cdb_firewall_userhost_mode(userhost, mode);
END //
delimiter ;

-- insert into mysql.firewall_users values ('root@localhost', 'RECORDING');
-- insert into mysql.firewall_whitelist (USERHOST, DIGEST) values ('root@localhost', 'select ?');
-- select load_cdb_firewall_userhost_mode();

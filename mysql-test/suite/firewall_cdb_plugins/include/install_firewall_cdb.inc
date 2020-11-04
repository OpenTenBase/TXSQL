disable_query_log;
disable_warnings;

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

eval INSTALL PLUGIN FIREWALL_CDB SONAME '$FIREWALL_CDB';
eval INSTALL PLUGIN CDB_FIREWALL_USERS SONAME '$FIREWALL_CDB';
eval INSTALL PLUGIN CDB_FIREWALL_WHITELIST SONAME '$FIREWALL_CDB';
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
delimiter //;
CREATE PROCEDURE mysql.sp_reload_cdb_firewall_user_rules(IN username varchar(127), IN hostname varchar(255))
BEGIN
  DECLARE userhost varchar(511);
  select concat(username, '@', hostname) into userhost;
  delete from mysql.cdb_firewall_whitelist where USERHOST = userhost;
  select reload_cdb_firewall_user_rules(userhost);
END //
delimiter ;//

DROP PROCEDURE IF EXISTS mysql.sp_set_cdb_firewall_mode;
delimiter //;
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
delimiter ;//

disable_warnings;
enable_query_log;

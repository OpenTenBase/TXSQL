SET @save_sqlf=@@global.slow_query_log_file;
SET GLOBAL slow_query_log_file= '.../my_slow_for_tencentroot.log';
#
# 1. Test slow log in root user
#
SET timestamp=10;
SELECT unix_timestamp(), sleep(2);
unix_timestamp()	sleep(2)
10	0
[ PASS ] timestamp found 1
#
# 2. Test slow log in tencentroot user
#
SET @save_tencentroot_enabled=@@global.cdb_tencentroot_slow_log_enabled;
SET GLOBAL cdb_tencentroot_slow_log_enabled = off;
SET timestamp=10;
SELECT unix_timestamp(), sleep(2);
unix_timestamp()	sleep(2)
10	0
[ PASS ] no slow log is added
#
# 3. Test slow log in tencentroot user when cdb_tencentroot_slow_log_enabled truned on
#
SET GLOBAL cdb_tencentroot_slow_log_enabled = on;
SET timestamp=10;
SELECT unix_timestamp(), sleep(2);
unix_timestamp()	sleep(2)
10	0
[ PASS ] timestamp found 2
SET @@global.cdb_tencentroot_slow_log_enabled=@save_tencentroot_enabled;
SET @@global.slow_query_log_file=@save_sqlf;

# test for  posix_fallocate for creating temp table
# returns errors EINVAL
# restart
# Restart server to cleanup the temp tablespaces
# create connections and temp tables to fill up temp
# tablespace which allocated by default.
# establishing connection 1
# establishing connection 2
# establishing connection 3
# establishing connection 4
# establishing connection 5
SET SESSION debug='+d,ib_temp_posix_fallocate_fail_einval';
CREATE TEMPORARY TABLE ttext1(c1 INT, c2 TEXT);
SELECT ID,PATH FROM INFORMATION_SCHEMA.INNODB_SESSION_TEMP_TABLESPACES ORDER BY ID, PATH;
ID	PATH
0	./#innodb_temp/temp_11.ibt
0	./#innodb_temp/temp_12.ibt
0	./#innodb_temp/temp_13.ibt
0	./#innodb_temp/temp_14.ibt
0	./#innodb_temp/temp_15.ibt
0	./#innodb_temp/temp_16.ibt
0	./#innodb_temp/temp_17.ibt
0	./#innodb_temp/temp_18.ibt
0	./#innodb_temp/temp_19.ibt
8	./#innodb_temp/temp_10.ibt
9	./#innodb_temp/temp_8.ibt
9	./#innodb_temp/temp_9.ibt
10	./#innodb_temp/temp_6.ibt
10	./#innodb_temp/temp_7.ibt
11	./#innodb_temp/temp_4.ibt
11	./#innodb_temp/temp_5.ibt
12	./#innodb_temp/temp_2.ibt
12	./#innodb_temp/temp_3.ibt
13	./#innodb_temp/temp_1.ibt
13	./#innodb_temp/temp_20.ibt
SET SESSION debug='-d,ib_temp_posix_fallocate_fail_einval';
# cleaning up connection connection_1
# cleaning up connection connection_2
# cleaning up connection connection_3
# cleaning up connection connection_4
# cleaning up connection connection_5

# Write file to make mysql-test-run.pl start up the server again
--exec echo "restart" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect

--error 2013
ALTER TABLE t1 ADD INDEX k_c2(c2), add unique index k_c3(c3);

--enable_reconnect
--source include/wait_until_connected_again.inc
--disable_reconnect

SELECT COUNT(*) FROM t1;

CHECK TABLE t1;

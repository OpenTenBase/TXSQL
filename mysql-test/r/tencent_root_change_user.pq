#
# test txsql tencentroot change user
#
create user 'test_nopw'@'%' identified by "";
flush privileges;
ERROR HY000: Change user is not supported for tencent root.
ERROR 28000: Access denied for user 'tencentroot'@'localhost' (using password: NO)
drop user test_nopw;

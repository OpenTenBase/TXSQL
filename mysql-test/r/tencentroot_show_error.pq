#
# Prepare
#
CALL mtr.add_suppression('could not be resolved: Name or service not known');
call mtr.add_suppression(".*Hostname.*does not resolve to.*");
create table ip_holder(ip varchar(100));
set global tencent_root_whitelist=concat(@@tencent_root_whitelist, ";", "<local_ip>");
#
# run
#
#
# case1: normal user show processlist shouldn't show tencentroot information
show processlist;
Id	User	Host	db	Command	Time	State	Info
#	event_scheduler	#:#	NULL	Wait	#	Waiting on empty queue	NULL
#	root	#:#	test	Wait	#		NULL
#	root	#:#	test	Wait	#	init	show processlist
#
# case2: tencentroot user show processlist should show host information
show processlist;
Id	User	Host	db	Command	Time	State	Info
#	event_scheduler	#:#	NULL	Wait	#	Waiting on empty queue	NULL
#	root	#:#	test	Wait	#		NULL
#	root	#:#	test	Wait	#		NULL
#	tencentroot	#:#	test	Wait	#	init	show processlist
#	tencentroot	#:#	test	Wait	#		NULL
#	tencentroot	#:#	test	Wait	#		NULL
#
# Cleanup
#
set global tencent_root_whitelist="localhost;127.0.0.1;::1";
drop table ip_holder;

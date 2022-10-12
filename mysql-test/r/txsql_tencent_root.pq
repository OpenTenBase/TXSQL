#connect user=tencentroot
#1.1 check max_connections for non tencent users.
set global max_connections=2;
#connect con_tencentroot_1
#reserved connections test
#1.2 check processlist for non tencent users.
#non tencent connection
show processlist;
Id	User	Host	db	Command	Time	State	Info
#####	event_scheduler	localhost	NULL	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	show processlist
#####	root	localhost	test	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	NULL
select id, user, command from information_schema.processlist order by user;
id	user	command
#####	event_scheduler	#####
#####	root	#####
#####	root	#####
#####	root	#####
#1.3 check show processlist for tencent users.
show processlist;
Id	User	Host	db	Command	Time	State	Info
#####	event_scheduler	localhost	NULL	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	NULL
#####	tencentroot	localhost	test	#####	##	#####	show processlist
#####	root	localhost	test	#####	##	#####	NULL
#####	root	localhost	test	#####	##	#####	NULL
#####	tencentroot	localhost	test	#####	##	#####	NULL
#####	tencentroot	localhost	test	#####	##	#####	NULL
select id, user, command from information_schema.processlist order by user;
id	user	command
#####	event_scheduler	#####
#####	root	#####
#####	root	#####
#####	root	#####
#####	tencentroot	#####
#####	tencentroot	#####
#####	tencentroot	#####
#1.4 kill operation for non tencent users.
kill ##;
kill ##;
ERROR HY###: Unknown thread id: ##
kill ##;
kill ##;
select id, user, command from information_schema.processlist order by user;
id	user	command
#####	event_scheduler	#####
#####	root	#####
#####	tencentroot	#####
#####	tencentroot	#####
#1.5 check set privileges for non tencent users.
set global tencent_root_whitelist='localhost';
ERROR HY000: Unknown system variable 'tencent_root_whitelist'
set global tencent_root_embedded=OFF;
ERROR HY000: Unknown system variable 'tencent_root_embedded'
set global tencent_root_whitelist='localhost;127.0.0.1';
set global tencent_root_whitelist= 'localhost;127.0.0.1;::1';
SET GLOBAL max_connections = 151;

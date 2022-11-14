SHOW VARIABLES LIKE "print_proxy_info_for_processlist";
Variable_name	Value
print_proxy_info_for_processlist	OFF
SHOW PROCESSLIST;
Id	User	Host	db	Command	Time	State	Info
0	event_scheduler	localhost	NULL	Daemon	0	Waiting on empty queue	NULL
0	root	localhost	test	Query	0	init	SHOW PROCESSLIST
SET SESSION print_proxy_info_for_processlist = ON;
SHOW PROCESSLIST;
Id	User	Host	db	Command	Time	State	Info	Proxy_host
0	event_scheduler	localhost	NULL	Daemon	0	Waiting on empty queue	NULL	
0	root	localhost	test	Query	0	init	SHOW PROCESSLIST	
CREATE USER 'jj'@'%' IDENTIFIED BY 'pass';
GRANT ALL ON *.* TO 'jj'@'%';
SET GLOBAL DEBUG = "+d, fake_proxy_ip_port";
SHOW PROCESSLIST;
Id	User	Host	db	Command	Time	State	Info	Proxy_host
0	event_scheduler	localhost	NULL	Daemon	0	Waiting on empty queue	NULL	
0	root	localhost	test	Query	0	init	SHOW PROCESSLIST	
0	jj	0.0.0.0:0	test	Sleep	0		NULL	0.0.0.0:0
SET GLOBAL DEBUG = "-d, fake_proxy_ip_port";
DROP USER 'jj'@'%';

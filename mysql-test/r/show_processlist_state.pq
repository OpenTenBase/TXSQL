"Test SHOW PROCESSLIST, column INFO"
show processlist;
Id	User	Host	db	Command	Time	State	Info
ID	event_scheduler	HOST	NULL	Wait	TIME	WAITING	NULL
ID	root	HOST	test	Wait	TIME	init	show processlist

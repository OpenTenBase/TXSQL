# restart
show status like 'Innodb_row_lock_current_waits';
Variable_name	Value
Innodb_row_lock_current_waits	0
SET DEBUG = "+d,ib_simulate_row_lock_waits_underflow";
show status like 'Innodb_row_lock_current_waits';
Variable_name	Value
Innodb_row_lock_current_waits	0
SET DEBUG = "-d,ib_simulate_row_lock_waits_underflow";
show status like 'Innodb_row_lock_current_waits';
Variable_name	Value
Innodb_row_lock_current_waits	0
# restart

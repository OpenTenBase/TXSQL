#
# prepare
show variables like '%cdb_working_mode_enabled';
Variable_name	Value
# connection1
set global cdb_working_mode_enabled= on;
#
# run
# connection2
#
# cleanup
set global cdb_working_mode_enabled= 0;

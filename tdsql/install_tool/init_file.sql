#set sql_log_bin=0;
use performance_schema;
update setup_consumers set enabled='no'; 
update setup_instruments set enabled='no';

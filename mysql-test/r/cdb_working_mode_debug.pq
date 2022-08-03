CALL mtr.add_suppression('[txsql]:');
SET @global_saved_tmp =  @@global.cdb_working_mode_enabled;
SET DEBUG_SYNC='after_lock_sys_variables_acquire SIGNAL lock_sys_variables_acquired WAIT_FOR lock_thd_data_acquired';
SET GLOBAL cdb_working_mode_enabled=ON;;
SET DEBUG_SYNC='now WAIT_FOR lock_sys_variables_acquired';
SET DEBUG_SYNC='materialize_session_variable_array_THD_locked SIGNAL lock_thd_data_acquired';
SHOW VARIABLES LIKE '%cdb_working_mode_enabled%';;
# reaping execution status for SET GLOBAL cdb_working_mode_enabled=ON;
Variable_name	Value
cdb_working_mode_enabled	ON
SET DEBUG_SYNC='RESET';
# Restoring the original values.
SET @@global.cdb_working_mode_enabled = @global_saved_tmp;

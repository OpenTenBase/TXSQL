#
# Prepare
#
CREATE DEFINER=root@localhost PROCEDURE check_var()
BEGIN
-- Dump all global variables except those that may change.
-- timestamp changes if time passes. server_uuid changes if server restarts.
SELECT * FROM performance_schema.global_variables
WHERE variable_name  IN ('tencent_root_limit','tencent_root_whitelist')
ORDER BY VARIABLE_NAME;
END$$
#
# Run 
#
call check_var();
VARIABLE_NAME	VARIABLE_VALUE
#
# Cleanup 
#
drop procedure check_var;

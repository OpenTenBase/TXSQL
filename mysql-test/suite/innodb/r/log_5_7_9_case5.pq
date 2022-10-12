# Create an empty data directory...
# Unzip 5_7_9_clean.zip to the created directory...
# Kill the server
############################################################################################
# Case 5: Attempt to start on redo files created by MEB.
# 5A: InnoDB starts and performs upgrade on redo files created by MEB.
############################################################################################
# Case 5A: Start MySQL... (attempt 1)
# Verify...
Pattern "log file was created by mysqlbackup --apply-log at MEB4567890123456789012345678901\." not found
Pattern "Upgrading redo log: .*, LSN=" not found
Pattern "innodb-unknown-compound-v" not found
# Case 5A: Start MySQL... (attempt 2)
# Verify...
Pattern "log file was created by mysqlbackup --apply-log at MEB4567890123456789012345678901\." not found
Pattern "Upgrading redo log: .*, LSN=" not found
Pattern "innodb-unknown-compound-v" not found
############################################################################################
# Cleanup...
# restart

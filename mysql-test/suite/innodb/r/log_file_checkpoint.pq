include/stop_mysqld.inc [server 1]
# Initialize new data directory...
Pattern "The latest found checkpoint is at lsn = REDO_CHECKPOINT_LSN in redo log file" not found
Pattern "innodb-unknown-param" not found
# restart

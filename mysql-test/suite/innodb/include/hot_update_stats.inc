# This include file will create some connections
# to force hot update logic work.

#--disable_query_log
#--disable_result_log

--let LOCK_HOT_UPDATE_THRESHOLD = 50

#  Prepare an object that we can wait for in InnoDB
#  lock system: a row in an InnoDB table.

# Creating 10 + HOT_UPDATE_THRESHOLD threads named hotupdate0...$LOCK_HOT_UPDATE_THRESHOLD:
#   hotupdate0 will obtain X lock on hotupdate only row
#   hotupdate1...hotupdate$LOCK_HOT_UPDATE_THRESHOLD will wait for the lock

# hotupdate0 connects and obtains the X lock

--disable_result_log
DROP PROCEDURE IF EXISTS update_hold_test;
--enable_result_log
DELIMITER //;

CREATE PROCEDURE update_hold_test()
BEGIN
    START TRANSACTION;
    UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
    select sleep(0.3);
    COMMIT;
END//
DELIMITER ;//

--echo # establishing connection hotupdate0
--connect (hotupdate0,localhost, root,,)
BEGIN;
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;

# hotupdate1...hotupdate$LOCK_HOT_UPDATE_THRESHOLD connect and wait for an x lock

--let $hotupdate_connection_number = 1
while($hotupdate_connection_number <= $LOCK_HOT_UPDATE_THRESHOLD)
{
    --let $hotupdate_connection_name = hotupdate$hotupdate_connection_number
    --echo # establishing connection $hotupdate_connection_name
    --connect ($hotupdate_connection_name, localhost, root,,)
    --send call update_hold_test()
    --inc $hotupdate_connection_number
}

--connection hotupdate0
COMMIT;
sleep 1;
select count(1) from information_schema.INNODB_HOT_ROW_UPDATE_STATS;

while($hotupdate_connection_number <= 60)
{
    --let $hotupdate_connection_name = hotupdate$hotupdate_connection_number
    --echo # establishing connection $hotupdate_connection_name
    --connect ($hotupdate_connection_name, localhost, root,,)
    UPDATE hotupdates SET counter=counter+1 WHERE id = 1;

    --inc $hotupdate_connection_number
}

--connection hotupdate0


--let $hotupdate_connection_number = 1
while($hotupdate_connection_number <= $LOCK_HOT_UPDATE_THRESHOLD)
{
    --let $hotupdate_connection_name = hotupdate$hotupdate_connection_number
    --connection $hotupdate_connection_name
    --echo # cleaning up connection $hotupdate_connection_name
    --reap
    --sleep 0.1

    --disconnect $hotupdate_connection_name

    --inc $hotupdate_connection_number
}

while($hotupdate_connection_number <= 60)
{
    --let $hotupdate_connection_name = hotupdate$hotupdate_connection_number
    --connection $hotupdate_connection_name
    --echo # cleaning up connection $hotupdate_connection_name

    --disconnect $hotupdate_connection_name

    --inc $hotupdate_connection_number
}
--connection default
DROP PROCEDURE IF EXISTS update_hold_test;
--disconnect hotupdate0

#--enable_result_log
#--enable_query_log

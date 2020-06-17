# This include file will create some connections
# to force hot update logic work.

#--disable_query_log
#--disable_result_log

--let LOCK_HOT_UPDATE_THRESHOLD = 80

#  Prepare an object that we can wait for in InnoDB
#  lock system: a row in an InnoDB table.

# Creating 10 + HOT_UPDATE_THRESHOLD threads named hotupdate0...$LOCK_HOT_UPDATE_THRESHOLD:
#   hotupdate0 will obtain X lock on hotupdate only row
#   hotupdate1...hotupdate$LOCK_HOT_UPDATE_THRESHOLD will wait for the lock

DROP PROCEDURE IF EXISTS insert_test;
DROP PROCEDURE IF EXISTS update_hold_test;
DELIMITER //;

CREATE PROCEDURE update_hold_test()
BEGIN
    START TRANSACTION;
    UPDATE hotupdates SET counter=counter+1 WHERE id = 1000;
    select sleep(0.3);
    COMMIT;
END//

CREATE PROCEDURE insert_test()
BEGIN
    DECLARE i INT DEFAULT 0;
    WHILE i < 100 DO
        INSERT INTO hotupdates(id, f2, counter) VALUES (i, repeat('~', 240), 0);
        SET i = i + 1;
    END WHILE;
END//

DELIMITER ;//


# hotupdate0 connects and obtains the X lock

--echo # establishing connection hotupdate0
--connect (hotupdate0,localhost, root,,)
BEGIN;
UPDATE hotupdates SET counter=counter+1 WHERE id = 1000;

# hotupdate1...hotupdate$LOCK_HOT_UPDATE_THRESHOLD connect and wait for an x lock

--let $hotupdate_connection_number = 1
while($hotupdate_connection_number <= $LOCK_HOT_UPDATE_THRESHOLD)
{
    --let $hotupdate_connection_name = hotupdate$hotupdate_connection_number
    --echo # establishing connection $hotupdate_connection_name
    --connect ($hotupdate_connection_name, localhost, root,,)
    # issue transactions to hold the row lock longer so that 
    # the insert_test will have a chance to split the page while the hot row is still hot.
    --send call update_hold_test() 

    --inc $hotupdate_connection_number
    if ($hotupdate_connection_number == 40) 
    {
        --connection hotupdate0
        COMMIT;
    }

    if ($hotupdate_connection_number == 41) {
        --connection hotupdate0
        --send call insert_test()
    }
}

--connection hotupdate0
--reap
DROP PROCEDURE IF EXISTS insert_test;
DROP PROCEDURE IF EXISTS update_hold_test;
--connection default

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

while($hotupdate_connection_number <= $LOCK_HOT_UPDATE_THRESHOLD)
{
    --let $hotupdate_connection_name = hotupdate$hotupdate_connection_number
    --connection $hotupdate_connection_name
    --echo # cleaning up connection $hotupdate_connection_name

    --disconnect $hotupdate_connection_name

    --inc $hotupdate_connection_number
}

--disconnect hotupdate0

#--enable_result_log
#--enable_query_log

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

--echo # establishing connection hotupdate0
--connect (hotupdate0,localhost, root,,)
BEGIN;
UPDATE hotupdates SET counter=counter+1 WHERE id = 4;
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;
# hotupdate1...hotupdate$LOCK_HOT_UPDATE_THRESHOLD connect and wait for an x lock

--let $hotupdate_connection_number = 1
while($hotupdate_connection_number <= $LOCK_HOT_UPDATE_THRESHOLD)
{
    --let $hotupdate_connection_name = hotupdate$hotupdate_connection_number
    --echo # establishing connection $hotupdate_connection_name
    --connect ($hotupdate_connection_name, localhost, root,,)
    --eval SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL $hotupdate_connection_name'
    --send UPDATE hotupdates SET counter=counter+1 WHERE id = 5;

    --inc $hotupdate_connection_number
}

--connection hotupdate0
COMMIT;
--sleep 1

while($hotupdate_connection_number <= 60)
{
    --let $hotupdate_connection_name = hotupdate$hotupdate_connection_number
    --echo # establishing connection $hotupdate_connection_name
    --connect ($hotupdate_connection_name, localhost, root,,)
    UPDATE hotupdates SET counter=counter+1 WHERE id = 1;

    --inc $hotupdate_connection_number
}

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

while($hotupdate_connection_number <= 60)
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

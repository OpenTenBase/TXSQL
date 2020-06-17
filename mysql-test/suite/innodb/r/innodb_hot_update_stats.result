USE test;
SET @@global.innodb_hot_update_detect=true;
CREATE TABLE hotupdates (id int PRIMARY KEY, counter int) Engine=InnoDB;
INSERT INTO hotupdates VALUES (1,0);
########################################################
#                                                      #
# Scenario 1 test basic function of hot update stats   #
#                                                      #
########################################################
SET GLOBAL DEBUG='+d,hot_update_add_new_item';
DROP PROCEDURE IF EXISTS update_hold_test;
CREATE PROCEDURE update_hold_test()
BEGIN
START TRANSACTION;
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
select sleep(0.3);
COMMIT;
END//
# establishing connection hotupdate0
BEGIN;
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate1
call update_hold_test();
# establishing connection hotupdate2
call update_hold_test();
# establishing connection hotupdate3
call update_hold_test();
# establishing connection hotupdate4
call update_hold_test();
# establishing connection hotupdate5
call update_hold_test();
# establishing connection hotupdate6
call update_hold_test();
# establishing connection hotupdate7
call update_hold_test();
# establishing connection hotupdate8
call update_hold_test();
# establishing connection hotupdate9
call update_hold_test();
# establishing connection hotupdate10
call update_hold_test();
# establishing connection hotupdate11
call update_hold_test();
# establishing connection hotupdate12
call update_hold_test();
# establishing connection hotupdate13
call update_hold_test();
# establishing connection hotupdate14
call update_hold_test();
# establishing connection hotupdate15
call update_hold_test();
# establishing connection hotupdate16
call update_hold_test();
# establishing connection hotupdate17
call update_hold_test();
# establishing connection hotupdate18
call update_hold_test();
# establishing connection hotupdate19
call update_hold_test();
# establishing connection hotupdate20
call update_hold_test();
# establishing connection hotupdate21
call update_hold_test();
# establishing connection hotupdate22
call update_hold_test();
# establishing connection hotupdate23
call update_hold_test();
# establishing connection hotupdate24
call update_hold_test();
# establishing connection hotupdate25
call update_hold_test();
# establishing connection hotupdate26
call update_hold_test();
# establishing connection hotupdate27
call update_hold_test();
# establishing connection hotupdate28
call update_hold_test();
# establishing connection hotupdate29
call update_hold_test();
# establishing connection hotupdate30
call update_hold_test();
# establishing connection hotupdate31
call update_hold_test();
# establishing connection hotupdate32
call update_hold_test();
# establishing connection hotupdate33
call update_hold_test();
# establishing connection hotupdate34
call update_hold_test();
# establishing connection hotupdate35
call update_hold_test();
# establishing connection hotupdate36
call update_hold_test();
# establishing connection hotupdate37
call update_hold_test();
# establishing connection hotupdate38
call update_hold_test();
# establishing connection hotupdate39
call update_hold_test();
# establishing connection hotupdate40
call update_hold_test();
# establishing connection hotupdate41
call update_hold_test();
# establishing connection hotupdate42
call update_hold_test();
# establishing connection hotupdate43
call update_hold_test();
# establishing connection hotupdate44
call update_hold_test();
# establishing connection hotupdate45
call update_hold_test();
# establishing connection hotupdate46
call update_hold_test();
# establishing connection hotupdate47
call update_hold_test();
# establishing connection hotupdate48
call update_hold_test();
# establishing connection hotupdate49
call update_hold_test();
# establishing connection hotupdate50
call update_hold_test();
COMMIT;
select count(1) from information_schema.INNODB_HOT_ROW_UPDATE_STATS;
count(1)
1
# establishing connection hotupdate51
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate52
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate53
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate54
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate55
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate56
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate57
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate58
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate59
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# establishing connection hotupdate60
UPDATE hotupdates SET counter=counter+1 WHERE id = 1;
# cleaning up connection hotupdate1
sleep(0.3)
0
# cleaning up connection hotupdate2
sleep(0.3)
0
# cleaning up connection hotupdate3
sleep(0.3)
0
# cleaning up connection hotupdate4
sleep(0.3)
0
# cleaning up connection hotupdate5
sleep(0.3)
0
# cleaning up connection hotupdate6
sleep(0.3)
0
# cleaning up connection hotupdate7
sleep(0.3)
0
# cleaning up connection hotupdate8
sleep(0.3)
0
# cleaning up connection hotupdate9
sleep(0.3)
0
# cleaning up connection hotupdate10
sleep(0.3)
0
# cleaning up connection hotupdate11
sleep(0.3)
0
# cleaning up connection hotupdate12
sleep(0.3)
0
# cleaning up connection hotupdate13
sleep(0.3)
0
# cleaning up connection hotupdate14
sleep(0.3)
0
# cleaning up connection hotupdate15
sleep(0.3)
0
# cleaning up connection hotupdate16
sleep(0.3)
0
# cleaning up connection hotupdate17
sleep(0.3)
0
# cleaning up connection hotupdate18
sleep(0.3)
0
# cleaning up connection hotupdate19
sleep(0.3)
0
# cleaning up connection hotupdate20
sleep(0.3)
0
# cleaning up connection hotupdate21
sleep(0.3)
0
# cleaning up connection hotupdate22
sleep(0.3)
0
# cleaning up connection hotupdate23
sleep(0.3)
0
# cleaning up connection hotupdate24
sleep(0.3)
0
# cleaning up connection hotupdate25
sleep(0.3)
0
# cleaning up connection hotupdate26
sleep(0.3)
0
# cleaning up connection hotupdate27
sleep(0.3)
0
# cleaning up connection hotupdate28
sleep(0.3)
0
# cleaning up connection hotupdate29
sleep(0.3)
0
# cleaning up connection hotupdate30
sleep(0.3)
0
# cleaning up connection hotupdate31
sleep(0.3)
0
# cleaning up connection hotupdate32
sleep(0.3)
0
# cleaning up connection hotupdate33
sleep(0.3)
0
# cleaning up connection hotupdate34
sleep(0.3)
0
# cleaning up connection hotupdate35
sleep(0.3)
0
# cleaning up connection hotupdate36
sleep(0.3)
0
# cleaning up connection hotupdate37
sleep(0.3)
0
# cleaning up connection hotupdate38
sleep(0.3)
0
# cleaning up connection hotupdate39
sleep(0.3)
0
# cleaning up connection hotupdate40
sleep(0.3)
0
# cleaning up connection hotupdate41
sleep(0.3)
0
# cleaning up connection hotupdate42
sleep(0.3)
0
# cleaning up connection hotupdate43
sleep(0.3)
0
# cleaning up connection hotupdate44
sleep(0.3)
0
# cleaning up connection hotupdate45
sleep(0.3)
0
# cleaning up connection hotupdate46
sleep(0.3)
0
# cleaning up connection hotupdate47
sleep(0.3)
0
# cleaning up connection hotupdate48
sleep(0.3)
0
# cleaning up connection hotupdate49
sleep(0.3)
0
# cleaning up connection hotupdate50
sleep(0.3)
0
# cleaning up connection hotupdate51
# cleaning up connection hotupdate52
# cleaning up connection hotupdate53
# cleaning up connection hotupdate54
# cleaning up connection hotupdate55
# cleaning up connection hotupdate56
# cleaning up connection hotupdate57
# cleaning up connection hotupdate58
# cleaning up connection hotupdate59
# cleaning up connection hotupdate60
DROP PROCEDURE IF EXISTS update_hold_test;
SET GLOBAL DEBUG='-d,hot_update_add_new_item';
SET DEBUG_SYNC = 'RESET';
DROP TABLE hotupdates;
SET @@global.innodb_hot_update_detect=false;
#####################
#                   #
# End of            #
#                   #
#####################

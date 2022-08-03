USE test;
SET @@global.innodb_hot_update_detect=true;
SET @@global.innodb_hot_update_wait_timeout=1000000;
DROP TABLE IF EXISTS cats;
DROP TABLE IF EXISTS hotupdates;
CREATE TABLE hotupdates (id int PRIMARY KEY, f2 CHAR(255), counter int) Engine=InnoDB;
start transaction;
insert into hotupdates(id, f2, counter) values (1000, repeat('~', 50), 0);
commit;
########################################################
#                                                      #
# Scenario: Hot Update during Page Reorganization      #
#                                                      #
########################################################
SET GLOBAL DEBUG='+d,hot_update_add_new_item,hot_update_move_item';
DROP PROCEDURE IF EXISTS update_hold_test;
Warnings:
Note	1305	PROCEDURE test.update_hold_test does not exist
DROP PROCEDURE IF EXISTS insert_test;
Warnings:
Note	1305	PROCEDURE test.insert_test does not exist
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
WHILE i < 10 DO
INSERT INTO hotupdates(id, f2, counter) VALUES (i, repeat('~', 240), 0);
SET i = i + 1;
END WHILE;
END//
# establishing connection hotupdate0
BEGIN;
UPDATE hotupdates SET counter=counter+1 WHERE id = 1000;
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
COMMIT;
# establishing connection hotupdate40
call update_hold_test();
SET GLOBAL DEBUG='+d,do_page_reorganize';
call insert_test();
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
# establishing connection hotupdate51
call update_hold_test();
# establishing connection hotupdate52
call update_hold_test();
# establishing connection hotupdate53
call update_hold_test();
# establishing connection hotupdate54
call update_hold_test();
# establishing connection hotupdate55
call update_hold_test();
# establishing connection hotupdate56
call update_hold_test();
# establishing connection hotupdate57
call update_hold_test();
# establishing connection hotupdate58
call update_hold_test();
# establishing connection hotupdate59
call update_hold_test();
# establishing connection hotupdate60
call update_hold_test();
# establishing connection hotupdate61
call update_hold_test();
# establishing connection hotupdate62
call update_hold_test();
# establishing connection hotupdate63
call update_hold_test();
# establishing connection hotupdate64
call update_hold_test();
# establishing connection hotupdate65
call update_hold_test();
# establishing connection hotupdate66
call update_hold_test();
# establishing connection hotupdate67
call update_hold_test();
# establishing connection hotupdate68
call update_hold_test();
# establishing connection hotupdate69
call update_hold_test();
# establishing connection hotupdate70
call update_hold_test();
# establishing connection hotupdate71
call update_hold_test();
# establishing connection hotupdate72
call update_hold_test();
# establishing connection hotupdate73
call update_hold_test();
# establishing connection hotupdate74
call update_hold_test();
# establishing connection hotupdate75
call update_hold_test();
# establishing connection hotupdate76
call update_hold_test();
# establishing connection hotupdate77
call update_hold_test();
# establishing connection hotupdate78
call update_hold_test();
# establishing connection hotupdate79
call update_hold_test();
# establishing connection hotupdate80
call update_hold_test();
SET GLOBAL DEBUG='-d,do_page_reorganize';
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
sleep(0.3)
0
# cleaning up connection hotupdate52
sleep(0.3)
0
# cleaning up connection hotupdate53
sleep(0.3)
0
# cleaning up connection hotupdate54
sleep(0.3)
0
# cleaning up connection hotupdate55
sleep(0.3)
0
# cleaning up connection hotupdate56
sleep(0.3)
0
# cleaning up connection hotupdate57
sleep(0.3)
0
# cleaning up connection hotupdate58
sleep(0.3)
0
# cleaning up connection hotupdate59
sleep(0.3)
0
# cleaning up connection hotupdate60
sleep(0.3)
0
# cleaning up connection hotupdate61
sleep(0.3)
0
# cleaning up connection hotupdate62
sleep(0.3)
0
# cleaning up connection hotupdate63
sleep(0.3)
0
# cleaning up connection hotupdate64
sleep(0.3)
0
# cleaning up connection hotupdate65
sleep(0.3)
0
# cleaning up connection hotupdate66
sleep(0.3)
0
# cleaning up connection hotupdate67
sleep(0.3)
0
# cleaning up connection hotupdate68
sleep(0.3)
0
# cleaning up connection hotupdate69
sleep(0.3)
0
# cleaning up connection hotupdate70
sleep(0.3)
0
# cleaning up connection hotupdate71
sleep(0.3)
0
# cleaning up connection hotupdate72
sleep(0.3)
0
# cleaning up connection hotupdate73
sleep(0.3)
0
# cleaning up connection hotupdate74
sleep(0.3)
0
# cleaning up connection hotupdate75
sleep(0.3)
0
# cleaning up connection hotupdate76
sleep(0.3)
0
# cleaning up connection hotupdate77
sleep(0.3)
0
# cleaning up connection hotupdate78
sleep(0.3)
0
# cleaning up connection hotupdate79
sleep(0.3)
0
# cleaning up connection hotupdate80
sleep(0.3)
0
DROP PROCEDURE IF EXISTS update_hold_test;
DROP PROCEDURE IF EXISTS insert_test;
DROP PROCEDURE IF EXISTS update_hold_test;
Warnings:
Note	1305	PROCEDURE test.update_hold_test does not exist
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
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate1';
call update_hold_test();;
# establishing connection hotupdate2
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate2';
call update_hold_test();;
# establishing connection hotupdate3
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate3';
call update_hold_test();;
# establishing connection hotupdate4
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate4';
call update_hold_test();;
# establishing connection hotupdate5
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate5';
call update_hold_test();;
# establishing connection hotupdate6
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate6';
call update_hold_test();;
# establishing connection hotupdate7
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate7';
call update_hold_test();;
# establishing connection hotupdate8
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate8';
call update_hold_test();;
# establishing connection hotupdate9
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate9';
call update_hold_test();;
# establishing connection hotupdate10
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate10';
call update_hold_test();;
# establishing connection hotupdate11
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate11';
call update_hold_test();;
# establishing connection hotupdate12
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate12';
call update_hold_test();;
# establishing connection hotupdate13
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate13';
call update_hold_test();;
# establishing connection hotupdate14
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate14';
call update_hold_test();;
# establishing connection hotupdate15
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate15';
call update_hold_test();;
# establishing connection hotupdate16
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate16';
call update_hold_test();;
# establishing connection hotupdate17
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate17';
call update_hold_test();;
# establishing connection hotupdate18
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate18';
call update_hold_test();;
# establishing connection hotupdate19
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate19';
call update_hold_test();;
# establishing connection hotupdate20
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate20';
call update_hold_test();;
# establishing connection hotupdate21
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate21';
call update_hold_test();;
# establishing connection hotupdate22
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate22';
call update_hold_test();;
# establishing connection hotupdate23
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate23';
call update_hold_test();;
# establishing connection hotupdate24
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate24';
call update_hold_test();;
# establishing connection hotupdate25
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate25';
call update_hold_test();;
# establishing connection hotupdate26
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate26';
call update_hold_test();;
# establishing connection hotupdate27
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate27';
call update_hold_test();;
# establishing connection hotupdate28
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate28';
call update_hold_test();;
# establishing connection hotupdate29
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate29';
call update_hold_test();;
# establishing connection hotupdate30
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate30';
call update_hold_test();;
# establishing connection hotupdate31
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate31';
call update_hold_test();;
# establishing connection hotupdate32
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate32';
call update_hold_test();;
# establishing connection hotupdate33
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate33';
call update_hold_test();;
# establishing connection hotupdate34
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate34';
call update_hold_test();;
# establishing connection hotupdate35
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate35';
call update_hold_test();;
# establishing connection hotupdate36
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate36';
call update_hold_test();;
# establishing connection hotupdate37
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate37';
call update_hold_test();;
# establishing connection hotupdate38
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate38';
call update_hold_test();;
# establishing connection hotupdate39
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate39';
call update_hold_test();;
COMMIT;
# establishing connection hotupdate40
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate40';
call update_hold_test();;
# establishing connection hotupdate41
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate41';
call update_hold_test();;
# establishing connection hotupdate42
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate42';
call update_hold_test();;
# establishing connection hotupdate43
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate43';
call update_hold_test();;
# establishing connection hotupdate44
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate44';
call update_hold_test();;
# establishing connection hotupdate45
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate45';
call update_hold_test();;
# establishing connection hotupdate46
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate46';
call update_hold_test();;
# establishing connection hotupdate47
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate47';
call update_hold_test();;
# establishing connection hotupdate48
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate48';
call update_hold_test();;
# establishing connection hotupdate49
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate49';
call update_hold_test();;
# establishing connection hotupdate50
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate50';
call update_hold_test();;
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
DROP PROCEDURE IF EXISTS update_hold_test;
SET GLOBAL DEBUG='-d,hot_update_add_new_item';
SET GLOBAL DEBUG='-d,hot_update_move_item';
SET DEBUG_SYNC = 'RESET';
DROP TABLE hotupdates;
SET @@global.innodb_hot_update_detect=false;
#####################
#                   #
# End of            #
#                   #
#####################

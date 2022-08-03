USE test;
SET @@global.innodb_hot_update_detect=true;
SET @@global.innodb_hot_update_wait_timeout=1000000;
DROP TABLE IF EXISTS cats;
DROP TABLE IF EXISTS hotupdates;
CREATE TABLE hotupdates (id int PRIMARY KEY, f2 CHAR(255), counter int) Engine=InnoDB;
start transaction;
insert into hotupdates(id, f2, counter) values (4, repeat('~', 50), 0);
insert into hotupdates(id, f2, counter) values (5, repeat('~', 50), 0);
commit;
########################################################
#                                                      #
# Scenario: Hot Update on Multiple Updates             #
#                                                      #
########################################################
SET GLOBAL DEBUG='+d,hot_update_add_new_item,hot_update_move_item';
# establishing connection hotupdate0
BEGIN;
UPDATE hotupdates SET counter=counter+1 WHERE id = 4;
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;
# establishing connection hotupdate1
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate1';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate2
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate2';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate3
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate3';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate4
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate4';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate5
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate5';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate6
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate6';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate7
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate7';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate8
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate8';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate9
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate9';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate10
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate10';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate11
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate11';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate12
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate12';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate13
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate13';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate14
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate14';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate15
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate15';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate16
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate16';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate17
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate17';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate18
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate18';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate19
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate19';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate20
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate20';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate21
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate21';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate22
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate22';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate23
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate23';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate24
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate24';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate25
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate25';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate26
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate26';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate27
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate27';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate28
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate28';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate29
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate29';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate30
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate30';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate31
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate31';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate32
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate32';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate33
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate33';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate34
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate34';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate35
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate35';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate36
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate36';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate37
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate37';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate38
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate38';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate39
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate39';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate40
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate40';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate41
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate41';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate42
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate42';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate43
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate43';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate44
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate44';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate45
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate45';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate46
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate46';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate47
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate47';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate48
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate48';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate49
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate49';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
# establishing connection hotupdate50
SET DEBUG_SYNC = 'lock_wait_will_wait SIGNAL hotupdate50';
UPDATE hotupdates SET counter=counter+1 WHERE id = 5;;
COMMIT;
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
# cleaning up connection hotupdate2
# cleaning up connection hotupdate3
# cleaning up connection hotupdate4
# cleaning up connection hotupdate5
# cleaning up connection hotupdate6
# cleaning up connection hotupdate7
# cleaning up connection hotupdate8
# cleaning up connection hotupdate9
# cleaning up connection hotupdate10
# cleaning up connection hotupdate11
# cleaning up connection hotupdate12
# cleaning up connection hotupdate13
# cleaning up connection hotupdate14
# cleaning up connection hotupdate15
# cleaning up connection hotupdate16
# cleaning up connection hotupdate17
# cleaning up connection hotupdate18
# cleaning up connection hotupdate19
# cleaning up connection hotupdate20
# cleaning up connection hotupdate21
# cleaning up connection hotupdate22
# cleaning up connection hotupdate23
# cleaning up connection hotupdate24
# cleaning up connection hotupdate25
# cleaning up connection hotupdate26
# cleaning up connection hotupdate27
# cleaning up connection hotupdate28
# cleaning up connection hotupdate29
# cleaning up connection hotupdate30
# cleaning up connection hotupdate31
# cleaning up connection hotupdate32
# cleaning up connection hotupdate33
# cleaning up connection hotupdate34
# cleaning up connection hotupdate35
# cleaning up connection hotupdate36
# cleaning up connection hotupdate37
# cleaning up connection hotupdate38
# cleaning up connection hotupdate39
# cleaning up connection hotupdate40
# cleaning up connection hotupdate41
# cleaning up connection hotupdate42
# cleaning up connection hotupdate43
# cleaning up connection hotupdate44
# cleaning up connection hotupdate45
# cleaning up connection hotupdate46
# cleaning up connection hotupdate47
# cleaning up connection hotupdate48
# cleaning up connection hotupdate49
# cleaning up connection hotupdate50
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

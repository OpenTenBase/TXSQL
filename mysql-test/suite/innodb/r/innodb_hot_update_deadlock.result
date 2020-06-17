USE test;
SET @@global.innodb_hot_update_detect=true;
SET @@global.innodb_hot_update_wait_timeout=1000000;
DROP TABLE IF EXISTS cats;
DROP TABLE IF EXISTS hotupdates;
DROP PROCEDURE IF EXISTS deadlock_test;
CREATE TABLE hotupdates (id int PRIMARY KEY, balance int) Engine=InnoDB;
start transaction;
insert into hotupdates(id, balance) values (0, 50);
insert into hotupdates(id, balance) values (1, 50);
commit;
CREATE PROCEDURE deadlock_test()
BEGIN
DECLARE row_id INT DEFAULT 0;
DECLARE op_row_id INT DEFAULT 0;
SELECT FLOOR(RAND()*(1-0+1))+0 into row_id;
SET op_row_id = 1 - row_id;
START TRANSACTION;
UPDATE hotupdates set balance = balance - 1 where id = row_id;
UPDATE hotupdates set balance = balance + 1 where id = op_row_id;
COMMIT;
END//
########################################################
#                                                      #
# Scenario: Hot Update During Deadlocks                #
#                                                      #
########################################################
100
DROP TABLE hotupdates;
affected rows: 0
SET @@global.innodb_hot_update_detect=false;
affected rows: 0
DROP PROCEDURE IF EXISTS deadlock_test;
affected rows: 0
#####################
#                   #
# End of            #
#                   #
#####################

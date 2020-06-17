USE test;
SET @@global.innodb_hot_update_detect=true;
SET @@global.innodb_hot_update_wait_timeout=1000000;
DROP TABLE IF EXISTS cats;
DROP TABLE IF EXISTS hotupdates;
DROP PROCEDURE IF EXISTS mixed_workload_test;
CREATE TABLE hotupdates (id int PRIMARY KEY, balance int) Engine=InnoDB;
CREATE PROCEDURE mixed_workload_test()
BEGIN
DECLARE row_id INT DEFAULT 0;
DECLARE op_row_id INT DEFAULT 0;
DECLARE opid INT DEFAULT 0;
SELECT FLOOR(RAND() * 5) + 0 into opid;
IF opid = 0 or opid = 1 THEN
SELECT FLOOR(RAND() * 2) + 0 into row_id;
SET op_row_id = 1 - row_id;
START TRANSACTION;
UPDATE hotupdates set balance = balance - 1 where id = row_id;
UPDATE hotupdates set balance = balance + 1 where id = op_row_id;
COMMIT;
ELSE
IF opid = 2 THEN
SELECT FLOOR(RAND() * 31) + 0 into row_id;
IF row_id <= 1 THEN
START TRANSACTION;
UPDATE hotupdates set balance = balance + 0 where id = row_id;
COMMIT;
ELSE
START TRANSACTION;
UPDATE hotupdates set balance = balance + 1 where id = row_id;
COMMIT;
END IF;
ELSE
IF opid = 3 THEN
SELECT FLOOR(RAND() * 101) + 2 into row_id;
select * from hotupdates where id = row_id;
ELSE
SELECT FLOOR(RAND() * 101) + 2 into row_id;
select sum(balance) from hotupdates where id <= row_id;
END IF;
END IF;
END IF;
END//
##########################################################################
#                                                                        #
# Scenario: Hot updates mixed with normal updates, and queries           #
#                                                                        #
##########################################################################
100
DROP TABLE hotupdates;
affected rows: 0
SET @@global.innodb_hot_update_detect=false;
affected rows: 0
DROP PROCEDURE IF EXISTS mixed_workload_test;
affected rows: 0
#####################
#                   #
# End of            #
#                   #
#####################

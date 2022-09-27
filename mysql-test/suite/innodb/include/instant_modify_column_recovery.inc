
--echo #
--echo # Scenario 1:
--echo # Create normal table and partitioned table, DML on them, restart
--echo #

--eval CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=$row_format

INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);


ALTER TABLE t1 ADD COLUMN c1 INT DEFAULT 10, ALGORITHM=inplace;

INSERT INTO t1 VALUES(0, 6, 20), (0, 7, 20);
ALTER TABLE t1 MODIFY COLUMN c1 BIGINT, ALGORITHM=INSTANT;
INSERT INTO t1 VALUES(0, 8, 30), (0, 9, 30);

--source include/restart_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;

UPDATE t1 SET b = 10;

--source include/restart_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;

UPDATE t1 SET c1 = 15;

--source include/restart_mysqld.inc

CHECK TABLE t1;

DELETE FROM t1 WHERE a > 0;

--source include/restart_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;

INSERT INTO t1(a, b) VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);

ALTER TABLE t1 ADD COLUMN c2 VARCHAR(100) DEFAULT 'Hello World!', ALGORITHM=inplace;

UPDATE t1 SET c2 = 'Hello MySQL!';

ALTER TABLE t1 MODIFY COLUMN c4 VARCHAR(150), ALGORITHM=INSTANT;

UPDATE t1 SET c2 = 'Hello MySQL World!';

--source include/restart_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;

DROP TABLE t1;



--echo #
--echo # Scenario 2:
--echo # Create normal table and partitioned table, DML on them, crash
--echo #

--eval CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=$row_format

INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);

ALTER TABLE t1 ADD COLUMN c1 INT DEFAULT 10, ALGORITHM=inplace;

ALTER TABLE t1 MODIFY COLUMN c1 BIGINT, ALGORITHM=INSTANT;

START TRANSACTION;

INSERT INTO t1 VALUES(0, 6, 20), (0, 7, 20);

# Server will crash on next commit
--source include/expect_crash.inc
SET SESSION DEBUG="+d,crash_commit_after";
--echo CR_SERVER_LOST
--error 2013

COMMIT;

--source include/start_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;


INSERT INTO t1 VALUES(0, 6, 20), (0, 7, 20);

START TRANSACTION;

UPDATE t1 SET b = 10;

# Server will crash on next commit
--source include/expect_crash.inc
SET SESSION DEBUG="+d,crash_commit_after";
--echo CR_SERVER_LOST
--error 2013

COMMIT;

--source include/start_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;


UPDATE t1 SET b = 10;

START TRANSACTION;

UPDATE t1 SET c1 = 11;

# Server will crash on next commit
--source include/expect_crash.inc
SET SESSION DEBUG="+d,crash_commit_after";
--echo CR_SERVER_LOST
--error 2013

COMMIT;

--source include/start_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;


UPDATE t1 SET c1 = 11;

START TRANSACTION;

DELETE FROM t1 WHERE a > 0;

# Server will crash on next commit
--source include/expect_crash.inc
SET SESSION DEBUG="+d,crash_commit_after";
--echo CR_SERVER_LOST
--error 2013

COMMIT;

--source include/start_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;


DELETE FROM t1 WHERE a > 0;

SELECT * FROM t1;


CHECK TABLE t1;

DROP TABLE t1;



--eval CREATE TABLE t1 (a INT NOT NULL AUTO_INCREMENT PRIMARY KEY, b INT) ROW_FORMAT=$row_format

INSERT INTO t1 VALUES(0, 1), (0, 2), (0, 3), (0, 4), (0, 5);


ALTER TABLE t1 ADD COLUMN c1 INT DEFAULT 10, ALGORITHM=inplace;

ALTER TABLE t1 MODIFY COLUMN c1 BIGINT, ALGORITHM=INSTANT;

START TRANSACTION;

INSERT INTO t1 VALUES(6, 6, 20), (7, 7, 20);

# Server will crash on next commit
--source include/expect_crash.inc
SET SESSION DEBUG="+d,crash_commit_before";
--echo CR_SERVER_LOST
--error 2013

COMMIT;

--source include/start_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;

INSERT INTO t1 VALUES(6, 6, 20), (7, 7, 20);

START TRANSACTION;

UPDATE t1 SET b = 10;

# Server will crash on next commit
--source include/expect_crash.inc
SET SESSION DEBUG="+d,crash_commit_before";
--echo CR_SERVER_LOST
--error 2013

COMMIT;

--source include/start_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;


UPDATE t1 SET b = 10;

START TRANSACTION;

UPDATE t1 SET c1 = 11;

# Server will crash on next commit
--source include/expect_crash.inc
SET SESSION DEBUG="+d,crash_commit_before";
--echo CR_SERVER_LOST
--error 2013

COMMIT;


--source include/start_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;


UPDATE t1 SET c1 = 11;

START TRANSACTION;

DELETE FROM t1 WHERE a > 0;

# Server will crash on next commit
--source include/expect_crash.inc
SET SESSION DEBUG="+d,crash_commit_before";
--echo CR_SERVER_LOST
--error 2013

COMMIT;

--source include/start_mysqld.inc

CHECK TABLE t1;

SELECT * FROM t1;


DELETE FROM t1 WHERE a > 0;

SELECT * FROM t1;


CHECK TABLE t1;

DROP TABLE t1;

# With a low wait_timeout and high cdb_kill_idle_trans_timeout the lower value takes precedence
SET GLOBAL cdb_kill_idle_trans_timeout= 100000;
SET SESSION wait_timeout= 2;
BEGIN;
show tables;
ERROR HY000: MySQL server has gone away
# Current connection idle transaction killed, reconnecting
SET GLOBAL cdb_kill_idle_trans_timeout= 1;
#
# Tests for cdb_kill_idle_trans_timeout feature that do not depend on transactional SEs
#
CREATE TABLE t1 (a INT) ENGINE=MyISAM;
INSERT INTO t1 VALUES (1),(2);
SELECT * FROM t1;
a
1
2
BEGIN;
INSERT INTO t1 VALUES (3);
SELECT * FROM t1;
a
1
2
3
SELECT * FROM t1;
ERROR HY000: MySQL server has gone away
# Current connection idle transaction killed, reconnecting
DROP TABLE t1;
#
# Test cdb_kill_idle_trans_timeout_timeout feature with InnoDB
#
CREATE TABLE t1 (a INT) ENGINE=InnoDB;
INSERT INTO t1 VALUES (1),(2);
SELECT * FROM t1;
a
1
2
BEGIN;
INSERT INTO t1 VALUES (3);
SELECT * FROM t1;
a
1
2
3
SELECT * FROM t1;
ERROR HY000: MySQL server has gone away
# Current connection idle transaction killed, reconnecting
#
# Test that row locks are released on idle transaction kill
#
# Take row locks in connection conn1
BEGIN;
SELECT * FROM t1 FOR UPDATE;
a
1
2
# Take row locks in connection default
UPDATE t1 SET a=4;
SELECT * FROM t1;
a
4
4
# Show that connection conn1 has been killed
SELECT * FROM t1;
ERROR HY000: MySQL server has gone away
# connection default
# Cleanup
DROP TABLE t1;
SET GLOBAL cdb_kill_idle_trans_timeout= saved_cdb_kill_idle_trans_timeout;

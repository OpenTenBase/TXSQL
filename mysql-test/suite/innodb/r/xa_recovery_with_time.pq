#
# Feature#Issue 115
# https://git.code.oa.com/TXSQL/MySQL-8.0/issues/115
#
CREATE TABLE t1 (a INT) ENGINE=InnoDB;
INSERT INTO t1 VALUES (1);
XA START 'x';
UPDATE t1 set a=2;
XA END 'x';
XA PREPARE 'x';
call mtr.add_suppression("Found 1 prepared XA transactions");
xa recover;
formatID	gtrid_length	bqual_length	data
1	1	0	x
xa recover with time;
formatID	gtrid_length	bqual_length	data	prepare_time
1	1	0	x	2020-04-28 12:34:56
# Kill and restart
SELECT * FROM t1 LOCK IN SHARE MODE;
# restart
xa recover;
formatID	gtrid_length	bqual_length	data
1	1	0	x
xa recover with time;
formatID	gtrid_length	bqual_length	data	prepare_time
1	1	0	x	1970-00-00 00:00:00
SET TRANSACTION ISOLATION LEVEL READ UNCOMMITTED;
SELECT * FROM t1;
a
2
XA ROLLBACK 'x';
SELECT * FROM t1;
a
1
DROP TABLE t1;

# --include save_and_set_px.inc
CREATE TABLE t1 ( a INTEGER, b CHAR(1) );
INSERT INTO t1 VALUES ( 1, 'a' ), ( 2, 'b' ), ( 3, 'c' );
# Query rewrite plugin was installed.
INSERT INTO query_rewrite.rewrite_rules ( pattern, replacement )
VALUES ( 'SELECT a FROM test.t1 WHERE a = ?',
'SELECT b FROM test.t1 WHERE a = ?' ),
( 'SELECT 1, ?', 'SELECT "rewritten"' );
CALL query_rewrite.flush_rewrite_rules();
PREPARE stmt2 FROM 'SELECT a FROM test.t1 WHERE a = ?';
Warnings:
Note	1105	Query 'SELECT a FROM test.t1 WHERE a = ?' rewritten to 'SELECT b FROM test.t1 WHERE a = ?' by a query rewrite plugin
SET @var = 1;
EXECUTE stmt2 USING @var;
b
a
DROP TABLE t1;
Warnings:
Warning	1620	Plugin is busy and will be uninstalled on shutdown
# Query rewrite plugin was queued for uninstalling.
# --include set_px_to_saved.inc

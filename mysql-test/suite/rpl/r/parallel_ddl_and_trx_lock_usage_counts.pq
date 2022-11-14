# restart:--tencent-root-cnf=tmy.conf
SET @old_global_isolation= @@global.transaction_isolation;
SET @old_session_isolation= @@session.transaction_isolation;
SET GLOBAL TRANSACTION ISOLATION LEVEL READ COMMITTED;
SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED;
SET @@global.transaction_isolation= @old_global_isolation;
SET @@session.transaction_isolation= @old_session_isolation;
CREATE TABLE t1 (c1 char(127) PRIMARY KEY, c2 int, c3 char(127)) ENGINE=INNODB;
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
SET SESSION innodb_txsql_parallel_ddl=ON;
ALTER TABLE t1 ADD INDEX k_c2_on(c2);
ALTER TABLE t1 DROP INDEX k_c2_on;
ALTER TABLE t1 ADD INDEX k_c2_on(c2);
ALTER TABLE t1 DROP INDEX k_c2_on;
include/assert.inc [parallel_ddl_counts = 2]
DROP TABLE t1;

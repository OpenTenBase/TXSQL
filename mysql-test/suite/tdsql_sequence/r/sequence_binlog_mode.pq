create database db_a;
use db_a;
create table t0 (a int);
CREATE TDSQL_SEQUENCE sequence_000 TDSQL_MAXVALUE 1000 TDSQL_minvalue 100;
set session autocommit = 1;
show variables like 'binlog_format';
Variable_name	Value
binlog_format	ROW
INSERT INTO t0 values(sequence_000.nextval);
SET SESSION binlog_format = "MIXED";
INSERT INTO t0 values(sequence_000.nextval);
ERROR HY000: Refenrence to sequence using mixed/statement mode is not safe
SELECT sequence_000.nextval;
sequence_000.nextval
101
SET SESSION binlog_format = "STATEMENT";
INSERT INTO t0 values(sequence_000.nextval);
ERROR HY000: Cannot execute statement: impossible to write to binary log since BINLOG_FORMAT = STATEMENT and at least one table uses a storage engine limited to row-based logging. InnoDB is limited to row-logging when transaction isolation level is READ COMMITTED or READ UNCOMMITTED.
SET SESSION transaction_isolation = "REPEATABLE-READ";
INSERT INTO t0 values(sequence_000.nextval);
ERROR HY000: Refenrence to sequence using mixed/statement mode is not safe
SELECT sequence_000.nextval;
sequence_000.nextval
102
SET SESSION binlog_format = "ROW";
INSERT INTO t0 values(sequence_000.nextval);
SELECT * FROM t0;
a
100
103
DROP DATABASE db_a;

CREATE TABLE t(
id INT NOT NULL PRIMARY KEY
) ENGINE=InnoDB;
INSERT INTO t (id) VALUES (1), (2);
SELECT @@global.innodb_use_trx_lock;
@@global.innodb_use_trx_lock
0
SELECT @@session.innodb_use_trx_lock;
ERROR HY000: Variable 'innodb_use_trx_lock' is a GLOBAL variable
SHOW GLOBAL VARIABLES LIKE  'innodb_use_trx_lock';
Variable_name	Value
innodb_use_trx_lock	OFF
SELECT * FROM performance_schema.global_variables
WHERE variable_name='innodb_use_trx_lock';
VARIABLE_NAME	VARIABLE_VALUE
innodb_use_trx_lock	OFF
SELECT @@global.transaction_isolation;
@@global.transaction_isolation
REPEATABLE-READ
SELECT @@session.transaction_isolation;
@@session.transaction_isolation
REPEATABLE-READ
SET @old_global_isolation= @@global.transaction_isolation;
SET @old_innodb_use_trx_lock= @@global.innodb_use_trx_lock;
#Case 1.1:
call mtr.add_suppression("Option innodb_use_trx_lock=ON is ignored because global.transaction_isolation is not READ COMMITTED");
SET @@global.innodb_use_trx_lock = ON;
ERROR 42000: Variable 'innodb_use_trx_lock' can't be set to the value of 'ON'
SELECT @@global.innodb_use_trx_lock;
@@global.innodb_use_trx_lock
0
#Case 1.2:
SET @@global.transaction_isolation = "READ-COMMITTED";
SET @@global.innodb_use_trx_lock = ON;
SELECT @@global.innodb_use_trx_lock;
@@global.innodb_use_trx_lock
1
#Case 2.1:
SET @@global.transaction_isolation = "REPEATABLE-READ";
ERROR 25001: When using trx lock, REPEATABLE READ or SERIALIZABLE is not allowed.
SET @@global.transaction_isolation = "SERIALIZABLE";
ERROR 25001: When using trx lock, REPEATABLE READ or SERIALIZABLE is not allowed.
SET @@global.transaction_isolation = "READ-UNCOMMITTED";
SET @@global.transaction_isolation = "READ-COMMITTED";
#Case 2.2:
SET @@session.transaction_isolation = "REPEATABLE-READ";
ERROR 25001: When using trx lock, REPEATABLE READ or SERIALIZABLE is not allowed.
SET @@session.transaction_isolation = "SERIALIZABLE";
ERROR 25001: When using trx lock, REPEATABLE READ or SERIALIZABLE is not allowed.
SET @@session.transaction_isolation = "READ-UNCOMMITTED";
SET @@session.transaction_isolation = "READ-COMMITTED";
#Case 2.3:
SET @@global.innodb_use_trx_lock = OFF;
SET @@session.transaction_isolation = "REPEATABLE-READ";
SET @@global.innodb_use_trx_lock = ON;
BEGIN;
select * from t where id = 1 for update;
id
1
select * from t where id = 1 for share;;
COMMIT;
id
1
SET @@session.transaction_isolation = "READ-COMMITTED";
#Case 3.1:
SET @@global.innodb_use_trx_lock = OFF;
SET @@global.transaction_isolation = "REPEATABLE-READ";
call mtr.add_suppression("Option innodb_use_trx_lock=ON is ignored because global.transaction_isolation is not READ COMMITTED");
SET @@global.innodb_use_trx_lock = ON;
ERROR 42000: Variable 'innodb_use_trx_lock' can't be set to the value of 'ON'
SELECT @@global.innodb_use_trx_lock;
@@global.innodb_use_trx_lock
0
BEGIN;
select * from t where id = 1 for update;
id
1
SET @@global.transaction_isolation = "READ-COMMITTED";
call mtr.add_suppression("Cannot turn on innodb_use_trx_lock is ignored  since there are still un-finished transactions using lock sys.");
SET @@global.innodb_use_trx_lock = ON;
ERROR 42000: Variable 'innodb_use_trx_lock' can't be set to the value of 'ON'
SELECT @@global.innodb_use_trx_lock;
@@global.innodb_use_trx_lock
0
COMMIT;
SET @@global.innodb_use_trx_lock = ON;
SELECT @@global.innodb_use_trx_lock;
@@global.innodb_use_trx_lock
1
BEGIN;
select * from t where id = 1 for update;
id
1
call mtr.add_suppression("Cannot turn off innodb_use_trx_lock is ignored  since there are still un-finished transactions using trx lock.");
SET @@global.innodb_use_trx_lock = OFF;
ERROR 42000: Variable 'innodb_use_trx_lock' can't be set to the value of 'OFF'
COMMIT;
SET @@global.innodb_use_trx_lock = OFF;
SELECT @@global.innodb_use_trx_lock;
@@global.innodb_use_trx_lock
0
DROP TABLE t;
SET @@global.innodb_use_trx_lock= @old_innodb_use_trx_lock;
SET @@global.transaction_isolation= @old_global_isolation;

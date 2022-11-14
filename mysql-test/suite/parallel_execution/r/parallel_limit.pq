show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,READ_MASK,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ``@`` TO `root`@`localhost` WITH GRANT OPTION
select user();
user()
root@localhost
create database test1;
create user 'user1'@'%';
grant all on *.* to 'user1'@'%';
# --include save_and_set_px.inc
DROP TABLE IF EXISTS t1;
Warnings:
Note	1051	Unknown table 'test.t1'
CREATE TABLE t1(
id INT PRIMARY KEY AUTO_INCREMENT,
col1 VARCHAR(100),
col2 INT,
col3 INT,
KEY idx_col2(col2)
)ENGINE = InnoDB;
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
query without limit
explain format=tree select 1 from t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel index scan on t1 using idx_col2  (cost=1.25 rows=10)

select 1 from t1;
1
1
1
1
1
1
1
1
1
1
1
limit after table scan
explain format=tree select * from t1 limit 10;
EXPLAIN
-> Limit: 10 row(s)  (cost=1.25 rows=10)
    -> Table scan on t1  (cost=1.25 rows=10)

select * from t1 limit 10;
id	col1	col2	col3
1	testtesttesttest	10	10
10	testtesttesttest	1	1
2	testtesttesttest	9	9
3	testtesttesttest	8	8
4	testtesttesttest	7	7
5	testtesttesttest	6	6
6	testtesttesttest	5	5
7	testtesttesttest	4	4
8	testtesttesttest	3	3
9	testtesttesttest	2	2
limit after index scan
explain format=tree select col2 from t1 order by col2 limit 5;
EXPLAIN
-> Limit: 5 row(s)  (cost=0.38 rows=5)
    -> Index scan on t1 using idx_col2  (cost=0.38 rows=5)

select * from t1 order by col2 limit 5;
id	col1	col2	col3
10	testtesttesttest	1	1
9	testtesttesttest	2	2
8	testtesttesttest	3	3
7	testtesttesttest	4	4
6	testtesttesttest	5	5
explain format=tree select col2 from t1 order by col2 desc limit 5;
EXPLAIN
-> Limit: 5 row(s)  (cost=0.38 rows=5)
    -> Index scan on t1 using idx_col2 (reverse)  (cost=0.38 rows=5)

select * from t1 order by col2 desc limit 5;
id	col1	col2	col3
1	testtesttesttest	10	10
2	testtesttesttest	9	9
3	testtesttesttest	8	8
4	testtesttesttest	7	7
5	testtesttesttest	6	6
limit after filesort
explain format=tree select sum(id) from t1 group by col3 order by col3 limit 5;
EXPLAIN
-> Limit: 5 row(s)
    -> Sort: t1.col3, limit input to 5 row(s) per chunk
        -> Table scan on <temporary>
            -> Final Aggregate using temporary table
                -> PX Receiver (slice: 0; workers: 1)
                    -> PX Sender (slice: 1; workers: 4)
                        -> Table scan on <temporary>
                            -> Aggregate using temporary table
                                -> Parallel table scan on t1  (cost=1.25 rows=10)

select sum(id) from t1 group by col3 order by col3 limit 5;
sum(id)
10
9
8
7
6
drop table t1;
create table t1(a int, b int, c int, d int);
insert into t1 values(1,2,3,4);
insert into t1 values(1,1,1,1);
insert into t1 values(2,3,4,5);
insert into t1 values(2,2,2,2);
insert into t1 values(3,4,5,6);
insert into t1 values(4,5,6,7);
insert into t1 values(5,6,7,8);
create table t2(a int, b int, c int, d int);
insert into t2 values(1,1,1,1);
insert into t2 values(2,2,2,2);
insert into t2 values(3,3,3,3);
insert into t2 values(4,4,4,4);
insert into t2 values(5,5,5,5);
limit in one branch of union
explain format=tree (select * from t1 order by a limit 2) union (select * from t2);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=0.37..2.59 rows=7)
            -> Union materialize with deduplication  (cost=2.77..4.99 rows=7)
                -> Limit: 2 row(s)  (cost=0.95 rows=2)
                    -> PX Receiver (merge sort; slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 4)
                            -> Sort: t1.a, limit input to 2 row(s) per chunk  (cost=0.95 rows=7)
                                -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=0.75 rows=5)

(select * from t1 order by a limit 2) union (select * from t2);
a	b	c	d
1	1	1	1
1	2	3	4
2	2	2	2
3	3	3	3
4	4	4	4
5	5	5	5
limit after union
explain format=tree (select * from t1) union (select * from t2) limit 2;
EXPLAIN
-> Limit: 2 row(s)  (cost=3.11..5.40 rows=12)
    -> Table scan on <union temporary>  (cost=2.50 rows=0)
        -> Union materialize with deduplication  (cost=3.11..5.40 rows=12)
            -> Limit table size: 2 unique row(s)
                -> Table scan on t1  (cost=0.95 rows=7)
            -> Limit table size: 2 unique row(s)
                -> Table scan on t2  (cost=0.75 rows=5)

explain format=tree (select * from t1 order by a) union (select * from t2 order by a) limit 2;
EXPLAIN
-> Limit: 2 row(s)  (cost=3.11..5.40 rows=12)
    -> Table scan on <union temporary>  (cost=2.50 rows=0)
        -> Union materialize with deduplication  (cost=3.11..5.40 rows=12)
            -> Limit table size: 2 unique row(s)
                -> Table scan on t1  (cost=0.95 rows=7)
            -> Limit table size: 2 unique row(s)
                -> Table scan on t2  (cost=0.75 rows=5)

# --include set_px_to_saved.inc
DROP TABLE t1;
DROP TABLE t2;
drop database test1;
drop user user1;

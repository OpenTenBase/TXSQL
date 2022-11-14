# Infrastructure for test parallel union compatibility check
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
create table t1(a int, b int, c int, d int);
insert into t1 values(1,2,3,4);
insert into t1 values(2,3,4,5);
insert into t1 values(3,4,5,6);
insert into t1 values(4,5,6,7);
insert into t1 values(5,6,7,8);
create table t2(a int, b int, c int, d int);
insert into t2 values(1,1,1,1);
insert into t2 values(2,2,2,2);
insert into t2 values(3,3,3,3);
insert into t2 values(4,4,4,4);
insert into t2 values(5,5,5,5);
create table t3(a int, b int, c int, d int, primary key(a), key idx(b));
insert into t3 values(1,1,1,1);
insert into t3 values(2,2,2,2);
insert into t3 values(3,3,3,3);
insert into t3 values(4,4,4,4);
insert into t3 values(5,5,5,5);
insert into t3 values(6,6,6,6);
insert into t3 values(7,7,7,7);
insert into t3 values(8,8,8,8);
insert into t3 values(9,9,9,9);
insert into t3 values(10,10,10,10);
create table t4(a int, b int, c int, d int);
insert into t4 values(100,100,100,100);
insert into t4 values(200,200,200,200);
insert into t4 values(300,300,300,300);
insert into t4 values(400,400,400,400);
insert into t4 values(500,500,500,500);
create table t5(a mediumtext);
INSERT INTO t5 VALUES ('a'),('b');
CREATE TABLE t6 (b varchar(20));
# --include save_and_set_px.inc
#
# Basic check for select with union all / union
#
# Check for union all
explain format=tree select * from t1 union all select * from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select * from t1 union all select * from t2 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union all select * from t2 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union all select * from t3 where a between 1 and 9;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t3.a between 1 and 9)  (cost=*** rows=***)
                            -> Parallel index range scan on t3 using PRIMARY over (1 <= a <= 9)  (cost=*** rows=***)

explain format=tree select * from t1 where a >=1 union all select * from t1 where b > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a >= 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t1.b > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union all select * from t2 where a = 1 union all select * from t3;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a = 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Parallel table scan on t3  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union all select * from t2 where c > 3 union all select * from t1 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.c > 3)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union all select * from t2 where c > 1 union all select * from t1 where a > 1 union all select * from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.c > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 4)
                        -> Parallel table scan on t4  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union all select * from t4 union all select * from t1 where b > 1 union all select * from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t4  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.b > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 4)
                        -> Parallel table scan on t4  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union all select * from t4 union all select * from t1 union all select * from t2 where a > 1 union all select * from t3 union all select * from t3 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t4  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 6; workers: 4)
                        -> Parallel table scan on t3  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 7; workers: 4)
                        -> Filter: (t3.a > 1)  (cost=*** rows=***)
                            -> Parallel index range scan on t3 using PRIMARY over (1 < a)  (cost=*** rows=***)

# Check for union
explain format=tree select * from t1 union select * from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.62 rows=***)
            -> Union materialize with deduplication  (cost=***..5.12 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select * from t1 union select * from t2 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.58 rows=***)
            -> Union materialize with deduplication  (cost=***..4.74 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union select * from t2 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.54 rows=***)
            -> Union materialize with deduplication  (cost=***..4.37 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union select * from t3 where a between 1 and 9;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.62 rows=***)
            -> Union materialize with deduplication  (cost=***..6.50 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t3.a between 1 and 9)  (cost=*** rows=***)
                            -> Parallel index range scan on t3 using PRIMARY over (1 <= a <= 9)  (cost=*** rows=***)

explain format=tree select * from t1 where a >=1 union select * from t1 where b > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.54 rows=***)
            -> Union materialize with deduplication  (cost=***..4.37 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a >= 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t1.b > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union select * from t2 where a = 1 union select * from t3;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.65 rows=***)
            -> Union materialize with deduplication  (cost=***..6.67 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.a = 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Parallel table scan on t3  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union select * from t2 where c > 3 union select * from t1 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.55 rows=***)
            -> Union materialize with deduplication  (cost=***..5.30 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.c > 3)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union select * from t2 where c > 1 union select * from t1 where a > 1 union select * from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.61 rows=***)
            -> Union materialize with deduplication  (cost=***..6.61 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: (t2.c > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 4)
                        -> Parallel table scan on t4  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union select * from t4 union select * from t1 where b > 1 union select * from t4;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.66 rows=***)
            -> Union materialize with deduplication  (cost=***..7.00 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t4  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Filter: (t1.b > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 4)
                        -> Parallel table scan on t4  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union select * from t4 union select * from t1 union select * from t2 where a > 1 union select * from t3 union select * from t3 where a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.90 rows=***)
            -> Union materialize with deduplication  (cost=***..12.45 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: (t1.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t4  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 4; workers: 4)
                        -> Parallel table scan on t1  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 5; workers: 4)
                        -> Filter: (t2.a > 1)  (cost=*** rows=***)
                            -> Parallel table scan on t2  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 6; workers: 4)
                        -> Parallel table scan on t3  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 7; workers: 4)
                        -> Filter: (t3.a > 1)  (cost=*** rows=***)
                            -> Parallel index range scan on t3 using PRIMARY over (1 < a)  (cost=*** rows=***)

#
# Check for select with union all / union
# Each query block has complex iterators
#
# Check for union all
explain format=tree select a,sum(b) from t1 group by a union all select b,sum(c) from t2 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select a,sum(b) from t1 group by a union all select b,sum(c) from t2 group by b union all select b,sum(d) from t3 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> Group aggregate: sum(t3.d)
                    -> PX Receiver (merge sort; slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Group aggregate: sum(t3.d)  (cost=*** rows=***)
                                -> Parallel index scan on t3 using idx  (cost=*** rows=***)

explain format=tree select a,sum(b),sum(d) from t1 group by a union all select b,sum(b),avg(d) from t2 group by b union all select b,sum(d),avg(c) from t3 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> Group aggregate: sum(t3.d), sum(t3.c), count(t3.c)
                    -> PX Receiver (merge sort; slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Group aggregate: sum(t3.d), sum(t3.c), count(t3.c)  (cost=*** rows=***)
                                -> Parallel index scan on t3 using idx  (cost=*** rows=***)

explain format=tree (select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union all (select * from t3 where c > 1 and d > 2 order by b,c desc);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=*** rows=***)
                            -> Filter: ((t2.a > 1) and (t2.a is not null))  (cost=*** rows=***)
                                -> Parallel table scan on t2  (cost=*** rows=***)
                            -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: ((t3.c > 1) and (t3.d > 2))  (cost=*** rows=***)
                            -> Parallel table scan on t3  (cost=*** rows=***)

explain format=tree (select * from t3 where c > 1 and d > 3 order by a,b) union all (select t2.* from t1,t2,t3 where t1.a = t2.a and t2.a = t3.a order by t1.a);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: ((t3.c > 1) and (t3.d > 3))  (cost=*** rows=***)
                            -> Parallel table scan on t3  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> Nested loop inner join  (cost=*** rows=***)
                    -> Inner hash join (t2.a = t1.a)  (cost=*** rows=***)
                        -> Table scan on t2  (cost=*** rows=***)
                        -> Hash
                            -> PX Receiver (slice: 1; workers: 1)
                                -> PX Sender (slice: 3; workers: 4)
                                    -> Filter: (t1.a is not null)  (cost=*** rows=***)
                                        -> Parallel table scan on t1  (cost=*** rows=***)
                    -> Single-row covering index lookup on t3 using PRIMARY (a=t1.a)  (cost=*** rows=***)

explain format=tree (select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union all (select t2.* from t1 left join t2 on t1.a = t2.a where t1.a > 1 order by t1.b desc);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=*** rows=***)
                            -> Filter: ((t2.a > 1) and (t2.a is not null))  (cost=*** rows=***)
                                -> Parallel table scan on t2  (cost=*** rows=***)
                            -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=*** rows=***)
            -> Stream results  (cost=*** rows=***)
                -> Left hash join (t2.a = t1.a)  (cost=*** rows=***)
                    -> Filter: (t1.a > 1)  (cost=*** rows=***)
                        -> Table scan on t1  (cost=*** rows=***)
                    -> Hash
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Parallel table scan on t2  (cost=*** rows=***)

# Check for union
explain format=tree select a,sum(b) from t1 group by a union select b,sum(c) from t2 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.50 rows=***)
            -> Union materialize with deduplication  (cost=***..2.50 rows=***)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=*** rows=***)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select a,sum(b) from t1 group by a union select b,sum(c) from t2 group by b union select b,sum(d) from t3 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.62 rows=***)
            -> Union materialize with deduplication  (cost=***..5.88 rows=***)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=*** rows=***)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=*** rows=***)
                -> Group aggregate: sum(t3.d)
                    -> PX Receiver (merge sort; slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Group aggregate: sum(t3.d)  (cost=*** rows=***)
                                -> Parallel index scan on t3 using idx  (cost=*** rows=***)

explain format=tree select a,sum(b),sum(d) from t1 group by a union select b,sum(b),avg(d) from t2 group by b union select b,sum(d),avg(c) from t3 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.62 rows=***)
            -> Union materialize with deduplication  (cost=***..5.88 rows=***)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=*** rows=***)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t2  (cost=*** rows=***)
                -> Group aggregate: sum(t3.d), sum(t3.c), count(t3.c)
                    -> PX Receiver (merge sort; slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Group aggregate: sum(t3.d), sum(t3.c), count(t3.c)  (cost=*** rows=***)
                                -> Parallel index scan on t3 using idx  (cost=*** rows=***)

explain format=tree (select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select * from t3 where c > 1 and d > 2 order by b,c desc);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.58 rows=***)
            -> Union materialize with deduplication  (cost=***..6.94 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=*** rows=***)
                            -> Filter: ((t2.a > 1) and (t2.a is not null))  (cost=*** rows=***)
                                -> Parallel table scan on t2  (cost=*** rows=***)
                            -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: ((t3.c > 1) and (t3.d > 2))  (cost=*** rows=***)
                            -> Parallel table scan on t3  (cost=*** rows=***)

explain format=tree (select * from t3 where c > 1 and d > 3 order by a,b) union (select t2.* from t1,t2,t3 where t1.a = t2.a and t2.a = t3.a order by t1.a);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.58 rows=***)
            -> Union materialize with deduplication  (cost=***..8.69 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Filter: ((t3.c > 1) and (t3.d > 3))  (cost=*** rows=***)
                            -> Parallel table scan on t3  (cost=*** rows=***)
                -> Nested loop inner join  (cost=*** rows=***)
                    -> Inner hash join (t2.a = t1.a)  (cost=*** rows=***)
                        -> Table scan on t2  (cost=*** rows=***)
                        -> Hash
                            -> PX Receiver (slice: 1; workers: 1)
                                -> PX Sender (slice: 3; workers: 4)
                                    -> Filter: (t1.a is not null)  (cost=*** rows=***)
                                        -> Parallel table scan on t1  (cost=*** rows=***)
                    -> Single-row covering index lookup on t3 using PRIMARY (a=t1.a)  (cost=*** rows=***)

explain format=tree (select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select t2.* from t1 left join t2 on t1.a = t2.a where t1.a > 1 order by t1.b desc);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.66 rows=***)
            -> Union materialize with deduplication  (cost=***..8.03 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=*** rows=***)
                            -> Filter: ((t2.a > 1) and (t2.a is not null))  (cost=*** rows=***)
                                -> Parallel table scan on t2  (cost=*** rows=***)
                            -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=*** rows=***)
                -> Left hash join (t2.a = t1.a)  (cost=*** rows=***)
                    -> Filter: (t1.a > 1)  (cost=*** rows=***)
                        -> Table scan on t1  (cost=*** rows=***)
                    -> Hash
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Parallel table scan on t2  (cost=*** rows=***)

#
# Query contains union all / union
#
# Check for union all
explain format=tree select * from t1 union all select * from t2 order by a;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Sort: a  (cost=***..5.00 rows=***)
            -> Table scan on <union temporary>  (cost=*** rows=***)
                -> Union materialize  (cost=***..5.00 rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 4)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union all select * from t2 union all select * from t3 order by a,b;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Sort: a, b  (cost=***..6.92 rows=***)
            -> Table scan on <union temporary>  (cost=*** rows=***)
                -> Union materialize  (cost=***..6.92 rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 4)
                            -> Filter: (t1.a > 1)  (cost=*** rows=***)
                                -> Parallel table scan on t1  (cost=*** rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=*** rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Parallel table scan on t3  (cost=*** rows=***)

explain format=tree select * from ((select t2.a as c1,t2.b as c2,t2.c as c3,t2.d as c4 from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union all (select t3.a as c1,t3.b as c2,t3.c as c3,t3.d as c4 from t3 where c > 1 and d > 2 order by b,c desc)) as v where v.c1 > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on v  (cost=***..2.52 rows=***)
            -> Union materialize  (cost=***..6.19 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=*** rows=***)
                            -> Filter: ((t2.a > 1) and (t2.a > 1) and (t2.a is not null))  (cost=*** rows=***)
                                -> Parallel table scan on t2  (cost=*** rows=***)
                            -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: ((t3.c > 1) and (t3.d > 2) and (t3.a > 1))  (cost=*** rows=***)
                            -> Parallel index range scan on t3 using PRIMARY over (1 < a)  (cost=*** rows=***)

# Check for union
explain format=tree select * from t1 union select * from t2 order by a;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Sort: a  (cost=***..5.00 rows=***)
            -> Table scan on <union temporary>  (cost=*** rows=***)
                -> Union materialize with deduplication  (cost=***..5.00 rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 4)
                            -> Parallel table scan on t1  (cost=*** rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select * from t1 where a > 1 union select * from t2 union select * from t3 order by a,b;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Sort: a, b  (cost=***..6.92 rows=***)
            -> Table scan on <union temporary>  (cost=*** rows=***)
                -> Union materialize with deduplication  (cost=***..6.92 rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 2; workers: 4)
                            -> Filter: (t1.a > 1)  (cost=*** rows=***)
                                -> Parallel table scan on t1  (cost=*** rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 3; workers: 4)
                            -> Parallel table scan on t2  (cost=*** rows=***)
                    -> PX Receiver (slice: 1; workers: 1)
                        -> PX Sender (slice: 4; workers: 4)
                            -> Parallel table scan on t3  (cost=*** rows=***)

explain format=tree select * from ((select t2.a as c1,t2.b as c2,t2.c as c3,t2.d as c4 from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a) union (select t3.a as c1,t3.b as c2,t3.c as c3,t3.d as c4 from t3 where c > 1 and d > 2 order by b,c desc)) as v where v.c1 > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on v  (cost=***..2.52 rows=***)
            -> Union materialize with deduplication  (cost=***..6.19 rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Nested loop inner join  (cost=*** rows=***)
                            -> Filter: ((t2.a > 1) and (t2.a > 1) and (t2.a is not null))  (cost=*** rows=***)
                                -> Parallel table scan on t2  (cost=*** rows=***)
                            -> Single-row covering index lookup on t3 using PRIMARY (a=t2.a)  (cost=*** rows=***)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Filter: ((t3.c > 1) and (t3.d > 2) and (t3.a > 1))  (cost=*** rows=***)
                            -> Parallel index range scan on t3 using PRIMARY over (1 < a)  (cost=*** rows=***)

#
# Part of branch query blocks don't pass compatibility check
#
explain format=tree select sum(a) from t1 group by b union all select variance(a) from t2 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select sum(a) from t1 group by b union select variance(a) from t2 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.50 rows=***)
            -> Union materialize with deduplication  (cost=***..2.50 rows=***)
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=*** rows=***)
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Parallel table scan on t2  (cost=*** rows=***)

#
# Part of branch query blocks don't pass compatibility check
#
explain format=tree select variance(a) from t1 group by b union all select variance(a) from t2 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Parallel table scan on t1  (cost=*** rows=***)
            -> Stream results
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Parallel table scan on t2  (cost=*** rows=***)

explain format=tree select variance(a) from t1 group by b union select variance(a) from t2 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=***..2.50 rows=***)
            -> Union materialize with deduplication  (cost=***..2.50 rows=***)
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Parallel table scan on t1  (cost=*** rows=***)
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 3; workers: 4)
                                -> Parallel table scan on t2  (cost=*** rows=***)

#
# Union's temporary table contains incompatible fields.
#
explain format=tree SELECT left(a,100000000) FROM t5 UNION  SELECT b FROM t6;
EXPLAIN
-> Table scan on <union temporary>  (cost=***..2.54 rows=***)
    -> Union materialize with deduplication  (cost=***..3.64 rows=***)
        -> Table scan on t5  (cost=*** rows=***)
        -> Table scan on t6  (cost=*** rows=***)

# --include set_px_to_saved.inc
drop table t1;
drop table t2;
drop table t3;
drop table t4;
drop table t5;
drop table t6;
drop database test1;
drop user user1;

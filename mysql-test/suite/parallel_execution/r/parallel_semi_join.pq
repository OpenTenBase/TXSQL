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
set optimizer_switch='semijoin=on,loosescan=on';
set @old_opt_switch=@@optimizer_switch;
set optimizer_switch='subquery_materialization_cost_based=off';
create table t1 (a int, b int, index a (a,b));
create table t2 (a int, index a (a));
insert into t1 values (1,10), (2,20), (3,30), (4,40);
create table t0(a int);
insert into t0 values (0),(1),(2),(3),(4),(5),(6),(7),(8),(9);
insert into t1
select rand()*100000+200,rand()*100000 from t0 A, t0 B, t0 C, t0 D;
insert into t2 values (2), (3), (4), (5);
ANALYZE TABLE t1,t2;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
loosescan stategy check
explain select * from t2 where t2.a in (select a from t1);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	index	a	a	10	NULL	#	#	Using where; Using index; LooseScan
1	SIMPLE	t2	NULL	ref	a	a	5	test.t1.a	#	#	Using index
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a` from `test`.`t2` semi join (`test`.`t1`) where (`test`.`t2`.`a` = `test`.`t1`.`a`)
explain format=tree select * from t2 where t2.a in (select a from t1);
EXPLAIN
-> Nested loop inner join  (cost=*** rows=***)
    -> Remove duplicates from input sorted on a  (cost=*** rows=***)
        -> Filter: (t1.a is not null)  (cost=*** rows=***)
            -> Index scan on t1 using a  (cost=*** rows=***)
    -> Covering index lookup on t2 using a (a=t1.a)  (cost=*** rows=***)

materialization stategy check
set optimizer_switch='semijoin=on,loosescan=off';
set optimizer_switch='materialization=on';
explain select * from t2 where t2.a in (select a from t1);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	a	a	5	NULL	#	#	Parallel scan (4 workers); Using where; Using index
2	MATERIALIZED	t1	NULL	index	a	a	10	NULL	#	#	Using index
1	SIMPLE	<subquery2>	NULL	eq_ref	<auto_distinct_key>	<auto_distinct_key>	5	test.t2.a	#	#	NULL
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	#	#	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	#	#	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a` from `test`.`t2` semi join (`test`.`t1`) where (`<subquery2>`.`a` = `test`.`t2`.`a`)
explain format=tree select * from t2 where t2.a in (select a from t1);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=*** rows=***)
            -> Filter: (t2.a is not null)  (cost=*** rows=***)
                -> Parallel index scan on t2 using a  (cost=*** rows=***)
            -> Single-row index lookup on <subquery2> using <auto_distinct_key> (a=t2.a)
                -> Materialize with deduplication  (cost=***..2037.40 rows=***)
                    -> Filter: (t1.a is not null)  (cost=*** rows=***)
                        -> Index scan on t1 using a  (cost=*** rows=***)

firstmatch stategy check
set optimizer_switch='semijoin=on,materialization=off';
set optimizer_switch='firstmatch=on';
explain select * from t2 where t2.a in (select a from t1);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	a	a	5	NULL	#	#	Parallel scan (4 workers); Using where; Using index
1	SIMPLE	t1	NULL	ref	a	a	5	test.t2.a	#	#	Using index; FirstMatch(t2)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	#	#	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	#	#	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a` from `test`.`t2` semi join (`test`.`t1`) where (`test`.`t1`.`a` = `test`.`t2`.`a`)
explain format=tree select * from t2 where t2.a in (select a from t1);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop semijoin  (cost=*** rows=***)
            -> Filter: (t2.a is not null)  (cost=*** rows=***)
                -> Parallel index scan on t2 using a  (cost=*** rows=***)
            -> Covering index lookup on t1 using a (a=t2.a)  (cost=*** rows=***)

duplicateweedout stategy check
set optimizer_switch='semijoin=on,firstmatch=off';
set optimizer_switch='duplicateweedout=on';
explain select * from t2 where t2.a in (select a from t1);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	a	a	5	NULL	#	#	Parallel scan (4 workers); Using where; Using index
1	SIMPLE	t1	NULL	ref	a	a	5	test.t2.a	#	#	Using index; Start temporary; End temporary
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	#	#	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	#	#	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a` from `test`.`t2` semi join (`test`.`t1`) where (`test`.`t1`.`a` = `test`.`t2`.`a`)
explain format=tree select * from t2 where t2.a in (select a from t1);
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=*** rows=***)
            -> Filter: (t2.a is not null)  (cost=*** rows=***)
                -> Parallel index scan on t2 using a  (cost=*** rows=***)
            -> Limit: 1 row(s)  (cost=*** rows=***)
                -> Covering index lookup on t1 using a (a=t2.a)  (cost=*** rows=***)

set optimizer_switch=default;
# --include set_px_to_saved.inc
drop table t0;
drop table t1;
drop table t2;
drop database test1;
drop user user1;

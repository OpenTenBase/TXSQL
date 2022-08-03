# Infrastructure for Optimizer Outline
show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,GROUP_REPLICATION_ADMIN,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PERSIST_RO_VARIABLES_ADMIN,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ''@'' TO 'root'@'localhost' WITH GRANT OPTION
set global cdb_opt_outline_enabled = off;
select user();
user()
tencentroot@localhost
drop database if exists test;
create database test;
use test;
drop table if exists t1;
Warnings:
Note	1051	Unknown table 'test.t1'
drop table if exists t2;
Warnings:
Note	1051	Unknown table 'test.t2'
drop table if exists t3;
Warnings:
Note	1051	Unknown table 'test.t3'
drop table if exists t4;
Warnings:
Note	1051	Unknown table 'test.t4'
drop table if exists t5;
Warnings:
Note	1051	Unknown table 'test.t5'
create table t1(a int, b int, c int, unique key idx1(a));
create table t2(a int, b int, c int, unique key idx2(a));
create table t3(a int, b int, c int, unique key idx3(a));
create table t4(a int, b int, c int, unique key idx4(a));
create table t5(a int, b int, c int, unique key idx5(a));
insert into t1 values(1,2,3);
insert into t1 values(2,3,4);
insert into t1 values(3,2,3);
insert into t1 values(4,3,4);
insert into t1 values(5,2,3);
insert into t1 values(6,3,4);
insert into t2 values(1,2,3);
insert into t2 values(2,3,4);
insert into t2 values(3,2,3);
insert into t2 values(4,3,4);
insert into t3 values(1,2,3);
insert into t3 values(2,3,4);
insert into t3 values(5,2,3);
insert into t3 values(3,3,4);
insert into t4 values(1,2,3);
insert into t4 values(2,3,4);
insert into t4 values(6,2,3);
insert into t4 values(5,3,4);
insert into t5 values(1,2,3);
insert into t5 values(2,3,4);
insert into t5 values(6,2,3);
insert into t5 values(7,3,4);
create user 'user1'@'%';
grant all on test.* to 'user1'@'%';
grant all on mysql.* to 'user1'@'%';
create user 'user2'@'%';
grant all on test.* to 'user1'@'%';
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "OUTLINE:select * from t1 use index(idx1), t2 where t1.a = t2.a";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from `test`.`t1` USE INDEX (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`)
show cdb_outline_info;
origin	outline	hit
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from `test`.`t1` USE INDEX (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from `test`.`t1` USE INDEX (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	0
set global cdb_opt_outline_enabled = on;
select * from t1, t2 where t1.a = t2.a;
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	2	3	3	2	3
4	3	4	4	3	4
explain select * from t1, t2 where t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	idx2	NULL	NULL	NULL	4	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` USE INDEX (`idx1`) join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
explain select * from t1 use index(idx1), t2 where t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	idx2	NULL	NULL	NULL	4	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` USE INDEX (`idx1`) join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from `test`.`t1` USE INDEX (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`)
select * from t1, t2 where t1.a = t2.a;
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	2	3	3	2	3
4	3	4	4	3	4
outline reset "select * from t1, t2 where t1.a = t2.a";
explain select * from t1, t2 where t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	idx2	NULL	NULL	NULL	4	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
outline "select * from t1" set outline_info "OUTLINE:select * from t1 use index(idx1)";
ERROR 42000: Access denied; you need (at least one of) the SUPER privilege(s) for this operation
outline reset all;
ERROR 42000: Access denied; you need (at least one of) the SUPER privilege(s) for this operation
truncate table mysql.outline;
outline flush;
set global cdb_opt_outline_enabled = off;
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "OPT:1#max_execution_time(100)";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ max_execution_time(100)*/ * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)
show cdb_outline_info;
origin	outline	hit
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ max_execution_time(100)*/ * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ max_execution_time(100)*/ * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	0
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "OPT:1#SET_VAR(sort_buffer_size = 16M)#";
outline "select * from t1 left join t2 on t1.c = t2.c" set outline_info "OPT:1#SET_VAR(foreign_key_checks=OFF)#";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size = 16M)*/ * from (`test`.`t1` join `test`.`t2`) where (`t1`.`a` = `t2`.`a`)
2	bc47a70ca97c08023f034d8f0de7b86124d363d82b3ea512e5604266a885fae6	select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	select /*+ SET_VAR(foreign_key_checks=OFF)*/ * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	select /*+ SET_VAR(foreign_key_checks=OFF)*/ * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size = 16M)*/ * from (`test`.`t1` join `test`.`t2`) where (`t1`.`a` = `t2`.`a`)	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	select /*+ SET_VAR(foreign_key_checks=OFF)*/ * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size = 16M)*/ * from (`test`.`t1` join `test`.`t2`) where (`t1`.`a` = `t2`.`a`)	0
set session cdb_opt_outline_enabled = on;
explain format=tree select * from t1, t2 where t1.a = t2.a;
EXPLAIN
-> Nested loop inner join  (cost=2.05 rows=4)
    -> Filter: (t2.a is not null)  (cost=0.65 rows=4)
        -> Table scan on t2  (cost=0.65 rows=4)
    -> Single-row index lookup on t1 using idx1 (a=t2.a)  (cost=0.28 rows=1)

explain format=json select * from t1, t2 where t1.a = t2.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.05"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "ALL",
          "possible_keys": [
            "idx2"
          ],
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ],
          "attached_condition": "(`test`.`t2`.`a` is not null)"
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t2.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "2.05",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size=16777216) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
explain select * from t1, t2 where t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	idx2	NULL	NULL	NULL	4	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size=16777216) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
select * from t1, t2 where t1.a = t2.a;
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	2	3	3	2	3
4	3	4	4	3	4
explain format=tree select * from t1 left join t2 on t1.c = t2.c;
EXPLAIN
-> Left hash join (t2.c = t1.c)  (cost=2.76 rows=24)
    -> Table scan on t1  (cost=0.85 rows=6)
    -> Hash
        -> Table scan on t2  (cost=0.11 rows=4)

explain format=json select * from t1 left join t2 on t1.c = t2.c;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "3.50"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t1",
          "access_type": "ALL",
          "rows_examined_per_scan": 6,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.60",
            "prefix_cost": "0.85",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t2",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 24,
          "filtered": "100.00",
          "using_join_buffer": "hash join",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "2.40",
            "prefix_cost": "3.50",
            "data_read_per_join": "384"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ],
          "attached_condition": "<if>(is_not_null_compl(t2), (`test`.`t2`.`c` = `test`.`t1`.`c`), true)"
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ SET_VAR(foreign_key_checks='OFF') */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` left join `test`.`t2` on((`test`.`t2`.`c` = `test`.`t1`.`c`)) where true
explain select * from t1 left join t2 on t1.c = t2.c;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	6	100.00	NULL
1	SIMPLE	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using where; Using join buffer (hash join)
Warnings:
Note	1003	/* select#1 */ select /*+ SET_VAR(foreign_key_checks='OFF') */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` left join `test`.`t2` on((`test`.`t2`.`c` = `test`.`t1`.`c`)) where true
select * from t1 left join t2 on t1.c = t2.c;
a	b	c	a	b	c
1	2	3	1	2	3
1	2	3	3	2	3
2	3	4	2	3	4
2	3	4	4	3	4
3	2	3	1	2	3
3	2	3	3	2	3
4	3	4	2	3	4
4	3	4	4	3	4
5	2	3	1	2	3
5	2	3	3	2	3
6	3	4	2	3	4
6	3	4	4	3	4
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "OUTLINE:select * from t1 use index(idx1), t2 where t1.a = t2.a";
ERROR HY000: SET outline_info failed, reason: insert failed, duplicated record
outline "select * from t1 where t1.a in (select b from t2)" set outline_info "OPT:2#qb_name(qb2)";
outline "select * from t1 where t1.a in (select b from t2)" set outline_info "OPT:1#SEMIJOIN(@qb2 MATERIALIZATION, DUPSWEEDOUT)";
outline "select * from t1 where t1.a in (select b from t2)" set outline_info "OPT:1#max_execution_time(100)";
select * from  mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size = 16M)*/ * from (`test`.`t1` join `test`.`t2`) where (`t1`.`a` = `t2`.`a`)
2	bc47a70ca97c08023f034d8f0de7b86124d363d82b3ea512e5604266a885fae6	select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	select /*+ SET_VAR(foreign_key_checks=OFF)*/ * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))
3	88b353822893500c158505e1b55aeeccb4d52f7b7ad42fb0745f994f535db746	select * from `test`.`t1` where `t1`.`a` in (select `b` from `test`.`t2`)	select /*+ SEMIJOIN(@`qb2` MATERIALIZATION, DUPSWEEDOUT) max_execution_time(100)*/ * from `test`.`t1` where `t1`.`a` in (select /*+ QB_NAME(`qb2`) */ `b` from `test`.`t2`)
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	select /*+ SET_VAR(foreign_key_checks=OFF)*/ * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	4
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size = 16M)*/ * from (`test`.`t1` join `test`.`t2`) where (`t1`.`a` = `t2`.`a`)	4
select * from `test`.`t1` where `t1`.`a` in (select `b` from `test`.`t2`)	select /*+ SEMIJOIN(@`qb2` MATERIALIZATION, DUPSWEEDOUT) max_execution_time(100)*/ * from `test`.`t1` where `t1`.`a` in (select /*+ QB_NAME(`qb2`) */ `b` from `test`.`t2`)	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	select /*+ SET_VAR(foreign_key_checks=OFF)*/ * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	4
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size = 16M)*/ * from (`test`.`t1` join `test`.`t2`) where (`t1`.`a` = `t2`.`a`)	4
select * from `test`.`t1` where `t1`.`a` in (select `b` from `test`.`t2`)	select /*+ SEMIJOIN(@`qb2` MATERIALIZATION, DUPSWEEDOUT) max_execution_time(100)*/ * from `test`.`t1` where `t1`.`a` in (select /*+ QB_NAME(`qb2`) */ `b` from `test`.`t2`)	0
outline "select * from t1 join t2 on t1.a = t2.a" set outline_info "OPT:1#BKA(t1)";
outline "select * from t1 join t2 on t1.a = t2.a" set outline_info "OPT:1#NO_BKA(t2)";
outline "select t1.a from t1 inner join t2 inner join t3" set outline_info "OPT:1#NO_BKA(t1, t2)#";
outline "select t1.a from t1 inner join t2 inner join t4" set outline_info "OPT:1#NO_BNL()#";
outline "select * from (select * from t1) as t1_alias" set outline_info "OPT:1#NO_MERGE(t1_alias)#";
outline "select count(*) from t1 join t2 on t1.a = t2.a" set outline_info "OPT:1#HASH_JOIN(t1,t2)#";
outline "select t1.a, avg(t1.b) from t1 join t2 on t1.a = t2.a group by t1.a" set outline_info "OPT:1#NO_HASH_JOIN(t1,t2)#";
outline "select a from t1 where t1.a in (select t1.b from t1 join t2 on t1.a = t2.a)" set outline_info "OPT:2#QB_NAME(qb2)#";
outline "select a from t1 where t1.a in (select t1.b from t1 join t2 on t1.a = t2.a)" set outline_info "OPT:1#HASH_JOIN(t1@qb2,t2@qb2)#";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size = 16M)*/ * from (`test`.`t1` join `test`.`t2`) where (`t1`.`a` = `t2`.`a`)
10	6685d4eac2ad993e7ed68b18ec19551b4fda9834ff23746a206a05c53576d265	select `a` from `test`.`t1` where `t1`.`a` in (select `t1`.`b` from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))))	select /*+ HASH_JOIN(t1@qb2,t2@qb2)*/ `a` from `test`.`t1` where `t1`.`a` in (select /*+ QB_NAME(`qb2`) */ `t1`.`b` from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))))
2	bc47a70ca97c08023f034d8f0de7b86124d363d82b3ea512e5604266a885fae6	select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	select /*+ SET_VAR(foreign_key_checks=OFF)*/ * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))
3	88b353822893500c158505e1b55aeeccb4d52f7b7ad42fb0745f994f535db746	select * from `test`.`t1` where `t1`.`a` in (select `b` from `test`.`t2`)	select /*+ SEMIJOIN(@`qb2` MATERIALIZATION, DUPSWEEDOUT) max_execution_time(100)*/ * from `test`.`t1` where `t1`.`a` in (select /*+ QB_NAME(`qb2`) */ `b` from `test`.`t2`)
4	ea9e01af47e39b510b2f992f19f78056cb20124de31967e25fd97601386eaad4	select * from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select /*+ BKA(`t1`@`select#1`) NO_BKA(t2)*/ * from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))
5	08f756fef2fa288bdd459279931a4eeb5cc050b9d50e74a3dd9d0a59586da444	select `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t3`)	select /*+ NO_BKA(t1, t2)*/ `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t3`)
6	13bae6da7345941bd7edba4e2702a713a8c9365f77b91693bba10fe5ff66b9a1	select `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t4`)	select /*+ NO_BNL()*/ `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t4`)
7	7fedf71ce2a938f663ea5828f971c30108cfa62e882432626999ddf503513563	select * from (select * from `test`.`t1`) `t1_alias`	select /*+ NO_MERGE(t1_alias)*/ * from (select * from `test`.`t1`) `t1_alias`
8	f0eb5b91ae1da9629d1947f4f42c2732daf10be2c5de3cff83579428d56d9d63	select count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select /*+ HASH_JOIN(t1,t2)*/ count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))
9	73c6614fddecb012edd14b7a57c7cbb3946e91f292c9811d5e7c9b8b54861752	select `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`	select /*+ NO_HASH_JOIN(t1,t2)*/ `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select /*+ BKA(`t1`@`select#1`) NO_BKA(t2)*/ * from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	0
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	select /*+ SET_VAR(foreign_key_checks=OFF)*/ * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	4
select * from (select * from `test`.`t1`) `t1_alias`	select /*+ NO_MERGE(t1_alias)*/ * from (select * from `test`.`t1`) `t1_alias`	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size = 16M)*/ * from (`test`.`t1` join `test`.`t2`) where (`t1`.`a` = `t2`.`a`)	4
select * from `test`.`t1` where `t1`.`a` in (select `b` from `test`.`t2`)	select /*+ SEMIJOIN(@`qb2` MATERIALIZATION, DUPSWEEDOUT) max_execution_time(100)*/ * from `test`.`t1` where `t1`.`a` in (select /*+ QB_NAME(`qb2`) */ `b` from `test`.`t2`)	0
select `a` from `test`.`t1` where `t1`.`a` in (select `t1`.`b` from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))))	select /*+ HASH_JOIN(t1@qb2,t2@qb2)*/ `a` from `test`.`t1` where `t1`.`a` in (select /*+ QB_NAME(`qb2`) */ `t1`.`b` from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))))	0
select `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t3`)	select /*+ NO_BKA(t1, t2)*/ `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t3`)	0
select `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t4`)	select /*+ NO_BNL()*/ `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t4`)	0
select `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`	select /*+ NO_HASH_JOIN(t1,t2)*/ `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`	0
select count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select /*+ HASH_JOIN(t1,t2)*/ count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select /*+ BKA(`t1`@`select#1`) NO_BKA(t2)*/ * from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	0
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	select /*+ SET_VAR(foreign_key_checks=OFF)*/ * from (`test`.`t1` left join `test`.`t2` on((`t1`.`c` = `t2`.`c`)))	4
select * from (select * from `test`.`t1`) `t1_alias`	select /*+ NO_MERGE(t1_alias)*/ * from (select * from `test`.`t1`) `t1_alias`	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size = 16M)*/ * from (`test`.`t1` join `test`.`t2`) where (`t1`.`a` = `t2`.`a`)	4
select * from `test`.`t1` where `t1`.`a` in (select `b` from `test`.`t2`)	select /*+ SEMIJOIN(@`qb2` MATERIALIZATION, DUPSWEEDOUT) max_execution_time(100)*/ * from `test`.`t1` where `t1`.`a` in (select /*+ QB_NAME(`qb2`) */ `b` from `test`.`t2`)	0
select `a` from `test`.`t1` where `t1`.`a` in (select `t1`.`b` from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))))	select /*+ HASH_JOIN(t1@qb2,t2@qb2)*/ `a` from `test`.`t1` where `t1`.`a` in (select /*+ QB_NAME(`qb2`) */ `t1`.`b` from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))))	0
select `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t3`)	select /*+ NO_BKA(t1, t2)*/ `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t3`)	0
select `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t4`)	select /*+ NO_BNL()*/ `t1`.`a` from ((`test`.`t1` join `test`.`t2`) join `test`.`t4`)	0
select `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`	select /*+ NO_HASH_JOIN(t1,t2)*/ `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`	0
select count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select /*+ HASH_JOIN(t1,t2)*/ count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	0
set global cdb_opt_outline_enabled = on;
explain format=tree select * from t1, t2 where t1.a = t2.a;
EXPLAIN
-> Nested loop inner join  (cost=2.05 rows=4)
    -> Filter: (t2.a is not null)  (cost=0.65 rows=4)
        -> Table scan on t2  (cost=0.65 rows=4)
    -> Single-row index lookup on t1 using idx1 (a=t2.a)  (cost=0.28 rows=1)

explain format=json select * from t1, t2 where t1.a = t2.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.05"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "ALL",
          "possible_keys": [
            "idx2"
          ],
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ],
          "attached_condition": "(`test`.`t2`.`a` is not null)"
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t2.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "2.05",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size=16777216) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
explain select * from t1, t2 where t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	idx2	NULL	NULL	NULL	4	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select /*+ MAX_EXECUTION_TIME(100) SET_VAR(sort_buffer_size=16777216) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
select * from t1, t2 where t1.a = t2.a;
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	2	3	3	2	3
4	3	4	4	3	4
explain format=tree select * from t1 where t1.a in (select b from t2);
EXPLAIN
-> Nested loop inner join  (cost=1.45 rows=4)
    -> Filter: (`<subquery2>`.b is not null)  (cost=0.80 rows=4)
        -> Table scan on <subquery2>  (cost=0.80 rows=4)
            -> Materialize with deduplication  (cost=0.65 rows=4)
                -> Filter: (t2.b is not null)  (cost=0.65 rows=4)
                    -> Table scan on t2  (cost=0.65 rows=4)
    -> Single-row index lookup on t1 using idx1 (a=`<subquery2>`.b)  (cost=0.35 rows=1)

explain format=json select * from t1 where t1.a in (select b from t2);
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "3.85"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "<subquery2>",
          "access_type": "ALL",
          "attached_condition": "(`<subquery2>`.`b` is not null)",
          "materialized_from_subquery": {
            "using_temporary_table": true,
            "query_block": {
              "table": {
                "table_name": "t2",
                "access_type": "ALL",
                "rows_examined_per_scan": 4,
                "rows_produced_per_join": 4,
                "filtered": "100.00",
                "cost_info": {
                  "read_cost": "0.25",
                  "eval_cost": "0.40",
                  "prefix_cost": "0.65",
                  "data_read_per_join": "64"
                },
                "used_columns": [
                  "b"
                ]
              }
            }
          }
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "<subquery2>.b"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 1,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.10",
            "prefix_cost": "3.85",
            "data_read_per_join": "16"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ MAX_EXECUTION_TIME(100) SEMIJOIN(@`qb2` MATERIALIZATION, DUPSWEEDOUT) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` semi join (`test`.`t2`) where (`test`.`t1`.`a` = `<subquery2>`.`b`)
explain select * from t1 where t1.a in (select b from t2);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	<subquery2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	<subquery2>.b	1	100.00	NULL
2	MATERIALIZED	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select /*+ MAX_EXECUTION_TIME(100) SEMIJOIN(@`qb2` MATERIALIZATION, DUPSWEEDOUT) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` semi join (`test`.`t2`) where (`test`.`t1`.`a` = `<subquery2>`.`b`)
select * from t1 where t1.a in (select b from t2);
a	b	c
2	3	4
3	2	3
explain format=tree select * from t1 join t2 on t1.a = t2.a;
EXPLAIN
-> Batched key access inner join
    -> Batch input rows
        -> Filter: (t2.a is not null)  (cost=0.65 rows=4)
            -> Table scan on t2  (cost=0.65 rows=4)
    -> Multi-range index lookup on t1 using idx1 (a=t2.a)

explain format=json select * from t1 join t2 on t1.a = t2.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.05"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "ALL",
          "possible_keys": [
            "idx2"
          ],
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ],
          "attached_condition": "(`test`.`t2`.`a` is not null)"
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t2.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "using_join_buffer": "Batched Key Access",
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "2.05",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ BKA(`t1`@`select#1`) NO_BKA(`t2`@`select#1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
explain select * from t1 join t2 on t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	idx2	NULL	NULL	NULL	4	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	Using join buffer (Batched Key Access)
Warnings:
Note	1003	/* select#1 */ select /*+ BKA(`t1`@`select#1`) NO_BKA(`t2`@`select#1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
select * from t1 join t2 on t1.a = t2.a;
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	2	3	3	2	3
4	3	4	4	3	4
explain format=tree select t1.a from t1 inner join t2 inner join t3;
EXPLAIN
-> Inner hash join (no condition)  (cost=12.35 rows=96)
    -> Index scan on t1 using idx1  (cost=0.05 rows=6)
    -> Hash
        -> Inner hash join (no condition)  (cost=2.50 rows=16)
            -> Index scan on t3 using idx3  (cost=0.16 rows=4)
            -> Hash
                -> Index scan on t2 using idx2  (cost=0.65 rows=4)

explain format=json select t1.a from t1 inner join t2 inner join t3;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "12.35"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "index",
          "key": "idx2",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          }
        }
      },
      {
        "table": {
          "table_name": "t3",
          "access_type": "index",
          "key": "idx3",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 16,
          "filtered": "100.00",
          "using_index": true,
          "using_join_buffer": "hash join",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "1.60",
            "prefix_cost": "2.50",
            "data_read_per_join": "256"
          }
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "index",
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "rows_examined_per_scan": 6,
          "rows_produced_per_join": 96,
          "filtered": "100.00",
          "using_index": true,
          "using_join_buffer": "hash join",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "9.60",
            "prefix_cost": "12.35",
            "data_read_per_join": "1K"
          },
          "used_columns": [
            "a"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ NO_BKA(`t1`@`select#1`) NO_BKA(`t2`@`select#1`) */ `test`.`t1`.`a` AS `a` from `test`.`t1` join `test`.`t2` join `test`.`t3`
explain select t1.a from t1 inner join t2 inner join t3;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	NULL	idx2	5	NULL	4	100.00	Using index
1	SIMPLE	t3	NULL	index	NULL	idx3	5	NULL	4	100.00	Using index; Using join buffer (hash join)
1	SIMPLE	t1	NULL	index	NULL	idx1	5	NULL	6	100.00	Using index; Using join buffer (hash join)
Warnings:
Note	1003	/* select#1 */ select /*+ NO_BKA(`t1`@`select#1`) NO_BKA(`t2`@`select#1`) */ `test`.`t1`.`a` AS `a` from `test`.`t1` join `test`.`t2` join `test`.`t3`
select t1.a from t1 inner join t2 inner join t3;
a
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
1
1
1
1
1
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
6
6
6
6
6
6
6
6
6
6
6
6
6
6
6
6
explain format=tree select t1.a from t1 inner join t2 inner join t4;
EXPLAIN
-> Nested loop inner join  (cost=16.85 rows=96)
    -> Nested loop inner join  (cost=3.25 rows=16)
        -> Index scan on t2 using idx2  (cost=0.65 rows=4)
        -> Index scan on t4 using idx4  (cost=0.35 rows=4)
    -> Index scan on t1 using idx1  (cost=0.29 rows=6)

explain format=json select t1.a from t1 inner join t2 inner join t4;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "16.85"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "index",
          "key": "idx2",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          }
        }
      },
      {
        "table": {
          "table_name": "t4",
          "access_type": "index",
          "key": "idx4",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 16,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "1.60",
            "prefix_cost": "3.25",
            "data_read_per_join": "256"
          }
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "index",
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "rows_examined_per_scan": 6,
          "rows_produced_per_join": 96,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "4.00",
            "eval_cost": "9.60",
            "prefix_cost": "16.85",
            "data_read_per_join": "1K"
          },
          "used_columns": [
            "a"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ NO_BNL(@`select#1`) */ `test`.`t1`.`a` AS `a` from `test`.`t1` join `test`.`t2` join `test`.`t4`
explain select t1.a from t1 inner join t2 inner join t4;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	NULL	idx2	5	NULL	4	100.00	Using index
1	SIMPLE	t4	NULL	index	NULL	idx4	5	NULL	4	100.00	Using index
1	SIMPLE	t1	NULL	index	NULL	idx1	5	NULL	6	100.00	Using index
Warnings:
Note	1003	/* select#1 */ select /*+ NO_BNL(@`select#1`) */ `test`.`t1`.`a` AS `a` from `test`.`t1` join `test`.`t2` join `test`.`t4`
select t1.a from t1 inner join t2 inner join t4;
a
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
1
1
1
1
1
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
2
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
3
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
4
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
5
6
6
6
6
6
6
6
6
6
6
6
6
6
6
6
6
explain format=tree select * from (select * from t1) as t1_alias;
EXPLAIN
-> Table scan on t1_alias  (cost=3.18 rows=6)
    -> Materialize  (cost=0.85 rows=6)
        -> Table scan on t1  (cost=0.85 rows=6)

explain format=json select * from (select * from t1) as t1_alias;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "3.17"
    },
    "table": {
      "table_name": "t1_alias",
      "access_type": "ALL",
      "rows_examined_per_scan": 6,
      "rows_produced_per_join": 6,
      "filtered": "100.00",
      "cost_info": {
        "read_cost": "2.58",
        "eval_cost": "0.60",
        "prefix_cost": "3.18",
        "data_read_per_join": "144"
      },
      "used_columns": [
        "a",
        "b",
        "c"
      ],
      "materialized_from_subquery": {
        "using_temporary_table": true,
        "dependent": false,
        "cacheable": true,
        "query_block": {
          "select_id": 2,
          "cost_info": {
            "query_cost": "0.85"
          },
          "table": {
            "table_name": "t1",
            "access_type": "ALL",
            "rows_examined_per_scan": 6,
            "rows_produced_per_join": 6,
            "filtered": "100.00",
            "cost_info": {
              "read_cost": "0.25",
              "eval_cost": "0.60",
              "prefix_cost": "0.85",
              "data_read_per_join": "96"
            },
            "used_columns": [
              "a",
              "b",
              "c"
            ]
          }
        }
      }
    }
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ NO_MERGE(`t1_alias`@`select#1`) */ `t1_alias`.`a` AS `a`,`t1_alias`.`b` AS `b`,`t1_alias`.`c` AS `c` from (/* select#2 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1`) `t1_alias`
explain select * from (select * from t1) as t1_alias;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	<derived2>	NULL	ALL	NULL	NULL	NULL	NULL	6	100.00	NULL
2	DERIVED	t1	NULL	ALL	NULL	NULL	NULL	NULL	6	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select /*+ NO_MERGE(`t1_alias`@`select#1`) */ `t1_alias`.`a` AS `a`,`t1_alias`.`b` AS `b`,`t1_alias`.`c` AS `c` from (/* select#2 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1`) `t1_alias`
select * from (select * from t1) as t1_alias;
a	b	c
1	2	3
2	3	4
3	2	3
4	3	4
5	2	3
6	3	4
explain format=tree select count(*) from t1 join t2 on t1.a = t2.a;
EXPLAIN
-> Aggregate: count(0)
    -> Nested loop inner join  (cost=2.05 rows=4)
        -> Filter: (t2.a is not null)  (cost=0.65 rows=4)
            -> Index scan on t2 using idx2  (cost=0.65 rows=4)
        -> Single-row index lookup on t1 using idx1 (a=t2.a)  (cost=0.28 rows=1)

explain format=json select count(*) from t1 join t2 on t1.a = t2.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.05"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "index",
          "possible_keys": [
            "idx2"
          ],
          "key": "idx2",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a"
          ],
          "attached_condition": "(`test`.`t2`.`a` is not null)"
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t2.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "2.05",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ HASH_JOIN(`t1`@`select#1`) HASH_JOIN(`t2`@`select#1`) */ count(0) AS `count(0)` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
explain select count(*) from t1 join t2 on t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	idx2	idx2	5	NULL	4	100.00	Using where; Using index
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	Using index
Warnings:
Note	1003	/* select#1 */ select /*+ HASH_JOIN(`t1`@`select#1`) HASH_JOIN(`t2`@`select#1`) */ count(0) AS `count(0)` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
select count(*) from t1 join t2 on t1.a = t2.a;
count(0)
4
explain format=tree select t1.a, avg(t1.b) from t1 join t2 on t1.a = t2.a group by t1.a;
EXPLAIN
-> Table scan on <temporary>
    -> Aggregate using temporary table
        -> Nested loop inner join  (cost=2.05 rows=4)
            -> Filter: (t2.a is not null)  (cost=0.65 rows=4)
                -> Index scan on t2 using idx2  (cost=0.65 rows=4)
            -> Single-row index lookup on t1 using idx1 (a=t2.a)  (cost=0.28 rows=1)

explain format=json select t1.a, avg(t1.b) from t1 join t2 on t1.a = t2.a group by t1.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.05"
    },
    "grouping_operation": {
      "using_temporary_table": true,
      "using_filesort": false,
      "nested_loop": [
        {
          "table": {
            "table_name": "t2",
            "access_type": "index",
            "possible_keys": [
              "idx2"
            ],
            "key": "idx2",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "rows_examined_per_scan": 4,
            "rows_produced_per_join": 4,
            "filtered": "100.00",
            "using_index": true,
            "cost_info": {
              "read_cost": "0.25",
              "eval_cost": "0.40",
              "prefix_cost": "0.65",
              "data_read_per_join": "64"
            },
            "used_columns": [
              "a"
            ],
            "attached_condition": "(`test`.`t2`.`a` is not null)"
          }
        },
        {
          "table": {
            "table_name": "t1",
            "access_type": "eq_ref",
            "possible_keys": [
              "idx1"
            ],
            "key": "idx1",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "ref": [
              "test.t2.a"
            ],
            "rows_examined_per_scan": 1,
            "rows_produced_per_join": 4,
            "filtered": "100.00",
            "cost_info": {
              "read_cost": "1.00",
              "eval_cost": "0.40",
              "prefix_cost": "2.05",
              "data_read_per_join": "64"
            },
            "used_columns": [
              "a",
              "b"
            ]
          }
        }
      ]
    }
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ NO_HASH_JOIN(`t1`@`select#1`) NO_HASH_JOIN(`t2`@`select#1`) */ `test`.`t1`.`a` AS `a`,avg(`test`.`t1`.`b`) AS `avg(``t1``.``b``)` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`) group by `test`.`t1`.`a`
explain select t1.a, avg(t1.b) from t1 join t2 on t1.a = t2.a group by t1.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	idx2	idx2	5	NULL	4	100.00	Using where; Using index; Using temporary
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select /*+ NO_HASH_JOIN(`t1`@`select#1`) NO_HASH_JOIN(`t2`@`select#1`) */ `test`.`t1`.`a` AS `a`,avg(`test`.`t1`.`b`) AS `avg(``t1``.``b``)` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`) group by `test`.`t1`.`a`
select t1.a, avg(t1.b) from t1 join t2 on t1.a = t2.a group by t1.a;
a	avg(`t1`.`b`)
1	2.0000
2	3.0000
3	2.0000
4	3.0000
explain format=tree select a from t1 where t1.a in (select t1.b from t1 join t2 on t1.a = t2.a);
EXPLAIN
-> Nested loop inner join  (cost=3.85 rows=24)
    -> Filter: (t1.a is not null)  (cost=0.85 rows=6)
        -> Index scan on t1 using idx1  (cost=0.85 rows=6)
    -> Single-row index lookup on <subquery2> using <auto_distinct_key> (b=t1.a)
        -> Materialize with deduplication  (cost=2.05 rows=4)
            -> Filter: (t1.b is not null)  (cost=2.05 rows=4)
                -> Nested loop inner join  (cost=2.05 rows=4)
                    -> Filter: (t2.a is not null)  (cost=0.65 rows=4)
                        -> Index scan on t2 using idx2  (cost=0.65 rows=4)
                    -> Single-row index lookup on t1 using idx1 (a=t2.a)  (cost=0.28 rows=1)

explain format=json select a from t1 where t1.a in (select t1.b from t1 join t2 on t1.a = t2.a);
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "4.90"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t1",
          "access_type": "index",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "rows_examined_per_scan": 6,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.60",
            "prefix_cost": "0.85",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a"
          ],
          "attached_condition": "(`test`.`t1`.`a` is not null)"
        }
      },
      {
        "table": {
          "table_name": "<subquery2>",
          "access_type": "eq_ref",
          "key": "<auto_distinct_key>",
          "key_length": "5",
          "ref": [
            "test.t1.a"
          ],
          "rows_examined_per_scan": 1,
          "materialized_from_subquery": {
            "using_temporary_table": true,
            "query_block": {
              "nested_loop": [
                {
                  "table": {
                    "table_name": "t2",
                    "access_type": "index",
                    "possible_keys": [
                      "idx2"
                    ],
                    "key": "idx2",
                    "used_key_parts": [
                      "a"
                    ],
                    "key_length": "5",
                    "rows_examined_per_scan": 4,
                    "rows_produced_per_join": 4,
                    "filtered": "100.00",
                    "using_index": true,
                    "cost_info": {
                      "read_cost": "0.25",
                      "eval_cost": "0.40",
                      "prefix_cost": "0.65",
                      "data_read_per_join": "64"
                    },
                    "used_columns": [
                      "a"
                    ],
                    "attached_condition": "(`test`.`t2`.`a` is not null)"
                  }
                },
                {
                  "table": {
                    "table_name": "t1",
                    "access_type": "eq_ref",
                    "possible_keys": [
                      "idx1"
                    ],
                    "key": "idx1",
                    "used_key_parts": [
                      "a"
                    ],
                    "key_length": "5",
                    "ref": [
                      "test.t2.a"
                    ],
                    "rows_examined_per_scan": 1,
                    "rows_produced_per_join": 4,
                    "filtered": "100.00",
                    "cost_info": {
                      "read_cost": "1.00",
                      "eval_cost": "0.40",
                      "prefix_cost": "2.05",
                      "data_read_per_join": "64"
                    },
                    "used_columns": [
                      "a",
                      "b"
                    ]
                  }
                }
              ]
            }
          }
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ HASH_JOIN(`t1`@`qb2`) HASH_JOIN(`t2`@`qb2`) */ `test`.`t1`.`a` AS `a` from `test`.`t1` semi join (`test`.`t1` join `test`.`t2`) where ((`test`.`t1`.`a` = `test`.`t2`.`a`) and (`<subquery2>`.`b` = `test`.`t1`.`a`))
explain select a from t1 where t1.a in (select t1.b from t1 join t2 on t1.a = t2.a);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	index	idx1	idx1	5	NULL	6	100.00	Using where; Using index
1	SIMPLE	<subquery2>	NULL	eq_ref	<auto_distinct_key>	<auto_distinct_key>	5	test.t1.a	1	100.00	NULL
2	MATERIALIZED	t2	NULL	index	idx2	idx2	5	NULL	4	100.00	Using where; Using index
2	MATERIALIZED	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select /*+ HASH_JOIN(`t1`@`qb2`) HASH_JOIN(`t2`@`qb2`) */ `test`.`t1`.`a` AS `a` from `test`.`t1` semi join (`test`.`t1` join `test`.`t2`) where ((`test`.`t1`.`a` = `test`.`t2`.`a`) and (`<subquery2>`.`b` = `test`.`t1`.`a`))
select a from t1 where t1.a in (select t1.b from t1 join t2 on t1.a = t2.a);
a
2
3
select * from t1, t3, t4, t2 where t1.a = t4.a and t2.a = t3.a and t1.a = t2.a;
a	b	c	a	b	c	a	b	c	a	b	c
1	2	3	1	2	3	1	2	3	1	2	3
2	3	4	2	3	4	2	3	4	2	3	4
truncate table mysql.outline;
outline flush;
set global cdb_opt_outline_enabled = off;
set session cdb_opt_outline_enabled = on;
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "INDEX:1#test#t1#idx1#1#0";
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "INDEX:1#test#t2#idx2#1#0";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)
show cdb_outline_info;
origin	outline	hit
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "OUTLINE:select * from t1, t2 use index(idx2) where t1.a = t2.a";
ERROR HY000: SET outline_info failed, reason: insert failed, duplicated record
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "OUTLINE:select * from t1 left join t2 use index(idx2) on t1.a = t2.a";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)
2	521fdf4654908ef638087a1a1e564df1e3387aa08bd0b8d6bdf2e7046cb4576a	select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX:1#test#t1#idx1#1#0";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)
2	521fdf4654908ef638087a1a1e564df1e3387aa08bd0b8d6bdf2e7046cb4576a	select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
outline reset "select * from t1 left join t2 on t1.a = t2.b";
ERROR HY000: Can't find record in 'outline'
outline "select * from t1 where t1.a in (select t1.a from t1,t2 where t1.a = t2.a)" set outline_info "INDEX:2#test#t1#idx1#1#0";
outline "select * from t1 where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a)" set outline_info "INDEX:1#test#t1#idx1#1#0";
outline "select * from t1 left join t4 on t1.a=t4.a where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a)" set outline_info "INDEX:2#test#t1#idx1#1#0";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)
2	521fdf4654908ef638087a1a1e564df1e3387aa08bd0b8d6bdf2e7046cb4576a	select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))
3	b39094e748b69670b2f87d6a2a58f44c45550f5385b64ab46f6d6b07715ab75a	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`))
4	9d653647ac1a0c5c5c0741cab3cf8b564b56b42671faf10900da6aa953b09b30	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))
5	4d18fb8b5d5c1d422c0b722353fb9914ac1f51bf066f3acf45f66bb1c8be5d83	select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	0
outline "select * from t1 left join t4 on t1.a=t4.a where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a)" set outline_info "INDEX:2#test#t1#idx1#1#0";
ERROR HY000: SET outline_info failed, reason: add index hints info failed
outline "select * from t1 left join t4 on t1.a=t4.a where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a)" set outline_info "INDEX:1#test#t1#idx1#1#0";
outline "select * from t1 left join t2 on t1.a = t2.a right join t3 on t1.a = t3.a" set outline_info "INDEX:1#test#t3#idx3#1#0";
outline "select * from t1,t2,t3 left join t4 on t3.a = t4.a, t5 left join (select t1.a from t1 left join t2 on t1.a = t2.a) as v on t5.a = v.a" set outline_info "INDEX:2#test#t1#idx1#1#0";
outline "select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select t1.a from t1 left join t2 on t1.a = t2.a))" set outline_info "INDEX:1#test#t1#idx1#1#0";
outline "select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select t1.a from t1 left join t2 on t1.a = t2.a))" set outline_info "INDEX:2#test#t1#idx1#2#0";
outline "select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select t1.a from t1 left join t2 on t1.a = t2.a))" set outline_info "INDEX:3#test#t1#idx1#1#0";
outline "select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select t1.a from t1 left join t2 on t1.a = t2.a))" set outline_info "INDEX:3#test#t1#idx1#1#0";
ERROR HY000: SET outline_info failed, reason: add index hints info failed
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)
2	521fdf4654908ef638087a1a1e564df1e3387aa08bd0b8d6bdf2e7046cb4576a	select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))
3	b39094e748b69670b2f87d6a2a58f44c45550f5385b64ab46f6d6b07715ab75a	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`))
4	9d653647ac1a0c5c5c0741cab3cf8b564b56b42671faf10900da6aa953b09b30	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))
5	4d18fb8b5d5c1d422c0b722353fb9914ac1f51bf066f3acf45f66bb1c8be5d83	select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`)))))
6	1528400a6d748c446a2677534948c87eb61be956c6c351fdf74c8336d9c7ed95	select * from (`test`.`t3` left join (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) on((`t1`.`a` = `t3`.`a`)))	select * from (`test`.`t3` USE INDEX FOR JOIN (`idx3`) left join (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) on((`t1`.`a` = `t3`.`a`)))
7	00839a77e242fa1163a11b7f69882352c664fd6854f5b8b75e254cb9995c418f	select * from `test`.`t1` join `test`.`t2` join (`test`.`t3` left join `test`.`t4` on((`t3`.`a` = `t4`.`a`))) join (`test`.`t5` left join (select `t1`.`a` from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))) `v` on((`t5`.`a` = `v`.`a`)))	select * from `test`.`t1` join `test`.`t2` join (`test`.`t3` left join `test`.`t4` on((`t3`.`a` = `t4`.`a`))) join (`test`.`t5` left join (select `t1`.`a` from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))) `v` on((`t5`.`a` = `v`.`a`)))
8	5a14471d6720f3c5da3e400f81c15fbe67a9fab1bfed343a54e79962ec338b33	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` where `t1`.`b` in (select `t1`.`a` from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` FORCE INDEX FOR JOIN (`idx1`) where `t1`.`b` in (select `t1`.`a` from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))))
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`)))))	0
select * from (`test`.`t3` left join (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) on((`t1`.`a` = `t3`.`a`)))	select * from (`test`.`t3` USE INDEX FOR JOIN (`idx3`) left join (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) on((`t1`.`a` = `t3`.`a`)))	0
select * from `test`.`t1` join `test`.`t2` join (`test`.`t3` left join `test`.`t4` on((`t3`.`a` = `t4`.`a`))) join (`test`.`t5` left join (select `t1`.`a` from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))) `v` on((`t5`.`a` = `v`.`a`)))	select * from `test`.`t1` join `test`.`t2` join (`test`.`t3` left join `test`.`t4` on((`t3`.`a` = `t4`.`a`))) join (`test`.`t5` left join (select `t1`.`a` from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))) `v` on((`t5`.`a` = `v`.`a`)))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` where `t1`.`b` in (select `t1`.`a` from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` FORCE INDEX FOR JOIN (`idx1`) where `t1`.`b` in (select `t1`.`a` from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))))	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` USE INDEX (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`)))))	0
select * from (`test`.`t3` left join (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) on((`t1`.`a` = `t3`.`a`)))	select * from (`test`.`t3` USE INDEX FOR JOIN (`idx3`) left join (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) on((`t1`.`a` = `t3`.`a`)))	0
select * from `test`.`t1` join `test`.`t2` join (`test`.`t3` left join `test`.`t4` on((`t3`.`a` = `t4`.`a`))) join (`test`.`t5` left join (select `t1`.`a` from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))) `v` on((`t5`.`a` = `v`.`a`)))	select * from `test`.`t1` join `test`.`t2` join (`test`.`t3` left join `test`.`t4` on((`t3`.`a` = `t4`.`a`))) join (`test`.`t5` left join (select `t1`.`a` from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))) `v` on((`t5`.`a` = `v`.`a`)))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` USE INDEX FOR JOIN (`idx2`)) where (`t1`.`a` = `t2`.`a`)	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`))	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` where `t1`.`b` in (select `t1`.`a` from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` FORCE INDEX FOR JOIN (`idx1`) where `t1`.`b` in (select `t1`.`a` from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))))	0
truncate table mysql.outline;
outline flush;
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "  INDEX:1#test#t1#idx1#1#0";
ERROR HY000: SET outline_info failed, reason: syntax error
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX+1#test#t1#idx1#1#0";
ERROR HY000: SET outline_info failed, reason: syntax error
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX:1#test#t1#idx1#0";
ERROR HY000: SET outline_info failed, reason: parse index hints info failed
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX:1#test#idx1#1#0";
ERROR HY000: SET outline_info failed, reason: parse index hints info failed
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX:1#test#test#idx1#3#0";
ERROR HY000: SET outline_info failed, reason: parse index hints info failed, index type not right.
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX:1#test#t1#idx1#idx2#1#0";
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX:1#test#t2#idx2#4#0";
ERROR HY000: SET outline_info failed, reason: parse index hints info failed, index type not right.
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX:1#test#t2#idx2#999#0";
ERROR HY000: SET outline_info failed, reason: parse index hints info failed, index type not right.
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX   :1#test#t1#idx1#1#0";
ERROR HY000: SET outline_info failed, reason: add index hints info failed
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "INDEX   :1#test# t2  # idx2 # 1#0";
outline "select * from t3, t2 where t3.a = t2.a" set outline_info "INDEX   :1#test# t3  # idx3 # 2 #0";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	521fdf4654908ef638087a1a1e564df1e3387aa08bd0b8d6bdf2e7046cb4576a	select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` IGNORE INDEX FOR GROUP BY (`idx1`) left join `test`.`t2` USE INDEX FOR JOIN (`idx2`) on((`t1`.`a` = `t2`.`a`)))
2	10eb1fc037b0856d5af0802b5deb35b85b5698e4152a32ba7f7c42b79c091f62	select * from `test`.`t3` join `test`.`t2` where (`t3`.`a` = `t2`.`a`)	select * from `test`.`t3` FORCE INDEX FOR JOIN (`idx3`) join `test`.`t2` where (`t3`.`a` = `t2`.`a`)
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` IGNORE INDEX FOR GROUP BY (`idx1`) left join `test`.`t2` USE INDEX FOR JOIN (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from `test`.`t3` join `test`.`t2` where (`t3`.`a` = `t2`.`a`)	select * from `test`.`t3` FORCE INDEX FOR JOIN (`idx3`) join `test`.`t2` where (`t3`.`a` = `t2`.`a`)	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from (`test`.`t1` left join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select * from (`test`.`t1` IGNORE INDEX FOR GROUP BY (`idx1`) left join `test`.`t2` USE INDEX FOR JOIN (`idx2`) on((`t1`.`a` = `t2`.`a`)))	0
select * from `test`.`t3` join `test`.`t2` where (`t3`.`a` = `t2`.`a`)	select * from `test`.`t3` FORCE INDEX FOR JOIN (`idx3`) join `test`.`t2` where (`t3`.`a` = `t2`.`a`)	0
explain format=tree select * from t1,t2 where t1.a = t2.a;
EXPLAIN
-> Nested loop inner join  (cost=2.05 rows=4)
    -> Filter: (t2.a is not null)  (cost=0.65 rows=4)
        -> Table scan on t2  (cost=0.65 rows=4)
    -> Single-row index lookup on t1 using idx1 (a=t2.a)  (cost=0.28 rows=1)

explain format=json select * from t1,t2 where t1.a = t2.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.05"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "ALL",
          "possible_keys": [
            "idx2"
          ],
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ],
          "attached_condition": "(`test`.`t2`.`a` is not null)"
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t2.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "2.05",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
explain select * from t1, t2 where t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	idx2	NULL	NULL	NULL	4	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` join `test`.`t2` where (`test`.`t1`.`a` = `test`.`t2`.`a`)
select * from t1, t2 where t1.a = t2.a;
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	2	3	3	2	3
4	3	4	4	3	4
explain format=tree select * from t1 left join t2 on t1.a = t2.a;
EXPLAIN
-> Nested loop left join  (cost=2.95 rows=6)
    -> Table scan on t1  (cost=0.85 rows=6)
    -> Single-row index lookup on t2 using idx2 (a=t1.a)  (cost=0.27 rows=1)

explain format=json select * from t1 left join t2 on t1.a = t2.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.95"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t1",
          "access_type": "ALL",
          "rows_examined_per_scan": 6,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.60",
            "prefix_cost": "0.85",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t2",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx2"
          ],
          "key": "idx2",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t1.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.50",
            "eval_cost": "0.60",
            "prefix_cost": "2.95",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` IGNORE INDEX FOR GROUP BY (`idx1`) left join `test`.`t2` USE INDEX FOR JOIN (`idx2`) on((`test`.`t2`.`a` = `test`.`t1`.`a`)) where true
explain select * from t1 left join t2 on t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	6	100.00	NULL
1	SIMPLE	t2	NULL	eq_ref	idx2	idx2	5	test.t1.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` IGNORE INDEX FOR GROUP BY (`idx1`) left join `test`.`t2` USE INDEX FOR JOIN (`idx2`) on((`test`.`t2`.`a` = `test`.`t1`.`a`)) where true
select * from t1 left join t2 on t1.a = t2.a;
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	2	3	3	2	3
4	3	4	4	3	4
5	2	3	NULL	NULL	NULL
6	3	4	NULL	NULL	NULL
explain format=tree select * from t1 where t1.a in (select t1.a from t1,t2 where t1.a = t2.a);
EXPLAIN
-> Nested loop inner join  (cost=3.45 rows=4)
    -> Nested loop inner join  (cost=2.05 rows=4)
        -> Filter: (t2.a is not null)  (cost=0.65 rows=4)
            -> Index scan on t2 using idx2  (cost=0.65 rows=4)
        -> Single-row index lookup on t1 using idx1 (a=t2.a)  (cost=0.28 rows=1)
    -> Single-row index lookup on t1 using idx1 (a=t2.a)  (cost=0.28 rows=1)

explain format=json select * from t1 where t1.a in (select t1.a from t1,t2 where t1.a = t2.a);
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "3.45"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "index",
          "possible_keys": [
            "idx2"
          ],
          "key": "idx2",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a"
          ],
          "attached_condition": "(`test`.`t2`.`a` is not null)"
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t2.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "2.05",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t2.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "3.45",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` join `test`.`t2` join `test`.`t1` where ((`test`.`t1`.`a` = `test`.`t2`.`a`) and (`test`.`t1`.`a` = `test`.`t2`.`a`))
explain select * from t1 where t1.a in (select t1.a from t1,t2 where t1.a = t2.a);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	idx2	idx2	5	NULL	4	100.00	Using where; Using index
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	NULL
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t2.a	1	100.00	Using index
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` join `test`.`t2` join `test`.`t1` where ((`test`.`t1`.`a` = `test`.`t2`.`a`) and (`test`.`t1`.`a` = `test`.`t2`.`a`))
select * from t1 where t1.a in (select t1.a from t1,t2 where t1.a = t2.a);
a	b	c
1	2	3
2	3	4
3	2	3
4	3	4
explain format=tree select * from t1 where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a);
EXPLAIN
-> Remove duplicate t1 rows using temporary table (weedout)  (cost=9.96 rows=96)
    -> Left hash join (no condition), extra conditions: (t1.a = t2.a)  (cost=9.96 rows=96)
        -> Nested loop inner join  (cost=7.20 rows=24)
            -> Inner hash join (no condition)  (cost=3.30 rows=24)
                -> Index range scan on t1 using idx1, with index condition: (t1.a is not null)  (cost=0.21 rows=6)
                -> Hash
                    -> Index scan on t2 using idx2  (cost=0.65 rows=4)
            -> Single-row index lookup on t1 using idx1 (a=t1.a)  (cost=0.07 rows=1)
        -> Hash
            -> Index scan on t3 using idx3  (cost=0.11 rows=4)

Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
explain format=json select * from t1 where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a);
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "30.05"
    },
    "duplicates_removal": {
      "using_temporary_table": true,
      "nested_loop": [
        {
          "table": {
            "table_name": "t2",
            "access_type": "index",
            "key": "idx2",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "rows_examined_per_scan": 4,
            "rows_produced_per_join": 4,
            "filtered": "100.00",
            "using_index": true,
            "cost_info": {
              "read_cost": "0.25",
              "eval_cost": "0.40",
              "prefix_cost": "0.65",
              "data_read_per_join": "64"
            },
            "used_columns": [
              "a"
            ]
          }
        },
        {
          "table": {
            "table_name": "t1",
            "access_type": "range",
            "possible_keys": [
              "idx1"
            ],
            "key": "idx1",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "rows_examined_per_scan": 6,
            "rows_produced_per_join": 24,
            "filtered": "100.00",
            "index_condition": "(`test`.`t1`.`a` is not null)",
            "using_join_buffer": "hash join",
            "cost_info": {
              "read_cost": "0.25",
              "eval_cost": "2.40",
              "prefix_cost": "3.30",
              "data_read_per_join": "384"
            },
            "used_columns": [
              "a",
              "b",
              "c"
            ]
          }
        },
        {
          "table": {
            "table_name": "t1",
            "access_type": "eq_ref",
            "possible_keys": [
              "idx1"
            ],
            "key": "idx1",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "ref": [
              "test.t1.a"
            ],
            "rows_examined_per_scan": 1,
            "rows_produced_per_join": 24,
            "filtered": "100.00",
            "using_index": true,
            "cost_info": {
              "read_cost": "1.50",
              "eval_cost": "2.40",
              "prefix_cost": "7.20",
              "data_read_per_join": "384"
            },
            "used_columns": [
              "a"
            ]
          }
        },
        {
          "table": {
            "table_name": "t3",
            "access_type": "index",
            "key": "idx3",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "rows_examined_per_scan": 4,
            "rows_produced_per_join": 24,
            "filtered": "100.00",
            "using_index": true,
            "using_join_buffer": "hash join",
            "cost_info": {
              "read_cost": "0.25",
              "eval_cost": "2.40",
              "prefix_cost": "30.05",
              "data_read_per_join": "384"
            },
            "attached_condition": "<if>(is_not_null_compl(t3), (`test`.`t1`.`a` = `test`.`t2`.`a`), true)"
          }
        }
      ]
    }
  }
}
Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` join `test`.`t1` semi join (`test`.`t2` left join `test`.`t3` on((`test`.`t1`.`a` = `test`.`t2`.`a`))) where (`test`.`t1`.`a` = `test`.`t1`.`a`)
explain select * from t1 where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	NULL	idx2	5	NULL	4	100.00	Using index; Start temporary
1	SIMPLE	t1	NULL	range	idx1	idx1	5	NULL	6	100.00	Using index condition; Using join buffer (hash join)
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t1.a	1	100.00	Using index
1	SIMPLE	t3	NULL	index	NULL	idx3	5	NULL	4	100.00	Using where; Using index; End temporary; Using join buffer (hash join)
Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` join `test`.`t1` semi join (`test`.`t2` left join `test`.`t3` on((`test`.`t1`.`a` = `test`.`t2`.`a`))) where (`test`.`t1`.`a` = `test`.`t1`.`a`)
select * from t1 where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a);
a	b	c
1	2	3
2	3	4
3	2	3
4	3	4
5	2	3
6	3	4
explain format=tree select * from t1 left join t4 on t1.a=t4.a where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a);
EXPLAIN
-> Remove duplicate t1 rows using temporary table (weedout)  (cost=9.96 rows=96)
    -> Left hash join (no condition), extra conditions: (t1.a = t2.a)  (cost=9.96 rows=96)
        -> Nested loop inner join  (cost=11.10 rows=24)
            -> Nested loop left join  (cost=7.20 rows=24)
                -> Inner hash join (no condition)  (cost=3.30 rows=24)
                    -> Index range scan on t1 using idx1, with index condition: (t1.a is not null)  (cost=0.21 rows=6)
                    -> Hash
                        -> Index scan on t2 using idx2  (cost=0.65 rows=4)
                -> Single-row index lookup on t4 using idx4 (a=t1.a)  (cost=0.07 rows=1)
            -> Single-row index lookup on t1 using idx1 (a=t1.a)  (cost=0.07 rows=1)
        -> Hash
            -> Index scan on t3 using idx3  (cost=0.11 rows=4)

Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
explain format=json select * from t1 left join t4 on t1.a=t4.a where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a);
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "33.95"
    },
    "duplicates_removal": {
      "using_temporary_table": true,
      "nested_loop": [
        {
          "table": {
            "table_name": "t2",
            "access_type": "index",
            "key": "idx2",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "rows_examined_per_scan": 4,
            "rows_produced_per_join": 4,
            "filtered": "100.00",
            "using_index": true,
            "cost_info": {
              "read_cost": "0.25",
              "eval_cost": "0.40",
              "prefix_cost": "0.65",
              "data_read_per_join": "64"
            },
            "used_columns": [
              "a"
            ]
          }
        },
        {
          "table": {
            "table_name": "t1",
            "access_type": "range",
            "possible_keys": [
              "idx1"
            ],
            "key": "idx1",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "rows_examined_per_scan": 6,
            "rows_produced_per_join": 24,
            "filtered": "100.00",
            "index_condition": "(`test`.`t1`.`a` is not null)",
            "using_join_buffer": "hash join",
            "cost_info": {
              "read_cost": "0.25",
              "eval_cost": "2.40",
              "prefix_cost": "3.30",
              "data_read_per_join": "384"
            },
            "used_columns": [
              "a",
              "b",
              "c"
            ]
          }
        },
        {
          "table": {
            "table_name": "t4",
            "access_type": "eq_ref",
            "possible_keys": [
              "idx4"
            ],
            "key": "idx4",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "ref": [
              "test.t1.a"
            ],
            "rows_examined_per_scan": 1,
            "rows_produced_per_join": 24,
            "filtered": "100.00",
            "cost_info": {
              "read_cost": "1.50",
              "eval_cost": "2.40",
              "prefix_cost": "7.20",
              "data_read_per_join": "384"
            },
            "used_columns": [
              "a",
              "b",
              "c"
            ]
          }
        },
        {
          "table": {
            "table_name": "t1",
            "access_type": "eq_ref",
            "possible_keys": [
              "idx1"
            ],
            "key": "idx1",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "ref": [
              "test.t1.a"
            ],
            "rows_examined_per_scan": 1,
            "rows_produced_per_join": 24,
            "filtered": "100.00",
            "using_index": true,
            "cost_info": {
              "read_cost": "1.50",
              "eval_cost": "2.40",
              "prefix_cost": "11.10",
              "data_read_per_join": "384"
            },
            "used_columns": [
              "a"
            ]
          }
        },
        {
          "table": {
            "table_name": "t3",
            "access_type": "index",
            "key": "idx3",
            "used_key_parts": [
              "a"
            ],
            "key_length": "5",
            "rows_examined_per_scan": 4,
            "rows_produced_per_join": 24,
            "filtered": "100.00",
            "using_index": true,
            "using_join_buffer": "hash join",
            "cost_info": {
              "read_cost": "0.25",
              "eval_cost": "2.40",
              "prefix_cost": "33.95",
              "data_read_per_join": "384"
            },
            "attached_condition": "<if>(is_not_null_compl(t3), (`test`.`t1`.`a` = `test`.`t2`.`a`), true)"
          }
        }
      ]
    }
  }
}
Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t4`.`a` AS `a`,`test`.`t4`.`b` AS `b`,`test`.`t4`.`c` AS `c` from `test`.`t1` join `test`.`t1` left join `test`.`t4` on((`test`.`t4`.`a` = `test`.`t1`.`a`)) semi join (`test`.`t2` left join `test`.`t3` on((`test`.`t1`.`a` = `test`.`t2`.`a`))) where (`test`.`t1`.`a` = `test`.`t1`.`a`)
explain select * from t1 left join t4 on t1.a=t4.a where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	index	NULL	idx2	5	NULL	4	100.00	Using index; Start temporary
1	SIMPLE	t1	NULL	range	idx1	idx1	5	NULL	6	100.00	Using index condition; Using join buffer (hash join)
1	SIMPLE	t4	NULL	eq_ref	idx4	idx4	5	test.t1.a	1	100.00	NULL
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t1.a	1	100.00	Using index
1	SIMPLE	t3	NULL	index	NULL	idx3	5	NULL	4	100.00	Using where; Using index; End temporary; Using join buffer (hash join)
Warnings:
Note	1276	Field or reference 'test.t1.a' of SELECT #2 was resolved in SELECT #1
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t4`.`a` AS `a`,`test`.`t4`.`b` AS `b`,`test`.`t4`.`c` AS `c` from `test`.`t1` join `test`.`t1` left join `test`.`t4` on((`test`.`t4`.`a` = `test`.`t1`.`a`)) semi join (`test`.`t2` left join `test`.`t3` on((`test`.`t1`.`a` = `test`.`t2`.`a`))) where (`test`.`t1`.`a` = `test`.`t1`.`a`)
select * from t1 left join t4 on t1.a=t4.a where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a);
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	2	3	NULL	NULL	NULL
4	3	4	NULL	NULL	NULL
5	2	3	5	3	4
6	3	4	6	2	3
explain format=tree select * from t1 left join t2 on t1.a = t2.a right join t3 on t1.a = t3.a;
EXPLAIN
-> Nested loop left join  (cost=2.05 rows=4)
    -> Table scan on t3  (cost=0.65 rows=4)
    -> Nested loop left join  (cost=1.38 rows=1)
        -> Single-row index lookup on t1 using idx1 (a=t3.a)  (cost=0.28 rows=1)
        -> Filter: (t2.a = t3.a)  (cost=0.28 rows=1)
            -> Single-row index lookup on t2 using idx2 (a=t1.a)  (cost=0.28 rows=1)

explain format=json select * from t1 left join t2 on t1.a = t2.a right join t3 on t1.a = t3.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "3.45"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t3",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t3.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "2.05",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t2",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx2"
          ],
          "key": "idx2",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t1.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "3.45",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ],
          "attached_condition": "<if>(is_not_null_compl(t2), (`test`.`t2`.`a` = `test`.`t3`.`a`), true)"
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c` from `test`.`t3` left join (`test`.`t1` left join `test`.`t2` on((`test`.`t2`.`a` = `test`.`t3`.`a`))) on((`test`.`t1`.`a` = `test`.`t3`.`a`)) where true
explain select * from t1 left join t2 on t1.a = t2.a right join t3 on t1.a = t3.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t3	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	NULL
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t3.a	1	100.00	NULL
1	SIMPLE	t2	NULL	eq_ref	idx2	idx2	5	test.t1.a	1	100.00	Using where
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c` from `test`.`t3` left join (`test`.`t1` left join `test`.`t2` on((`test`.`t2`.`a` = `test`.`t3`.`a`))) on((`test`.`t1`.`a` = `test`.`t3`.`a`)) where true
select * from t1 left join t2 on t1.a = t2.a right join t3 on t1.a = t3.a;
a	b	c	a	b	c	a	b	c
1	2	3	1	2	3	1	2	3
2	3	4	2	3	4	2	3	4
3	2	3	3	2	3	3	3	4
5	2	3	NULL	NULL	NULL	5	2	3
explain format=tree select * from t1,t2,t3 left join t4 on t3.a = t4.a, t5 left join (select t1.a from t1 left join t2 on t1.a = t2.a) as v on t5.a = v.a;
EXPLAIN
-> Inner hash join (no condition)  (cost=57.80 rows=384)
    -> Table scan on t1  (cost=0.01 rows=6)
    -> Hash
        -> Nested loop left join  (cost=19.15 rows=64)
            -> Inner hash join (no condition)  (cost=11.75 rows=64)
                -> Table scan on t5  (cost=0.04 rows=4)
                -> Hash
                    -> Nested loop left join  (cost=5.10 rows=16)
                        -> Inner hash join (no condition)  (cost=2.50 rows=16)
                            -> Table scan on t3  (cost=0.16 rows=4)
                            -> Hash
                                -> Table scan on t2  (cost=0.65 rows=4)
                        -> Single-row index lookup on t4 using idx4 (a=t3.a)  (cost=0.07 rows=1)
            -> Nested loop left join  (cost=1.12 rows=1)
                -> Single-row index lookup on t1 using idx1 (a=t5.a)  (cost=0.02 rows=1)
                -> Single-row index lookup on t2 using idx2 (a=t5.a)  (cost=0.02 rows=1)

explain format=json select * from t1,t2,t3 left join t4 on t3.a = t4.a, t5 left join (select t1.a from t1 left join t2 on t1.a = t2.a) as v on t5.a = v.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "65.20"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t3",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 16,
          "filtered": "100.00",
          "using_join_buffer": "hash join",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "1.60",
            "prefix_cost": "2.50",
            "data_read_per_join": "256"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t4",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx4"
          ],
          "key": "idx4",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t3.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 16,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "1.60",
            "prefix_cost": "5.10",
            "data_read_per_join": "256"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t5",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 64,
          "filtered": "100.00",
          "using_join_buffer": "hash join",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "6.40",
            "prefix_cost": "11.75",
            "data_read_per_join": "1024"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t5.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 64,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "6.40",
            "prefix_cost": "19.15",
            "data_read_per_join": "1024"
          },
          "used_columns": [
            "a"
          ]
        }
      },
      {
        "table": {
          "table_name": "t2",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx2"
          ],
          "key": "idx2",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t5.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 64,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "6.40",
            "prefix_cost": "26.55",
            "data_read_per_join": "1024"
          },
          "used_columns": [
            "a"
          ]
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "ALL",
          "rows_examined_per_scan": 6,
          "rows_produced_per_join": 384,
          "filtered": "100.00",
          "using_join_buffer": "hash join",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "38.40",
            "prefix_cost": "65.20",
            "data_read_per_join": "6K"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t4`.`a` AS `a`,`test`.`t4`.`b` AS `b`,`test`.`t4`.`c` AS `c`,`test`.`t5`.`a` AS `a`,`test`.`t5`.`b` AS `b`,`test`.`t5`.`c` AS `c`,`test`.`t1`.`a` AS `a` from `test`.`t1` join `test`.`t2` join `test`.`t3` left join `test`.`t4` on((`test`.`t4`.`a` = `test`.`t3`.`a`)) join `test`.`t5` left join (`test`.`t1` left join `test`.`t2` on((`test`.`t2`.`a` = `test`.`t5`.`a`))) on((`test`.`t1`.`a` = `test`.`t5`.`a`)) where true
explain select * from t1,t2,t3 left join t4 on t3.a = t4.a, t5 left join (select t1.a from t1 left join t2 on t1.a = t2.a) as v on t5.a = v.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	NULL
1	SIMPLE	t3	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using join buffer (hash join)
1	SIMPLE	t4	NULL	eq_ref	idx4	idx4	5	test.t3.a	1	100.00	NULL
1	SIMPLE	t5	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using join buffer (hash join)
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t5.a	1	100.00	Using index
1	SIMPLE	t2	NULL	eq_ref	idx2	idx2	5	test.t5.a	1	100.00	Using index
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	6	100.00	Using join buffer (hash join)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t4`.`a` AS `a`,`test`.`t4`.`b` AS `b`,`test`.`t4`.`c` AS `c`,`test`.`t5`.`a` AS `a`,`test`.`t5`.`b` AS `b`,`test`.`t5`.`c` AS `c`,`test`.`t1`.`a` AS `a` from `test`.`t1` join `test`.`t2` join `test`.`t3` left join `test`.`t4` on((`test`.`t4`.`a` = `test`.`t3`.`a`)) join `test`.`t5` left join (`test`.`t1` left join `test`.`t2` on((`test`.`t2`.`a` = `test`.`t5`.`a`))) on((`test`.`t1`.`a` = `test`.`t5`.`a`)) where true
select * from t1,t2,t3 left join t4 on t3.a = t4.a, t5 left join (select t1.a from t1 left join t2 on t1.a = t2.a) as v on t5.a = v.a;
a	b	c	a	b	c	a	b	c	a	b	c	a	b	c	a
1	2	3	1	2	3	1	2	3	1	2	3	1	2	3	1
1	2	3	1	2	3	1	2	3	1	2	3	2	3	4	2
1	2	3	1	2	3	1	2	3	1	2	3	6	2	3	6
1	2	3	1	2	3	1	2	3	1	2	3	7	3	4	NULL
1	2	3	1	2	3	2	3	4	2	3	4	1	2	3	1
1	2	3	1	2	3	2	3	4	2	3	4	2	3	4	2
1	2	3	1	2	3	2	3	4	2	3	4	6	2	3	6
1	2	3	1	2	3	2	3	4	2	3	4	7	3	4	NULL
1	2	3	1	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
1	2	3	1	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
1	2	3	1	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
1	2	3	1	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
1	2	3	1	2	3	5	2	3	5	3	4	1	2	3	1
1	2	3	1	2	3	5	2	3	5	3	4	2	3	4	2
1	2	3	1	2	3	5	2	3	5	3	4	6	2	3	6
1	2	3	1	2	3	5	2	3	5	3	4	7	3	4	NULL
1	2	3	2	3	4	1	2	3	1	2	3	1	2	3	1
1	2	3	2	3	4	1	2	3	1	2	3	2	3	4	2
1	2	3	2	3	4	1	2	3	1	2	3	6	2	3	6
1	2	3	2	3	4	1	2	3	1	2	3	7	3	4	NULL
1	2	3	2	3	4	2	3	4	2	3	4	1	2	3	1
1	2	3	2	3	4	2	3	4	2	3	4	2	3	4	2
1	2	3	2	3	4	2	3	4	2	3	4	6	2	3	6
1	2	3	2	3	4	2	3	4	2	3	4	7	3	4	NULL
1	2	3	2	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
1	2	3	2	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
1	2	3	2	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
1	2	3	2	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
1	2	3	2	3	4	5	2	3	5	3	4	1	2	3	1
1	2	3	2	3	4	5	2	3	5	3	4	2	3	4	2
1	2	3	2	3	4	5	2	3	5	3	4	6	2	3	6
1	2	3	2	3	4	5	2	3	5	3	4	7	3	4	NULL
1	2	3	3	2	3	1	2	3	1	2	3	1	2	3	1
1	2	3	3	2	3	1	2	3	1	2	3	2	3	4	2
1	2	3	3	2	3	1	2	3	1	2	3	6	2	3	6
1	2	3	3	2	3	1	2	3	1	2	3	7	3	4	NULL
1	2	3	3	2	3	2	3	4	2	3	4	1	2	3	1
1	2	3	3	2	3	2	3	4	2	3	4	2	3	4	2
1	2	3	3	2	3	2	3	4	2	3	4	6	2	3	6
1	2	3	3	2	3	2	3	4	2	3	4	7	3	4	NULL
1	2	3	3	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
1	2	3	3	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
1	2	3	3	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
1	2	3	3	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
1	2	3	3	2	3	5	2	3	5	3	4	1	2	3	1
1	2	3	3	2	3	5	2	3	5	3	4	2	3	4	2
1	2	3	3	2	3	5	2	3	5	3	4	6	2	3	6
1	2	3	3	2	3	5	2	3	5	3	4	7	3	4	NULL
1	2	3	4	3	4	1	2	3	1	2	3	1	2	3	1
1	2	3	4	3	4	1	2	3	1	2	3	2	3	4	2
1	2	3	4	3	4	1	2	3	1	2	3	6	2	3	6
1	2	3	4	3	4	1	2	3	1	2	3	7	3	4	NULL
1	2	3	4	3	4	2	3	4	2	3	4	1	2	3	1
1	2	3	4	3	4	2	3	4	2	3	4	2	3	4	2
1	2	3	4	3	4	2	3	4	2	3	4	6	2	3	6
1	2	3	4	3	4	2	3	4	2	3	4	7	3	4	NULL
1	2	3	4	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
1	2	3	4	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
1	2	3	4	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
1	2	3	4	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
1	2	3	4	3	4	5	2	3	5	3	4	1	2	3	1
1	2	3	4	3	4	5	2	3	5	3	4	2	3	4	2
1	2	3	4	3	4	5	2	3	5	3	4	6	2	3	6
1	2	3	4	3	4	5	2	3	5	3	4	7	3	4	NULL
2	3	4	1	2	3	1	2	3	1	2	3	1	2	3	1
2	3	4	1	2	3	1	2	3	1	2	3	2	3	4	2
2	3	4	1	2	3	1	2	3	1	2	3	6	2	3	6
2	3	4	1	2	3	1	2	3	1	2	3	7	3	4	NULL
2	3	4	1	2	3	2	3	4	2	3	4	1	2	3	1
2	3	4	1	2	3	2	3	4	2	3	4	2	3	4	2
2	3	4	1	2	3	2	3	4	2	3	4	6	2	3	6
2	3	4	1	2	3	2	3	4	2	3	4	7	3	4	NULL
2	3	4	1	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
2	3	4	1	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
2	3	4	1	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
2	3	4	1	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
2	3	4	1	2	3	5	2	3	5	3	4	1	2	3	1
2	3	4	1	2	3	5	2	3	5	3	4	2	3	4	2
2	3	4	1	2	3	5	2	3	5	3	4	6	2	3	6
2	3	4	1	2	3	5	2	3	5	3	4	7	3	4	NULL
2	3	4	2	3	4	1	2	3	1	2	3	1	2	3	1
2	3	4	2	3	4	1	2	3	1	2	3	2	3	4	2
2	3	4	2	3	4	1	2	3	1	2	3	6	2	3	6
2	3	4	2	3	4	1	2	3	1	2	3	7	3	4	NULL
2	3	4	2	3	4	2	3	4	2	3	4	1	2	3	1
2	3	4	2	3	4	2	3	4	2	3	4	2	3	4	2
2	3	4	2	3	4	2	3	4	2	3	4	6	2	3	6
2	3	4	2	3	4	2	3	4	2	3	4	7	3	4	NULL
2	3	4	2	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
2	3	4	2	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
2	3	4	2	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
2	3	4	2	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
2	3	4	2	3	4	5	2	3	5	3	4	1	2	3	1
2	3	4	2	3	4	5	2	3	5	3	4	2	3	4	2
2	3	4	2	3	4	5	2	3	5	3	4	6	2	3	6
2	3	4	2	3	4	5	2	3	5	3	4	7	3	4	NULL
2	3	4	3	2	3	1	2	3	1	2	3	1	2	3	1
2	3	4	3	2	3	1	2	3	1	2	3	2	3	4	2
2	3	4	3	2	3	1	2	3	1	2	3	6	2	3	6
2	3	4	3	2	3	1	2	3	1	2	3	7	3	4	NULL
2	3	4	3	2	3	2	3	4	2	3	4	1	2	3	1
2	3	4	3	2	3	2	3	4	2	3	4	2	3	4	2
2	3	4	3	2	3	2	3	4	2	3	4	6	2	3	6
2	3	4	3	2	3	2	3	4	2	3	4	7	3	4	NULL
2	3	4	3	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
2	3	4	3	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
2	3	4	3	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
2	3	4	3	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
2	3	4	3	2	3	5	2	3	5	3	4	1	2	3	1
2	3	4	3	2	3	5	2	3	5	3	4	2	3	4	2
2	3	4	3	2	3	5	2	3	5	3	4	6	2	3	6
2	3	4	3	2	3	5	2	3	5	3	4	7	3	4	NULL
2	3	4	4	3	4	1	2	3	1	2	3	1	2	3	1
2	3	4	4	3	4	1	2	3	1	2	3	2	3	4	2
2	3	4	4	3	4	1	2	3	1	2	3	6	2	3	6
2	3	4	4	3	4	1	2	3	1	2	3	7	3	4	NULL
2	3	4	4	3	4	2	3	4	2	3	4	1	2	3	1
2	3	4	4	3	4	2	3	4	2	3	4	2	3	4	2
2	3	4	4	3	4	2	3	4	2	3	4	6	2	3	6
2	3	4	4	3	4	2	3	4	2	3	4	7	3	4	NULL
2	3	4	4	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
2	3	4	4	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
2	3	4	4	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
2	3	4	4	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
2	3	4	4	3	4	5	2	3	5	3	4	1	2	3	1
2	3	4	4	3	4	5	2	3	5	3	4	2	3	4	2
2	3	4	4	3	4	5	2	3	5	3	4	6	2	3	6
2	3	4	4	3	4	5	2	3	5	3	4	7	3	4	NULL
3	2	3	1	2	3	1	2	3	1	2	3	1	2	3	1
3	2	3	1	2	3	1	2	3	1	2	3	2	3	4	2
3	2	3	1	2	3	1	2	3	1	2	3	6	2	3	6
3	2	3	1	2	3	1	2	3	1	2	3	7	3	4	NULL
3	2	3	1	2	3	2	3	4	2	3	4	1	2	3	1
3	2	3	1	2	3	2	3	4	2	3	4	2	3	4	2
3	2	3	1	2	3	2	3	4	2	3	4	6	2	3	6
3	2	3	1	2	3	2	3	4	2	3	4	7	3	4	NULL
3	2	3	1	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
3	2	3	1	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
3	2	3	1	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
3	2	3	1	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
3	2	3	1	2	3	5	2	3	5	3	4	1	2	3	1
3	2	3	1	2	3	5	2	3	5	3	4	2	3	4	2
3	2	3	1	2	3	5	2	3	5	3	4	6	2	3	6
3	2	3	1	2	3	5	2	3	5	3	4	7	3	4	NULL
3	2	3	2	3	4	1	2	3	1	2	3	1	2	3	1
3	2	3	2	3	4	1	2	3	1	2	3	2	3	4	2
3	2	3	2	3	4	1	2	3	1	2	3	6	2	3	6
3	2	3	2	3	4	1	2	3	1	2	3	7	3	4	NULL
3	2	3	2	3	4	2	3	4	2	3	4	1	2	3	1
3	2	3	2	3	4	2	3	4	2	3	4	2	3	4	2
3	2	3	2	3	4	2	3	4	2	3	4	6	2	3	6
3	2	3	2	3	4	2	3	4	2	3	4	7	3	4	NULL
3	2	3	2	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
3	2	3	2	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
3	2	3	2	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
3	2	3	2	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
3	2	3	2	3	4	5	2	3	5	3	4	1	2	3	1
3	2	3	2	3	4	5	2	3	5	3	4	2	3	4	2
3	2	3	2	3	4	5	2	3	5	3	4	6	2	3	6
3	2	3	2	3	4	5	2	3	5	3	4	7	3	4	NULL
3	2	3	3	2	3	1	2	3	1	2	3	1	2	3	1
3	2	3	3	2	3	1	2	3	1	2	3	2	3	4	2
3	2	3	3	2	3	1	2	3	1	2	3	6	2	3	6
3	2	3	3	2	3	1	2	3	1	2	3	7	3	4	NULL
3	2	3	3	2	3	2	3	4	2	3	4	1	2	3	1
3	2	3	3	2	3	2	3	4	2	3	4	2	3	4	2
3	2	3	3	2	3	2	3	4	2	3	4	6	2	3	6
3	2	3	3	2	3	2	3	4	2	3	4	7	3	4	NULL
3	2	3	3	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
3	2	3	3	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
3	2	3	3	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
3	2	3	3	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
3	2	3	3	2	3	5	2	3	5	3	4	1	2	3	1
3	2	3	3	2	3	5	2	3	5	3	4	2	3	4	2
3	2	3	3	2	3	5	2	3	5	3	4	6	2	3	6
3	2	3	3	2	3	5	2	3	5	3	4	7	3	4	NULL
3	2	3	4	3	4	1	2	3	1	2	3	1	2	3	1
3	2	3	4	3	4	1	2	3	1	2	3	2	3	4	2
3	2	3	4	3	4	1	2	3	1	2	3	6	2	3	6
3	2	3	4	3	4	1	2	3	1	2	3	7	3	4	NULL
3	2	3	4	3	4	2	3	4	2	3	4	1	2	3	1
3	2	3	4	3	4	2	3	4	2	3	4	2	3	4	2
3	2	3	4	3	4	2	3	4	2	3	4	6	2	3	6
3	2	3	4	3	4	2	3	4	2	3	4	7	3	4	NULL
3	2	3	4	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
3	2	3	4	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
3	2	3	4	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
3	2	3	4	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
3	2	3	4	3	4	5	2	3	5	3	4	1	2	3	1
3	2	3	4	3	4	5	2	3	5	3	4	2	3	4	2
3	2	3	4	3	4	5	2	3	5	3	4	6	2	3	6
3	2	3	4	3	4	5	2	3	5	3	4	7	3	4	NULL
4	3	4	1	2	3	1	2	3	1	2	3	1	2	3	1
4	3	4	1	2	3	1	2	3	1	2	3	2	3	4	2
4	3	4	1	2	3	1	2	3	1	2	3	6	2	3	6
4	3	4	1	2	3	1	2	3	1	2	3	7	3	4	NULL
4	3	4	1	2	3	2	3	4	2	3	4	1	2	3	1
4	3	4	1	2	3	2	3	4	2	3	4	2	3	4	2
4	3	4	1	2	3	2	3	4	2	3	4	6	2	3	6
4	3	4	1	2	3	2	3	4	2	3	4	7	3	4	NULL
4	3	4	1	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
4	3	4	1	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
4	3	4	1	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
4	3	4	1	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
4	3	4	1	2	3	5	2	3	5	3	4	1	2	3	1
4	3	4	1	2	3	5	2	3	5	3	4	2	3	4	2
4	3	4	1	2	3	5	2	3	5	3	4	6	2	3	6
4	3	4	1	2	3	5	2	3	5	3	4	7	3	4	NULL
4	3	4	2	3	4	1	2	3	1	2	3	1	2	3	1
4	3	4	2	3	4	1	2	3	1	2	3	2	3	4	2
4	3	4	2	3	4	1	2	3	1	2	3	6	2	3	6
4	3	4	2	3	4	1	2	3	1	2	3	7	3	4	NULL
4	3	4	2	3	4	2	3	4	2	3	4	1	2	3	1
4	3	4	2	3	4	2	3	4	2	3	4	2	3	4	2
4	3	4	2	3	4	2	3	4	2	3	4	6	2	3	6
4	3	4	2	3	4	2	3	4	2	3	4	7	3	4	NULL
4	3	4	2	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
4	3	4	2	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
4	3	4	2	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
4	3	4	2	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
4	3	4	2	3	4	5	2	3	5	3	4	1	2	3	1
4	3	4	2	3	4	5	2	3	5	3	4	2	3	4	2
4	3	4	2	3	4	5	2	3	5	3	4	6	2	3	6
4	3	4	2	3	4	5	2	3	5	3	4	7	3	4	NULL
4	3	4	3	2	3	1	2	3	1	2	3	1	2	3	1
4	3	4	3	2	3	1	2	3	1	2	3	2	3	4	2
4	3	4	3	2	3	1	2	3	1	2	3	6	2	3	6
4	3	4	3	2	3	1	2	3	1	2	3	7	3	4	NULL
4	3	4	3	2	3	2	3	4	2	3	4	1	2	3	1
4	3	4	3	2	3	2	3	4	2	3	4	2	3	4	2
4	3	4	3	2	3	2	3	4	2	3	4	6	2	3	6
4	3	4	3	2	3	2	3	4	2	3	4	7	3	4	NULL
4	3	4	3	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
4	3	4	3	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
4	3	4	3	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
4	3	4	3	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
4	3	4	3	2	3	5	2	3	5	3	4	1	2	3	1
4	3	4	3	2	3	5	2	3	5	3	4	2	3	4	2
4	3	4	3	2	3	5	2	3	5	3	4	6	2	3	6
4	3	4	3	2	3	5	2	3	5	3	4	7	3	4	NULL
4	3	4	4	3	4	1	2	3	1	2	3	1	2	3	1
4	3	4	4	3	4	1	2	3	1	2	3	2	3	4	2
4	3	4	4	3	4	1	2	3	1	2	3	6	2	3	6
4	3	4	4	3	4	1	2	3	1	2	3	7	3	4	NULL
4	3	4	4	3	4	2	3	4	2	3	4	1	2	3	1
4	3	4	4	3	4	2	3	4	2	3	4	2	3	4	2
4	3	4	4	3	4	2	3	4	2	3	4	6	2	3	6
4	3	4	4	3	4	2	3	4	2	3	4	7	3	4	NULL
4	3	4	4	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
4	3	4	4	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
4	3	4	4	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
4	3	4	4	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
4	3	4	4	3	4	5	2	3	5	3	4	1	2	3	1
4	3	4	4	3	4	5	2	3	5	3	4	2	3	4	2
4	3	4	4	3	4	5	2	3	5	3	4	6	2	3	6
4	3	4	4	3	4	5	2	3	5	3	4	7	3	4	NULL
5	2	3	1	2	3	1	2	3	1	2	3	1	2	3	1
5	2	3	1	2	3	1	2	3	1	2	3	2	3	4	2
5	2	3	1	2	3	1	2	3	1	2	3	6	2	3	6
5	2	3	1	2	3	1	2	3	1	2	3	7	3	4	NULL
5	2	3	1	2	3	2	3	4	2	3	4	1	2	3	1
5	2	3	1	2	3	2	3	4	2	3	4	2	3	4	2
5	2	3	1	2	3	2	3	4	2	3	4	6	2	3	6
5	2	3	1	2	3	2	3	4	2	3	4	7	3	4	NULL
5	2	3	1	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
5	2	3	1	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
5	2	3	1	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
5	2	3	1	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
5	2	3	1	2	3	5	2	3	5	3	4	1	2	3	1
5	2	3	1	2	3	5	2	3	5	3	4	2	3	4	2
5	2	3	1	2	3	5	2	3	5	3	4	6	2	3	6
5	2	3	1	2	3	5	2	3	5	3	4	7	3	4	NULL
5	2	3	2	3	4	1	2	3	1	2	3	1	2	3	1
5	2	3	2	3	4	1	2	3	1	2	3	2	3	4	2
5	2	3	2	3	4	1	2	3	1	2	3	6	2	3	6
5	2	3	2	3	4	1	2	3	1	2	3	7	3	4	NULL
5	2	3	2	3	4	2	3	4	2	3	4	1	2	3	1
5	2	3	2	3	4	2	3	4	2	3	4	2	3	4	2
5	2	3	2	3	4	2	3	4	2	3	4	6	2	3	6
5	2	3	2	3	4	2	3	4	2	3	4	7	3	4	NULL
5	2	3	2	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
5	2	3	2	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
5	2	3	2	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
5	2	3	2	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
5	2	3	2	3	4	5	2	3	5	3	4	1	2	3	1
5	2	3	2	3	4	5	2	3	5	3	4	2	3	4	2
5	2	3	2	3	4	5	2	3	5	3	4	6	2	3	6
5	2	3	2	3	4	5	2	3	5	3	4	7	3	4	NULL
5	2	3	3	2	3	1	2	3	1	2	3	1	2	3	1
5	2	3	3	2	3	1	2	3	1	2	3	2	3	4	2
5	2	3	3	2	3	1	2	3	1	2	3	6	2	3	6
5	2	3	3	2	3	1	2	3	1	2	3	7	3	4	NULL
5	2	3	3	2	3	2	3	4	2	3	4	1	2	3	1
5	2	3	3	2	3	2	3	4	2	3	4	2	3	4	2
5	2	3	3	2	3	2	3	4	2	3	4	6	2	3	6
5	2	3	3	2	3	2	3	4	2	3	4	7	3	4	NULL
5	2	3	3	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
5	2	3	3	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
5	2	3	3	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
5	2	3	3	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
5	2	3	3	2	3	5	2	3	5	3	4	1	2	3	1
5	2	3	3	2	3	5	2	3	5	3	4	2	3	4	2
5	2	3	3	2	3	5	2	3	5	3	4	6	2	3	6
5	2	3	3	2	3	5	2	3	5	3	4	7	3	4	NULL
5	2	3	4	3	4	1	2	3	1	2	3	1	2	3	1
5	2	3	4	3	4	1	2	3	1	2	3	2	3	4	2
5	2	3	4	3	4	1	2	3	1	2	3	6	2	3	6
5	2	3	4	3	4	1	2	3	1	2	3	7	3	4	NULL
5	2	3	4	3	4	2	3	4	2	3	4	1	2	3	1
5	2	3	4	3	4	2	3	4	2	3	4	2	3	4	2
5	2	3	4	3	4	2	3	4	2	3	4	6	2	3	6
5	2	3	4	3	4	2	3	4	2	3	4	7	3	4	NULL
5	2	3	4	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
5	2	3	4	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
5	2	3	4	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
5	2	3	4	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
5	2	3	4	3	4	5	2	3	5	3	4	1	2	3	1
5	2	3	4	3	4	5	2	3	5	3	4	2	3	4	2
5	2	3	4	3	4	5	2	3	5	3	4	6	2	3	6
5	2	3	4	3	4	5	2	3	5	3	4	7	3	4	NULL
6	3	4	1	2	3	1	2	3	1	2	3	1	2	3	1
6	3	4	1	2	3	1	2	3	1	2	3	2	3	4	2
6	3	4	1	2	3	1	2	3	1	2	3	6	2	3	6
6	3	4	1	2	3	1	2	3	1	2	3	7	3	4	NULL
6	3	4	1	2	3	2	3	4	2	3	4	1	2	3	1
6	3	4	1	2	3	2	3	4	2	3	4	2	3	4	2
6	3	4	1	2	3	2	3	4	2	3	4	6	2	3	6
6	3	4	1	2	3	2	3	4	2	3	4	7	3	4	NULL
6	3	4	1	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
6	3	4	1	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
6	3	4	1	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
6	3	4	1	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
6	3	4	1	2	3	5	2	3	5	3	4	1	2	3	1
6	3	4	1	2	3	5	2	3	5	3	4	2	3	4	2
6	3	4	1	2	3	5	2	3	5	3	4	6	2	3	6
6	3	4	1	2	3	5	2	3	5	3	4	7	3	4	NULL
6	3	4	2	3	4	1	2	3	1	2	3	1	2	3	1
6	3	4	2	3	4	1	2	3	1	2	3	2	3	4	2
6	3	4	2	3	4	1	2	3	1	2	3	6	2	3	6
6	3	4	2	3	4	1	2	3	1	2	3	7	3	4	NULL
6	3	4	2	3	4	2	3	4	2	3	4	1	2	3	1
6	3	4	2	3	4	2	3	4	2	3	4	2	3	4	2
6	3	4	2	3	4	2	3	4	2	3	4	6	2	3	6
6	3	4	2	3	4	2	3	4	2	3	4	7	3	4	NULL
6	3	4	2	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
6	3	4	2	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
6	3	4	2	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
6	3	4	2	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
6	3	4	2	3	4	5	2	3	5	3	4	1	2	3	1
6	3	4	2	3	4	5	2	3	5	3	4	2	3	4	2
6	3	4	2	3	4	5	2	3	5	3	4	6	2	3	6
6	3	4	2	3	4	5	2	3	5	3	4	7	3	4	NULL
6	3	4	3	2	3	1	2	3	1	2	3	1	2	3	1
6	3	4	3	2	3	1	2	3	1	2	3	2	3	4	2
6	3	4	3	2	3	1	2	3	1	2	3	6	2	3	6
6	3	4	3	2	3	1	2	3	1	2	3	7	3	4	NULL
6	3	4	3	2	3	2	3	4	2	3	4	1	2	3	1
6	3	4	3	2	3	2	3	4	2	3	4	2	3	4	2
6	3	4	3	2	3	2	3	4	2	3	4	6	2	3	6
6	3	4	3	2	3	2	3	4	2	3	4	7	3	4	NULL
6	3	4	3	2	3	3	3	4	NULL	NULL	NULL	1	2	3	1
6	3	4	3	2	3	3	3	4	NULL	NULL	NULL	2	3	4	2
6	3	4	3	2	3	3	3	4	NULL	NULL	NULL	6	2	3	6
6	3	4	3	2	3	3	3	4	NULL	NULL	NULL	7	3	4	NULL
6	3	4	3	2	3	5	2	3	5	3	4	1	2	3	1
6	3	4	3	2	3	5	2	3	5	3	4	2	3	4	2
6	3	4	3	2	3	5	2	3	5	3	4	6	2	3	6
6	3	4	3	2	3	5	2	3	5	3	4	7	3	4	NULL
6	3	4	4	3	4	1	2	3	1	2	3	1	2	3	1
6	3	4	4	3	4	1	2	3	1	2	3	2	3	4	2
6	3	4	4	3	4	1	2	3	1	2	3	6	2	3	6
6	3	4	4	3	4	1	2	3	1	2	3	7	3	4	NULL
6	3	4	4	3	4	2	3	4	2	3	4	1	2	3	1
6	3	4	4	3	4	2	3	4	2	3	4	2	3	4	2
6	3	4	4	3	4	2	3	4	2	3	4	6	2	3	6
6	3	4	4	3	4	2	3	4	2	3	4	7	3	4	NULL
6	3	4	4	3	4	3	3	4	NULL	NULL	NULL	1	2	3	1
6	3	4	4	3	4	3	3	4	NULL	NULL	NULL	2	3	4	2
6	3	4	4	3	4	3	3	4	NULL	NULL	NULL	6	2	3	6
6	3	4	4	3	4	3	3	4	NULL	NULL	NULL	7	3	4	NULL
6	3	4	4	3	4	5	2	3	5	3	4	1	2	3	1
6	3	4	4	3	4	5	2	3	5	3	4	2	3	4	2
6	3	4	4	3	4	5	2	3	5	3	4	6	2	3	6
6	3	4	4	3	4	5	2	3	5	3	4	7	3	4	NULL
explain format=tree select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select t1.a from t1 left join t2 on t1.a = t2.a));
EXPLAIN
-> Nested loop semijoin  (cost=5.05 rows=6)
    -> Nested loop inner join  (cost=2.95 rows=6)
        -> Filter: (t1.a is not null)  (cost=0.85 rows=6)
            -> Table scan on t1  (cost=0.85 rows=6)
        -> Filter: (t1.b is not null)  (cost=0.27 rows=1)
            -> Single-row index lookup on t1 using idx1 (a=t1.a)  (cost=0.27 rows=1)
    -> Nested loop left join  (cost=1.87 rows=1)
        -> Single-row index lookup on t1 using idx1 (a=t1.b)  (cost=0.27 rows=1)
        -> Filter: (t1.a = t2.a)  (cost=0.27 rows=1)
            -> Single-row index lookup on t2 using idx2 (a=t1.b)  (cost=0.27 rows=1)

explain format=json select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select t1.a from t1 left join t2 on t1.a = t2.a));
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "7.15"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t1",
          "access_type": "ALL",
          "possible_keys": [
            "idx1"
          ],
          "rows_examined_per_scan": 6,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.60",
            "prefix_cost": "0.85",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ],
          "attached_condition": "(`test`.`t1`.`a` is not null)"
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t1.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.50",
            "eval_cost": "0.60",
            "prefix_cost": "2.95",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a",
            "b"
          ],
          "attached_condition": "(`test`.`t1`.`b` is not null)"
        }
      },
      {
        "table": {
          "table_name": "t1",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx1"
          ],
          "key": "idx1",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t1.b"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "1.50",
            "eval_cost": "0.60",
            "prefix_cost": "5.05",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a"
          ]
        }
      },
      {
        "table": {
          "table_name": "t2",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx2"
          ],
          "key": "idx2",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t1.b"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "using_index": true,
          "first_match": "t1",
          "cost_info": {
            "read_cost": "1.50",
            "eval_cost": "0.60",
            "prefix_cost": "7.15",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a"
          ],
          "attached_condition": "<if>(is_not_null_compl(t2), (`test`.`t1`.`a` = `test`.`t2`.`a`), true)"
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` join `test`.`t1` semi join (`test`.`t1` left join `test`.`t2` on((`test`.`t1`.`a` = `test`.`t2`.`a`))) where ((`test`.`t1`.`a` = `test`.`t1`.`a`) and (`test`.`t1`.`a` = `test`.`t1`.`b`))
explain select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select t1.a from t1 left join t2 on t1.a = t2.a));
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	idx1	NULL	NULL	NULL	6	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t1.a	1	100.00	Using where
1	SIMPLE	t1	NULL	eq_ref	idx1	idx1	5	test.t1.b	1	100.00	Using index
1	SIMPLE	t2	NULL	eq_ref	idx2	idx2	5	test.t1.b	1	100.00	Using where; Using index; FirstMatch(t1)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` join `test`.`t1` semi join (`test`.`t1` left join `test`.`t2` on((`test`.`t1`.`a` = `test`.`t2`.`a`))) where ((`test`.`t1`.`a` = `test`.`t1`.`a`) and (`test`.`t1`.`a` = `test`.`t1`.`b`))
select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select t1.a from t1 left join t2 on t1.a = t2.a));
a	b	c
1	2	3
2	3	4
3	2	3
4	3	4
5	2	3
6	3	4
explain format=tree select * from t1 left join t2 on t1.a = t2.a;
EXPLAIN
-> Nested loop left join  (cost=2.95 rows=6)
    -> Table scan on t1  (cost=0.85 rows=6)
    -> Single-row index lookup on t2 using idx2 (a=t1.a)  (cost=0.27 rows=1)

explain format=json select * from t1 left join t2 on t1.a = t2.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.95"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t1",
          "access_type": "ALL",
          "rows_examined_per_scan": 6,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.60",
            "prefix_cost": "0.85",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      },
      {
        "table": {
          "table_name": "t2",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx2"
          ],
          "key": "idx2",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t1.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 6,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.50",
            "eval_cost": "0.60",
            "prefix_cost": "2.95",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` IGNORE INDEX FOR GROUP BY (`idx1`) left join `test`.`t2` USE INDEX FOR JOIN (`idx2`) on((`test`.`t2`.`a` = `test`.`t1`.`a`)) where true
explain select * from t1 left join t2 on t1.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	6	100.00	NULL
1	SIMPLE	t2	NULL	eq_ref	idx2	idx2	5	test.t1.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t1` IGNORE INDEX FOR GROUP BY (`idx1`) left join `test`.`t2` USE INDEX FOR JOIN (`idx2`) on((`test`.`t2`.`a` = `test`.`t1`.`a`)) where true
select * from t1 left join t2 on t1.a = t2.a;
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	2	3	3	2	3
4	3	4	4	3	4
5	2	3	NULL	NULL	NULL
6	3	4	NULL	NULL	NULL
explain format=tree select * from t3, t2 where t3.a = t2.a;
EXPLAIN
-> Nested loop inner join  (cost=2.05 rows=4)
    -> Filter: (t2.a is not null)  (cost=0.65 rows=4)
        -> Table scan on t2  (cost=0.65 rows=4)
    -> Single-row index lookup on t3 using idx3 (a=t2.a)  (cost=0.28 rows=1)

explain format=json select * from t3, t2 where t3.a = t2.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.05"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "t2",
          "access_type": "ALL",
          "possible_keys": [
            "idx2"
          ],
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ],
          "attached_condition": "(`test`.`t2`.`a` is not null)"
        }
      },
      {
        "table": {
          "table_name": "t3",
          "access_type": "eq_ref",
          "possible_keys": [
            "idx3"
          ],
          "key": "idx3",
          "used_key_parts": [
            "a"
          ],
          "key_length": "5",
          "ref": [
            "test.t2.a"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "1.00",
            "eval_cost": "0.40",
            "prefix_cost": "2.05",
            "data_read_per_join": "64"
          },
          "used_columns": [
            "a",
            "b",
            "c"
          ]
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t3` FORCE INDEX FOR JOIN (`idx3`) join `test`.`t2` where (`test`.`t3`.`a` = `test`.`t2`.`a`)
explain select * from t3, t2 where t3.a = t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	idx2	NULL	NULL	NULL	4	100.00	Using where
1	SIMPLE	t3	NULL	eq_ref	idx3	idx3	5	test.t2.a	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select `test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c` from `test`.`t3` FORCE INDEX FOR JOIN (`idx3`) join `test`.`t2` where (`test`.`t3`.`a` = `test`.`t2`.`a`)
select * from t3, t2 where t3.a = t2.a;
a	b	c	a	b	c
1	2	3	1	2	3
2	3	4	2	3	4
3	3	4	3	2	3
truncate table mysql.outline;
outline flush;
set global cdb_forbid_mysql_write = on;
delete from mysql.outline;
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
insert into mysql.outline values(1,"****", "fafafa", "*****");
ERROR 42000: Access denied for user 'root'@'localhost' to database 'mysql'
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "OUTLINE:select /*+max_execution_time(1000)*/* from t1, t2 where t1.a = t2.a";
select * from mysql.outline;
Id	Digest	Digest_text	Outline_text
1	a6f1b785369c22689ac1de6884404c800086e80df716969a4a6dec7436943910	select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(1000) */ * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)
show cdb_outline_info;
origin	outline	hit
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(1000) */ * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	0
select * from information_schema.cdb_outline_info;
ORIGIN	OUTLINE	HIT
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select /*+ MAX_EXECUTION_TIME(1000) */ * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	0
outline "select a from t1" set outline_info "OUTLINE:select b from t1";
ERROR HY000: SET outline_info failed, reason: different outline
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "OUTLINE:select * from t1, t2 use index(idx2) where t1.a = t2.b";
ERROR HY000: SET outline_info failed, reason: different outline
outline "select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select * from t1 left join t2 on t1.a = t2.a))" set outline_info "OUTLINE:select * from t1 where t1.a in (select t1.a from t1 where t1.b in (select * from t1 left join t2 USE index(idx2) on t1.c = t2.a))";
ERROR HY000: SET outline_info failed, reason: different outline
outline "select * from t1 left join t2 on t1.a = t2.a" set outline_info "";
ERROR HY000: SET outline_info failed, reason: unknown error
outline reset all;
outline "select * from t1, t2 where t1.a = t2.a" set outline_info "OUTLINE:select * from t1 use index(idx1), t2 where t1.a = t2.a";
outline "select count(*) from t1 join t2 on t1.a = t2.a" set outline_info "OPT:1#HASH_JOIN(t1,t2)#";
outline "select * from t1 where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a)" set outline_info "INDEX:1#test#t1#idx1#1#0";
outline "select * from t1 left join t4 on t1.a=t4.a where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a)" set outline_info "INDEX:1#test#t1#idx1#1#0";
outline "select count(*) from t1 join t2 on t1.a = t2.a" set outline_info "OPT:1#HASH_JOIN(t1,t2)#";
Warnings:
Warning	3126	Hint HASH_JOIN(`t1` ) is ignored as conflicting/duplicated
Warning	3126	Hint HASH_JOIN(`t2` ) is ignored as conflicting/duplicated
outline "select t1.a, avg(t1.b) from t1 join t2 on t1.a = t2.a group by t1.a" set outline_info "OPT:1#NO_HASH_JOIN(t1,t2)#";
outline "select a from t1 where t1.a in (select t2.b from t2 where t2.a = ?)" set outline_info "INDEX:1#test#t1#idx1#1#0#";
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	0
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from `test`.`t1` USE INDEX (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	0
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	0
select `a` from `test`.`t1` where `t1`.`a` in (select `t2`.`b` from `test`.`t2` where (`t2`.`a` = ?))	select `a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t2`.`b` from `test`.`t2` where (`t2`.`a` = ?))	0
select `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`	select /*+ NO_HASH_JOIN(t1,t2)*/ `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`	0
select count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select /*+ HASH_JOIN(`t1`@`select#1`) HASH_JOIN(`t2`@`select#1`) HASH_JOIN(t1,t2)*/ count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	0
set session cdb_opt_outline_enabled = on;
prepare stmt1 from "select * from t1, t2 where t1.a = t2.a";
prepare stmt2 from "select count(*) from t1 join t2 on t1.a = t2.a";
Warnings:
Warning	3126	Hint HASH_JOIN(`t1` ) is ignored as conflicting/duplicated
Warning	3126	Hint HASH_JOIN(`t2` ) is ignored as conflicting/duplicated
prepare stmt3 from "select * from t1 where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a)";
prepare stmt4 from "select * from t1 left join t4 on t1.a=t4.a where t1.a in (select t1.a from t1,t2 left join t3 on t1.a = t2.a)";
prepare stmt5 from "select a from t1 where t1.a in (select t2.b from t2 where t2.a = ?)";
set @t2a = 10000;
execute stmt5 using @t2a;
a
DEALLOCATE prepare stmt5;
prepare stmt6 from "select a from t1 where t1.a in (select t2.b from t2 where t2.a = ?)";
set @t2a1 = 10;
execute stmt6 using @t2a1;
a
DEALLOCATE prepare stmt6;
show cdb_outline_info;
origin	outline	hit
select * from (`test`.`t1` left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from (`test`.`t1` USE INDEX FOR JOIN (`idx1`) left join `test`.`t4` on((`t1`.`a` = `t4`.`a`))) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	1
select * from `test`.`t1` join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	select * from `test`.`t1` USE INDEX (`idx1`) join `test`.`t2` where (`t1`.`a` = `t2`.`a`)	1
select * from `test`.`t1` where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	select * from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t1`.`a` from `test`.`t1` join (`test`.`t2` left join `test`.`t3` on((`t1`.`a` = `t2`.`a`))))	1
select `a` from `test`.`t1` where `t1`.`a` in (select `t2`.`b` from `test`.`t2` where (`t2`.`a` = ?))	select `a` from `test`.`t1` USE INDEX FOR JOIN (`idx1`) where `t1`.`a` in (select `t2`.`b` from `test`.`t2` where (`t2`.`a` = ?))	2
select `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`	select /*+ NO_HASH_JOIN(t1,t2)*/ `t1`.`a`,avg(`t1`.`b`) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) group by `t1`.`a`	0
select count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	select /*+ HASH_JOIN(`t1`@`select#1`) HASH_JOIN(`t2`@`select#1`) HASH_JOIN(t1,t2)*/ count(?) from (`test`.`t1` join `test`.`t2` on((`t1`.`a` = `t2`.`a`)))	1
set global cdb_forbid_mysql_write = off;
set global cdb_opt_outline_enabled = off;
drop table t1;
drop table t2;
drop table t3;
drop table t4;
drop table t5;
drop user 'user1'@'%';
drop user 'user2'@'%';

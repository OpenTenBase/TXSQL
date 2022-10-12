show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ABORT_EXEMPT,AUDIT_ADMIN,AUTHENTICATION_POLICY_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,FIREWALL_EXEMPT,FLUSH_OPTIMIZER_COSTS,FLUSH_STATUS,FLUSH_TABLES,FLUSH_USER_RESOURCES,GROUP_REPLICATION_ADMIN,GROUP_REPLICATION_STREAM,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PASSWORDLESS_USER_ADMIN,PERSIST_RO_VARIABLES_ADMIN,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SENSITIVE_VARIABLES_OBSERVER,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ``@`` TO `root`@`localhost` WITH GRANT OPTION
select user();
user()
root@localhost
create database test1;
create user 'user1'@'%';
grant all on *.* to 'user1'@'%';
CREATE TABLE t1 (a int not null, b char (10) not null);
insert into t1 values(1,'a'),(2,'b'),(3,'c'),(3,'c');
CREATE TABLE t2 (a int not null, b char (10) not null);
insert into t2 values (3,'c'),(4,'d'),(5,'f'),(6,'e');
ANALYZE TABLE t1,t2;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
explain select * from t1,(select t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers)
1	PRIMARY	<derived2>	NULL	ref	<auto_key0>	<auto_key0>	4	test.t1.a	2	100.00	NULL
1	PRIMARY	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Send to (<receiver1>)
1	PRIMARY	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Receive from (<sender1>)
2	DERIVED	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select `test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
explain format=tree select * from t1,(select t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=2.65 rows=0)
            -> Parallel table scan on t1  (cost=0.65 rows=4)
            -> Index lookup on tt using <auto_key0> (a=t1.a)
                -> Materialize  (cost=0.00..0.00 rows=0)
                    -> Table scan on <temporary>
                        -> Aggregate using temporary table
                            -> Table scan on t2  (cost=0.65 rows=4)

explain format=json select * from t1,(select t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
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
          "table_name": "t1",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "parallel_scan": "4 workers",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "192"
          },
          "used_columns": [
            "a",
            "b"
          ]
        }
      },
      {
        "table": {
          "table_name": "tt",
          "access_type": "ref",
          "possible_keys": [
            "<auto_key0>"
          ],
          "key": "<auto_key0>",
          "used_key_parts": [
            "a"
          ],
          "key_length": "4",
          "ref": [
            "test.t1.a"
          ],
          "rows_examined_per_scan": 2,
          "rows_produced_per_join": 8,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "2.00",
            "eval_cost": "0.80",
            "prefix_cost": "3.45",
            "data_read_per_join": "192"
          },
          "used_columns": [
            "a",
            "count(t2.b)"
          ],
          "materialized_from_subquery": {
            "using_temporary_table": true,
            "dependent": false,
            "cacheable": true,
            "query_block": {
              "select_id": 2,
              "cost_info": {
                "query_cost": "0.65"
              },
              "grouping_operation": {
                "using_temporary_table": true,
                "using_filesort": false,
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
                    "data_read_per_join": "192"
                  },
                  "used_columns": [
                    "a",
                    "b"
                  ]
                }
              }
            }
          }
        }
      },
      {
        "exchange_table": {
          "table_name": "<sender1>",
          "access_type": "ALL",
          "rows_examined_per_scan": 0,
          "rows_produced_per_join": 0,
          "filtered": "0.00",
          "send_to": "<receiver1>",
          "cost_info": {
            "read_cost": "0.00",
            "eval_cost": "0.00",
            "prefix_cost": "0.00",
            "data_read_per_join": "0"
          }
        }
      },
      {
        "exchange_table": {
          "table_name": "<receiver1>",
          "access_type": "ALL",
          "rows_examined_per_scan": 0,
          "rows_produced_per_join": 0,
          "filtered": "0.00",
          "receive_from": "<sender1>",
          "cost_info": {
            "read_cost": "0.00",
            "eval_cost": "0.00",
            "prefix_cost": "0.00",
            "data_read_per_join": "0"
          }
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select `test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
select * from t1,(select t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
a	b	a	count(t2.b)
3	c	3	1
3	c	3	1
explain select * from (select t2.a, count(t2.b) from t2 group by t2.a) as tt;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	<derived2>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	NULL
2	DERIVED	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers); Using temporary
2	DERIVED	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Send to (<receiver1>)
2	DERIVED	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Receive from (<sender1>); Using temporary
Warnings:
Note	1003	/* select#1 */ select `tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from (/* select#2 */ select `test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt`
explain format=tree select * from (select t2.a, count(t2.b) from t2 group by t2.a) as tt;
EXPLAIN
-> Table scan on tt  (cost=2.50..2.50 rows=0)
    -> Materialize  (cost=2.50..2.50 rows=0)
        -> Table scan on <temporary>
            -> Final Aggregate using temporary table
                -> PX Receiver (slice: 0; workers: 1)
                    -> PX Sender (slice: 1; workers: 4)
                        -> Table scan on <temporary>
                            -> Aggregate using temporary table
                                -> Parallel table scan on t2  (cost=0.65 rows=4)

explain format=json select * from (select t2.a, count(t2.b) from t2 group by t2.a) as tt;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.95"
    },
    "table": {
      "table_name": "tt",
      "access_type": "ALL",
      "rows_examined_per_scan": 4,
      "rows_produced_per_join": 4,
      "filtered": "100.00",
      "cost_info": {
        "read_cost": "2.55",
        "eval_cost": "0.40",
        "prefix_cost": "2.95",
        "data_read_per_join": "96"
      },
      "used_columns": [
        "a",
        "count(t2.b)"
      ],
      "materialized_from_subquery": {
        "using_temporary_table": true,
        "dependent": false,
        "cacheable": true,
        "query_block": {
          "select_id": 2,
          "cost_info": {
            "query_cost": "0.65"
          },
          "grouping_operation": {
            "using_temporary_table": true,
            "using_filesort": false,
            "table": {
              "table_name": "t2",
              "access_type": "ALL",
              "rows_examined_per_scan": 4,
              "rows_produced_per_join": 4,
              "filtered": "100.00",
              "parallel_scan": "4 workers",
              "cost_info": {
                "read_cost": "0.25",
                "eval_cost": "0.40",
                "prefix_cost": "0.65",
                "data_read_per_join": "192"
              },
              "used_columns": [
                "a",
                "b"
              ]
            },
            "parallel_information": [
              {
                "exchange_table": {
                  "table_name": "<sender1>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 0,
                  "rows_produced_per_join": 0,
                  "filtered": "0.00",
                  "send_to": "<receiver1>",
                  "cost_info": {
                    "read_cost": "0.00",
                    "eval_cost": "0.00",
                    "prefix_cost": "0.00",
                    "data_read_per_join": "0"
                  }
                }
              },
              {
                "exchange_table": {
                  "table_name": "<receiver1>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 0,
                  "rows_produced_per_join": 0,
                  "filtered": "0.00",
                  "receive_from": "<sender1>",
                  "cost_info": {
                    "read_cost": "0.00",
                    "eval_cost": "0.00",
                    "prefix_cost": "0.00",
                    "data_read_per_join": "0"
                  }
                }
              }
            ]
          }
        }
      }
    }
  }
}
Warnings:
Note	1003	/* select#1 */ select `tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from (/* select#2 */ select `test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt`
select * from (select t2.a, count(t2.b) from t2 group by t2.a) as tt;
a	count(t2.b)
3	1
4	1
5	1
6	1
dervied table with unsupported function
explain select * from t1,(select rand(), t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	PRIMARY	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>)
1	PRIMARY	<derived2>	NULL	ref	<auto_key0>	<auto_key0>	4	test.t1.a	2	100.00	NULL
2	DERIVED	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`rand()` AS `rand()`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select rand() AS `rand()`,`test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
explain format=tree select * from t1,(select rand(), t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
EXPLAIN
-> Nested loop inner join  (cost=2.65 rows=0)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel table scan on t1  (cost=0.65 rows=4)
    -> Index lookup on tt using <auto_key0> (a=t1.a)
        -> Materialize  (cost=0.00..0.00 rows=0)
            -> Table scan on <temporary>
                -> Aggregate using temporary table
                    -> Table scan on t2  (cost=0.65 rows=4)

explain select * from t1,(select rand(), t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	PRIMARY	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>)
1	PRIMARY	<derived2>	NULL	ref	<auto_key0>	<auto_key0>	4	test.t1.a	2	100.00	NULL
2	DERIVED	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`rand()` AS `rand()`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select rand() AS `rand()`,`test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
explain format=json select * from t1,(select rand(), t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
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
          "table_name": "t1",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "parallel_scan": "4 workers",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "192"
          },
          "used_columns": [
            "a",
            "b"
          ]
        }
      },
      {
        "exchange_table": {
          "table_name": "<sender1>",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 0,
          "filtered": "100.00",
          "send_to": "<receiver1>",
          "cost_info": {
            "read_cost": "0.00",
            "eval_cost": "0.00",
            "prefix_cost": "0.00",
            "data_read_per_join": "0"
          }
        }
      },
      {
        "exchange_table": {
          "table_name": "<receiver1>",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 0,
          "filtered": "100.00",
          "receive_from": "<sender1>",
          "cost_info": {
            "read_cost": "0.00",
            "eval_cost": "0.00",
            "prefix_cost": "0.00",
            "data_read_per_join": "0"
          }
        }
      },
      {
        "table": {
          "table_name": "tt",
          "access_type": "ref",
          "possible_keys": [
            "<auto_key0>"
          ],
          "key": "<auto_key0>",
          "used_key_parts": [
            "a"
          ],
          "key_length": "4",
          "ref": [
            "test.t1.a"
          ],
          "rows_examined_per_scan": 2,
          "rows_produced_per_join": 8,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "2.00",
            "eval_cost": "0.80",
            "prefix_cost": "3.45",
            "data_read_per_join": "256"
          },
          "used_columns": [
            "rand()",
            "a",
            "count(t2.b)"
          ],
          "materialized_from_subquery": {
            "using_temporary_table": true,
            "dependent": false,
            "cacheable": true,
            "query_block": {
              "select_id": 2,
              "cost_info": {
                "query_cost": "0.65"
              },
              "grouping_operation": {
                "using_temporary_table": true,
                "using_filesort": false,
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
                    "data_read_per_join": "192"
                  },
                  "used_columns": [
                    "a",
                    "b"
                  ]
                }
              }
            }
          }
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`rand()` AS `rand()`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select rand() AS `rand()`,`test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
# --include save_and_set_px.inc
explain select * from t1,(select t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers)
1	PRIMARY	<derived2>	NULL	ref	<auto_key0>	<auto_key0>	4	test.t1.a	2	100.00	NULL
1	PRIMARY	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Send to (<receiver1>)
1	PRIMARY	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Receive from (<sender1>)
2	DERIVED	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select `test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
explain format=tree select * from t1,(select t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=2.65 rows=0)
            -> Parallel table scan on t1  (cost=0.65 rows=4)
            -> Index lookup on tt using <auto_key0> (a=t1.a)
                -> Materialize  (cost=0.00..0.00 rows=0)
                    -> Table scan on <temporary>
                        -> Aggregate using temporary table
                            -> Table scan on t2  (cost=0.65 rows=4)

explain format=json select * from t1,(select t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
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
          "table_name": "t1",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "parallel_scan": "4 workers",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "192"
          },
          "used_columns": [
            "a",
            "b"
          ]
        }
      },
      {
        "table": {
          "table_name": "tt",
          "access_type": "ref",
          "possible_keys": [
            "<auto_key0>"
          ],
          "key": "<auto_key0>",
          "used_key_parts": [
            "a"
          ],
          "key_length": "4",
          "ref": [
            "test.t1.a"
          ],
          "rows_examined_per_scan": 2,
          "rows_produced_per_join": 8,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "2.00",
            "eval_cost": "0.80",
            "prefix_cost": "3.45",
            "data_read_per_join": "192"
          },
          "used_columns": [
            "a",
            "count(t2.b)"
          ],
          "materialized_from_subquery": {
            "using_temporary_table": true,
            "dependent": false,
            "cacheable": true,
            "query_block": {
              "select_id": 2,
              "cost_info": {
                "query_cost": "0.65"
              },
              "grouping_operation": {
                "using_temporary_table": true,
                "using_filesort": false,
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
                    "data_read_per_join": "192"
                  },
                  "used_columns": [
                    "a",
                    "b"
                  ]
                }
              }
            }
          }
        }
      },
      {
        "exchange_table": {
          "table_name": "<sender1>",
          "access_type": "ALL",
          "rows_examined_per_scan": 0,
          "rows_produced_per_join": 0,
          "filtered": "0.00",
          "send_to": "<receiver1>",
          "cost_info": {
            "read_cost": "0.00",
            "eval_cost": "0.00",
            "prefix_cost": "0.00",
            "data_read_per_join": "0"
          }
        }
      },
      {
        "exchange_table": {
          "table_name": "<receiver1>",
          "access_type": "ALL",
          "rows_examined_per_scan": 0,
          "rows_produced_per_join": 0,
          "filtered": "0.00",
          "receive_from": "<sender1>",
          "cost_info": {
            "read_cost": "0.00",
            "eval_cost": "0.00",
            "prefix_cost": "0.00",
            "data_read_per_join": "0"
          }
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select `test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
select * from t1,(select t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
a	b	a	count(t2.b)
3	c	3	1
3	c	3	1
explain select * from (select t2.a, count(t2.b) from t2 group by t2.a) as tt;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	<derived2>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	NULL
2	DERIVED	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers); Using temporary
2	DERIVED	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Send to (<receiver1>)
2	DERIVED	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Receive from (<sender1>); Using temporary
Warnings:
Note	1003	/* select#1 */ select `tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from (/* select#2 */ select `test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt`
explain format=tree select * from (select t2.a, count(t2.b) from t2 group by t2.a) as tt;
EXPLAIN
-> Table scan on tt  (cost=2.50..2.50 rows=0)
    -> Materialize  (cost=2.50..2.50 rows=0)
        -> Table scan on <temporary>
            -> Final Aggregate using temporary table
                -> PX Receiver (slice: 0; workers: 1)
                    -> PX Sender (slice: 1; workers: 4)
                        -> Table scan on <temporary>
                            -> Aggregate using temporary table
                                -> Parallel table scan on t2  (cost=0.65 rows=4)

explain format=json select * from (select t2.a, count(t2.b) from t2 group by t2.a) as tt;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "2.95"
    },
    "table": {
      "table_name": "tt",
      "access_type": "ALL",
      "rows_examined_per_scan": 4,
      "rows_produced_per_join": 4,
      "filtered": "100.00",
      "cost_info": {
        "read_cost": "2.55",
        "eval_cost": "0.40",
        "prefix_cost": "2.95",
        "data_read_per_join": "96"
      },
      "used_columns": [
        "a",
        "count(t2.b)"
      ],
      "materialized_from_subquery": {
        "using_temporary_table": true,
        "dependent": false,
        "cacheable": true,
        "query_block": {
          "select_id": 2,
          "cost_info": {
            "query_cost": "0.65"
          },
          "grouping_operation": {
            "using_temporary_table": true,
            "using_filesort": false,
            "table": {
              "table_name": "t2",
              "access_type": "ALL",
              "rows_examined_per_scan": 4,
              "rows_produced_per_join": 4,
              "filtered": "100.00",
              "parallel_scan": "4 workers",
              "cost_info": {
                "read_cost": "0.25",
                "eval_cost": "0.40",
                "prefix_cost": "0.65",
                "data_read_per_join": "192"
              },
              "used_columns": [
                "a",
                "b"
              ]
            },
            "parallel_information": [
              {
                "exchange_table": {
                  "table_name": "<sender1>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 0,
                  "rows_produced_per_join": 0,
                  "filtered": "0.00",
                  "send_to": "<receiver1>",
                  "cost_info": {
                    "read_cost": "0.00",
                    "eval_cost": "0.00",
                    "prefix_cost": "0.00",
                    "data_read_per_join": "0"
                  }
                }
              },
              {
                "exchange_table": {
                  "table_name": "<receiver1>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 0,
                  "rows_produced_per_join": 0,
                  "filtered": "0.00",
                  "receive_from": "<sender1>",
                  "cost_info": {
                    "read_cost": "0.00",
                    "eval_cost": "0.00",
                    "prefix_cost": "0.00",
                    "data_read_per_join": "0"
                  }
                }
              }
            ]
          }
        }
      }
    }
  }
}
Warnings:
Note	1003	/* select#1 */ select `tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from (/* select#2 */ select `test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt`
select * from (select t2.a, count(t2.b) from t2 group by t2.a) as tt;
a	count(t2.b)
3	1
4	1
5	1
6	1
dervied table with unsupported function
explain select * from t1,(select rand(), t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	PRIMARY	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>)
1	PRIMARY	<derived2>	NULL	ref	<auto_key0>	<auto_key0>	4	test.t1.a	2	100.00	NULL
2	DERIVED	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`rand()` AS `rand()`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select rand() AS `rand()`,`test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
explain format=tree select * from t1,(select rand(), t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
EXPLAIN
-> Nested loop inner join  (cost=2.65 rows=0)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Parallel table scan on t1  (cost=0.65 rows=4)
    -> Index lookup on tt using <auto_key0> (a=t1.a)
        -> Materialize  (cost=0.00..0.00 rows=0)
            -> Table scan on <temporary>
                -> Aggregate using temporary table
                    -> Table scan on t2  (cost=0.65 rows=4)

explain select * from t1,(select rand(), t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	PRIMARY	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>)
1	PRIMARY	<derived2>	NULL	ref	<auto_key0>	<auto_key0>	4	test.t1.a	2	100.00	NULL
2	DERIVED	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`rand()` AS `rand()`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select rand() AS `rand()`,`test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
explain format=json select * from t1,(select rand(), t2.a, count(t2.b) from t2 group by t2.a) as tt where t1.a = tt.a;
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
          "table_name": "t1",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "parallel_scan": "4 workers",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "192"
          },
          "used_columns": [
            "a",
            "b"
          ]
        }
      },
      {
        "exchange_table": {
          "table_name": "<sender1>",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 0,
          "filtered": "100.00",
          "send_to": "<receiver1>",
          "cost_info": {
            "read_cost": "0.00",
            "eval_cost": "0.00",
            "prefix_cost": "0.00",
            "data_read_per_join": "0"
          }
        }
      },
      {
        "exchange_table": {
          "table_name": "<receiver1>",
          "access_type": "ALL",
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 0,
          "filtered": "100.00",
          "receive_from": "<sender1>",
          "cost_info": {
            "read_cost": "0.00",
            "eval_cost": "0.00",
            "prefix_cost": "0.00",
            "data_read_per_join": "0"
          }
        }
      },
      {
        "table": {
          "table_name": "tt",
          "access_type": "ref",
          "possible_keys": [
            "<auto_key0>"
          ],
          "key": "<auto_key0>",
          "used_key_parts": [
            "a"
          ],
          "key_length": "4",
          "ref": [
            "test.t1.a"
          ],
          "rows_examined_per_scan": 2,
          "rows_produced_per_join": 8,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "2.00",
            "eval_cost": "0.80",
            "prefix_cost": "3.45",
            "data_read_per_join": "256"
          },
          "used_columns": [
            "rand()",
            "a",
            "count(t2.b)"
          ],
          "materialized_from_subquery": {
            "using_temporary_table": true,
            "dependent": false,
            "cacheable": true,
            "query_block": {
              "select_id": 2,
              "cost_info": {
                "query_cost": "0.65"
              },
              "grouping_operation": {
                "using_temporary_table": true,
                "using_filesort": false,
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
                    "data_read_per_join": "192"
                  },
                  "used_columns": [
                    "a",
                    "b"
                  ]
                }
              }
            }
          }
        }
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`tt`.`rand()` AS `rand()`,`tt`.`a` AS `a`,`tt`.`count(t2.b)` AS `count(t2.b)` from `test`.`t1` join (/* select#2 */ select rand() AS `rand()`,`test`.`t2`.`a` AS `a`,count(`test`.`t2`.`b`) AS `count(t2.b)` from `test`.`t2` group by `test`.`t2`.`a`) `tt` where (`tt`.`a` = `test`.`t1`.`a`)
# --include set_px_to_saved.inc
drop table t1,t2;
drop database test1;
drop user user1;

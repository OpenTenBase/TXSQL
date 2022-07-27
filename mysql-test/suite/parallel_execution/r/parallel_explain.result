show grants;
Grants for root@localhost
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS, FILE, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES, EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE, CREATE ROLE, DROP ROLE ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT APPLICATION_PASSWORD_ADMIN,AUDIT_ADMIN,BACKUP_ADMIN,BINLOG_ADMIN,BINLOG_ENCRYPTION_ADMIN,CLONE_ADMIN,CONNECTION_ADMIN,ENCRYPTION_KEY_ADMIN,GROUP_REPLICATION_ADMIN,INNODB_REDO_LOG_ARCHIVE,INNODB_REDO_LOG_ENABLE,PERSIST_RO_VARIABLES_ADMIN,REPLICATION_APPLIER,REPLICATION_SLAVE_ADMIN,RESOURCE_GROUP_ADMIN,RESOURCE_GROUP_USER,ROLE_ADMIN,SERVICE_CONNECTION_ADMIN,SESSION_VARIABLES_ADMIN,SET_USER_ID,SHOW_ROUTINE,SYSTEM_USER,SYSTEM_VARIABLES_ADMIN,TABLE_ENCRYPTION_ADMIN,XA_RECOVER_ADMIN ON *.* TO `root`@`localhost` WITH GRANT OPTION
GRANT PROXY ON ''@'' TO 'root'@'localhost' WITH GRANT OPTION
select user();
user()
root@localhost
create database test1;
create user 'user1'@'%';
grant all on *.* to 'user1'@'%';
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
ANALYZE TABLE t1,t2,t3,t4;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
test.t2	analyze	status	OK
test.t3	analyze	status	OK
test.t4	analyze	status	OK
# --include save_and_set_px.inc
set txsql_parallel_table_record_threshold = 2;
EXPLAIN FORMAT=tree SELECT /*+NO_PARALLEL(t1)*/ * FROM t1;
EXPLAIN
-> Table scan on t1  (cost=0.95 rows=7)

EXPLAIN SELECT /*+NO_PARALLEL(t1)*/ * FROM t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select /*+ NO_PARALLEL(`t1`@`select#1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
EXPLAIN FORMAT=JSON SELECT /*+NO_PARALLEL(t1)*/ * FROM t1;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "0.95"
    },
    "table": {
      "table_name": "t1",
      "access_type": "ALL",
      "rows_examined_per_scan": 7,
      "rows_produced_per_join": 7,
      "filtered": "100.00",
      "cost_info": {
        "read_cost": "0.25",
        "eval_cost": "0.70",
        "prefix_cost": "0.95",
        "data_read_per_join": "168"
      },
      "used_columns": [
        "a",
        "b",
        "c",
        "d"
      ]
    }
  }
}
Warnings:
Note	1003	/* select#1 */ select /*+ NO_PARALLEL(`t1`@`select#1`) */ `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
EXPLAIN FORMAT=tree SELECT * FROM t1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel table scan on t1  (cost=0.95 rows=7)

EXPLAIN SELECT * FROM t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
EXPLAIN FORMAT=JSON SELECT * FROM t1;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "0.95"
    },
    "table": {
      "table_name": "t1",
      "access_type": "ALL",
      "rows_examined_per_scan": 7,
      "rows_produced_per_join": 7,
      "filtered": "100.00",
      "parallel_scan": "4 workers",
      "cost_info": {
        "read_cost": "0.25",
        "eval_cost": "0.70",
        "prefix_cost": "0.95",
        "data_read_per_join": "168"
      },
      "used_columns": [
        "a",
        "b",
        "c",
        "d"
      ]
    },
    "parallel_information": [
      {
        "exchange_table": {
          "table_name": "<sender1>",
          "access_type": "ALL",
          "rows_examined_per_scan": 7,
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
          "rows_examined_per_scan": 7,
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
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1`
EXPLAIN FORMAT=tree SELECT sum(a) FROM t1;
EXPLAIN
-> Aggregate: sum(t1.a)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Aggregate: sum(t1.a)
                -> Parallel table scan on t1  (cost=0.95 rows=7)

EXPLAIN SELECT sum(a) FROM t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select sum(`test`.`t1`.`a`) AS `sum(a)` from `test`.`t1`
EXPLAIN FORMAT=json SELECT sum(a) FROM t1;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "0.95"
    },
    "table": {
      "table_name": "t1",
      "access_type": "ALL",
      "rows_examined_per_scan": 7,
      "rows_produced_per_join": 7,
      "filtered": "100.00",
      "parallel_scan": "4 workers",
      "cost_info": {
        "read_cost": "0.25",
        "eval_cost": "0.70",
        "prefix_cost": "0.95",
        "data_read_per_join": "168"
      },
      "used_columns": [
        "a"
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
Warnings:
Note	1003	/* select#1 */ select sum(`test`.`t1`.`a`) AS `sum(a)` from `test`.`t1`
EXPLAIN FORMAT=tree SELECT count(a) FROM t1 group by b;
EXPLAIN
-> Table scan on <temporary>
    -> Final Aggregate using temporary table
        -> PX Receiver (slice: 0; workers: 1)
            -> PX Sender (slice: 1; workers: 4)
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> Parallel table scan on t1  (cost=0.95 rows=7)

EXPLAIN SELECT count(a) FROM t1 group by b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers); Using temporary
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Receive from (<sender1>); Using temporary
Warnings:
Note	1003	/* select#1 */ select count(`test`.`t1`.`a`) AS `count(a)` from `test`.`t1` group by `test`.`t1`.`b`
EXPLAIN FORMAT=json SELECT count(a) FROM t1 group by b;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "0.95"
    },
    "grouping_operation": {
      "using_temporary_table": true,
      "using_filesort": false,
      "table": {
        "table_name": "t1",
        "access_type": "ALL",
        "rows_examined_per_scan": 7,
        "rows_produced_per_join": 7,
        "filtered": "100.00",
        "parallel_scan": "4 workers",
        "cost_info": {
          "read_cost": "0.25",
          "eval_cost": "0.70",
          "prefix_cost": "0.95",
          "data_read_per_join": "168"
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
Warnings:
Note	1003	/* select#1 */ select count(`test`.`t1`.`a`) AS `count(a)` from `test`.`t1` group by `test`.`t1`.`b`
EXPLAIN FORMAT=tree SELECT * FROM t3 order by a;
EXPLAIN
-> PX Receiver (merge sort; slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Parallel index scan on t3 using PRIMARY  (cost=1.25 rows=10)

EXPLAIN SELECT * FROM t3 order by a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t3	NULL	index	NULL	PRIMARY	4	NULL	10	100.00	Parallel scan (4 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	10	100.00	Receive from (<sender1>); Using merge sort
Warnings:
Note	1003	/* select#1 */ select `test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t3`.`d` AS `d` from `test`.`t3` order by `test`.`t3`.`a`
EXPLAIN FORMAT=json SELECT * FROM t3 order by a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "1.25"
    },
    "ordering_operation": {
      "using_filesort": false,
      "table": {
        "table_name": "t3",
        "access_type": "index",
        "key": "PRIMARY",
        "used_key_parts": [
          "a"
        ],
        "key_length": "4",
        "rows_examined_per_scan": 10,
        "rows_produced_per_join": 10,
        "filtered": "100.00",
        "parallel_scan": "4 workers",
        "cost_info": {
          "read_cost": "0.25",
          "eval_cost": "1.00",
          "prefix_cost": "1.25",
          "data_read_per_join": "240"
        },
        "used_columns": [
          "a",
          "b",
          "c",
          "d"
        ]
      },
      "parallel_information": [
        {
          "exchange_table": {
            "table_name": "<sender1>",
            "access_type": "ALL",
            "rows_examined_per_scan": 10,
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
            "rows_examined_per_scan": 10,
            "rows_produced_per_join": 0,
            "filtered": "100.00",
            "receive_from": "<sender1>",
            "using_merge_sort": true,
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
Warnings:
Note	1003	/* select#1 */ select `test`.`t3`.`a` AS `a`,`test`.`t3`.`b` AS `b`,`test`.`t3`.`c` AS `c`,`test`.`t3`.`d` AS `d` from `test`.`t3` order by `test`.`t3`.`a`
explain format=tree select * from t1 union select * from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=0.65 rows=4)

explain select * from t1 union select * from t2;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers)
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender3>)
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2`
explain format=json select * from t1 union select * from t2;
EXPLAIN
{
  "query_block": {
    "union_result": {
      "using_temporary_table": true,
      "table_name": "<union1,2>",
      "access_type": "ALL",
      "query_specifications": [
        {
          "dependent": false,
          "cacheable": true,
          "query_block": {
            "select_id": 1,
            "cost_info": {
              "query_cost": "0.95"
            },
            "table": {
              "table_name": "t1",
              "access_type": "ALL",
              "rows_examined_per_scan": 7,
              "rows_produced_per_join": 7,
              "filtered": "100.00",
              "parallel_scan": "4 workers",
              "cost_info": {
                "read_cost": "0.25",
                "eval_cost": "0.70",
                "prefix_cost": "0.95",
                "data_read_per_join": "168"
              },
              "used_columns": [
                "a",
                "b",
                "c",
                "d"
              ]
            },
            "parallel_information": [
              {
                "exchange_table": {
                  "table_name": "<sender2>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 7,
                  "rows_produced_per_join": 0,
                  "filtered": "100.00",
                  "send_to": "<receiver2>",
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
                  "table_name": "<receiver2>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 7,
                  "rows_produced_per_join": 0,
                  "filtered": "100.00",
                  "receive_from": "<sender2>",
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
        },
        {
          "dependent": false,
          "cacheable": true,
          "query_block": {
            "select_id": 2,
            "cost_info": {
              "query_cost": "0.65"
            },
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
                "data_read_per_join": "96"
              },
              "used_columns": [
                "a",
                "b",
                "c",
                "d"
              ]
            },
            "parallel_information": [
              {
                "exchange_table": {
                  "table_name": "<sender3>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 4,
                  "rows_produced_per_join": 0,
                  "filtered": "100.00",
                  "send_to": "<receiver3>",
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
                  "table_name": "<receiver3>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 4,
                  "rows_produced_per_join": 0,
                  "filtered": "100.00",
                  "receive_from": "<sender3>",
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
      ]
    }
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2`
explain format=tree select * from t1 union all select * from t2;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Append
            -> Stream results  (cost=0.95 rows=7)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 2; workers: 4)
                        -> Parallel table scan on t1  (cost=0.95 rows=7)
            -> Stream results  (cost=0.65 rows=4)
                -> PX Receiver (slice: 1; workers: 1)
                    -> PX Sender (slice: 3; workers: 4)
                        -> Parallel table scan on t2  (cost=0.65 rows=4)

explain select * from t1 union all select * from t2;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers)
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Receive from (<sender2>)
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers)
2	UNION	<sender3>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver3>)
2	UNION	<receiver3>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender3>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union all /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2`
explain format=json select * from t1 union all select * from t2;
EXPLAIN
{
  "query_block": {
    "union_result": {
      "using_temporary_table": false,
      "query_specifications": [
        {
          "dependent": false,
          "cacheable": true,
          "query_block": {
            "select_id": 1,
            "cost_info": {
              "query_cost": "0.95"
            },
            "table": {
              "table_name": "t1",
              "access_type": "ALL",
              "rows_examined_per_scan": 7,
              "rows_produced_per_join": 7,
              "filtered": "100.00",
              "parallel_scan": "4 workers",
              "cost_info": {
                "read_cost": "0.25",
                "eval_cost": "0.70",
                "prefix_cost": "0.95",
                "data_read_per_join": "168"
              },
              "used_columns": [
                "a",
                "b",
                "c",
                "d"
              ]
            },
            "parallel_information": [
              {
                "exchange_table": {
                  "table_name": "<sender2>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 7,
                  "rows_produced_per_join": 0,
                  "filtered": "100.00",
                  "send_to": "<receiver2>",
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
                  "table_name": "<receiver2>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 7,
                  "rows_produced_per_join": 0,
                  "filtered": "100.00",
                  "receive_from": "<sender2>",
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
        },
        {
          "dependent": false,
          "cacheable": true,
          "query_block": {
            "select_id": 2,
            "cost_info": {
              "query_cost": "0.65"
            },
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
                "data_read_per_join": "96"
              },
              "used_columns": [
                "a",
                "b",
                "c",
                "d"
              ]
            },
            "parallel_information": [
              {
                "exchange_table": {
                  "table_name": "<sender3>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 4,
                  "rows_produced_per_join": 0,
                  "filtered": "100.00",
                  "send_to": "<receiver3>",
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
                  "table_name": "<receiver3>",
                  "access_type": "ALL",
                  "rows_examined_per_scan": 4,
                  "rows_produced_per_join": 0,
                  "filtered": "100.00",
                  "receive_from": "<sender3>",
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
      ]
    }
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c`,`test`.`t1`.`d` AS `d` from `test`.`t1` union all /* select#2 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t2`
explain format=tree select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 4)
        -> Nested loop inner join  (cost=2.05 rows=4)
            -> Filter: ((t2.a > 1) and (t2.a is not null))  (cost=0.65 rows=4)
                -> Parallel table scan on t2  (cost=0.65 rows=4)
            -> Single-row index lookup on t3 using PRIMARY (a=t2.a)  (cost=0.28 rows=1)

explain select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers); Using where
1	SIMPLE	t3	NULL	eq_ref	PRIMARY	PRIMARY	4	test.t2.a	1	100.00	Using index
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t3` join `test`.`t2` where ((`test`.`t3`.`a` = `test`.`t2`.`a`) and (`test`.`t2`.`a` > 1))
explain format=json select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1;
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
          "rows_examined_per_scan": 4,
          "rows_produced_per_join": 4,
          "filtered": "100.00",
          "parallel_scan": "4 workers",
          "cost_info": {
            "read_cost": "0.25",
            "eval_cost": "0.40",
            "prefix_cost": "0.65",
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a",
            "b",
            "c",
            "d"
          ],
          "attached_condition": "((`test`.`t2`.`a` > 1) and (`test`.`t2`.`a` is not null))"
        }
      },
      {
        "table": {
          "table_name": "t3",
          "access_type": "eq_ref",
          "possible_keys": [
            "PRIMARY"
          ],
          "key": "PRIMARY",
          "used_key_parts": [
            "a"
          ],
          "key_length": "4",
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
            "data_read_per_join": "96"
          },
          "used_columns": [
            "a"
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
      }
    ]
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t3` join `test`.`t2` where ((`test`.`t3`.`a` = `test`.`t2`.`a`) and (`test`.`t2`.`a` > 1))
explain format=tree select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a;
EXPLAIN
-> Nested loop inner join  (cost=2.05 rows=4)
    -> PX Receiver (merge sort; slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Sort: t2.a  (cost=0.65 rows=4)
                -> Filter: ((t2.a > 1) and (t2.a is not null))  (cost=0.65 rows=4)
                    -> Parallel table scan on t2  (cost=0.65 rows=4)
    -> Single-row index lookup on t3 using PRIMARY (a=t2.a)  (cost=0.28 rows=1)

explain select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Parallel scan (4 workers); Using where; Using filesort
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Receive from (<sender1>); Using merge sort
1	SIMPLE	t3	NULL	eq_ref	PRIMARY	PRIMARY	4	test.t2.a	1	100.00	Using index
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t3` join `test`.`t2` where ((`test`.`t3`.`a` = `test`.`t2`.`a`) and (`test`.`t2`.`a` > 1)) order by `test`.`t2`.`a`
explain format=json select t2.* from t3 join t2 on t3.a = t2.a where t3.a > 1 order by t2.a;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "6.05"
    },
    "ordering_operation": {
      "using_filesort": true,
      "cost_info": {
        "sort_cost": "4.00"
      },
      "nested_loop": [
        {
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
              "data_read_per_join": "96"
            },
            "used_columns": [
              "a",
              "b",
              "c",
              "d"
            ],
            "attached_condition": "((`test`.`t2`.`a` > 1) and (`test`.`t2`.`a` is not null))"
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
            "using_merge_sort": true,
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
            "table_name": "t3",
            "access_type": "eq_ref",
            "possible_keys": [
              "PRIMARY"
            ],
            "key": "PRIMARY",
            "used_key_parts": [
              "a"
            ],
            "key_length": "4",
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
              "data_read_per_join": "96"
            },
            "used_columns": [
              "a"
            ]
          }
        }
      ]
    }
  }
}
Warnings:
Note	1003	/* select#1 */ select `test`.`t2`.`a` AS `a`,`test`.`t2`.`b` AS `b`,`test`.`t2`.`c` AS `c`,`test`.`t2`.`d` AS `d` from `test`.`t3` join `test`.`t2` where ((`test`.`t3`.`a` = `test`.`t2`.`a`) and (`test`.`t2`.`a` > 1)) order by `test`.`t2`.`a`
set @@txsql_parallel_table_record_threshold=5;
explain format=tree select sum(a) from t1 group by b union select variance(a) from t2 group by b;
EXPLAIN
-> PX Receiver (slice: 0; workers: 1)
    -> PX Sender (slice: 1; workers: 1)
        -> Table scan on <union temporary>  (cost=2.50 rows=0)
            -> Union materialize with deduplication
                -> Table scan on <temporary>
                    -> Final Aggregate using temporary table
                        -> PX Receiver (slice: 1; workers: 1)
                            -> PX Sender (slice: 2; workers: 4)
                                -> Table scan on <temporary>
                                    -> Aggregate using temporary table
                                        -> Parallel table scan on t1  (cost=0.95 rows=7)
                -> Table scan on <temporary>
                    -> Aggregate using temporary table
                        -> Table scan on t2  (cost=0.65 rows=4)

explain select sum(a) from t1 group by b union select variance(a) from t2 group by b;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	PRIMARY	t1	NULL	ALL	NULL	NULL	NULL	NULL	7	100.00	Parallel scan (4 workers); Using temporary
1	PRIMARY	<sender2>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Send to (<receiver2>)
1	PRIMARY	<receiver2>	NULL	ALL	NULL	NULL	NULL	NULL	0	0.00	Receive from (<sender2>); Using temporary
2	UNION	t2	NULL	ALL	NULL	NULL	NULL	NULL	4	100.00	Using temporary
NULL	UNION RESULT	<union1,2>	NULL	ALL	NULL	NULL	NULL	NULL	NULL	NULL	Using temporary
Warnings:
Note	1003	/* select#1 */ select sum(`test`.`t1`.`a`) AS `sum(a)` from `test`.`t1` group by `test`.`t1`.`b` union /* select#2 */ select variance(`test`.`t2`.`a`) AS `variance(a)` from `test`.`t2` group by `test`.`t2`.`b`
explain format=json select sum(a) from t1 group by b union select variance(a) from t2 group by b;
EXPLAIN
{
  "query_block": {
    "union_result": {
      "using_temporary_table": true,
      "table_name": "<union1,2>",
      "access_type": "ALL",
      "query_specifications": [
        {
          "dependent": false,
          "cacheable": true,
          "query_block": {
            "select_id": 1,
            "cost_info": {
              "query_cost": "0.95"
            },
            "grouping_operation": {
              "using_temporary_table": true,
              "using_filesort": false,
              "table": {
                "table_name": "t1",
                "access_type": "ALL",
                "rows_examined_per_scan": 7,
                "rows_produced_per_join": 7,
                "filtered": "100.00",
                "parallel_scan": "4 workers",
                "cost_info": {
                  "read_cost": "0.25",
                  "eval_cost": "0.70",
                  "prefix_cost": "0.95",
                  "data_read_per_join": "168"
                },
                "used_columns": [
                  "a",
                  "b"
                ]
              },
              "parallel_information": [
                {
                  "exchange_table": {
                    "table_name": "<sender2>",
                    "access_type": "ALL",
                    "rows_examined_per_scan": 0,
                    "rows_produced_per_join": 0,
                    "filtered": "0.00",
                    "send_to": "<receiver2>",
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
                    "table_name": "<receiver2>",
                    "access_type": "ALL",
                    "rows_examined_per_scan": 0,
                    "rows_produced_per_join": 0,
                    "filtered": "0.00",
                    "receive_from": "<sender2>",
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
        },
        {
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
                  "data_read_per_join": "96"
                },
                "used_columns": [
                  "a",
                  "b"
                ]
              }
            }
          }
        }
      ]
    }
  }
}
Warnings:
Note	1003	/* select#1 */ select sum(`test`.`t1`.`a`) AS `sum(a)` from `test`.`t1` group by `test`.`t1`.`b` union /* select#2 */ select variance(`test`.`t2`.`a`) AS `variance(a)` from `test`.`t2` group by `test`.`t2`.`b`
set @@txsql_parallel_table_record_threshold=0;
# --include set_px_to_saved.inc
drop table t1;
drop table t2;
drop table t3;
drop table t4;
drop database test1;
drop user user1;

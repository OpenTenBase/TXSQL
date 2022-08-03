SET optimizer_trace_max_mem_size=1048576;
SET optimizer_trace="enabled=on,one_line=off";
create table t1 (a int primary key auto_increment, x int, b int, c char(64), key(b));
insert t1 values (1,1,1,'x'),(2,2,2,'x');
insert t1(x,b,c) select x,b,c from t1 where b = 1;
insert t1(x,b,c) select x,b,c from t1 where b = 1;
insert t1(x,b,c) select x,b,c from t1 where b = 1;
insert t1(x,b,c) select x,b,c from t1 where b = 1;
insert t1(x,b,c) select x,b,c from t1 where b = 1;
insert t1(x,b,c) select x,b,c from t1 where b = 2;
insert t1(x,b,c) select x,b,c from t1 where b = 2;
insert t1(x,b,c) select x,b,c from t1 where b = 2;
insert t1(x,b,c) select x,b,c from t1 where b = 2;
analyze table t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
select n_rows,clustered_index_size from mysql.innodb_table_stats where table_name = 't1';
n_rows	clustered_index_size
48	1
select index_name,stat_value,sample_size from mysql.innodb_index_stats where table_name = 't1' and stat_name = 'n_diff_pfx01';
index_name	stat_value	sample_size
PRIMARY	48	1
b	2	1
# With correct stat_n_rows, it is range access.
explain select * from t1 where b >= 2;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	range	b	b	5	NULL	16	100.00	Parallel scan (4 workers); Using index condition
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	16	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	16	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`x` AS `x`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` where (`test`.`t1`.`b` >= 2)
SELECT TRACE FROM information_schema.OPTIMIZER_TRACE;
TRACE
{
  "steps": [
    {
      "join_preparation": {
        "select#": 1,
        "steps": [
          {
            "expanded_query": "/* select#1 */ select `t1`.`a` AS `a`,`t1`.`x` AS `x`,`t1`.`b` AS `b`,`t1`.`c` AS `c` from `t1` where (`t1`.`b` >= 2)"
          }
        ]
      }
    },
    {
      "join_optimization": {
        "select#": 1,
        "steps": [
          {
            "condition_processing": {
              "condition": "WHERE",
              "original_condition": "(`t1`.`b` >= 2)",
              "steps": [
                {
                  "transformation": "equality_propagation",
                  "resulting_condition": "(`t1`.`b` >= 2)"
                },
                {
                  "transformation": "constant_propagation",
                  "resulting_condition": "(`t1`.`b` >= 2)"
                },
                {
                  "transformation": "trivial_condition_removal",
                  "resulting_condition": "(`t1`.`b` >= 2)"
                }
              ]
            }
          },
          {
            "substitute_generated_columns": {
            }
          },
          {
            "table_dependencies": [
              {
                "table": "`t1`",
                "row_may_be_null": false,
                "map_bit": 0,
                "depends_on_map_bits": [
                ]
              }
            ]
          },
          {
            "ref_optimizer_key_uses": [
            ]
          },
          {
            "rows_estimation": [
              {
                "table": "`t1`",
                "range_analysis": {
                  "table_scan": {
                    "rows": 48,
                    "cost": 7.15
                  },
                  "potential_range_indexes": [
                    {
                      "index": "PRIMARY",
                      "usable": false,
                      "cause": "not_applicable"
                    },
                    {
                      "index": "b",
                      "usable": true,
                      "key_parts": [
                        "b",
                        "a"
                      ]
                    }
                  ],
                  "setup_range_conditions": [
                  ],
                  "group_index_range": {
                    "chosen": false,
                    "cause": "not_group_by_or_distinct"
                  },
                  "skip_scan_range": {
                    "potential_skip_scan_indexes": [
                      {
                        "index": "b",
                        "usable": false,
                        "cause": "query_references_nonkey_column"
                      }
                    ]
                  },
                  "analyzing_range_alternatives": {
                    "range_scan_alternatives": [
                      {
                        "index": "b",
                        "ranges": [
                          "2 <= b"
                        ],
                        "index_dives_for_eq_ranges": true,
                        "rowid_ordered": false,
                        "using_mrr": false,
                        "index_only": false,
                        "rows": 16,
                        "cost": 5.86,
                        "chosen": true
                      }
                    ],
                    "analyzing_roworder_intersect": {
                      "usable": false,
                      "cause": "too_few_roworder_scans"
                    }
                  },
                  "chosen_range_access_summary": {
                    "range_access_plan": {
                      "type": "range_scan",
                      "index": "b",
                      "rows": 16,
                      "ranges": [
                        "2 <= b"
                      ]
                    },
                    "rows_for_plan": 16,
                    "cost_for_plan": 5.86,
                    "chosen": true
                  }
                }
              }
            ]
          },
          {
            "considered_execution_plans": [
              {
                "plan_prefix": [
                ],
                "table": "`t1`",
                "best_access_path": {
                  "considered_access_paths": [
                    {
                      "rows_to_scan": 16,
                      "filtering_effect": [
                      ],
                      "final_filtering_effect": 1,
                      "access_type": "range",
                      "range_details": {
                        "used_index": "b"
                      },
                      "resulting_rows": 16,
                      "cost": 7.46,
                      "chosen": true
                    }
                  ]
                },
                "condition_filtering_pct": 100,
                "rows_for_plan": 16,
                "cost_for_plan": 7.46,
                "chosen": true
              }
            ]
          },
          {
            "attaching_conditions_to_tables": {
              "original_condition": "(`t1`.`b` >= 2)",
              "attached_conditions_computation": [
              ],
              "attached_conditions_summary": [
                {
                  "table": "`t1`",
                  "attached": "(`t1`.`b` >= 2)"
                }
              ]
            }
          },
          {
            "finalizing_table_conditions": [
              {
                "table": "`t1`",
                "original_table_condition": "(`t1`.`b` >= 2)",
                "final_table_condition   ": "(`t1`.`b` >= 2)"
              }
            ]
          },
          {
            "refine_plan": [
              {
                "table": "`t1`",
                "pushed_index_condition": "(`t1`.`b` >= 2)",
                "table_condition_attached": null
              }
            ]
          }
        ]
      }
    },
    "...",
    {
      "join_explain": {
        "select#": 1,
        "steps": [
        ]
      }
    }
  ]
}
SET DEBUG_SYNC = "innodb_inplace_alter_table_enter signal ddl_in_progress wait_for finish_ddl execute 1";
alter table t1 drop column x;;
SET DEBUG_SYNC = "now wait_for ddl_in_progress";
analyze table t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
select n_rows,clustered_index_size from mysql.innodb_table_stats where table_name = 't1';
n_rows	clustered_index_size
48	1
select index_name,stat_value,sample_size from mysql.innodb_index_stats where table_name = 't1' and stat_name = 'n_diff_pfx01';
index_name	stat_value	sample_size
PRIMARY	48	1
b	2	1
# With zero stat_n_rows, it is table scan (2.45) even if the real table might be very big.
# With correct stat_n_rows, it is range access.
explain select * from t1 where b >= 2;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	range	b	b	5	NULL	16	100.00	Parallel scan (4 workers); Using index condition
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	16	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	16	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`x` AS `x`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` where (`test`.`t1`.`b` >= 2)
SELECT TRACE FROM information_schema.OPTIMIZER_TRACE;
TRACE
{
  "steps": [
    {
      "join_preparation": {
        "select#": 1,
        "steps": [
          {
            "expanded_query": "/* select#1 */ select `t1`.`a` AS `a`,`t1`.`x` AS `x`,`t1`.`b` AS `b`,`t1`.`c` AS `c` from `t1` where (`t1`.`b` >= 2)"
          }
        ]
      }
    },
    {
      "join_optimization": {
        "select#": 1,
        "steps": [
          {
            "condition_processing": {
              "condition": "WHERE",
              "original_condition": "(`t1`.`b` >= 2)",
              "steps": [
                {
                  "transformation": "equality_propagation",
                  "resulting_condition": "(`t1`.`b` >= 2)"
                },
                {
                  "transformation": "constant_propagation",
                  "resulting_condition": "(`t1`.`b` >= 2)"
                },
                {
                  "transformation": "trivial_condition_removal",
                  "resulting_condition": "(`t1`.`b` >= 2)"
                }
              ]
            }
          },
          {
            "substitute_generated_columns": {
            }
          },
          {
            "table_dependencies": [
              {
                "table": "`t1`",
                "row_may_be_null": false,
                "map_bit": 0,
                "depends_on_map_bits": [
                ]
              }
            ]
          },
          {
            "ref_optimizer_key_uses": [
            ]
          },
          {
            "rows_estimation": [
              {
                "table": "`t1`",
                "range_analysis": {
                  "table_scan": {
                    "rows": 48,
                    "cost": 7.15
                  },
                  "potential_range_indexes": [
                    {
                      "index": "PRIMARY",
                      "usable": false,
                      "cause": "not_applicable"
                    },
                    {
                      "index": "b",
                      "usable": true,
                      "key_parts": [
                        "b",
                        "a"
                      ]
                    }
                  ],
                  "setup_range_conditions": [
                  ],
                  "group_index_range": {
                    "chosen": false,
                    "cause": "not_group_by_or_distinct"
                  },
                  "skip_scan_range": {
                    "potential_skip_scan_indexes": [
                      {
                        "index": "b",
                        "usable": false,
                        "cause": "query_references_nonkey_column"
                      }
                    ]
                  },
                  "analyzing_range_alternatives": {
                    "range_scan_alternatives": [
                      {
                        "index": "b",
                        "ranges": [
                          "2 <= b"
                        ],
                        "index_dives_for_eq_ranges": true,
                        "rowid_ordered": false,
                        "using_mrr": false,
                        "index_only": false,
                        "rows": 16,
                        "cost": 5.86,
                        "chosen": true
                      }
                    ],
                    "analyzing_roworder_intersect": {
                      "usable": false,
                      "cause": "too_few_roworder_scans"
                    }
                  },
                  "chosen_range_access_summary": {
                    "range_access_plan": {
                      "type": "range_scan",
                      "index": "b",
                      "rows": 16,
                      "ranges": [
                        "2 <= b"
                      ]
                    },
                    "rows_for_plan": 16,
                    "cost_for_plan": 5.86,
                    "chosen": true
                  }
                }
              }
            ]
          },
          {
            "considered_execution_plans": [
              {
                "plan_prefix": [
                ],
                "table": "`t1`",
                "best_access_path": {
                  "considered_access_paths": [
                    {
                      "rows_to_scan": 16,
                      "filtering_effect": [
                      ],
                      "final_filtering_effect": 1,
                      "access_type": "range",
                      "range_details": {
                        "used_index": "b"
                      },
                      "resulting_rows": 16,
                      "cost": 7.46,
                      "chosen": true
                    }
                  ]
                },
                "condition_filtering_pct": 100,
                "rows_for_plan": 16,
                "cost_for_plan": 7.46,
                "chosen": true
              }
            ]
          },
          {
            "attaching_conditions_to_tables": {
              "original_condition": "(`t1`.`b` >= 2)",
              "attached_conditions_computation": [
              ],
              "attached_conditions_summary": [
                {
                  "table": "`t1`",
                  "attached": "(`t1`.`b` >= 2)"
                }
              ]
            }
          },
          {
            "finalizing_table_conditions": [
              {
                "table": "`t1`",
                "original_table_condition": "(`t1`.`b` >= 2)",
                "final_table_condition   ": "(`t1`.`b` >= 2)"
              }
            ]
          },
          {
            "refine_plan": [
              {
                "table": "`t1`",
                "pushed_index_condition": "(`t1`.`b` >= 2)",
                "table_condition_attached": null
              }
            ]
          }
        ]
      }
    },
    "...",
    {
      "join_explain": {
        "select#": 1,
        "steps": [
        ]
      }
    }
  ]
}
SET DEBUG_SYNC = "now signal finish_ddl";
select n_rows,clustered_index_size from mysql.innodb_table_stats where table_name = 't1';
n_rows	clustered_index_size
48	1
select index_name,stat_value,sample_size from mysql.innodb_index_stats where table_name = 't1' and stat_name = 'n_diff_pfx01';
index_name	stat_value	sample_size
PRIMARY	48	1
b	2	1
# With stat_n_rows corrected by the completion of the online alter, it is range access again.
explain select * from t1 where b >= 2;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	range	b	b	5	NULL	16	100.00	Parallel scan (4 workers); Using index condition
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	16	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	16	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,`test`.`t1`.`b` AS `b`,`test`.`t1`.`c` AS `c` from `test`.`t1` where (`test`.`t1`.`b` >= 2)
drop table t1;
SET DEBUG_SYNC = 'RESET';

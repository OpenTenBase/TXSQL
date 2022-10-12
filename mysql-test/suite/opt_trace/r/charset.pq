set @@session.end_markers_in_json=on;
set @@session.optimizer_trace="enabled=on";
create table t1(a int);
insert into t1 values(1);
set names latin1;
explain select '¡¬√ƒ≈', _latin1'¡¬√ƒ≈' from t1 limit 1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	NULL
Warnings:
Note	1003	/* select#1 */ select '¡¬√ƒ≈' AS `¡¬√ƒ≈`,_latin1'\xC1\xC2\xC3\xC4\xC5' AS `¡¬√ƒ≈` from `test`.`t1` limit 1
select (@query:=QUERY)+NULL, (@trace:=TRACE)+NULL from information_schema.OPTIMIZER_TRACE;
(@query:=QUERY)+NULL	(@trace:=TRACE)+NULL
NULL	NULL
select @query, @trace;
@query	@trace
                      "access_type": "scan",
                      "chosen": true
                      "cost": 0.35,
                      "resulting_rows": 1,
                      "rows_to_scan": 1,
                    {
                    }
                  "attached": null
                  "considered_access_paths": [
                  "cost": 0.25
                  "rows": 1,
                  "table": "`t1`",
                  ] /* considered_access_paths */
                "best_access_path": {
                "chosen": true
                "condition_filtering_pct": 100,
                "cost_for_plan": 0.35,
                "depends_on_map_bits": [
                "map_bit": 0,
                "plan_prefix": [
                "row_may_be_null": false,
                "rows_for_plan": 1,
                "table": "`t1`"
                "table": "`t1`",
                "table": "`t1`",
                "table": "`t1`",
                "table_scan": {
                ] /* depends_on_map_bits */
                ] /* plan_prefix */,
                {
                }
                } /* best_access_path */,
                } /* table_scan */
              "attached_conditions_computation": [
              "attached_conditions_summary": [
              "original_condition": null,
              ] /* attached_conditions_computation */,
              ] /* attached_conditions_summary */
              {
              {
              {
              {
              }
              }
              }
              }
            "attaching_conditions_to_tables": {
            "considered_execution_plans": [
            "expanded_query": "/* select#1 */ select '¡¬√ƒ≈' AS `¡¬√ƒ≈`,_latin1'\\xC1\\xC2\\xC3\\xC4\\xC5' AS `¡¬√ƒ≈` from `t1` limit 1"
            "finalizing_table_conditions": [
            "refine_plan": [
            "rows_estimation": [
            "table_dependencies": [
            ] /* considered_execution_plans */
            ] /* finalizing_table_conditions */
            ] /* refine_plan */
            ] /* rows_estimation */
            ] /* table_dependencies */
            } /* attaching_conditions_to_tables */
          {
          {
          {
          {
          {
          {
          {
          }
          }
          },
          },
          },
          },
          },
        "select#": 1,
        "select#": 1,
        "select#": 1,
        "steps": [
        "steps": [
        "steps": [
        ] /* steps */
        ] /* steps */
        ] /* steps */
      "join_explain": {
      "join_optimization": {
      "join_preparation": {
      } /* join_explain */
      } /* join_optimization */
      } /* join_preparation */
    {
    {
    {
    }
    },
    },
  "steps": [
  ] /* steps */
explain select '¡¬√ƒ≈', _latin1'¡¬√ƒ≈' from t1 limit 1	{
}
set names utf8;
Warnings:
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
select @query, @trace;
@query	@trace
                      "access_type": "scan",
                      "chosen": true
                      "cost": 0.35,
                      "resulting_rows": 1,
                      "rows_to_scan": 1,
                    {
                    }
                  "attached": null
                  "considered_access_paths": [
                  "cost": 0.25
                  "rows": 1,
                  "table": "`t1`",
                  ] /* considered_access_paths */
                "best_access_path": {
                "chosen": true
                "condition_filtering_pct": 100,
                "cost_for_plan": 0.35,
                "depends_on_map_bits": [
                "map_bit": 0,
                "plan_prefix": [
                "row_may_be_null": false,
                "rows_for_plan": 1,
                "table": "`t1`"
                "table": "`t1`",
                "table": "`t1`",
                "table": "`t1`",
                "table_scan": {
                ] /* depends_on_map_bits */
                ] /* plan_prefix */,
                {
                }
                } /* best_access_path */,
                } /* table_scan */
              "attached_conditions_computation": [
              "attached_conditions_summary": [
              "original_condition": null,
              ] /* attached_conditions_computation */,
              ] /* attached_conditions_summary */
              {
              {
              {
              {
              }
              }
              }
              }
            "attaching_conditions_to_tables": {
            "considered_execution_plans": [
            "expanded_query": "/* select#1 */ select '√Å√Ç√É√Ñ√Ö' AS `√Å√Ç√É√Ñ√Ö`,_latin1'\\xC1\\xC2\\xC3\\xC4\\xC5' AS `√Å√Ç√É√Ñ√Ö` from `t1` limit 1"
            "finalizing_table_conditions": [
            "refine_plan": [
            "rows_estimation": [
            "table_dependencies": [
            ] /* considered_execution_plans */
            ] /* finalizing_table_conditions */
            ] /* refine_plan */
            ] /* rows_estimation */
            ] /* table_dependencies */
            } /* attaching_conditions_to_tables */
          {
          {
          {
          {
          {
          {
          {
          }
          }
          },
          },
          },
          },
          },
        "select#": 1,
        "select#": 1,
        "select#": 1,
        "steps": [
        "steps": [
        "steps": [
        ] /* steps */
        ] /* steps */
        ] /* steps */
      "join_explain": {
      "join_optimization": {
      "join_preparation": {
      } /* join_explain */
      } /* join_optimization */
      } /* join_preparation */
    {
    {
    {
    }
    },
    },
  "steps": [
  ] /* steps */
explain select '√Å√Ç√É√Ñ√Ö', _latin1'√Å√Ç√É√Ñ√Ö' from t1 limit 1	{
}
set names latin1;
create view v1 as select '¡¬√ƒ≈' as col from t1 limit 1;
select * from v1 where v1.col = '¡¬√ƒ≈';
col
¡¬√ƒ≈
select (@query:=QUERY)+NULL, (@trace:=TRACE)+NULL from information_schema.OPTIMIZER_TRACE;
(@query:=QUERY)+NULL	(@trace:=TRACE)+NULL
NULL	NULL
set names utf8;
Warnings:
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
select @query, @trace;
@query	@trace
                            "access_type": "scan",
                            "chosen": true
                            "cost": 0.35,
                            "resulting_rows": 1,
                            "rows_to_scan": 1,
                          {
                          }
                        "attached": null
                        "considered_access_paths": [
                        "cost": 0.25
                        "rows": 1,
                        "table": "`t1`",
                        ] /* considered_access_paths */
                      "best_access_path": {
                      "chosen": true
                      "condition_filtering_pct": 100,
                      "cost_for_plan": 0.35,
                      "depends_on_map_bits": [
                      "map_bit": 0,
                      "plan_prefix": [
                      "row_may_be_null": false,
                      "rows_for_plan": 1,
                      "table": "`t1`"
                      "table": "`t1`",
                      "table": "`t1`",
                      "table": "`t1`",
                      "table_scan": {
                      ] /* depends_on_map_bits */
                      ] /* plan_prefix */,
                      {
                      }
                      } /* best_access_path */,
                      } /* table_scan */
                    "attached_conditions_computation": [
                    "attached_conditions_summary": [
                    "original_condition": null,
                    ] /* attached_conditions_computation */,
                    ] /* attached_conditions_summary */
                    {
                    {
                    {
                    {
                    }
                    }
                    }
                    }
                  "attaching_conditions_to_tables": {
                  "considered_execution_plans": [
                  "expanded_query": "/* select#2 */ select '√Å√Ç√É√Ñ√Ö' AS `col` from `t1` limit 1"
                  "finalizing_table_conditions": [
                  "refine_plan": [
                  "resulting_condition": "multiple equal('√Å√Ç√É√Ñ√Ö', `v1`.`col`)"
                  "resulting_condition": "multiple equal('√Å√Ç√É√Ñ√Ö', `v1`.`col`)"
                  "resulting_condition": "multiple equal('√Å√Ç√É√Ñ√Ö', `v1`.`col`)"
                  "rows_estimation": [
                  "table_dependencies": [
                  "transformation": "constant_propagation",
                  "transformation": "equality_propagation",
                  "transformation": "trivial_condition_removal",
                  ] /* considered_execution_plans */
                  ] /* finalizing_table_conditions */
                  ] /* refine_plan */
                  ] /* rows_estimation */
                  ] /* table_dependencies */
                  } /* attaching_conditions_to_tables */
                "cannot_insert_duplicates": false,
                "columns": 1,
                "cost": 1,
                "depends_on_map_bits": [
                "empty": false
                "key_length": 0,
                "location": "TempTable"
                "makes_grouped_rows": false,
                "map_bit": 0,
                "row_length": 7,
                "row_may_be_null": false,
                "rows": 1,
                "table": "`v1`",
                "table": "`v1`",
                "table": "`v1`",
                "table_type": "system",
                "unique_constraint": false,
                ] /* depends_on_map_bits */
                {
                {
                {
                {
                {
                {
                {
                {
                {
                {
                }
                }
                }
                },
                },
                },
                },
                },
                },
                },
              "attached_conditions_computation": [
              "attached_conditions_summary": [
              "condition": "WHERE",
              "materialized": true
              "original_condition": "(`v1`.`col` = '√Å√Ç√É√Ñ√Ö')",
              "original_condition": "true",
              "select#": 2,
              "select#": 2,
              "select#": 2,
              "select#": 2,
              "steps": [
              "steps": [
              "steps": [
              "steps": [
              "table": "`v1`",
              "tmp_table_info": {
              ] /* attached_conditions_computation */,
              ] /* attached_conditions_summary */
              ] /* steps */
              ] /* steps */
              ] /* steps */
              ] /* steps */
              {
              {
              }
              }
              } /* tmp_table_info */
            "attaching_conditions_to_tables": {
            "condition_on_constant_tables": "true",
            "condition_processing": {
            "condition_value": true
            "creating_tmp_table": {
            "expanded_query": "/* select#1 */ select `v1`.`col` AS `col` from `v1` where (`v1`.`col` = '√Å√Ç√É√Ñ√Ö')"
            "join_execution": {
            "join_optimization": {
            "join_preparation": {
            "ref_optimizer_key_uses": [
            "refine_plan": [
            "rows_estimation": [
            "substitute_generated_columns": {
            "table_dependencies": [
            "view": {
            ] /* ref_optimizer_key_uses */
            ] /* refine_plan */
            ] /* rows_estimation */
            ] /* table_dependencies */
            } /* attaching_conditions_to_tables */
            } /* condition_processing */
            } /* creating_tmp_table */
            } /* join_execution */
            } /* join_optimization */
            } /* join_preparation */
            } /* substitute_generated_columns */
            } /* view */
          {
          {
          {
          {
          {
          {
          {
          {
          {
          {
          {
          {
          {
          {
          }
          }
          },
          },
          },
          },
          },
          },
          },
          },
          },
          },
          },
          },
        "select#": 1,
        "select#": 1,
        "select#": 1,
        "steps": [
        "steps": [
        "steps": [
        ] /* steps */
        ] /* steps */
        ] /* steps */
      "join_execution": {
      "join_optimization": {
      "join_preparation": {
      } /* join_execution */
      } /* join_optimization */
      } /* join_preparation */
    {
    {
    {
    }
    },
    },
  "steps": [
  ] /* steps */
select * from v1 where v1.col = '√Å√Ç√É√Ñ√Ö'	{
}
drop table t1;
drop view v1;
set names latin1;
create table t1(c char(4) primary key) charset latin1;
insert into t1 values ('aaa'), ('¡¬√ƒ');
select * from t1 where c < '¡¬√ƒ≈';
c
aaa
¡¬√ƒ
select (@query:=QUERY)+NULL, (@trace:=TRACE)+NULL from information_schema.OPTIMIZER_TRACE;
(@query:=QUERY)+NULL	(@trace:=TRACE)+NULL
NULL	NULL
set names utf8;
Warnings:
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
select @query, @trace;
@query	@trace
                          "c <= '√Å√Ç√É√Ñ'"
                        "c"
                        "cause": "cost"
                        "cause": "prefix_not_const_equality"
                        "chosen": false,
                        "cost": 0.71,
                        "in_memory": 1,
                        "index": "PRIMARY",
                        "index": "PRIMARY",
                        "index_dives_for_eq_ranges": true,
                        "index_only": true,
                        "ranges": [
                        "rowid_ordered": true,
                        "rows": 2,
                        "usable": false,
                        "using_mrr": false,
                        ] /* ranges */,
                      "access_type": "scan",
                      "cause": "too_few_roworder_scans"
                      "chosen": true
                      "cost": 0.45,
                      "index": "PRIMARY",
                      "key_parts": [
                      "resulting_rows": 2,
                      "rows_to_scan": 2,
                      "usable": false,
                      "usable": true,
                      ] /* key_parts */
                      {
                      {
                      }
                      }
                    "analyzing_roworder_intersect": {
                    "cause": "not_group_by_or_distinct"
                    "chosen": false,
                    "chosen": true
                    "cost": 0.450244,
                    "cost": 2.55
                    "index": "PRIMARY",
                    "potential_skip_scan_indexes": [
                    "range_scan_alternatives": [
                    "rows": 2,
                    ] /* potential_skip_scan_indexes */
                    ] /* range_scan_alternatives */,
                    {
                    {
                    }
                    }
                    } /* analyzing_roworder_intersect */
                  "analyzing_range_alternatives": {
                  "attached": "(`t1`.`c` < '√Å√Ç√É√Ñ√Ö')"
                  "best_covering_index_scan": {
                  "considered_access_paths": [
                  "group_index_range": {
                  "potential_range_indexes": [
                  "resulting_condition": "(`t1`.`c` < '√Å√Ç√É√Ñ√Ö')"
                  "resulting_condition": "(`t1`.`c` < '√Å√Ç√É√Ñ√Ö')"
                  "resulting_condition": "(`t1`.`c` < '√Å√Ç√É√Ñ√Ö')"
                  "setup_range_conditions": [
                  "skip_scan_range": {
                  "table": "`t1`",
                  "table_scan": {
                  "transformation": "constant_propagation",
                  "transformation": "equality_propagation",
                  "transformation": "trivial_condition_removal",
                  ] /* considered_access_paths */
                  ] /* potential_range_indexes */,
                  ] /* setup_range_conditions */,
                  } /* analyzing_range_alternatives */
                  } /* best_covering_index_scan */,
                  } /* group_index_range */,
                  } /* skip_scan_range */,
                  } /* table_scan */,
                "best_access_path": {
                "chosen": true
                "condition_filtering_pct": 100,
                "cost_for_plan": 0.45,
                "depends_on_map_bits": [
                "final_table_condition   ": "(`t1`.`c` < '√Å√Ç√É√Ñ√Ö')"
                "map_bit": 0,
                "original_table_condition": "(`t1`.`c` < '√Å√Ç√É√Ñ√Ö')",
                "plan_prefix": [
                "range_analysis": {
                "row_may_be_null": false,
                "rows_for_plan": 2,
                "table": "`t1`"
                "table": "`t1`",
                "table": "`t1`",
                "table": "`t1`",
                "table": "`t1`",
                ] /* depends_on_map_bits */
                ] /* plan_prefix */,
                {
                {
                {
                {
                }
                }
                } /* best_access_path */,
                } /* range_analysis */
                },
                },
              "attached_conditions_computation": [
              "attached_conditions_summary": [
              "condition": "WHERE",
              "original_condition": "(`t1`.`c` < '√Å√Ç√É√Ñ√Ö')",
              "original_condition": "(`t1`.`c` < '√Å√Ç√É√Ñ√Ö')",
              "steps": [
              ] /* attached_conditions_computation */,
              ] /* attached_conditions_summary */
              ] /* steps */
              {
              {
              {
              {
              {
              }
              }
              }
              }
              }
            "attaching_conditions_to_tables": {
            "condition_processing": {
            "considered_execution_plans": [
            "expanded_query": "/* select#1 */ select `t1`.`c` AS `c` from `t1` where (`t1`.`c` < '√Å√Ç√É√Ñ√Ö')"
            "finalizing_table_conditions": [
            "ref_optimizer_key_uses": [
            "refine_plan": [
            "rows_estimation": [
            "substitute_generated_columns": {
            "table_dependencies": [
            ] /* considered_execution_plans */
            ] /* finalizing_table_conditions */
            ] /* ref_optimizer_key_uses */
            ] /* refine_plan */
            ] /* rows_estimation */
            ] /* table_dependencies */
            } /* attaching_conditions_to_tables */
            } /* condition_processing */
            } /* substitute_generated_columns */
          {
          {
          {
          {
          {
          {
          {
          {
          {
          {
          }
          }
          },
          },
          },
          },
          },
          },
          },
          },
        "select#": 1,
        "select#": 1,
        "steps": [
        "steps": [
        "steps": [
        ] /* steps */
        ] /* steps */
        ] /* steps */
      "join_execution": {
      "join_optimization": {
      "join_preparation": {
      } /* join_execution */
      } /* join_optimization */
      } /* join_preparation */
    "...",
    {
    {
    {
    }
    },
    },
  "steps": [
  ] /* steps */
select * from t1 where c < '√Å√Ç√É√Ñ√Ö'	{
}
drop table t1;
create table `t
1	a`(`col
1	a` int, index `index
1	a` (`col
1	a`));
insert into `t
1	a` values(2),(3);
create view `v
1	a` as select * from `t
1	a`;
select `col
1	a` from `v
1	a` where `col
1	a` < 6;
col
1	a
2
3
select query, trace from information_schema.OPTIMIZER_TRACE;
query	trace
select `col
1	a` from `v
1	a` where `col
1	a` < 6	{
  "steps": [
    {
      "join_preparation": {
        "select#": 1,
        "steps": [
          {
            "join_preparation": {
              "select#": 2,
              "steps": [
                {
                  "expanded_query": "/* select#2 */ select `t\n1\ta`.`col\n1\ta` AS `col\n1\ta` from `t\n1\ta`"
                }
              ] /* steps */
            } /* join_preparation */
          },
          {
            "view": {
              "table": "`v\n1\ta`",
              "select#": 2,
              "merged": true
            } /* view */
          },
          {
            "expanded_query": "/* select#1 */ select `t\n1\ta`.`col\n1\ta` AS `col\n1\ta` from (`t\n1\ta`) where (`t\n1\ta`.`col\n1\ta` < 6)"
          },
          {
            "transformations_to_nested_joins": {
              "transformations": [
                "parenthesis_removal"
              ] /* transformations */,
              "expanded_query": "/* select#1 */ select `t\n1\ta`.`col\n1\ta` AS `col\n1\ta` from `t\n1\ta` where (`t\n1\ta`.`col\n1\ta` < 6)"
            } /* transformations_to_nested_joins */
          }
        ] /* steps */
      } /* join_preparation */
    },
    {
      "join_optimization": {
        "select#": 1,
        "steps": [
          {
            "condition_processing": {
              "condition": "WHERE",
              "original_condition": "(`t\n1\ta`.`col\n1\ta` < 6)",
              "steps": [
                {
                  "transformation": "equality_propagation",
                  "resulting_condition": "(`t\n1\ta`.`col\n1\ta` < 6)"
                },
                {
                  "transformation": "constant_propagation",
                  "resulting_condition": "(`t\n1\ta`.`col\n1\ta` < 6)"
                },
                {
                  "transformation": "trivial_condition_removal",
                  "resulting_condition": "(`t\n1\ta`.`col\n1\ta` < 6)"
                }
              ] /* steps */
            } /* condition_processing */
          },
          {
            "substitute_generated_columns": {
            } /* substitute_generated_columns */
          },
          {
            "table_dependencies": [
              {
                "table": "`t\n1\ta`",
                "row_may_be_null": false,
                "map_bit": 0,
                "depends_on_map_bits": [
                ] /* depends_on_map_bits */
              }
            ] /* table_dependencies */
          },
          {
            "ref_optimizer_key_uses": [
            ] /* ref_optimizer_key_uses */
          },
          {
            "rows_estimation": [
              {
                "table": "`t\n1\ta`",
                "range_analysis": {
                  "table_scan": {
                    "rows": 2,
                    "cost": 2.55
                  } /* table_scan */,
                  "potential_range_indexes": [
                    {
                      "index": "index\n1\ta",
                      "usable": true,
                      "key_parts": [
                        "col\n1\ta"
                      ] /* key_parts */
                    }
                  ] /* potential_range_indexes */,
                  "best_covering_index_scan": {
                    "index": "index\n1\ta",
                    "cost": 0.450336,
                    "chosen": true
                  } /* best_covering_index_scan */,
                  "setup_range_conditions": [
                  ] /* setup_range_conditions */,
                  "group_index_range": {
                    "chosen": false,
                    "cause": "not_group_by_or_distinct"
                  } /* group_index_range */,
                  "skip_scan_range": {
                    "potential_skip_scan_indexes": [
                      {
                        "index": "index\n1\ta",
                        "usable": false,
                        "cause": "prefix_not_const_equality"
                      }
                    ] /* potential_skip_scan_indexes */
                  } /* skip_scan_range */,
                  "analyzing_range_alternatives": {
                    "range_scan_alternatives": [
                      {
                        "index": "index\n1\ta",
                        "ranges": [
                          "NULL < col\n1\ta < 6"
                        ] /* ranges */,
                        "index_dives_for_eq_ranges": true,
                        "rowid_ordered": false,
                        "using_mrr": false,
                        "index_only": true,
                        "in_memory": 1,
                        "rows": 2,
                        "cost": 0.460336,
                        "chosen": false,
                        "cause": "cost"
                      }
                    ] /* range_scan_alternatives */,
                    "analyzing_roworder_intersect": {
                      "usable": false,
                      "cause": "too_few_roworder_scans"
                    } /* analyzing_roworder_intersect */
                  } /* analyzing_range_alternatives */
                } /* range_analysis */
              }
            ] /* rows_estimation */
          },
          {
            "considered_execution_plans": [
              {
                "plan_prefix": [
                ] /* plan_prefix */,
                "table": "`t\n1\ta`",
                "best_access_path": {
                  "considered_access_paths": [
                    {
                      "rows_to_scan": 2,
                      "access_type": "scan",
                      "resulting_rows": 2,
                      "cost": 0.45,
                      "chosen": true
                    }
                  ] /* considered_access_paths */
                } /* best_access_path */,
                "condition_filtering_pct": 100,
                "rows_for_plan": 2,
                "cost_for_plan": 0.45,
                "chosen": true
              }
            ] /* considered_execution_plans */
          },
          {
            "attaching_conditions_to_tables": {
              "original_condition": "(`t\n1\ta`.`col\n1\ta` < 6)",
              "attached_conditions_computation": [
              ] /* attached_conditions_computation */,
              "attached_conditions_summary": [
                {
                  "table": "`t\n1\ta`",
                  "attached": "(`t\n1\ta`.`col\n1\ta` < 6)"
                }
              ] /* attached_conditions_summary */
            } /* attaching_conditions_to_tables */
          },
          {
            "finalizing_table_conditions": [
              {
                "table": "`t\n1\ta`",
                "original_table_condition": "(`t\n1\ta`.`col\n1\ta` < 6)",
                "final_table_condition   ": "(`t\n1\ta`.`col\n1\ta` < 6)"
              }
            ] /* finalizing_table_conditions */
          },
          {
            "refine_plan": [
              {
                "table": "`t\n1\ta`"
              }
            ] /* refine_plan */
          }
        ] /* steps */
      } /* join_optimization */
    },
    "...",
    {
      "join_execution": {
        "steps": [
        ] /* steps */
      } /* join_execution */
    }
  ] /* steps */
}
drop table `t
1	a`;
drop view `v
1	a`;
set optimizer_trace=default;

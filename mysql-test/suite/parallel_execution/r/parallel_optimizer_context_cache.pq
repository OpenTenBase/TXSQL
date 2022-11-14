###############################################
# pq optimization context copy test
###############################################
# 1) test reading handler::stats
# 2) test reading the value of index dive and handler::stats
# 3) test optimization context with DETERMINISTIC FUNCTION
# 4) test optimization context with sub stmt
# 5) test copying resource of now()
# 6) test cache status counter
# 7) test copying resource of user()
# 8) test copying resource of last insert id
# 9) test ror scan of index dive cache
# 10) test copying user vars

# 1) test reading const table and handler::stats

CREATE TABLE t1 (
f1 varchar(16) collate latin1_swedish_ci PRIMARY KEY,
f2 varchar(16) collate latin1_swedish_ci
);
CREATE TABLE t2 (
f1 varchar(16) collate latin1_swedish_ci PRIMARY KEY,
f3 varchar(16) collate latin1_swedish_ci
);
INSERT INTO t1 VALUES ('bla','blah');
INSERT INTO t1 VALUES ('blc','blad');
INSERT INTO t1 VALUES ('bld','blad');
INSERT INTO t2 VALUES ('bla','sheep');
INSERT INTO t2 VALUES ('blc','sheep');
INSERT INTO t2 VALUES ('bld','sheep');
SET optimizer_trace="enabled=on";
SET optimizer_trace_features="statistics=on";
SELECT * FROM t1 LEFT JOIN t2 USING(f1) WHERE f1='bla';
f1	f2	f3
bla	blah	sheep
SELECT * FROM information_schema.optimizer_trace;
QUERY	TRACE	MISSING_BYTES_BEYOND_MAX_MEM_SIZE	INSUFFICIENT_PRIVILEGES
SELECT * FROM t1 LEFT JOIN t2 USING(f1) WHERE f1='bla'	{
  "steps": [
    {
      "join_preparation": {
        "select#": 1,
        "steps": [
          {
            "expanded_query": "/* select#1 */ select `t1`.`f1` AS `f1`,`t1`.`f2` AS `f2`,`t2`.`f3` AS `f3` from (`t1` left join `t2` on((`t1`.`f1` = `t2`.`f1`))) where (`t1`.`f1` = 'bla')"
          },
          {
            "transformations_to_nested_joins": {
              "transformations": [
                "parenthesis_removal"
              ],
              "expanded_query": "/* select#1 */ select `t1`.`f1` AS `f1`,`t1`.`f2` AS `f2`,`t2`.`f3` AS `f3` from `t1` left join `t2` on((`t1`.`f1` = `t2`.`f1`)) where (`t1`.`f1` = 'bla')"
            }
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
              "original_condition": "(`t1`.`f1` = 'bla')",
              "steps": [
                {
                  "transformation": "equality_propagation",
                  "resulting_condition": "multiple equal('bla', `t1`.`f1`)"
                },
                {
                  "transformation": "constant_propagation",
                  "resulting_condition": "multiple equal('bla', `t1`.`f1`)"
                },
                {
                  "transformation": "trivial_condition_removal",
                  "resulting_condition": "multiple equal('bla', `t1`.`f1`)"
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
                "table=***
                "row_may_be_null": false,
                "map_bit": 0,
                "depends_on_map_bits": [
                ]
              },
              {
                "table=***
                "row_may_be_null": true,
                "map_bit": 1,
                "depends_on_map_bits": [
                  0
                ]
              }
            ]
          },
          {
            "ref_optimizer_key_uses": [
              {
                "table=***
                "field": "f1",
                "equals": "'bla'",
                "null_rejecting": true
              },
              {
                "table=***
                "field": "f1",
                "equals": "'bla'",
                "null_rejecting": true
              }
            ]
          },
          {
            "rows_estimation": [
              {
                "table=***
                "rows": 1,
                "cost=***
                "table_type": "const",
                "empty": false
              },
              {
                "table=***
                "rows": 1,
                "cost=***
                "table_type": "const",
                "empty": false
              }
            ]
          },
          {
            "condition_on_constant_tables": "true",
            "condition_value": true
          },
          {
            "attaching_conditions_to_tables": {
              "original_condition": "true",
              "attached_conditions_computation": [
              ],
              "attached_conditions_summary": [
              ]
            }
          },
          {
            "refine_plan": [
            ]
          }
        ]
      }
    },
    {
      "join_execution": {
        "select#": 1,
        "steps": [
        ]
      }
    }
  ]
}	0	0
SET optimizer_trace="enabled=off";
DROP TABLE t1,t2;

# 2) test reading the value of index dive and handler::stats

CREATE TABLE t2 (
auto int not null auto_increment,
fld1 int(6) unsigned zerofill DEFAULT '000000' NOT NULL,
companynr tinyint(2) unsigned zerofill DEFAULT '00' NOT NULL,
fld3 char(30) DEFAULT '' NOT NULL,
fld4 char(35) DEFAULT '' NOT NULL,
fld5 char(35) DEFAULT '' NOT NULL,
fld6 char(4) DEFAULT '' NOT NULL,
UNIQUE fld1 (fld1),
KEY fld3 (fld3),
PRIMARY KEY (auto)
);
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
create table t3 (
period    int not null,
name      char(32) not null,
companynr int not null,
price     double(11,0),
price2     double(11,0),
key (period),
key (name)
);
Warnings:
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
Warnings:
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
Warning	1681	Specifying number of digits for floating point data types is deprecated and will be removed in a future release.
SET optimizer_trace="enabled=on";
SET optimizer_trace_features="statistics=on";
select t2.fld3,companynr from t2 where companynr = 57+1 order by fld3;
fld3	companynr
SELECT * FROM information_schema.optimizer_trace;
QUERY	TRACE	MISSING_BYTES_BEYOND_MAX_MEM_SIZE	INSUFFICIENT_PRIVILEGES
select t2.fld3,companynr from t2 where companynr = 57+1 order by fld3	{
  "steps": [
    {
      "join_preparation": {
        "select#": 1,
        "steps": [
          {
            "expanded_query": "/* select#1 */ select `t2`.`fld3` AS `fld3`,`t2`.`companynr` AS `companynr` from `t2` where (`t2`.`companynr` = (57 + 1)) order by `t2`.`fld3`"
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
              "original_condition": "(`t2`.`companynr` = (57 + 1))",
              "steps": [
                {
                  "transformation": "equality_propagation",
                  "resulting_condition": "multiple equal((57 + 1), `t2`.`companynr`)"
                },
                {
                  "transformation": "constant_propagation",
                  "resulting_condition": "multiple equal((57 + 1), `t2`.`companynr`)"
                },
                {
                  "transformation": "trivial_condition_removal",
                  "resulting_condition": "multiple equal((57 + 1), `t2`.`companynr`)"
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
                "table=***
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
                "table=***
                "table_scan": {
                  "rows": 9,
                  "cost=***
                }
              }
            ]
          },
          {
            "considered_execution_plans": [
              {
                "plan_prefix": [
                ],
                "table=***
                "best_access_path": {
                  "considered_access_paths": [
                    {
                      "rows_to_scan": 9,
                      "access_type": "scan",
                      "resulting_rows": 9,
                      "cost=***
                      "chosen": true,
                      "use_tmp_table=***
                    }
                  ]
                },
                "condition_filtering_pct": 100,
                "rows_for_plan": 9,
                "cost=***
                "sort_cost=***
                "new_cost=***
                "chosen": true
              }
            ]
          },
          {
            "attaching_conditions_to_tables": {
              "original_condition": "(`t2`.`companynr` = (57 + 1))",
              "attached_conditions_computation": [
              ],
              "attached_conditions_summary": [
                {
                  "table=***
                  "attached": "(`t2`.`companynr` = (57 + 1))"
                }
              ]
            }
          },
          {
            "optimizing_distinct_group_by_order_by": {
              "simplifying_order_by": {
                "original_clause": "`t2`.`fld3`",
                "items": [
                  {
                    "item": "`t2`.`fld3`"
                  }
                ],
                "resulting_clause_is_simple": true,
                "resulting_clause": "`t2`.`fld3`"
              }
            }
          },
          {
            "reconsidering_access_paths_for_index_ordering": {
              "clause": "ORDER BY",
              "steps": [
              ],
              "index_order_summary": {
                "table=***
                "index_provides_order": false,
                "order_direction": "undefined",
                "index": "unknown",
                "plan_changed": false
              }
            }
          },
          {
            "finalizing_table_conditions": [
              {
                "table=***
                "original_table_condition": "(`t2`.`companynr` = (57 + 1))",
                "final_table_condition   ": "(`t2`.`companynr` = <cache>((57 + 1)))"
              }
            ]
          },
          {
            "refine_plan": [
              {
                "table=***
              }
            ]
          },
          {
            "considering_tmp_tables": [
              {
                "adding_sort_to_table=***
              }
            ]
          }
        ]
      }
    },
    {
      "optimizer_statistics": [
        {
          "db": "test",
          "table=***
          "table_statistics": {
            "data_file_length": 16384,
            "delete_length": 0,
            "records": 9,
            "deleted": 0,
            "table_in_mem_estimate": 1
          },
          "index_statistics": [
            {
              "name": "PRIMARY",
              "in_memory_estimate": 1,
              "records_per_key": [
                1
              ],
              "records_in_range": [
              ]
            },
            {
              "name": "fld1",
              "in_memory_estimate": 1,
              "records_per_key": [
                1
              ],
              "records_in_range": [
              ]
            },
            {
              "name": "fld3",
              "in_memory_estimate": 1,
              "records_per_key": [
                1,
                1
              ],
              "records_in_range": [
              ]
            }
          ]
        }
      ]
    },
    {
      "join_execution": {
        "steps": [
        ]
      }
    }
  ]
}	0	0
select name,count(*) from t3 where name >= "extramarital" and price <= 39654943 group by name;
name	count(*)
Iranizes	4181
extramarital	4181
gazer	4181
gems	4181
spates	4181
tucked	4181
violinist	4181
SELECT * FROM information_schema.optimizer_trace;
QUERY	TRACE	MISSING_BYTES_BEYOND_MAX_MEM_SIZE	INSUFFICIENT_PRIVILEGES
select name,count(*) from t3 where name >= "extramarital" and price <= 39654943 group by name	{
  "steps": [
    {
      "join_preparation": {
        "select#": 1,
        "steps": [
          {
            "expanded_query": "/* select#1 */ select `t3`.`name` AS `name`,count(0) AS `count(*)` from `t3` where ((`t3`.`name` >= 'extramarital') and (`t3`.`price` <= 39654943)) group by `t3`.`name`"
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
              "original_condition": "((`t3`.`name` >= 'extramarital') and (`t3`.`price` <= 39654943))",
              "steps": [
                {
                  "transformation": "equality_propagation",
                  "resulting_condition": "((`t3`.`name` >= 'extramarital') and (`t3`.`price` <= 39654943))"
                },
                {
                  "transformation": "constant_propagation",
                  "resulting_condition": "((`t3`.`name` >= 'extramarital') and (`t3`.`price` <= 39654943))"
                },
                {
                  "transformation": "trivial_condition_removal",
                  "resulting_condition": "((`t3`.`name` >= 'extramarital') and (`t3`.`price` <= 39654943))"
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
                "table=***
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
                "table=***
                "range_analysis": {
                  "table_scan": {
                    "rows": 41736,
                    "cost=***
                  },
                  "potential_range_indexes": [
                    {
                      "index": "PRIMARY",
                      "usable": false,
                      "cause": "not_applicable"
                    },
                    {
                      "index": "period",
                      "usable": false,
                      "cause": "not_applicable"
                    },
                    {
                      "index": "name",
                      "usable": true,
                      "key_parts": [
                        "name",
                        "t2nr"
                      ]
                    }
                  ],
                  "setup_range_conditions": [
                  ],
                  "group_index_range": {
                    "chosen": false,
                    "cause": "not_applicable_aggregate_function"
                  },
                  "skip_scan_range": {
                    "chosen": false,
                    "cause": "has_group_by"
                  },
                  "analyzing_range_alternatives": {
                    "range_scan_alternatives": [
                      {
                        "index": "name",
                        "ranges": [
                          "'extramarital' <= name"
                        ],
                        "index_dives_for_eq_ranges": true,
                        "rowid_ordered": false,
                        "using_mrr": false,
                        "index_only": false,
                        "in_memory": 0.00909091,
                        "rows": 20868,
                        "cost=***
                        "chosen": false,
                        "cause": "cost"
                      }
                    ],
                    "analyzing_roworder_intersect": {
                      "usable": false,
                      "cause": "too_few_roworder_scans"
                    }
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
                "table=***
                "best_access_path": {
                  "considered_access_paths": [
                    {
                      "rows_to_scan": 41736,
                      "access_type": "scan",
                      "resulting_rows": 41736,
                      "cost=***
                      "chosen": true,
                      "use_tmp_table=***
                    }
                  ]
                },
                "condition_filtering_pct": 100,
                "rows_for_plan": 41736,
                "cost=***
                "sort_cost=***
                "new_cost=***
                "chosen": true
              }
            ]
          },
          {
            "attaching_conditions_to_tables": {
              "original_condition": "((`t3`.`name` >= 'extramarital') and (`t3`.`price` <= 39654943))",
              "attached_conditions_computation": [
              ],
              "attached_conditions_summary": [
                {
                  "table=***
                  "attached": "((`t3`.`name` >= 'extramarital') and (`t3`.`price` <= 39654943))"
                }
              ]
            }
          },
          {
            "optimizing_distinct_group_by_order_by": {
              "simplifying_group_by": {
                "original_clause": "`t3`.`name`",
                "items": [
                  {
                    "item": "`t3`.`name`"
                  }
                ],
                "resulting_clause_is_simple": true,
                "resulting_clause": "`t3`.`name`"
              }
            }
          },
          {
            "reconsidering_access_paths_for_index_ordering": {
              "clause": "GROUP BY",
              "steps": [
                {
                  "rows_estimation": {
                    "table=***
                    "index": "name",
                    "range_analysis": {
                      "table_scan": {
                        "rows": 41736,
                        "cost=***
                      },
                      "potential_range_indexes": [
                        {
                          "index": "PRIMARY",
                          "usable": false,
                          "cause": "not_applicable"
                        },
                        {
                          "index": "period",
                          "usable": false,
                          "cause": "not_applicable"
                        },
                        {
                          "index": "name",
                          "usable": true,
                          "key_parts": [
                            "name",
                            "t2nr"
                          ]
                        }
                      ],
                      "setup_range_conditions": [
                      ],
                      "group_index_range": {
                        "chosen": false,
                        "cause": "not_applicable_aggregate_function"
                      },
                      "skip_scan_range": {
                        "chosen": false,
                        "cause": "has_group_by"
                      },
                      "analyzing_range_alternatives": {
                        "range_scan_alternatives": [
                          {
                            "index": "name",
                            "ranges": [
                              "'extramarital' <= name"
                            ],
                            "index_dives_for_eq_ranges": true,
                            "rowid_ordered": false,
                            "using_mrr": false,
                            "index_only": false,
                            "in_memory": 0.00909091,
                            "rows": 20868,
                            "cost=***
                            "chosen": false,
                            "cause": "cost"
                          }
                        ],
                        "analyzing_roworder_intersect": {
                          "usable": false,
                          "cause": "too_few_roworder_scans"
                        }
                      }
                    }
                  }
                }
              ],
              "index_order_summary": {
                "table=***
                "index_provides_order": true,
                "order_direction": "asc",
                "index": "name",
                "plan_changed": true,
                "access_type": "index"
              }
            }
          },
          {
            "finalizing_table_conditions": [
              {
                "table=***
                "original_table_condition": "((`t3`.`name` >= 'extramarital') and (`t3`.`price` <= 39654943))",
                "final_table_condition   ": "((`t3`.`name` >= 'extramarital') and (`t3`.`price` <= 39654943))"
              }
            ]
          },
          {
            "refine_plan": [
              {
                "table=***
              }
            ]
          },
          {
            "considering_tmp_tables": [
            ]
          }
        ]
      }
    },
    {
      "optimizer_statistics": [
        {
          "db": "test",
          "table=***
          "table_statistics": {
            "data_file_length": 4734976,
            "delete_length": 0,
            "records": 41736,
            "deleted": 0,
            "table_in_mem_estimate": 0.0045045
          },
          "index_statistics": [
            {
              "name": "PRIMARY",
              "in_memory_estimate": 0.0045045,
              "records_per_key": [
                1
              ],
              "records_in_range": [
              ]
            },
            {
              "name": "name",
              "in_memory_estimate": 0.0045045,
              "records_per_key": [
                4637.33,
                1
              ],
              "records_in_range": [
              ]
            },
            {
              "name": "period",
              "in_memory_estimate": 0.0045045,
              "records_per_key": [
                4173.6,
                1
              ],
              "records_in_range": [
              ]
            }
          ]
        }
      ]
    },
    {
      "join_execution": {
        "steps": [
        ]
      }
    }
  ]
}	0	0
select t2.fld3,count(*) from t2,t3 where t2.fld1=158402 and t3.name=t2.fld3 group by t3.name;
fld3	count(*)
spates	4181
SET optimizer_trace="enabled=off";
drop table t2,t3;

# 3) test optimization context with DETERMINISTIC FUNCTION

CREATE FUNCTION crc(_text TEXT) RETURNS BIGINT(20) UNSIGNED DETERMINISTIC
RETURN CONV(LEFT(MD5(_text),16),16,10);
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
CREATE TABLE t121 (
id bigint(20) UNSIGNED NOT NULL
);
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
INSERT INTO t121 (id) VALUES (crc('photos'));
INSERT INTO t121 (id) VALUES (crc('photos1'));
CREATE TABLE `t8` (
`pseudo` varchar(35) character set latin1 NOT NULL default '',
`email` varchar(60) character set latin1 NOT NULL default '',
PRIMARY KEY  (`pseudo`),
UNIQUE KEY `email` (`email`)
) ENGINE=MyISAM CHARSET=latin1 ROW_FORMAT=DYNAMIC;
INSERT INTO t8 (pseudo,email) VALUES ('joce','test');
INSERT INTO t8 (pseudo,email) VALUES ('joce1','test1');
INSERT INTO t8 (pseudo,email) VALUES ('2joce1','2test1');
return before end_optimization_context
SELECT pseudo FROM t8 WHERE pseudo=(SELECT pseudo FROM t8 WHERE pseudo LIKE '%joce%');
ERROR 21000: Subquery returns more than 1 row
analyze table t121;
Table	Op	Msg_type	Msg_text
test.t121	analyze	status	OK
ALTER TABLE t121 ADD INDEX idx (id);

# 4) test optimization context with sub stmt

SELECT * FROM t121 FORCE INDEX (idx) WHERE (id = crc('photos')) AND TRUE;
id
15460588431078774149
DROP FUNCTION crc;
drop table t8,t121;

# 5) test copying resource of now()

CREATE TABLE t111(c1 DATE NOT NULL PRIMARY KEY, c2 DATE NULL, c3 INT,
INDEX idx2(c2)) ENGINE=InnoDB;
SET TIMESTAMP=1235553613;
INSERT INTO t111 VALUES(NOW(),NOW(),3),
(ADDTIME(NOW(),'1 01:01:01'),ADDTIME(NOW(),'1 01:01:01'),4),
(ADDTIME(NOW(),'2 01:01:01'),ADDTIME(NOW(),'2 01:01:01'),5),
(ADDTIME(NOW(),'3 01:01:01'),ADDTIME(NOW(),'3 01:01:01'),6);
Warnings:
Note	1292	Incorrect date value: '2009-02-25 12:20:13' for column 'c1' at row 1
Note	1292	Incorrect date value: '2009-02-25 12:20:13' for column 'c2' at row 1
Note	1292	Incorrect date value: '2009-02-26 13:21:14' for column 'c1' at row 2
Note	1292	Incorrect date value: '2009-02-26 13:21:14' for column 'c2' at row 2
Note	1292	Incorrect date value: '2009-02-27 13:21:14' for column 'c1' at row 3
Note	1292	Incorrect date value: '2009-02-27 13:21:14' for column 'c2' at row 3
Note	1292	Incorrect date value: '2009-02-28 13:21:14' for column 'c1' at row 4
Note	1292	Incorrect date value: '2009-02-28 13:21:14' for column 'c2' at row 4
SELECT * FROM t111 WHERE c1 > ADDTIME(NOW(),'1 01:01:01') ORDER BY c1 DESC;
c1	c2	c3
2009-02-28	2009-02-28	6
2009-02-27	2009-02-27	5
drop table t111;

# 6) test cache status counter

CREATE TABLE t131(wildstr VARCHAR(10), str VARCHAR(10), like_result INTEGER)
COLLATE utf8mb4_0900_ai_ci;
INSERT INTO t131 VALUES('abc', 'abc', 1), ('AbC', 'aBc', 1), ('_bc', 'abc', 1),
('a_c', 'abc', 1), ('ab_', 'abc', 1), ('%c', 'abc', 1), ('a%c', 'abc', 1),
('a%', 'abc', 1), ('a%d_f', 'abcdef', 1), ('a%d%g', 'abcdefg', 1),
('a_c_e', 'abcde', 1), ('a%%de', 'abcde', 1), ('a__de', 'abcde', 1),
(_utf16 0x65700025636E005F5E93, _utf16 0x65706C49636E5B575E93, 1),
('a\\', 'a\\', 1);
SELECT wildstr, str, like_result FROM t131 WHERE (str LIKE wildstr) <>
like_result;
wildstr	str	like_result
drop table t131;

# 7) test copying resource of user()

create table t101 (a char(10)) character set latin1;
select * from t101 where a=version();
a
select * from t101 where a=database();
a
select * from t101 where a=user();
a
drop table t101;

# 8) test copying resource of last insert id

set @@session.sql_auto_is_null=1;
create table t91 (a int not null auto_increment,b int not null,primary key (a,b));
insert into t91 SET A=NULL,B=1;
insert into t91 SET a=null,b=2;
select * from t91 where a is null;
a	b
2	2
drop table t91;

# 9) test ror scan of index dive cache

CREATE TABLE t1
(
/* Field names reflect value(rowid) distribution, st=STairs, swt= SaWTooth */
st_a int,
swt1a int,
swt2a int,
st_b int,
swt1b int,
swt2b int,
key sta_swt12a(st_a,swt1a,swt2a),
key sta_swt1a(st_a,swt1a),
key sta_swt2a(st_a,swt2a),
key sta_swt21a(st_a,swt2a,swt1a),
key st_a(st_a),
key stb_swt1a_2b(st_b,swt1b,swt2a),
key stb_swt1b(st_b,swt1b),
key st_b(st_b)
) ;
ALTER TABLE t1 DISABLE KEYS;
Warnings:
Note	1031	Table storage engine for 't1' doesn't have this option
#
# Printing of many insert into t1 disabled.
#
ALTER TABLE t1 ENABLE KEYS;
Warnings:
Note	1031	Table storage engine for 't1' doesn't have this option
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
explain
select * from t1
where st_a=1 and swt1a=1 and swt2a=1 and st_b=1 and swt1b=1 and swt2b=1 limit 5;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	index_merge	sta_swt12a,sta_swt1a,sta_swt2a,sta_swt21a,st_a,stb_swt1a_2b,stb_swt1b,st_b	sta_swt12a,stb_swt1a_2b	15,15	NULL	3	9.00	Using intersect(sta_swt12a,stb_swt1a_2b); Using where
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`st_a` AS `st_a`,`test`.`t1`.`swt1a` AS `swt1a`,`test`.`t1`.`swt2a` AS `swt2a`,`test`.`t1`.`st_b` AS `st_b`,`test`.`t1`.`swt1b` AS `swt1b`,`test`.`t1`.`swt2b` AS `swt2b` from `test`.`t1` where ((`test`.`t1`.`swt2b` = 1) and (`test`.`t1`.`swt1b` = 1) and (`test`.`t1`.`st_b` = 1) and (`test`.`t1`.`swt2a` = 1) and (`test`.`t1`.`swt1a` = 1) and (`test`.`t1`.`st_a` = 1)) limit 5
select * from t1
where st_a=1 and swt1a=1 and swt2a=1 and st_b=1 and swt1b=1 and swt2b=1 limit 5;
st_a	swt1a	swt2a	st_b	swt1b	swt2b
1	1	1	1	1	1
1	1	1	1	1	1
1	1	1	1	1	1
1	1	1	1	1	1
1	1	1	1	1	1
drop table t1;

# 10) test copying user vars (note the case is blocked by check_px_execution now)

create table t1 (c1 int ,c2 int);
set @arg01= 8;
set @arg02= 8.0;
set @arg03= 80.00000000000e-1;
set @arg04= 'abc' ;
set @arg05= CAST('abc' as binary) ;
set @arg06= '1991-08-05' ;
set @arg07= CAST('1991-08-05' as date);
set @arg08= '1991-08-05 01:01:01' ;
set @arg09= CAST('1991-08-05 01:01:01' as datetime) ;
set @arg10= unix_timestamp('1991-01-01 01:01:01');
set @arg11= YEAR('1991-01-01 01:01:01');
set @arg12= 8 ;
set @arg12= NULL ;
set @arg13= 8.0 ;
set @arg13= NULL ;
set @arg14= 'abc';
set @arg14= NULL ;
set @arg15= CAST('abc' as binary) ;
set @arg15= NULL ;
explain select
8                           as const01, @arg01 as param01,
8.0                         as const02, @arg02 as param02,
80.00000000000e-1           as const03, @arg03 as param03,
'abc'                       as const04, @arg04 as param04,
CAST('abc' as binary)       as const05, @arg05 as param05,
'1991-08-05'                as const06, @arg06 as param06,
CAST('1991-08-05' as date)  as const07, @arg07 as param07,
'1991-08-05 01:01:01'       as const08, @arg08 as param08,
CAST('1991-08-05 01:01:01'  as datetime) as const09, @arg09 as param09,
unix_timestamp('1991-01-01 01:01:01')    as const10, @arg10 as param10,
YEAR('1991-01-01 01:01:01') as const11, @arg11 as param11,
NULL                        as const12, @arg12 as param12,
@arg13 as param13,
@arg14 as param14,
@arg15 as param15,
sum(c1) from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Parallel scan (1 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select 8 AS `const01`,(@`arg01`) AS `param01`,8.0 AS `const02`,(@`arg02`) AS `param02`,80.00000000000e-1 AS `const03`,(@`arg03`) AS `param03`,'abc' AS `const04`,(@`arg04`) AS `param04`,cast('abc' as char charset binary) AS `const05`,(@`arg05`) AS `param05`,'1991-08-05' AS `const06`,(@`arg06`) AS `param06`,cast('1991-08-05' as date) AS `const07`,(@`arg07`) AS `param07`,'1991-08-05 01:01:01' AS `const08`,(@`arg08`) AS `param08`,cast('1991-08-05 01:01:01' as datetime) AS `const09`,(@`arg09`) AS `param09`,unix_timestamp('1991-01-01 01:01:01') AS `const10`,(@`arg10`) AS `param10`,year('1991-01-01 01:01:01') AS `const11`,(@`arg11`) AS `param11`,NULL AS `const12`,(@`arg12`) AS `param12`,(@`arg13`) AS `param13`,(@`arg14`) AS `param14`,(@`arg15`) AS `param15`,sum(`test`.`t1`.`c1`) AS `sum(c1)` from `test`.`t1`
select
8                           as const01, @arg01 as param01,
8.0                         as const02, @arg02 as param02,
80.00000000000e-1           as const03, @arg03 as param03,
'abc'                       as const04, @arg04 as param04,
CAST('abc' as binary)       as const05, @arg05 as param05,
'1991-08-05'                as const06, @arg06 as param06,
CAST('1991-08-05' as date)  as const07, @arg07 as param07,
'1991-08-05 01:01:01'       as const08, @arg08 as param08,
CAST('1991-08-05 01:01:01'  as datetime) as const09, @arg09 as param09,
unix_timestamp('1991-01-01 01:01:01')    as const10, @arg10 as param10,
YEAR('1991-01-01 01:01:01') as const11, @arg11 as param11,
NULL                        as const12, @arg12 as param12,
@arg13 as param13,
@arg14 as param14,
@arg15 as param15,
sum(c1) from t1;
const01	param01	const02	param02	const03	param03	const04	param04	const05	param05	const06	param06	const07	param07	const08	param08	const09	param09	const10	param10	const11	param11	const12	param12	param13	param14	param15	sum(c1)
8	8	8.0	8.0	8	8	abc	abc	abc	abc	1991-08-05	1991-08-05	1991-08-05	1991-08-05	1991-08-05 01:01:01	1991-08-05 01:01:01	1991-08-05 01:01:01	1991-08-05 01:01:01	662680861	662680861	1991	1991	NULL	NULL	NULL	NULL	NULL	NULL
explain select
sum(c1) from t1
where 8  = @arg01
and 8.0 = @arg02
and 80.00000000000e-1 = @arg03
and 'abc' = @arg04
and CAST('abc' as binary) = @arg05
and '1991-08-05' = @arg06
and CAST('1991-08-05' as date)  =  @arg07
and '1991-08-05 01:01:01' = @arg08
and CAST('1991-08-05 01:01:01'  as datetime) = @arg09
and unix_timestamp('1991-01-01 01:01:01') = @arg10
and YEAR('1991-01-01 01:01:01') = @arg11
and @arg12 is null
and @arg13 is null
and @arg14 is null
and @arg15 is null;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Parallel scan (1 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select sum(`test`.`t1`.`c1`) AS `sum(c1)` from `test`.`t1` where true
select
sum(c1) from t1
where 8  = @arg01
and 8.0 = @arg02
and 80.00000000000e-1 = @arg03
and 'abc' = @arg04
and CAST('abc' as binary) = @arg05
and '1991-08-05' = @arg06
and CAST('1991-08-05' as date)  =  @arg07
and '1991-08-05 01:01:01' = @arg08
and CAST('1991-08-05 01:01:01'  as datetime) = @arg09
and unix_timestamp('1991-01-01 01:01:01') = @arg10
and YEAR('1991-01-01 01:01:01') = @arg11
and @arg12 is null
and @arg13 is null
and @arg14 is null
and @arg15 is null;
sum(c1)
NULL
drop table t1;
CREATE TABLE t1 (a TIME(6));
INSERT INTO t1 VALUES ('11:22:33.123456');
SET @a=112233.123456;
flush status;
SELECT /*+parallel(2)*/ * FROM t1 WHERE a=@a;
a
11:22:33.123456
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	1
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	1
Txsql_parallel_stmt_thread_refused	0
drop table t1;
create table t1
(
a int, b varchar(30),
primary key(a)
);
insert into t1 values (1,'one');
insert into t1 values (2,'two');
insert into t1 values (3,'three');
insert into t1 values (4,'four');
set @arg00=2 ;
flush status;
select a, b FROM t1 outer_table where
a = (select a from t1 where a = @arg00 and b = outer_table.b) and b='two';
a	b
2	two
show status like '%txsql_parallel_stmt_%';
Variable_name	Value
Txsql_parallel_stmt_error	0
Txsql_parallel_stmt_executed	1
Txsql_parallel_stmt_fallback	0
Txsql_parallel_stmt_hint_executed	0
Txsql_parallel_stmt_thread_refused	0
drop table t1;

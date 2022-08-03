"WL6369 Explain for Connection"
QUERY: SELECT group_concat(c order by (select c from t2 where t2.a=t1.a limit 1)) as grp from t1 
STATUS:NOT_EQUAL
SYNC_POINT:before_reset_query_plan
Diff: EXPLAIN NORMAL vs OTHER
@@ -13,3 +13,2 @@
       "filtered": "100.00",
-      "parallel_scan": "4 workers",
       "cost_info": {
@@ -25,36 +24,2 @@
     },
-    "parallel_information": [
-      {
-        "exchange_table": {
-          "table_name": "<sender1>",
-          "access_type": "ALL",
-          "rows_examined_per_scan": 4,
-          "rows_produced_per_join": 0,
-          "filtered": "100.00",
-          "send_to": "<receiver1>",
-          "cost_info": {
-            "read_cost": "0.00",
-            "eval_cost": "0.00",
-            "prefix_cost": "0.00",
-            "data_read_per_join": "0"
-          }
-        }
-      },
-      {
-        "exchange_table": {
-          "table_name": "<receiver1>",
-          "access_type": "ALL",
-          "rows_examined_per_scan": 4,
-          "rows_produced_per_join": 0,
-          "filtered": "100.00",
-          "receive_from": "<sender1>",
-          "cost_info": {
-            "read_cost": "0.00",
-            "eval_cost": "0.00",
-            "prefix_cost": "0.00",
-            "data_read_per_join": "0"
-          }
-        }
-      }
-    ],
     "select_list_subqueries": [
@@ -74,2 +39,3 @@
             "filtered": "25.00",
+            "using_where": true,
             "cost_info": {
@@ -83,4 +49,3 @@
               "c"
-            ],
-            "attached_condition": "(`test`.`t2`.`a` = `test`.`t1`.`a`)"
+            ]
           }
QUERY: SELECT * FROM t1 UNION SELECT * FROM t1 ORDER BY (SELECT a FROM t2 WHERE b = 12) 
STATUS:NOT_EQUAL
SYNC_POINT:before_reset_query_plan
Diff: EXPLAIN NORMAL vs OTHER
@@ -24,3 +24,2 @@
                 "filtered": "100.00",
-                "parallel_scan": "2 workers",
                 "cost_info": {
@@ -34,37 +33,3 @@
                 ]
-              },
-              "parallel_information": [
-                {
-                  "exchange_table": {
-                    "table_name": "<sender2>",
-                    "access_type": "ALL",
-                    "rows_examined_per_scan": 2,
-                    "rows_produced_per_join": 0,
-                    "filtered": "100.00",
-                    "send_to": "<receiver2>",
-                    "cost_info": {
-                      "read_cost": "0.00",
-                      "eval_cost": "0.00",
-                      "prefix_cost": "0.00",
-                      "data_read_per_join": "0"
-                    }
-                  }
-                },
-                {
-                  "exchange_table": {
-                    "table_name": "<receiver2>",
-                    "access_type": "ALL",
-                    "rows_examined_per_scan": 2,
-                    "rows_produced_per_join": 0,
-                    "filtered": "100.00",
-                    "receive_from": "<sender2>",
-                    "cost_info": {
-                      "read_cost": "0.00",
-                      "eval_cost": "0.00",
-                      "prefix_cost": "0.00",
-                      "data_read_per_join": "0"
-                    }
-                  }
-                }
-              ]
+              }
             }
@@ -85,3 +50,2 @@
                 "filtered": "100.00",
-                "parallel_scan": "2 workers",
                 "cost_info": {
@@ -95,37 +59,3 @@
                 ]
-              },
-              "parallel_information": [
-                {
-                  "exchange_table": {
-                    "table_name": "<sender3>",
-                    "access_type": "ALL",
-                    "rows_examined_per_scan": 2,
-                    "rows_produced_per_join": 0,
-                    "filtered": "100.00",
-                    "send_to": "<receiver3>",
-                    "cost_info": {
-                      "read_cost": "0.00",
-                      "eval_cost": "0.00",
-                      "prefix_cost": "0.00",
-                      "data_read_per_join": "0"
-                    }
-                  }
-                },
-                {
-                  "exchange_table": {
-                    "table_name": "<receiver3>",
-                    "access_type": "ALL",
-                    "rows_examined_per_scan": 2,
-                    "rows_produced_per_join": 0,
-                    "filtered": "100.00",
-                    "receive_from": "<sender3>",
-                    "cost_info": {
-                      "read_cost": "0.00",
-                      "eval_cost": "0.00",
-                      "prefix_cost": "0.00",
-                      "data_read_per_join": "0"
-                    }
-                  }
-                }
-              ]
+              }
             }
@@ -149,2 +79,3 @@
               "filtered": "50.00",
+              "using_where": true,
               "cost_info": {
@@ -157,4 +88,3 @@
                 "b"
-              ],
-              "attached_condition": "(`test`.`t2`.`b` = 12)"
+              ]
             }
QUERY: SELECT SQL_CALC_FOUND_ROWS 'foo' UNION SELECT 'bar' LIMIT 0 
STATUS: EQUAL

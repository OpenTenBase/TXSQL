# include/check_secondary_index.inc
#
# SUMMARY
#  check if secondary index is valid.
#
# Usage
#  let $check_secondary_index__table_name=t1
#  let $check_secondary_index__pk_name=c1
#  let $check_secondary_index__idx_col_name=c2
#  let $check_secondary_index__idx_name=idx_c2
#  --source include/check_secondary_index.inc
#


# --echo check_secondary_index__table_name=$check_secondary_index__table_name;
# --echo check_secondary_index__pk_name=$check_secondary_index__pk_name;
# --echo check_secondary_index__idx_col_name=$check_secondary_index__idx_col_name;
# --echo check_secondary_index__idx_name=$check_secondary_index__idx_name;

eval CHECK TABLE $check_secondary_index__table_name;

eval CREATE TEMPORARY TABLE pri_res SELECT $check_secondary_index__pk_name, $check_secondary_index__idx_col_name FROM $check_secondary_index__table_name FORCE INDEX (PRI);
eval CREATE TEMPORARY TABLE idx_res SELECT $check_secondary_index__pk_name, $check_secondary_index__idx_col_name FROM $check_secondary_index__table_name FORCE INDEX ($check_secondary_index__idx_name);
 
eval SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.$check_secondary_index__pk_name = idx_res.$check_secondary_index__pk_name WHERE idx_res.$check_secondary_index__pk_name IS NULL OR pri_res.$check_secondary_index__idx_col_name != idx_res.$check_secondary_index__idx_col_name;
eval SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.$check_secondary_index__pk_name = idx_res.$check_secondary_index__pk_name WHERE pri_res.$check_secondary_index__pk_name IS NULL OR pri_res.$check_secondary_index__idx_col_name != idx_res.$check_secondary_index__idx_col_name;

DROP TABLE pri_res;
DROP TABLE idx_res;

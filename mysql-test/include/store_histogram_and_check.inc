eval ANALYZE TABLE $tbl_name UPDATE HISTOGRAM ON $col_name WITH $buckets BUCKETS;
let $result1= `SELECT JSON_REMOVE(HISTOGRAM, '$."last-updated"') AS HISTOGRAM FROM information_schema.COLUMN_STATISTICS WHERE TABLE_NAME="$tbl_name" AND COLUMN_NAME="$col_name";`;
eval ANALYZE TABLE $tbl_name UPDATE HISTOGRAM ON $col_name USING DATA '$json_data';
let $result2= `SELECT JSON_REMOVE(HISTOGRAM, '$."last-updated"') AS HISTOGRAM FROM information_schema.COLUMN_STATISTICS WHERE TABLE_NAME="$tbl_name" AND COLUMN_NAME="$col_name";`;
#--echo $json_data
#--echo $result1
#--echo $result2
assert($json_data == $result1);
assert($json_data == $result2);

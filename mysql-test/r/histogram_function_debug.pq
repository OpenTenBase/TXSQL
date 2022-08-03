##################
# HISTOGRAM FUNCTION with debug
##################
#
# Cover the test about transformation between value_map and value_vector.
#
set debug="+d, histogram_low_memory_limit";
set debug="+d, set_m_rows_to_skip_1";
CREATE TABLE tbl (col_varchar VARCHAR(255));
INSERT INTO tbl VALUES ('aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa');
INSERT INTO tbl VALUES ('baaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa');
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_varchar, 100) FROM tbl), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_varchar, 100) FROM tbl), '$."last-updated"')
{"buckets": [["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 1.0]], "data-type": "string", "null-values": 0.0, "collation-id": 255, "sampling-rate": 0.5, "histogram-type": "singleton", "number-of-buckets-specified": 100}
set debug="+d, set_m_rows_to_skip_0";
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_varchar, 100) FROM tbl), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_varchar, 100) FROM tbl), '$."last-updated"')
{"buckets": [["base64:type254:YmFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 1.0]], "data-type": "string", "null-values": 0.0, "collation-id": 255, "sampling-rate": 0.5, "histogram-type": "singleton", "number-of-buckets-specified": 100}
set debug="-d, set_m_rows_to_skip_1";
set debug="-d, set_m_rows_to_skip_0";
set debug="-d, histogram_low_memory_limit";
DROP TABLE tbl;
#
# Test case to verify if value add statement crash safe.
#
CREATE TABLE tbl2 (col_integer INTEGER);
INSERT INTO tbl2 VALUES (-2147483648);
INSERT INTO tbl2 VALUES (2147483647);
set debug="+d, set_default_map_use";
set debug="+d, histogram_low_memory_limit";
set debug="+d, set_m_rows_to_skip_0";
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_integer, 100) FROM tbl2), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_integer, 100) FROM tbl2), '$."last-updated"')
{"buckets": [[2147483647, 1.0]], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 0.5, "histogram-type": "singleton", "number-of-buckets-specified": 100}
set session debug="+d, map_add_value_error";
SELECT HISTOGRAM(col_integer, 100) FROM tbl2;
ERROR HY000: Runtime error: Out of memory in function histogram.
set session debug="-d, map_add_value_error";
set session debug="+d, vector_add_value_error";
set session debug="+d, vector_add_values_error";
SELECT HISTOGRAM(col_integer, 100) FROM tbl2;
ERROR HY000: Runtime error: Out of memory in function histogram.
set session debug="-d, vector_add_value_error";
set session debug="-d, vector_add_values_error";
set session debug="+d, histogram_use_map";
set session debug="+d, vector_replace_value_error";
set session debug="+d, vector_add_values_error";
SELECT HISTOGRAM(col_integer, 100) FROM tbl2;
ERROR HY000: Runtime error: Out of memory in function histogram.
set session debug="-d, vector_add_values_error";
set session debug="-d, vector_replace_value_error";
set session debug="-d, histogram_use_map";
set session debug="+d, fill_into_value_map_error";
SELECT HISTOGRAM(col_integer, 100) FROM tbl2;
ERROR HY000: Runtime error: Out of memory in function histogram.
set session debug="-d, fill_into_value_map_error";
set debug="-d, set_default_map_use";
DROP TABLE tbl2;
#
# Test case to verify if algorithm z can be used correctly.
#
CREATE TABLE tbl (col_varchar VARCHAR(255));
INSERT INTO tbl VALUES (NULL);
INSERT INTO tbl VALUES ('aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa');
INSERT INTO tbl VALUES ('baaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa');
INSERT INTO tbl VALUES (NULL);
set debug="+d, use_algorithm_z";
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_varchar, 100) FROM tbl), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_varchar, 100) FROM tbl), '$."last-updated"')
{"buckets": [], "data-type": "string", "null-values": 1.0, "collation-id": 255, "sampling-rate": 0.25, "histogram-type": "singleton", "number-of-buckets-specified": 100}
set debug="-d, use_algorithm_z";
set debug="-d, set_m_rows_to_skip_0";
set debug="-d, histogram_low_memory_limit";
DROP TABLE tbl;

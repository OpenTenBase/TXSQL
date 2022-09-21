##################
# HISTOGRAM FUNCTION
##################
#
# Verify that we can build a histogram for all valid data types.
#
CREATE TABLE all_types (
col_bool BOOLEAN,
col_bit BIT(64),
col_tinyint TINYINT,
col_smallint SMALLINT,
col_mediumint MEDIUMINT,
col_integer INTEGER,
col_bigint BIGINT,
col_tinyint_unsigned TINYINT UNSIGNED,
col_smallint_unsigned SMALLINT UNSIGNED,
col_mediumint_unsigned MEDIUMINT UNSIGNED,
col_integer_unsigned INTEGER UNSIGNED,
col_bigint_unsigned BIGINT UNSIGNED,
col_float FLOAT,
col_double DOUBLE,
col_decimal DECIMAL(65, 2),
col_date DATE,
col_time TIME,
col_year YEAR,
col_datetime DATETIME,
col_timestamp TIMESTAMP NULL,
col_char CHAR(255),
col_varchar VARCHAR(255),
col_tinytext TINYTEXT,
col_text TEXT,
col_mediumtext MEDIUMTEXT,
col_longtext LONGTEXT,
col_binary BINARY(255),
col_varbinary VARBINARY(255),
col_tinyblob TINYBLOB,
col_blob BLOB,
col_mediumblob MEDIUMBLOB,
col_longblob LONGBLOB,
col_enum ENUM('red', 'black', 'pink', 'white', 'purple'),
col_set SET('one', 'two', 'three'));
INSERT INTO all_types VALUES (
NULL,     # BOOLEAN
NULL,     # BIT
NULL,     # TINYINT
NULL,     # SMALLINT
NULL,     # MEDIUMINT
NULL,     # INTEGER
NULL,     # BIGINT
NULL,     # TINYINT_UNSIGNED
NULL,     # SMALLINT_UNSIGNED
NULL,     # MEDIUMINT_UNSIGNED
NULL,     # INTEGER_UNSIGNED
NULL,     # BIGINT_UNSIGNED
NULL,     # FLOAT
NULL,     # DOUBLE
NULL,     # DECIMAL(65, 2)
NULL,     # DATE
NULL,     # TIME
NULL,     # YEAR
NULL,     # DATETIME
NULL,     # TIMESTAMP
NULL,     # CHAR
NULL,     # VARCHAR
NULL,     # TINYTEXT
NULL,     # TEXT
NULL,     # MEDIUMTEXT
NULL,     # LONGTEXT
NULL,     # BINARY
NULL,     # VARBINARY
NULL,     # TINYBLOB
NULL,     # BLOB
NULL,     # MEDIUMBLOB
NULL,     # LONGBLOB
NULL,     # ENUM
NULL);
INSERT INTO all_types VALUES (
FALSE,                                                               # BOOLEAN
b'0000000000000000000000000000000000000000000000000000000000000000', # BIT
-128,                                                                # TINYINT
-32768,                                                              # SMALLINT
-8388608,                                                            # MEDIUMINT
-2147483648,                                                         # INTEGER
-9223372036854775808,                                                # BIGINT
0,                                                                   # TINYINT_UNSIGNED
0,                                                                   # SMALLINT_UNSIGNED
0,                                                                   # MEDIUMINT_UNSIGNED
0,                                                                   # INTEGER_UNSIGNED
0,                                                                   # BIGINT_UNSIGNED
-3.402823466E+38,                                                    # FLOAT
-1.7976931348623157E+308,                                            # DOUBLE
-999999999999999999999999999999999999999999999999999999999999999.99, # DECIMAL(65, 2)
'1000-01-01',                                                        # DATE
'-838:59:59.000000',                                                 # TIME
1901,                                                                # YEAR
'1000-01-01 00:00:00',                                               # DATETIME
'1970-01-02 00:00:01',                                               # TIMESTAMP
'',                                                                  # CHAR
'',                                                                  # VARCHAR
'',                                                                  # TINYTEXT
'',                                                                  # TEXT
'',                                                                  # MEDIUMTEXT
'',                                                                  # LONGTEXT
'',                                                                  # BINARY
'',                                                                  # VARBINARY
'',                                                                  # TINYBLOB
'',                                                                  # BLOB
'',                                                                  # MEDIUMBLOB
'',                                                                  # LONGBLOB
'red',                                                               # ENUM
'');
INSERT INTO all_types VALUES (
TRUE,                                                                # BOOLEAN
b'1111111111111111111111111111111111111111111111111111111111111111', # BIT
127,                                                                 # TINYINT
32767,                                                               # SMALLINT
8388607,                                                             # MEDIUMINT
2147483647,                                                          # INTEGER
9223372036854775807,                                                 # BIGINT
255,                                                                 # TINYINT_UNSIGNED
65535,                                                               # SMALLINT_UNSIGNED
16777215,                                                            # MEDIUMINT_UNSIGNED
4294967295,                                                          # INTEGER_UNSIGNED
18446744073709551615,                                                # BIGINT_UNSIGNED
3.402823466E+38,                                                     # FLOAT
1.7976931348623157E+308,                                             # DOUBLE
999999999999999999999999999999999999999999999999999999999999999.99,  # DECIMAL(65, 2)
'9999-12-31',                                                        # DATE
'838:59:59.000000',                                                  # TIME
2155,                                                                # YEAR
'9999-12-31 23:59:59',                                               # DATETIME
'2038-01-19 03:14:07',                                               # TIMESTAMP
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # CHAR
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # VARCHAR
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # TINYTEXT
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # TEXT
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # MEDIUMTEXT
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # LONGTEXT
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # BINARY
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # VARBINARY
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # TINYBLOB
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # BLOB
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # MEDIUMBLOB
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',                     # LONGBLOB
'purple',                                                            # ENUM
'three');
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_bool, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_bool, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[0, 0.3333333333333333], [1, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_bit, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_bit, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[-1, 0.3333333333333333], [0, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 63, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_tinyint, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_tinyint, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[-128, 0.3333333333333333], [127, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_smallint, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_smallint, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[-32768, 0.3333333333333333], [32767, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_mediumint, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_mediumint, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[-8388608, 0.3333333333333333], [8388607, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_integer, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_integer, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[-2147483648, 0.3333333333333333], [2147483647, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_bigint, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_bigint, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[-9223372036854775808, 0.3333333333333333], [9223372036854775807, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_tinyint_unsigned, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_tinyint_unsigned, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[0, 0.3333333333333333], [255, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_smallint_unsigned, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_smallint_unsigned, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[0, 0.3333333333333333], [65535, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_mediumint_unsigned, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_mediumint_unsigned, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[0, 0.3333333333333333], [16777215, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_integer_unsigned, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_integer_unsigned, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[0, 0.3333333333333333], [4294967295, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_bigint_unsigned, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_bigint_unsigned, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[0, 0.3333333333333333], [18446744073709551615, 0.6666666666666666]], "data-type": "uint", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_float, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_float, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[-3.4028234663852886e38, 0.3333333333333333], [3.4028234663852886e38, 0.6666666666666666]], "data-type": "double", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_double, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_double, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[-1.7976931348623157e308, 0.3333333333333333], [1.7976931348623157e308, 0.6666666666666666]], "data-type": "double", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_decimal, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_decimal, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[-999999999999999999999999999999999999999999999999999999999999999.99, 0.3333333333333333], [999999999999999999999999999999999999999999999999999999999999999.99, 0.6666666666666666]], "data-type": "decimal", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_date, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_date, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["1000-01-01", 0.3333333333333333], ["9999-12-31", 0.6666666666666666]], "data-type": "date", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_time, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_time, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["-838:59:59.000000", 0.3333333333333333], ["838:59:59.000000", 0.6666666666666666]], "data-type": "time", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_year, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_year, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[1901, 0.3333333333333333], [2155, 0.6666666666666666]], "data-type": "int", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_datetime, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_datetime, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["1000-01-01 00:00:00.000000", 0.3333333333333333], ["9999-12-31 23:59:59.000000", 0.6666666666666666]], "data-type": "datetime", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_timestamp, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_timestamp, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["1970-01-02 00:00:01.000000", 0.3333333333333333], ["2038-01-19 03:14:07.000000", 0.6666666666666666]], "data-type": "datetime", "null-values": 0.3333333333333333, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_char, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_char, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 255, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_varchar, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_varchar, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 255, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_tinytext, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_tinytext, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 255, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_text, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_text, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 255, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_mediumtext, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_mediumtext, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 255, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_longtext, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_longtext, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 255, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_binary, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_binary, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 63, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_varbinary, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_varbinary, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 63, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_tinyblob, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_tinyblob, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 63, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_blob, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_blob, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 63, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_mediumblob, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_mediumblob, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 63, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_longblob, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_longblob, 1024) FROM all_types), '$."last-updated"')
{"buckets": [["base64:type254:", 0.3333333333333333], ["base64:type254:YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh", 0.6666666666666666]], "data-type": "string", "null-values": 0.3333333333333333, "collation-id": 63, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_enum, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_enum, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[1, 0.3333333333333333], [5, 0.6666666666666666]], "data-type": "enum", "null-values": 0.3333333333333333, "collation-id": 255, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_set, 1024) FROM all_types), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_set, 1024) FROM all_types), '$."last-updated"')
{"buckets": [[0, 0.3333333333333333], [4, 0.6666666666666666]], "data-type": "set", "null-values": 0.3333333333333333, "collation-id": 255, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 1024}
DROP TABLE all_types;
#
# Check that we don't try to build histogram statistics for
# unsupported data types.
#
CREATE TABLE unsupported_types (col_geometry GEOMETRY,
col_point POINT,
col_linestring LINESTRING,
col_polygon POLYGON,
col_multipoint MULTIPOINT,
col_multilinestring MULTILINESTRING,
col_multipolygon MULTIPOLYGON,
col_geometrycollection GEOMETRYCOLLECTION,
col_json JSON);
SELECT HISTOGRAM(col_geometry, 100) FROM unsupported_types;
ERROR HY000: Incorrect arguments to histogram
SELECT HISTOGRAM(col_point, 100) FROM unsupported_types;
ERROR HY000: Incorrect arguments to histogram
SELECT HISTOGRAM(col_linestring, 100) FROM unsupported_types;
ERROR HY000: Incorrect arguments to histogram
SELECT HISTOGRAM(col_polygon, 100) FROM unsupported_types;
ERROR HY000: Incorrect arguments to histogram
SELECT HISTOGRAM(col_multipoint, 100) FROM unsupported_types;
ERROR HY000: Incorrect arguments to histogram
SELECT HISTOGRAM(col_multilinestring, 100) FROM unsupported_types;
ERROR HY000: Incorrect arguments to histogram
SELECT HISTOGRAM(col_multipolygon, 100) FROM unsupported_types;
ERROR HY000: Incorrect arguments to histogram
SELECT HISTOGRAM(col_geometrycollection, 100) FROM unsupported_types;
ERROR HY000: Incorrect arguments to histogram
SELECT HISTOGRAM(col_json, 100) FROM unsupported_types;
ERROR HY000: Incorrect arguments to histogram
DROP TABLE unsupported_types;
CREATE TABLE t1 (col_integer INT);
#
# Non-existing column
#
SELECT HISTOGRAM(foobar, 100) FROM  t1;
ERROR 42S22: Unknown column 'foobar' in 'field list'
#
# Non-existing table
#
SELECT HISTOGRAM(foobar, 100) FROM foobar;
ERROR 42S02: Table 'test.foobar' doesn't exist
#
# Temporary table
#
CREATE TEMPORARY TABLE temp_table (col1 INT);
SELECT JSON_REMOVE((SELECT HISTOGRAM(col1, 100) FROM temp_table), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col1, 100) FROM temp_table), '$."last-updated"')
{"buckets": [], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
DROP TABLE temp_table;
#
# View
#
CREATE VIEW my_view AS SELECT * FROM t1;
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_integer, 100) FROM my_view), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_integer, 100) FROM my_view), '$."last-updated"')
{"buckets": [], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
DROP VIEW my_view;
#
# Generated column
#
ALTER TABLE t1 ADD COLUMN virtual_generated INT AS (col_integer + 10) VIRTUAL,
ADD COLUMN stored_generated INT AS (col_integer + 20) STORED;
INSERT INTO t1 (col_integer) VALUES (10), (20), (30);
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_integer, 100) FROM t1), '$."last-updated"') AS histogram_col_integer;
histogram_col_integer
{"buckets": [[10, 0.3333333333333333], [20, 0.6666666666666666], [30, 1.0]], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
SELECT JSON_REMOVE((SELECT HISTOGRAM(virtual_generated, 100) FROM t1), '$."last-updated"') AS histogram_virtual_generated;
histogram_virtual_generated
{"buckets": [[20, 0.3333333333333333], [30, 0.6666666666666666], [40, 1.0]], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
SELECT JSON_REMOVE((SELECT HISTOGRAM(stored_generated, 100) FROM t1), '$."last-updated"') AS histogram_stored_generated;
histogram_stored_generated
{"buckets": [[30, 0.3333333333333333], [40, 0.6666666666666666], [50, 1.0]], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
ALTER TABLE t1 DROP COLUMN virtual_generated, DROP COLUMN stored_generated;
DELETE FROM t1;
#
# Check that the server switches between singleton and equi-height when
# it should.
#
INSERT INTO t1 VALUES (1), (2), (3), (4), (5), (6), (7), (8), (9), (10);
SELECT JSON_EXTRACT((SELECT HISTOGRAM(col_integer, 10) FROM t1), '$."histogram-type"') AS should_be_singleton;
should_be_singleton
"singleton"
SELECT JSON_EXTRACT((SELECT HISTOGRAM(col_integer, 9) FROM t1), '$."histogram-type"') AS should_be_equiheight;
should_be_equiheight
"equi-height"
DROP TABLE t1;
#
# Check that invalid range for the number of buckets is rejected
# (should be in the range [1, 1024]).
#
SELECT HISTOGRAM(foo, 0) FROM foo;
ERROR 22003: Number of buckets value is out of range in 'SELECT HISTOGRAM_FUNC()'
SELECT HISTOGRAM(foo, 1025) FROM foo;
ERROR 22003: Number of buckets value is out of range in 'SELECT HISTOGRAM_FUNC()'
SELECT HISTOGRAM(foo, -1) FROM foo;
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near '-1) FROM foo' at line 1
#
# Histogram statistics can be created for columns covered by a
# single-part unique index
#
CREATE TABLE t1 (col1 INT PRIMARY KEY,
col2 INT,
col3 INT,
UNIQUE INDEX index_1 (col2),
UNIQUE INDEX index_2 (col3, col2));
SELECT JSON_REMOVE((SELECT HISTOGRAM(col1, 100) FROM t1), '$."last-updated"') AS histogram_col1;
histogram_col1
{"buckets": [], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col2, 100) FROM t1), '$."last-updated"') AS histogram_col2;
histogram_col2
{"buckets": [], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
SELECT JSON_REMOVE((SELECT HISTOGRAM(col3, 100) FROM t1), '$."last-updated"') AS histogram_col3;
histogram_col3
{"buckets": [], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
DROP TABLE t1;
#
# The server should not create a histogram with more buckets than
# specified
#
CREATE TABLE t1 (col1 INT);
INSERT INTO t1 SELECT 1;
INSERT INTO t1 SELECT col1 + 1 FROM t1;
INSERT INTO t1 SELECT col1 + 2 FROM t1;
INSERT INTO t1 SELECT col1 + 4 FROM t1;
INSERT INTO t1 SELECT col1 + 8 FROM t1;
INSERT INTO t1 SELECT col1 + 16 FROM t1;
INSERT INTO t1 SELECT col1 + 32 FROM t1;
INSERT INTO t1 SELECT col1 + 64 FROM t1;
INSERT INTO t1 SELECT col1 + 128 FROM t1;
SELECT JSON_LENGTH(JSON_EXTRACT((SELECT HISTOGRAM(col1, 10) FROM t1), '$."buckets"')) <= 10;
JSON_LENGTH(JSON_EXTRACT((SELECT HISTOGRAM(col1, 10) FROM t1), '$."buckets"')) <= 10
1
SELECT JSON_LENGTH(JSON_EXTRACT((SELECT HISTOGRAM(col1, 57) FROM t1), '$."buckets"')) <= 57;
JSON_LENGTH(JSON_EXTRACT((SELECT HISTOGRAM(col1, 57) FROM t1), '$."buckets"')) <= 57
1
SELECT JSON_LENGTH(JSON_EXTRACT((SELECT HISTOGRAM(col1, 255) FROM t1), '$."buckets"')) <= 255;
JSON_LENGTH(JSON_EXTRACT((SELECT HISTOGRAM(col1, 255) FROM t1), '$."buckets"')) <= 255
1
DROP TABLE t1;
#
# Using select histogram in a prepared statement.
#
CREATE TABLE t(x INT);
PREPARE ps FROM 'SELECT JSON_REMOVE((SELECT HISTOGRAM(x, 10) FROM t), \'$."last-updated"\') AS histogram_x';
EXECUTE ps;
histogram_x
{"buckets": [], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 10}
EXECUTE ps;
histogram_x
{"buckets": [], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 10}
deallocate prepare ps;
DROP TABLE t;
#
# Test case to verify if expression can run correctly.
#
CREATE TABLE tbl (col_integer INTEGER);
INSERT INTO tbl VALUES (-2147483648);
INSERT INTO tbl VALUES (2147483646);
SELECT JSON_REMOVE((SELECT HISTOGRAM(col_integer + 1, 100) FROM tbl), '$."last-updated"');
JSON_REMOVE((SELECT HISTOGRAM(col_integer + 1, 100) FROM tbl), '$."last-updated"')
{"buckets": [[-2147483647, 0.5], [2147483647, 1.0]], "data-type": "int", "null-values": 0.0, "collation-id": 8, "sampling-rate": 1.0, "histogram-type": "singleton", "number-of-buckets-specified": 100}
DROP TABLE tbl;
# 
# Test case to verify histogram function can not be used as window function.
#
CREATE TABLE tbl (col1 INT, col2 INT);
INSERT INTO tbl VALUES (1, 2);
SELECT col1, col2, HISTOGRAM(col2, 100) OVER(PARTITION BY col1) as a FROM tbl;
ERROR 42000: This version of MySQL doesn't yet support 'histogram as window function'
DROP TABLE tbl;

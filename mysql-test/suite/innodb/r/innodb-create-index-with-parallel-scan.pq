CREATE TABLE t1 (c1 char(127) PRIMARY KEY, c2 char(127), c3 char(127)) ENGINE=INNODB;
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK


#TEST PARALLEL CASE
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
ALTER TABLE t1 ADD INDEX k_c2(c2), add unique index k_c3(c3);
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c2 FROM t1 FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c2 FROM t1 FORCE INDEX (k_c2);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
DROP TABLE pri_res;
DROP TABLE idx_res;
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c3 FROM t1 FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c3 FROM t1 FORCE INDEX (k_c3);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
DROP TABLE pri_res;
DROP TABLE idx_res;
ALTER TABLE t1 DROP INDEX k_c2, DROP INDEX k_c3;


#TEST NON-PARALLEL CASE
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=1;
ALTER TABLE t1 ADD INDEX k_c2(c2), add unique index k_c3(c3);
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c2 FROM t1 FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c2 FROM t1 FORCE INDEX (k_c2);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
DROP TABLE pri_res;
DROP TABLE idx_res;
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c3 FROM t1 FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c3 FROM t1 FORCE INDEX (k_c3);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
DROP TABLE pri_res;
DROP TABLE idx_res;
ALTER TABLE t1 DROP INDEX k_c2, DROP INDEX k_c3;


#TEST WRITE TMP FILE FAILURE
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
SET GLOBAL debug='+d,row_merge_write_failure';
ALTER TABLE t1 ADD INDEX k_c2(c2), add index k_c3(c3);
ERROR HY000: Temporary file write failure.
SET GLOBAL debug='-d,row_merge_write_failure';


#TEST CREATE TMP FILE FAILURE
SET session innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=2;
SET SESSION innodb_parallel_read_threads=2;
SET GLOBAL debug="+d,row_merge_file_create_if_needed_INJECT_ERR";
ALTER TABLE t1 ADD INDEX k_c2(c2), add index k_c3(c3);
ERROR HY000: Out of memory; check if mysqld or some other process uses all available memory; if not, you may have to use 'ulimit' to allow mysqld to use more memory or you can add more swap space
SET GLOBAL debug="-d,row_merge_file_create_if_needed_INJECT_ERR";


#TEST CREATE FTS
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
SET GLOBAL debug="+d,force_parallel";
ALTER TABLE t1 ADD FULLTEXT INDEX fts_c2(c2);
Warnings:
Warning	124	InnoDB rebuilding table to add column FTS_DOC_ID
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SET GLOBAL debug="-d,force_parallel";
ALTER TABLE t1 DROP INDEX fts_c2;


#TEST CREATE CLUSTERED INDEX
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
SET GLOBAL debug="+d,force_parallel";
CREATE FULLTEXT INDEX c_c2 ON t1(c2);
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SET GLOBAL debug="-d,force_parallel";
ALTER TABLE t1 DROP INDEX c_c2;


#TEST CREATE INDEX ON VFIELD
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
CREATE TABLE t_vf (c1 INT PRIMARY KEY, c2 INT, c3 INT  GENERATED ALWAYS AS (c2));
SET GLOBAL debug="+d,force_parallel";
ALTER TABLE t_vf ADD INDEX k_c3 (c3);
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
SET GLOBAL debug="-d,force_parallel";
DROP TABLE t_vf;


#TEST SORT BUF FAILURE
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
SET GLOBAL debug="+d,sort_merge_buf_failure";
SET GLOBAL debug="+d,ib_row_merge_buf_add_two";
ALTER TABLE t1 ADD INDEX k_c2(c2), ADD INDEX k_c3(c3);
ERROR HY000: Temporary file write failure.
SET GLOBAL debug="-d,sort_merge_buf_failure";
SET GLOBAL debug="-d,ib_row_merge_buf_add_two";


#TEST ADD BUF FAILURE
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
SET GLOBAL debug="+d,ib_row_merge_buf_add_failure";
ALTER TABLE t1 ADD INDEX k_c2(c2), ADD INDEX k_c3(c3);
ERROR HY000: Got error 11 - 'InnoDB error' from storage engine
SET GLOBAL debug="-d,sort_merge_buf_failure";


#TEST RE-ADD BUF FAILURE
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
SET GLOBAL debug="";
SET GLOBAL debug="+d,ib_row_merge_buf_add_two";
SET GLOBAL debug="+d,ib_row_merge_buf_readd_failure";
ALTER TABLE t1 ADD INDEX k_c2(c2), ADD INDEX k_c3(c3);
ERROR HY000: Got error 11 - 'InnoDB error' from storage engine
SET GLOBAL debug="-d,ib_row_merge_buf_add_two";
SET GLOBAL debug="-d,ib_row_merge_buf_readd_failure";
SET GLOBAL debug="";


#TEST TRX INTERRUPTED
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
SET GLOBAL debug="";
SET GLOBAL debug="+d,trx_is_interrupted_yes";
SET GLOBAL debug="+d,ib_row_merge_buf_add_two";
ALTER TABLE t1 ADD INDEX k_c2(c2);
ERROR 70100: Query execution was interrupted
SET GLOBAL debug="-d,trx_is_interrupted_yes";
SET GLOBAL debug="-d,ib_row_merge_buf_add_two";
SET GLOBAL debug="";


#TEST WRITE MERGE BUF FAIL WITH ADD INJECT
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
SET GLOBAL debug="";
SET GLOBAL debug="+d,write_merge_buf_to_file_failure";
SET GLOBAL debug="+d,ib_row_merge_buf_add_two";
ALTER TABLE t1 ADD INDEX k_c2(c2);
ERROR HY000: Temporary file write failure.
SET GLOBAL debug="-d,write_merge_buf_to_file_failure";
SET GLOBAL debug="-d,ib_row_merge_buf_add_two";
SET GLOBAL debug="";


#TEST GLOBAL_ERROR FLAG SET
SET SESSION innodb_parallel_ddl=on;
SET SESSION innodb_ddl_threads=16;
set session innodb_parallel_read_threads=16;
SET GLOBAL debug="+d,set_global_err";
ALTER TABLE t1 ADD INDEX k_c2(c2), ADD INDEX k_c3(c3);
ERROR HY000: Got error 11 - 'InnoDB error' from storage engine
SET GLOBAL debug="-d,set_global_err";


#TEST NON-PARALLEL SINCE parallel param is off
SET GLOBAL innodb_parallel_ddl=OFF;
SET GLOBAL debug="";
ALTER TABLE t1 ADD INDEX k_c2(c2), add index k_c3(c3);
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c2 FROM t1 FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c2 FROM t1 FORCE INDEX (k_c2);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
DROP TABLE pri_res;
DROP TABLE idx_res;
CHECK TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c3 FROM t1 FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c3 FROM t1 FORCE INDEX (k_c3);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
DROP TABLE pri_res;
DROP TABLE idx_res;
ALTER TABLE t1 DROP INDEX k_c2, DROP INDEX k_c3;
DROP TABLE t1;


#TEST GEOM TABLE
CREATE TABLE geom (g GEOMETRY NOT NULL);
ANALYZE TABLE geom;
Table	Op	Msg_type	Msg_text
test.geom	analyze	status	OK
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
SET GLOBAL debug="+d,force_parallel";
ALTER TABLE geom ADD SPATIAL INDEX(g);
Warnings:
Warning	3674	The spatial index on column 'g' will not be used by the query optimizer since the column does not have an SRID attribute. Consider adding an SRID attribute to the column.
CHECK TABLE geom;
Table	Op	Msg_type	Msg_text
test.geom	check	status	OK
SET GLOBAL debug="-d,force_parallel";
DROP TABLE geom;


#TEST TABLE WITH MULTI-VALUE COLUMN TYPE
CREATE TABLE customers (id int primary key, custinfo JSON);
ANALYZE TABLE customers;
Table	Op	Msg_type	Msg_text
test.customers	analyze	status	OK
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
SET GLOBAL debug="+d,force_parallel";
CREATE INDEX zips ON customers ( (CAST(custinfo->'$.zipcode' AS UNSIGNED ARRAY)) );
CHECK TABLE customers;
Table	Op	Msg_type	Msg_text
test.customers	check	status	OK
SET GLOBAL debug="-d,force_parallel";
DROP TABLE customers;


#TEST TABLE WITH NOT ENOUGHT RECORDS
CREATE TABLE t2 (c1 int primary key, c2 int) ENGINE=INNODB;
INSERT INTO t2 VALUES(1,1);
INSERT INTO t2 VALUES(2,2);
INSERT INTO t2 VALUES(3,3);
INSERT INTO t2 VALUES(4,4);
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
SET GLOBAL debug="";
ALTER TABLE t2 ADD INDEX k_c2(c2);
CHECK TABLE t2;
Table	Op	Msg_type	Msg_text
test.t2	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c2 FROM t2 FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c2 FROM t2 FORCE INDEX (k_c2);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
DROP TABLE pri_res;
DROP TABLE idx_res;
CHECK TABLE t2;
Table	Op	Msg_type	Msg_text
test.t2	check	status	OK
DROP TABLE t2;


#TEST TABLE WITH DUP RECORDS
CREATE TABLE t3 (c1 char(127) PRIMARY KEY, c2 char(127), c3 char(127)) ENGINE=INNODB;
ANALYZE TABLE t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	Error	Table 'test.t1' doesn't exist
test.t1	analyze	status	Operation failed
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
ALTER TABLE t3 ADD UNIQUE INDEX k_c2(c2);
ERROR 23000: Duplicate entry '0000000000000000000000000000000000000000000000000000000000000000' for key 't3.k_c2'
DROP TABLE t3;


#TEST TABLE WITH COMPACT ROW FORMAT
CREATE TABLE t4 (c1 char(127) PRIMARY KEY, c2 char(127), c3 char(127)) ENGINE=INNODB ROW_FORMAT=COMPACT;
ANALYZE TABLE t4;
Table	Op	Msg_type	Msg_text
test.t4	analyze	status	OK
SET SESSION innodb_parallel_ddl=ON;
SET SESSION innodb_ddl_threads=16;
SET SESSION innodb_parallel_read_threads=16;
ALTER TABLE t4 ADD INDEX k_c2(c2), add unique index k_c3(c3);
CHECK TABLE t4;
Table	Op	Msg_type	Msg_text
test.t4	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c2 FROM t4 FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c2 FROM t4 FORCE INDEX (k_c2);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
DROP TABLE pri_res;
DROP TABLE idx_res;
CHECK TABLE t4;
Table	Op	Msg_type	Msg_text
test.t4	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c3 FROM t4 FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c3 FROM t4 FORCE INDEX (k_c3);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
DROP TABLE pri_res;
DROP TABLE idx_res;
ALTER TABLE t4 DROP INDEX k_c2, DROP INDEX k_c3;
DROP TABLE t4;


#TEST AUTOINC
CREATE TABLE t_ai (c1 INT PRIMARY KEY, c2 char(127), c3 char(127)) ENGINE=INNODB;
ANALYZE TABLE t_ai;
Table	Op	Msg_type	Msg_text
test.t_ai	analyze	status	OK
ALTER TABLE t_ai ADD INDEX k_c2(c2), add unique index k_c3(c3);
CHECK TABLE t_ai;
Table	Op	Msg_type	Msg_text
test.t_ai	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c2 FROM t_ai FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c2 FROM t_ai FORCE INDEX (k_c2);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c2 != idx_res.c2;
c1	c2	c1	c2
DROP TABLE pri_res;
DROP TABLE idx_res;
CHECK TABLE t_ai;
Table	Op	Msg_type	Msg_text
test.t_ai	check	status	OK
CREATE TEMPORARY TABLE pri_res SELECT c1, c3 FROM t_ai FORCE INDEX (PRI);
CREATE TEMPORARY TABLE idx_res SELECT c1, c3 FROM t_ai FORCE INDEX (k_c3);
SELECT * FROM pri_res LEFT JOIN idx_res ON pri_res.c1 = idx_res.c1 WHERE idx_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
SELECT * FROM idx_res LEFT JOIN pri_res ON pri_res.c1 = idx_res.c1 WHERE pri_res.c1 IS NULL OR pri_res.c3 != idx_res.c3;
c1	c3	c1	c3
DROP TABLE pri_res;
DROP TABLE idx_res;
DROP TABLE t_ai;
SET GLOBAL innodb_ddl_threads=4;

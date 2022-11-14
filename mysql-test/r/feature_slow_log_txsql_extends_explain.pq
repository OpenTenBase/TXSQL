SET @old_log_output = @@global.log_output;
SET @old_slow_query_log = @@global.slow_query_log;
SET @old_slow_query_log_file = @@global.slow_query_log_file;
SET @old_long_query_time = @@global.long_query_time;
SET @old_log_slow_verbosity = @@global.log_slow_verbosity;
SET GLOBAL slow_query_log_file = '.../log/explain_in_slow.log';
SET GLOBAL log_output = 'FILE';
SET GLOBAL slow_query_log = 'ON';
SET SESSION long_query_time = 0;
SELECT @@global.slow_query_log_file;
SET SESSION long_query_time = 1;
select 1;
1
1
select sleep(1);
sleep(1)
0
SET SESSION long_query_time = 0;
SELECT @@session.log_slow_verbosity;
@@session.log_slow_verbosity

set session log_slow_verbosity = "explain";
SELECT @@session.log_slow_verbosity;
@@session.log_slow_verbosity
explain
select 1;
1
1
select sleep(1);
sleep(1)
0
select concat("Hello", " ", "World.");
concat("Hello", " ", "World.")
Hello World.
create table t1 (a int, b int);
create table t2 (a int, b int);
show tables;
Tables_in_test
t1
t2
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `a` int DEFAULT NULL,
  `b` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
SELECT * FROM t1 ot WHERE ot.b<0 OR ot.a IN (SELECT it.a+1 FROM t2 it);
a	b
SELECT t1.* FROM t1 LEFT OUTER JOIN 
(SELECT COUNT(a) AS cnt FROM t2) AS derived
ON TRUE
WHERE t1.a > derived.cnt;
a	b
CREATE TABLE test_log (sql_text TEXT);
LOAD DATA LOCAL INFILE '.../log/explain_in_slow.log'
  INTO TABLE test_log FIELDS TERMINATED BY '\n' LINES TERMINATED BY '\n';
select sql_text from test_log where sql_text like "# explain:%" or 
(sql_text not like "SET timestamp=%" and 
sql_text not like "# %" and 
(sql_text like "create%" or
sql_text like "select%" or
sql_text like "show%" or
sql_text like "use%" or
sql_text like "set%"));
sql_text
CREATE TABLE test_log (sql_text TEXT);
SELECT * FROM t1 ot WHERE ot.b<0 OR ot.a IN (SELECT it.a+1 FROM t2 it);
SELECT @@global.slow_query_log_file;
SELECT @@global.slow_query_log_file;
SELECT @@session.log_slow_verbosity;
SELECT @@session.log_slow_verbosity;
SELECT t1.* FROM t1 LEFT OUTER JOIN 
SET SESSION long_query_time = 0;
SET SESSION long_query_time = 0;
create table t1 (a int, b int);
create table t2 (a int, b int);
select 1;
select concat("Hello", " ", "World.");
select sleep(1);
select sleep(1);
set session log_slow_verbosity = "explain";
show create table t1;
show tables;
use test;
drop table t1, t2;
drop table test_log;
SET SESSION long_query_time = @old_long_query_time;
SET SESSION log_slow_verbosity = @old_log_slow_verbosity;
SET GLOBAL  slow_query_log_file = @old_slow_query_log_file;
SET GLOBAL  slow_query_log = @old_slow_query_log;
SET GLOBAL  log_output = @old_log_output;

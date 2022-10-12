CREATE SCHEMA xtest;
CREATE TABLE xtest.xtable (value FLOAT);
Got expected error: Invalid value of placeholder (code 5154)

2 rows affected
Records: 2  Duplicates: 0  Warnings: 0

2 rows affected
Records: 2  Duplicates: 0  Warnings: 0

1 rows affected
Mysqlx.Ok {
  msg: "bye!"
}
ok
SELECT * FROM xtest.xtable;
value
23.1
34.2
45.4
56.3
67.5

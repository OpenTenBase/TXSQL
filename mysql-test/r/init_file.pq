INSERT INTO init_file.startup VALUES ( NOW() );
SELECT * INTO @X FROM init_file.startup limit 0,1;
SELECT * INTO @Y FROM init_file.startup limit 1,1;
SELECT YEAR(@X)-YEAR(@Y);
YEAR(@X)-YEAR(@Y)
0
DROP DATABASE init_file;
ok
end of 4.1 tests
select * from t1;
x
11
13
3
5
7
select * from t2;
y
11
13
3
30
drop table t1, t2;

reset master;
create table t1 (a int);
insert into t1 values (1);
insert into t1 values (2);
insert into t1 values (3);
update t1 set a=a+2 where a=2;
update t1 set a=a+2 where a=3;
create table t2 (word varchar(20));
load data infile '../../std_data/words.dat' into table t2;
flush logs;
drop table t1;
drop table t2;
RESET MASTER;
select * from t1;
a
1
4
5
select * from t2;
word
Aarhus
Aarhus
Aaron
Aaron
Ababa
Ababa
Abba
Abbott
Abby
Abe
Abel
Abelian
Abelson
Aberdeen
Abernathy
aback
aback
abaft
abaft
abandon
abandon
abandoned
abandoned
abandoning
abandoning
abandonment
abandonment
abandons
abandons
abase
abased
abasement
abasements
abases
abash
abashed
abashes
abashing
abasing
abate
abated
abatement
abatements
abater
abates
abating
abbe
abbey
abbeys
abbot
abbots
abbreviate
abbreviated
abbreviates
abbreviating
abbreviation
abbreviations
abdomen
abdomens
abdominal
abduct
abducted
abduction
abductions
abductor
abductors
abducts
abed
aberrant
aberration
flush logs;
drop table t2;
create table t2 (word varchar(20));
load data infile '../../std_data/words.dat' into table t2;
insert into t2 select * from t2;
insert into t2 select * from t2;
insert into t2 select * from t2;
insert into t2 select * from t2;
insert into t2 select * from t2;
insert into t2 select * from t2;
insert into t2 select * from t2;
insert into t2 select * from t2;
insert into t2 select * from t2;
select count(*) from t2;
count(*)
35840
flush logs;
RESET MASTER;
select count(*) from t2;
count(*)
35840
drop table t1;
drop table t2;
RESET MASTER;
USE test;
SET @old_binlog_format= @@binlog_format;
SET SESSION binlog_format='ROW';
CREATE TABLE t1(c1 INT);
INSERT INTO t1 VALUES (1);
FLUSH LOGS;
DROP TABLE t1;
SET SESSION binlog_format= @old_binlog_format;
RESET MASTER;

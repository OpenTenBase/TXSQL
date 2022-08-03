select @test_compress_string:='string for test compress function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa ';
@test_compress_string:='string for test compress function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa '
Warning	1287	Setting user variables within expressions is deprecated and will be removed in a future release. Consider alternatives: 'SET variable=expression, ...', or 'SELECT expression(s) INTO variables(s)'.
Warnings:
string for test compress function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 
select length(@test_compress_string);
length(@test_compress_string)
117
select uncompress(compress(@test_compress_string));
uncompress(compress(@test_compress_string))
string for test compress function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 
explain select uncompress(compress(@test_compress_string));
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	NULL	NULL	NULL	NULL	NULL	NULL	NULL	NULL	NULL	No tables used
Warnings:
Note	1003	/* select#1 */ select uncompress(compress((@`test_compress_string`))) AS `uncompress(compress(@test_compress_string))`
select uncompressed_length(compress(@test_compress_string))=length(@test_compress_string);
uncompressed_length(compress(@test_compress_string))=length(@test_compress_string)
1
explain select uncompressed_length(compress(@test_compress_string))=length(@test_compress_string);
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	NULL	NULL	NULL	NULL	NULL	NULL	NULL	NULL	NULL	No tables used
Warnings:
Note	1003	/* select#1 */ select (uncompressed_length(compress((@`test_compress_string`))) = length((@`test_compress_string`))) AS `uncompressed_length(compress(@test_compress_string))=length(@test_compress_string)`
select uncompressed_length(compress(@test_compress_string));
uncompressed_length(compress(@test_compress_string))
117
select length(compress(@test_compress_string))<length(@test_compress_string);
length(compress(@test_compress_string))<length(@test_compress_string)
1
create table t1 (a blob, b binary(255), c char(4)) engine=innodb;
insert into t1 (a,b,c) values (compress(@test_compress_string),compress(@test_compress_string),'d ');
select uncompress(a) from t1;
uncompress(a)
string for test compress function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 
select uncompress(b) from t1;
uncompress(b)
string for test compress function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 
select concat('|',c,'|') from t1;
concat('|',c,'|')
|d|
drop table t1;
select compress("");
compress("")

select uncompress("");
uncompress("")

select uncompress(compress(""));
uncompress(compress(""))

select uncompressed_length("");
uncompressed_length("")
0
create table t1 (a blob);
insert t1 values (compress(null)), ('A\0\0\0BBBBBBBB'), (compress(space(50000))), (space(50000));
select length(a) from t1;
length(a)
12
50000
76
NULL
select length(uncompress(a)) from t1;
length(uncompress(a))
50000
NULL
NULL
NULL
Warning	1256	Uncompressed data size too large; the maximum size is 67108864 (probably, length of uncompressed data was corrupted)
Warning	1259	ZLIB: Input data corrupted
Warnings:
drop table t1;
set @@global.max_allowed_packet=1048576*100;
select compress(repeat('aaaaaaaaaa', IF(XXX, 10, 10000000))) is null;
compress(repeat('aaaaaaaaaa', IF(XXX, 10, 10000000))) is null
0
set @@global.max_allowed_packet=default;
create table t1(a blob);
insert into t1 values(NULL), (compress('a'));
select uncompress(a), uncompressed_length(a) from t1;
uncompress(a)	uncompressed_length(a)
NULL	NULL
a	1
drop table t1;
create table t1(a blob);
insert into t1 values ('0'), (NULL), ('0');
select compress(a), compress(a) from t1;
select compress(a) is null from t1;
compress(a) is null
0
0
1
drop table t1;
End of 4.1 tests
create table t1 (a varchar(32) not null);
insert into t1 values ('foo');
analyze table t1;
Table	Op	Msg_type	Msg_text
test.t1	analyze	status	OK
explain select * from t1 where uncompress(a) is null;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Parallel scan (1 workers); Using where
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a` from `test`.`t1` where (uncompress(`test`.`t1`.`a`) is null)
select * from t1 where uncompress(a) is null;
a
Warning	1259	ZLIB: Input data corrupted
Warnings:
foo
explain select *, uncompress(a) from t1;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	SIMPLE	t1	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Parallel scan (1 workers)
1	SIMPLE	<sender1>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Send to (<receiver1>)
1	SIMPLE	<receiver1>	NULL	ALL	NULL	NULL	NULL	NULL	1	100.00	Receive from (<sender1>)
Warnings:
Note	1003	/* select#1 */ select `test`.`t1`.`a` AS `a`,uncompress(`test`.`t1`.`a`) AS `uncompress(a)` from `test`.`t1`
select *, uncompress(a) from t1;
a	uncompress(a)
Warning	1259	ZLIB: Input data corrupted
Warnings:
foo	NULL
select *, uncompress(a), uncompress(a) is null from t1;
a	uncompress(a)	uncompress(a) is null
Warning	1259	ZLIB: Input data corrupted
Warning	1259	ZLIB: Input data corrupted
Warnings:
foo	NULL	1
drop table t1;
CREATE TABLE t1 (c1 INT);
INSERT INTO t1 VALUES (1), (1111), (11111);
SELECT UNCOMPRESS(c1), UNCOMPRESSED_LENGTH(c1) FROM t1;
UNCOMPRESS(c1)	UNCOMPRESSED_LENGTH(c1)
NULL	0
NULL	0
NULL	825307441
EXPLAIN SELECT * FROM (SELECT UNCOMPRESSED_LENGTH(c1) FROM t1) AS s;
DROP TABLE t1;
End of 5.0 tests
#
# Bug#18693654 VALGRIND WARNINGS IN INFLATE ON UNCOMPRESS
#
SELECT UNCOMPRESS( CAST( 0 AS BINARY(5) ) );
UNCOMPRESS( CAST( 0 AS BINARY(5) ) )
NULL
Warning	1259	ZLIB: Input data corrupted
Warnings:

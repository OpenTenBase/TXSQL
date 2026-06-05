CREATE TABLE t1(a INT PRIMARY KEY, b INT, c varchar(100));
INSERT INTO t1 VALUES (1,2,"abc"),(2,3,"bcd"),(3,4,"cde"),(4,5,"def");
UPDATE t1 SET b = b + 1 WHERE a = 2 RETURNING a;
a
2
UPDATE t1 SET b = b + 1 WHERE a = 2 RETURNING a, b;
a	b
2	5
UPDATE t1 SET b = b + 1 WHERE a = 2 RETURNING a, b, c;
a	b	c
2	6	bcd
UPDATE t1 SET b = b + 1 WHERE a = 2 RETURNING a+1, b, c;
a+1	b	c
3	7	bcd
UPDATE t1 SET b = b + 1 WHERE a = 2 RETURNING *;
a	b	c
2	8	bcd
UPDATE t1 SET b = b + 1 RETURNING *;
a	b	c
1	3	abc
2	9	bcd
3	5	cde
4	6	def
UPDATE t1 SET b = b + 1 WHERE a = 2 and c = 'bcd' RETURNING *;
a	b	c
2	10	bcd
DELETE FROM t1 WHERE a = 2 RETURNING b+1,c;
b+1	c
11	bcd
DELETE FROM t1 WHERE a = 1 RETURNING *;
a	b	c
1	3	abc
DELETE FROM t1 WHERE a = 3 and b = 4 RETURNING *;
a	b	c
DELETE FROM t1 RETURNING *;
a	b	c
3	5	cde
4	6	def
DROP TABLE t1;
CREATE TABLE t1 (a INT PRIMARY KEY, b INT, c INT);
INSERT INTO t1 VALUES(1,2,3),(3,4, 7),(5,6, 11);
SELECT * FROM t1;
a	b	c
1	2	3
3	4	7
5	6	11
CREATE TABLE t2 (a INT PRIMARY KEY, b INT, c INT as (a+b));
INSERT INTO t2(a,b) VALUES(1,2),(3,4),(5,6);
SELECT * FROM t2;
a	b	c
1	2	3
3	4	7
5	6	11
UPDATE t1 SET b=3 WHERE a=1 RETURNING c;
c
3
SELECT * FROM t1;
a	b	c
1	3	3
3	4	7
5	6	11
UPDATE t1 SET b=2 WHERE a=1 RETURNING *;
a	b	c
1	2	3
SELECT * FROM t1;
a	b	c
1	2	3
3	4	7
5	6	11
UPDATE t1 SET b=b*2 RETURNING b, c+b as d;
b	d
4	7
8	15
12	23
SELECT * FROM t1;
a	b	c
1	4	3
3	8	7
5	12	11
UPDATE t1 SET b=b+2 RETURNING *, c+b as d;
a	b	c	d
1	6	3	9
3	10	7	17
5	14	11	25
SELECT * FROM t1;
a	b	c
1	6	3
3	10	7
5	14	11
UPDATE t1 SET c=c+1 WHERE 1 RETURNING a,t.*,a,b,c;
ERROR 42S02: Unknown table 't'
UPDATE t1 SET b=b*2 RETURNING a, c+b as d, (SELECT count(*) FROM t2) as x;
a	d	x
1	15	3
3	27	3
5	39	3
SELECT * FROM t1;
a	b	c
1	12	3
3	20	7
5	28	11
SELECT * FROM t2;
a	b	c
1	2	3
3	4	7
5	6	11
SELECT a,t2.*,b FROM t2;
a	a	b	c	b
1	1	2	3	2
3	3	4	7	4
5	5	6	11	6
SELECT a,*,b FROM t2;
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near '*,b FROM t2' at line 1
UPDATE t2 SET b=b+2 WHERE a=5 and b>2 RETURNING c,b,a;
c	b	a
13	8	5
SELECT * FROM t1;
a	b	c
1	12	3
3	20	7
5	28	11
SELECT * FROM t2;
a	b	c
1	2	3
3	4	7
5	8	13
UPDATE t2 SET b=b*c RETURNING t2.a, *, b, c+b as d;
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near '*, b, c+b as d' at line 1
SELECT * FROM t2;
a	b	c
1	2	3
3	4	7
5	8	13
UPDATE t2 SET b=b*c RETURNING t2.a, t2.*, b, c+b as d;
a	a	b	c	b	d
1	1	6	7	6	13
3	3	28	31	28	59
5	5	104	109	104	213
SELECT * FROM t2;
a	b	c
1	6	7
3	28	31
5	104	109
UPDATE t2 SET b=b*2 RETURNING a, b, c+b as d, t2.*, (SELECT t1.b + t1.c FROM t1 WHERE t1.a=t2.a) as x;
a	b	d	a	b	c	x
1	12	25	1	12	13	15
3	56	115	3	56	59	27
5	208	421	5	208	213	39
SELECT * FROM t1;
a	b	c
1	12	3
3	20	7
5	28	11
SELECT * FROM t2;
a	b	c
1	12	13
3	56	59
5	208	213
UPDATE t2 SET b=b/2 limit 2 RETURNING sqrt(a+sqrt(a)) as s,t2.*, (SELECT count(*) FROM t1) as x;
s	a	b	c	x
1.4142135623730951	1	6	7	3
2.1753277471610746	3	28	31	3
SELECT * FROM t1;
a	b	c
1	12	3
3	20	7
5	28	11
SELECT * FROM t2;
a	b	c
1	6	7
3	28	31
5	208	213
UPDATE t1 SET c=c+1 WHERE 1 or (a=6 and b>4) RETURNING a;
a
1
3
5
SELECT * FROM t1;
a	b	c
1	12	4
3	20	8
5	28	12
SELECT * FROM t2;
a	b	c
1	6	7
3	28	31
5	208	213
explain UPDATE t2 SET b=b*2 RETURNING a, b, c+b as d, t2.*, (SELECT t1.b + t1.c FROM t1 WHERE t1.a=t2.a) as x;
id	select_type	table	partitions	type	possible_keys	key	key_len	ref	rows	filtered	Extra
1	UPDATE	t2	NULL	index	NULL	PRIMARY	4	NULL	3	100.00	NULL
2	DEPENDENT SUBQUERY	t1	NULL	eq_ref	PRIMARY	PRIMARY	4	test.t2.a	1	100.00	NULL
Warnings:
Note	1276	Field or reference 'test.t2.a' of SELECT #2 was resolved in SELECT #1
Note	1003	update `test`.`t2` set `test`.`t2`.`b` = (`test`.`t2`.`b` * 2)
SELECT * FROM t1;
a	b	c
1	12	4
3	20	8
5	28	12
DELETE FROM t1 WHERE a=1 RETURNING *;
a	b	c
1	12	4
SELECT * FROM t1;
a	b	c
3	20	8
5	28	12
DELETE FROM t1 WHERE a=3 RETURNING a+b, b-c as m;
a+b	m
23	12
SELECT * FROM t1;
a	b	c
5	28	12
SELECT * FROM t2;
a	b	c
1	6	7
3	28	31
5	208	213
DELETE FROM t1 WHERE a=6 and b>4 RETURNING sqrt(a+sqrt(b)) as s, t1.*, (SELECT count(*) FROM t2) as x;
s	a	b	c	x
SELECT * FROM t1;
a	b	c
5	28	12
SELECT * FROM t2;
a	b	c
1	6	7
3	28	31
5	208	213
DELETE FROM t2 limit 2 RETURNING *,a-b;
a	b	c	a-b
1	6	7	-5
3	28	31	-25
DELETE FROM t1 limit 1 RETURNING a,t.*,a,b,c;
ERROR 42S02: Unknown table 't'
SELECT * FROM t1;
a	b	c
5	28	12
SELECT * FROM t2;
a	b	c
5	208	213
DELETE FROM t1 WHERE 1 or (a=6 and b>4) RETURNING a;
a
5
SELECT * FROM t1;
a	b	c
explain format=json DELETE FROM t1 WHERE a=6 and b>4 RETURNING sqrt(a+sqrt(b)) as s, t1.*, (SELECT count(*) FROM t2) as x;
EXPLAIN
{
  "query_block": {
    "select_id": 1,
    "table": {
      "delete": true,
      "table_name": "t1",
      "access_type": "range",
      "possible_keys": [
        "PRIMARY"
      ],
      "key": "PRIMARY",
      "used_key_parts": [
        "a"
      ],
      "key_length": "4",
      "ref": [
        "const"
      ],
      "rows_examined_per_scan": 1,
      "filtered": "100.00",
      "attached_condition": "((`test`.`t1`.`a` = 6) and (`test`.`t1`.`b` > 4))"
    },
    "returning_list_subqueries": [
      {
        "dependent": false,
        "cacheable": true,
        "query_block": {
          "select_id": 2,
          "cost_info": {
            "query_cost": "0.35"
          },
          "table": {
            "table_name": "t2",
            "access_type": "index",
            "key": "PRIMARY",
            "used_key_parts": [
              "a"
            ],
            "key_length": "4",
            "rows_examined_per_scan": 1,
            "rows_produced_per_join": 1,
            "filtered": "100.00",
            "using_index": true,
            "cost_info": {
              "read_cost": "0.25",
              "eval_cost": "0.10",
              "prefix_cost": "0.35",
              "data_read_per_join": "16"
            }
          }
        }
      }
    ]
  }
}
Warnings:
Note	1003	delete from `test`.`t1` where ((`test`.`t1`.`a` = 6) and (`test`.`t1`.`b` > 4))
DROP TABLE t1;
DROP TABLE t2;
CREATE TABLE t1(id1 INT PRIMARY KEY, val1 VARCHAR(1));
CREATE TABLE t2(id2 INT PRIMARY KEY, val2 VARCHAR(1));
INSERT INTO t1 VALUES(1, 'a'), (2, 'b'), (3, 'c');
INSERT INTO t2 SELECT * FROM t1;
DELETE FROM t1 WHERE id1=100000 RETURNING val1;
val1
DELETE FROM t1 RETURNING val2;
ERROR 42S22: Unknown column 'val2' in 'field list'
SELECT * FROM t1;
id1	val1
1	a
2	b
3	c
DELETE FROM t1 WHERE id1=1 RETURNING (SELECT MAX(id2) FROM t2);
(SELECT MAX(id2) FROM t2)
3
DELETE FROM t1 WHERE RETURNING MAX(id1);
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'RETURNING MAX(id1)' at line 1
DELETE FROM t1;
INSERT INTO t1 VALUES(1, 'a'), (2, 'b'), (3, 'c');
DELETE FROM t1 WHERE id1=1 RETURNING (SELECT id2 FROM t2);
ERROR 21000: Subquery returns more than 1 row
DELETE FROM t2 LIMIT 2;
DELETE FROM t1 WHERE id1=2 RETURNING (SELECT * FROM t2);
ERROR 21000: Operand should contain 1 column(s)
DELETE FROM t1 RETURNING (SELECT id1 FROM t1);
ERROR HY000: You can't specify target table 't1' for update in FROM clause
DELETE FROM t1;
INSERT INTO t1 VALUES (1,'a'),(2,'b'),(3,'c');
CREATE VIEW v1 AS SELECT * FROM t1;
UPDATE v1 SET val1='d' RETURNING id1;
id1
1
2
3
SELECT * FROM v1;
id1	val1
1	d
2	d
3	d
DROP VIEW v1;
CREATE VIEW v2(a,c) AS SELECT id1, COUNT(val1) FROM t1 GROUP BY id1;
UPDATE v2 set val1='e' RETURNING * ;
ERROR HY000: The target table v2 of the UPDATE is not updatable
DROP VIEW v2;
DROP TABLE t1;
DROP TABLE t2;
CREATE TABLE table_a (id int, value int);
CREATE TABLE table_b (id int, ta_id int, value int);
INSERT INTO table_a VALUES (1, 10);
INSERT INTO table_a VALUES (2, 20);
INSERT INTO table_a VALUES (3, 30);
INSERT INTO table_b VALUES (1, 1, 100);
INSERT INTO table_b VALUES (2, 1, 200);
INSERT INTO table_b VALUES (3, 2, 300);
INSERT INTO table_b VALUES (4, 2, 400);
CREATE VIEW v AS
SELECT      a.id a_id, b.id b_id, b.ta_id, a.value v1, b.value v2
FROM        table_a a
INNER JOIN  table_b b ON (b.ta_id = a.id);
INSERT INTO v (a_id, v1) VALUES (3, 30) returning *;
ERROR HY000: Feature returning with multi table is unsupported ().
INSERT INTO v (a_id, v1) VALUES (3, 30);
update v set a_id = 2 returning *;
ERROR HY000: Feature returning with multi table is unsupported ().
drop view v;
drop table table_a;
drop table table_b;
CREATE TABLE `sbtest9` (
`id` int NOT NULL,
`c1` int DEFAULT NULL,
`c2` char(16) DEFAULT NULL,
`c3` char(64) NOT NULL,
`c4` varchar(128) DEFAULT NULL,
`c5` datetime(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
`c6` text,
`c7` char(64) DEFAULT NULL,
`c8` varchar(256) DEFAULT NULL,
`c9` blob,
`c10` varchar(128) DEFAULT NULL,
`c11` varchar(128) DEFAULT NULL,
`c12` varchar(128) DEFAULT NULL,
`c13` varchar(128) DEFAULT NULL,
`c14` varchar(128) DEFAULT NULL,
`c15` varchar(128) DEFAULT NULL,
`c16` varchar(128) DEFAULT NULL,
`c17` varchar(128) DEFAULT NULL,
`c18` varchar(128) DEFAULT NULL,
`c19` varchar(128) DEFAULT NULL,
`c20` varchar(128) DEFAULT NULL,
`c21` varchar(128) DEFAULT NULL,
`c22` varchar(128) DEFAULT NULL,
`c23` varchar(128) DEFAULT NULL,
`c24` varchar(128) DEFAULT NULL,
`c25` varchar(128) DEFAULT NULL,
`c26` varchar(128) DEFAULT NULL,
`c27` varchar(128) DEFAULT NULL,
`c28` varchar(128) DEFAULT NULL,
`c29` varchar(128) DEFAULT NULL,
`c30` varchar(128) DEFAULT NULL,
`c31` varchar(128) DEFAULT NULL,
`c32` varchar(128) DEFAULT NULL,
`c33` varchar(128) DEFAULT NULL,
`c34` varchar(128) DEFAULT NULL,
`c35` char(143) DEFAULT NULL,
`c36` varchar(128) DEFAULT NULL,
`c37` varchar(128) DEFAULT NULL,
`c38` varchar(128) DEFAULT NULL,
`c39` varchar(128) DEFAULT NULL,
`c40` varchar(128) DEFAULT NULL,
`c41` varchar(128) DEFAULT NULL,
`c42` varchar(128) DEFAULT NULL,
`c43` varchar(128) DEFAULT NULL,
`c44` varchar(128) DEFAULT NULL,
`c45` varchar(128) DEFAULT NULL,
`c46` varchar(128) DEFAULT NULL,
`c47` varchar(128) DEFAULT NULL,
`c48` varchar(128) DEFAULT NULL,
`c49` varchar(128) DEFAULT NULL,
`c50` varchar(128) DEFAULT NULL,
`c51` varchar(128) DEFAULT NULL,
`c52` varchar(128) DEFAULT NULL,
`c53` varchar(128) DEFAULT NULL,
`c54` varchar(128) DEFAULT NULL,
`c55` varchar(128) DEFAULT NULL,
`c56` varchar(128) DEFAULT NULL,
`c57` varchar(128) DEFAULT NULL,
`c58` varchar(128) DEFAULT NULL,
`c59` varchar(128) DEFAULT NULL,
`c60` varchar(128) DEFAULT NULL,
`c61` varchar(128) DEFAULT NULL,
`c62` varchar(128) DEFAULT NULL,
`c63` varchar(128) DEFAULT NULL,
`c64` varchar(128) DEFAULT NULL,
`c65` varchar(128) DEFAULT NULL,
`c66` varchar(128) DEFAULT NULL,
`c67` varchar(128) DEFAULT NULL,
`c68` varchar(128) DEFAULT NULL,
`c69` varchar(128) DEFAULT NULL,
`last_ts` datetime(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
PRIMARY KEY (`id`,`c3`,`c5`),
KEY `idx_c1` (`c1`),
KEY `idx_c2` (`c2`)
) ENGINE=INNODB DEFAULT CHARSET=utf8mb3;
Warnings:
Warning	1287	'utf8mb3' is deprecated and will be removed in a future release. Please use utf8mb4 instead
UPDATE
sbtest9
set
c1 = -6324,
c2 = '@#$   ',
c5 = '2003-10-09 03:25:15.389172',
last_ts = '2025-06-18 14:30:02.549981',
c3 = 'gfedclk      jihqponm         ',
c4 = 'gfedclkjihVUTSRQPONMLKJ                     IHGFEDCBAzyxwvutsrqponm                        ',
c6 = 'HIJ   KLMN   ',
c7 = 'gfedclkjihHGFEDC                                                BAzyxwvutsrqponm                                                ',
c8 = 'edcbajihgffedc*&^%$#@!ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponm                                                            lkjihgfedcba*&^%$#@!ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponmlk                                                               ',
c9 = 'cdef   '
where
id = 704 RETURNING *;
id	c1	c2	c3	c4	c5	c6	c7	c8	c9	c10	c11	c12	c13	c14	c15	c16	c17	c18	c19	c20	c21	c22	c23	c24	c25	c26	c27	c28	c29	c30	c31	c32	c33	c34	c35	c36	c37	c38	c39	c40	c41	c42	c43	c44	c45	c46	c47	c48	c49	c50	c51	c52	c53	c54	c55	c56	c57	c58	c59	c60	c61	c62	c63	c64	c65	c66	c67	c68	c69	last_ts
drop table sbtest9;

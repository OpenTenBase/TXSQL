CREATE TABLE t (
id int NOT NULL,
doc_id int DEFAULT NULL,
doc json DEFAULT NULL,
PRIMARY KEY (id),
KEY mv_id (id,(cast(json_extract(doc,'$.val') as unsigned array))),
KEY mv_doc_id (doc_id,(cast(json_extract(doc,'$.val') as unsigned array)))
);
INSERT INTO t values (1, 2, '{"val":[1,2,3,4,5]}'), (2, 1, '{"val":[2,3,4,5]}'), (3, 2, '{"val":[2,3,4,5]}'), (4, NULL, '{"val":[3,4,5]}'), (5, 2, '{"val":[4,5]}');
ANALYZE TABLE t;
Table	Op	Msg_type	Msg_text
test.t	analyze	status	OK
EXPLAIN FORMAT=TREE SELECT COUNT(*) FROM t FORCE INDEX (mv_id) WHERE id = '1';
EXPLAIN
-> Aggregate: count(0)  (cost=1.50 rows=1)
    -> Index lookup on t using mv_id (id=1)  (cost=1.00 rows=5)

SELECT COUNT(*) FROM t FORCE INDEX (mv_id) WHERE id = '1';
COUNT(*)
1
DROP TABLE t;

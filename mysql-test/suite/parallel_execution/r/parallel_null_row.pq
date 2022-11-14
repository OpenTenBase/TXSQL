set optimizer_switch='batched_key_access=on,block_nested_loop=off,mrr_cost_based=off';
set optimizer_switch='condition_fanout_filter=off';
CREATE TABLE a (id INTEGER);
CREATE TABLE b (id INTEGER);
CREATE ALGORITHM=MERGE VIEW vmerge AS SELECT 1 AS id, id AS b_id FROM b;
CREATE ALGORITHM=TEMPTABLE VIEW vmat AS SELECT 1 AS id, id AS b_id FROM b;
INSERT INTO a(id) VALUES (1);
SELECT *
FROM a LEFT JOIN vmerge AS v ON a.id = v.id;
id	id	b_id
1	NULL	NULL
drop view vmerge;
drop view vmat;
DROP TABLE a,b;

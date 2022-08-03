CREATE TABLE t1(c1 INT, c2 char(20)) ENCRYPTION="Y" ENGINE = InnoDB;
ALTER TABLE test.t1 DISCARD TABLESPACE;
ALTER TABLE test.t1 IMPORT TABLESPACE;
SELECT * FROM t1;
c1	c2
0	aaaaa
0	aaaaa
0	aaaaa
0	aaaaa
1	bbbbb
1	bbbbb
1	bbbbb
1	bbbbb
2	ccccc
2	ccccc
2	ccccc
2	ccccc
3	ddddd
3	ddddd
3	ddddd
3	ddddd
4	eeeee
4	eeeee
4	eeeee
4	eeeee
5	fffff
5	fffff
5	fffff
5	fffff
6	ggggg
6	ggggg
6	ggggg
6	ggggg
7	hhhhh
7	hhhhh
7	hhhhh
7	hhhhh
8	iiiii
8	iiiii
8	iiiii
8	iiiii
9	jjjjj
9	jjjjj
9	jjjjj
9	jjjjj
DROP TABLE t1;

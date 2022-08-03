# Test for 'instant add column' and 'instant modify column'
set global cdb_instant_modify_column_enabled=on;
CREATE TABLE `sbtest`(
`id` int NOT NULL AUTO_INCREMENT,
`k` int NOT NULL DEFAULT '0',
`c` char(120) NOT NULL DEFAULT '',
`pad` char(60) NOT NULL DEFAULT '',
PRIMARY KEY (`id`),
KEY `k_1` (`k`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
INSERT INTO sbtest(k,c,pad) values(222,'32872105378-79313759117-49621625454-91304065241-64247960720-05073279609','92000168187-59139270393-89355074292-45650165525');
ALTER TABLE sbtest modify c varchar(150) NOT NULL DEFAULT '',algorithm=instant;
INSERT INTO sbtest(k,c,pad) values(222,'32872105378-79313759117-49621625454-91304065241-64247960720-05073279609','92000168187-59139270393-89355074292-45650165525');
CREATE TABLE `sbtest1`(
`id` int NOT NULL AUTO_INCREMENT,
`k` int NOT NULL DEFAULT '0',
`c` char(120) NOT NULL DEFAULT '',
`pad` char(60) NOT NULL DEFAULT '',
PRIMARY KEY (`id`),
KEY `k_1` (`k`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
INSERT INTO sbtest1(k,c,pad) values(222,'32872105378-79313759117-49621625454-91304065241-64247960720-05073279609','92000168187-59139270393-89355074292-45650165525');
ALTER TABLE sbtest1 add column d int default 0, algorithm = instant;
INSERT INTO sbtest1(k,c,pad,d) values(222,'32872105378-79313759117-49621625454-91304065241-64247960720-05073279609','92000168187-59139270393-89355074292-45650165525', 1);
# Restart to make sure that all pages have been flushed
# restart
Start analyzing test/sbtest.
Finish analyzing test/sbtest, 0 corrupted pages found.
Normal/Warning data has been dumped to test.sbtest.normal.sql-n and test.sbtest.warning.sql.
Please refer to test.sbtest.info.txt to get more details.
Start analyzing test/sbtest1.
Finish analyzing test/sbtest1, 0 corrupted pages found.
Normal/Warning data has been dumped to test.sbtest1.normal.sql-n and test.sbtest1.warning.sql.
Please refer to test.sbtest1.info.txt to get more details.
DELETE FROM sbtest;
DELETE FROM sbtest1;
DROP TABLE sbtest, sbtest1;

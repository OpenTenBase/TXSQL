use test;
CREATE TABLE `role` (
`playerId` bigint(20) NOT NULL,
`userName` varchar(180) NOT NULL,
`playerName` varchar(50) NOT NULL,
`lv` int(11) NOT NULL,
`serverId` int(11) NOT NULL,
`loginTime` bigint(20) NOT NULL,
`loginOutTime` bigint(20) NOT NULL,
`data` longtext NOT NULL,
PRIMARY KEY (`playerId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
#
# Test externally field's page is corrupted.
Start analyzing test/role.
start_slot > the total number of slots in page!
Finish analyzing test/role, 1 corrupted pages found.
Normal/Warning data has been dumped to test.role.normal.sql-n and test.role.warning.sql.
Please refer to test.role.info.txt to get more details.
# restart
# Remove test.role.normal.sql-0 to avoid its impact on following tests
#
# Test create_sql mode with primary key specified.
Start analyzing test/role.
Finish analyzing test/role, 0 corrupted pages found.
Normal/Warning data has been dumped to test.role.normal.sql-n and test.role.warning.sql.
Please refer to test.role.info.txt to get more details.
# restart
DELETE FROM role;
# Remove test.role.normal.sql-0 to avoid its impact on following tests
#
# Test recovery from sdi information with primary key specified.
Start analyzing test/role.
Finish analyzing test/role, 0 corrupted pages found.
Normal/Warning data has been dumped to test.role.normal.sql-n and test.role.warning.sql.
Please refer to test.role.info.txt to get more details.
# restart
DELETE FROM role;
# Remove test.role.normal.sql-0 to avoid its impact on following tests
CREATE TABLE `vt` (
`a` int(11) DEFAULT NULL,
`b` int(11) DEFAULT NULL,
`c` int(11) GENERATED ALWAYS AS ((`a` + `b`)) VIRTUAL,
`d` varchar(60) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
CREATE PROCEDURE load_vt (count INT UNSIGNED)
BEGIN
DECLARE i INT UNSIGNED DEFAULT 1;
DECLARE s VARCHAR(60) DEFAULT REPEAT('x', 60);
WHILE i <= count DO
INSERT INTO vt(a,b,d) VALUES(i,i,s);
SET i=i+1;
END WHILE;
END;
//
call load_vt(1000);
#
# Test virtual generated column.
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
# restart
DELETE FROM vt;
# Remove test.vt.normal.sql-0 to avoid its impact on following tests
# Test create_sql mode when ibdata does not exist and the default characterset is not specified
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
# restart
DELETE FROM vt;
# Remove test.vt.normal.sql-0 to avoid its impact on following tests
# Test recovery from sdi information in virtual generated column situation.
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
# restart
DELETE FROM vt;
# Remove test.vt.normal.sql-0 to avoid its impact on following tests
#
# Test full scan mode with cluster index id
# restart
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
DELETE FROM vt;
# Remove test.vt.normal.sql-0 to avoid its impact on following tests
#
# Test the next record address is invalid.
DELETE FROM vt where a = 2;
DELETE FROM vt where a = 3;
Start analyzing test/vt.
Finish analyzing test/vt, 1 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
# restart
DELETE FROM vt;
# Remove test.vt.normal.sql-0 and test.vt.warning.sql to avoid its impact on following tests
#
# Test leaf level scan.
DELETE FROM vt WHERE a = '1';
Start analyzing test/vt.
Finish analyzing test/vt, 1 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
# restart
DELETE FROM vt;
# Remove test.vt.normal.sql-0 and test.vt.warning.sql to avoid its impact on following tests
#
# Test range scan.
Start analyzing test/vt.
Finish analyzing test/vt, 1 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
# restart
DELETE FROM vt;
# Remove test.vt.normal.sql-0 and test.vt.warning.sql to avoid its impact on following tests
#
# Test instant add column in normal mode
DELETE FROM vt;
ALTER TABLE vt ADD COLUMN ic int, algorithm=instant;
INSERT INTO vt(a, b, d, ic) VALUES(1, 1, '#ac', 1);
INSERT INTO vt(a, b, d, ic) VALUES(2, 2, '#ab', 1);
INSERT INTO vt(a, b, d, ic) VALUES(3, 2, '#aa', 3);
INSERT INTO vt(a, b, d, ic) VALUES(4, 3, '#xc', 3);
INSERT INTO vt(a, b, d, ic) VALUES(5, 5, '#ad', 3);
# restart
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
DELETE FROM vt;
# Remove test.vt.normal.sql-0 to avoid its impact on following tests
#
# Test instant add column in create_sql mode
# restart
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
DELETE FROM vt;
# Remove test.vt.normal.sql-0 to avoid its impact on following tests
#
# Test recovery from sdi information in instant add column situation
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
# restart
DELETE FROM vt;
# Remove test.vt.normal.sql-0 to avoid its impact on following tests
#
# Test cluster index root page corrupted.
DROP TABLE vt;
CREATE TABLE `vt` (
`a` int(11) DEFAULT NULL,
`b` int(11) DEFAULT NULL,
`c` int(11) GENERATED ALWAYS AS ((`a` + `b`)) VIRTUAL,
`d` varchar(60) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
# restart
DELETE FROM vt;
# Remove test.vt.normal.sql-0 and test.vt.warning.sql to avoid its impact on following tests
#
# Test dump to a remote database.
DELETE FROM vt;
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
# Remove test.vt.normal.sql-0 and test.vt.warning.sql to avoid its impact on following tests
#
# Test all parameters 1.
DELETE FROM vt;
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
DELETE FROM vt;
#
# Test all parameters 2.
DELETE FROM vt;
Start analyzing test/vt.
Finish analyzing test/vt, 0 corrupted pages found.
Normal/Warning data has been dumped to test.vt.normal.sql-n and test.vt.warning.sql.
Please refer to test.vt.info.txt to get more details.
DELETE FROM vt;
#
# Test create_sql's table_name is incorrect.
Start analyzing test/role2.
Finish analyzing test/role2, 0 corrupted pages found.
Normal/Warning data has been dumped to test.role2.normal.sql-n and test.role2.warning.sql.
Please refer to test.role2.info.txt to get more details.
# restart
CREATE TABLE `role2` (
`playerId` bigint(20) NOT NULL,
`userName` varchar(180) NOT NULL,
`playerName` varchar(50) NOT NULL,
`lv` int(11) NOT NULL,
`serverId` int(11) NOT NULL,
`loginTime` bigint(20) NOT NULL,
`loginOutTime` bigint(20) NOT NULL,
`data` longtext NOT NULL,
PRIMARY KEY (`playerId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
#
# Test create statement's table definition is incorrect 1 (loss of primary key information).
# Test whether the program will crash or not.
# restart
Start analyzing test/role2.
Finish analyzing test/role2, 0 corrupted pages found.
Normal/Warning data has been dumped to test.role2.normal.sql-n and test.role2.warning.sql.
Please refer to test.role2.info.txt to get more details.
#
# Test create statement's table definition is incorrect 2 (wrong field type).
# Test whether the program will crash or not.
Start analyzing test/role2.
Finish analyzing test/role2, 0 corrupted pages found.
Normal/Warning data has been dumped to test.role2.normal.sql-n and test.role2.warning.sql.
Please refer to test.role2.info.txt to get more details.
#
# Test create statement's table definition is incorrect 3 (lose field definition).
# Test whether the program will crash or not.
Start analyzing test/role2.
Finish analyzing test/role2, 0 corrupted pages found.
Normal/Warning data has been dumped to test.role2.normal.sql-n and test.role2.warning.sql.
Please refer to test.role2.info.txt to get more details.
#
# Test create statement's table definition is incorrect 4 (extra definition fields are added).
# Test whether the program will crash or not.
Start analyzing test/role2.
Finish analyzing test/role2, 0 corrupted pages found.
Normal/Warning data has been dumped to test.role2.normal.sql-n and test.role2.warning.sql.
Please refer to test.role2.info.txt to get more details.
#
# Test SQL statements with syntax errors.
# Test whether the program will crash or not.
Error occurs in parse create statement!
# Test dump_definition_only
Table definition has been dumped to test.role.def.txt.
DROP TABLE role;
SHOW CREATE TABLE role;
Table	Create Table
role	CREATE TABLE `role` (
  `playerId` bigint NOT NULL,
  `userName` varchar(180) NOT NULL,
  `playerName` varchar(50) NOT NULL,
  `lv` int NOT NULL,
  `serverId` int NOT NULL,
  `loginTime` bigint NOT NULL,
  `loginOutTime` bigint NOT NULL,
  `data` longtext NOT NULL,
  PRIMARY KEY (`playerId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=DYNAMIC
# all done.
DROP TABLE role2;
DROP TABLE role;
DROP TABLE vt;
DROP PROCEDURE load_vt;

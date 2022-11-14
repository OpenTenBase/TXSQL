CREATE TABLE test_encryption(c1 varchar(1) ENCRYPTION, c2 varchar(1) ENCRYPTION algorithm=aes256, c3 varchar(1) column_format ENCRYPTION, c4 varchar(1) column_format ENCRYPTION algorithm=aes128, 
c5 varchar(1) ENCRYPTION algorithm=SM4 ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
SHOW CREATE TABLE test_encryption;
Table	Create Table
test_encryption	CREATE TABLE `test_encryption` (
  `c1` varchar(1) /*!80022 ENCRYPTION ALGORITHM = AES192 */ DEFAULT NULL,
  `c2` varchar(1) /*!80022 ENCRYPTION ALGORITHM = AES256 */ DEFAULT NULL,
  `c3` varchar(1) /*!80022 ENCRYPTION ALGORITHM = AES192 */ DEFAULT NULL,
  `c4` varchar(1) /*!80022 ENCRYPTION ALGORITHM = AES128 */ DEFAULT NULL,
  `c5` varchar(1) /*!80022 ENCRYPTION ALGORITHM = SM4 */ DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
insert into test_encryption values('a','b','c','d', 'e');
insert into test_encryption values('数','据','库','系', '统');
# restart
# Test whether crash will occur in the case of column encryption
Ibddump currently does not support encrypted columns.The parsing data of field c1 is encrypted.
DROP TABLE test_encryption;

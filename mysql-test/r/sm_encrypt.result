SHOW VARIABLES LIKE "block_encryption_mode";
Variable_name	Value
block_encryption_mode	aes-128-ecb
SELECT HEX(SM4_ENCRYPT('kongzhi','test')) AS test;
ERROR HY000: The block encryption mode aes-128-ecb is not supported
SET block_encryption_mode='sm4-128-cbc';
SHOW VARIABLES LIKE "block_encryption_mode";
Variable_name	Value
block_encryption_mode	sm4-128-cbc
SELECT HEX(SM4_ENCRYPT('kongzhi','test12345678901234567890123456789')) AS test;
ERROR HY000: The key length is not legal and its length must be no greater than 32 bytes.
SELECT HEX(SM4_ENCRYPT('kongzhi','test')) AS test;
test
6DD4764B7E0C3A96C23ED09632D4C3BC
SELECT HEX(SM4_ENCRYPT('kongzhi','test', 'test_iv')) AS test;
ERROR HY000: The initialization vector supplied to sm4_encrypt is too short. Must be at least 16 bytes long
SELECT HEX(SM4_ENCRYPT('kongzhi','test', 'test_iv123456789')) AS test;
test
39F97CDCFED6A764707DC12690902C78
SELECT HEX(AES_ENCRYPT('kongzhi','test')) AS test;
test
01047F2C75B904BD88E5B27764B15E22
SET block_encryption_mode='aes-128-ecb';
SELECT SM4_DECRYPT(UNHEX('6DD4764B7E0C3A96C23ED09632D4C3BC'),'test') AS test;
ERROR HY000: The block encryption mode aes-128-ecb is not supported
SET block_encryption_mode='sm4-128-cbc';
SHOW VARIABLES LIKE "block_encryption_mode";
Variable_name	Value
block_encryption_mode	sm4-128-cbc
SELECT SM4_DECRYPT(UNHEX('6DD4764B7E0C3A96C23ED09632D4C3BC'),'test12345678901234567890123456789') AS test;
ERROR HY000: The key length is not legal and its length must be no greater than 32 bytes.
SELECT SM4_DECRYPT(UNHEX('6DD4764B7E0C3A96C23ED09632D4C3BC'),'test') AS test;
test
kongzhi
SELECT SM4_DECRYPT(UNHEX('39F97CDCFED6A764707DC12690902C78'),'test','test_iv') AS test;
ERROR HY000: The initialization vector supplied to sm4_decrypt is too short. Must be at least 16 bytes long
SELECT SM4_DECRYPT(UNHEX('39F97CDCFED6A764707DC12690902C78'),'test','test_iv123456789') AS test;
test
kongzhi
SELECT AES_DECRYPT(UNHEX('01047F2C75B904BD88E5B27764B15E22'),'test') AS test;
test
kongzhi
SET block_encryption_mode='aes-128-ecb';
SELECT HEX(SM3_DIGEST('kongzhi')) AS test;
ERROR HY000: The block encryption mode aes-128-ecb is not supported
SET block_encryption_mode='sm4-128-cbc';
SELECT HEX(SM3_DIGEST('kongzhi')) AS test;
test
F992FD9B6749F754EFF9C270053EBD8AC1272F42FC70A0DE75993CC835D19332
SET block_encryption_mode='aes-128-ecb';
SELECT HEX(SM3_HMAC('kongzhi','test')) AS test;
ERROR HY000: The block encryption mode aes-128-ecb is not supported
SET block_encryption_mode='sm4-128-cbc';
SELECT HEX(SM3_HMAC('kongzhi','test')) AS test;
test
6739F7E1E065C5DA65F8227C946613C1D5A62E43B70E54747670A3F4EF93E0A0

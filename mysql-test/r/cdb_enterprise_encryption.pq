INSTALL PLUGIN cdb_enterprise_encryption SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION modify_public_params RETURNS STRING SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION create_asymmetric_priv_key RETURNS STRING SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION create_asymmetric_pub_key RETURNS STRING SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION asymmetric_derive RETURNS STRING SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION create_digest RETURNS STRING SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION asymmetric_encrypt RETURNS STRING SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION asymmetric_decrypt RETURNS STRING SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION asymmetric_sign RETURNS STRING SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION asymmetric_verify RETURNS STRING SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION count_key_records RETURNS INTEGER SONAME'cdb_enterprise_encryption.so';
CREATE FUNCTION remove_key_records RETURNS STRING SONAME'cdb_enterprise_encryption.so';
SET @bits = 1024;
SELECT MODIFY_PUBLIC_PARAMS('rsa');
ERROR HY000: Can't initialize function 'modify_public_params'; Requires two parameters: the algo and the key length in bits.

SELECT MODIFY_PUBLIC_PARAMS(123456);
ERROR HY000: Can't initialize function 'modify_public_params'; Requires two parameters: the algo and the key length in bits.

SELECT MODIFY_PUBLIC_PARAMS('dh', 'error_input');
ERROR HY000: Can't initialize function 'modify_public_params'; Wrong parameter types.

SELECT MODIFY_PUBLIC_PARAMS('abc', 1024);
ERROR HY000: Can't initialize function 'modify_public_params'; Error algo input.

UNINSTALL PLUGIN cdb_enterprise_encryption;
SELECT MODIFY_PUBLIC_PARAMS('DH', @bits);
ERROR HY000: Can't initialize function 'modify_public_params'; The plugin works unnormally.

SELECT MODIFY_PUBLIC_PARAMS('Dsa', @bits);
ERROR HY000: Can't initialize function 'modify_public_params'; The plugin works unnormally.

INSTALL PLUGIN cdb_enterprise_encryption SONAME'cdb_enterprise_encryption.so';
SELECT MODIFY_PUBLIC_PARAMS('DH', @bits);
MODIFY_PUBLIC_PARAMS('DH', @bits)
Successfully modified!
SELECT MODIFY_PUBLIC_PARAMS('Dsa', @bits);
MODIFY_PUBLIC_PARAMS('Dsa', @bits)
Successfully modified!
SELECT CREATE_ASYMMETRIC_PRIV_KEY('error_input');
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Requires two parameters: the algo and the key length in bits.

SELECT CREATE_ASYMMETRIC_PRIV_KEY(123456);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Requires two parameters: the algo and the key length in bits.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('rsa', 'error_input');
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('abc', 1024);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Error algo input.

SET @priv_rsa = CREATE_ASYMMETRIC_PRIV_KEY('rsa', @bits);
SET @priv_dsa = CREATE_ASYMMETRIC_PRIV_KEY('DSA', @bits);
SET @priv_dh1 = CREATE_ASYMMETRIC_PRIV_KEY('Dh', @bits);
SET @priv_dh2 = CREATE_ASYMMETRIC_PRIV_KEY('dh', @bits);
SELECT CREATE_ASYMMETRIC_PUB_KEY('error_input', @priv_rsa);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Make sure to use the same algo as the priv_key.

SELECT CREATE_ASYMMETRIC_PUB_KEY('rsa', 123456);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PUB_KEY('dsa', @priv_dsa, 'error_input');
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Requires two parameters: the algo and the priv_key.

SELECT CREATE_ASYMMETRIC_PUB_KEY('rsa', 'error_input');
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong priv_key format.

SET @pub_rsa = CREATE_ASYMMETRIC_PUB_KEY('RSA', @priv_rsa);
SET @pub_dsa = CREATE_ASYMMETRIC_PUB_KEY('Dsa', @priv_dsa);
SET @pub_dh1 = CREATE_ASYMMETRIC_PUB_KEY('DH', @priv_dh1);
SET @pub_dh2 = CREATE_ASYMMETRIC_PUB_KEY('dh', @priv_dh2);
SELECT ASYMMETRIC_DERIVE(123456, @pub_dh1);
ERROR HY000: Can't initialize function 'asymmetric_derive'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_DERIVE(@priv_dh2, @pub_dh1, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_derive'; Requires two parameters: one dh_priv_key and the other dh_pub_key.

SELECT ASYMMETRIC_DERIVE('error_input', @pub_dh1);
ERROR HY000: Can't initialize function 'asymmetric_derive'; Wrong priv_key format.

SELECT ASYMMETRIC_DERIVE(@priv_dh2, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_derive'; Wrong pub_key format.

SET @sym_key1 = ASYMMETRIC_DERIVE(@priv_dh1, @pub_dh2);
SET @sym_key2 = ASYMMETRIC_DERIVE(@priv_dh2, @pub_dh1);
SELECT STRCMP(@sym_key1, @sym_key2);
STRCMP(@sym_key1, @sym_key2)
0
SET @ori_msg = 'My secret message';
SELECT ASYMMETRIC_ENCRYPT(123456);
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Requires two parameters: the message to encrypt and one rsa key.

SELECT ASYMMETRIC_ENCRYPT(@ori_msg, 123456);
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_ENCRYPT(@ori_msg, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Wrong key format.

SET @ecpt_msg1 = ASYMMETRIC_ENCRYPT(@ori_msg, @priv_rsa);
SET @ecpt_msg2 = ASYMMETRIC_ENCRYPT(@ori_msg, @pub_rsa);
SELECT ASYMMETRIC_DECRYPT(@ecpt_msg1);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Requires two parameters: the encrypted message and the other rsa key.

SELECT ASYMMETRIC_DECRYPT(@ecpt_msg1, 123456);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_DECRYPT(@ecpt_msg1, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Wrong key format.

SET @back_msg1 = ASYMMETRIC_DECRYPT(@ecpt_msg1, @pub_rsa);
SET @back_msg2 = ASYMMETRIC_DECRYPT(@ecpt_msg2, @priv_rsa);
SELECT STRCMP(@ori_msg, @back_msg1);
STRCMP(@ori_msg, @back_msg1)
0
SELECT STRCMP(@ori_msg, @back_msg2);
STRCMP(@ori_msg, @back_msg2)
0
SET @clr_msg = 'This is my clear message to sign';
SELECT CREATE_DIGEST(@clr_msg);
ERROR HY000: Can't initialize function 'create_digest'; Requires two parameters: the algo and the clear text.

SELECT CREATE_DIGEST('error_input', @clr_msg);
ERROR HY000: Can't initialize function 'create_digest'; Choose'md5', 'sha1', 'sha224', sha256', 'sha384' or 'sha512' as the first parameter.

SET @hash1 = CREATE_DIGEST('Md5', @clr_msg);
SET @hash1 = CREATE_DIGEST('SHA1', @clr_msg);
SET @hash2 = CREATE_DIGEST('SHA224', @clr_msg);
SET @hash3 = CREATE_DIGEST('Sha256', @clr_msg);
SET @hash4 = CREATE_DIGEST('Sha384', @clr_msg);
SET @hash5 = CREATE_DIGEST('sha512', @clr_msg);
SELECT ASYMMETRIC_SIGN('error_input', @hash1, @priv_rsa);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_SIGN('rsa', @hash1);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN('rsa', @hash1, 123456);
ERROR HY000: Can't initialize function 'asymmetric_sign'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_SIGN('rsa', @hash1, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_sign'; Wrong priv_key format.

SET @sig1 = ASYMMETRIC_SIGN('rsa', @hash5, @priv_rsa);
SET @sig2 = ASYMMETRIC_SIGN('Dsa', @hash5, @priv_dsa);
SELECT ASYMMETRIC_VERIFY('rsa', @hash1, @sig1);
ERROR HY000: Can't initialize function 'asymmetric_verify'; Requires four parameters: the algo, the hash, the sig, the pub_key.

SELECT ASYMMETRIC_VERIFY('error_input', @hash1, @sig1, @pub_rsa);
ERROR HY000: Can't initialize function 'asymmetric_verify'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_VERIFY('dsa', @hash1, @sig1, 123456);
ERROR HY000: Can't initialize function 'asymmetric_verify'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_VERIFY('dsa', @hash1, @sig1, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_verify'; Wrong pub_key format.

SET @rst1 = ASYMMETRIC_VERIFY('RSA', @hash5, @sig1, @pub_rsa);
SET @rst2 = ASYMMETRIC_VERIFY('dsa', @hash5, @sig2, @pub_dsa);
SELECT @rst1, @rst2;
@rst1	@rst2
Succeed!	Succeed!
SELECT COUNT_KEY_RECORDS('rsa', 'error_input');
ERROR HY000: Can't initialize function 'count_key_records'; Requires one parameter: the algo.

SELECT COUNT_KEY_RECORDS('error_input');
ERROR HY000: Can't initialize function 'count_key_records'; Choose 'rsa', 'dh' or 'dsa' as the parameter.

SELECT COUNT_KEY_RECORDS(123456);
ERROR HY000: Can't initialize function 'count_key_records'; Wrong parameter types.

SELECT COUNT_KEY_RECORDS('RSA');
COUNT_KEY_RECORDS('RSA')
1
SELECT COUNT_KEY_RECORDS('dh');
COUNT_KEY_RECORDS('dh')
2
SELECT COUNT_KEY_RECORDS('Dsa');
COUNT_KEY_RECORDS('Dsa')
1
SELECT REMOVE_KEY_RECORDS('rsa', 'error_input');
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS('rsa');
ERROR HY000: Can't initialize function 'remove_key_records'; Requires two parameters: the algo and the counts of the keys to remove.

SELECT REMOVE_KEY_RECORDS(123456);
ERROR HY000: Can't initialize function 'remove_key_records'; Requires two parameters: the algo and the counts of the keys to remove.

SELECT REMOVE_KEY_RECORDS('RSA', 1);
REMOVE_KEY_RECORDS('RSA', 1)
Successfully removed!
SELECT REMOVE_KEY_RECORDS('dh', 2);
REMOVE_KEY_RECORDS('dh', 2)
Successfully removed!
SELECT REMOVE_KEY_RECORDS('Dsa', 1);
REMOVE_KEY_RECORDS('Dsa', 1)
Successfully removed!
DROP FUNCTION IF EXISTS modify_public_params;
DROP FUNCTION IF EXISTS create_asymmetric_priv_key;
DROP FUNCTION IF EXISTS create_asymmetric_pub_key;
DROP FUNCTION IF EXISTS asymmetric_derive;
DROP FUNCTION IF EXISTS create_digest;
DROP FUNCTION IF EXISTS asymmetric_encrypt;
DROP FUNCTION IF EXISTS asymmetric_decrypt;
DROP FUNCTION IF EXISTS asymmetric_sign;
DROP FUNCTION IF EXISTS asymmetric_verify;
DROP FUNCTION IF EXISTS count_key_records;
DROP FUNCTION IF EXISTS remove_key_records;
UNINSTALL PLUGIN cdb_enterprise_encryption;

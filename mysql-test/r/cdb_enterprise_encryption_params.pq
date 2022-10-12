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
SELECT MODIFY_PUBLIC_PARAMS(@not_exist);
ERROR HY000: Can't initialize function 'modify_public_params'; Requires two parameters: the algo and the key length in bits.

SELECT MODIFY_PUBLIC_PARAMS(null);
ERROR HY000: Can't initialize function 'modify_public_params'; Requires two parameters: the algo and the key length in bits.

SELECT MODIFY_PUBLIC_PARAMS('rsa');
ERROR HY000: Can't initialize function 'modify_public_params'; Requires two parameters: the algo and the key length in bits.

SELECT MODIFY_PUBLIC_PARAMS(123456);
ERROR HY000: Can't initialize function 'modify_public_params'; Requires two parameters: the algo and the key length in bits.

SELECT MODIFY_PUBLIC_PARAMS('dh', 'error_input');
ERROR HY000: Can't initialize function 'modify_public_params'; Wrong parameter types.

SELECT MODIFY_PUBLIC_PARAMS('abc', 1024);
ERROR HY000: Can't initialize function 'modify_public_params'; Error algo input.

SELECT MODIFY_PUBLIC_PARAMS(@not_exist, 1024);
ERROR HY000: Can't initialize function 'modify_public_params'; Wrong parameter value. Argument 1 cannot be null

SELECT MODIFY_PUBLIC_PARAMS(@not_exist, null);
ERROR HY000: Can't initialize function 'modify_public_params'; Wrong parameter types.

SELECT MODIFY_PUBLIC_PARAMS(null, 1024);
ERROR HY000: Can't initialize function 'modify_public_params'; Wrong parameter value. Argument 1 cannot be null

SELECT MODIFY_PUBLIC_PARAMS('dh', null);
ERROR HY000: Can't initialize function 'modify_public_params'; Wrong parameter types.

SELECT MODIFY_PUBLIC_PARAMS('dsa', null);
ERROR HY000: Can't initialize function 'modify_public_params'; Wrong parameter types.

SELECT MODIFY_PUBLIC_PARAMS('dh', @not_exist);
ERROR HY000: Can't initialize function 'modify_public_params'; Wrong parameter types.

SELECT MODIFY_PUBLIC_PARAMS('rsa', @not_exist);
ERROR HY000: Can't initialize function 'modify_public_params'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PRIV_KEY(@not_exist);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Requires two parameters: the algo and the key length in bits.

SELECT CREATE_ASYMMETRIC_PRIV_KEY(null);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Requires two parameters: the algo and the key length in bits.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('rsa');
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Requires two parameters: the algo and the key length in bits.

SELECT CREATE_ASYMMETRIC_PRIV_KEY(123456);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Requires two parameters: the algo and the key length in bits.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('dh', 'error_input');
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('abc', 1024);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Error algo input.

SELECT CREATE_ASYMMETRIC_PRIV_KEY(@not_exist, 1024);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Wrong parameter value. Argument 1 cannot be null

SELECT CREATE_ASYMMETRIC_PRIV_KEY(@not_exist, null);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PRIV_KEY(null, 1024);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Wrong parameter value. Argument 1 cannot be null

SELECT CREATE_ASYMMETRIC_PRIV_KEY('dh', null);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('dsa', null);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('rsa', null);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('rsa', @not_exist);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('abc', 1024);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Error algo input.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('abc', 1024*1024*1024);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; Error algo input.

SELECT CREATE_ASYMMETRIC_PRIV_KEY('rsa', 1024*1024*1024);
ERROR HY000: Can't initialize function 'create_asymmetric_priv_key'; The key bits are too big for rsa.

SELECT CREATE_ASYMMETRIC_PUB_KEY(@not_exist);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Requires two parameters: the algo and the priv_key.

SELECT CREATE_ASYMMETRIC_PUB_KEY(null);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Requires two parameters: the algo and the priv_key.

SELECT CREATE_ASYMMETRIC_PUB_KEY('rsa');
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Requires two parameters: the algo and the priv_key.

SELECT CREATE_ASYMMETRIC_PUB_KEY(123456);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Requires two parameters: the algo and the priv_key.

SELECT CREATE_ASYMMETRIC_PUB_KEY('dh', 'error_input');
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong priv_key format.

SELECT CREATE_ASYMMETRIC_PUB_KEY('abc', 1024);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PUB_KEY(@not_exist, 1024);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PUB_KEY(@not_exist, null);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter value. Argument 1 cannot be null

SELECT CREATE_ASYMMETRIC_PUB_KEY(null, 1024);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PUB_KEY('dh', null);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter value. Argument 2 cannot be null

SELECT CREATE_ASYMMETRIC_PUB_KEY('dsa', null);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter value. Argument 2 cannot be null

SELECT CREATE_ASYMMETRIC_PUB_KEY('rsa', null);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter value. Argument 2 cannot be null

SELECT CREATE_ASYMMETRIC_PUB_KEY('rsa', @not_exist);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter value. Argument 2 cannot be null

SELECT CREATE_ASYMMETRIC_PUB_KEY('abc', 1024);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PUB_KEY('abc', 1024*1024*1024);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter types.

SELECT CREATE_ASYMMETRIC_PUB_KEY('rsa', 1024*1024*1024);
ERROR HY000: Can't initialize function 'create_asymmetric_pub_key'; Wrong parameter types.

SELECT ASYMMETRIC_DERIVE(@not_exist);
ERROR HY000: Can't initialize function 'asymmetric_derive'; Requires two parameters: one dh_priv_key and the other dh_pub_key.

SELECT ASYMMETRIC_DERIVE('one input');
ERROR HY000: Can't initialize function 'asymmetric_derive'; Requires two parameters: one dh_priv_key and the other dh_pub_key.

SELECT ASYMMETRIC_DERIVE('frist', 'second', 'third');
ERROR HY000: Can't initialize function 'asymmetric_derive'; Requires two parameters: one dh_priv_key and the other dh_pub_key.

SELECT ASYMMETRIC_DERIVE(null);
ERROR HY000: Can't initialize function 'asymmetric_derive'; Requires two parameters: one dh_priv_key and the other dh_pub_key.

SELECT ASYMMETRIC_DERIVE(@not_exist, @not_exist);
ERROR HY000: Can't initialize function 'asymmetric_derive'; Wrong priv_key format.

SELECT ASYMMETRIC_DERIVE('first', @not_exist);
ERROR HY000: Can't initialize function 'asymmetric_derive'; Wrong priv_key format.

SELECT ASYMMETRIC_DERIVE(@not_exist, 'second');
ERROR HY000: Can't initialize function 'asymmetric_derive'; Wrong priv_key format.

SELECT ASYMMETRIC_DERIVE(null, null);
ERROR HY000: Can't initialize function 'asymmetric_derive'; Wrong priv_key format.

SELECT ASYMMETRIC_DERIVE('first', null);
ERROR HY000: Can't initialize function 'asymmetric_derive'; Wrong priv_key format.

SELECT ASYMMETRIC_DERIVE(null, 'second');
ERROR HY000: Can't initialize function 'asymmetric_derive'; Wrong priv_key format.

SET @clr_msg = 'This is my clear message to sign';
SELECT CREATE_DIGEST(@not_exist);
ERROR HY000: Can't initialize function 'create_digest'; Requires two parameters: the algo and the clear text.

SELECT CREATE_DIGEST(null);
ERROR HY000: Can't initialize function 'create_digest'; Requires two parameters: the algo and the clear text.

SELECT CREATE_DIGEST('md5');
ERROR HY000: Can't initialize function 'create_digest'; Requires two parameters: the algo and the clear text.

SELECT CREATE_DIGEST(123456);
ERROR HY000: Can't initialize function 'create_digest'; Requires two parameters: the algo and the clear text.

SELECT CREATE_DIGEST('md5', @not_exist);
CREATE_DIGEST('md5', @not_exist)
d41d8cd98f00b204
SELECT CREATE_DIGEST('md5', null);
CREATE_DIGEST('md5', null)
d41d8cd98f00b204
SELECT CREATE_DIGEST('md5', '');
CREATE_DIGEST('md5', '')
d41d8cd98f00b204
SELECT CREATE_DIGEST('md5', 123);
ERROR HY000: Can't initialize function 'create_digest'; All parameter types should be STRING_RESULT.

SELECT CREATE_DIGEST('abc', 1024);
ERROR HY000: Can't initialize function 'create_digest'; All parameter types should be STRING_RESULT.

SELECT CREATE_DIGEST(@not_exist, null);
ERROR HY000: Can't initialize function 'create_digest'; Wrong parameter value. Argument 1 cannot be null

SELECT CREATE_DIGEST('', null);
ERROR HY000: Can't initialize function 'create_digest'; Choose'md5', 'sha1', 'sha224', sha256', 'sha384' or 'sha512' as the first parameter.

SELECT CREATE_DIGEST(null, 1024);
ERROR HY000: Can't initialize function 'create_digest'; All parameter types should be STRING_RESULT.

SELECT CREATE_DIGEST('error_alg', @clr_msg);
ERROR HY000: Can't initialize function 'create_digest'; Choose'md5', 'sha1', 'sha224', sha256', 'sha384' or 'sha512' as the first parameter.

SELECT CREATE_DIGEST(@clr_msg);
ERROR HY000: Can't initialize function 'create_digest'; Requires two parameters: the algo and the clear text.

SET @ori_msg = 'My secret message';
SELECT ASYMMETRIC_ENCRYPT(123456);
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Requires two parameters: the message to encrypt and one rsa key.

SELECT ASYMMETRIC_ENCRYPT(null);
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Requires two parameters: the message to encrypt and one rsa key.

SELECT ASYMMETRIC_ENCRYPT(@not_exist);
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Requires two parameters: the message to encrypt and one rsa key.

SELECT ASYMMETRIC_ENCRYPT('');
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Requires two parameters: the message to encrypt and one rsa key.

SELECT ASYMMETRIC_ENCRYPT(@not_exist, @not_exist);
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Wrong key format.

SELECT ASYMMETRIC_ENCRYPT(null, null);
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Wrong key format.

SELECT ASYMMETRIC_ENCRYPT('', '');
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Wrong key format.

SELECT ASYMMETRIC_ENCRYPT(@ori_msg, 123456);
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_ENCRYPT(@ori_msg, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; Wrong key format.

SET @bits = 1024;
SET @priv_rsa = CREATE_ASYMMETRIC_PRIV_KEY('rsa', @bits);
set @encrypt = ASYMMETRIC_ENCRYPT('', @priv_rsa);
set @encrypt = ASYMMETRIC_ENCRYPT(null, @priv_rsa);
set @encrypt = ASYMMETRIC_ENCRYPT(@not_exist, @priv_rsa);
SELECT ASYMMETRIC_ENCRYPT(123, @priv_rsa);
ERROR HY000: Can't initialize function 'asymmetric_encrypt'; All parameter types should be STRING_RESULT.

SET @ori_msg = 'My secret message';
SELECT ASYMMETRIC_DECRYPT(123456);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Requires two parameters: the encrypted message and the other rsa key.

SELECT ASYMMETRIC_DECRYPT(null);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Requires two parameters: the encrypted message and the other rsa key.

SELECT ASYMMETRIC_DECRYPT(@not_exist);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Requires two parameters: the encrypted message and the other rsa key.

SELECT ASYMMETRIC_DECRYPT('');
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Requires two parameters: the encrypted message and the other rsa key.

SELECT ASYMMETRIC_DECRYPT(@not_exist, @not_exist);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Wrong key format.

SELECT ASYMMETRIC_DECRYPT(null, null);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Wrong key format.

SELECT ASYMMETRIC_DECRYPT('', '');
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Wrong key format.

SELECT ASYMMETRIC_DECRYPT(@ori_msg, 123456);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_DECRYPT(@ori_msg, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Wrong key format.

SET @ori_msg = 'My secret message';
SET @priv_rsa = CREATE_ASYMMETRIC_PRIV_KEY('rsa', @bits);
SET @ecpt_msg1 = ASYMMETRIC_ENCRYPT(@ori_msg, @priv_rsa);
SET @back_msg1 = ASYMMETRIC_DECRYPT(@ecpt_msg1, '');
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Wrong key format.

SET @back_msg1 = ASYMMETRIC_DECRYPT('', @priv_rsa);
SET @back_msg1 = ASYMMETRIC_DECRYPT(null, @priv_rsa);
SET @back_msg1 = ASYMMETRIC_DECRYPT(@not_exist, @priv_rsa);
SET @back_msg1 = ASYMMETRIC_DECRYPT('abc', @priv_rsa);
SELECT ASYMMETRIC_DECRYPT(@ecpt_msg1);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Requires two parameters: the encrypted message and the other rsa key.

SELECT ASYMMETRIC_DECRYPT(@ecpt_msg1, 123456);
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_DECRYPT(@ecpt_msg1, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_decrypt'; Wrong key format.

SELECT ASYMMETRIC_SIGN('abc');
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN(123);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN(@not_exist);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN(null);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN('rsa', '');
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN('rsa', null);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN('rsa', '1');
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN('rsa', 1);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN('rsa', @not_exist);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Requires two parameters: the algo, the hash and the priv_key.

SELECT ASYMMETRIC_SIGN('rsa', '', '');
ERROR HY000: Can't initialize function 'asymmetric_sign'; Wrong priv_key format.

SELECT ASYMMETRIC_SIGN('rsa', null, null);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Wrong priv_key format.

SELECT ASYMMETRIC_SIGN('rsa', @not_exist, @not_exist);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Wrong priv_key format.

SELECT ASYMMETRIC_SIGN('error', '', '');
ERROR HY000: Can't initialize function 'asymmetric_sign'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_SIGN('error', null, null);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_SIGN('error', @not_exist, @not_exist);
ERROR HY000: Can't initialize function 'asymmetric_sign'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_SIGN('error', '', '');
ERROR HY000: Can't initialize function 'asymmetric_sign'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_SIGN('error', 'asdf', 'asdf');
ERROR HY000: Can't initialize function 'asymmetric_sign'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_VERIFY('rsa', '', '', '');
ERROR HY000: Can't initialize function 'asymmetric_verify'; Wrong pub_key format.

SELECT ASYMMETRIC_VERIFY('rsa', @not_exist, @not_exist, @not_exist);
ERROR HY000: Can't initialize function 'asymmetric_verify'; Wrong pub_key format.

SELECT ASYMMETRIC_VERIFY('rsa', null, null, null);
ERROR HY000: Can't initialize function 'asymmetric_verify'; Wrong pub_key format.

SELECT ASYMMETRIC_VERIFY('asdf', '', '', '');
ERROR HY000: Can't initialize function 'asymmetric_verify'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_VERIFY('asdf', @not_exist, @not_exist, @not_exist);
ERROR HY000: Can't initialize function 'asymmetric_verify'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_VERIFY('asdf', null, null, null);
ERROR HY000: Can't initialize function 'asymmetric_verify'; Choose 'rsa' or 'dsa' as the first parameter.

SELECT ASYMMETRIC_VERIFY('error_input', 1, 1, 1);
ERROR HY000: Can't initialize function 'asymmetric_verify'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_VERIFY('dsa', 'asdf', 'asdf', 'asdf');
ERROR HY000: Can't initialize function 'asymmetric_verify'; Wrong pub_key format.

SELECT ASYMMETRIC_VERIFY('dsa', @not_exist, null, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_verify'; Wrong pub_key format.

SELECT ASYMMETRIC_VERIFY('dsa', 123, 123, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_verify'; All parameter types should be STRING_RESULT.

SELECT ASYMMETRIC_VERIFY('asdf', null, 123, 'error_input');
ERROR HY000: Can't initialize function 'asymmetric_verify'; All parameter types should be STRING_RESULT.

SELECT COUNT_KEY_RECORDS(null);
ERROR HY000: Can't initialize function 'count_key_records'; Wrong parameter value. Argument 1 cannot be null or empty

SELECT COUNT_KEY_RECORDS('');
ERROR HY000: Can't initialize function 'count_key_records'; Choose 'rsa', 'dh' or 'dsa' as the parameter.

SELECT COUNT_KEY_RECORDS(@not_exist);
ERROR HY000: Can't initialize function 'count_key_records'; Wrong parameter value. Argument 1 cannot be null or empty

SELECT COUNT_KEY_RECORDS('error_input');
ERROR HY000: Can't initialize function 'count_key_records'; Choose 'rsa', 'dh' or 'dsa' as the parameter.

SELECT COUNT_KEY_RECORDS(123456);
ERROR HY000: Can't initialize function 'count_key_records'; Wrong parameter types.

SELECT COUNT_KEY_RECORDS('rsa', 'error_input');
ERROR HY000: Can't initialize function 'count_key_records'; Requires one parameter: the algo.

SELECT REMOVE_KEY_RECORDS('rsa');
ERROR HY000: Can't initialize function 'remove_key_records'; Requires two parameters: the algo and the counts of the keys to remove.

SELECT REMOVE_KEY_RECORDS('asd');
ERROR HY000: Can't initialize function 'remove_key_records'; Requires two parameters: the algo and the counts of the keys to remove.

SELECT REMOVE_KEY_RECORDS(@not_exist);
ERROR HY000: Can't initialize function 'remove_key_records'; Requires two parameters: the algo and the counts of the keys to remove.

SELECT REMOVE_KEY_RECORDS(null);
ERROR HY000: Can't initialize function 'remove_key_records'; Requires two parameters: the algo and the counts of the keys to remove.

SELECT REMOVE_KEY_RECORDS('');
ERROR HY000: Can't initialize function 'remove_key_records'; Requires two parameters: the algo and the counts of the keys to remove.

SELECT REMOVE_KEY_RECORDS(123456);
ERROR HY000: Can't initialize function 'remove_key_records'; Requires two parameters: the algo and the counts of the keys to remove.

SELECT REMOVE_KEY_RECORDS(null, null);
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS('', '');
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS(@not_exist, @not_exis);
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS(@not_exist, '');
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS('asdf', null);
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS('asdf', '');
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS('asdf', @not_exist);
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS('asdf', 1123);
REMOVE_KEY_RECORDS('asdf', 1123)
NULL
SELECT REMOVE_KEY_RECORDS(null, 1);
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter value. Argument 1 cannot be null or empty

SELECT REMOVE_KEY_RECORDS(@not_exist, 1);
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter value. Argument 1 cannot be null or empty

SELECT REMOVE_KEY_RECORDS('', 1);
REMOVE_KEY_RECORDS('', 1)
NULL
SELECT REMOVE_KEY_RECORDS('rsa', @not_exist);
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS('rsa', 'error_input');
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS('rsa', null);
ERROR HY000: Can't initialize function 'remove_key_records'; Wrong parameter types.

SELECT REMOVE_KEY_RECORDS('rsa', 1024*1024*1024);
REMOVE_KEY_RECORDS('rsa', 1024*1024*1024)
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

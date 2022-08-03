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
SET @bits=1024;
SELECT COUNT_KEY_RECORDS('RSA');
COUNT_KEY_RECORDS('RSA')
0
SELECT COUNT_KEY_RECORDS('dh');
COUNT_KEY_RECORDS('dh')
0
SELECT COUNT_KEY_RECORDS('Dsa');
COUNT_KEY_RECORDS('Dsa')
0
SET @priv_rsa = CREATE_ASYMMETRIC_PRIV_KEY('rsa', @bits);
SET @priv_dsa = CREATE_ASYMMETRIC_PRIV_KEY('DSA', @bits);
SET @priv_dh1 = CREATE_ASYMMETRIC_PRIV_KEY('Dh', @bits);
SET @priv_dh2 = CREATE_ASYMMETRIC_PRIV_KEY('dh', @bits);
SELECT COUNT_KEY_RECORDS('RSA');
COUNT_KEY_RECORDS('RSA')
1
SELECT COUNT_KEY_RECORDS('dh');
COUNT_KEY_RECORDS('dh')
2
SELECT COUNT_KEY_RECORDS('Dsa');
COUNT_KEY_RECORDS('Dsa')
1
SET @pub_rsa = CREATE_ASYMMETRIC_PUB_KEY('RSA', @priv_rsa);
SET @pub_dsa = CREATE_ASYMMETRIC_PUB_KEY('Dsa', @priv_dsa);
SET @pub_dh1 = CREATE_ASYMMETRIC_PUB_KEY('DH', @priv_dh1);
SET @pub_dh2 = CREATE_ASYMMETRIC_PUB_KEY('dh', @priv_dh2);
SELECT COUNT_KEY_RECORDS('RSA');
COUNT_KEY_RECORDS('RSA')
1
SELECT COUNT_KEY_RECORDS('dh');
COUNT_KEY_RECORDS('dh')
2
SELECT COUNT_KEY_RECORDS('Dsa');
COUNT_KEY_RECORDS('Dsa')
1
SELECT REMOVE_KEY_RECORDS('RSA', 1);
REMOVE_KEY_RECORDS('RSA', 1)
Successfully removed!
SELECT REMOVE_KEY_RECORDS('dh', 2);
REMOVE_KEY_RECORDS('dh', 2)
Successfully removed!
SELECT REMOVE_KEY_RECORDS('Dsa', 1);
REMOVE_KEY_RECORDS('Dsa', 1)
Successfully removed!
SELECT COUNT_KEY_RECORDS('RSA');
COUNT_KEY_RECORDS('RSA')
0
SELECT COUNT_KEY_RECORDS('dh');
COUNT_KEY_RECORDS('dh')
0
SELECT COUNT_KEY_RECORDS('Dsa');
COUNT_KEY_RECORDS('Dsa')
0
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

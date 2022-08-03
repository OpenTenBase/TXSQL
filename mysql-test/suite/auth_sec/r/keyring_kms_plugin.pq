call mtr.add_suppression("\\[ERROR\\] Plugin keyring_kms reported*");
SELECT PLUGIN_NAME, PLUGIN_STATUS, PLUGIN_TYPE, PLUGIN_LIBRARY
FROM INFORMATION_SCHEMA.PLUGINS WHERE PLUGIN_NAME like 'keyring_kms%';
PLUGIN_NAME	PLUGIN_STATUS	PLUGIN_TYPE	PLUGIN_LIBRARY
INSTALL PLUGIN keyring_kms SONAME 'keyring_kms.so';
SELECT PLUGIN_NAME, PLUGIN_STATUS, PLUGIN_TYPE, PLUGIN_LIBRARY
FROM INFORMATION_SCHEMA.PLUGINS WHERE PLUGIN_NAME like 'keyring_kms%';
PLUGIN_NAME	PLUGIN_STATUS	PLUGIN_TYPE	PLUGIN_LIBRARY
keyring_kms	ACTIVE	KEYRING	keyring_kms.so
SHOW VARIABLES LIKE 'keyring_kms_verify';
Variable_name	Value
SHOW STATUS LIKE 'keyring_kms_error';
Variable_name	Value
SET @@global.keyring_kms_file = 'keyring_kms_file';
ERROR HY000: Unknown system variable 'keyring_kms_file'
SET @@global.keyring_kms_config = '{\"secret_id\":\"AKID797wa55caP3VltqoZ9aqumW6at374nXI\",\"secret_key\":\"nV5Uiq3BPbdWJyUo4DB1f7UikaDh3HJC\",\"kms_endpoint\":\"https://kms-dev.api.tencentyun.com\",\"cam_endpoint\":\"https://sts.test.api.qcloud.com\",\"role_name\":\"kmsCDBRole\",\"uin\":3374997818}';
ERROR HY000: Unknown system variable 'keyring_kms_config'
SET @@global.keyring_kms_dump = 1;
ERROR HY000: Unknown system variable 'keyring_kms_dump'
SET @@global.keyring_kms_switch = 1;
ERROR HY000: Unknown system variable 'keyring_kms_switch'
SET @@global.keyring_kms_verify = 1;
ERROR HY000: Unknown system variable 'keyring_kms_verify'
SET @@global.keyring_kms_check = 1;
ERROR HY000: Unknown system variable 'keyring_kms_check'
SHOW VARIABLES LIKE 'keyring_kms_verify';
Variable_name	Value
keyring_kms_verify	ON
SHOW STATUS LIKE 'keyring_kms_error';
Variable_name	Value
keyring_kms_error	
SET @@global.keyring_kms_file = 'keyring_kms_file';
SET @@global.keyring_kms_config = '{\"secret_id\":\"AKID797wa55caP3VltqoZ9aqumW6at374nXI\",\"secret_key\":\"nV5Uiq3BPbdWJyUo4DB1f7UikaDh3HJC\",\"kms_endpoint\":\"https://kms-dev.api.tencentyun.com\",\"cam_endpoint\":\"https://sts.test.api.qcloud.com\",\"role_name\":\"kmsCDBRole\",\"uin\":3374997818}';
ERROR 42000: Variable 'keyring_kms_config' can't be set to the value of '{"secret_id":"AKID797wa55caP3VltqoZ9aqumW6at374nXI","secret_key":"nV5Uiq3BPbdWJyUo4DB1f7UikaDh3HJC","kms_endpoint":"https://kms-dev.api.tencentyun.com","cam_endpoint":"https://sts.test.api.qcloud.com"'
SET @@global.keyring_kms_dump = 1;
SET @@global.keyring_kms_switch = 1;
SET @@global.keyring_kms_verify = 1;
SET @@global.keyring_kms_check = 1;
ERROR 42000: Variable 'keyring_kms_check' can't be set to the value of '1'
SHOW VARIABLES LIKE 'keyring_kms_verify';
Variable_name	Value
keyring_kms_verify	ON
UNINSTALL PLUGIN keyring_kms;
SELECT PLUGIN_NAME, PLUGIN_STATUS, PLUGIN_TYPE, PLUGIN_LIBRARY
FROM INFORMATION_SCHEMA.PLUGINS WHERE PLUGIN_NAME like 'keyring_kms%';
PLUGIN_NAME	PLUGIN_STATUS	PLUGIN_TYPE	PLUGIN_LIBRARY

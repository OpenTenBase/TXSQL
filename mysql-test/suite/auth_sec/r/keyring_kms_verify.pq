call mtr.add_suppression("\\[Warning\\] \\[MY-011197\\] \\[Server\\] Plugin keyring_kms reported*");
INSTALL PLUGIN keyring_kms SONAME 'keyring_kms.so';
SET SESSION debug = '+d,keyring_kms_intruement_wrong_magic';
SHOW VARIABLES LIKE 'keyring_kms_verify';
Variable_name	Value
keyring_kms_verify	ON
SHOW STATUS LIKE 'keyring_kms_error';
Variable_name	Value
keyring_kms_error	
SET GLOBAL keyring_kms_verify=OFF;
SET SESSION debug = '+d,keyring_kms_intruement_wrong_magic';
# restart
SHOW VARIABLES LIKE 'keyring_kms_verify';
Variable_name	Value
keyring_kms_verify	ON
SHOW STATUS LIKE 'keyring_kms_error';
Variable_name	Value
keyring_kms_error	magic mismatch.
# restart:--keyring_kms_verify=OFF
SHOW VARIABLES LIKE 'keyring_kms_verify';
Variable_name	Value
keyring_kms_verify	OFF
SHOW STATUS LIKE 'keyring_kms_error';
Variable_name	Value
keyring_kms_error	
UNINSTALL PLUGIN keyring_kms;

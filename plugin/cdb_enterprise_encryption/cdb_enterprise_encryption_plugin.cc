#include "cdb_enterprise_encryption.h"

static int cdb_enterprise_encryption_init(void *arg MY_ATTRIBUTE((unused))) {
  mysql_mutex_init(0, &dh_params_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(0, &dsa_params_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(0, &rsa_key_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(0, &dh_key_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(0, &dsa_key_lock, MY_MUTEX_INIT_FAST);
  return 0;
}

static int cdb_enterprise_encryption_deinit(void *arg MY_ATTRIBUTE((unused))) {
  mysql_mutex_destroy(&dh_params_lock);
  mysql_mutex_destroy(&dsa_params_lock);
  mysql_mutex_destroy(&rsa_key_lock);
  mysql_mutex_destroy(&dh_key_lock);
  mysql_mutex_destroy(&dsa_key_lock);

  rsa_bits_threshold = 0;
  dh_bits_threshold = 0;
  dsa_bits_threshold = 0;
  return 0;
}

static MYSQL_SYSVAR_ULONG(rsa_bits_threshold, rsa_bits_threshold,
                          PLUGIN_VAR_OPCMDARG,
                          "Maximum RSA key length in bits for"
                          " CREATE_PRIV_KEY(). The minimum and"
                          " maximum values for this variable are"
                          " 1,024 and 16,384.",
                          NULL,  // check
                          NULL,  // update
                          16384, 1024, 16384, 1);

static MYSQL_SYSVAR_ULONG(dh_bits_threshold, dh_bits_threshold,
                          PLUGIN_VAR_OPCMDARG,
                          "Maximum DH key length in bits for"
                          " CREATE_PRIV_KEY(). The minimum and"
                          " maximum values for this variable are"
                          " 1,024 and 10,000.",
                          NULL,  // check
                          NULL,  // update
                          10000, 1024, 10000, 1);

static MYSQL_SYSVAR_ULONG(dsa_bits_threshold, dsa_bits_threshold,
                          PLUGIN_VAR_OPCMDARG,
                          "Maximum DSA key length in bits for"
                          " CREATE_PRIV_KEY(). The minimum and"
                          " maximum values for this variable are"
                          " 1,024 and 10,000.",
                          NULL,  // check
                          NULL,  // update
                          10000, 1024, 10000, 1);

struct SYS_VAR *cdb_enterprise_encryption_sys_vars[] = {
    MYSQL_SYSVAR(rsa_bits_threshold), MYSQL_SYSVAR(dh_bits_threshold),
    MYSQL_SYSVAR(dsa_bits_threshold), NULL};

static struct st_mysql_daemon cdb_enterprise_encryption = {
    MYSQL_DAEMON_INTERFACE_VERSION};

mysql_declare_plugin(cdb_enterprise_encryption){
    MYSQL_DAEMON_PLUGIN,
    &cdb_enterprise_encryption,
    "cdb_enterprise_encryption",
    "txsql team",
    "CDB Enterprise Encryption Plugin",
    PLUGIN_LICENSE_GPL,
    cdb_enterprise_encryption_init,     /* Plugin Init                  */
    NULL,                               /* check uninstall function     */
    cdb_enterprise_encryption_deinit,   /* Plugin Deinit                */
    0x0100,                             /* Plugin version: 1.0          */
    NULL,                               /* status variables             */
    cdb_enterprise_encryption_sys_vars, /* system variables             */
    NULL,                               /* config options               */
    0,                                  /* flags                        */
} mysql_declare_plugin_end;

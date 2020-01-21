/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "my_config.h"

#include <mysql/plugin_keyring.h>
#include <memory>

#include <mysql/components/my_service.h>
#include <mysql/components/services/log_builtins.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_psi_config.h"
#include "mysqld_error.h"
#include "plugin/keyring_kms/buffered_file_io.h"
#include "plugin/keyring_kms/common/keyring.h"
#include "plugin/keyring_kms/keyring_kms.h"
#include <tencentcloud/core/TencentCloud.h>

#ifdef _WIN32
#define MYSQL_DEFAULT_KEYRINGFILE MYSQL_KEYRINGDIR "\\keyring"
#else
#define MYSQL_DEFAULT_KEYRINGFILE MYSQL_KEYRINGDIR "/keyring"
#endif

using keyring_kms::Buffered_file_io;
using keyring_kms::Key;
using keyring_kms::Keys_container;
using keyring_kms::Keys_iterator;
using keyring_kms::Logger;

mysql_rwlock_t LOCK_keyring;
mysql_mutex_t  LOCK_kms;

int check_keyring_file_data(MYSQL_THD thd MY_ATTRIBUTE((unused)),
                            SYS_VAR *var MY_ATTRIBUTE((unused)), void *save,
                            st_mysql_value *value) {
  char buff[FN_REFLEN + 1];
  const char *keyring_filename;
  int len = sizeof(buff);
  std::unique_ptr<IKeys_container> new_keys(new Keys_container(logger.get()));

  (*(const char **)save) = nullptr;
  keyring_filename = value->val_str(value, buff, &len);
  mysql_rwlock_wrlock(&LOCK_keyring);
  if (create_keyring_dir_if_does_not_exist(keyring_filename)) {
    mysql_rwlock_unlock(&LOCK_keyring);
    logger->log(ERROR_LEVEL, ER_KEYRING_FAILED_TO_SET_KEYRING_FILE_DATA);
    return 1;
  }
  try {
    IKeyring_io *keyring_io(new Buffered_file_io(logger.get()));
    if (new_keys->init(keyring_io, keyring_filename)) {
      mysql_rwlock_unlock(&LOCK_keyring);
      return 1;
    }
    *reinterpret_cast<IKeys_container **>(save) = new_keys.get();
    new_keys.release();
    mysql_rwlock_unlock(&LOCK_keyring);
  } catch (...) {
    mysql_rwlock_unlock(&LOCK_keyring);
    return 1;
  }
  return (0);
}

static char default_keyring_kms_file[FN_REFLEN] = "";

static MYSQL_SYSVAR_STR(
  file,                                                        /* name       */
  keyring_kms_file,                                            /* value      */
  PLUGIN_VAR_RQCMDARG,                                         /* flags      */
  "The path to the keyring kms file. Must be specified",       /* comment    */
  check_keyring_file_data,                                     /* check()    */
  update_keyring_file_data,                                    /* update()   */
  default_keyring_kms_file                                     /* default    */
);

static MYSQL_SYSVAR_STR(
  config,                                                      /* name       */
  keyring_kms_config,                                          /* value      */
  PLUGIN_VAR_RQCMDARG,                                         /* flags      */
  "The kms configuration in json format. Must be specified",   /* comment    */
  check_kms_config_data,                                       /* check()    */
  update_kms_config_data,                                      /* update()   */
  nullptr                                                      /* default    */
);

static MYSQL_SYSVAR_BOOL(
  check,                                                       /* name       */
  keyring_kms_check,                                           /* value      */
  PLUGIN_VAR_OPCMDARG,                                         /* flags      */
  "Check if kms configuration works as expected",              /* comment    */
  check_kms_config,                                            /* check      */
  nullptr,                                                     /* update     */
  0                                                            /* default    */
);

static struct SYS_VAR *keyring_kms_system_variables[]= {
  MYSQL_SYSVAR(file),
  MYSQL_SYSVAR(config),
  MYSQL_SYSVAR(check),
  nullptr
};

static SHOW_VAR keyring_kms_status_variables[]= {
  {"keyring_kms_secret_id",
  (char*) &show_kms_secret_id,
  SHOW_FUNC, SHOW_SCOPE_GLOBAL},
  {"keyring_kms_secret_key",
  (char*) &show_kms_secret_key,
  SHOW_FUNC, SHOW_SCOPE_GLOBAL},
  {"keyring_kms_kms_endpoint",
  (char*) &show_kms_end_point,
  SHOW_FUNC, SHOW_SCOPE_GLOBAL},
  {"keyring_kms_error",
  (char*) &show_kms_error,
  SHOW_FUNC, SHOW_SCOPE_GLOBAL},
  {"keyring_kms_region",
  (char*) &show_kms_region,
  SHOW_FUNC, SHOW_SCOPE_GLOBAL},
  {nullptr, nullptr, SHOW_LONG, SHOW_SCOPE_GLOBAL},
};

static SERVICE_TYPE(registry) *reg_srv = nullptr;
SERVICE_TYPE(log_builtins) *log_bi = nullptr;
SERVICE_TYPE(log_builtins_string) *log_bs = nullptr;

static int keyring_kms_init(MYSQL_PLUGIN plugin_info MY_ATTRIBUTE((unused))) {
  if (init_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs)) return true;

  try {
    SSL_library_init();  // always returns 1
#ifndef HAVE_WOLFSSL
    ERR_load_BIO_strings();
#endif
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

#ifdef HAVE_PSI_INTERFACE
    keyring_init_psi_keys();
#endif
    TencentCloud::InitAPI();

    if (init_keyring_locks()) return true;

    if (keyring_kms_file != nullptr && keyring_kms_file[0] == '\0') {
      snprintf(default_keyring_kms_file, sizeof(default_keyring_kms_file),
                  "%s/keyring_kms_file", mysql_real_data_home);
      keyring_kms_file = default_keyring_kms_file;
    }

    logger.reset(new Logger());
    if (create_keyring_dir_if_does_not_exist(keyring_kms_file)) {
      logger->log(ERROR_LEVEL, ER_KEYRING_FAILED_TO_CREATE_KEYRING_DIR);
      return false;
    }
    keys.reset(new Keys_container(logger.get()));
    std::vector<std::string> allowedFileVersionsToInit;
    // this keyring will work with keyring files in the following versions:
    allowedFileVersionsToInit.push_back(keyring_kms::keyring_file_version_2_0);
    allowedFileVersionsToInit.push_back(keyring_kms::keyring_file_version_1_0);
    IKeyring_io *keyring_io =
        new Buffered_file_io(logger.get(), &allowedFileVersionsToInit);
    if (keys->init(keyring_io, keyring_kms_file)) {
      is_keys_container_initialized = false;
      logger->log(ERROR_LEVEL, ER_KEYRING_FILE_INIT_FAILED);
      return false;
    }
    is_keys_container_initialized = true;

    load_kms_config_data();

    return false;
  } catch (...) {
    if (logger != nullptr)
      logger->log(ERROR_LEVEL, ER_KEYRING_INTERNAL_EXCEPTION_FAILED_FILE_INIT);
    deinit_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs);
    return true;
  }
}

static int keyring_kms_deinit(void *arg MY_ATTRIBUTE((unused))) {
// not taking a lock here as the calls to keyring_deinit are serialized by
// the plugin framework
#ifndef HAVE_WOLFSSL
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  ERR_remove_thread_state(0);
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
#endif
  ERR_free_strings();
  EVP_cleanup();
  CRYPTO_cleanup_all_ex_data();
  keys.reset();
  logger.reset();
  keyring_file_data.reset();
  mysql_rwlock_destroy(&LOCK_keyring);
  mysql_mutex_destroy(&LOCK_kms);

  deinit_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs);
  TencentCloud::ShutdownAPI();
  return 0;
}

static bool mysql_key_fetch(const char *key_id, char **key_type,
                            const char *user_id, void **key, size_t *key_len) {
  return kms_key_fetch(key_id, key_type, user_id, key, key_len);
}

static bool mysql_key_store(const char *key_id, const char *key_type,
                            const char *user_id, const void *key,
                            size_t key_len) {
  return kms_key_store(key_id, key_type, user_id, key, key_len);
}

static bool mysql_key_remove(const char *key_id, const char *user_id) {
  return kms_key_remove(key_id, user_id);
}

static bool mysql_key_generate(const char *key_id, const char *key_type,
                               const char *user_id, size_t key_len) {
  return kms_key_generate(key_id, key_type, user_id, key_len);
}

static void mysql_key_iterator_init(void **key_iterator) {
  *key_iterator = new Keys_iterator(logger.get());
  mysql_key_iterator_init<keyring_kms::Key>(
      static_cast<Keys_iterator *>(*key_iterator), "keyring_kms_file");
}

static void mysql_key_iterator_deinit(void *key_iterator) {
  mysql_key_iterator_deinit<keyring_kms::Key>(
      static_cast<Keys_iterator *>(key_iterator), "keyring_kms_file");
  delete static_cast<Keys_iterator *>(key_iterator);
}

static bool mysql_key_iterator_get_key(void *key_iterator, char *key_id,
                                       char *user_id) {
  return mysql_key_iterator_get_key<keyring_kms::Key>(
      static_cast<Keys_iterator *>(key_iterator), key_id, user_id,
      "keyring_kms_file");
}

/* Plugin type-specific descriptor */
static struct st_mysql_keyring keyring_kms_descriptor = {
    MYSQL_KEYRING_INTERFACE_VERSION,
    mysql_key_store,
    mysql_key_fetch,
    mysql_key_remove,
    mysql_key_generate,
    mysql_key_iterator_init,
    mysql_key_iterator_deinit,
    mysql_key_iterator_get_key};

mysql_declare_plugin(keyring_kms){
    MYSQL_KEYRING_PLUGIN,          /*   type                            */
    &keyring_kms_descriptor,       /*   descriptor                      */
    "keyring_kms",                 /*   name                            */
    "Tencent Corporation",         /*   author                          */
    "store/fetch authentication data to/from Tencent KMS",/*description */
    PLUGIN_LICENSE_GPL,
    keyring_kms_init,              /*   init function (when loaded)     */
    nullptr,                       /*   check uninstall function        */
    keyring_kms_deinit,            /*   deinit function (when unloaded) */
    0x0100,                        /*   version                         */
    keyring_kms_status_variables,  /*   status variables                */
    keyring_kms_system_variables,  /*   system variables                */
    nullptr,
    PLUGIN_OPT_ALLOW_EARLY,
} mysql_declare_plugin_end;

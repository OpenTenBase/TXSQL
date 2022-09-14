/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0
*/
#include <string.h>
#include <string>
#include "firewall_cdb.h"
#include "firewall_table_service.h"
#include "mysql/components/services/udf_registration.h"

extern "C" {

/* For load_cdb_firewall_userhost_mode */

bool load_cdb_firewall_userhost_mode_init(UDF_INIT *, UDF_ARGS *,
                                          char *message) {
  if (!check_access()) {
    strcpy(message, "This function requires SUPER_ACL");
    return 1;
  }
  return 0;
}

char *load_cdb_firewall_userhost_mode(UDF_INIT *, UDF_ARGS *, char *,
                                      unsigned long *length,
                                      unsigned char *is_null, unsigned char *) {
  cdb_firewall_manager.load_usermode_from_table();
  const char *message = "load cdb firewall userhost mode";
  *length = static_cast<unsigned long>(strlen(message));
  *is_null = 0;
  return const_cast<char *>(message);
}

void load_cdb_firewall_userhost_mode_deinit(UDF_INIT *) {}

/*--------------------------------------------------*/

/* For sp_set_cdb_firewall_mode */

bool set_cdb_firewall_userhost_mode_init(UDF_INIT *, UDF_ARGS *args,
                                         char *message) {
  if (!check_access()) {
    strcpy(message, "This function requires SUPER_ACL");
    return 1;
  }

  if (args->arg_count != 2 || args->arg_type[0] != STRING_RESULT ||
      args->arg_type[1] != STRING_RESULT) {
    strcpy(message, "Wrong arguments to set_cdb_firewall_userhost_mode");
    return 1;
  }
  return 0;
}
char *set_cdb_firewall_userhost_mode(UDF_INIT *, UDF_ARGS *args, char *,
                                     unsigned long *length,
                                     unsigned char *is_null, unsigned char *) {
  std::string userhost((char *)args->args[0]);
  std::string mode((char *)args->args[1]);
  cdb_firewall_manager.userhost_mode_cache->set_mode_in_cache(userhost, mode);
  const char *message = "set cdb firewall userhost mode";
  *length = static_cast<unsigned long>(strlen(message));
  *is_null = 0;
  return const_cast<char *>(message);
}

void set_cdb_firewall_userhost_mode_deinit(UDF_INIT *) {}

/*--------------------------------------------------*/

/* For load_cdb_firewall_whitelist */

bool load_cdb_firewall_whitelist_init(UDF_INIT *, UDF_ARGS *, char *message) {
  if (!check_access()) {
    strcpy(message, "This function requires SUPER_ACL");
    return 1;
  }
  return 0;
}

char *load_cdb_firewall_whitelist(UDF_INIT *, UDF_ARGS *, char *,
                                  unsigned long *length, unsigned char *is_null,
                                  unsigned char *) {
  cdb_firewall_manager.load_whitelist_from_table();
  const char *message = "load cdb firewall whitelist";
  *length = static_cast<unsigned long>(strlen(message));
  *is_null = 0;
  return const_cast<char *>(message);
}

void load_cdb_firewall_whitelist_deinit(UDF_INIT *) {}

/*--------------------------------------------------*/

/* For flush_cdb_firewall_whitelist */

bool flush_cdb_firewall_whitelist_init(UDF_INIT *, UDF_ARGS *, char *message) {
  if (!check_access()) {
    strcpy(message, "This function requires SUPER_ACL");
    return 1;
  }
  return 0;
}

char *flush_cdb_firewall_whitelist(UDF_INIT *, UDF_ARGS *, char *,
                                   unsigned long *length,
                                   unsigned char *is_null, unsigned char *) {
  cdb_firewall_manager.flush_whitelist();
  const char *message = "flush cdb firewall whitelist";
  *length = static_cast<unsigned long>(strlen(message));
  *is_null = 0;
  return const_cast<char *>(message);
}

void flush_cdb_firewall_whitelist_deinit(UDF_INIT *) {}

/*--------------------------------------------------*/

/* For flush_cdb_firewall_whitelist */

bool reload_cdb_firewall_user_rules_init(UDF_INIT *, UDF_ARGS *args,
                                         char *message) {
  if (!check_access()) {
    strcpy(message, "This function requires SUPER_ACL");
    return 1;
  }
  if (args->arg_count != 1 || args->arg_type[0] != STRING_RESULT) {
    strcpy(message, "Wrong arguments to reload_cdb_firewall_user_rules");
    return 1;
  }
  return 0;
}

char *reload_cdb_firewall_user_rules(UDF_INIT *initid, UDF_ARGS *args, char *,
                                     unsigned long *length,
                                     unsigned char *is_null, unsigned char *) {
  std::string userhost((char *)args->args[0]);
  cdb_firewall_manager.whitelist_cache->delete_rules_for_userhost(userhost);
  std::string ret("reload cdb firewall rules for user ");
  ret += userhost;
  initid->ptr = strdup(ret.c_str());
  *length = static_cast<unsigned long>(strlen(initid->ptr));
  *is_null = 0;
  return const_cast<char *>(initid->ptr);
}

void reload_cdb_firewall_user_rules_deinit(UDF_INIT *initid) {
  if (initid->ptr) free(initid->ptr);
}

/*--------------------------------------------------*/

/* For cdb_firewall_flush_status */

bool cdb_firewall_flush_status_init(UDF_INIT *, UDF_ARGS *, char *message) {
  if (!check_access()) {
    strcpy(message, "This function requires SUPER_ACL");
    return 1;
  }
  return 0;
}

char *cdb_firewall_flush_status(UDF_INIT *, UDF_ARGS *, char *,
                                unsigned long *length, unsigned char *is_null,
                                unsigned char *) {
  cdb_firewall_manager.reset_counters();
  const char *message = "flush cdb firewall whitelist";
  *length = static_cast<unsigned long>(strlen(message));
  *is_null = 0;
  return const_cast<char *>(message);
}

void cdb_firewall_flush_status_deinit(UDF_INIT *) {}

/*--------------------------------------------------*/

/* For normalize_statement */

bool cdb_normalize_statement_init(UDF_INIT *, UDF_ARGS *args, char *message) {
  if (args->arg_count != 1 || args->arg_type[0] != STRING_RESULT) {
    strcpy(message, "Wrong arguments to reload_cdb_firewall_user_rules");
    return 1;
  }
  return 0;
}

char *cdb_normalize_statement(UDF_INIT *initid, UDF_ARGS *args, char *,
                              unsigned long *length, unsigned char *is_null,
                              unsigned char *) {
  std::string query((char *)args->args[0]);
  std::string digest;
  std::string *ret;
  if (get_query_normalized(query, digest))
    ret = new std::string("Parse query failed");
  else
    ret = &digest;
  initid->ptr = strdup(ret->c_str());
  *length = static_cast<unsigned long>(strlen(initid->ptr));
  *is_null = 0;
  return const_cast<char *>(initid->ptr);
}

void cdb_normalize_statement_deinit(UDF_INIT *initid) {
  if (initid->ptr) free(initid->ptr);
}
}

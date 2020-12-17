/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0 
*/
#include <mysql/components/my_service.h>
#include <mysql/components/services/log_builtins.h>

#include "mysql/plugin.h"
#include "mysql/plugin_firewall.h"
#include "my_compiler.h"
#include "firewall_cdb.h"
#include "sql/sql_lex.h"
#include "sql/sql_plugin_var.h"
#include "mysql/psi/mysql_rwlock.h"
#include "i_s.h"
#include "sql/auth/auth_acls.h"

/**
  We handle recovery in a kind of lazy manner. If the server was shut down
  with this plugin installed, we should ideally load the rules into memory
  right on the spot in CDB_Firewall_Manager::init() as the server comes to. 
  But this plugin is relying on transactions to do that and unfortunately the
  transaction log is not set up at this point. (It's set up about 200 lines
  further down in init_server_components() at the time of writing.) That's why
  we simply set this flag in firewall_cdb_plugin_init() and reload in
  CDB_Firewall_Manager::check_sql() if it is set.
*/

extern bool cdb_fire_wall_enabled;
static mysql_rwlock_t LOCK_table;

static SERVICE_TYPE(registry) *reg_srv = nullptr;
SERVICE_TYPE(log_builtins) *log_bi = nullptr;
SERVICE_TYPE(log_builtins_string) *log_bs = nullptr;

#ifdef HAVE_PSI_INTERFACE
static void init_firewall_cdb_psi_keys() {
  PSI_rwlock_info all_firewall_cdb_rwlocks[] = {{&cdb_firewall_manager.whitelist_cache->key_rwlock_LOCK_whitelist_cache,
                                                 "LOCK_plugin_firewallcdb_whitelist",
                                                 0, 0, PSI_DOCUMENT_ME},
                                                {&cdb_firewall_manager.userhost_mode_cache->key_rwlock_LOCK_usermode_cache,
                                                 "LOCK_plugin_firewallcdb_whitelist",
                                                 0, 0, PSI_DOCUMENT_ME}};

  const char *category = "firewall_cdb";
  int count;

  count = static_cast<int>(array_elements(all_firewall_cdb_rwlocks));
  mysql_rwlock_register(category, all_firewall_cdb_rwlocks, count);
}
#endif

/** 
  Check whether current user has SUPER_ACL
  @retval
    true: has SUPER_ACL
    false: hasn't SUPER_ACL
*/
bool check_access() {
  THD *thd = current_thd;
  return thd->security_context()->check_access(SUPER_ACL);
}

CDB_Firewall_Manager cdb_firewall_manager;

/** check if sql can be executed */
int CDB_Firewall_Manager::check_sql(THD *thd MY_ATTRIBUTE((unused))) {
  /* Check if status overflow */
  DBUG_ASSERT(ULLONG_MAX/2 > access_granted);
  DBUG_ASSERT(ULLONG_MAX/2 > access_denied);
  DBUG_ASSERT(ULLONG_MAX/2 > access_suspicious);
  DBUG_ASSERT(ULLONG_MAX/2 > cached_entries);

  /* firewall not enable or SUPER_ACL */
  if (cdb_firewall_manager.firewall_mode == false || check_access())
    return 0;

  if (needs_initial_load) {
    load_whitelist_from_table();
    load_usermode_from_table();
    if (userhost_mode_cache->init_failed() || whitelist_cache->init_failed())
      return 0;
    needs_initial_load = false;
  }

  /* There is no statement, skip firewall */
  if (thd->query().length == 0)
    return 0;

  std::string userhost = get_userhost(thd);
  Userhost_Mode_Cache::Userhost_Mode mode
      = userhost_mode_cache->get_userhost_mode(userhost);

  /* Firewall not working, do nothing */
  if (mode == Userhost_Mode_Cache::OFF
      || mode == Userhost_Mode_Cache::NONE)
    return 0;
  
  /* Recording mode,  */
  if (mode == Userhost_Mode_Cache::RECORDING) {
    /*
      In this function, we also check whether query in whitelist.
      But if query not in whitelist and should be inserted into whitelist,
      we can use the intermediate results: digest(normalized_query).
      If we call whitelist_cache->record_sql after whitelist_cache->query_in_whitelist,
      digest(normalized_query) may be caculated twice.
    */
    if (whitelist_cache->record_sql(thd, userhost))
      cached_entries++;
    return 0;
  }

  bool query_in_whitelist= whitelist_cache->query_in_whitelist(thd, userhost);

  /* If query in whitelist, nothing needs to be done
     in the protect mode and the detect mode. */
  if (query_in_whitelist) {
    access_granted++;
    return 0;
  }

  bool query_rejected = mode == Userhost_Mode_Cache::PROTECT ? true : false;
  /* If query not in the whitelist,
     the protect mode and the detect mode 
     both should record query in the error-log

     Error-log format:
     Query: $query. Userhost: $userhost. Rejected by cdb firewall: true/false.      
  */

  /*
    Here mode is PROTECT or DETECT,
    when mode is PROTECT, only if cdb_firewall_trace is true
    statement will be traced in error log.
  */
  if (cdb_firewall_manager.firewall_trace || mode == Userhost_Mode_Cache::DETECT) {
    std::string message;
    message.append("Query: ");
    message.append(thd->query().str);
    message.append(". Userhost: ");
    message.append(userhost);
    message.append(". Rejected by cdb firewall: ");
    if (query_rejected)
      message.append("true.");
    else
      message.append("false.");

    LogPluginErr(WARNING_LEVEL, ER_REWRITER_QUERY_ERROR_MSG, message.c_str());
  }

  if (query_rejected)
    /* PROTECTING mode */
    access_denied++;
  else
    /* DETECTING mode */
    access_suspicious++;

  return query_rejected;
}

std::string CDB_Firewall_Manager::get_userhost(THD *thd) {
  const char *user = thd->m_main_security_ctx.user().str;
  const char *host = thd->m_main_security_ctx.host().str;
  std::string ret(user);
  ret.append(1, '@');
  ret.append(host);
  return ret;
}

static inline int firewall_cdb_check_sql(THD *thd MY_ATTRIBUTE((unused))) {
  return cdb_firewall_manager.check_sql(thd);
}

void CDB_Firewall_Manager::reset_counters() {
  /*
    To be atomically set these variables,
    should be hold a mutex.

    But we just want to reset these variables,
    don't case whether all vars set to 0 in same time.
    So, there is no mutex.
  */
  access_denied = 0;
  access_granted = 0;
  access_suspicious = 0;
  cached_entries = 0;
}

/**
  Initialize the plugin at server start or plugin installation
  @return 0 on success, 1 on failure
*/
int CDB_Firewall_Manager::init() {
  userhost_mode_cache = new Userhost_Mode_Cache();
  whitelist_cache = new Whitelist_Cache();

  /*
    We don't need to protect this with a mutex here. This function is called
    by a single thread when loading the plugin or starting up the server.
  */
  needs_initial_load = true;

  reset_counters();

  return 0;
}

int CDB_Firewall_Manager::deinit() {
  delete userhost_mode_cache;
  delete whitelist_cache;

  return 0;
}

static int firewall_cdb_plugin_init(void *arg MY_ATTRIBUTE((unused))) {
  /* Initialize error logging service. */
  if (init_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs)) return 1;

  if (cdb_firewall_manager.init())
    return 1;
  
#ifdef HAVE_PSI_INTERFACE
  init_firewall_cdb_psi_keys();
#endif

  cdb_fire_wall_enabled= true;
  return 0;
}

/**
  Terminate the plugin at server shutdown or plugin deinstallation
  @return 0 on success, 1 on failure
*/
static int firewall_cdb_plugin_deinit(void *arg MY_ATTRIBUTE((unused))) {
  mysql_rwlock_destroy(&LOCK_table);
  deinit_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs);
  cdb_fire_wall_enabled= false;
  return cdb_firewall_manager.deinit();
}

/*
  Plugin type-specific descriptor
*/
static struct st_cdb_firewall firewall_cdb_descriptor= {
  CDB_FIREWALL_INTERFACE_VERSION,
  firewall_cdb_check_sql                                /** check if sql can be executed */
};

/*
  Plugin system variables.
*/
static MYSQL_SYSVAR_BOOL(
    mode, /* firewall_cdb_mode*/
    cdb_firewall_manager.firewall_mode,
    PLUGIN_VAR_NOCMDARG,
    "Whether CDB Firewall is enabled or disabled (the default)",
    NULL, NULL, false);

static MYSQL_SYSVAR_BOOL(
    trace, /* firewall_cdb_trace*/
    cdb_firewall_manager.firewall_trace,
    PLUGIN_VAR_NOCMDARG,
  "Whether the CDB Firewall trace is enabled or disabled (the default)."
  "When cdb_firewall_trace is enabled, for PROTECTING mode,"
    "the firewall writes rejected statements to the error log",
    NULL, NULL, false);

static SYS_VAR* firewall_system_variables[] = {
  MYSQL_SYSVAR(mode),
  MYSQL_SYSVAR(trace),
  NULL
};

/*
  Plugin status variables for SHOW STATUS
*/
static SHOW_VAR firewall_system_status[]= {
  { "Firewall_access_denied",
    const_cast<char *>(reinterpret_cast<volatile char *>(&cdb_firewall_manager.access_denied)),
    SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
  { "Firewall_access_granted",
    const_cast<char *>(reinterpret_cast<volatile char *>(&cdb_firewall_manager.access_granted)),
    SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
  { "Firewall_access_suspicious",
    const_cast<char *>(reinterpret_cast<volatile char *>(&cdb_firewall_manager.access_suspicious)),
    SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
  { "Firewall_cached_entries",
    const_cast<char *>(reinterpret_cast<volatile char *>(&cdb_firewall_manager.cached_entries)),
    SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
  { 0, 0, SHOW_UNDEF, SHOW_SCOPE_GLOBAL}
};

/*
  Plugin library descriptor
*/
mysql_declare_plugin(firewall_cdb) {
  MYSQL_FIREWALL_PLUGIN,        /* type                            */
  &firewall_cdb_descriptor,     /* descriptor                      */
  "FIREWALL_CDB",               /* name                            */
  plugin_author,                /* author                          */
  "CDB Firewall",               /* description                     */
  PLUGIN_LICENSE_GPL,
  firewall_cdb_plugin_init,     /* init function (when loaded)     */
  NULL,                         /* check uninstall function        */
  firewall_cdb_plugin_deinit,   /* deinit function (when unloaded) */
  FIREWALL_CDB_VERSION,         /* version                         */
  firewall_system_status,       /* status variables                */
  firewall_system_variables,    /* system variables                */
  NULL,
  0,
},
i_s_cdb_firewall_users,
i_s_cdb_firewall_whitelist
mysql_declare_plugin_end;

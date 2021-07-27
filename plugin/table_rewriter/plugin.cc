#include "plugin/table_rewriter/plugin.h"

#include "my_config.h"

#include <mysql/plugin_audit.h>
#include <mysql/psi/mysql_thread.h>
#include <stddef.h>
#include <algorithm>
#include <atomic>
#include <new>

#include <mysql/components/my_service.h>
#include <mysql/components/services/log_builtins.h>
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysqld_error.h"
#include "plugin/table_rewriter/rewriter.h"
#include "template_utils.h"

using std::string;

static bool need_reload_rules = true;

static const size_t MAX_QUERY_LENGTH_IN_LOG = 100;

static MYSQL_PLUGIN plugin_info;

static mysql_rwlock_t LOCK_table;
static Table_rewriter *table_rewriter;

#define PLUGIN_NAME "Table_rewriter"

static SERVICE_TYPE(registry) *reg_srv = nullptr;
SERVICE_TYPE(log_builtins) *log_bi = nullptr;
SERVICE_TYPE(log_builtins_string) *log_bs = nullptr;

/// Enabled.
static bool sys_var_enabled;
static char *sys_var_add_rule;
static char *sys_var_del_rule;

// add new rule to memory hash table
static void lock_and_add_rule(
    std::string &db, std::string &table, std::string new_table) {
  mysql_rwlock_wrlock(&LOCK_table);
  table_rewriter->add_rule(db, table, new_table);
  mysql_rwlock_unlock(&LOCK_table);
}

// del rule from memory hash table
static void lock_and_del_rule(
    std::string &db, std::string &table) {
  mysql_rwlock_wrlock(&LOCK_table);
  table_rewriter->del_rule(db, table);
  mysql_rwlock_unlock(&LOCK_table);
}

static void update_enabled(MYSQL_THD, SYS_VAR *, void *, const void *value) {
  sys_var_enabled = *static_cast<const bool *>(value);
}

static std::string get_token(char **pos) {
  char *ptr = *pos;
  while (*ptr && *ptr!=',') ++ptr;
  auto str = std::string(*pos, ptr - *pos);
  if (*ptr) *pos = ptr + 1;
  return str;
}

// set table_rewriter_add_rule = "db,table,table_new";
static void add_rule(MYSQL_THD, SYS_VAR *, void *, const void *value) {
  char *pos = *static_cast<char **>(const_cast<void *>(value));
  std::string db = get_token(&pos);
  std::string table = get_token(&pos);
  std::string table_new = get_token(&pos);
  lock_and_add_rule(db, table, table_new);
}

// set table_rewriter_del_rule = "db,table"
static void del_rule(MYSQL_THD, SYS_VAR *, void *, const void *value) {
  char *pos = *static_cast<char **>(const_cast<void *>(value)); 
  std::string db = get_token(&pos);
  std::string table = get_token(&pos);
  lock_and_del_rule(db, table);
}

static MYSQL_SYSVAR_BOOL(enabled,              // Name.
                         sys_var_enabled,      // Variable.
                         PLUGIN_VAR_NOCMDARG,  // Not a command-line argument.
                         "Whether table name should actually be rewritten.",
                         nullptr,         // Check function.
                         update_enabled,  // Update function.
                         1                // Default value.
);
static MYSQL_SYSVAR_STR(add_rule,
                        sys_var_add_rule,
                        PLUGIN_VAR_NOCMDARG ,
                        "Trigger add new rule into memory hashtable.",
                        nullptr,
                        add_rule,
                        ""
);
static MYSQL_SYSVAR_STR(del_rule,
                        sys_var_del_rule,
                        PLUGIN_VAR_NOCMDARG ,
                        "Trigger delete rule into memory hashtable.",
                        nullptr,
                        del_rule,
                        ""
);
SYS_VAR *rewriter_plugin_sys_vars[] = {MYSQL_SYSVAR(enabled), MYSQL_SYSVAR(add_rule), MYSQL_SYSVAR(del_rule), nullptr};

MYSQL_PLUGIN get_table_rewriter_plugin_info() { return plugin_info; }

/// @name Plugin declaration.
///@{

static int rewrite_query_notify(MYSQL_THD thd, mysql_event_class_t event_class,
                                const void *event);
static int rewriter_plugin_init(MYSQL_PLUGIN plugin_ref);
static int rewriter_plugin_deinit(void *);

/* Audit plugin descriptor */
static struct st_mysql_audit rewrite_table_descriptor = {
    MYSQL_AUDIT_INTERFACE_VERSION, /* interface version */
    nullptr,                       /* release_thd()     */
    rewrite_query_notify,          /* event_notify()    */
    {
        0,
        0,
        (unsigned long)MYSQL_AUDIT_PARSE_ALL,
    } /* class mask        */
};

/* Plugin descriptor */
mysql_declare_plugin(table_rewriter){
    MYSQL_AUDIT_PLUGIN,        /* plugin type                   */
    &rewrite_table_descriptor, /* type specific descriptor      */
    PLUGIN_NAME,               /* plugin name                   */
    "tdsql-thawne",            /* author                        */
    "A table name rewrite plugin that"
    " rewrites table name in the "
    " parse tree.",              /* description                   */
    PLUGIN_LICENSE_GPL,          /* license                       */
    rewriter_plugin_init,        /* plugin initializer            */
    nullptr,                     /* plugin check uninstall        */
    rewriter_plugin_deinit,      /* plugin deinitializer          */
    0x0002,                      /* version                       */
    nullptr,                     /* status variables              */
    rewriter_plugin_sys_vars,    /* system variables              */
    nullptr,                     /* reserverd                     */
    PLUGIN_OPT_ALLOW_EARLY,      /* flags                         */
} mysql_declare_plugin_end;

///@}

#ifdef HAVE_PSI_INTERFACE
PSI_rwlock_key key_rwlock_LOCK_table_;

static PSI_rwlock_info all_rwlocks[] = {{&key_rwlock_LOCK_table_,
                                                 "LOCK_plugin_table_rewriter_table_",
                                                 0, 0, PSI_DOCUMENT_ME}};

static void init_table_rewriter_psi_keys() {
  const char *category = "table rewriter";
  int count;

  count = static_cast<int>(array_elements(all_rwlocks));
  mysql_rwlock_register(category, all_rwlocks, count);
}
#endif

static int rewriter_plugin_init(MYSQL_PLUGIN plugin_ref) {
#ifdef HAVE_PSI_INTERFACE
  init_table_rewriter_psi_keys();
#endif
  mysql_rwlock_init(key_rwlock_LOCK_table_, &LOCK_table);
  plugin_info = plugin_ref;

  table_rewriter = new Table_rewriter();
  /*
    We don't need to protect this with a mutex here. This function is called
    by a single thread when loading the plugin or starting up the server.
  */
  need_reload_rules = true;

  // Initialize error logging service.
  if (init_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs)) return 1;

  return 0;
}

static int rewriter_plugin_deinit(void *) {
  plugin_info = nullptr;
  delete table_rewriter;
  mysql_rwlock_destroy(&LOCK_table);
  deinit_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs);
  return 0;
}

/**
  Reloads the rules into the in-memory table. This function assumes that the
  appropriate lock is already taken and doesn't concern itself with locks.
*/
static bool reload(MYSQL_THD thd) {
  longlong errcode = 0;
  try {
    errcode = table_rewriter->reload(thd);
    if (errcode == 0) return false;
  } catch (const std::bad_alloc &) {
    errcode = ER_REWRITER_OOM;
  }
  DBUG_ASSERT(errcode != 0);
  LogPluginErr(ERROR_LEVEL, errcode);
  return errcode != 0;
}

static bool lock_and_reload(MYSQL_THD thd) {
  bool err = false;
  mysql_rwlock_wrlock(&LOCK_table);
  if (need_reload_rules) {
    err = reload(thd);
    need_reload_rules = false;
  }
  mysql_rwlock_unlock(&LOCK_table);

  return err;
}

bool reload_rules_table() {
  MYSQL_THD thd = mysql_parser_current_session();
  need_reload_rules = true;
  return lock_and_reload(thd);
}

/**
  Entry point to the plugin. The server calls this function after each parsed
  query when the plugin is active. The function extracts the digest of the
  query. If the digest matches an existing rewrite rule, it is executed.
*/
static int rewrite_query_notify(
    MYSQL_THD thd, mysql_event_class_t event_class MY_ATTRIBUTE((unused)),
    const void *event) {
  DBUG_ASSERT(event_class == MYSQL_AUDIT_PARSE_CLASS);

  const struct mysql_event_parse *event_parse =
      static_cast<const struct mysql_event_parse *>(event);

  if (event_parse->event_subclass != MYSQL_AUDIT_PARSE_POSTPARSE ||
      !sys_var_enabled)
    return 0;

  if (need_reload_rules) lock_and_reload(thd);

  /* hold lock before rewrite table name */
  mysql_rwlock_rdlock(&LOCK_table);

  try {
    table_rewriter->rewrite_table_name(thd);
  } catch (std::bad_alloc &) {
    LogPluginErr(ERROR_LEVEL, ER_REWRITER_OOM);
  }

  /* release table name */
  mysql_rwlock_unlock(&LOCK_table);

  return 0;
}

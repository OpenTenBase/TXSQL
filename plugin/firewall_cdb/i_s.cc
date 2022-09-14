/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0
*/
#include "i_s.h"
#include "firewall_cdb.h"
#include "mysql/plugin.h"
#include "sql/sql_class.h" /* For THD */
#include "sql/table.h"
#include "user_mode_cache.h"
#include "whitelist_cache.h"

#if !defined __STRICT_ANSI__ && defined __GNUC__ && !defined __clang__
#define STRUCT_FLD(name, value) \
  name:                         \
  value
#else
#define STRUCT_FLD(name, value) value
#endif

/* Don't use a static const variable here, as some C++ compilers (notably
   HPUX aCC: HP ANSI C++ B3910B A.03.65) can't handle it. */
#define END_OF_ST_FIELD_INFO                                           \
  {                                                                    \
    STRUCT_FLD(field_name, nullptr), STRUCT_FLD(field_length, 0),      \
    STRUCT_FLD(field_type, MYSQL_TYPE_NULL), STRUCT_FLD(value, 0),     \
    STRUCT_FLD(field_flags, 0), STRUCT_FLD(old_name, ""),              \
    STRUCT_FLD(open_method, 0)                                         \
  }

static struct st_mysql_information_schema i_s_info = {
    MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION};

static ST_FIELD_INFO cdb_firewall_users_info[] = {
    {STRUCT_FLD(field_name, "userhost"),
     STRUCT_FLD(field_length, USERHOST_LENGTH),
     STRUCT_FLD(field_type, MYSQL_TYPE_STRING), STRUCT_FLD(value, 0),
     STRUCT_FLD(field_flags, 0), STRUCT_FLD(old_name, ""),
     STRUCT_FLD(open_method, 0)},
    {STRUCT_FLD(field_name, "mode"), STRUCT_FLD(field_length, MODE_LENGTH),
     STRUCT_FLD(field_type, MYSQL_TYPE_STRING), STRUCT_FLD(value, 0),
     STRUCT_FLD(field_flags, 0), STRUCT_FLD(old_name, ""),
     STRUCT_FLD(open_method, 0)},
    END_OF_ST_FIELD_INFO};

static ST_FIELD_INFO cdb_firewall_whitelist_info[] = {
    {STRUCT_FLD(field_name, "userhost"),
     STRUCT_FLD(field_length, USERHOST_LENGTH),
     STRUCT_FLD(field_type, MYSQL_TYPE_STRING), STRUCT_FLD(value, 0),
     STRUCT_FLD(field_flags, 0), STRUCT_FLD(old_name, ""),
     STRUCT_FLD(open_method, 0)},
    {STRUCT_FLD(field_name, "digest"), STRUCT_FLD(field_length, DIGEST_LENGTH),
     STRUCT_FLD(field_type, MYSQL_TYPE_STRING), STRUCT_FLD(value, 0),
     STRUCT_FLD(field_flags, 0), STRUCT_FLD(old_name, ""),
     STRUCT_FLD(open_method, 0)},
    END_OF_ST_FIELD_INFO};

/** Fill the dynamic table information_schema.cdb_firewall_users
    @return 0 on success, 1 on failure */
static int i_s_cdb_firewall_users_fill(THD *thd, TABLE_LIST *tables, Item *) {
  return cdb_firewall_manager.userhost_mode_cache->fill_i_s_table(thd, tables);
}

/** Fill the dynamic table information_schema.cdb_firewall_whitelist
    @return 0 on success, 1 on failure */
static int i_s_cdb_firewall_whitelist_fill(THD *thd, TABLE_LIST *tables,
                                           Item *) {
  return cdb_firewall_manager.whitelist_cache->fill_i_s_table(thd, tables);
}

static int cdb_firewall_users_init(void *p) {
  ST_SCHEMA_TABLE *schema;

  DBUG_TRACE;

  schema = (ST_SCHEMA_TABLE *)p;

  schema->fields_info = cdb_firewall_users_info;
  schema->fill_table = i_s_cdb_firewall_users_fill;

  return 0;
}

static int cdb_firewall_whitelist_init(void *p) {
  ST_SCHEMA_TABLE *schema;

  DBUG_TRACE;

  schema = (ST_SCHEMA_TABLE *)p;

  schema->fields_info = cdb_firewall_whitelist_info;
  schema->fill_table = i_s_cdb_firewall_whitelist_fill;

  return 0;
}

/** Unbind a dynamic INFORMATION_SCHEMA table.
    @return 0 on success */
static int i_s_common_deinit(void *p MY_ATTRIBUTE((unused))) { return 0; }

struct st_mysql_plugin i_s_cdb_firewall_users = {
    /* the plugin type (a MYSQL_XXX_PLUGIN value) */
    /* int */
    STRUCT_FLD(type, MYSQL_INFORMATION_SCHEMA_PLUGIN),

    /* pointer to type-specific plugin descriptor */
    /* void* */
    STRUCT_FLD(info, &i_s_info),

    /* plugin name */
    /* const char* */
    STRUCT_FLD(name, "CDB_FIREWALL_USERS"),

    /* plugin author (for SHOW PLUGINS) */
    /* const char* */
    STRUCT_FLD(author, plugin_author),

    /* general descriptive text (for SHOW PLUGINS) */
    /* const char* */
    STRUCT_FLD(descr, "CDB Firewall userhosts and its mode"),

    /* the plugin license (PLUGIN_LICENSE_XXX) */
    /* int */
    STRUCT_FLD(license, PLUGIN_LICENSE_GPL),

    /* the function to invoke when plugin is loaded */
    /* int (*)(void*); */
    STRUCT_FLD(init, cdb_firewall_users_init),

    /* the function to invoke when plugin is un installed */
    /* int (*)(void*); */
    nullptr,

    /* the function to invoke when plugin is unloaded */
    /* int (*)(void*); */
    i_s_common_deinit,

    /* plugin version (for SHOW PLUGINS) */
    /* unsigned int */
    STRUCT_FLD(version, FIREWALL_CDB_VERSION),

    /* SHOW_VAR* */
    nullptr,

    /* SYS_VAR** */
    nullptr,

    /* reserved for dependency checking */
    /* void* */
    nullptr,

    /* Plugin flags */
    /* unsigned long */
    STRUCT_FLD(flags, 0UL),
};

struct st_mysql_plugin i_s_cdb_firewall_whitelist = {
    /* the plugin type (a MYSQL_XXX_PLUGIN value) */
    /* int */
    MYSQL_INFORMATION_SCHEMA_PLUGIN,

    /* pointer to type-specific plugin descriptor */
    /* void* */
    &i_s_info,

    /* plugin name */
    /* const char* */
    "CDB_FIREWALL_WHITELIST",

    /* plugin author (for SHOW PLUGINS) */
    /* const char* */
    plugin_author,

    /* general descriptive text (for SHOW PLUGINS) */
    /* const char* */
    "CDB Firewall whitelist",

    /* the plugin license (PLUGIN_LICENSE_XXX) */
    /* int */
    PLUGIN_LICENSE_GPL,

    /* the function to invoke when plugin is loaded */
    /* int (*)(void*); */
    cdb_firewall_whitelist_init,

    /* the function to invoke when plugin is un installed */
    /* int (*)(void*); */
    nullptr,

    /* the function to invoke when plugin is unloaded */
    /* int (*)(void*); */
    i_s_common_deinit,

    /* plugin version (for SHOW PLUGINS) */
    /* unsigned int */
    FIREWALL_CDB_VERSION,

    /* SHOW_VAR* */
    nullptr,

    /* SYS_VAR** */
    nullptr,

    /* reserved for dependency checking */
    /* void* */
    nullptr,

    /* Plugin flags */
    /* unsigned long */
    0UL,
};

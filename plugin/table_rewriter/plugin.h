#ifndef TABLE_REWRITE_PLUGIN_INCLUDED
#define TABLE_REWRITE_PLUGIN_INCLUDED

#include <mysql/plugin_audit.h>
#include "my_config.h"

bool reload_rules_table();

MYSQL_PLUGIN get_table_rewriter_plugin_info();

#endif

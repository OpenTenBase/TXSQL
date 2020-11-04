#include "sql/sql_cdb_firewall.h"
#include "mysql/plugin_firewall.h"
#include "sql/sql_class.h"  // THD
#include "mysql/plugin.h"

static bool firewall_plugin_check_sql(THD *thd, plugin_ref plugin, void *arg MY_ATTRIBUTE((unused))) {
#ifdef DBUG_OFF
  st_cdb_firewall *data = (st_cdb_firewall *)plugin[0].plugin->info;
#else
  st_cdb_firewall *data = (st_cdb_firewall *)plugin[0]->plugin->info;
#endif
  if (data->check_sql(thd))
    thd->cdb_sql_rejected_by_firewall = true;
  else
    thd->cdb_sql_rejected_by_firewall = false;
  return false;
}

bool cdb_firewall_check_sql(THD *thd) {
  plugin_foreach_func *funcs[] = {firewall_plugin_check_sql ,NULL};
  plugin_foreach(thd, funcs, MYSQL_FIREWALL_PLUGIN, 0);
  return false;
}

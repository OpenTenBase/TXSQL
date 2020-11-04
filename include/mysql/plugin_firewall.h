/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0
*/
#ifndef _PLUGIN_FIREWALL_H_
#define _PLUGIN_FIREWALL_H_

#include "plugin.h"

#define CDB_FIREWALL_INTERFACE_VERSION 0x0001

struct st_cdb_firewall {
  int interface_version; /** version plugin uses */

  int (*check_sql)(MYSQL_THD); /** check if sql can be executed */
};
#endif //_PLUGIN_FIREWALL_H_

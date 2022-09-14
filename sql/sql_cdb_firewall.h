#ifndef _SQL_CDB_FIREWALL_H_
#define _SQL_CDB_FIREWALL_H_

#include "mysql/plugin_audit.h"

bool cdb_firewall_check_sql(THD *thd);

#endif // _SQL_CDB_FIREWALL_H_

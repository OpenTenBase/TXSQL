/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0
*/
#ifndef _I_S_H_
#define _I_S_H_
#include <sys/types.h>

const char plugin_author[] = "Tencent Corporation";
const uint USERHOST_LENGTH = 100;
const uint MODE_LENGTH = 63;
const uint DIGEST_LENGTH = 5000;

extern struct st_mysql_plugin i_s_cdb_firewall_users;
extern struct st_mysql_plugin i_s_cdb_firewall_whitelist;

#endif  //_I_S_H_

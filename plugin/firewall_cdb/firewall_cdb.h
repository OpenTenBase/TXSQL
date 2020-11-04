#ifndef FIREWALL_CDB_INCLUDE
#define FIREWALL_CDB_INCLUDE
/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0 
*/
#include "sql/sql_class.h"
#include "user_mode_cache.h"
#include "whitelist_cache.h"

#define FIREWALL_CDB_VERSION 0x0001

class Userhost_Mode_Cache;
class Whitelist_Cache;

class CDB_Firewall_Manager
{
public:
  bool firewall_mode;
  bool firewall_trace;

  /**
    Number of statement rejected by CDB Firewall.
  */
  std::atomic_ullong access_denied;

  /**
    Number of statement passed by CDB Firewall in PROTECTING mode.
  */
  std::atomic_ullong access_granted;

  /**
    Number of statement recorded in error log in DETECTING mode.
  */
  std::atomic_ullong access_suspicious;

  /**
    Number of statement cached in whitelist by RECORDING mode.
  */
  std::atomic_ullong cached_entries;

  /**
    Reset access_denied, access_granted, access_suspicious, cached_entries.
  */
  void reset_counters();

  Userhost_Mode_Cache *userhost_mode_cache;
  Whitelist_Cache *whitelist_cache;

  /**
    CDB_Firewall_Manager will inited a global var
    cdb_firewall_manager, so constructor do nothing.
    Construct and destroy implements in init() and deinit()
  */
  CDB_Firewall_Manager() {}

  ~CDB_Firewall_Manager() {}

  /**
    Initialize the plugin at server start or plugin installation
    Construct userhost_mode_cache and whitelist_cache.
    @return 0 on success, 1 on failure
  */
  int init();

  /**
    Destory userhost_mode_cache and whitelist_cache
  */
  int deinit();

  int check_sql(THD *thd MY_ATTRIBUTE((unused)));

  /**
    Get userhost from thd in format user@host.
  */
  std::string get_userhost(THD *thd);

  /**
    Load whitelist rule into whitelist_cache.
  */
  void load_whitelist_from_table() {
    whitelist_cache->load_data_from_table();
  }

  /**
    Flush whitelist rule into disk table.
  */
  void flush_whitelist() {
    whitelist_cache->flush_to_disk();
  }

  /**
    Load userhost and mode into userhost_mode_cache.
  */
  void load_usermode_from_table() {
    userhost_mode_cache->load_data_from_table();
  }

  /**
    Whether the firewall should load data from disk table.
    When firewall inited, this var is true, and will be set to
    true after loading data.
  */
  bool needs_initial_load;
};

extern CDB_Firewall_Manager cdb_firewall_manager;

extern bool check_access();

#endif /* FIREWALL_CDB_INCLUDE */

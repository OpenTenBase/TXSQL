/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0 
*/
#include <string>
#include "my_config.h"
#include "sql/sql_class.h"

#ifndef USERHOST_MODE_CACHE
#define USERHOST_MODE_CACHE

class Userhost_Mode_Cache
{
public:
#ifdef HAVE_PSI_INTERFACE
  PSI_rwlock_key key_rwlock_LOCK_usermode_cache;
#endif

  Userhost_Mode_Cache() {
    m_init_failed = false;
    mysql_rwlock_init(key_rwlock_LOCK_usermode_cache, &LOCK_usermode_cache);
  }

  ~Userhost_Mode_Cache() {
    mysql_rwlock_destroy(&LOCK_usermode_cache);
  }

  enum Userhost_Mode {
    OFF,
    PROTECT,
    DETECT,
    RECORDING,
    NONE
  };
  
  /**
    Implementation of read user mode from TABLE firewall_users,
    store mode in m_user_mode_map.
    Called by Userhost_Mode_Cache::load_data_from_table
    The server doesn't handle different sessions in the same thread, 
    so we load the mode into the m_user_mode_map in this function, 
    intended to be run in a new thread. The main thread will do join().

    @param thd The session to be used for loading user mode.
  */
  void do_load_data_from_table(THD *thd);

  void load_data_from_table();

  bool set_mode_in_cache(std::string userhost, std::string str_mode);

  Userhost_Mode get_userhost_mode(std::string &userhost);
  
  bool fill_i_s_table(THD *thd, TABLE_LIST *tables);
  bool init_failed() { return m_init_failed; }

private:
  bool m_init_failed;

  mysql_rwlock_t LOCK_usermode_cache;
  void rdlock() { mysql_rwlock_rdlock(&LOCK_usermode_cache); }
  void wrlock() { mysql_rwlock_wrlock(&LOCK_usermode_cache); }
  void unlock() { mysql_rwlock_unlock(&LOCK_usermode_cache); }

  Userhost_Mode string_to_mode(std::string str_mode) {
    if (str_mode == "OFF")
      return OFF;
    if (str_mode == "PROTECT")
      return PROTECT;
    if (str_mode == "DETECT")
      return DETECT;
    if (str_mode == "RECORDING")
      return RECORDING;
    return NONE;
  }

  std::string *mode_to_string(Userhost_Mode mode) {
    static std::string mode_str[] = {"OFF", "PROTECT", "DETECT", "RECORDING", "NONE"};
    return &mode_str[(uint)mode];
  }

  typedef malloc_unordered_map<std::string, Userhost_Mode> Userhost_Mode_Map;
  typedef malloc_unordered_map<std::string, Userhost_Mode>::iterator Userhost_Mode_Map_Iterator;
  Userhost_Mode_Map m_user_mode_map{PSI_INSTRUMENT_ME};
};
#endif

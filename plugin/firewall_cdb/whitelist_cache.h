/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0 
*/
#include <string>
#include "map_helpers.h"
#include "sql/sql_class.h"

#ifndef WHITELIST_CACHE
#define WHITELIST_CACHE

extern bool get_query_normalized(std::string &query, std::string &ret);

class Whitelist_Cache {
private:
  typedef std::string Userhost;
  typedef std::string Digest;
  typedef std::string Digest_Hash;
 
  mysql_rwlock_t LOCK_whitelist_cache;
  void rdlock() { mysql_rwlock_rdlock(&LOCK_whitelist_cache); }
  void wrlock() { mysql_rwlock_wrlock(&LOCK_whitelist_cache); }
  void unlock() { mysql_rwlock_unlock(&LOCK_whitelist_cache); }

  bool m_init_failed;
public:
  bool init_failed() { return m_init_failed; }
#ifdef HAVE_PSI_INTERFACE
  PSI_rwlock_key key_rwlock_LOCK_whitelist_cache;
#endif

  Whitelist_Cache() {
    m_init_failed = false;
    mysql_rwlock_init(key_rwlock_LOCK_whitelist_cache, &LOCK_whitelist_cache);
  }

  ~Whitelist_Cache() {
    mysql_rwlock_destroy(&LOCK_whitelist_cache);
  }

  /**
    Implementation of read whitelist rule from TABLE firewall_whitelist,
    store whitelist in m_whitelist_map.
    Called by Whitelist_Cache::load_data_from_table
    The server doesn't handle different sessions in the same thread, 
    so we load the whitelist into the m_whitelist_map in this function, 
    intended to be run in a new thread. The main thread will do join().

    @param thd The session to be used for loading rules.
  */
  void do_load_data_from_table(THD *thd);

  /**
    Load whitelist from disk table.
    Create a new thread and call Whitelist_Cache::do_load_data_from_table
  */
  void load_data_from_table();

  /**
    @return
      @retval true:  query digest in whitelist
      @retval false: query digest not in whitelist
  */
  bool query_in_whitelist(THD *thd, Userhost &userhost);

  /**
    Called by CDB_Firewall_Manager::check_sql
    If sql not in the memory whitelist,
    store sql in the memory whitelist and m_tmp_whitelist.
    When flush whitelist into disk (by call user defined function),
    sql in tmp_whitelist will be inserted into mysql.firewall_whitelist

    @param thd The session to be used.
    @param userhost user@host of current thd

    @return
      @retval true: sql first recorded in whitelist
      @retval false: sql already in whitelist
  */
  bool record_sql(THD *thd, Userhost &userhost);

  /**
    Store whitelist in m_tmp_whitelist to disk.
    Create a new thread and call Whitelist_Cache::do_flush_to_disk 
  */
  void flush_to_disk();

  /**
    Implementation of flush whitelist rule to disk TABLE firewall_whitelist
    from m_whitelist_map
    Called by Whitelist_Cache::flush_to_disk
    The server doesn't handle different sessions in the same thread, 
    so we flush the whitelist rules into disk table in this function, 
    intended to be run in a new thread. The main thread will do join().

    @param thd The session to be used for flush rules.
  */
  bool do_flush_to_disk(THD *thd);

  /**
    Fill table information_schema.CDB_FIREWALL_WHITELIST
    by userhost and whitelist stored in memory(m_whitelist_map)

    @param thd The session to be used.
    @param tables List of I_S tables to handle
  */
  bool fill_i_s_table(THD *thd, TABLE_LIST *tables);

  /**
    Delete all whitelist rule for specific userhost

    @param userhost The userhost of the rule to be deleted.
  */
  void delete_rules_for_userhost(Userhost userhost);

private:
  /**
    first, find User_Whitelist_Map in Whitelist_Map by userhost
    second, find digest in User_Whitelist_Map by digest_hash
    finally, check if digest match the query
  */

  typedef malloc_unordered_map<Digest_Hash, Digest> User_Whitelist_Map;

  typedef User_Whitelist_Map::iterator User_Whitelist_Map_Iterator;
  
  typedef malloc_unordered_map<Userhost, User_Whitelist_Map>
      Whitelist_Map;
  
  typedef Whitelist_Map::iterator Whitelist_Map_Iterator;
  
  Whitelist_Map m_whitelist_map{PSI_INSTRUMENT_ME};

  std::vector<std::pair<Userhost, Digest>> m_tmp_whitelist;
  /**
    Insert one rule into memory whitelist.
    New rules in whitelist will be stored to disk after calling flush_to_disk.

    @param userhost user@host of current thd
    @param digest digest of the rule to be inserted
    @param locked true if already hold the rwlock
  */
  bool insert_rule(Userhost userhost, Digest digest, bool locked = false);
};
#endif

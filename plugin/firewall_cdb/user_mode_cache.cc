/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0
*/
#include "user_mode_cache.h"
#include <mysql/service_parser.h>
#include <string>
#include "firewall_table_service.h"
#include "mysqld_error.h"
#include "sql/field.h"
#include "sql/sql_show.h"
#include "sql/table.h"

namespace fts = firewall_table_service;

void Userhost_Mode_Cache::load_data_from_table() {
  fts::load_or_flush_data(fts::Callback_type::USERMODE_LOAD);
}

void Userhost_Mode_Cache::do_load_data_from_table(THD *thd) {
  fts::User_Mode_Cursor user_mode_cursor(thd);
  if (user_mode_cursor.inited() == false) {
    m_init_failed = true;
    return;
  }

  for (; user_mode_cursor.is_finished() == false; user_mode_cursor.read()) {
    std::string userhost, mode;
    user_mode_cursor.copy_and_set(&userhost,
                                  user_mode_cursor.userhost_column());
    user_mode_cursor.copy_and_set(&mode, user_mode_cursor.mode_column());
    set_mode_in_cache(userhost, mode);
  }
}

bool Userhost_Mode_Cache::set_mode_in_cache(std::string userhost,
                                            std::string str_mode) {
  Userhost_Mode_Cache::Userhost_Mode mode = string_to_mode(str_mode);
  if (mode == Userhost_Mode_Cache::NONE) return true;

  wrlock();
  Userhost_Mode_Map_Iterator it;
  it = m_user_mode_map.find(userhost);

  if (it == m_user_mode_map.end())
    m_user_mode_map.emplace(userhost, mode);
  else
    it->second = mode;

  unlock();
  return false;
}

bool Userhost_Mode_Cache::fill_i_s_table(THD *thd, TABLE_LIST *tables) {
  TABLE *table = tables->table;
  int status = 0;

  rdlock();
  for (auto it = m_user_mode_map.begin(); it != m_user_mode_map.end(); it++) {
    table->field[0]->store(it->first.c_str(), it->first.length(),
                           system_charset_info);

    std::string *mode = mode_to_string(it->second);
    table->field[1]->store(mode->c_str(), mode->length(), system_charset_info);

    if (schema_table_store_record(thd, table)) {
      status = 1;
      break;
    }
  }

  unlock();
  return status;
}

Userhost_Mode_Cache::Userhost_Mode Userhost_Mode_Cache::get_userhost_mode(
    std::string &userhost) {
  auto ret = Userhost_Mode_Cache::NONE;
  rdlock();
  Userhost_Mode_Map_Iterator it = m_user_mode_map.find(userhost);
  if (it != m_user_mode_map.end()) ret = it->second;
  unlock();
  return ret;
}

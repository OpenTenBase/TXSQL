/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0
*/
#include "whitelist_cache.h"
#include <mysql/service_parser.h>
#include <string>
#include "firewall_table_service.h"
#include "sha2.h"  //SHA256
#include "sql/field.h"
#include "sql/sql_show.h"
#include "sql/table.h"

namespace fts = firewall_table_service;

bool parse(MYSQL_THD thd, const std::string &query, bool is_prepared = false) {
  MYSQL_LEX_STRING query_str = {const_cast<char *>(query.c_str()),
                                query.length()};
  return mysql_parser_parse(thd, query_str, is_prepared, nullptr, nullptr);
}

std::string get_current_query_normalized(MYSQL_THD thd) {
  MYSQL_LEX_STRING normalized_pattern = mysql_parser_get_normalized_query(thd);
  std::string s;
  s.assign(normalized_pattern.str, normalized_pattern.length);
  return s;
}

struct normalize_callback_args {
  THD *thd;
  std::string *query;
  std::string *digest;
  bool *failed;
};

void *normalize_callback(void *p_args) {
  normalize_callback_args *args =
      pointer_cast<normalize_callback_args *>(p_args);

  *(args->failed) = parse(args->thd, *(args->query));

  *(args->digest) = get_current_query_normalized(args->thd);

  return nullptr;
}

bool get_query_normalized(std::string &query, std::string &digest) {
  MYSQL_THD new_thd = mysql_parser_open_session();
  MYSQL_THD thd = current_thd;

  new_thd->set_db(thd->db());

  bool ret = false;
  normalize_callback_args args = {new_thd, &query, &digest, &ret};

  my_thread_handle handle;

  mysql_parser_start_thread(new_thd, normalize_callback, &args, &handle);

  mysql_parser_join_thread(&handle);

  return ret;
}

std::string get_digest_hash(std::string digest) {
  uchar buf[DIGEST_HASH_SIZE];
  SHA_EVP256(reinterpret_cast<const uchar *>(digest.c_str()), digest.length(),
             buf);
  std::string ret(reinterpret_cast<char *>(buf), sizeof(buf));
  return ret;
}

bool Whitelist_Cache::insert_rule(Userhost userhost, Digest digest,
                                  bool locked) {
  if (!locked) wrlock();
  Whitelist_Map_Iterator whitelist_map_it = m_whitelist_map.find(userhost);
  Digest_Hash digest_hash = get_digest_hash(digest);

  if (whitelist_map_it == m_whitelist_map.end()) {
    User_Whitelist_Map user_whitelist_map{PSI_INSTRUMENT_ME};
    user_whitelist_map.emplace(digest_hash, digest);
    m_whitelist_map.insert(std::make_pair(userhost, user_whitelist_map));
    if (!locked) unlock();
    return 0;
  }

  bool exist =
      check_existence_digest(whitelist_map_it->second, digest_hash, digest);
  if (!exist)
    whitelist_map_it->second.emplace(digest_hash, digest);
  if (!locked) unlock();
  return 0;
}

bool Whitelist_Cache::query_in_whitelist(THD *thd, Userhost &userhost) {
  rdlock();
  Whitelist_Map_Iterator it = m_whitelist_map.find(userhost);
  if (it == m_whitelist_map.end()) {
    unlock();
    return false;
  }

  User_Whitelist_Map &whitelist_map = it->second;

  Digest normalized_query = get_current_query_normalized(thd);
  Digest_Hash digest_hash = get_digest_hash(normalized_query);

  /* Different digest may be has same digest_hash,
     so we should check all digest in whitelist that
     digest_hash equal to the inpuh statement.
  */
  bool exist =
      check_existence_digest(whitelist_map, digest_hash, normalized_query);
  unlock();
  if (exist) return true;
  return false;
}

bool Whitelist_Cache::record_sql(THD *thd, Userhost &userhost) {
  /* 1. find sql digest in whitelist or not */
  wrlock();
  Whitelist_Map_Iterator it = m_whitelist_map.find(userhost);
  Digest normalized_query;
  Digest_Hash digest_hash;
  User_Whitelist_Map_Iterator whitelist_it;

  normalized_query = get_current_query_normalized(thd);
  digest_hash = get_digest_hash(normalized_query);

  if (it == m_whitelist_map.end()) goto query_not_in_whitelist;

  /* digest hash equal but digest not equal */
  if (!check_existence_digest(it->second, digest_hash, normalized_query))
    goto query_not_in_whitelist;

  /* sql digest in whitelist, do nothing */
  unlock();
  return false;

  /* 2. sql digest not in whitelist, insert it into white.
        In order yo store in disk later, userhost and sql diest
        should be insert into m_tmp_whitelist
  */
query_not_in_whitelist:
  /* insert rule will compute digest_hash once again,
     but insert_rule in this place call frequency is low,
     the performance loss it not large
  */
  insert_rule(userhost, normalized_query, true);

  m_tmp_whitelist.emplace_back(userhost, normalized_query);
  unlock();
  return true;
}

void Whitelist_Cache::load_data_from_table() {
  fts::load_or_flush_data(fts::Callback_type::WHITELIST_LOAD);
}

void Whitelist_Cache::do_load_data_from_table(THD *thd) {
  fts::Whitelist_Cursor whitelist_cursor(thd);
  if (whitelist_cursor.inited() == false) {
    m_init_failed = true;
    return;
  }

  for (; whitelist_cursor.is_finished() == false; whitelist_cursor.read()) {
    Userhost userhost;
    Digest digest;
    whitelist_cursor.copy_and_set(&userhost,
                                  whitelist_cursor.userhost_column());
    whitelist_cursor.copy_and_set(&digest, whitelist_cursor.digest_column());
    insert_rule(userhost, digest, false);
  }
}

void Whitelist_Cache::flush_to_disk() {
  fts::load_or_flush_data(fts::Callback_type::WHITELIST_FLUSH);
}

bool Whitelist_Cache::do_flush_to_disk(THD *thd) {
  fts::Whitelist_Cursor whitelist_cursor(thd);

  rdlock();
  for (auto it = m_tmp_whitelist.begin(); it != m_tmp_whitelist.end(); it++) {
    Userhost userhost = it->first;
    Digest digest = it->second;

    fts::Field_idx_values_sequence field_idx_and_values;
    field_idx_and_values.emplace_back(whitelist_cursor.userhost_column(),
                                      userhost);
    field_idx_and_values.emplace_back(whitelist_cursor.digest_column(), digest);

    if (whitelist_cursor.store_field_values_and_flush(field_idx_and_values)) {
      unlock();
      return true;
    }
  }

  m_tmp_whitelist.clear();

  unlock();
  return false;
}

bool Whitelist_Cache::fill_i_s_table(THD *thd, TABLE_LIST *tables) {
  TABLE *table = tables->table;

  rdlock();
  for (auto it_w = m_whitelist_map.begin(); it_w != m_whitelist_map.end();
       it_w++) {
    User_Whitelist_Map user_map = it_w->second;
    Userhost userhost = it_w->first;
    for (auto it_u = user_map.begin(); it_u != user_map.end(); it_u++) {
      Digest digest = it_u->second;

      table->field[0]->store(userhost.c_str(), userhost.length(),
                             system_charset_info);
      table->field[1]->store(digest.c_str(), digest.length(),
                             system_charset_info);

      if (schema_table_store_record(thd, table)) {
        unlock();
        return true;
      }
    }
  }
  unlock();
  return false;
}

void Whitelist_Cache::delete_rules_for_userhost(Userhost userhost) {
  wrlock();
  m_whitelist_map.erase(userhost);
  unlock();
}

bool Whitelist_Cache::check_existence_digest(
    const User_Whitelist_Map &whitelist_map, const Digest_Hash &digest_hash,
    const Digest &digest) {
  auto it_range = whitelist_map.equal_range(digest_hash);
  for (auto it_whitelist = it_range.first; it_whitelist != it_range.second;
       it_whitelist++) {
    if (digest.compare(it_whitelist->second) == 0) return true;
  }
  return false;
}
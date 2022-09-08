#ifndef OPT_STATISTICS_INCLUDED
#define OPT_STATISTICS_INCLUDED

/*
   Copyright (c) 2014, 2022, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <sys/types.h>

#include "my_inttypes.h"  // IWYU pragma: keep
#include "my_sqlcommand.h"
#include "mysql/service_parser.h"
#include "sql_string.h"
#include "rwlock_scoped_lock.h"

#include <map>
#include <string>

struct TABLE;

typedef float rec_per_key_t;
class KEY;
class Parser_state;

/**
  Guesstimate for "records per key" when index statistics is not available.

  @param table         the table
  @param key           the index
  @param used_keyparts the number of key part that should be included in the
                       estimate

  @return estimated records per key value
*/

rec_per_key_t guess_rec_per_key(const TABLE *const table, const KEY *const key,
                                uint used_keyparts);

/**
  sql_statistics_info:
  Info of SQL digest statistics. It indicates if the kind of SQL could be
  optimized.
*/

class sql_statistics_info {
public:
  uchar digest[PARSER_SERVICE_DIGEST_LENGTH];
  String digest_text;
  enum enum_sql_command sql_command;
  time_t first_update_timestamp;
  time_t last_update_timestamp;
  time_t last_access_timestamp;
  ulonglong execute_count;
  ulonglong total_affected_rows;
  ulonglong aver_affected_rows;
  ulonglong last_affected_rows;
  /* Tt is really decided by THD::decide_logging_format */
  bool stmt_binlog_format_if_possible;

  sql_statistics_info() : sql_command(SQLCOM_END),
                          first_update_timestamp(0),
                          last_update_timestamp(0),
                          last_access_timestamp(0),
                          execute_count(0),
                          total_affected_rows(0),
                          aver_affected_rows(0),
                          last_affected_rows(0),
                          stmt_binlog_format_if_possible(true) { digest[0] = '\0'; }
};

class digest_compare
{
public:
  bool operator() (const uchar *lhs, const uchar *rhs) const {
    return memcmp(lhs, rhs, PARSER_SERVICE_DIGEST_LENGTH) < 0;
  }
};

typedef std::pair<uchar*, sql_statistics_info*> sql_statistics_info_pair;

class timestamp_compare
{
public:
  bool operator() (const sql_statistics_info_pair &lhs,
                   const sql_statistics_info_pair &rhs) {
    if (lhs.second->last_access_timestamp ==
        rhs.second->last_access_timestamp) {
      return (lhs.second->last_update_timestamp <
              rhs.second->last_update_timestamp);
    } else {
      return (lhs.second->last_access_timestamp <
              rhs.second->last_access_timestamp);
    }
  }
};

/**
  sql_statistics:
  The map of sql_statistics_info.

  Key: digest, the fist element of sql_statistics_info.
  Value: sql_statistics_info.
*/

class sql_statistics {
  mysql_rwlock_t m_lock;
  std::map<uchar*, sql_statistics_info*, digest_compare> m_map;

public:
  sql_statistics();
  ~sql_statistics();
  static bool is_satisfied_sql_command(enum enum_sql_command sql_command);
  static std::string print_digest(const unsigned char *digest);

  /**
    Update info in sql_statistics after excuting a query successfully.

    @param thd  Thread handle.
    @param affected_rows affected_rows in my_ok(),
           the affected_rows of the query.
  */
  void update_info(THD *thd, ulonglong affected_rows);

  /**
    Check if the sql have the possibility to binlog statement format.

    For the reason, before the first mark after decide_logging_format,
    the value alway is true. It means every SQL would try at the first time,
    then they would be marked correctly.

    @param thd  Thread handle.
    @return true if the sql have the possibility to binlog statement format.
  */
  bool stmt_binlog_format_if_possible(THD *thd);

  /**
    Mark if the sql have the possibility to binlog statement format.

    For the reason, before the first mark after decide_logging_format,
    the value alway is true. It means every SQL would try at the first time,
    then they would be marked correctly.

    @param thd  Thread handle.
  */
  void mark_non_stmt_binlog_of_sql_digest(THD *thd);

  /**
    The backgroud thread to clean extra sql_statistics_info that
    exceed cdb_sql_statistics_info_threshold.
  */
  void clear_expired_info_by_bg_thread();

  /**
    Fill handlerton based CDB_SQL_STATISTICS tables.

    @param thd  Thread handle.
    @TABLE table  Information Schema tables to fill
    @return Operation status
  */
  int  fill_i_s(THD* thd, TABLE *table);
};

/**
  Enables digests in the parser state if any feature needs it.

  @param ps This parser state will have digests enabled if any plugin
  needs it.
*/
void enable_digest_if_any_feature_needs_it(Parser_state *ps);
void sql_statistics_init();
void sql_statistics_deinit();
void sql_statistics_update_info(THD *thd, ulonglong affected_rows);
bool sql_statistics_stmt_binlog_format_if_possible(THD *thd);
void sql_statistics_mark_non_stmt_binlog_of_sql_digest(THD *thd);
void sql_statistics_clear_expired_info_by_bg_thread();
int  sql_statistics_fill_i_s(THD* thd, TABLE *table);

#endif /* OPT_STATISTICS_INCLUDED */

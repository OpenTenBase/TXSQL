#include "plugin/table_rewriter/rewriter.h"

#include "my_config.h"

#include <mysql/service_table_rewrite_rules.h>
#include <stddef.h>
#include <memory>
#include <string>

#include "m_string.h"  // Needed because debug_sync.h is not self-sufficient.
#include "my_dbug.h"
#include "mysql/components/services/my_thread_bits.h"
#include "mysqld_error.h"
#include "nullable.h"
#include "sql/debug_sync.h"
#include "template_utils.h"

using Mysql::Nullable;
using table_rewrite_rules_service::Cursor;
using std::string;

extern uint lower_case_table_names;
extern const char* mysql_get_db_name(MYSQL_TABLE_LIST table);
extern const char *mysql_get_table_name(MYSQL_TABLE_LIST );
extern void mysql_reset_table_name(MYSQL_TABLE_LIST , const char *new_name);
extern CHARSET_INFO *files_charset_info;

//typedef int (*parse_table_visit_function)(MYSQL_TABLE_LIST , unsigned char *arg);
int mysql_parser_visit_table(MYSQL_THD thd, parse_table_visit_function processor,
                                unsigned char *arg);
/**
  @file Table_rewriter.cc
  Implementation of the Table_rewriter class's member functions.
*/

Table_rewriter::Table_rewriter() {}

Table_rewriter::~Table_rewriter() {}

void Table_rewriter::load_rewrite_rules(MYSQL_THD session_thd) {
  DBUG_TRACE;
  Cursor c(session_thd);
  m_reload_status = 0;

  if (c.table_is_malformed()) {
    m_reload_status = ER_TABLE_REWRITER_TABLE_MALFORMED_ERROR;
    return;
  }

  while(c.advance()) {
    if (c.had_serious_read_error()) {
      m_reload_status = ER_TABLE_REWRITER_READ_FAILED;
      return;
    }

    add_rule(c.db(), c.table_name(), c.table_name_new());
  }
}

void Table_rewriter::add_rule(std::string db, std::string table, std::string new_table) {
  DBUG_TRACE;
  std::string db_table = db + "." + table;
  if (lower_case_table_names) {
    my_casedn_str(files_charset_info, const_cast<char*>(db_table.c_str()));
    my_casedn_str(files_charset_info, const_cast<char*>(new_table.c_str()));
  }

  m_rules.emplace(db_table, new_table);
}

void Table_rewriter::del_rule(std::string db, std::string table) {
  DBUG_TRACE;
  std::string db_table = db + "." + table;
  if (lower_case_table_names) {
    my_casedn_str(files_charset_info, const_cast<char*>(db_table.c_str()));
  }
  m_rules.erase(db_table);
}

namespace {

struct Reload_callback_args {
  Table_rewriter *me;
  MYSQL_THD session_thd;
};

extern "C" void *reload_callback(void *p_args) {
  Reload_callback_args *args = pointer_cast<Reload_callback_args *>(p_args);
  (args->me->load_rewrite_rules)(args->session_thd);
  return nullptr;
}

}  // namespace

/* Invoke new thread to reload table rewrite rules, caller should have hold the mutex */
longlong Table_rewriter::reload(MYSQL_THD) {
  auto local_thd = mysql_parser_open_session();

  Reload_callback_args args = {this, local_thd};

  m_reload_status = 0;

  my_thread_handle handle;
  mysql_parser_start_thread(local_thd, reload_callback, &args, &handle);

  mysql_parser_join_thread(&handle);

  return m_reload_status;
}

void Table_rewriter::rewrite(MYSQL_TABLE_LIST tl) {
  std::string db = mysql_get_db_name(tl);
  std::string table = mysql_get_table_name(tl);
  auto iter = m_rules.find(db + "." + table);
  if (iter != m_rules.end()) {
    std::string &new_table_name = iter->second;
    mysql_reset_table_name(tl, new_table_name.c_str());
  }
}

static int process_table(MYSQL_TABLE_LIST tl, uchar *arg) {
  Table_rewriter *rewriter = pointer_cast<Table_rewriter*>(arg);
  rewriter->rewrite(tl);
  return 0;
}

void Table_rewriter::rewrite_table_name(MYSQL_THD thd) {
  if (!m_rules.empty()) {
    mysql_parser_visit_table(thd, process_table, pointer_cast<uchar*>(this));
  }
}

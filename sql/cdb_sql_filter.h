#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "mysqld.h"
#include "sql_lex.h"

namespace cdb_sql_filter {
enum Rule_Type {
  SELECT_RULE,
  UPDATE_RULE,
  INSERT_RULE,
  DELETE_RULE,
  REPLACE_RELE,
  END_RULE
};

extern const char *Rule_Type_String[];

struct display_result {
  int id;
  longlong created_time;
  longlong expire_time;
  int concurrence;
  int current_conn;
  longlong rejected_sql_count;
  bool expired;
  const char *type;
  std::string origin_rule_str;
  std::string key_string;
};

class Rule {
 private:
  int id;
  longlong created_time;
  /* Rule expired after expire_time */
  longlong expire_time;
  int concurrence;
  volatile int current_conn;
  longlong rejected_sql_count;
  bool expired;
  char *origin_rule_str;
  std::vector<char *> key_words;
  cdb_sql_filter::Rule_Type type;
  bool deleted;

  mysql_mutex_t m_lock;
  void lock() { mysql_mutex_lock(&m_lock); }
  void unlock() { mysql_mutex_unlock(&m_lock); }

 public:
  Rule(char *rule_string, int new_id, cdb_sql_filter::Rule_Type new_type,
       int new_expire_time, int new_concurrence, char *origin_rule_str);

  cdb_sql_filter::Rule_Type get_type();

  bool get_first_key_and_move_pos(char **key, char **current_pos);

  void handle_key_string(char *key_string);

  bool handle_sql_enter_low(const char *sql, bool &matched);

  bool handle_sql_exit_low();

  inline bool has_concurrence() {
    bool r = false;
    lock();
    r = current_conn > 0 ? true : false;
    unlock();
    return r;
  }

  inline bool is_deleted() { return deleted; }

  ~Rule();

  void display_one_rule(std::vector<cdb_sql_filter::display_result> &ret);

  friend class Cdb_Sql_Filter_Manager;
};

class Cdb_Sql_Filter_Manager {
 private:
  int id_max;
  /* indexed by enum Rule_Type */
  std::map<int, Rule *> filter_rules[cdb_sql_filter::END_RULE];
  const char *errmsg;

  int get_new_id();

  PSI_rwlock_key m_key_lock;
  mysql_rwlock_t m_lock;
  void rdlock() { mysql_rwlock_rdlock(&m_lock); }
  void wrlock() { mysql_rwlock_wrlock(&m_lock); }
  void unlock() { mysql_rwlock_unlock(&m_lock); }

 public:
  /* global var for variables cdb_sql_filter_seperator in sys_vars.cc */
  char *cdb_sql_filter_seperator;

  /* global var for variables cdb_sql_filter_enable in sys_vars.cc */
  bool cdb_sql_filter_enable;

  Cdb_Sql_Filter_Manager() : id_max(1) {}

  ~Cdb_Sql_Filter_Manager() {}

  char get_seperator();

  const char *get_errmsg();

  void init();

  /* destory mysql_rwlock */
  void clean_up();

  /* if is clean_up is ture, assert all connection is 0 */
  bool reset_all_rules(bool is_clean_up);

  /*
    check if this sql reached connection limit of any rule,
    if reached connection limit, reject this sql.
    Else, increase current connection of all match rules.

    @in/out v: matched rule_id

    @return
      false: enter,
      true : reject
  */
  bool handle_sql_enter(const char *sql, enum_sql_command sql_cmd,
                        std::vector<Rule *> &v);

  /*
    decrease current connection of matched rules
    @in v: matched rule_id

    @return
      false: success
      true : error
  */
  bool handle_sql_exit(std::vector<Rule *> &v);

  /*
    remove rules with deleted mark

    @return
      value of removed rules
  */
  int remove_delete_rule_if_possible();

  /*
    delete one rule

    @in pre_check: just check the rule syntax without saving

    @return
      false: success
      true : error
  */
  bool handle_delete_rule(char *rule, bool pre_check = false);

  /*
    add one rule

    @in pre_check: just check the rule syntax without saving

    @return
      false: success
      true : error
  */
  bool handle_add_rule(char *rule, bool pre_check = false);

  /*
    handle raw rule from sys_var cdb_sql_filter

    @in pre_check: just check the rule syntax without saving

    @return
      false: success
      true : error
  */
  bool handle_rule(char *rule, bool pre_check = false);

  /* store all rules in vector ret */
  void get_all_rules_for_display(
      std::vector<cdb_sql_filter::display_result> &ret);
};

}  // namespace cdb_sql_filter

extern cdb_sql_filter::Cdb_Sql_Filter_Manager cdb_sql_filter_manager;

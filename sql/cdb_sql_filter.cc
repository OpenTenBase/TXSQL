#include "cdb_sql_filter.h"

const char *cdb_sql_filter::Rule_Type_String[] = {"SELECT", "UPDATE",  "INSERT",
                                                  "DELETE", "REPLACE", "END"};

using namespace cdb_sql_filter;

Cdb_Sql_Filter_Manager cdb_sql_filter_manager;

static inline char SEPERATOR() {
  return cdb_sql_filter_manager.get_seperator();
}

static char *get_separator(char *str) { return strchr(str, SEPERATOR()); }

/* Make sure the atoi parameter does not overflow integer range  */
static int string_to_int(char *str) {
  char buf[31];
  int ret = atoi(str);
  sprintf(buf, "%d", ret);
  if (strncmp(str, buf, strlen(buf)) != 0) return 0;
  if (ret < -1) return 0;
  return ret;
}

Rule::Rule(char *rule_string, int new_id, Rule_Type new_type,
           int new_expire_time, int new_concurrence, char *new_origin_rule_str)
    : id(new_id),
      concurrence(new_concurrence),
      current_conn(0),
      rejected_sql_count(0),
      expired(false),
      origin_rule_str(new_origin_rule_str),
      type(new_type) {
  rejected_sql_count = 0;
  created_time = time(NULL);
  if (new_expire_time > 0)
    expire_time = (longlong)new_expire_time + created_time;
  else
    expire_time = -1;
  mysql_mutex_init(key_LOCK_Sql_Filter_Rule, &m_lock, MY_MUTEX_INIT_FAST);
  handle_key_string(rule_string);
  origin_rule_str = strdup(new_origin_rule_str);
}

Rule::~Rule() {
  for (uint i = 0; i < key_words.size(); i++) {
    if (key_words[i]) free(key_words[i]);
  }
  free(origin_rule_str);
  mysql_mutex_destroy(&m_lock);
}

Rule_Type Rule::get_type() { return type; }

bool Rule::get_first_key_and_move_pos(char **key, char **current_pos) {
  char *key_end = get_separator(*current_pos);
  if (key_end == 0) {
    if (**current_pos != 0)
      *key = strdup(*current_pos);
    else
      *key = 0;
    return true;
  }

  size_t key_len = key_end - (*current_pos);
  if (key_len == 0) {
    *key = 0;
    (*current_pos)++;
    return false;
  }

  *key = strndup(*current_pos, key_len);
  *current_pos = key_end + 1;
  return false;
}

void Rule::handle_key_string(char *key_string) {
  char *current_pos = key_string;
  char *key = 0;
  while (get_first_key_and_move_pos(&key, &current_pos) == false)
    if (key) key_words.push_back(key);
  if (key) key_words.push_back(key);

  if (key_words.size() == 0) type = END_RULE;
}

bool Rule::handle_sql_enter_low(const char *sql, bool &matched) {
  if (expired) return false;

  /* -1 means no expire time */
  if (expire_time != -1 && time(NULL) >= expire_time) {
    expired = true;
    return false;
  }

  for (uint i = 0; i < key_words.size(); i++) {
    if (strstr(sql, key_words[i]) == NULL) return false;
  }

  if (concurrence == -1) {
    __sync_fetch_and_add(&rejected_sql_count, 1);
    return true;
  }

  lock();
  if (concurrence == current_conn) {
    __sync_fetch_and_add(&rejected_sql_count, 1);
    unlock();
    return true;
  }

  current_conn++;
  unlock();
  matched = true;

  return false;
}

bool Rule::handle_sql_exit_low() {
  lock();
  if (current_conn > 0) current_conn--;
  unlock();
  return false;
}

void Rule::display_one_rule(std::vector<cdb_sql_filter::display_result> &ret) {
  display_result one_rule;
  one_rule.id = id;
  one_rule.created_time = created_time;
  one_rule.expire_time = expire_time;
  one_rule.concurrence = concurrence;
  one_rule.current_conn = __sync_fetch_and_add(&current_conn, 0);
  one_rule.rejected_sql_count = __sync_fetch_and_add(&rejected_sql_count, 0);

  if (expire_time != -1 && time(NULL) >= expire_time) expired = true;

  one_rule.expired = expired;
  one_rule.type = Rule_Type_String[(uint)type];
  one_rule.origin_rule_str = std::string(origin_rule_str);

  std::string &tmp_str = one_rule.key_string;
  if (key_words.size() > 0) tmp_str = key_words[0];
  for (size_t i = 1; i < key_words.size(); i++)
    tmp_str += std::string(1, SEPERATOR()) + std::string(key_words[i]);

  ret.push_back(one_rule);
}

char Cdb_Sql_Filter_Manager::get_seperator() {
  if (cdb_sql_filter_seperator && strlen(cdb_sql_filter_seperator) < 1)
    return ',';
  else
    return cdb_sql_filter_seperator[0];
}

int Cdb_Sql_Filter_Manager::get_new_id() {
  return __sync_fetch_and_add(&id_max, 1);
}

const char *Cdb_Sql_Filter_Manager::get_errmsg() { return errmsg; }

void Cdb_Sql_Filter_Manager::init() { mysql_rwlock_init(m_key_lock, &m_lock); }

void Cdb_Sql_Filter_Manager::clean_up() {
  reset_all_rules(true);
  mysql_rwlock_destroy(&m_lock);
}

bool Cdb_Sql_Filter_Manager::reset_all_rules(bool is_clean_up) {
  std::map<int, Rule *>::iterator iter;
  wrlock();
  for (int i = (int)SELECT_RULE; i < (int)END_RULE; i++) {
    for (iter = filter_rules[i].begin(); iter != filter_rules[i].end();
         iter++) {
      if (iter->second) {
        if (is_clean_up) assert(iter->second->current_conn == 0);
        delete iter->second;
      }
    }
    filter_rules[i].clear();
  }
  unlock();
  id_max = 1;
  return false;
}

bool Cdb_Sql_Filter_Manager::handle_sql_enter(const char *sql,
                                              enum_sql_command sql_cmd,
                                              std::vector<Rule *> &v) {
  Rule_Type rule_type = END_RULE;
  switch (sql_cmd) {
    case SQLCOM_SELECT:
      rule_type = SELECT_RULE;
      break;
    case SQLCOM_INSERT:
    case SQLCOM_INSERT_SELECT:
      rule_type = INSERT_RULE;
      break;
    case SQLCOM_UPDATE:
    case SQLCOM_UPDATE_MULTI:
      rule_type = UPDATE_RULE;
      break;
    case SQLCOM_DELETE:
    case SQLCOM_DELETE_MULTI:
      rule_type = DELETE_RULE;
      break;
    case SQLCOM_REPLACE:
    case SQLCOM_REPLACE_SELECT:
      rule_type = REPLACE_RELE;
      break;
    default:
      break;
  }
  if (rule_type >= END_RULE) return false;

  std::map<int, Rule *> &rules_map = filter_rules[rule_type];
  std::map<int, Rule *>::iterator iter;

  rdlock();
  for (iter = rules_map.begin(); iter != rules_map.end(); iter++) {
    bool matched = false;
    bool ret = iter->second->handle_sql_enter_low(sql, matched);
    if (matched) v.push_back(iter->second);
    if (ret) {
      unlock();
      return true;
    }
  }
  unlock();

  return false;
}

bool Cdb_Sql_Filter_Manager::handle_sql_exit(std::vector<Rule *> &v) {
  std::vector<Rule *>::iterator iter;
  rdlock();
  for (iter = v.begin(); iter != v.end(); iter++)
    if (*iter) (*iter)->handle_sql_exit_low();
  unlock();
  return false;
}

/*
  Detecte rules with delete mark and remove them if possible.
  Without wrlock protection, caller should make sure the wrlcok
  is owned.
*/
int Cdb_Sql_Filter_Manager::remove_delete_rule_if_possible() {
  int deleted_rules = 0;

  Rule *tmp = NULL;
  std::map<int, Rule *>::iterator iter;
  for (int i = (int)SELECT_RULE; i < (int)END_RULE; i++) {
    std::map<int, Rule *> &rules_map = filter_rules[i];
    for (iter = rules_map.begin(); iter != rules_map.end();) {
      tmp = iter->second;
      if (tmp->is_deleted() && !tmp->has_concurrence()) {
        rules_map.erase(iter++);
        delete (tmp);
        continue;
      }
      iter++;
    }
  }

  return deleted_rules;
}

bool Cdb_Sql_Filter_Manager::handle_delete_rule(char *rule, bool pre_check) {
  if (rule == NULL || strlen(rule) == 0) {
    errmsg = "Error syntax, should be reset_all";
    return true;
  }

  for (char *pos = rule; *pos != 0; pos++)
    if (*pos > '9' || *pos < '0') {
      errmsg = "Error syntax, delete id should be an integer";
      return true;
    }

  int rule_id = string_to_int(rule);
  if (rule_id == 0) {
    errmsg = "There is no such rule";
    return true;
  }

  Rule *tmp = 0;
  std::map<int, Rule *>::iterator iter;
  wrlock();
  for (int i = (int)SELECT_RULE; i < (int)END_RULE; i++) {
    iter = filter_rules[i].find(rule_id);
    if (iter != filter_rules[i].end()) {
      // if just checking the rule syntax,
      // we only need find out the role included in filter_rules.
      if (pre_check) {
        return false;
      }
      // Rule to be deleted is found and mark it as deleted. This rule will
      // not be use in future
      tmp = iter->second;
      filter_rules[i].erase(rule_id);
      break;
    }
  }
  remove_delete_rule_if_possible();
  unlock();
  if (tmp) {
    delete tmp;
    return false;
  }

  errmsg = "There is no such rule";
  return true;
}

bool Cdb_Sql_Filter_Manager::handle_add_rule(char *rule, bool pre_check) {
  /* save origin rule str to stroe in Rule instance */
  char *origin_rule_str = rule;

  /* 1. handle sql_command */

  const size_t SQL_COMMAND_SIZE = 6;
  const size_t REPLACE_SIZE = 7;

  if (rule == NULL) {
    errmsg = "Error syntax, rule string is too short";
    return true;
  }

  if (strlen(rule) > CDB_SQL_FILTER_STR_LEN) {
    errmsg = "Error syntax, rule string is too long, longer than 10240";
    return true;
  }

  const char *SELECT = Rule_Type_String[SELECT_RULE];
  const char *UPDATE = Rule_Type_String[UPDATE_RULE];
  const char *INSERT = Rule_Type_String[INSERT_RULE];
  const char *DELETE = Rule_Type_String[DELETE_RULE];
  const char *REPLACE = Rule_Type_String[REPLACE_RELE];
  const char __attribute__((unused)) *sql_command = NULL;
  Rule_Type rule_type = END_RULE;

  /*
    length of rule shorter than SQL_COMMAND_SIZE is ok,
    strncmp will return true
  */
  if (strncmp(SELECT, rule, SQL_COMMAND_SIZE) == 0) {
    sql_command = SELECT;
    rule_type = SELECT_RULE;
  } else if (strncmp(UPDATE, rule, SQL_COMMAND_SIZE) == 0) {
    sql_command = UPDATE;
    rule_type = UPDATE_RULE;
  }

  else if (strncmp(INSERT, rule, SQL_COMMAND_SIZE) == 0) {
    sql_command = INSERT;
    rule_type = INSERT_RULE;
  } else if (strncmp(DELETE, rule, SQL_COMMAND_SIZE) == 0) {
    sql_command = DELETE;
    rule_type = DELETE_RULE;
  } else if (strncmp(REPLACE, rule, REPLACE_SIZE) == 0) {
    sql_command = REPLACE;
    rule_type = REPLACE_RELE;
  } else {
    errmsg =
        "sql_command should be [ SELECT | UPDATE | INSERT | DELETE | REPLACE ]"
        "(must uppercase), and without prefix space";
    return true;
  }

  /* handle separator */
  char *next_separator = get_separator(rule);
  if (next_separator == NULL) {
    errmsg = "Error syntax after sql_command, there is no expire time";
    return true;
  }

  rule = next_separator + 1;

  /* 2. handle expire time */
  int expire_time = string_to_int(rule);
  if (expire_time == 0) {
    errmsg = "expire time should be an non-zero integer or -1";
    return true;
  }

  next_separator = get_separator(rule);
  if (next_separator == NULL) {
    errmsg = "Error syntax after expire time, there is no concurrence";
    return true;
  }
  rule = next_separator + 1;

  /* 3. handle concurrence */
  int concurrence = string_to_int(rule);
  if (concurrence == 0) {
    errmsg = "concurrence should be an non-zero integer or -1";
    return true;
  }

  next_separator = get_separator(rule);
  if (next_separator == NULL) {
    errmsg = "Error syntax after concurrence, there is no keys";
    return true;
  }
  rule = next_separator + 1;

  if (pre_check) {
    return false;
  }

  int new_rule_id = get_new_id();
  Rule *new_rule = new Rule(rule, new_rule_id, rule_type, expire_time,
                            concurrence, origin_rule_str);

  /* new_rule.type will be set to END_RULE when init failed */
  if (new_rule->get_type() == END_RULE) {
    errmsg = "Error syntax, kew words is invalid";
    delete new_rule;
    return true;
  }

  wrlock();
  filter_rules[rule_type][new_rule_id] = new_rule;
  unlock();
  return false;
}

bool Cdb_Sql_Filter_Manager::handle_rule(char *rule, bool pre_check) {
  if (rule == NULL || strlen(rule) == 0) {
    errmsg = "Error syntax";
    return true;
  }
  if (rule[0] == '+')
    return handle_add_rule(rule + 1, pre_check);
  else if (rule[0] == '-')
    return handle_delete_rule(rule + 1, pre_check);
  else if (strcmp(rule, "reset_all") != 0) {
    errmsg = "Error syntax";
    return true;
  } else if (pre_check) {
    return false;
  } else {
    return reset_all_rules(false);
  }
}

void Cdb_Sql_Filter_Manager::get_all_rules_for_display(
    std::vector<display_result> &ret) {
  std::map<int, Rule *>::iterator iter;

  rdlock();
  for (int i = (int)SELECT_RULE; i < (int)END_RULE; i++) {
    for (iter = filter_rules[i].begin(); iter != filter_rules[i].end();
         iter++) {
      iter->second->display_one_rule(ret);
    }
  }
  unlock();
}

#include "opt_outline_builder.h"
#include "opt_outline.h"

#include "sql/opt_explain.h"
#include "sql/opt_explain_json.h"
#include "sql/opt_explain_format.h"
#include "sql/opt_explain_traditional.h"

#include <string.h>

using namespace outline;
Cdb_outline_builder cdb_outline_builder;

#define DIGEST_HASH_LEN 32
#define COMMAND_CNT 7
const char *KEYS[] = {"OUTLINE", "OPT", "INDEX", "END"};
const char *COMMANDS[] = {"SELECT", "INSERT", "UPDATE", "DELETE", "REPLACE", "(", "WITH"};

char* get_explainable_query_str(const char *query) 
{
  int offset = INT_MAX, i = 0;
  char *start = nullptr, *temp = nullptr;
  while(i < COMMAND_CNT) {
    if ((temp = strcasestr(const_cast<char *>(query), COMMANDS[i]))) {
      if (temp - query < offset) {
        start = temp;
        offset = start - query;
      }
    }
    i++;
  }
  return start;
}

bool Cdb_outline_builder::build_outline_info(char *cdb_hints_info)
{
  /*
   cdb_hints_info is the outline info or hints info that user will add.

   ie. outline "sql" set outline_info "OUTLINE: ...";  
       outline "sql" set outline_info "OPT: ...";  
       outline "sql" set outline_info "INDEX: ...";  

   cdb_hints_info point to the added_outline_info value. 
   */
  char *added_outline_info = cdb_hints_info;
  char *start = NULL;
  outline_info_type type;
  if (!added_outline_info || 0 == strlen(added_outline_info)) {
    error_message = "unknown error";
    return true;
  } else if (0 == strncmp(added_outline_info, KEYS[OUTLINE], strlen(KEYS[OUTLINE]))) {
    /* call the add outline info interface */
    start = added_outline_info + strlen(KEYS[OUTLINE]);
    type = OUTLINE;
  } else if (0 == strncmp(added_outline_info, KEYS[OPT], strlen(KEYS[OPT]))) {
    /* call the add optimizer hints info interface */
    start = added_outline_info + strlen(KEYS[OPT]);
    type = OPT;
  } else if (0 == strncmp(added_outline_info, KEYS[INDEX], strlen(KEYS[INDEX]))) {
    /* call the add index hints info interface */
    start = added_outline_info + strlen(KEYS[INDEX]);
    type = INDEX;
  } else {
    error_message = "syntax error";
    return true;
  }
  
  return handle_outline_info(type, start);
}

bool Cdb_outline_builder::handle_outline_info(outline_info_type type, char *added_outline_info)
{
  bool ret = false;
  char *start = NULL;
  if (NULL == (start = strchr(added_outline_info, ':'))) {
    error_message = "syntax error";
    return true;
  } else { /*do nothing.*/ }
  switch (type) {
    case OUTLINE:
      ret = add_outline_info(start+1);
      break;
    case OPT:
      ret = add_optimizer_hints_info(start+1);
      break;
    case INDEX:
      ret = add_index_hints_info(start+1);
      break;
    default:
      ret = true;
      break;
  }
  return ret;
}

bool Cdb_outline_builder::add_outline_info(char *info)
{
  bool err = false;
  THD *thd = mysql_parser_current_session();

  std::string origin, outline, digest_hash; 
  String origin_pattern, outline_pattern;

  MYSQL_LEX_STRING str = thd->lex->outline_origin_sql_str;
  /* calculate the digest pattern of the origin sql. */
  if (mysql_parse_pattern(str, QT_NORMALIZED_FORMAT, origin))
    return true;
  if (get_digest_hash(digest_hash))
    return true;
  
  MYSQL_LEX_STRING outline_str = {const_cast<char *>(info), strlen(info)};
  /* calculate the outline pattern of the origin sql. */
  if (mysql_parse_pattern(outline_str, QT_NORMALIZED_FORMAT, outline))
    return true;

  /* confirm the origin query and outline have same trunk. */
  if (verify_origin_and_outline_pattern(origin, outline)) {
    error_message = "different outline";
    return true;
  }

  Outline_pattern op;
  op.m_origin_query = origin;
  op.m_outline_query = outline;
  Outline_table outline_table(TL_WRITE_DEFAULT);

  if ((err = outline_table.open_outline_table(thd))) {
    error_message = "open outline table failed";

    goto close_table;
  } else if ((err = outline_table.insert_outline_table(digest_hash.c_str(), 
                                                        origin.c_str(), 
                                                        outline.c_str()))) {
    error_message = "insert failed, duplicated record";
  
    goto close_table;
  } else if ((err = construct_param_positions(thd, op))) { 
  
    goto close_table;
  } else if ((err = cdb_outline_loader.load_outline_info(digest_hash, op))) {
    error_message = "load outline info failed";

    goto close_table;
  } else { /*do nothing.*/ }

close_table:
  bool error = false;
  if ((error = outline_table.close_outline_table()))
    error_message = "close outline table failed";
  err = err || error;

  return err;
}

bool Cdb_outline_builder::add_optimizer_hints_info(char *opt_hints_info)
{
  bool err = false;
  THD *thd = mysql_parser_current_session();

  MYSQL_LEX_STRING str = thd->lex->outline_origin_sql_str;
  Opt_ud_optimizer_hints oh;
  std::string digest_hash;
  char pos[32] = "";

  /* split the field of the optimizer hints info, seperator is '#' */
  int ret = sscanf(opt_hints_info, "%[^'#']#%[^'#']", pos, oh.m_ptr);
  if (2 != ret) {
    error_message = "parse optimizer hints info failed";
    return true;
  } else { 
    oh.m_position = atoi(pos);
  }

  if (mysql_parser_parse(thd, str, true, NULL, NULL))
    return true;
  if (get_digest_hash(digest_hash))
    return true;

  Outline_pattern op;
  Outline_table outline(TL_WRITE_DEFAULT);
  if ((err = outline.open_outline_table(thd))) {
    error_message = "open outline table failed";

    goto close_table;
  } else if ((err = outline.add_optimizer_hints_info(digest_hash, 
                                                     oh, 
                                                     op.m_origin_query,
                                                     op.m_outline_query))) {
    error_message = "add optimizer outline table failed";
   
    goto close_table;
  } else if ((err = construct_param_positions(thd, op))) { 

    goto close_table;
  } else if ((err = cdb_outline_loader.load_outline_info(digest_hash, op))) {
    error_message = "load outline info failed";
   
    goto close_table;
  }

close_table: 
  bool error = false;
  if ((error = outline.close_outline_table()))
    error_message = "close outline table failed";
  err = err || error;

  return err;
}

bool Cdb_outline_builder::add_index_hints_info(char *info)
{
  bool err = false;
  THD *thd = mysql_parser_current_session();

  MYSQL_LEX_STRING str = thd->lex->outline_origin_sql_str;
  char pos[32] = "", db[256] = "", table[256] = "", 
       index[256] = "", type[256] = "", clause[256] = "";
  Opt_ud_index_hints ih;
  std::string digest_hash;

  /* split the field of the index hints info, seperator is '#' */
  int ret = sscanf(info, "%[^'#']#%[^'#']#%[^'#']#%[^'#']#%[^'#']#%[^'#']", 
                   pos, db, table, index, type, clause);
  if (6 != ret) {
    error_message = "parse index hints info failed";
    return true;
  } else { 
    ih.set_position(atoi(pos));
    ih.set_db_name(db);
    ih.set_alias_name(table);
    ih.set_index_name(index);
    ih.set_index_type(atoi(type));
    ih.set_clause_type(atoi(clause));
    ih.format();
  }

  if (!ih.check_valid()) {
    /* the index type must be `0`INDEX_HINT_IGNORE, `1`INDEX_HINT_USE, `2`INDEX_HINT_FORCE */
    error_message = "parse index hints info failed, index type not right.";
    return true;
  }

  if (mysql_parser_parse(thd, str, true, NULL, NULL))
    return true;
  if (get_digest_hash(digest_hash))
    return true;

  Outline_pattern op;
  Outline_table outline(TL_WRITE_DEFAULT);
  if ((err = outline.open_outline_table(thd))) {
    error_message = "open outline table failed";
    
    goto close_table;
  } else if ((err = outline.add_index_hints_info(digest_hash, 
                                                 ih, 
                                                 op.m_origin_query,
                                                 op.m_outline_query))) {
    error_message = "add index hints info failed";

    goto close_table;
  } else if ((err = construct_param_positions(thd, op))) { 

    goto close_table;
  } else if ((err = cdb_outline_loader.load_outline_info(digest_hash, op))) {
    error_message = "load outline info failed";

    goto close_table;
  }

close_table: 
  bool error = false;
  if ((error = outline.close_outline_table()))
    error_message = "close outline table failed";
  err = err || error;

  return err;
}

/*
  reset all outline info or one certain outline info.

  ie. outline reset all; 
      outline reset "sql";

  @in all is true when reset all outline info.
  */
bool Cdb_outline_builder::reset_outline_info(bool all)
{
  bool err = false;
  THD *thd = mysql_parser_current_session();
  MYSQL_LEX_STRING str = thd->lex->outline_origin_sql_str;
  std::string digest_hash;

  if (!all) {
    if (mysql_parser_parse(thd, str, true, NULL, NULL))
      return true;
    if (get_digest_hash(digest_hash))
      return true;
  } else { 
    thd->lex->reset_query_tables_list(false);
  }
  
  Outline_table outline(TL_WRITE_DEFAULT);
  if ((err = outline.open_outline_table(thd))) {
    error_message = "open outline table failed";

    goto close_table;
  } else if (!all) {
    if ((err =outline.delete_outline_record(digest_hash))) {
      error_message = "delete outline record failed";

      goto close_table;
    } else if ((err =cdb_outline_loader.remove_outline_info(digest_hash))) {
      error_message = "remove from loader failed";

      goto close_table;
    } else { /*do nothing.*/ }
  } else if ((err =outline.delete_all_records())) {
    error_message = "delete all outline records failed";

    goto close_table;
  } else { 
    cdb_outline_loader.clear_all_outline_info();
  } 

close_table: 
  bool error = false;
  if ((error = outline.close_outline_table()))
    error_message = "close outline table failed";
  err = err || error;

  return err;
}

bool Cdb_outline_builder::load_outline_info_rules(THD *thd)
{
  bool err = false;
  Outline_pattern op;
  std::string digest_hash;

  Outline_table outline(TL_WRITE_DEFAULT);
  cdb_outline_loader.clear_all_outline_info();
  thd->lex->reset_query_tables_list(false);
  if ((err = outline.open_outline_table(thd))) {
    error_message = "open outline table failed";
    goto close_table;
  } else { /*do nothing.*/ }

  while(!outline.get_next_outline_record(digest_hash, op.m_origin_query, op.m_outline_query)) {
    if ((err = construct_param_positions(thd, op)))
      goto close_table;
    else if ((err = cdb_outline_loader.load_outline_info(digest_hash, op))) {
      error_message = "load outline info failed";
      goto close_table;
    } else { /*do nothing*/ }
  }

close_table: 
  bool error = false;
  if ((error = outline.close_outline_table()))
    error_message = "close outline table failed";
  err = err || error;

  return err;
}

bool Cdb_outline_builder::mysql_parse_pattern(const MYSQL_LEX_STRING &str,
                                              enum_query_type query_type,
                                              std::string &pattern)
{
  THD *thd = mysql_parser_current_session();

  String origin_pattern;
  if (mysql_parser_parse(thd, str, true, NULL, NULL)) {
    error_message = "parse query failed";
    return true;
  } else {
    origin_pattern.mem_free();
    thd->lex->unit->print(thd, &origin_pattern, query_type);
    pattern = std::string(origin_pattern.ptr(), origin_pattern.length());

    return false;
  }
}

bool Cdb_outline_builder::get_digest_hash(std::string &str, bool is_prepare)
{
  uchar digest[DIGEST_HASH_LEN] = "";
  THD *thd = mysql_parser_current_session();
  if (thd->lex->is_explain()) {
    char *begin = get_explainable_query_str(thd->query().str);
    if (!begin) goto get_hash;
    int32_t length = thd->query().length - (begin - thd->query().str);
    std::string str2 = std::string(begin, length);
    MYSQL_LEX_STRING query = {const_cast<char *>(str2.c_str()), str2.length()};
    if (mysql_parser_parse(thd, query, is_prepare, NULL, NULL))
      return true;
  }

get_hash:
  compute_digest_hash(&thd->m_digest->m_digest_storage, digest);
  /* convert the unsigned char to char string. */
  const size_t string_size = DIGEST_HASH_LEN * 2;
  char digest_str[string_size + sizeof('\0')];
  for (int i = 0; i < DIGEST_HASH_LEN; ++i)
    snprintf(digest_str + i * 2, string_size, "%02x", digest[i]);
  str.assign(digest_str, string_size);

  return false;
}

bool Cdb_outline_builder::construct_param_positions(THD* thd, Outline_pattern &op)
{
  MYSQL_LEX_STRING ost = {const_cast<char *>(op.m_outline_query.c_str()),
                                             op.m_outline_query.length()};
  if (mysql_parser_parse(thd, ost, true, NULL, NULL)) {
    error_message = "parse query failed";
    return true;
  } else { /*do nothing. */}

  /* calculate the position of the arguments. */
  int num_params = mysql_parser_get_number_params(thd);
  Parameter_array_ptr param_positions(num_params);
  mysql_parser_extract_prepared_params(thd, param_positions.get());
  std::vector<int> positions(param_positions.get(),
                              param_positions.get() + num_params);
  op.m_param_positions.assign(positions.begin(), positions.end());
  op.m_is_hit = false;
  op.m_hit_times = 0;

  return false;
}

bool Cdb_outline_builder::verify_origin_and_outline_pattern(const std::string& origin,
                                                            const std::string& outline)
{
  std::string str1_trunk, str2_trunk;
  enum_query_type type = enum_query_type(QT_NORMALIZED_FORMAT | QT_IGNORE_ALL_HINTS);

  MYSQL_LEX_STRING str1 = {const_cast<char *>(origin.c_str()), origin.length()};
  if (mysql_parse_pattern(str1, type, str1_trunk))
    return true;

  MYSQL_LEX_STRING str2 = {const_cast<char *>(outline.c_str()), outline.length()};
  if (mysql_parse_pattern(str2, type, str2_trunk))
    return true;

  return strcmp(str1_trunk.c_str(), str2_trunk.c_str()) != 0;
}

const char *Cdb_outline_builder::get_error_message()
{
  return error_message;
}

bool banding_outline_to_origin_query(THD *thd, bool is_prepare)
{
  if (thd->m_digest == NULL || thd->m_digest->m_digest_storage.m_byte_count == 0)
    return false;

  if (thd->lex->sql_command != SQLCOM_SELECT &&
      thd->lex->sql_command != SQLCOM_DELETE &&
      thd->lex->sql_command != SQLCOM_DELETE_MULTI &&
      thd->lex->sql_command != SQLCOM_UPDATE &&
      thd->lex->sql_command != SQLCOM_UPDATE_MULTI &&
      thd->lex->sql_command != SQLCOM_INSERT &&
      thd->lex->sql_command != SQLCOM_INSERT_SELECT &&
      thd->lex->sql_command != SQLCOM_REPLACE &&
      thd->lex->sql_command != SQLCOM_REPLACE_SELECT &&
      thd->lex->sql_command != SQLCOM_PREPARE)
    return false;

  bool is_hierarchical = false, is_tree = false;
  bool is_explain = thd->lex->is_explain();
  if (thd->lex->explain_format) {
    is_hierarchical = thd->lex->explain_format->is_hierarchical();
    is_tree = thd->lex->explain_format->is_tree();
  }
  const LEX_CSTRING origin_query = thd->query();
  std::string o_q_str = std::string(origin_query.str, origin_query.length);
  std::string digest_hash;
  if (cdb_outline_builder.get_digest_hash(digest_hash, is_prepare))
    return true;
  outline::Outline_pattern op = cdb_outline_loader.get_pattern_from_outline(digest_hash);

  if (!op.m_is_hit) {
    if (is_explain) {
      char *str = new (thd->mem_root) char[o_q_str.length()+1];
      strcpy(str, o_q_str.c_str());
      if(mysql_parser_parse(thd, {str, o_q_str.length()}, is_prepare, NULL, NULL))
        return true;
    }
    return false;
  }

  outline::Cdb_outline_walker cdb_outline_walker(op);
  cdb_outline_walker.walk_build_literals(thd);
  std::string ostr = cdb_outline_walker.get_outline_query();

  char *query = nullptr;
  MYSQL_LEX_STRING outline_query = {const_cast<char *>(ostr.c_str()), ostr.length()};
  if (is_explain) {
    std::string str;
    if (is_hierarchical)
      str.append("explain format=json ");
    else if (is_tree)
      str.append("explain format=tree ");
    else
      str.append("explain ");
    query = new (thd->mem_root) char[str.length()+ostr.length()+1];
    snprintf(query, str.length()+ostr.length()+1, "%s%s", str.c_str(), ostr.c_str());
    outline_query = {query, str.length()+ostr.length()+1};
  }

  if (mysql_parser_parse(thd, outline_query, is_prepare, NULL, NULL))
    return true;

  return false;
}

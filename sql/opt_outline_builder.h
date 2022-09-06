/* Copyright (c) 2020, tencent and/or its affiliates. All rights reserved.
 */
#ifndef OPT_OUTLINE_BUILDER_INCLUDED
#define OPT_OUTLINE_BUILDER_INCLUDED

#include "mysql/service_parser.h"
#include "opt_outline_loader.h"
#include "sql/sql_class.h"
#include "sql/sql_base.h"
#include "sql/sql_lex.h"
#include "sql/log.h"

namespace outline {

class Cdb_outline_builder
{
public:
  enum outline_info_type {OUTLINE = 0, OPT, INDEX, END};
public:
  /*
    called when set the added_outline_info variables, classify the command 

    @in cdb_hints_info.
    @retval false: success 
    @retval  true : failed 
  */
  bool build_outline_info(char *cdb_hints_info);

  /*
    called by build_outline_info, handle the command

    @retval false: success 
    @retval  true : failed 
  */
  bool handle_outline_info(outline_info_type type, char *added_outline_info);

  /*
    add the outline info when call the command:

    ie. outline "..." set outline_info "OUTLINE: ...";

    @in: outline_info (outline info that user added.)

    @retval false: success 
    @retval  true : failed 
  */
  bool add_outline_info(char *outline_info);

  /*
    add the optimizer hints info when call the command:

    ie. outline "..." set outline_info "OPT: ...";

    @in: opt_hints_info (optimizer hints info that user added.)

    @retval false: success 
    @retval  true : failed 
  */
  bool add_optimizer_hints_info(char *opt_hints_info);

  /*
    add the index hints info when call the command:

    ie. outline "..." set outline_info "INDEX: ...";

    @in: index_hints_info (index hints info that user added.)

    @retval false: success 
    @retval  true : failed 
  */
  bool add_index_hints_info(char *index_hints_info);

  /*
    get the pattern of the str of certain query type.

    @in: str(sql string), query_type(ie. QT_NORMALIZED_FORMAT)
    @out: pattern

    @retval false: success 
    @retval  true : failed 
  */
  bool mysql_parse_pattern(const MYSQL_LEX_STRING &str,
                           enum_query_type query_type,
                           std::string &pattern);

  /*
    construct the param positions of the query.

    ie. "select * from t1 where t1.a = ?"

    calculate the positon of the '?' in the query and store to outline_pattern.

    @out: op

    @retval false: success 
    @retval  true : failed 
  */
  bool construct_param_positions(THD* thd, Outline_pattern &op);

  /*
    confirm that origin query sql and outline banding query sql have the same trunk 
    pattern, avoid convert into different trunk sql. 

    ie. outline "select a from t1" set outline_info "OUTLINE:select b from t1"

    the origin sql is different from the outline one because of the select list.

    so return failed.

    @in: origin, outline

    @retval false: same trunk 
    @retval  true : different trunk

   */
  bool verify_origin_and_outline_pattern(const std::string& origin, 
                                         const std::string& outline);

  bool load_outline_info_rules(THD *thd);

  /*
    get the digest_hash value of the current session thd->lex

    @retval: digest_hash

  */
  bool get_digest_hash(std::string &str, bool is_prepare = false);
  
  /*
    reset the outline info of mysql.outline.

    @in: true if want to reset the mysql.outline.
    @retval false: success 
    @retval  true : failed 
  */
  bool reset_outline_info(bool all);
  
  /* get the const char * error message. */
  const char *get_error_message();

private:
  const char *error_message;

};

} // namespace outline.

extern outline::Cdb_outline_builder cdb_outline_builder;

/* 
  try to convert the origin sql into a outline banding query. 
  
  @retval false: success 
  @retval  true : failed 
 */
bool banding_outline_to_origin_query(THD *thd, bool is_prepare = false);

#endif /* OPT_OUTLINE_BUILDER_INCLUDED */
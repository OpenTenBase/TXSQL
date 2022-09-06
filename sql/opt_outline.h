/* Copyright (c) 2020, tencent and/or its affiliates. All rights reserved.
 */
#ifndef OPT_OUTLINE_INCLUDED
#define OPT_OUTLINE_INCLUDED

#include "sql/table.h"
#include "sql/field.h"
#include "sql/opt_hints.h"
#include "mysql/mysql_lex_string.h"

namespace outline {

/* outline record
 */
class Outline_record {
public:
  int id() const { return m_id; }
  void set_id(int id) { m_id = id; }
  std::string digest() const { return m_digest; }
  void set_digest(const std::string &digest) { m_digest = digest; }
  std::string digest_text() const { return m_digest_text; }
  void set_digest_text(const std::string &digest_text) { m_digest_text = digest_text; }
  std::string outline_text() const { return m_outline_text; }
  void set_outline_text(const std::string &outline_text) { m_outline_text = outline_text; }
private:
  int m_id;
  std::string m_digest;
  std::string m_digest_text;
  std::string m_outline_text;
};

/* store the outline record in mysql.outline
 */
class Outline_table {
public:
  Outline_table(thr_lock_type lock_type);

  /*
  @retval  true    Error to open table.
  @retval  false     Success to open table.
  */
  bool open_outline_table(THD *thd );
  
  /*
  add the outline info into mysql.outline.

  @in : digest_hash digest_text outline_text

  @retval  true    Error to insert table.
  @retval  false     Success to insert table.
  */
  bool insert_outline_table(const char *d, const char *dt, const char *ot);
  
  /*
  delete the record from mysql.outline when call the following command:

  ie. outline reset "sql";
      or
      outline reset all;

  @in : digest_hash

  @retval  true    Error to delete the record from table.
  @retval  false     Success to delete the record from table.
  
  */
  bool delete_outline_record(const std::string& digest);

  /*
  add optimizer hints info to origin sql, generate the outline text.
  if the digest exsits, update the outline.
  
  @in : digest_hash, uoh
  @out: ret_origin_text, ret_outline_text

  @retval  true    Error to add optimizer hints info.
  @retval  false     Success to add optimizer hints info.
  
  */
  bool add_optimizer_hints_info(const std::string& digest, 
                                const Opt_ud_optimizer_hints& uoh,
                                std::string &ret_origin_text,
                                std::string& ret_outline_text);

  /*
  add index hints info to origin sql, generate the outline text.
  if the digest exsits, update the outline.

  @in : digest_hash, uih
  @out: ret_origin_text, ret_outline_text

  @retval  true    Error to add index hints info.
  @retval  false     Success to add index hints info.

  */
  bool add_index_hints_info(const std::string &digest, 
                            const Opt_ud_index_hints& uih,
                            std::string& ret_origin_text,
                            std::string& ret_outline_text);

  /*
  get digest_hash and outline_text of the record from mysql.outline. 
  for traverse the table.

  @in/out: digest_hash origin_text outline_text
  
  @retval  true    Error to get the next outline record.
  @retval  false     Success to get the next outline record.
  
  */
  bool get_next_outline_record(std::string& digest_hash, 
                               std::string& origin_text,
                               std::string& outline_text);

  /*
  reset the mysql.outline when call the following command:

  ie. outline reset all;

  @retval  true    Error to delete all records.
  @retval  false     Success to delete all records.

  */
  bool delete_all_records();

  /*
  close the outline table.

  @retval  true    Error to close table.
  @retval  false     Success to close table.
  */  
  bool close_outline_table();

public:
  TABLE_LIST m_table_list;
  Field *m_id{nullptr};
  Field *m_digest{nullptr};
  Field *m_digest_text{nullptr};
  Field *m_outline_text{nullptr};
};

} // namespace outline.

#endif /* OPT_OUTLINE_INCLUDED */
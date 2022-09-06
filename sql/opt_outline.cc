#include "opt_outline.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"
#include "sql/sql_parse.h"
#include "sql/sql_lex.h"
#include "sql/lock.h"
#include "sql/log.h"

using namespace outline;

Outline_table::Outline_table(thr_lock_type lock_type) 
  : m_table_list("mysql", "outline", lock_type) { }

bool Outline_table::open_outline_table(THD *thd) 
{
  const uint flags = MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY;
  if (open_and_lock_tables(thd, &m_table_list, flags)) {
    sql_print_error("error in open and lock outline table.");
    return true;
  } else if (!m_table_list.table) {
    sql_print_error("outline table is null pointer.");
    return true;
  }

  /* the id field is auto-increment field, so next_number_field must be setted. */
  m_table_list.table->next_number_field = m_table_list.table->found_next_number_field;

  m_id = find_field_in_table_sef(m_table_list.table, "Id");
  m_digest = find_field_in_table_sef(m_table_list.table, "Digest");
  m_digest_text = find_field_in_table_sef(m_table_list.table, "Digest_text");
  m_outline_text = find_field_in_table_sef(m_table_list.table, "Outline_text");

  if (!m_digest || !m_digest_text || !m_outline_text || !m_id) {
    sql_print_error("can not find the field in outline table.");
    return true;
  }

  m_table_list.table->use_all_columns();
  if (m_table_list.lock_descriptor().type == TL_WRITE_DEFAULT)
    bitmap_set_all(m_table_list.table->write_set);
  else
    bitmap_set_all(m_table_list.table->read_set);

  m_table_list.table->file->ha_rnd_init(true);
  return false;
}

bool Outline_table::insert_outline_table(const char *d, const char *dt, const char *ot)
{
  empty_record(m_table_list.table);
  if(m_digest->store(d, strlen(d), system_charset_info)) {
    sql_print_error("failed to store the digest.");
    return true;
  }

  uchar table_key[MAX_KEY_LENGTH] = { 0 };
  KEY *unique_key = m_table_list.table->key_info+1;
  key_copy(table_key, m_table_list.table->record[0], unique_key, unique_key->key_length);
  int rc = m_table_list.table->file->ha_index_read_idx_map(m_table_list.table->record[0],
                                                           1, 
                                                           table_key,
                                                           HA_WHOLE_KEY,
                                                           HA_READ_KEY_EXACT);

  if (!rc) {
    /* when add the outline to same origin sql, duplicated error. */
    return true; 
  } else if (HA_ERR_KEY_NOT_FOUND == rc) {
    if (m_digest->store(d, strlen(d), system_charset_info) ||
        m_digest_text->store(dt, strlen(dt), system_charset_info) ||
        m_outline_text->store(ot, strlen(ot), system_charset_info)) {
      sql_print_error("failed to store the fields.");
      return true;
    }
    rc = m_table_list.table->file->ha_write_row(m_table_list.table->record[0]);
    if (rc) {
      m_table_list.table->file->print_error(rc, MYF(0));
      return true;
    } 
  }

  return false;
}

bool Outline_table::close_outline_table() 
{
  if (m_table_list.table) {
    m_table_list.table->file->ha_release_auto_increment();

    /* reset the next_number_field since the auto-increment key exsits. */
    m_table_list.table->next_number_field = NULL;

    return m_table_list.table->file->ha_index_or_rnd_end();
  }
  else {
    sql_print_error("null poiner error.");
    return true;
  }
}

bool Outline_table::delete_outline_record(const std::string& digest)
{
  TABLE *table = m_table_list.table;
  empty_record(table);
  if (m_digest->store(digest.c_str(), digest.length(), system_charset_info)) {
    sql_print_error("failed to store the digest.");
    return true;
  }

  uchar table_key[MAX_KEY_LENGTH] = { 0 };
  KEY *unique_key = table->key_info+1;
  key_copy(table_key, table->record[0], unique_key, unique_key->key_length);
  int rc = table->file->ha_index_read_idx_map(table->record[0], 
                                              1, 
                                              table_key,
                                              HA_WHOLE_KEY,
                                              HA_READ_KEY_EXACT);
  if(HA_ERR_KEY_NOT_FOUND == rc) {
    /* error when delete the not found record. */
  } else {
    rc = table->file->ha_delete_row(table->record[0]);
  }

  if (rc) {
    table->file->print_error(rc, MYF(0));
    return true;
  } 

  return false;
}

bool Outline_table::delete_all_records()
{
  TABLE *table = m_table_list.table;
  int rc = 0;

  while(!table->file->ha_rnd_next(m_table_list.table->record[0])) {
    rc = table->file->ha_delete_row(table->record[0]);
    if (rc) 
      break;
  }

  if (rc) {
    table->file->print_error(rc, MYF(0));
    return true;
  } 

  return false;
}

bool Outline_table::get_next_outline_record(std::string& digest_hash, 
                                            std::string& origin_text,
                                            std::string& outline_text)
{
  TABLE *table = m_table_list.table;
  int ha_rnd_status = table->file->ha_rnd_next(table->record[0]);

  if (0 != ha_rnd_status) 
    return true;
  
  String d, dt, o;
  digest_hash.assign(m_digest->val_str(&d)->ptr(), m_digest->val_str(&d)->length());
  origin_text.assign(m_digest_text->val_str(&dt)->ptr(), m_digest_text->val_str(&dt)->length());
  outline_text.assign(m_outline_text->val_str(&o)->ptr(), m_outline_text->val_str(&o)->length());
  
  return false;
}

bool Outline_table::add_optimizer_hints_info(const std::string& digest, 
                                             const Opt_ud_optimizer_hints& uoh,
                                             std::string& ret_origin_text,
                                             std::string& ret_outline_text)
{
  TABLE *table = m_table_list.table;
  THD *thd = mysql_parser_current_session();

  empty_record(table);
  if (m_digest->store(digest.c_str(), digest.length(), system_charset_info)) {
    sql_print_error("failed to store the digest.");
    return true;
  }

  String new_outline_text;
  uchar table_key[MAX_KEY_LENGTH] = { 0 };
  KEY *unique_key = table->key_info+1;
  key_copy(table_key, table->record[0], unique_key, unique_key->key_length);
  int rc = table->file->ha_index_read_idx_map(table->record[0], 
                                              1, 
                                              table_key,
                                              HA_WHOLE_KEY,
                                              HA_READ_KEY_EXACT);

  if (!rc) {
    String outline_text;

    /* when found, update the outline info. */
    std::string outline(m_outline_text->val_str(&outline_text)->ptr(),
                        m_outline_text->val_str(&outline_text)->length());

    MYSQL_LEX_STRING query_str = {const_cast<char *>(outline.c_str()), outline.length()};
    int parse_status = 0;
    if ((parse_status = mysql_parser_parse(thd, query_str, true, NULL, NULL))) {
      sql_print_error("parse the sql '%s' error, status '%d'.", query_str.str, parse_status);
      return true;
    }
    store_record(m_table_list.table, record[1]);

    thd->lex->opt_udo_hint = new Opt_ud_optimizer_hints();
    strncpy(thd->lex->opt_udo_hint->m_ptr, uoh.m_ptr, 256);
    thd->lex->opt_udo_hint->m_size = strlen(uoh.m_ptr);
    thd->lex->opt_udo_hint->m_position = uoh.m_position;

    new_outline_text.mem_free();
    thd->lex->unit->print(thd, &new_outline_text, enum_query_type(QT_NORMALIZED_FORMAT));
    m_outline_text->store(new_outline_text.ptr(), new_outline_text.length(), system_charset_info);

    rc = m_table_list.table->file->ha_update_row(m_table_list.table->record[1],
                                                 m_table_list.table->record[0]);
    delete thd->lex->opt_udo_hint;

  } else if (HA_ERR_KEY_NOT_FOUND == rc) {

    /* when not found, add the outline info. */
    String digest_text;
    digest_text.mem_free();
    thd->lex->unit->print(thd, &digest_text, enum_query_type(QT_NORMALIZED_FORMAT | QT_IGNORE_ALL_HINTS));
    m_digest_text->store(digest_text.ptr(), digest_text.length(), system_charset_info);

    thd->lex->opt_udo_hint = new Opt_ud_optimizer_hints();
    strncpy(thd->lex->opt_udo_hint->m_ptr, uoh.m_ptr, 256);
    thd->lex->opt_udo_hint->m_size = strlen(uoh.m_ptr);
    thd->lex->opt_udo_hint->m_position = uoh.m_position;

    new_outline_text.mem_free();
    thd->lex->unit->print(thd, &new_outline_text, enum_query_type(QT_NORMALIZED_FORMAT));
    m_outline_text->store(new_outline_text.ptr(), new_outline_text.length(), system_charset_info);

    rc = m_table_list.table->file->ha_write_row(m_table_list.table->record[0]);
    delete thd->lex->opt_udo_hint;

  } else { /*do nothing.*/ }

  if (rc) {
    m_table_list.table->file->print_error(rc, MYF(0));
    return true;
  } 

  String dt;
  ret_origin_text.assign(m_digest_text->val_str(&dt)->ptr(), m_digest_text->val_str(&dt)->length());
  ret_outline_text = std::string(new_outline_text.ptr(), new_outline_text.length());
  return false;
}

bool Outline_table::add_index_hints_info(const std::string &digest, 
                                         const Opt_ud_index_hints& uih,
                                         std::string& ret_digest_text,
                                         std::string& ret_outline_text)
{
  TABLE *table = m_table_list.table;
  THD *thd = mysql_parser_current_session();

  empty_record(table);
  if (m_digest->store(digest.c_str(), digest.length(), system_charset_info)) {
    sql_print_error("failed to store the digest.");
    return true;
  }

  String new_outline_text;
  uchar table_key[MAX_KEY_LENGTH] = { 0 };
  KEY *unique_key = table->key_info+1;
  key_copy(table_key, table->record[0], unique_key, unique_key->key_length);
  int rc = table->file->ha_index_read_idx_map(table->record[0], 
                                              1, 
                                              table_key,
                                              HA_WHOLE_KEY,
                                              HA_READ_KEY_EXACT);

  if (!rc) {

    String outline_text;
    std::string outline(m_outline_text->val_str(&outline_text)->ptr(),
                        m_outline_text->val_str(&outline_text)->length());

    MYSQL_LEX_STRING query_str = {const_cast<char *>(outline.c_str()), outline.length()};
    int parse_status = 0;
    if ((parse_status = mysql_parser_parse(thd, query_str, true, NULL, NULL))) {
      sql_print_error("parse the sql '%s' error, status '%d'.", query_str.str, parse_status);
      return true;
    }
    store_record(m_table_list.table, record[1]);

    Opt_ud_index_hints opt_udi_hint;
    opt_udi_hint.set_position(uih.get_position());
    opt_udi_hint.set_db_name(uih.get_db_name());
    opt_udi_hint.set_alias_name(uih.get_alias_name());
    opt_udi_hint.set_index_name(uih.get_index_name());
    opt_udi_hint.set_index_type(uih.get_hint_type());
    opt_udi_hint.set_clause_type(uih.get_clause_type());
    if (thd->lex->add_ud_index_hints(opt_udi_hint))
      return true;

    new_outline_text.mem_free();
    thd->lex->unit->print(thd, &new_outline_text, enum_query_type(QT_NORMALIZED_FORMAT));
    m_outline_text->store(new_outline_text.ptr(), new_outline_text.length(), system_charset_info);

    rc = m_table_list.table->file->ha_update_row(m_table_list.table->record[1],
                                                 m_table_list.table->record[0]);

  } else if (HA_ERR_KEY_NOT_FOUND == rc) {

    String digest_text;
    digest_text.mem_free();
    thd->lex->unit->print(thd, &digest_text, enum_query_type(QT_NORMALIZED_FORMAT | QT_IGNORE_ALL_HINTS));
    m_digest_text->store(digest_text.ptr(), digest_text.length(), system_charset_info);

    Opt_ud_index_hints opt_udi_hint;
    opt_udi_hint.set_position(uih.get_position());
    opt_udi_hint.set_db_name(uih.get_db_name());
    opt_udi_hint.set_alias_name(uih.get_alias_name());
    opt_udi_hint.set_index_name(uih.get_index_name());
    opt_udi_hint.set_index_type(uih.get_hint_type());
    opt_udi_hint.set_clause_type(uih.get_clause_type());
    if (thd->lex->add_ud_index_hints(opt_udi_hint))
      return true;

    new_outline_text.mem_free();
    thd->lex->unit->print(thd, &new_outline_text, enum_query_type(QT_NORMALIZED_FORMAT));
    m_outline_text->store(new_outline_text.ptr(), new_outline_text.length(), system_charset_info);

    rc = m_table_list.table->file->ha_write_row(m_table_list.table->record[0]);

  } else { /*do nothing.*/ }

  if (rc) {
    m_table_list.table->file->print_error(rc, MYF(0));
    return true;
  } 

  String dt;
  ret_digest_text.assign(m_digest_text->val_str(&dt)->ptr(), m_digest_text->val_str(&dt)->length());
  ret_outline_text = std::string(new_outline_text.ptr(), new_outline_text.length());
  return false;

}

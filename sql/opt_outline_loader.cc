#include "opt_outline_loader.h"

using namespace outline;
Cdb_outline_loader cdb_outline_loader;

static int walk_build_literals_processor(MYSQL_ITEM item, uchar *arg)
{
  Cdb_outline_walker *visitor = pointer_cast<Cdb_outline_walker *>(arg);
  return visitor->walk_build_literals_iterator(item);
}

bool Cdb_outline_walker::walk_build_literals(THD *thd)
{
  uchar *arg = pointer_cast<uchar *>(this);
  return mysql_parser_visit_tree(thd, walk_build_literals_processor, arg);
}

bool Cdb_outline_walker::walk_build_literals_iterator(MYSQL_ITEM item)
{
  std::string outline_literal;
  MYSQL_LEX_STRING outline_lex = mysql_parser_item_string(item);
  outline_literal.assign(outline_lex.str, outline_lex.length);
  mysql_parser_free_string(outline_lex);
  if (m_param_positions_itr != m_param_positions.end()) 
  {
    m_built_query += m_outline_query.substr(m_previous_slot,
                                            *m_param_positions_itr - m_previous_slot);
    m_built_query += outline_literal;
    m_previous_slot = *m_param_positions_itr++ + sizeof('?');
    return false;
  }
  return true;
}

std::string Cdb_outline_walker::get_outline_query()
{
  m_built_query += m_outline_query.substr(m_previous_slot);
  return m_built_query; 
}

bool Cdb_outline_loader::load_outline_info(const std::string& digest, const Outline_pattern& op) 
{
  mysql_rwlock_wrlock(&m_lock);
  omi itr = m_outline_infos.find(digest);
  if (itr == m_outline_infos.end())
    m_outline_infos.insert(std::pair<std::string, Outline_pattern>(digest, op));
  else 
    itr->second = op;
  mysql_rwlock_unlock(&m_lock);
  return false;
}

bool Cdb_outline_loader::remove_outline_info(const std::string& digest)
{
  mysql_rwlock_wrlock(&m_lock);
  bool ret = !(m_outline_infos.erase(digest) > 0);
  mysql_rwlock_unlock(&m_lock);
  return ret;
}

void Cdb_outline_loader::clear_all_outline_info()
{
  mysql_rwlock_wrlock(&m_lock);
  m_outline_infos.erase(m_outline_infos.begin(), m_outline_infos.end());
  mysql_rwlock_unlock(&m_lock);
}

void Cdb_outline_loader::get_outline_info_for_display(std::vector<Outline_pattern> &rules)
{
  mysql_rwlock_wrlock(&m_lock);
  omi ri = m_outline_infos.begin();
  while (ri != m_outline_infos.end()) {
    rules.push_back(ri->second);
    ri++;
  }
  mysql_rwlock_unlock(&m_lock);
}

Outline_pattern Cdb_outline_loader::get_pattern_from_outline(const std::string &digest)
{
  mysql_rwlock_rdlock(&m_lock);
  omi itr = m_outline_infos.find(digest);
  Outline_pattern outline_pattern;
  if (itr == m_outline_infos.end()) {
    outline_pattern.m_is_hit = false;
  } else {
    outline_pattern = itr->second; 
    outline_pattern.m_is_hit = true;
    itr->second.m_hit_times++;
  }
  mysql_rwlock_unlock(&m_lock);

  return outline_pattern;
}
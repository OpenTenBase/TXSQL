/* Copyright (c) 2020, tencent and/or its affiliates. All rights reserved.
 */
#ifndef OPT_OUTLINE_LOADER_INCLUDED
#define OPT_OUTLINE_LOADER_INCLUDED

#include "sql/sql_class.h"

#define MAX_OUTLINE_TEXT_LEN 10240

namespace outline {
typedef struct Outline_pattern
{
  std::string m_origin_query;
  std::string m_outline_query;
  std::vector<int> m_param_positions;
  ulong m_hit_times;
  bool m_is_hit;
}Outline_pattern;

class Parameter_array_ptr
{
public:
  Parameter_array_ptr(int array_size)
    : m_ptr(new int[array_size]) { }
  Parameter_array_ptr &operator=(const Parameter_array_ptr &pap);
  Parameter_array_ptr(const Parameter_array_ptr &);
  ~Parameter_array_ptr() { delete[] m_ptr; }
  int *get() { return m_ptr; }
private:
  int *m_ptr;
};

/* 
 cdb_outline_walker: 
 walk the whole statement and banding the arguments to the outline pattern
  
  ie. sql: select * from t1 left join t2 on t1.a = t2.a where t1.a = 11;

 the format outline pattern is:
  "
   select `*` 
   from (`test`.`t1` USE INDEX (`idx1`) left join `test`.`t2` on((`t1`.`a` = `t2`.`a`))) 
   where (`t1`.`a` = ?)
  "
  the parameter vector of the format outline pattern is [115], 115 is the position of
  `?`, when call "select * from t1 left join t2 on t1.a = t2.a where t1.a = 99", `99`
  will be setted to the 115th char of the outline pattern.
 */
class Cdb_outline_walker
{
public:
  Cdb_outline_walker(Outline_pattern &outline_pattern)
    : m_previous_slot(0)
  {
    m_outline_query = outline_pattern.m_outline_query;
    for (size_t i = 0; i < outline_pattern.m_param_positions.size(); ++i)
      m_param_positions.push_back(outline_pattern.m_param_positions.at(i));
    m_param_positions_itr = m_param_positions.begin();
  }
  /*
    walk and build the whole statement of thd->lex.

    @retval false: success 
    @retval  true : failed 

  */
  bool walk_build_literals(THD *thd);

  /*
    banding the parameters to sql when handle with node of sql lex.

    @retval false: success 
    @retval  true : failed 

  */
  bool walk_build_literals_iterator(MYSQL_ITEM item);
  
  std::string get_outline_query();
  
private:
  int m_previous_slot;
  std::string m_built_query;
  std::string m_outline_query;
  /* parameters position vector */
  std::vector<int> m_param_positions;
  /* parameters position vector begin iterator */
  std::vector<int>::iterator m_param_positions_itr;
  
};

class Cdb_outline_loader
{
  typedef std::unordered_map<std::string, Outline_pattern> om;
  typedef std::unordered_map<std::string, Outline_pattern>::iterator omi;
  typedef std::pair<std::unordered_map<std::string, Outline_pattern>::iterator, bool> omr;
public:
  /*
    load the digest-outline pair into the memory map.

    @retval false: success 
    @retval  true : failed 

  */
  bool load_outline_info(const std::string& digest, const Outline_pattern& op);

  /*
    remove the outline info in memory map.

    @retval false: success 
    @retval  true : failed 

  */
  bool remove_outline_info(const std::string& digest);

  /*
    when execute the origin sql, firstly find it in memory, if exsits, return the outline
    and banding the parameters to outline, execute the new query.

    @in: digest(the digest hash value of the origin sql)

    @retval Outline_pattern(which has the outline text and parameters position vector.)

  */
  Outline_pattern get_pattern_from_outline(const std::string &digest);

  /*
    get the rules in memory for output.
    @in @out: vector for the rules.
  */
  void get_outline_info_for_display(std::vector<Outline_pattern> &rules);

  /*
   reset the outline info memory map.
   */
  void clear_all_outline_info();

  /*
    init and exit outline loader.
   */
  void init_outline_loader() { mysql_rwlock_init(m_key_lock, &m_lock);}
  void exit_outline_loader() { mysql_rwlock_destroy(&m_lock);}

private:
  PSI_rwlock_key m_key_lock;
  mysql_rwlock_t m_lock;
  om m_outline_infos;

};
} // namespace outline.

extern outline::Cdb_outline_loader cdb_outline_loader;

#endif /* OPT_OUTLINE_LOADER_INCLUDED */
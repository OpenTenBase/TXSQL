#ifndef TABLE_REWRITER_INCLUDED
#define TABLE_REWRITER_INCLUDED

#include "my_config.h"

#include <memory>
#include <string>

#include "map_helpers.h"
#include "my_inttypes.h"
#include "plugin.h"

const int ER_TABLE_REWRITER_TABLE_MALFORMED_ERROR = 1;
const int ER_TABLE_REWRITER_READ_FAILED = 2;
/**
  Implementation of the post parse query Table_rewriter. The public interface
  consists of two operations: reload(), which loads the rules from the disk
  table, and rewrite_table_name(), which rewrites a query if applicable.
*/
class Table_rewriter {
 public:
  Table_rewriter();

  /**
    The number of rules currently loaded in the hash table. In case of rules
    that fail to load, this number will be lower than the number of rows in
    the database.
  */
  int get_number_loaded_rules() const { return m_rules.size(); }

  ~Table_rewriter();

  /**
    Attempts to rewrite table name in current query with digest in 'key'.

    @return A Rewrite_result object.
  */
  void rewrite_table_name(MYSQL_THD thd);

  /**
    Empty the hashtable and reload all rules from disk table.
   */
  longlong reload(MYSQL_THD thd);

  /**
    Implementation of the loading procedure. The server doesn't handle
    different sessions in the same thread, so we load the rules into the hash
    table in this function, intended to be run in a new thread. The main
    thread will do join().

    @param session_thd The session to be used for loading rules.
  */
  void load_rewrite_rules(MYSQL_THD thd);
  
  void rewrite(MYSQL_TABLE_LIST tl);

  void add_rule(std::string db, std::string table, std::string new_table);

  void del_rule(std::string db, std::string table);

 private:
  longlong m_reload_status;

  /// The in-memory rules hash table.
  malloc_unordered_multimap<std::string, std::string> m_rules{
      PSI_INSTRUMENT_ME};
};

#endif /* Table_rewriter_INCLUDED */

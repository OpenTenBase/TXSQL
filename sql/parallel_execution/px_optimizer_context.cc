#include "px_optimizer_context.h"

#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "mutex_lock.h"  // MUTEX_LOCK
#include "my_dbug.h"
#include "sql/item.h"
#include "sql/key.h"
#include "sql/item_func.h"        // user_var_entry
#include "sql/sql_class.h"        // THD
#include "sql/sql_plugin.h"       // intern_plugin_lock
#include "thr_lock.h"
#include "thr_mutex.h"

#define my_intern_plugin_lock(A, B) intern_plugin_lock(A, B)
#define my_intern_plugin_lock_ci(A, B) intern_plugin_lock(A, B)

bool post_init_worker_thd(THD *coordinator_thd, THD *worker_thd) {
  assert(coordinator_thd && worker_thd);
  DBUG_TRACE;

  plugin_ref old_table_plugin = worker_thd->variables.table_plugin;
  plugin_ref old_temp_table_plugin = worker_thd->variables.temp_table_plugin;

  worker_thd->variables.table_plugin = nullptr;
  worker_thd->variables.temp_table_plugin = nullptr;

  // 1. copy variables
  cleanup_variables(worker_thd, &worker_thd->variables);
  worker_thd->variables = coordinator_thd->variables;

  // 2. update table_plugin
  mysql_mutex_lock(&LOCK_plugin);
  worker_thd->variables.table_plugin =
    my_intern_plugin_lock(nullptr, coordinator_thd->variables.table_plugin);
  intern_plugin_unlock(nullptr, old_table_plugin);
  worker_thd->variables.temp_table_plugin = my_intern_plugin_lock(
    nullptr, coordinator_thd->variables.temp_table_plugin);
  intern_plugin_unlock(nullptr, old_temp_table_plugin);
  mysql_mutex_unlock(&LOCK_plugin); 

  // 3. copy charset
  worker_thd->charset_is_system_charset =
      coordinator_thd->charset_is_system_charset;
  worker_thd->charset_is_collation_connection =
      coordinator_thd->charset_is_collation_connection;
  worker_thd->charset_is_character_set_filesystem =
      coordinator_thd->charset_is_character_set_filesystem;

  // 4. copy user_vars
  // get lock for get_variable
  mysql_mutex_lock(&worker_thd->LOCK_thd_data);

  worker_thd->user_vars.clear();
  worker_thd->user_vars.reserve(coordinator_thd->user_vars.size());

  // default charset
  const CHARSET_INFO *cs = coordinator_thd->variables.collation_connection;

  for (const auto &key_and_value : coordinator_thd->user_vars) {
    user_var_entry *sql_uvar = key_and_value.second.get();

    /*
      deep copy for worker thd, for each coordinator's entry:
      - build locally a new entry (construction)
      - copy value from coordinator's entry
      - add it to the worker_thd.
      Thus, the entry will destruction with worker thd.
    */

    /* Copy VARIABLE_NAME */
    const char *name = sql_uvar->entry_name.ptr();
    size_t name_length = sql_uvar->entry_name.length();
    const Name_string key(name, name_length);

    user_var_entry *entry = get_variable(worker_thd, key, cs);
    if (entry != nullptr) {
      /* Copy VARIABLE_VALUE */
      bool null_value;
      String *str_value;
      String str_buffer;
      uint decimals = 0;
      str_value = sql_uvar->val_str(&null_value, &str_buffer, decimals);
      if (str_value != nullptr) {
        entry->store(str_value->ptr(), str_value->length(),
                    STRING_RESULT, cs, DERIVATION_IMPLICIT,
                    false /* unsigned_arg */);
      } else {
        entry->store(nullptr, 0,
                    STRING_RESULT, cs, DERIVATION_IMPLICIT,
                    false /* unsigned_arg */);
      }
    }
  }

  mysql_mutex_unlock(&worker_thd->LOCK_thd_data);

  return false;
}

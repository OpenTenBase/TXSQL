#include "my_config.h"

#include <ctype.h>
#include <mysql.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "plugin/table_rewriter/plugin.h"

extern "C" {

bool load_table_rewriter_rules_init(UDF_INIT *, UDF_ARGS *, char *message) {
  if (get_table_rewriter_plugin_info() != nullptr) return false;
  strncpy(message, "Table rewriter plugin needs to be installed.", MYSQL_ERRMSG_SIZE);
  return true;
}

char *load_table_rewriter_rules(UDF_INIT *, UDF_ARGS *, char *, unsigned long *length,
                         unsigned char *is_null, unsigned char *) {
  DBUG_ASSERT(get_table_rewriter_plugin_info() != nullptr);
  const char *message = nullptr;
  if (reload_rules_table()) {
    message = "Loading of some rule(s) failed.";
    *length = static_cast<unsigned long>(strlen(message));
  } else
    *is_null = 1;

  return const_cast<char *>(message);
}

void load_table_rewriter_rules_deinit(UDF_INIT *) {}
}

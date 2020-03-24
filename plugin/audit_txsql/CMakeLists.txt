# audit support from txsql

MYSQL_ADD_PLUGIN(audit_txsql audit_txsql.cc
  MODULE_ONLY MODULE_OUTPUT_NAME "audit_txsql")

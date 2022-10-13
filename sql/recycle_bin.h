/* Copyright (c) 2020, tencent and/or its affiliates. All rights reserved.
 */

#ifndef RECYCLE_BIN_INCLUDED
#define RECYCLE_BIN_INCLUDED

#include <stdio.h>
#include <sys/types.h>
#include <new>
#include <string>

#include "sql/field.h"
#include "sql/table.h"
#include "sql/event_data_objects.h"
#include "sql/events.h"
#include "sql/dd/object_id.h"
#include "sql/system_variables.h"
#include "sql/rpl_table_access.h"
#include "sql/sql_class.h"

/**
  @todo learn and complete the doxygen style comment

  @defgroup recycle_bin recycle bin
  @ingroup Runtime_Environment
  @{

  @file sql/recycle_bin.h

  A public interface of recycle_bin module.
*/

enum enum_recycle_bin_op : int {
  RB_NO_OP,
  RB_RECYCLE_TABLE_BY_DROP,    // for master drop table
  RB_RECYCLE_TABLE_BY_RENAME,  // for slave drop table or truncate table
  RB_PURGE_TABLE,              // purge recycle bin table
  RB_RECOVERY_TABLE,           // recovery table from recycle bin
  RB_RECYCLE_TABLE_BY_TRUNCATE // for master truncate table
};

#define RB_DEBUG_INFO    "recycle_bin_info"
/**
    Cache operation information to be store in mysql.recycle_bin_info
 */
class Recycle_table_record {
 public:
  /////////////////////////////////////////////////////////////////////////
  // table_name
  /////////////////////////////////////////////////////////////////////////
  static const char *field_table_name() { return "table_name"; }

  std::string table_name() const { return m_table_name; }

  void set_table_name(const std::string &table_name) {
    m_table_name = table_name;
  }

  /////////////////////////////////////////////////////////////////////////
  // origin_schema
  /////////////////////////////////////////////////////////////////////////

  static const char *field_origin_schema() { return "origin_schema"; }

  std::string origin_schema() const { return m_origin_schema; }

  void set_origin_schema(const std::string &origin_schema) {
    m_origin_schema = origin_schema;
  }

  /////////////////////////////////////////////////////////////////////////
  // origin_table
  /////////////////////////////////////////////////////////////////////////
  
  static const char *field_origin_table() { return "origin_table"; }

  std::string origin_table() const { return m_origin_table; }

  void set_origin_table(const std::string &origin_table) {
    m_origin_table = origin_table;
  }

  /////////////////////////////////////////////////////////////////////////
  // drop_time
  /////////////////////////////////////////////////////////////////////////

  static const char *field_drop_time() { return "drop_time"; }

  void set_drop_time(const my_timeval &drop_time) {
    m_drop_time = drop_time;
  }

  my_timeval drop_time() const { return m_drop_time; }

  /////////////////////////////////////////////////////////////////////////
  // purge_time
  /////////////////////////////////////////////////////////////////////////
  
  static const char *field_purge_time() { return "purge_time"; }

  void set_purge_time(const my_timeval &purge_time) {
    m_purge_time = purge_time;
  }
  
  my_timeval purge_time() const { return m_purge_time; }

 private:
  // Fields
  std::string m_table_name;
  std::string m_origin_schema;
  std::string m_origin_table;
  my_timeval m_drop_time;
  my_timeval m_purge_time;
};

///////////////////////////////////////////////////////////////////////////

/**
  The class is used to simplify table data access. It open table on init, and
  closes table on deinit.

  mysql.reyccyle_bin_info interface

  @todo:
  get_table_category:
  1. We need to add category for this table.
 */
class Recycle_bin_access_context : public System_table_access {
 public:
  static const LEX_CSTRING DB_NAME;
  static const LEX_CSTRING TABLE_NAME;

  Recycle_bin_access_context() : m_drop_thd_object(nullptr) {}
  ~Recycle_bin_access_context() override = default;

  /**
    Prepares before opening table.
    - set flags

    @param[in]  thd  Thread requesting to open the table
  */
  void before_open(THD *thd) override;

  /**
    Initialize the recycle_bin_info table access context as following:
      - Create a new THD if current_thd is NULL
      - Disable binlog temporarily if we are going to modify the table
      - Open and lock a table.

    @param[in,out] thd       Thread requesting to open the table
    @param[out]    table     We will store the open table here
    @param[in]     is_write  If true, the access will be for modifying the table

    @retval true  failed
    @retval false success
  */
  bool init(THD **thd, TABLE **table, bool is_write);

  /**
    De-initialize the recycle_bin_info table access context as following:
      - Close the table
      - Re-enable binlog if needed
      - Destroy the created THD if needed.

    @param thd         Thread requesting to close the table
    @param table       Table to be closed

  */
  void deinit(THD *thd, TABLE *table);

  /**
    Creates a new thread in the bootstrap process or in the mysqld startup,
    a thread is created in order to be able to access a table. And reset a
    new "statement".

    @returns THD* Pointer to thread structure
  */
  THD *create_thd();

  void drop_thd(THD *thd);

  /**
    Close opened table. It is not responsible for transaction committing
    and rollback.

    @param[in] thd    Thread requesting to close the table
    @param[in] table  Table to be closed
  */
  void close_table(THD *thd, TABLE *table);

 private:
  /* Pointer to new created THD. */
  THD *m_drop_thd_object;
  /* Modify the table if it is true. */
  bool m_is_write;
  /* Save the lock info. */
  Open_tables_backup m_backup;
  /* Save binlog options. */
  ulonglong m_tmp_disable_binlog__save_options;
  /* Whether or not `THD::set_skip_readonly_check` was invoked during `THD`
  initialization */
  bool m_skip_readonly_set{false};

  Recycle_bin_access_context &operator=(const Recycle_bin_access_context &info);
  Recycle_bin_access_context(const Recycle_bin_access_context &info);
};

/**
  @class Recycle_bin_persistor

  Access mysql.recycle_bin_info through Recycle_bin_access_context.
*/
class Recycle_bin_persistor {
 public:
  enum FIELDS_ID {
    FIELD_TABLE_NAME = 0,
    FIELD_ORIGIN_SCHEMA,
    FIELD_ORIGIN_TABLE,
    FIELD_DROP_TIME,
    FIELD_PURGE_TIME,
    FIELDS_COUNT
  };

  Recycle_bin_persistor() = default;
  virtual ~Recycle_bin_persistor() = default;

  /**
    Insert the record into recycle_bin_info.

    @param thd  Thread requesting to save recycle_bin_info into the table
    @param record the record of recycle_bin_info.

    @retval
      0    OK
    @retval
      1    The table was not found.
    @retval
      -1   Error
  */
  int save(THD *thd, const Recycle_table_record *record);

  /**
    Insert records into recycle_bin_info.

    @param thd  Thread requesting to save recycle_bin_info into the table
    @param records All records that need to be inserted

    @retval
      0    OK
    @retval
      1    The table was not found.
    @retval
      -1   Error
  */
  int save(THD *thd, std::vector<Recycle_table_record> &records);


  /**
    Delete the row in the recycle_bin_info table.

    @param thd  Thread requesting to save recycle_bin_info into the table
    @param records The record to be deleted

    @retval 0    OK.
    @retval 1    The table was not found.
    @retval -1   Error.
  */
  int drop(THD *thd, const Recycle_table_record *record);

  /**
    Delete the rows in the recycle_bin_info table.

    @param thd  Thread requesting to save recycle_bin_info into the table
    @param records The record to be deleted

    @retval 0    OK.
    @retval 1    The table was not found.
    @retval -1   Error.
  */
  int drop(THD *thd, std::vector<Recycle_table_record> &records);

  /**
    Delete the rows in the recycle_bin_info table.

    @param thd  Thread requesting to save recycle_bin_info into the table
    @param records The record to be deleted

    @retval 0    OK.
    @retval 1    The table was not found.
    @retval -1   Error.
  */
  int drop(THD *thd, Prealloced_array<TABLE_LIST *, 1> &tables);

 private:
  /**
    Write a row into the recycle_bin_info table.

    @param  table    Reference to a table object.
    @param  record   The record to be inserted.

    @retval 0    OK.
    @retval -1   Error.
  */
  int write_row(TABLE *table, const Recycle_table_record *record);

  /**
    Fill fields of the recycle_bin_info table.

    @param  fields   Reference to table fields.
    @param  record   Fill fields with the record.

    @retval false  OK.
    @retval true   Error.
  */
  bool fill_fields(Field **fields, const Recycle_table_record *record);

  /**
    Delete the row in the recycle_bin_info table.

    @param  table the record to be droped

    @retval 0    OK.
    @retval -1   Error.
  */
  int delete_row(TABLE *table, const Recycle_table_record *record);
};

/**
   @class Recycle_bin_event
   @brief A class mainly supported purge table automatically.

   When NOW() greater than or equal the purge_time of the record in
   Recycle_bin_table, we need to drop the table and delete the record.

   Use event module to implement schedule and execution of task.
 */
class Recycle_bin_event {
public:
  Recycle_bin_event() = default;

  bool queue_event(THD *thd, Event_queue *event_queue);

  bool init_job_data(THD *thd, Event_job_data &event);

  bool is_recycle_bin_event(LEX_CSTRING &db_name, LEX_CSTRING &event_name);

  static Recycle_bin_event &instance() {
    static Recycle_bin_event ins;
    return ins;
  }

private:
  bool init_event_basic(THD *thd, Event_basic &event);

  bool init_queue_element(THD *thd, Event_queue_element &event,
                          ulong interval_time);

private:
  static constexpr LEX_CSTRING m_event_name =
      {STRING_WITH_LEN("purge_recycle_bin_table")};
  static constexpr LEX_CSTRING m_definer =
      {STRING_WITH_LEN("@")};

  /**
    Due to the default definer is "@",So use empty user and host to avoid change_security_context in Event_job_data::execute here.
  */
  static constexpr LEX_CSTRING m_definer_user =
      {STRING_WITH_LEN("")};
  static constexpr LEX_CSTRING m_definer_host =
      {STRING_WITH_LEN("")};

  static constexpr const char *m_time_zone= "SYSTEM";
  /**
    @note: execute the sql code every 30 seconds
    @todo: use "DISABLE ON SLAVE" to disable execution on slave -by dct

    ```sql
    CREATE EVENT purge_recycle_bin_table
     ON SCHEDULE EVERY 30 SECOND
     DO
       BEGIN
         DECLARE CONTINUE HANDLER FOR SQLEXCEPTION BEGIN END;
         SET @purge_table_name = "";
         SET @drop_stmt = "";
         SET @drop_record_stmt= "";
         WHILE @purge_table_name IS NOT NULL DO
           SET @purge_table_name = NULL;
           SELECT `table_name` FROM `mysql`.`recycle_bin_info` WHERE `purge_time`=(SELECT MIN(`purge_time`) FROM `mysql`.`recycle_bin_info` WHERE `purge_time` <= NOW()) INTO @purge_table_name;
           IF @purge_table_name IS NOT NULL THEN
             SET @drop_stmt = CONCAT('DROP TABLE `__txsql_recycle_bin__`', '.`', @purge_table_name, '`');
             PREPARE stmt_p from @drop_stmt;
             EXECUTE stmt_p;
           END IF;
         END WHILE;
       END |
     ```
  */
  static constexpr LEX_CSTRING m_definition = {STRING_WITH_LEN(
      "BEGIN\n        DECLARE CONTINUE HANDLER FOR SQLEXCEPTION BEGIN "
      "END;\n        SET @purge_table_name = \"\";\n        SET "
      "@drop_stmt = \"\";\n        SET @drop_record_stmt= \"\";\n "
      "WHILE @purge_table_name IS NOT NULL DO\n          SET "
      "@purge_table_name = NULL;\n          SELECT `table_name` FROM "
      "`mysql`.`recycle_bin_info` WHERE `purge_time`=(SELECT "
      "MIN(`purge_time`) FROM `mysql`.`recycle_bin_info` WHERE "
      "`purge_time` <= NOW()) INTO @purge_table_name;\n          IF "
      "@purge_table_name IS NOT NULL THEN\n            SET @drop_stmt = "
      "CONCAT('DROP TABLE `__txsql_recycle_bin__`', '.`', "
      "@purge_table_name, '`');\n            PREPARE stmt_p from "
      "@drop_stmt;\n            EXECUTE stmt_p;\n          END IF;\n "
      "END WHILE;\n      END")};
  static const sql_mode_t m_sql_mode =
      (MODE_NO_ENGINE_SUBSTITUTION | MODE_ONLY_FULL_GROUP_BY |
      MODE_STRICT_TRANS_TABLES | MODE_NO_ZERO_IN_DATE |
      MODE_NO_ZERO_DATE | MODE_ERROR_FOR_DIVISION_BY_ZERO);

  // my_charset_utf8_general_ci
  static const dd::Object_id m_client_collation_id = 33;

  // my_charset_utf8_general_ci
  static const dd::Object_id m_connection_collation_id = 33;

  // same as default_charset_info = my_charset_utf8mb4_0900_ai_ci
  static const dd::Object_id m_schema_collation_id = 255;
};

LEX_CSTRING get_recycle_bin_table_name(THD *thd);

bool deny_access_recycle_bin_schema(THD *thd, TABLE_LIST *all_tables);

bool recycle_bin_enabled(THD *thd);

bool recycle_bin_enabled_in_user_thread(THD *thd);

static inline LEX_STRING make_lex_string(MEM_ROOT *mem_root,
                                         const LEX_CSTRING str) {
  LEX_STRING lex_str;
  lex_str.str = strmake_root(mem_root, str.str, str.length);
  lex_str.length = str.length;
  return lex_str;
}

static inline LEX_CSTRING make_lex_cstring(MEM_ROOT *mem_root,
                                           const LEX_CSTRING str) {
  LEX_STRING lex_str = make_lex_string(mem_root, str);
  return LEX_CSTRING{lex_str.str, lex_str.length};
}


/**
  @} (end of group recycle bin)
*/
#endif  // RECYCLE_BIN_INCLUDED

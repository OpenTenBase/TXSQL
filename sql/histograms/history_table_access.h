#ifndef HISTOGRAMS_HISTORY_TABLE_ACCESS_INCLUDED
#define HISTOGRAMS_HISTORY_TABLE_ACCESS_INCLUDED

/**
  @file sql/histograms/history_table_access.h
*/

#include "sql/dd/string_type.h"    // String_type
#include "sql/rpl_table_access.h"  // System_table_access
#include "sql/sql_class.h"         // Open_tables_backup

namespace dd {
class Column_statistics;
}  // namespace dd

namespace histograms {

class Histogram;

/**
  @class History_table_access_context

  The class is used to simplify table data access. It open table on init, and
  closes table on deinit.
*/
class History_table_access_context : public System_table_access {
 public:
  History_table_access_context();
  ~History_table_access_context() override;

  void before_open(THD *thd) override;

  /**
    Initialize the column_statistics_history table access context.

    @param thd      Thread requesting to open the table
    @param is_write If true, the access will be for modifying the table

    @retval false success
    @retval true  failed
  */
  bool init(THD *thd, bool is_write = false);
  bool deinit();

  THD *create_thd();
  void drop_thd(THD *thd);

  friend class History_table_persistor;

 private:
  /// Thread context.
  THD *m_thd{nullptr};
  /// Used to determine whether m_thd needs to be dropped.
  bool m_thd_need_drop{false};
  /// Whether there is an error when updating the table and needs to be
  /// rolledback.
  int m_error{};
  /// The table mysql.column_statistics_history;
  TABLE *m_table{nullptr};
  /// Backup state required by System_table_access::open_table()
  Open_tables_backup m_backup{};
  /// Store the thread_local variable current_thd.
  THD *m_thd_save{nullptr};
  /// Store the thread_local variable THR_MALLOC.
  MEM_ROOT **m_mem_save{nullptr};
#ifndef DBUG_OFF
  bool m_deinited{true};
#endif
};

/**
  @class History_table_persistor

  Access mysql.column_statistics_history through History_table_access_context.
*/
class History_table_persistor {
 public:
  History_table_persistor(THD *thd) : m_thd{thd} {}
  ~History_table_persistor() {}

  /**
    Create a record for the histogram and save the record in the
    mysql.column_statistics_history table

    @param schema_name  schema name of histogram.
    @param table_name   table name of histogram.
    @param column_name  column name of histogram.
    @param histogram    the histogram.

    @retval false success
    @retval true  failed
  */
  bool save(const dd::String_type &schema_name,
            const dd::String_type &table_name,
            const dd::String_type &column_name, const Histogram *histogram);

  /**
    Find the histogram by name and version.

    @param schema_name  schema name of histogram.
    @param table_name   table name of histogram.
    @param column_name  column name of histogram.
    @param version      number of version. If equal to
    Histogram::LATEST_VERSION, find the latest histogram.
    @param[out] histogram   the return value histogram.

    @retval false success
    @retval true  failed

    @note Table mysql.column_statistics_history is defined with primary key
    (name, version). If version is a specific version, find the corresponding
    record according to (name, version); if version is
    Histogram::LATEST_VERSION, find the last record according to (name).
  */
  bool load(const dd::String_type &schema_name,
            const dd::String_type &table_name,
            const dd::String_type &column_name, int64_t version,
            Histogram **histogram);

  /**
    Drop histogram version records by name

    @param schema_name  schema name of histogram.
    @param table_name   table name of histogram.
    @param column_name  column name of histogram.

    @retval false success
    @retval true  failed
  */
  bool drop(const dd::String_type &schema_name,
            const dd::String_type &table_name,
            const dd::String_type &column_name);

  /**
    Rename histogram version records by name

    @param old_schema_name  old schema name of histogram.
    @param old_table_name   old table name of histogram.
    @param new_schema_name  new schema name of histogram.
    @param new_table_name   new table name of histogram.
    @param column_name      column name of histogram.

    @retval false success
    @retval true  failed
  */
  bool rename(const dd::String_type &old_schema_name,
              const dd::String_type &old_table_name,
              const dd::String_type &new_schema_name,
              const dd::String_type &new_table_name,
              const dd::String_type &column_name);

 private:
  /**
    Create a record for the histogram and save the record in the
    mysql.column_statistics_history table

    @param table_ctx    context of access table.
    @param schema_name  schema name of histogram.
    @param table_name   table name of histogram.
    @param column_name  column name of histogram.
    @param histogram    the histogram.

    @retval false success
    @retval true  failed
  */
  bool write_row(History_table_access_context &table_ctx,
                 const dd::String_type &schema_name,
                 const dd::String_type &table_name,
                 const dd::String_type &column_name,
                 const Histogram *histogram);

  /**
    Find the histogram by name and version. Use argument histogram to get the
    histogram after the function call.

    @param table_ctx    context of access table.
    @param schema_name  schema name of histogram.
    @param table_name   table name of histogram.
    @param column_name  column name of histogram.
    @param version      number of version. If equal to
    Histogram::LATEST_VERSION, find the latest histogram.
    @param[out] histogram    the return value histogram.

    @retval false success
    @retval true  failed
  */
  bool read_row(History_table_access_context &table_ctx,
                const dd::String_type &schema_name,
                const dd::String_type &table_name,
                const dd::String_type &column_name, int64_t version,
                Histogram **histogram);

  /**
    This method keeps N records in table mysql.column_statistics_history and
    deletes redundant records. N depends on global system variable
    histogram_history_versions_limit.

    @param table_ctx    context of access table.
    @param schema_name  schema name of histogram.
    @param table_name   table name of histogram.
    @param column_name  column name of histogram.

    @retval false success
    @retval true  failed
  */
  bool shrink(History_table_access_context &table_ctx,
              const dd::String_type &schema_name,
              const dd::String_type &table_name,
              const dd::String_type &column_name);

  /**
    This method deletes all records with the same dd_name create by the last
    three parameters.

    @param table_ctx    context of access table.
    @param schema_name  schema name of histogram.
    @param table_name   table name of histogram.
    @param column_name  column name of histogram.

    @retval false success
    @retval true  failed
  */
  bool delete_stats(History_table_access_context &table_ctx,
                    const dd::String_type &schema_name,
                    const dd::String_type &table_name,
                    const dd::String_type &column_name);

  /**
    This method renames records that match the old name to the new name.

    @param table_ctx    context of access table.
    @param schema_name  schema name of histogram.
    @param table_name   table name of histogram.
    @param column_name  column name of histogram.

    @retval false success
    @retval true  failed
  */
  bool rename_stats(History_table_access_context &table_ctx,
                    const dd::String_type &old_schema_name,
                    const dd::String_type &old_table_name,
                    const dd::String_type &new_schema_name,
                    const dd::String_type &new_table_name,
                    const dd::String_type &column_name);

  /// Thread context.
  THD *m_thd{nullptr};

  /// The MEM_ROOT which stores the histogram.
  /// Note: When calling Histogram::store_histogram,
  /// m_mem_rot will move to dd along with the histogram.
  MEM_ROOT m_mem_root;
};

}  // namespace histograms

#endif

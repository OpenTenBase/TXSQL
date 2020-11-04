/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   CDB firewall for txsql 8.0 
*/
#ifndef _FIREWALL_TABLE_SERVICE_H_
#define _FIREWALL_TABLE_SERVICE_H_
#include <string>
#include <vector>

class THD;
struct TABLE_LIST;
class Field;
class Userhost_Mode_Cache;
class Whitelist_Cache;

namespace firewall_table_service {

  /*
  Parameter of  Cursor::store_field_values_and_flush
  std::pair::first is field_idx
  std::pair::second is the value that will be stored into field 
  */
  typedef std::vector<std::pair<uint, std::string>> Field_idx_values_sequence;

  typedef enum {
    USERMODE_LOAD,
    WHITELIST_LOAD,
    WHITELIST_FLUSH,
  } Callback_type;

  struct Firewall_callback_args{
    void *me;
    THD *thd;
    Callback_type type;

   Firewall_callback_args(void *_me, THD *_thd, Callback_type _type):
    me(_me), thd(_thd), type(_type){}
  };

  /**
  This function is used to create a new thread
  to load or flush data for User_Mode_Cache or Whitelist_Cache
  Called by User_Mode_Cache/Whitelist_Cache::load_data_from_table/flush_to_disk

  @param type specify cache and operations(load or flush)
  */
  void load_or_flush_data(Callback_type type);

  /**
  Writeable cursor that allows reading and updating rows
  in firewall table.
  */
  class Cursor {
 public:
    typedef uint column_id;

    static const column_id ILLEGAL_COLUMN_ID = -1;

    /**
    Creates a cursor to an already-opened table.
    */
    explicit Cursor(THD *thd, const char *db_name, const char *table_name);

    /// Closes the table scan if initiated and commits the transaction.
    ~Cursor();

    /**
    Whether the cursor finish reading of the table,
    in other words, last record of the table has beed
    read by this cursor
    */
    bool is_finished() { return m_is_finished; }

    /**
    Read next record of the table, it read the last record,
    set m_is_finished to true.
    */
    int read();

    /**
    Reads from a Cursor and writes to a property of type std::string
    after forcing a copy of the string buffer.
    */
    void copy_and_set(std::string *property, int colno);

    const char *get_db_name() { return "mysql"; }

    virtual const char *get_table_name()=0;

    /**
    Store field values to table.
    Called by Whitelist_Cursor or User_Mode_Cursor

    @param[in] field_idx_and_values The sequence of field_idx and field_values
    */
    bool store_field_values_and_flush(Field_idx_values_sequence &field_idx_and_values);

    bool inited() { return m_inited; }
 protected:
    bool m_inited;

    int field_index(const char *field_name);

    /**
    Fetches the value of the column with the given number as a C string.

    This interface is meant for crossing dynamic library boundaries, hence the
    use of C-style const char*. The function casts a column value to a C
    string and returns a copy, allocated in the callee's DL. The pointer
    must be freed using free_string().

    @param fieldno One of PATTERN_COLUMN, REPLACEMENT_COLUMN, ENABLED_COLUMN
    or MESSAGE_COLUMN.
    */
    const char *fetch_string(int fieldno);

    THD *m_thd;
    TABLE_LIST *m_table_list;
    bool m_table_is_malformed;
    bool m_is_finished;
    int m_last_read_status;
  };

  class User_Mode_Cursor : public Cursor {
 public:
    explicit User_Mode_Cursor(THD *thd);

    column_id userhost_column() const { return m_userhost_column; }
    column_id mode_column() const { return m_mode_column; }

    virtual const char *get_table_name() { return "cdb_firewall_users"; }

 private:
    uint m_userhost_column;
    uint m_mode_column;
  };

  class Whitelist_Cursor : public Cursor {
 public:
    explicit Whitelist_Cursor(THD *thd);

    virtual const char *get_table_name() { return "cdb_firewall_whitelist"; }

    column_id userhost_column() const { return m_userhost_column; }
    column_id digest_column() const { return m_digest_column; }

 private:
    uint m_userhost_column;
    uint m_digest_column;
  };

} // namespace firewall_table_service

#endif //_FIREWALL_TABLE_SERVICE_H_

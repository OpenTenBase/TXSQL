/* Copyright (c) 2020, Tencent and/or its affiliates. All rights reserved.
   for txsql 5.7
*/

#ifndef SQL_TENCENTROOT_H_INCLUDED
#define SQL_TENCENTROOT_H_INCLUDED


#include "sql/log.h"                        /* sql_print_error */
#include "sql/mysqld.h"
#include "sql/sys_vars_shared.h"            /* PolyLock */

#define TENCENT_ROOT_NO_LIMIT            -1
#define TENCENT_ROOT_NOT_FOUND           -1
#define TENCENT_ROOT_VERIFY_ERROR        -1

#define TENCENT_ROOT_DELIMITER           '#'
#define TENCENT_ROOT_IP_DELIMITER        ';'

#define TENCENT_ROOT_ALL_USER_STR        "%"

#define TENCENT_ROOT_WHITELIST_SIZE      512
#define TENCENT_ROOT_USER_SIZE           512

#define TENCENT_ROOT_DELIMITER_LEN       1
#define TENCENT_ROOT_IP_DELIMITER_LEN    1
/* Compatible with IPv6 */
#define TENCENT_ROOT_IP_STR_LEN          INET6_ADDRSTRLEN
#define TENCENT_ROOT_USER_STR_LEN        64
#define TENCENT_ROOT_PWD_STR_LEN         16
#define TENCENT_ROOT_CONF_NAME_STR_LEN   64
#define TENCENT_ROOT_LIMIT_STR_LEN       MAX_INT_WIDTH
#define TENCENT_ROOT_EMBEDDED_HOSTS_NUM  3

#define EXTRA_MAX_CONNECTIONS_DEFAULT    300

extern const char *const name_tencent_root_cnf;

extern const char *const name_tencent_root_enabled_conf;
extern const char *const name_tencent_root_enabled_var;

extern const char *const name_tencent_root_user;
extern const char *const name_tencent_root_pwd;
extern const char *const name_tencent_root_whitelist;
extern const char *const name_tencent_root_limit;

extern ulong tencent_root_connection_count[TENCENT_ROOT_USER_SIZE + 1];

extern
const char *tencent_root_embedded_hosts[TENCENT_ROOT_EMBEDDED_HOSTS_NUM + 1];

extern void split_val(const char *val, char **array, uint *size);
extern void build_val(const char *const *array, char **val);
extern void build_val(const char *const *array, char delimiter, char **val);
extern void* tencent_root_malloc(size_t size);


class Config_base
{
public:
  static inline void reset_array(char **val)
  {
    assert(val);
    for (; *val; val++)
    {
      my_free(*val);
      *val= 0;
    }
  }

  static inline void init_array(char **array, uint size)
  {
    for (uint i= 0; i < size; i++) array[i]= 0;
  }

  static inline  int in_array(const char **array, const char *val)
  {
    int n= 0;
    while (array[n])
    {
      if (!strcmp(val, array[n])) return n;
      ++n;
    }
    return TENCENT_ROOT_NOT_FOUND;
  }

  static inline int in_array(char **array, const char *val)
  {
    return in_array(const_cast<const char**>(array), val);
  }

  static inline void parse_val_with_size(const char *val, char **array, uint &size)
  {
    assert(val);
    assert(array);
    reset_array(array);
    split_val(val, array, &size);
  }

  static inline void parse_val(const char *val, char **array)
  {
    uint size= 0;
    parse_val_with_size(val, array, size);
  }
};


class Tencentroot_config_base: public Config_base
{
private:
  const char *var_name;

public:
  Tencentroot_config_base(const char *var_name_arg)
    : var_name(var_name_arg) {}

  virtual ~Tencentroot_config_base() {}

  const char *name() const
  {
    return var_name;
  }

  /**
     The type of set val
   */
  virtual Item_result type() const
  {
    return STRING_RESULT;
  }
  /**
     Use for array[] --> val.
     for example:
     {{localhost;127.0.0.1}, {127.0.0.1}, {127.0.0.1;::1}} -->
                  localhost127.0.0.1#127.0.0.1#127.0.0.1;::1
   */
  virtual bool build(char **val)= 0;

  /**
     Use for val --> array.
     for example:
     localhost127.0.0.1#127.0.0.1#127.0.0.1;::1 -->
                   {{localhost;127.0.0.1}, {127.0.0.1}, {127.0.0.1;::1}}
  */
  virtual bool parse(const char *val)= 0;

  /**
     Use for array[index]=val.
  */
  virtual bool set_val(ulong index, const char *val)= 0;

  /**
     Use for val=array[index].
  */
  virtual void get_val(ulong index, char **val)= 0;

  /**
     Find val in array[]
   */
  virtual int find_val(const char *val [[maybe_unused]])
  {
    assert(val);
    return TENCENT_ROOT_NOT_FOUND;
  }

  /**
     Clear array[]
   */
  virtual void clear()= 0;

  /**
     Verify the validity of val, often used before parse().
  */
  virtual int verify(const char *val, uint size, bool check_size)= 0;
};


class Tencentroot_config_charptr_array: public Tencentroot_config_base
{
private:
  typedef bool (*verify_val_func)(const char *val);

  char *array[TENCENT_ROOT_USER_SIZE + 1];
  const verify_val_func verify_val;

public:
  Tencentroot_config_charptr_array(const char *name_arg,
                                verify_val_func verify_val_arg)
    : Tencentroot_config_base(name_arg),
      verify_val(verify_val_arg)
  {
    for (size_t i= 0; i < TENCENT_ROOT_USER_SIZE + 1; i++)
    {
      array[i]= 0;
    }
  }

  ~Tencentroot_config_charptr_array()
  {
    clear();
  }

  bool build(char **val) override
  {
    assert(val);
    build_val(array, val);
    return true;
  }

  bool parse(const char *val) override
  {
    parse_val(val, array);
    return true;
  }

  bool set_val(ulong index, const char *val) override
  {
    assert(val);
    my_free(array[index]);
    array[index]= (char *)my_memdup(PSI_NOT_INSTRUMENTED,
                                    val,
                                    strlen(val) + 1,
                                    MYF(MY_FAE));
    return true;
  }

  void get_val(ulong index, char **val) override
  {
    assert(val);
    char *ptr= array[index];
    if (ptr)
    {
      *val= (char*) my_strdup(key_memory_Sys_var_charptr_value,
                              ptr, MYF(MY_FAE));
    }
    else
    {
      *val= 0;
    }
  }

  int find_val(const char *val) override
  {
    assert(val);
    return in_array(array, val);
  }

  void clear() override
  {
    reset_array(array);
  }

  int verify(const char *val, uint size, bool check_size) override
  {
    char *arr[TENCENT_ROOT_USER_SIZE + 1]= { 0 };
    uint arr_size= 0;
    parse_val_with_size(val, arr, arr_size);
    /* only password will check size */
    if (check_size && size != arr_size)
    {
      sql_print_information(
        "TXSQL: tencent root pwd parse failure, first check whether each user"
        " has a corresponding password configuration item, that is, the"
        " password item number (%u) and user item number (%u) separated by '#'"
        " are equal.", arr_size, size);
      reset_array(arr);
      return TENCENT_ROOT_VERIFY_ERROR;
    }
    for (int i= 0; arr[i]; i++)
    {
      if(!verify_val(arr[i]))
      {
        reset_array(arr);
        return TENCENT_ROOT_VERIFY_ERROR;
      }
    }
    reset_array(arr);
    return arr_size;
  }
};

class Tencentroot_config_user: public Tencentroot_config_charptr_array
{
public:
  Tencentroot_config_user(const char *name_arg)
    : Tencentroot_config_charptr_array(name_arg, verify_user) {}

  static bool verify_user(const char *val)
  {
    size_t len= strlen(val);
    if (len > 0 && len <= TENCENT_ROOT_USER_STR_LEN)
    {
      return true;
    }
    else
    {
      sql_print_information(
        "TXSQL: tencent root user parse failure, please check username"
        " (\"%s\") is between 1 and 64 (including 1 and 64).", val);
      return false;
    }
  }
};

class Tencentroot_config_password: public Tencentroot_config_charptr_array
{
public:
  Tencentroot_config_password(const char *name_arg)
    : Tencentroot_config_charptr_array(name_arg, verify_password) {}

  ~Tencentroot_config_password()
  {
    my_free(opt_tencent_root_pwd);
  }

  int parse_password(const char *val, uint size)
  {
    clear();
    int parse_size= verify(val, size, true);
    if (parse_size != TENCENT_ROOT_VERIFY_ERROR)
    {
      parse(val);
      my_free(opt_tencent_root_pwd);
      opt_tencent_root_pwd= 0;
      build(&opt_tencent_root_pwd);
    }
    return parse_size;
  }

  static bool verify_password(const char *val)
  {
    if (strlen(val) <= TENCENT_ROOT_PWD_STR_LEN)
    {
      return true;
    }
    else
    {
      sql_print_information(
        "TXSQL: tencent root pwd parse failure, please check whether the"
        " password length of each user is between 0 and 16 (including 0 and"
        " 16).");
      return false;
    }
  }
};

class Tencentroot_config_whitelist: public Tencentroot_config_base
{
private:
  char *array[TENCENT_ROOT_USER_SIZE + 1][TENCENT_ROOT_WHITELIST_SIZE + 1];

public:
  Tencentroot_config_whitelist(const char *name_arg)
    : Tencentroot_config_base(name_arg)
  {
    for (size_t i= 0; i < TENCENT_ROOT_USER_SIZE + 1; i++)
    {
      init_array(array[i], TENCENT_ROOT_WHITELIST_SIZE+1);
    }
  }

  ~Tencentroot_config_whitelist()
  {
    clear();
  }

  bool parse(const char *val) override
  {
    assert(val);
    clear();
    char *arr[TENCENT_ROOT_USER_SIZE + 1]= { 0 };
    parse_val(val, arr);
    for (uint i= 0; arr[i]; i++) set_val(i, arr[i]);
    reset_array(arr);
    return true;
  }

  bool build(char **val) override
  {
    assert(val);
    char *arr[TENCENT_ROOT_USER_SIZE + 1]= { 0 };
    for (uint i= 0; array[i][0] ; i++)
    {
      get_val((ulong)i, &arr[i]);
    }
    build_val(arr, val);
    reset_array(arr);
    return true;
  }

  bool set_val(ulong index, const char *val) override
  {
    int n= 0, len= 0;
    char **sub_arr= array[index];

    reset_array(sub_arr);
    while (tencent_root_embedded_hosts[n] && n < TENCENT_ROOT_WHITELIST_SIZE - 2)
    {
      len= strlen(tencent_root_embedded_hosts[n]) + 1;
      sub_arr[n]= (char*)tencent_root_malloc(len);
      snprintf(sub_arr[n], len, "%s", tencent_root_embedded_hosts[n]);
      ++n;
    }
    /*
      White list had being set to empty, we recognize this as normal case,
      simplify parse action and reset to default status
    */
    if (!strcmp(val, "")) return true;
    recreate_whitelist_array(val, &n, sub_arr);
    return true;
  }

  void get_val(ulong index, char **val) override
  {
    assert(val);
    assert(index <= TENCENT_ROOT_USER_SIZE);
    build_val(array[index], TENCENT_ROOT_IP_DELIMITER, val);
  }

  int find_val_in_user(const char *val, uint index) 
  {
    assert(val);
    assert(index <= TENCENT_ROOT_USER_SIZE);
    return in_array(array[index], val);
  }

  int find_val(const char *val) override
  {
    assert(val);
    for (uint user_index= 0; array[user_index][0]; user_index++)
    {
      if (find_val_in_user(val, user_index) != TENCENT_ROOT_NOT_FOUND)
      {
        return user_index;
      }
    }
    return TENCENT_ROOT_NOT_FOUND;
  }

  void clear() override
  {
    for (uint user_index= 0; array[user_index][0]; user_index++)
    {
      reset_array(array[user_index]);
    }
  }

  int verify(const char *val, uint size, bool check_size) override
  {
    assert(val);
    assert(size <= TENCENT_ROOT_USER_SIZE);
    char *arr[TENCENT_ROOT_USER_SIZE + 1]= { 0 };
    uint arr_size= 0;
    int result= TENCENT_ROOT_VERIFY_ERROR;
    parse_val_with_size(val, arr, arr_size);
    if (check_size && size != arr_size)
    {
      sql_print_information(
        "TXSQL: tencent root whitelist parse failure, first check whether each"
        " user has a corresponding whitelist configuration item, that is, the"
        " whitelist item number (%u) and user item (%u) separated by '#' are"
        " equal.", arr_size, size);
      goto error;
    }
    for (int i= 0; arr[i]; i++)
    {
      if(!verify_val(arr[i])) goto error;
    }
    result= (int)arr_size;
  error:
    reset_array(arr);
    return result;
  }

  /**
  check values in whilelist is legal or not.
  */
  static bool verify_val(const char *val)
  {
    /*
      Handle the situation when tencent_root_whitelist = NULL,
      when val is NULL.
    */
    if (!val) return false;

    int count= 0;
    char per_ip[TENCENT_ROOT_IP_STR_LEN]= { 0 };
    char fmt[64]= { 0 };
    const char *str= val;
    const char *end= str + strlen(str);
    unsigned char buf[sizeof(struct in6_addr) + 1];
    snprintf(fmt, sizeof(fmt), "%%%d[^%c]",
             (int)sizeof(per_ip)-1, TENCENT_ROOT_IP_DELIMITER);
    /*
      White list had being set to empty, we recognize this as normal case,
      simplify parse action and reset to default status
    */
    if (!val[0] || !strcmp(val, "")) return true;

    do
    {
      memset(per_ip, 0, sizeof(per_ip));
      if (1 != sscanf(str, fmt, per_ip))
      {
        sql_print_information(
          "TXSQL: tencent root whitelist parse failure, parse from: %s,"
          " ERROR!", str);
        return false;
      }
      /* Can be IPv4 or IPv6 */
      if (in_array(tencent_root_embedded_hosts, per_ip) ==
                                     TENCENT_ROOT_NOT_FOUND)
      { /* Skip 'embedded hosts' */
        if (1 != inet_pton(AF_INET, per_ip, buf) &&
            1 != inet_pton(AF_INET6, per_ip, buf))
        {
          sql_print_information(
            "TXSQL: tencent root whitelist parse failure, ip: %s, error:"
            " invalid ip address.", per_ip);
          return false;
        }
        /* Remove 3 local IP addresses */
        if (++count > TENCENT_ROOT_WHITELIST_SIZE - TENCENT_ROOT_EMBEDDED_HOSTS_NUM + 1)
        {
          sql_print_information(
            "TXSQL: tencent root whitelist parse failure, tencent root whitelist"
            " IPs other than local IP cannot exceed %d.",
            TENCENT_ROOT_WHITELIST_SIZE - TENCENT_ROOT_EMBEDDED_HOSTS_NUM);
          return false;
        }
      }

      str += strlen(per_ip) + 1;
    } while (str < end);

    return true;
  }

private:
  /* Re-create white list, val had been verified! */
  static void recreate_whitelist_array(const char *val, int *list_size, char **arr)
  {
    assert(val);
    assert(*list_size <= TENCENT_ROOT_WHITELIST_SIZE);
    assert(arr);
    int len= 0;
    char per_ip[TENCENT_ROOT_IP_STR_LEN]= { 0 };
    char fmt[64]= { 0 };
    const char *str= val;
    const char *end= str + strlen(str);
    snprintf(fmt, sizeof(fmt), "%%%d[^%c]",
             (int)sizeof(per_ip)-1, TENCENT_ROOT_IP_DELIMITER);

    if (*list_size > TENCENT_ROOT_WHITELIST_SIZE) return;
    do
    {
      memset(per_ip, 0, sizeof(per_ip));
      sscanf(str, fmt, per_ip);

      if (in_array(arr, per_ip) == TENCENT_ROOT_NOT_FOUND)
      {
        len= strlen(per_ip) + 1;
        arr[*list_size]= (char*)tencent_root_malloc(len);
        snprintf(arr[*list_size], len, "%s", per_ip);
        if (++(*list_size) >= TENCENT_ROOT_WHITELIST_SIZE) break;
      }

      str += strlen(per_ip) + 1;
    } while (str < end);
  }
};


class Tencentroot_config_limit: public Tencentroot_config_base
{
private:
  bool in_use;
  long array[TENCENT_ROOT_USER_SIZE + 1];
  ulong *upper_limit;

public:
  Tencentroot_config_limit(const char *name_arg, ulong *upper_limit_arg)
    : Tencentroot_config_base(name_arg),
      in_use(true),
      upper_limit(upper_limit_arg)
  {
    init(array);
  }

  virtual Item_result type() const override
  {
    return INT_RESULT;
  }

  bool build(char **val) override
  {
    assert(val);
    assert(is_use());
    char *arr_limit[TENCENT_ROOT_USER_SIZE + 1]= { 0 };
    for (uint i= 0; array[i] != TENCENT_ROOT_NO_LIMIT; i++)
    {
      get_val(i, arr_limit + i);
    }
    build_val(arr_limit, val);
    reset_array(arr_limit);
    return true;
  }

  bool parse(const char *val) override
  {
    assert(val);
    assert(is_use());
    clear();
    return parse_low(val, array);
  }

  bool set_val(ulong index, const char *val) override
  {
    assert(val);
    assert(is_use());
    sscanf(val, "%ld", array + index);
    return true;
  }

  void get_val(ulong index, char **val) override
  {
    assert(val);
    assert(!*val);
    uint max_size= 16;
    *val= (char*)tencent_root_malloc(max_size);
    if (index == 0 && !is_use())
    {
      memset(*val, 0, max_size);
    }
    else if(snprintf(*val, max_size, "%ld", array[index]) < 0)
    {
      my_free(*val);
      *val= 0;
    }
  }

  long get_val(ulong index)
  {
    if (index == 0 && !is_use()) return (long)*upper_limit;
    else return array[index];
  }

  void clear() override
  {
    reset(array);
  }

  int verify(const char *val, uint size, bool check_size) override
  {
    assert(val);
    long arr[TENCENT_ROOT_USER_SIZE + 1]= { 0 };
    init(arr);
    int arr_size= parse_low(val, arr);
    if (arr_size == TENCENT_ROOT_VERIFY_ERROR ||
        (check_size && (uint)arr_size != size))
    {
      sql_print_information(
        "TXSQL: tencent root limit parse failure, first check whether each"
        " user has a corresponding limit configuration item, that is, the"
        " limit item number (%d) and user item number (%u) separated by '#'"
        " are equal.", arr_size, size);
      return TENCENT_ROOT_VERIFY_ERROR;
    }
    long sum= 0;
    for(uint i= 0; arr[i] != TENCENT_ROOT_NO_LIMIT; i++)
    {
      sum+= arr[i];
    }
    if ((ulong)sum > *upper_limit)
    {
      sql_print_information(
        "TXSQL: tencent root limit parse failure,  please check that the sum"
        " (%ld) of the tencent root limit of all users is less than or equal to"
        " the system variable extra_max_connnections (%lu).", sum, *upper_limit);
      return TENCENT_ROOT_VERIFY_ERROR;
    }
    return arr_size;
  }

  /**
     check values in whilelist is legal or not.
  */
  bool verify_val(ulong index, const char *val)
  {
    ulong user_limit= 0;
    ulong sum= 0;
    if (!is_legal_decimal_number(val) || sscanf(val, "%lu", &user_limit) != 1)
    {
      sql_print_information(
        "TXSQL: tencent root limit parse failure, please check that the"
        " tencent root limit (\"%s\") is a valid decimal number.", val);
      return false;
    }
    sum+= user_limit;
    for (uint i= 0;
         array[i] != TENCENT_ROOT_NO_LIMIT;
         i++)
    {
      if (i != index) sum += array[i];
    }
    if ((ulong)sum <= *upper_limit)
    {
      return true;
    }
    else
    {
      sql_print_information(
        "TXSQL: tencent root limit parse failure,  please check that the sum"
        " (%lu) of the tencent root limit of all users is less than or equal to"
        " the system variable extra_max_connnections (%lu).", sum, *upper_limit);
      return false;
    }
  }

  /* In old style tmy.conf, there is no Tencent root limit configuration */
  void set_no_use()
  {
    in_use= false;
  }

  bool is_use()
  {
    return in_use;
  }

  bool verify_extra_max_connections(ulong val)
  {
    long sum= 0;
    for (uint i= 0; array[i] != TENCENT_ROOT_NO_LIMIT; i++)
    {
      sum+= array[i];
    }
    if (val >= (ulong)sum)
    {
      return true;
    }
    else
    {
      sql_print_information(
        "TXSQL: tencent root limit parse failure,  please check that the sum"
        " of the tencent root limit of all users is less than or equal to the"
        " system variable extra_max_connnections.");
      return false;
    }
  }

private:
  static bool is_legal_decimal_number(const char *val)
  {
    const char *start= val;
    if (!start) return false;

    while (*start != '\0' && std::isdigit(*start)) ++start;
    if(*val =='\0' || *start != '\0') return false;
    return true;
  }

  static int parse_low(const char *val, long *arr)
  {
    assert(val);
    char *arr_limit[TENCENT_ROOT_USER_SIZE + 1]= { 0 };
    uint size= 0;
    parse_val_with_size(val, arr_limit, size);
    reset(arr);
    for (uint i= 0; i < TENCENT_ROOT_USER_SIZE + 1 && arr_limit[i]; i++)
    {
      if (!is_legal_decimal_number(arr_limit[i]) ||
          sscanf(arr_limit[i], "%lu", (arr + i)) != 1)
      {
        sql_print_information(
          "TXSQL: tencent root limit parse failure, please check that the"
          " tencent root limit of each user is a valid decimal number.");
        reset_array(arr_limit);
        return TENCENT_ROOT_VERIFY_ERROR;
      }
    }
    reset_array(arr_limit);
    return (int)size;
  }

  static void init(long *arr)
  {
    for (size_t i= 0; i < TENCENT_ROOT_USER_SIZE + 1; i++)
    {
      arr[i]= TENCENT_ROOT_NO_LIMIT;
    }
  }

  static void reset(long *arr)
  {
    for(uint i= 0; arr[i] != TENCENT_ROOT_NO_LIMIT; i++)
    {
      arr[i]= TENCENT_ROOT_NO_LIMIT;
    }
  }
};

extern PolyLock_rwlock tencent_root_all_lock;

extern Tencentroot_config_password tencent_root_password_config;

extern bool in_tencent_root_whitelist(const char *ip);

extern bool is_tencent_root_array_var(const char *var_name);

extern bool reload_tencent_root_cnf(void);

extern bool flush_tencent_root_cnf(THD *thd);

extern const char *get_tencent_root_login_user(THD *thd);

extern bool init_tencent_root();

extern bool check_tencent_root_user_and_ip(THD *thd,
                                           NET *net,
                                           const char *user_name);

extern bool inc_tencent_root_count(THD *thd);

extern void dec_tencent_root_count(THD *thd);

extern void mysqld_show_tencent_root(THD *thd,
                                     const LEX_STRING tencent_root_user);

#endif

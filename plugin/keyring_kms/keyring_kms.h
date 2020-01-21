#include "my_config.h"
#include <mysql/plugin_keyring.h>

extern char *keyring_kms_file;
extern char *keyring_kms_config;
extern bool keyring_kms_check;

#define DUMP_FILE_SUFFIX     ".backup"

int check_kms_config_data(MYSQL_THD thd  MY_ATTRIBUTE((unused)),
                          SYS_VAR *var  MY_ATTRIBUTE((unused)),
                          void *save, st_mysql_value *value);

void update_kms_config_data(MYSQL_THD thd  MY_ATTRIBUTE((unused)),
                            SYS_VAR *var  MY_ATTRIBUTE((unused)),
                            void *var_ptr MY_ATTRIBUTE((unused)),
                            const void *save_ptr);

int load_kms_config_data();

int check_kms_config(MYSQL_THD thd  MY_ATTRIBUTE((unused)),
                     SYS_VAR *var  MY_ATTRIBUTE((unused)),
                     void *save, st_mysql_value *value);

int show_kms_secret_id(MYSQL_THD thd,
                       SHOW_VAR *var,
                       char *buff);

int show_kms_secret_key(MYSQL_THD thd,
                       SHOW_VAR *var,
                       char *buff);

int show_kms_end_point(MYSQL_THD thd,
                       SHOW_VAR *var,
                       char *buff);

int show_kms_cam_point(MYSQL_THD thd,
                       SHOW_VAR *var,
                       char *buff);

int show_kms_role_name(MYSQL_THD thd,
                       SHOW_VAR *var,
                       char *buff);

int show_kms_uin(MYSQL_THD thd,
                 SHOW_VAR *var,
                 char *buff);

int show_kms_error(MYSQL_THD thd,
                   SHOW_VAR *var,
                   char *buff);

int show_kms_region(MYSQL_THD thd,
                    SHOW_VAR *var,
                    char *buff);

bool kms_key_generate(const char *key_id, const char *key_type,
                      const char *user_id, size_t key_len);

bool kms_key_fetch(const char *key_id, char **key_type, const char *user_id,
                   void **key, size_t *key_len);

bool kms_key_store(const char *key_id, const char *key_type,
                   const char *user_id, const void *key, size_t key_len);

bool kms_key_remove(const char *key_id, const char *user_id);


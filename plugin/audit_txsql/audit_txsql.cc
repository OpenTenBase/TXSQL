/* Copyright (c) 2019, Tencent and/or its affiliates. All rights reserved.
   Audit support from txsql.
*/
#include <mysql/plugin_audit.h>
#include <typelib.h>
#include "audit_txsql.h"
#include "sql/mysqld.h"
#include "mysql/psi/mysql_mutex.h"

#define OS_FILE_MAX_PATH  4000
#define DELIMITER         '#'
#define ERR_LEN           1024
#define SECONDS_OF_A_DAY  (24*60*60)
#define AUDIT_ALL         0
#define AUDIT_FILTER      1
#define AUDIT_OFF         2

static char *filter_ip = NULL;
static char *filter_user = NULL;
static char *filter_db = NULL;
static char *audit_dir = NULL;
static ulong audit_mode = 0;
static uint  trunc_len = 0;
static long  file_max_size = 0;
static int   rotate_count = 0;
static bool  rotate_write = false;

static audit_handler normal_user;

static ulong log_safety_level = 0;
int log_safe_factor = 0;
#define AUDIT_LOG_SAFETY_NORMAL 0
#define AUDIT_LOG_SAFETY_SAFE 1
#define AUDIT_LOG_SAFETY_SAFEST 2

static char str10[] = "0123456789";
static char str100[] = "0001020304050607080910111213141516171819202122232425"
                       "2627282930313233343536373839404142434445464748495051"
                       "5253545556575859606162636465666768697071727374757677"
                       "78798081828384858687888990919293949596979899";

static const char *audit_filename_group[10] = {
  "audit_log1", "audit_log2", "audit_log3", "audit_log4", "audit_log5",
  "audit_log6", "audit_log7", "audit_log8", "audit_log9", "audit_log10"};

/**
  get_log_file_name_in_append
  get audit log file name in append mode
  @param[in,out] log_name log file name
  @param[in] log_dir log file dir
  @param[in] fps file position.
*/
void get_log_file_name_in_append(char *log_name, const char *log_dir, file_pos *fps) {
  time_t tmp_time;
  struct tm tmp_tm;
  time(&tmp_time);
  localtime_r(&tmp_time, &tmp_tm);

  if (labs(tmp_time - fps->date_start_time) > SECONDS_OF_A_DAY) {
    fps->date_start_time = tmp_time - (tmp_time + tmp_tm.tm_gmtoff) % SECONDS_OF_A_DAY;
    fps->count = 0;
    fps->pos = 0;
  }
  sprintf(log_name, "%saudit_log.%04d-%02d-%02d.%d", log_dir, tmp_tm.tm_year + 1900,
          tmp_tm.tm_mon + 1, tmp_tm.tm_mday, fps->count);
}

/**
  check_save_file_thread
  Flush must meet one of two conditions:
  1.Need to write the log file is greater than 2MB
  2.Interval more than 1s (FLUSH_PER_SEC))
  @param[in] ptr, audit_handler
*/
void *check_save_file_thread(void *ptr) {
  audit_handler *handler = (audit_handler*)ptr;
  int cur_write, cur_flush, cur_end;
  char *mem = handler->share_mem;
  time_t cur_time;
  int current_log_safety_level;

  time(&handler->last_flush);

  while (handler->is_running) {
    current_log_safety_level = log_safety_level;

    if (current_log_safety_level) mysql_mutex_lock(&handler->mem_lock);
    cur_flush = ((share_mem_head *)mem)->flush_pos;
    cur_write = ((share_mem_head *)mem)->write_pos;
    cur_end = ((share_mem_head *)mem)->end_pos;
    if (current_log_safety_level) mysql_mutex_unlock(&handler->mem_lock);

    usleep(current_log_safety_level == AUDIT_LOG_SAFETY_SAFE
               ? log_safe_factor * 1000
               : 4000);
    if (audit_mode != AUDIT_ALL && audit_mode != AUDIT_FILTER) {
      continue;
    }
    time(&cur_time);

    if (!rotate_write &&
        labs(cur_time - handler->fps.date_start_time) > SECONDS_OF_A_DAY) {
      char log_name[512] = {0};
      mysql_mutex_lock(&(handler->file_lock));
      if (NULL == handler->log_file_fp || NULL == handler->file_pos_fp ) {
        audit_err_log("Failed to create audit log file or position file, "
                      "check the Create File Permissions and Disk Space.");
        mysql_mutex_unlock(&(handler->file_lock));
        usleep(1000000);
        continue;
      }
      fclose(handler->log_file_fp);
      handler->fps.count = 0;
      handler->fps.pos = 0;
      get_log_file_name_in_append(log_name, audit_dir, &(handler->fps));

      handler->log_file_fp = fopen(log_name, "wb+");
      if (NULL == handler->log_file_fp) {
        audit_err_log(log_name);
        audit_err_log("Failed to create audit log file, check the "
                      "Create File Permissions and Disk Space.");
        mysql_mutex_unlock(&(handler->file_lock));
        usleep(1000000);
        continue;
      }
      fseek(handler->file_pos_fp, 0L, SEEK_SET);
      fwrite(&(handler->fps), sizeof(file_pos), 1, handler->file_pos_fp);
      fflush(handler->file_pos_fp);
      mysql_mutex_unlock(&(handler->file_lock));
    }

    /* When flush_pos < write_pos, The data to be saved is from the
       flush_pos to write_pos. */
    if (cur_flush < cur_write &&
        ((cur_write - cur_flush > FLUSH_SEND_BUFFER) ||
         (cur_time - handler->last_flush >= FLUSH_PER_SEC))) {
      if (handler->save_to_file(mem + cur_flush, cur_write - cur_flush)) {
        continue;
      }
      if (current_log_safety_level) mysql_mutex_lock(&handler->mem_lock);
      ((share_mem_head *)mem)->flush_pos = cur_write;
      if (current_log_safety_level) mysql_mutex_unlock(&handler->mem_lock);
    }

    /* When flush_pos > write_pos,  The data to be saved is from the
       flush_pos to end position and the start position to write_pos. */
    else if (cur_flush > cur_write &&
             ((cur_flush - cur_write < GLOBAL_AUDIT_MEM_BLOCK - FLUSH_SEND_BUFFER) ||
              (cur_time - handler->last_flush >= FLUSH_PER_SEC))) {
      /* If the write failed, continue, should not modify the position info. */
      if (handler->save_to_file(mem + cur_flush, cur_end - cur_flush)) {
        continue;
      }

      if (current_log_safety_level) mysql_mutex_lock(&handler->mem_lock);
      ((share_mem_head *)mem)->end_pos = 0;
      ((share_mem_head *)mem)->flush_pos = SHARE_MEM_HEAD_SIZE;
      if (current_log_safety_level) mysql_mutex_unlock(&handler->mem_lock);
      if (handler->save_to_file(mem + SHARE_MEM_HEAD_SIZE,
                                cur_write - SHARE_MEM_HEAD_SIZE)) {
        continue;
      }
      if (current_log_safety_level) mysql_mutex_lock(&handler->mem_lock);
      ((share_mem_head *)mem)->flush_pos = cur_write;
      if (current_log_safety_level) mysql_mutex_unlock(&handler->mem_lock);
    }
  }
  return NULL;
}

/**
  mem_and_thread_init
  init the memory and thread
*/
void audit_handler::mem_and_thread_init(void) {
  pthread_attr_t attr;

  share_mem = (char*) malloc(GLOBAL_AUDIT_MEM_BLOCK);
  memset(share_mem, 0, GLOBAL_AUDIT_MEM_BLOCK);
  ((share_mem_head*)share_mem)->write_pos = SHARE_MEM_HEAD_SIZE;
  ((share_mem_head*)share_mem)->flush_pos = SHARE_MEM_HEAD_SIZE;

  if (NULL == thread_file) {
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);

    thread_file = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(thread_file, &attr, check_save_file_thread, (void*)this);
    is_running = true;
  }
}

/**
  convert_to_json
  The statement to be recorded into json format, save to memory
  @param[in] event_general source data
*/
void audit_handler::convert_to_json(struct mysql_event_general *event_general) {
  int total_len = 0, query_len = 0, mem_pos = 0;
  int tmp_write = 0, new_write = 0, tmp_flush = 0;
  int exec_len = 0, time_len = 0, err_len = 0, thread_len = 0;
  int sent_rows_len = 0, affect_row_len = 0, check_row_len = 0;
  int lock_wait_len = 0, cpu_time_len = 0, io_wait_len = 0,
      ns_time_len = 0, trx_time_len = 0;
  int current_log_safety_level = log_safety_level;

  ulonglong thread_id = (ulonglong) event_general->general_thread_id;

  /* Get the length of the current action when converted to json string. */
  affect_row_len = longlong_len(&event_general->general_rows);
  exec_len = longlong_len(&event_general->general_exec_time);
  time_len = longlong_len(&event_general->general_time);
  err_len = int_len(&event_general->general_error_code);
  CLAC_ULOGN_LEN(total_len, FORMAT_THREAD_LEN, thread_len, thread_id);
  CLAC_ULOGN_LEN(total_len, FORMAT_CHECK_ROW_LEN, check_row_len,
                 event_general->general_check_rows);
  CLAC_ULOGN_LEN(total_len, FORMAT_CPU_TIME_LEN, cpu_time_len,
                 event_general->general_cpu_time);
  CLAC_ULOGN_LEN(total_len, FORMAT_NS_TIME_LEN, ns_time_len,
                 event_general->general_ns_st_time);
  CLAC_ULOGN_LEN(total_len, FORMAT_IO_WAIT_LEN, io_wait_len,
                 event_general->general_io_wait_time);
  CLAC_ULOGN_LEN(total_len, FORMAT_SENT_ROW_LEN, sent_rows_len,
                 event_general->general_sent_rows);
  CLAC_ULOGN_LEN(total_len,FORMAT_LOCK_WAIT_LEN, lock_wait_len,
                 event_general->general_lock_wait_time);
  CLAC_ULOGN_LEN(total_len,FORMAT_TRX_TIME_LEN, trx_time_len,
                 event_general->general_trx_utime);
  query_len = event_general->general_query.length > trunc_len ?
              trunc_len : event_general->general_query.length;

  total_len += (affect_row_len + exec_len + time_len + err_len +
                FORMAT_STR_LEN_part1 + event_general->general_ip.length +
                FORMAT_STR_LEN_part2 + event_general->general_user.length +
                event_general->general_db.length + query_len +
                event_general->general_sql_command.length);

  /* Lock to set mem offset. */
  mysql_mutex_lock(&mem_lock);
  tmp_write = ((share_mem_head*) share_mem)->write_pos;
  new_write = tmp_write + total_len;
  tmp_flush = ((share_mem_head*)share_mem)->flush_pos;

  /* Ignore the following cases in which pressure is high. */
  if (tmp_write < tmp_flush && new_write > tmp_flush) {
    ++ignore_actions;
    mysql_mutex_unlock(&mem_lock);
    return;
  } else if (tmp_write > tmp_flush && new_write > GLOBAL_AUDIT_MEM_BLOCK &&
             total_len + SHARE_MEM_HEAD_SIZE > tmp_flush) {
    ++ignore_actions;
    mysql_mutex_unlock(&mem_lock);
    return;
  }

  /* Update position information for normal cases. */
  if (new_write < GLOBAL_AUDIT_MEM_BLOCK) {
    ((share_mem_head*)share_mem)->write_pos = new_write;
    mem_pos = tmp_write;
  } else {
    /* Write from the beginning. */
    ((share_mem_head*)share_mem)->write_pos = SHARE_MEM_HEAD_SIZE + total_len;
    ((share_mem_head*)share_mem)->end_pos = tmp_write;
    mem_pos = SHARE_MEM_HEAD_SIZE;
  }
  if (current_log_safety_level < AUDIT_LOG_SAFETY_SAFEST)
    mysql_mutex_unlock(&mem_lock);

  /* Start memory copy after free the lock. */
  FORMAT_NUMBER_JSON("{\"timestamp\":", FORMAT_TIMESTAMP_LEN,
    event_general->general_time, time_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"threadId\":", FORMAT_THREAD_LEN,
    thread_id, thread_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"checkRows\":", FORMAT_CHECK_ROW_LEN,
    event_general->general_check_rows, check_row_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"affectRows\":", FORMAT_AFFROW_LEN,
    event_general->general_rows, affect_row_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"sentRows\":", FORMAT_SENT_ROW_LEN,
    event_general->general_sent_rows, sent_rows_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"lockWaitTime\":", FORMAT_LOCK_WAIT_LEN,
    event_general->general_lock_wait_time, lock_wait_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"cpuTime\":", FORMAT_CPU_TIME_LEN,
    event_general->general_cpu_time, cpu_time_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"ioWaitTime\":", FORMAT_IO_WAIT_LEN,
    event_general->general_io_wait_time, io_wait_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"nsTime\":", FORMAT_NS_TIME_LEN,
    event_general->general_ns_st_time, ns_time_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"trxLivingTime\":", FORMAT_TRX_TIME_LEN,
    event_general->general_trx_utime, trx_time_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"execTime\":", FORMAT_EXECTIME_LEN,
    event_general->general_exec_time, exec_len, longnum2str);
  FORMAT_NUMBER_JSON(",\"errCode\":", FORMAT_ERROR_LEN,
    event_general->general_error_code, err_len, num2str);
  FORMAT_STRING_JSON(",\"host\":\"", FORMAT_IP_LEN,
    event_general->general_ip.str, event_general->general_ip.length);
  FORMAT_STRING_JSON("\",\"user\":\"", FORMAT_USERNAME_LEN,
    event_general->general_user.str, event_general->general_user.length);
  FORMAT_STRING_JSON("\",\"dbName\":\"", FORMAT_DBNAME_LEN,
    event_general->general_db.str, event_general->general_db.length);

  memcpy(CUR_AUDIT_MEM_PTR, "\",\"sql\":\"", FORMAT_SQLTEXT_LEN);
  mem_pos += FORMAT_SQLTEXT_LEN;
  if (query_len > 0) {
    char *dest_ptr = CUR_AUDIT_MEM_PTR;
    char *src_ptr = const_cast<char *>(event_general->general_query.str);
    int  i = 0;
    while(i < query_len) {
      switch(src_ptr[i]) {
      case '\"':
        dest_ptr[i] = '\'';
        break;
      case '\\':
        dest_ptr[i] = '/';
        break;
      case '\n':
      case '\r':
      case '\t':
      case '\0':
        dest_ptr[i] = ' ';
        break;
      default:
        dest_ptr[i] = src_ptr[i];
      }
      i++;
    }

    trunc_sql_counts += (event_general->general_query.length > trunc_len);
    mem_pos += query_len;
  }
  *CUR_AUDIT_MEM_PTR = '\"';
  mem_pos++;

  FORMAT_STRING_JSON(",\"sqlType\":\"", FORMAT_SQL_TYPE_LEN,
                     event_general->general_sql_command.str,
                     event_general->general_sql_command.length);
  memcpy(CUR_AUDIT_MEM_PTR, "\"}\n", FORMAT_END_LEN);
  mem_pos += FORMAT_END_LEN;
  if (current_log_safety_level == AUDIT_LOG_SAFETY_SAFEST)
    mysql_mutex_unlock(&mem_lock);
}

/**
  save_to_file
  save data to file
  @param[in] mem data mem buffer
  @param[in] len data len
  @return[in] false on success, true on failure
*/
bool audit_handler::save_to_file(char *mem, int len) {
  char log_name[512]={0};

  if (0 == len) {
    return false;
  }

  mysql_mutex_lock(&file_lock);
  if (NULL == log_file_fp || NULL == file_pos_fp ) {
    audit_err_log("Failed to create audit log file or position file, "
                  "check the Create File Permissions and Disk Space.");
    mysql_mutex_unlock(&file_lock);
    usleep(1000000);
    return true;
  }

  /* If current file size is larger than file_max_size, close current file. */
  if (fps.pos + len > file_max_size) {
    fclose(log_file_fp);

    if (rotate_write) {
      fps.count++;
      if (fps.count >= rotate_count) {
        fps.count = 0;
      }
      fps.pos = 0;
      fps.date_start_time = 0;
      sprintf(log_name, "%s%s", audit_dir, audit_filename_group[fps.count]);
      if (access(log_name,F_OK) == 0) {
        remove(log_name);
      }
    } else {
      fps.count++;
      fps.pos = 0;
      get_log_file_name_in_append(log_name, audit_dir, &fps);
    }

    log_file_fp = fopen(log_name, "wb+");
    if (NULL == log_file_fp) {
      audit_err_log(log_name);
      audit_err_log("Failed to create audit log file, check the "
                    "Create File Permissions and Disk Space.");
      mysql_mutex_unlock(&file_lock);
      usleep(1000000);
      return true;
    }
  }

  fwrite(mem, 1, len, log_file_fp);
  fflush(log_file_fp);

  fps.pos += len;
  fseek(file_pos_fp, 0L, SEEK_SET);
  fwrite(&fps, sizeof(file_pos), 1, file_pos_fp);
  fflush(file_pos_fp);

  time(&last_flush);
  mysql_mutex_unlock(&file_lock);
  return false;
}

/**
  seek_file_pos
  check and reset the file pos
  @param[in] log_name file name
  @param[in] fps file number
  @return false on success, true on failure
*/
bool audit_handler::seek_file_pos(char *log_name, file_pos *fps) {
  /* The maximum file length : file_max_size
     There are three possible cases:
     1: 0 ~ file_max_size, normal
     2: pos < 0 or pos > file_max_size, abnormal
     3: pos = file_max_size, normal but it's rare to happen. */
  if (fps->pos >=0 && fps->pos < file_max_size) {
    /* case 1 */
    return (fseek(log_file_fp, fps->pos, SEEK_SET));
  } else if (fps->pos < 0 ||  fps->pos > file_max_size) {
    /* case 2. */
    fps->pos = 0;
    return (fseek(log_file_fp, 0L, SEEK_SET));
  } else {
    /* case 3. */
    fclose(log_file_fp);

    /* We have already reached the end of the file, open a new log file. */
    if (rotate_write) {
      fps->count++;
      if (fps->count >= rotate_count) {
        fps->count = 0;
      }
      fps->pos = 0;
      fps->date_start_time = 0;
      sprintf(log_name, "%s%s", audit_dir, audit_filename_group[fps->count]);
      if (access(log_name,F_OK) == 0) {
        remove(log_name);
      }
    } else {
      fps->count++;
      fps->pos = 0;
      get_log_file_name_in_append(log_name, audit_dir, fps);
    }

    fseek(file_pos_fp, 0L, SEEK_SET);
    fwrite(fps, sizeof(file_pos), 1, file_pos_fp);
    fflush(file_pos_fp);

    log_file_fp = fopen(log_name, "wb+");
    CHECK_FILE_PTR(log_file_fp, log_name);

    return false;
  }
}

/**
  audit_log_file_init
  When this function returns false, the file_pos_fp
  and log_file_fp are available
  @return false on success, true on failure
*/
void audit_handler::audit_log_file_init(const char *new_dir) {
  char file_name[512] = {0};
  char log_name[512] = {0};

  mysql_mutex_lock(&file_lock);
  /* Get the position file name. */
  if (file_pos_fp != NULL) {
    fclose(file_pos_fp);
  }

  if (log_file_fp != NULL) {
    fclose(log_file_fp);
  }

  if (rotate_write) {
    sprintf(file_name, "%s%s%s", new_dir, "rotate_", POS_FILE);
  } else {
    sprintf(file_name, "%s%s%s", new_dir, "append_", POS_FILE);
  }

  if ((access(file_name, F_OK)) == 0 &&
      (file_pos_fp = fopen(file_name, "rb+"))) {
    /* Get audit log file name and write pos. */
    fread(&fps, sizeof(file_pos), 1, file_pos_fp);

    /* Get audit log name, the audit log file to write. */

    if (rotate_write) {
      sprintf(log_name, "%s%s", new_dir, audit_filename_group[fps.count]);
    } else {
      get_log_file_name_in_append(log_name, new_dir, &fps);
    }
    /* Try to open log file according to fps. */
    if (access(log_name, F_OK) != 0 ||
        (log_file_fp = fopen(log_name, "rb+")) == nullptr ||
        seek_file_pos(log_name, &fps) == true) {
      /* Failed to seek log file pos, create a new one. */
      if (access(log_name,F_OK) == 0) {
        remove(log_name);
      }
      log_file_fp = fopen(log_name, "wb+");
      CHECK_FILE_PTR(log_file_fp, log_name);
      fps.pos = 0;
      fseek(file_pos_fp, 0L, SEEK_SET);
      fwrite(&fps, sizeof(file_pos), 1, file_pos_fp);
      fflush(file_pos_fp);
    }
  } else {
    /* When the pos_file does not exist. */
    file_pos_fp = fopen(file_name, "wb+");
    CHECK_FILE_PTR(file_pos_fp, file_name);
    fps.count = 0;
    fps.pos = 0;
    if (rotate_write) {
      fps.date_start_time = 0;
      sprintf(log_name, "%s%s", new_dir, audit_filename_group[fps.count]);
      if (access(log_name,F_OK) == 0) {
        remove(log_name);
      }
    } else {
      fps.date_start_time = 0;
      get_log_file_name_in_append(log_name, new_dir, &fps);
    }
    fwrite(&fps, sizeof(file_pos), 1, file_pos_fp);
    fflush(file_pos_fp);

    log_file_fp = fopen(log_name, "wb+");
    CHECK_FILE_PTR(log_file_fp, log_name);
  }
  mysql_mutex_unlock(&file_lock);
}

volatile ulonglong audit_handler::ignore_actions = 0;
volatile ulonglong audit_handler::trunc_sql_counts = 0;

void audit_handler::init() {
  mysql_mutex_init(0, &mem_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(0, &file_lock, MY_MUTEX_INIT_FAST);

  mem_and_thread_init();
  if (audit_mode == AUDIT_ALL || audit_mode == AUDIT_FILTER) {
    audit_log_file_init(audit_dir);
  }
}


/**
  num2str longnum2str
  Convert numbers to strings, the same functionality as sprintf
  To improve efficiency, the use of two string array 0 ~ 9 and
  00 ~ 99,str10 and str100
  @param[in] dest tail ptr
  @param[in] num  number
  @param[in] len  length
*/
static void num2str(char *dest, int num, int len) {
  int i = 1;
  while (len > 1) {
    memcpy(dest - 2 * i++, str100 + (num % 100) * 2, 2);
    len -= 2;
    num /= 100;
  }

  if (1 == len)
    *(dest - (2 * i - 1)) = str10[num];
}

static void longnum2str(char *dest, ulonglong num, int len) {
  int i = 1;
  while (len > 1) {
    memcpy(dest - 2 * i++, str100 + (num % 100) * 2, 2);
    len -= 2;
    num /= 100;
  }

  if (1 == len)
    *(dest - (2 * i - 1)) = str10[num];
}

/**
  Initialize the plugin at server start or plugin installation
  @return 0 on success, 1 on failure
*/
static int audit_txsql_plugin_init(void *arg MY_ATTRIBUTE((unused))) {
  normal_user.init();
  return(0);
}

/**
  Terminate the plugin at server shutdown or plugin deinstallation
  @return 0 on success, 1 on failure
*/
static int audit_txsql_plugin_deinit(void *arg MY_ATTRIBUTE((unused))) {
  normal_user.deinit();

  return(0);
}

/**
  audit_err_log
  @param[in] errstr error string
*/
void audit_err_log(const char *info) {
  time_t t;
  struct tm * lt;
  char err[512] = { 0 };

  time(&t);
  lt = localtime(&t);
  snprintf(err, sizeof(err), "%d-%02d-%02d %02d:%02d:%02d [AUDIT WARNING] %s\n",
          lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour,
          lt->tm_min, lt->tm_sec, info);
  fprintf(stderr, "%s", err);
}

struct mysql_event_general *setup_fake_event_general(
    struct mysql_event_general *fake_event_general,
    struct mysql_event_connection *event_connection,
    time_t cur_time) {
  memset(fake_event_general, 0, sizeof(*fake_event_general));

  fake_event_general->general_db.str = event_connection->database.str;
  fake_event_general->general_db.length = event_connection->database.length;
  fake_event_general->general_ip.str = event_connection->ip.str;
  fake_event_general->general_ip.length = event_connection->ip.length;
  fake_event_general->general_time = cur_time;
  fake_event_general->general_user.str = event_connection->user.str;
  fake_event_general->general_user.length = event_connection->user.length;
  fake_event_general->general_error_code = event_connection->status;

  return fake_event_general;
}

bool in_filter(const char *filter, const char *str, char delimiter) {
  if (filter == nullptr || str == nullptr)
    return false;

  char *ptr = strstr(const_cast<char *>(filter), const_cast<char *>(str));
  if (ptr == nullptr)
    return false;

  /* check the delimeter before str. */
  if (ptr - filter > 0 && *(ptr - 1) != delimiter)
    return false;

  /* check the delimiter after str. */
  uint filter_len = strlen(filter);
  uint str_len = strlen(str);
  if ((ptr - filter) + str_len < filter_len && *(ptr + str_len) != delimiter)
    return false;

  return true;
}

/**
  audit_txsql_notify
  @param[in] event_class audit event type
  @param[in] event audit event
  @return 0 on success, 1 on failure
*/
static int audit_txsql_notify(MYSQL_THD thd MY_ATTRIBUTE((unused)),
  mysql_event_class_t event_class, const void *event){
  time_t cur_time;
  struct mysql_event_general *event_general = NULL;
  struct mysql_event_general fake_event_general;

  if (MYSQL_AUDIT_GENERAL_CLASS == event_class) {
    event_general = static_cast<struct mysql_event_general *>
      (const_cast<void *>(event));

   } else if (MYSQL_AUDIT_CONNECTION_CLASS == event_class) {
    struct mysql_event_connection *event_connection =
      static_cast<struct mysql_event_connection *>(const_cast<void *>(event));

    time(&cur_time);
    event_general = setup_fake_event_general(
        &fake_event_general, event_connection, cur_time);

    switch (event_connection->event_subclass) {
      case MYSQL_AUDIT_CONNECTION_CONNECT:
        event_general->general_sql_command.str = const_cast<char *>("connect");
        event_general->general_sql_command.length = strlen("connect");
        break;
      case MYSQL_AUDIT_CONNECTION_DISCONNECT:
        event_general->general_sql_command.str = const_cast<char *>("disconnect");
        event_general->general_sql_command.length = strlen("disconnect");
        break;
      case MYSQL_AUDIT_CONNECTION_CHANGE_USER:
        event_general->general_sql_command.str = const_cast<char *>("change_user");
        event_general->general_sql_command.length = strlen("change_user");
        break;
      default:
        break;
    }
  }

  /* Ignore all events without query. */
  if (0 == event_general->general_query.length) {
    return 0;
  }

  if (AUDIT_ALL == audit_mode ||
      (AUDIT_FILTER == audit_mode &&
       (in_filter(filter_ip, event_general->general_ip.str, DELIMITER) ||
        in_filter(filter_user, event_general->general_user.str, DELIMITER) ||
        in_filter(filter_db, event_general->general_db.str, DELIMITER)))) {
    normal_user.convert_to_json(event_general);
  }

  return 0;
}

/*
  Plugin type-specific descriptor
*/
static struct st_mysql_audit audit_txsql_descriptor= {
  MYSQL_AUDIT_INTERFACE_VERSION,                    /* interface version    */
  NULL,                                             /* release_thd function */
  audit_txsql_notify,                               /* notify function      */
  { (ulong) MYSQL_AUDIT_GENERAL_STATUS,
    (ulong) MYSQL_AUDIT_CONNECTION_ALL }
};

/*
  Plugin system variables.
*/
static const char* audit_mode_names[4] = {"ALL", "FILTER", "OFF",
                                          (const char *) 0};
static TYPELIB audit_mode_names_typelib = {
  array_elements(audit_mode_names)-1, "", audit_mode_names, NULL};


/** Validate passed-in "value" is a valid directory name.
@param[in,out]	thd	thread handle
@param[in]	var	pointer to system variable
@param[out]	save	immediate result for update
@param[in]	value	incoming string
@return 0 for valid name */
static int audit_directory_validate(THD *thd,
                                    SYS_VAR *var MY_ATTRIBUTE((unused)),
                                    void *save,
                                    struct st_mysql_value *value) {
  const char *alter_audit_dir;
  char buff[OS_FILE_MAX_PATH] = {0};
  int len = sizeof(buff);
  char audit_abs_path[FN_REFLEN + 2] = {0};
  char err_info[ERR_LEN] = {0};

  DBUG_ASSERT(save != NULL);
  DBUG_ASSERT(value != NULL);

  alter_audit_dir = (const char *)value->val_str(value, buff, &len);
  if (!alter_audit_dir) {
    *static_cast<const char **>(save) = nullptr;
    return (0);
  }

  if (strlen(alter_audit_dir) > FN_REFLEN) {
    sprintf(err_info, "Path=%s, Path length should not exceed %d bytes",
            alter_audit_dir, FN_REFLEN);
    audit_err_log(err_info);

    *static_cast<const char **>(save) = NULL;
    return (1);
  }

  my_realpath(audit_abs_path, alter_audit_dir, 0);
  size_t tmp_abs_len = strlen(audit_abs_path);

  if (my_access(audit_abs_path, F_OK)) {
    sprintf(err_info, "Path=%s, Path doesn't exist.", audit_abs_path);
    audit_err_log(err_info);

    *static_cast<const char **>(save) = NULL;
    return (1);
  } else if (my_access(audit_abs_path, R_OK | W_OK)) {
    sprintf(err_info, "Path=%s, Server doesn't have permission in the given "
                      "location,", audit_abs_path);
    audit_err_log(err_info);

    *static_cast<const char **>(save) = NULL;
    return (1);
  }

  if (audit_abs_path[tmp_abs_len - 1] != '/') {
    audit_abs_path[tmp_abs_len] = '/';
    tmp_abs_len++;
  }

  *static_cast<const char **>(save) =
    static_cast<char *>thd_memdup(thd, audit_abs_path, tmp_abs_len + 1);
  if (audit_mode == AUDIT_ALL || audit_mode == AUDIT_FILTER) {
    normal_user.audit_log_file_init(*static_cast<const char **>(save));
  }

  return (0);
}

static void update_rotate_write_log(MYSQL_THD thd MY_ATTRIBUTE((unused)),
                                    SYS_VAR *var MY_ATTRIBUTE((unused)),
                                    void *ptr,
                                    const void *val) {
  *static_cast<char *>(ptr) = *static_cast<const char *>(val);

  if (audit_mode == AUDIT_FILTER || audit_mode == AUDIT_ALL) {
    normal_user.audit_log_file_init(audit_dir);
  }
}

static void update_audit_mode(MYSQL_THD thd MY_ATTRIBUTE((unused)),
                              SYS_VAR *var MY_ATTRIBUTE((unused)),
                              void *ptr,
                              const void *val) {
  *static_cast<unsigned long*>(ptr) = *static_cast<const unsigned long*>(val);

  if (audit_mode == AUDIT_ALL || audit_mode == AUDIT_FILTER) {
    normal_user.audit_log_file_init(audit_dir);
  }
}

void audit_handler::deinit(void) {
  is_running = false;
  usleep(10000);

  if (thread_file) {
    pthread_join(*thread_file, NULL);
  }

  mysql_mutex_lock(&mem_lock);
  if (NULL != share_mem) {
    free(share_mem);
    share_mem = NULL;
  }
  mysql_mutex_unlock(&mem_lock);

  mysql_mutex_lock(&file_lock);
  if (log_file_fp != NULL) {
    fclose(log_file_fp);
    log_file_fp = NULL;
  }

  if (file_pos_fp != NULL) {
    fclose(file_pos_fp);
    file_pos_fp = NULL;
  }
  mysql_mutex_unlock(&file_lock);

  if (thread_file != NULL) {
    free(thread_file);
    thread_file = NULL;
  }

  mysql_mutex_destroy(&mem_lock);
  mysql_mutex_destroy(&file_lock);
}

static MYSQL_SYSVAR_ENUM(
  audit_mode,
  audit_mode,
  PLUGIN_VAR_RQCMDARG,
  "Audit mode. 'fliter' means audit related logs according to filtering rules. "
  "'all' means audit all logs",
  NULL,
  update_audit_mode,
  AUDIT_OFF,
  &audit_mode_names_typelib);

static MYSQL_SYSVAR_STR(
  filter_ip,
  filter_ip,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "Target ip to audit",
  NULL,
  NULL,
  NULL);

static MYSQL_SYSVAR_STR(
  filter_user,
  filter_user,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "Target user to audit",
  NULL,
  NULL,
  NULL);

static MYSQL_SYSVAR_STR(
  filter_db,
  filter_db,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "Target db to audit",
  NULL,
  NULL,
  NULL);

static MYSQL_SYSVAR_STR(
  audit_dir,
  audit_dir,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "Directory to store audit log",
  audit_directory_validate,
  NULL,
  mysql_real_data_home_ptr);

static MYSQL_SYSVAR_BOOL(
  rotate_write_log,
  rotate_write,
  PLUGIN_VAR_OPCMDARG,
  "Whether write the log rotately",
  nullptr,
  &update_rotate_write_log,
  0
);

static MYSQL_SYSVAR_UINT(truncate_length, trunc_len, PLUGIN_VAR_RQCMDARG,
                         "Trucate length for query", NULL, NULL,
                         2048,                        /* Default */
                         1024,                        /* Minimum */
                         1048576,                     /* Maximum */
                         1);                          /* Step    */

static MYSQL_SYSVAR_LONG(log_file_max_size, file_max_size, PLUGIN_VAR_RQCMDARG,
                         "Max file size for single audit log file", NULL, NULL,
                         512*1024*1024,               /* 512M Default */
                         8*1024*1024,                 /* 8M   Minimum */
                         10*1024*1024*1024LL,         /* 10G  Maximum */
                         1);                          /* Step    */

static MYSQL_SYSVAR_INT(rotate_file_count, rotate_count, PLUGIN_VAR_RQCMDARG,
                        "Rotate audit log file count", NULL, NULL,
                        5,                            /* Default */
                        1,                            /* Minimum */
                        10,                           /* Maximum */
                        1);                           /* Step    */

static const char *audit_log_safety_level_names[4] = {"normal", "safe",
                                                      "safest", NullS};
static TYPELIB audit_log_safety_level_typelib = {
    array_elements(audit_log_safety_level_names) - 1, "audit_log_safety_level",
    audit_log_safety_level_names, nullptr};

static MYSQL_SYSVAR_ENUM(
    log_safety_level, log_safety_level, PLUGIN_VAR_RQCMDARG,
    "Audit log safety level. 'normal' means recording audit log in high speed, "
    "but may generate garbled audit logs in extreme scenarios. 'safe' means "
    "the speed of recording audit logs is almost the same as 'normal', but the "
    "probability of generating garbled audit logs is reduced, and the "
    "probability of ignoring audit logs is increased. The probability can be "
    "controlled by 'log_safe_factor'. 'safest' means there is no probability "
    "to record garbled audit logs, but the performance of mysqld is greatly "
    "affected.", nullptr, nullptr, 0, &audit_log_safety_level_typelib);

static MYSQL_SYSVAR_INT(
    log_safe_factor, log_safe_factor, PLUGIN_VAR_RQCMDARG,
    "Parameter used when log_safety_level is equal to safe. Used to control "
    "the copy time of audit logs in memory, in milliseconds. The higher the "
    "value, the lower the risk of garbled audit logs and the higher the "
    "probability of ignoring audit logs.",
    nullptr, nullptr, 10, /* Default */
    5,                    /* Minimum */
    500,                  /* Maximum */
    1);                   /* Step    */

static SYS_VAR* audit_system_variables[] = {
  MYSQL_SYSVAR(audit_mode),
  MYSQL_SYSVAR(truncate_length),
  MYSQL_SYSVAR(filter_ip),
  MYSQL_SYSVAR(filter_user),
  MYSQL_SYSVAR(filter_db),
  MYSQL_SYSVAR(audit_dir),
  MYSQL_SYSVAR(log_file_max_size),
  MYSQL_SYSVAR(rotate_file_count),
  MYSQL_SYSVAR(rotate_write_log),
  MYSQL_SYSVAR(log_safety_level),
  MYSQL_SYSVAR(log_safe_factor),
  NULL
};

/*
  Plugin status variables for SHOW STATUS
*/
static SHOW_VAR audit_system_status[]= {
  { "audit_txsql_truncate_counts",
  reinterpret_cast<char *>(const_cast<ulonglong *>(&audit_handler::trunc_sql_counts)),
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
  { "audit_txsql_ignore_actions",
  reinterpret_cast<char *>(const_cast<ulonglong *>(&audit_handler::ignore_actions)),
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL},
  { 0, 0, SHOW_UNDEF, SHOW_SCOPE_GLOBAL}
};

/*
  Plugin library descriptor
*/
mysql_declare_plugin(audit_txsql) {
  MYSQL_AUDIT_PLUGIN,          /* type                            */
  &audit_txsql_descriptor,     /* descriptor                      */
  "AUDIT_TXSQL",               /* name                            */
  "Saviochen",                 /* author                          */
  "Audit support from txsql",  /* description                     */
  PLUGIN_LICENSE_GPL,
  audit_txsql_plugin_init,     /* init function (when loaded)     */
  NULL,                        /* check uninstall function        */
  audit_txsql_plugin_deinit,   /* deinit function (when unloaded) */
  0x0001,                      /* version                         */
  audit_system_status,         /* status variables                */
  audit_system_variables,      /* system variables                */
  NULL,
  PLUGIN_OPT_ALLOW_EARLY,
}
mysql_declare_plugin_end;

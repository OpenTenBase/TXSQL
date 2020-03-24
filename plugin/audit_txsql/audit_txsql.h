#ifndef HEAD_AUDIT_TXSQL
#define HEAD_AUDIT_TXSQL

#define GLOBAL_AUDIT_MEM_BLOCK  8388608     /* 8*1024*1024, 8MB */
#define FLUSH_SEND_BUFFER       2097152     /* 2*1024*1024, 2MB */
#define FLUSH_PER_SEC           1
#define POS_FILE                "pos_file"
#define POS_FILE_LEN            8

#define FORMAT_STRING_JSON(format_str,format_len,src_str,src_strlen)\
do {\
  memcpy(CUR_AUDIT_MEM_PTR, format_str, format_len); \
  mem_pos += format_len; \
  if (src_str) {\
    memcpy(CUR_AUDIT_MEM_PTR, src_str, src_strlen); \
    mem_pos += src_strlen; \
  }\
} while (0)

#define FORMAT_NUMBER_JSON(format_str, format_len, num, num_len, conver_func)\
do {\
  memcpy(CUR_AUDIT_MEM_PTR, format_str, format_len); \
  mem_pos += format_len; \
  conver_func(CUR_AUDIT_MEM_PTR + num_len, num, num_len); \
  mem_pos += num_len; \
} while (0)

/* max number 999999999 */
#define ATOI_CHECK(str)\
do {\
  char *tmp_str = (str); \
  int i = 0; \
  while (*(tmp_str + i)) {\
    if (i > 8 || *(tmp_str + i) < '0' || *(tmp_str + i) > '9') {\
      audit_err_log("string to number error,check the number stirng");\
      return TRUE; \
    }\
    i++; \
  }\
} while (0)

#define CHECK_FILE_PTR(cur_fp, cur_file_name)\
if (NULL == cur_fp) {\
  audit_err_log(cur_file_name);\
  audit_err_log("Failed to create audit log file, "\
                "check the Create File Permissions and Disk Space, retry ...");\
  usleep(1000000);\
  cur_fp = fopen(cur_file_name, "wb+");\
  DBUG_ASSERT(NULL != cur_fp);\
}

#define CLAC_ULOGN_LEN(total_len, format_len, cur_len, value)\
do\
{\
total_len += format_len;\
cur_len = longlong_len(&value);\
total_len += cur_len;\
}while(0)

/**
  Information about current file position.
  count:the log file name,is a number,
  pos: have written the file location.
*/
typedef struct file_pos {
  int count;
  int pos;
} file_pos;

static int  longlong_len(ulonglong *num);
static int  int_len(int *num);
static void audit_err_log(const char *info);

typedef struct share_mem_head {
  int write_pos;
  int flush_pos;
  int end_pos;
} share_mem_head;

#define SHARE_MEM_HEAD_SIZE 16                          /* sizeof(share_mem_head) */
#define CUR_AUDIT_MEM_PTR   (this->share_mem + mem_pos) /* current memory position */

/**
  audit_handler
  An abstract handler for different users, such as normal user, super user and
  so on.
*/
class audit_handler
{
  public:

    /**
      Total times of ignoring audit event when write pressure is too high.
    */
    static volatile ulonglong ignore_actions;

    /**
      Total number of truncated long querys.
    */
    static volatile ulonglong trunc_sql_counts;

    mysql_mutex_t mem_lock;
    mysql_mutex_t file_lock;

    char *share_mem;
    file_pos fps;
    FILE *log_file_fp;
    FILE *file_pos_fp;
    time_t last_flush;
    pthread_t *thread_file;
    volatile char is_running;

    audit_handler() : share_mem(nullptr),
                      fps({0,0}),
                      log_file_fp(nullptr),
                      file_pos_fp(nullptr),
                      last_flush(0),
                      thread_file(nullptr),
                      is_running(false) {}
    /**
      mem_and_thread_init
      init the memory and thread.
    */
    void mem_and_thread_init(void);

    /**
      convert_to_json
      The statement to be recorded into json format, save to memory
      @param[in] event_general source data.
    */
    void convert_to_json(struct mysql_event_general *event_general);

    /**
      save_to_file
      save data to file.
      @param[in] mem data mem buffer
      @param[in] len data len
      @return[in] false on success, true on failure
    */
    bool save_to_file(char *mem, int len);

    /**
      seek_file_pos
      check and reset the file pos
      @param[in] log_name file name
      @param[in] fps file number
      @return false on success, true on failure.
    */
    bool seek_file_pos(char *log_name, file_pos *fps);

    /**
      audit_log_file_init
      When this function returns false, the file_pos_fp
      and log_file_fp are available.
    */
    void audit_log_file_init(const char *new_dir);

    /**
     init
     initialization for handler.
    */
    void init();

    /**
     deinit
     deinitialization for handler.
    */
    void deinit();
};

#define FORMAT_STR_LEN_part1 60 /* strlen("{\"timestamp\":,\"affectRows\":,\"execTime\":,\"errCode\":,\"host\":\"\"") */
#define FORMAT_STR_LEN_part2 46 /* strlen(",\"user\":\"\",\"dbName\":\"\",\"sql\":\"\",\"sqlType\":\"\"}\n") */
#define FORMAT_SQL_TYPE_LEN  12 /* strlen(",\"sqlType\":\"") */
#define FORMAT_TIMESTAMP_LEN 13 /* strlen("{\"timestamp\":") */
#define FORMAT_THREAD_LEN    12 /* strlen(",\"threadId\":") */
#define FORMAT_CHECK_ROW_LEN 13 /* strlen(",\"checkRows\":") */
#define FORMAT_SENT_ROW_LEN  12 /* strlen(",\"sentRows\":") */
#define FORMAT_LOCK_WAIT_LEN 16 /* strlen(",\"lockWaitTime\":") */
#define FORMAT_CPU_TIME_LEN  11 /* strlen(",\"cpuTime\":") */
#define FORMAT_IO_WAIT_LEN   14 /* strlen(",\"ioWaitTime\":") */
#define FORMAT_NS_TIME_LEN   10 /* strlen(",\"nsTime\":") */
#define FORMAT_TRX_TIME_LEN  17 /* strlen(",\"trxLivingTime\":") */
#define FORMAT_AFFROW_LEN    14 /* strlen(",\"affectRows\":") */
#define FORMAT_EXECTIME_LEN  12 /* strlen(",\"execTime\":") */
#define FORMAT_ERROR_LEN     11 /* strlen(",\"errCode\":") */
#define FORMAT_RULE_LEN      11 /* strlen(",\"ruleNum\":") */
#define FORMAT_IP_LEN        9  /* strlen(",\"host\":\"") */
#define FORMAT_USERNAME_LEN  10 /* strlen("\",\"user\":\"") */
#define FORMAT_DBNAME_LEN    12 /* strlen("\",\"dbName\":\"") */
#define FORMAT_POLICY_LEN    16 /* strlen("\",\"policyName\":\"") */
#define FORMAT_SQLTEXT_LEN   9  /* strlen("\",\"sql\":\"")*/
#define FORMAT_END_LEN       3  /* strlen("\"}\n") */

static void num2str(char *dest, int num, int len);
static void longnum2str(char *dest, ulonglong num, int len);

static inline int longlong_len(ulonglong *num) {
  /* abnormal, set length to 0 */
  if (*num > 9999999999999999L) {
    *num = 0;
    return 1;
  } else if (*num > 999999999999999L) {
    return 16;
  } else if (*num > 99999999999999L) {
    return 15;
  } else if (*num > 9999999999999L) {
    return 14;
  } else if (*num > 999999999999L) {
    return 13;
  } else if (*num > 99999999999L) {
    return 12;
  } else if (*num > 9999999999L) {
    return 11;
  } else if (*num > 999999999L) {
    return 10;
  } else if (*num > 99999999L) {
    return 9;
  } else if (*num > 9999999L) {
    return 8;
  } else if (*num > 999999L) {
    return 7;
  } else if (*num > 99999L) {
    return 6;
  } else if (*num > 9999L) {
    return 5;
  } else if (*num > 999L) {
    return 4;
  } else if (*num > 99L) {
    return 3;
  } else if (*num > 9L) {
    return 2;
  } else {
    return 1;
  }
}

static inline int int_len(int *num) {
  /* abnormal, set length to 0 */
  if (*num > 999999999) {
    *num = 0;
    return 1;
  } else if (*num > 99999999) {
    return 9;
  } else if (*num > 9999999) {
    return 8;
  } else if (*num > 999999) {
    return 7;
  } else if (*num > 99999) {
    return 6;
  } else if (*num > 9999) {
    return 5;
  } else if (*num > 999) {
    return 4;
  } else if (*num > 99) {
    return 3;
  } else if (*num > 9) {
    return 2;
  } else if (*num >= 0) {
    return 1;
  }

  /* abnormal, set length to 0 */
  *num = 0;
  return 1;
}

#endif

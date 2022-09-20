/*****************************************************************************

Copyright (c) 2011, 2022, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file buf/buf0dump.cc
 Implements a buffer pool dump/load.

 Created April 08, 2011 Vasil Dimov
 *******************************************************/

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <algorithm>

#include "buf0buf.h"
#include "buf0dump.h"
#include "dict0dict.h"

#include "my_io.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "my_thread.h"
#include "mysql/psi/mysql_stage.h"
#include "os0file.h"
#include "os0thread-create.h"
#include "os0thread.h"
#include "srv0srv.h"
#include "srv0start.h"
#include "sync0rw.h"
#include "univ.i"
#include "ut0byte.h"

/* Changes from txsql start. */
#include "btr0btr.h"
#include "rem0rec.h"
#include "btr0pcur.h"
#include "dict0dd.h"
#include "bp_sync.h"
#include <boost/unordered/unordered_set.hpp>
#include <unistd.h>
#include <mysqld.h>

static bool buf_recover_abort_flag = false;

#define SNAPSHOT_LENGTH_FORMAT "%lu,%lu,%lu,%lu,%lu,%lu,"
const rec_t MAGIC_NUM = 0xFF;
const uint  MAGIC_LEN = 1;

/* Flages that tell the bufferpool snapshot/recover thread whitch action should
it take after being waked up. */
static bool buf_snapshot_should_start = false;
static bool buf_recover_should_start = false;

/** Wakes up the buffer pool snapshot/recover thread and instructs it to start
a dump. This function is called by MySQL code via buffer_pool_snapshot_now()
and it should return immediately because the whole MySQL is frozen during
its execution. */
void buf_snapshot_start() {
  buf_snapshot_should_start = true;
  os_event_set(srv_buf_synchronize_event);
}

/** Wakes up the buffer pool snapshot/recover thread and instructs it to start
a load. This function is called by MySQL code via buffer_pool_recover_now()
and it should return immediately because the whole MySQL is frozen during
its execution. */
void buf_recover_start() {
  buf_recover_should_start = true;
  os_event_set(srv_buf_synchronize_event);
}
/* Changes from txsql end. */

enum status_severity { STATUS_VERBOSE, STATUS_INFO, STATUS_ERR };

static inline bool SHUTTING_DOWN() {
  return srv_shutdown_state.load() >= SRV_SHUTDOWN_CLEANUP;
}

/* Flags that tell the buffer pool dump/load thread which action should it
take after being waked up. */
static bool buf_dump_should_start = false;
static bool buf_load_should_start = false;

static bool buf_load_abort_flag = false;

/* Used to temporary store dump info in order to avoid IO while holding
buffer pool LRU list mutex during dump and also to sort the contents of the
dump before reading the pages from disk during load.
We store the space id in the high 32 bits and page no in low 32 bits. */
typedef uint64_t buf_dump_t;

/* Aux macros to create buf_dump_t and to extract space and page from it */
inline uint64_t BUF_DUMP_CREATE(space_id_t space, page_no_t page) {
  return ut_ull_create(space, page);
}
constexpr space_id_t BUF_DUMP_SPACE(uint64_t a) {
  return static_cast<space_id_t>((a) >> 32);
}
constexpr page_no_t BUF_DUMP_PAGE(uint64_t a) {
  return static_cast<page_no_t>((a)&0xFFFFFFFFUL);
}
/** Wakes up the buffer pool dump/load thread and instructs it to start
 a dump. This function is called by MySQL code via buffer_pool_dump_now()
 and it should return immediately because the whole MySQL is frozen during
 its execution. */
void buf_dump_start() {
  buf_dump_should_start = true;
  os_event_set(srv_buf_dump_event);
}

/** Wakes up the buffer pool dump/load thread and instructs it to start
 a load. This function is called by MySQL code via buffer_pool_load_now()
 and it should return immediately because the whole MySQL is frozen during
 its execution. */
void buf_load_start() {
  buf_load_should_start = true;
  os_event_set(srv_buf_dump_event);
}

/** Sets the global variable that feeds MySQL's innodb_buffer_pool_dump_status
 to the specified string. The format and the following parameters are the
 same as the ones used for printf(3). The value of this variable can be
 retrieved by:
 SELECT variable_value FROM performance_schema.global_status WHERE
 variable_name = 'INNODB_BUFFER_POOL_DUMP_STATUS';
 or by:
 SHOW STATUS LIKE 'innodb_buffer_pool_dump_status'; */
static MY_ATTRIBUTE((format(printf, 2, 3))) void buf_dump_status(
    enum status_severity severity, /*!< in: status severity */
    const char *fmt,               /*!< in: format */
    ...)                           /*!< in: extra parameters according
                                   to fmt */
{
  va_list ap;

  va_start(ap, fmt);

  ut_vsnprintf(export_vars.innodb_buffer_pool_dump_status,
               sizeof(export_vars.innodb_buffer_pool_dump_status), fmt, ap);

  switch (severity) {
    case STATUS_INFO:
      ib::info(ER_IB_MSG_119) << export_vars.innodb_buffer_pool_dump_status;
      break;

    case STATUS_ERR:
      ib::error(ER_IB_MSG_120) << export_vars.innodb_buffer_pool_dump_status;
      break;

    case STATUS_VERBOSE:
      break;
  }

  va_end(ap);
}

/** Sets the global variable that feeds MySQL's innodb_buffer_pool_load_status
 to the specified string. The format and the following parameters are the
 same as the ones used for printf(3). The value of this variable can be
 retrieved by:
 SELECT variable_value FROM performance_schema.global_status WHERE
 variable_name = 'INNODB_BUFFER_POOL_LOAD_STATUS';
 or by:
 SHOW STATUS LIKE 'innodb_buffer_pool_load_status'; */
static MY_ATTRIBUTE((format(printf, 2, 3))) void buf_load_status(
    enum status_severity severity, /*!< in: status severity */
    const char *fmt,               /*!< in: format */
    ...)                           /*!< in: extra parameters according to fmt */
{
  va_list ap;

  va_start(ap, fmt);

  ut_vsnprintf(export_vars.innodb_buffer_pool_load_status,
               sizeof(export_vars.innodb_buffer_pool_load_status), fmt, ap);

  switch (severity) {
    case STATUS_INFO:
      ib::info(ER_IB_MSG_121) << export_vars.innodb_buffer_pool_load_status;
      break;

    case STATUS_ERR:
      ib::error(ER_IB_MSG_122) << export_vars.innodb_buffer_pool_load_status;
      break;

    case STATUS_VERBOSE:
      break;
  }

  va_end(ap);
}

/** Returns the directory path where the buffer pool dump file will be created.
@return directory path */
static const char *get_buf_dump_dir() {
  const char *dump_dir;

  /* The dump file should be created in the default data directory if
  innodb_data_home_dir is set as an empty string. */
  if (strcmp(srv_data_home, "") == 0) {
    dump_dir = MySQL_datadir_path;
  } else {
    dump_dir = srv_data_home;
  }

  return (dump_dir);
}

/** Generate the path to the buffer pool dump/load/snapshot/recover file.
@param[out]	path		generated path
@param[in]	path_size	size of 'path', used as in snprintf(3).
@param[in]	filename	real filename without directory. */
void buf_generate_path(char *path, size_t path_size, const char* filename) {
  char buf[FN_REFLEN];

  snprintf(buf, sizeof(buf), "%s%c%s", get_buf_dump_dir(), OS_PATH_SEPARATOR,
           filename);

  /* Use this file if it exists. */
  if (os_file_exists(buf)) {
    /* my_realpath() assumes the destination buffer is big enough
    to hold FN_REFLEN bytes. */
    ut_a(path_size >= FN_REFLEN);

    my_realpath(path, buf, 0);
  } else {
    /* If it does not exist, then resolve only srv_data_home
    and append filename to it. */
    char srv_data_home_full[FN_REFLEN];

    my_realpath(srv_data_home_full, get_buf_dump_dir(), 0);

    if (srv_data_home_full[strlen(srv_data_home_full) - 1] ==
        OS_PATH_SEPARATOR) {
      snprintf(path, path_size, "%s%s", srv_data_home_full,
               filename);
    } else {
      snprintf(path, path_size, "%s%c%s", srv_data_home_full, OS_PATH_SEPARATOR,
               filename);
    }
  }
}

/** Perform a buffer pool dump into the file specified by
innodb_buffer_pool_filename. If any errors occur then the value of
innodb_buffer_pool_dump_status will be set accordingly, see buf_dump_status().
The dump filename can be specified by (relative to srv_data_home):
SET GLOBAL innodb_buffer_pool_filename='filename';
@param[in]      obey_shutdown   quit if we are in a shutting down state */
static void buf_dump(bool obey_shutdown) {
#define SHOULD_QUIT() (SHUTTING_DOWN() && obey_shutdown)

  char full_filename[OS_FILE_MAX_PATH];
  char tmp_filename[OS_FILE_MAX_PATH + 11];
  char now[32];
  FILE *f;
  ulint i;
  int ret;

  buf_generate_path(full_filename, sizeof(full_filename),
                    srv_buf_dump_filename);

  snprintf(tmp_filename, sizeof(tmp_filename), "%s.incomplete", full_filename);

  buf_dump_status(STATUS_INFO, "Dumping buffer pool(s) to %s", full_filename);

  f = fopen(tmp_filename, "w");
  if (f == nullptr) {
    buf_dump_status(STATUS_ERR, "Cannot open '%s' for writing: %s",
                    tmp_filename, strerror(errno));
    return;
  }
  /* else */

  /* walk through each buffer pool */
  for (i = 0; i < srv_buf_pool_instances && !SHOULD_QUIT(); i++) {
    buf_pool_t *buf_pool;
    buf_dump_t *dump;

    buf_pool = buf_pool_from_array(i);

    /* obtain buf_pool LRU list mutex before allocate, since
    UT_LIST_GET_LEN(buf_pool->LRU) could change */
    mutex_enter(&buf_pool->LRU_list_mutex);

    size_t n_pages = UT_LIST_GET_LEN(buf_pool->LRU);

    /* skip empty buffer pools */
    if (n_pages == 0) {
      mutex_exit(&buf_pool->LRU_list_mutex);
      continue;
    }

    if (srv_buf_pool_dump_pct != 100) {
      ut_ad(srv_buf_pool_dump_pct < 100);

      n_pages = n_pages * srv_buf_pool_dump_pct / 100;

      if (n_pages == 0) {
        n_pages = 1;
      }
    }

    dump = static_cast<buf_dump_t *>(
        ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY, n_pages * sizeof(*dump)));

    if (dump == nullptr) {
      mutex_exit(&buf_pool->LRU_list_mutex);
      fclose(f);
      buf_dump_status(STATUS_ERR, "Cannot allocate %zu bytes: %s",
                      n_pages * sizeof(*dump), strerror(errno));
      /* leave tmp_filename to exist */
      return;
    }
    {
      size_t j{0};
      for (auto bpage : buf_pool->LRU) {
        if (n_pages <= j) break;
        ut_a(buf_page_in_file(bpage));

        dump[j++] = BUF_DUMP_CREATE(bpage->id.space(), bpage->id.page_no());
      }

      ut_a(j == n_pages);
    }

    mutex_exit(&buf_pool->LRU_list_mutex);

    for (size_t j = 0; j < n_pages && !SHOULD_QUIT(); j++) {
      ret = fprintf(f, SPACE_ID_PF "," PAGE_NO_PF "\n", BUF_DUMP_SPACE(dump[j]),
                    BUF_DUMP_PAGE(dump[j]));
      if (ret < 0) {
        ut::free(dump);
        fclose(f);
        buf_dump_status(STATUS_ERR, "Cannot write to '%s': %s", tmp_filename,
                        strerror(errno));
        /* leave tmp_filename to exist */
        return;
      }

      if (j % 128 == 0) {
        buf_dump_status(STATUS_VERBOSE,
                        "Dumping buffer pool " ULINTPF "/" ULINTPF
                        ", page %zu/%zu",
                        i + 1, srv_buf_pool_instances, j + 1, n_pages);
      }
    }

    ut::free(dump);
  }

  ret = fclose(f);
  if (ret != 0) {
    buf_dump_status(STATUS_ERR, "Cannot close '%s': %s", tmp_filename,
                    strerror(errno));
    return;
  }
  /* else */

  ret = unlink(full_filename);
  if (ret != 0 && errno != ENOENT) {
    buf_dump_status(STATUS_ERR, "Cannot delete '%s': %s", full_filename,
                    strerror(errno));
    /* leave tmp_filename to exist */
    return;
  }
  /* else */

  ret = rename(tmp_filename, full_filename);
  if (ret != 0) {
    buf_dump_status(STATUS_ERR, "Cannot rename '%s' to '%s': %s", tmp_filename,
                    full_filename, strerror(errno));
    /* leave tmp_filename to exist */
    return;
  }
  /* else */

  /* success */

  ut_sprintf_timestamp(now);

  buf_dump_status(STATUS_INFO, "Buffer pool(s) dump completed at %s", now);
}

/** Artificially delay the buffer pool loading if necessary. The idea of this
function is to prevent hogging the server with IO and slowing down too much
normal client queries.
@param[in,out]  last_check_time         milliseconds since epoch of the last
                                        time we did check if throttling is
                                        needed, we do the check every
                                        srv_io_capacity IO ops.
@param[in]      last_activity_count     activity count
@param[in]      n_io                    number of IO ops done since buffer
                                        pool load has started */
static inline void buf_load_throttle_if_needed(
    std::chrono::steady_clock::time_point *last_check_time,
    ulint *last_activity_count, ulint n_io) {
  if (n_io % srv_io_capacity < srv_io_capacity - 1) {
    return;
  }

  if (*last_check_time == std::chrono::steady_clock::time_point{} ||
      *last_activity_count == 0) {
    *last_check_time = std::chrono::steady_clock::now();
    *last_activity_count = srv_get_activity_count();
    return;
  }

  /* srv_io_capacity IO operations have been performed by buffer pool
  load since the last time we were here. */

  /* If no other activity, then keep going without any delay. */
  if (srv_get_activity_count() == *last_activity_count) {
    return;
  }

  /* There has been other activity, throttle. */

  const auto elapsed_time = std::chrono::steady_clock::now() - *last_check_time;

  /* Notice that elapsed_time is not the time for the last
  srv_io_capacity IO operations performed by BP load. It is the
  time elapsed since the last time we detected that there has been
  other activity. This has a small and acceptable deficiency, e.g.:
  1. BP load runs and there is no other activity.
  2. Other activity occurs, we run N IO operations after that and
     enter here (where 0 <= N < srv_io_capacity).
  3. last_check_time is very old and we do not sleep at this time, but
     only update last_check_time and last_activity_count.
  4. We run srv_io_capacity more IO operations and call this function
     again.
  5. There has been more other activity and thus we enter here.
  6. Now last_check_time is recent and we sleep if necessary to prevent
     more than srv_io_capacity IO operations per second.
  The deficiency is that we could have slept at 3., but for this we
  would have to update last_check_time before the
  "cur_activity_count == *last_activity_count" check and calling
  ut_time_monotonic_ms() that often may turn out to be too expensive. */

  if (elapsed_time < std::chrono::seconds{1}) {
    std::this_thread::sleep_for(std::chrono::seconds{1} - elapsed_time);
  }

  *last_check_time = std::chrono::steady_clock::now();
  *last_activity_count = srv_get_activity_count();
}

/** Perform a buffer pool load from the file specified by
 innodb_buffer_pool_filename. If any errors occur then the value of
 innodb_buffer_pool_load_status will be set accordingly, see buf_load_status().
 The dump filename can be specified by (relative to srv_data_home):
 SET GLOBAL innodb_buffer_pool_filename='filename'; */
static void buf_load() {
  char full_filename[OS_FILE_MAX_PATH];
  char now[32];
  FILE *f;
  buf_dump_t *dump;
  ulint dump_n;
  ulint total_buffer_pools_pages;
  ulint i;
  ulint space_id;
  ulint page_no;
  int fscanf_ret;

  /* Ignore any leftovers from before */
  buf_load_abort_flag = false;

  buf_generate_path(full_filename, sizeof(full_filename), srv_buf_dump_filename);

  buf_load_status(STATUS_INFO, "Loading buffer pool(s) from %s", full_filename);

  f = fopen(full_filename, "r");
  if (f == nullptr) {
    buf_load_status(STATUS_ERR, "Cannot open '%s' for reading: %s",
                    full_filename, strerror(errno));
    return;
  }
  /* else */

  /* First scan the file to estimate how many entries are in it.
  This file is tiny (approx 500KB per 1GB buffer pool), reading it
  two times is fine. */
  dump_n = 0;
  while (fscanf(f, ULINTPF "," ULINTPF, &space_id, &page_no) == 2 &&
         !SHUTTING_DOWN()) {
    dump_n++;
  }

  if (!SHUTTING_DOWN() && !feof(f)) {
    /* fscanf() returned != 2 */
    const char *what;
    if (ferror(f)) {
      what = "reading";
    } else {
      what = "parsing";
    }
    fclose(f);
    buf_load_status(STATUS_ERR,
                    "Error %s '%s',"
                    " unable to load buffer pool (stage 1)",
                    what, full_filename);
    return;
  }

  /* If dump is larger than the buffer pool(s), then we ignore the
  extra trailing. This could happen if a dump is made, then buffer
  pool is shrunk and then load is attempted. */
  total_buffer_pools_pages = buf_pool_get_n_pages() * srv_buf_pool_instances;
  if (dump_n > total_buffer_pools_pages) {
    dump_n = total_buffer_pools_pages;
  }

  if (dump_n != 0) {
    dump = static_cast<buf_dump_t *>(
        ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY, dump_n * sizeof(*dump)));
  } else {
    fclose(f);
    ut_sprintf_timestamp(now);
    buf_load_status(STATUS_INFO,
                    "Buffer pool(s) load completed at %s"
                    " (%s was empty)",
                    now, full_filename);
    return;
  }

  if (dump == nullptr) {
    fclose(f);
    buf_load_status(STATUS_ERR, "Cannot allocate " ULINTPF " bytes: %s",
                    (ulint)(dump_n * sizeof(*dump)), strerror(errno));
    return;
  }

  rewind(f);

  for (i = 0; i < dump_n && !SHUTTING_DOWN(); i++) {
    fscanf_ret = fscanf(f, ULINTPF "," ULINTPF, &space_id, &page_no);

    if (fscanf_ret != 2) {
      if (feof(f)) {
        break;
      }
      /* else */

      ut::free(dump);
      fclose(f);
      buf_load_status(STATUS_ERR,
                      "Error parsing '%s', unable"
                      " to load buffer pool (stage 2)",
                      full_filename);
      return;
    }

    if (space_id > UINT32_MASK || page_no > UINT32_MASK) {
      ut::free(dump);
      fclose(f);
      buf_load_status(STATUS_ERR,
                      "Error parsing '%s': bogus"
                      " space,page " ULINTPF "," ULINTPF " at line " ULINTPF
                      ","
                      " unable to load buffer pool",
                      full_filename, space_id, page_no, i);
      return;
    }

    dump[i] = BUF_DUMP_CREATE(space_id, page_no);
  }

  /* Set dump_n to the actual number of initialized elements,
  i could be smaller than dump_n here if the file got truncated after
  we read it the first time. */
  dump_n = i;

  fclose(f);

  if (dump_n == 0) {
    ut::free(dump);
    ut_sprintf_timestamp(now);
    buf_load_status(STATUS_INFO,
                    "Buffer pool(s) load completed at %s"
                    " (%s was empty)",
                    now, full_filename);
    return;
  }

  if (!SHUTTING_DOWN()) {
    std::sort(dump, dump + dump_n);
  }

  std::chrono::steady_clock::time_point last_check_time;
  ulint last_activity_cnt = 0;

  /* Avoid calling the expensive fil_space_acquire_silent() for each
  page within the same tablespace. dump[] is sorted by (space, page),
  so all pages from a given tablespace are consecutive. */
  space_id_t cur_space_id = BUF_DUMP_SPACE(dump[0]);
  fil_space_t *space = fil_space_acquire_silent(cur_space_id);
  page_size_t page_size(space ? space->flags : 0);

#ifdef HAVE_PSI_STAGE_INTERFACE
  PSI_stage_progress *pfs_stage_progress =
      mysql_set_stage(srv_stage_buffer_pool_load.m_key);
#endif /* HAVE_PSI_STAGE_INTERFACE */

  mysql_stage_set_work_estimated(pfs_stage_progress, dump_n);
  mysql_stage_set_work_completed(pfs_stage_progress, 0);

  for (i = 0; i < dump_n && !SHUTTING_DOWN(); i++) {
    /* space_id for this iteration of the loop */
    const space_id_t this_space_id = BUF_DUMP_SPACE(dump[i]);

    if (this_space_id != cur_space_id) {
      if (space != nullptr) {
        fil_space_release(space);
      }

      cur_space_id = this_space_id;
      space = fil_space_acquire_silent(cur_space_id);

      if (space != nullptr) {
        const page_size_t cur_page_size(space->flags);
        page_size.copy_from(cur_page_size);
      }
    }

    if (space == nullptr) {
      continue;
    }

    buf_read_page_background(page_id_t(this_space_id, BUF_DUMP_PAGE(dump[i])),
                             page_size, true, true);

    if (i % 64 == 63) {
      os_aio_simulated_wake_handler_threads();
    }

    /* Update the progress every 32 MiB, which is every Nth page,
    where N = 32*1024^2 / page_size. */
    static const ulint update_status_every_n_mb = 32;
    static const ulint update_status_every_n_pages =
        update_status_every_n_mb * 1024 * 1024 / page_size.physical();

    if (i % update_status_every_n_pages == 0) {
      buf_load_status(STATUS_VERBOSE, "Loaded " ULINTPF "/" ULINTPF " pages",
                      i + 1, dump_n);
      mysql_stage_set_work_completed(pfs_stage_progress, i);
    }

    if (buf_load_abort_flag) {
      if (space != nullptr) {
        fil_space_release(space);
      }
      buf_load_abort_flag = false;
      ut::free(dump);
      buf_load_status(STATUS_INFO, "Buffer pool(s) load aborted on request");
      /* Premature end, set estimated = completed = i and
      end the current stage event. */
      mysql_stage_set_work_estimated(pfs_stage_progress, i);
      mysql_stage_set_work_completed(pfs_stage_progress, i);
#ifdef HAVE_PSI_STAGE_INTERFACE
      mysql_end_stage();
#endif /* HAVE_PSI_STAGE_INTERFACE */
      return;
    }

    buf_load_throttle_if_needed(&last_check_time, &last_activity_cnt, i);
  }

  if (space != nullptr) {
    fil_space_release(space);
  }

  ut::free(dump);

  ut_sprintf_timestamp(now);

  buf_load_status(STATUS_INFO, "Buffer pool(s) load completed at %s", now);

  /* Make sure that estimated = completed when we end. */
  mysql_stage_set_work_completed(pfs_stage_progress, dump_n);
  /* End the stage progress event. */
#ifdef HAVE_PSI_STAGE_INTERFACE
  mysql_end_stage();
#endif /* HAVE_PSI_STAGE_INTERFACE */
}

/* Changes from txsql start. */

/** Sets the global variable that feeds MySQL's
innodb_buffer_pool_snapshot_status to the specified string. The format and the
following parameters are the same as the ones used for printf(3). The value of
this variable can be retrieved by: SELECT variable_value FROM
information_schema.global_status WHERE variable_name =
'INNODB_BUFFER_POOL_SNAPSHOT_STATUS'; or by: SHOW STATUS LIKE
'innodb_buffer_pool_snapshot_status'; */
static MY_ATTRIBUTE((format(printf, 2, 3))) void buf_snapshot_status(
    enum status_severity severity, /*!< in: status severity */
    const char *fmt,               /*!< in: format */
    ...)                           /*!< in: extra parameters according
                                   to fmt */
{
  va_list ap;

  va_start(ap, fmt);

  ut_vsnprintf(export_vars.innodb_buffer_pool_snapshot_status,
               sizeof(export_vars.innodb_buffer_pool_snapshot_status), fmt, ap);

  switch (severity) {
    case STATUS_INFO:
      ib::info(ER_INNODB_BUFFER_POOL_SNAPSHOT_STATUS_INFO)
          << export_vars.innodb_buffer_pool_snapshot_status;
      break;

    case STATUS_ERR:
      ib::error(ER_INNODB_BUFFER_POOL_SNAPSHOT_STATUS_ERR)
          << export_vars.innodb_buffer_pool_snapshot_status;
      break;

    case STATUS_VERBOSE:
      break;
  }

  va_end(ap);
}

/** Sets the global variable that feeds MySQL's
innodb_buffer_pool_recover_status to the specified string. The format and the
following parameters are the same as the ones used for printf(3). The value of
this variable can be retrieved by: SELECT variable_value FROM
information_schema.global_status WHERE variable_name =
'INNODB_BUFFER_POOL_RECOVER_STATUS'; or by: SHOW STATUS LIKE
'innodb_buffer_pool_recover_status'; */
static MY_ATTRIBUTE((format(printf, 2, 3))) void buf_recover_status(
    enum status_severity severity, /*!< in: status severity */
    const char *fmt,               /*!< in: format */
    ...)                           /*!< in: extra parameters according to fmt */
{
  va_list ap;

  va_start(ap, fmt);

  ut_vsnprintf(export_vars.innodb_buffer_pool_recover_status,
               sizeof(export_vars.innodb_buffer_pool_recover_status), fmt, ap);

  switch (severity) {
    case STATUS_INFO:
      ib::info(ER_INNODB_BUFFER_POOL_RECOVER_STATUS_INFO)
          << export_vars.innodb_buffer_pool_recover_status;
      break;

    case STATUS_ERR:
      ib::error(ER_INNODB_BUFFER_POOL_RECOVER_STATUS_ERR)
          << export_vars.innodb_buffer_pool_recover_status;
      break;

    case STATUS_VERBOSE:
      break;
  }

  va_end(ap);
}

struct record_t {
  /* Normal record consists of two parts:"extra" + "data",
  The record pointer is pointed to the begning of "data" usually.
  "rec" below points to different place in buffer pool snapshot/recover.
  1. Snapshot: To convey record from master to slave, "rec" points to the
  begining of "extra".
  2. Recover: To loacate a record to btree leaf page, "rec" points to the
  begining of "data". */
  rec_t *rec;
  ulint extra_len;
  ulint data_len;

  record_t() : rec(nullptr), extra_len(0), data_len(0) {}
};

/* one value range in an index. */
struct range_t {
  /* Table name. */
  std::string table_name;
  /* Index name. */
  std::string index_name;
  /* Left-most record of current range. */
  record_t left;
  /* Right-most record of current range. */
  record_t right;
};

/** Link adjacent pages in bufferpool and get left most record and right
most record from pages list. */
void link_page(uint space, boost::unordered_set<uint> &page_set,
               std::vector<range_t> &multi_ranges, mem_heap_t *heap) {
  boost::unordered_set<uint>::iterator iter;
  buf_block_t *block = nullptr;
  buf_block_t *left_block = nullptr;
  buf_block_t *right_block = nullptr;
  dict_index_t *index = nullptr;
  page_no_t next_left_page_no = 0;
  page_no_t next_right_page_no = 0;
  ulint *offsets = nullptr;
  rec_t *rec = nullptr;
  range_t range;
  mtr_t mtr;
  page_id_t left_page_id(0, 0);
  page_id_t right_page_id(0, 0);
  space_index_t index_id = 0;

  while (page_set.size() > 0) {
    iter = page_set.begin();
    mtr_start(&mtr);
    block = (buf_block_t *)buf_page_try_get(page_id_t(space, *iter),
                                            UT_LOCATION_HERE, &mtr);
    page_set.erase(*iter);

    /* We only care about btree leaf page. */
    if (block == nullptr ||
#ifdef UNIV_DEBUG
        /* Check inside buf_page_try_get was moved here. */
        block->page.file_page_was_freed || block->page.was_stale() ||
#endif
        fil_page_get_type(block->frame) != FIL_PAGE_INDEX ||
        btr_page_get_level(block->frame) != 0) {
      mtr_commit(&mtr);
      continue;
    }

    /* Save index id. */
    index_id = btr_page_get_index_id(block->frame);
    index_id_t page_index_id(block->page.id.space(), index_id);

    /* Set an original value to left_page_id and right_page_id. */
    left_page_id.reset(block->page.id.space(), block->page.id.page_no());
    right_page_id.reset(block->page.id.space(), block->page.id.page_no());

    next_left_page_no = btr_page_get_prev(block->frame, &mtr);
    next_right_page_no = btr_page_get_next(block->frame, &mtr);
    mtr_commit(&mtr);

    /* Search the left side. */
    while (page_set.find(next_left_page_no) != page_set.end()) {
      mtr_start(&mtr);
      block = (buf_block_t *)buf_page_try_get(
          page_id_t(space, next_left_page_no), UT_LOCATION_HERE, &mtr);
      page_set.erase(next_left_page_no);

      /* If the page is not in BP now, we won`t count it. */
      if (block == nullptr
#ifdef UNIV_DEBUG
          /* Check inside buf_page_try_get was moved here. */
          || block->page.file_page_was_freed || block->page.was_stale()
#endif
      ) {
        mtr_commit(&mtr);
        break;
      }

      /* Update left_page_id. */
      left_page_id.reset(block->page.id.space(), block->page.id.page_no());
      next_left_page_no = btr_page_get_prev(block->frame, &mtr);

      mtr_commit(&mtr);
    }

    /* Search the right side. */
    while (page_set.find(next_right_page_no) != page_set.end()) {
      mtr_start(&mtr);
      block = (buf_block_t *)buf_page_try_get(
          page_id_t(space, next_right_page_no), UT_LOCATION_HERE, &mtr);
      page_set.erase(next_right_page_no);

      /* If the page is not in BP now, we won`t count it. */
      if (block == nullptr
#ifdef UNIV_DEBUG
          /* Check inside buf_page_try_get was moved here. */
          || block->page.file_page_was_freed || block->page.was_stale()
#endif
      ) {
        mtr_commit(&mtr);
        break;
      }

      /* Update right_page_id. */
      right_page_id.reset(block->page.id.space(), block->page.id.page_no());
      next_right_page_no = btr_page_get_next(block->frame, &mtr);

      mtr_commit(&mtr);
    }

    mutex_enter(&dict_sys->mutex);
    index = const_cast<dict_index_t *>(dict_index_find(page_index_id));
    mutex_exit(&dict_sys->mutex);

    /* We skip the following three types of indexes:
    1. Not in cache
    2. Ibuf index
    3. Space of index != space of page. This may occur in
    "innodb_temporary" space. */
    if (index == nullptr || dict_index_is_ibuf(index) ||
        index->table->space != space) {
      continue;
    }

    range.table_name = index->table_name;
    range.index_name = (const char *)index->name;

    mtr_start(&mtr);
    left_block =
        (buf_block_t *)buf_page_try_get(left_page_id, UT_LOCATION_HERE, &mtr);

    /* If there is no user record in current page list. ignore it. */
    if (left_block == nullptr ||
#ifdef UNIV_DEBUG
        left_block->page.file_page_was_freed || left_block->page.was_stale() ||
#endif
        index_id != btr_page_get_index_id(left_block->frame) ||
        0 == page_header_get_field(left_block->frame, PAGE_N_RECS)) {
      mtr_commit(&mtr);
      continue;
    }

    /* Get first user record from left most leaf page. */
    rec = page_rec_get_next(page_get_infimum_rec(left_block->frame));
    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED,
                              UT_LOCATION_HERE, &heap);
    range.left.rec = rec_key_fields_copy(
        heap, rec, offsets, index, range.left.extra_len, range.left.data_len);
    mtr_commit(&mtr);

    mtr_start(&mtr);
    right_block =
        (buf_block_t *)buf_page_try_get(right_page_id, UT_LOCATION_HERE, &mtr);

    /* If there is no user record in current page list. ignore it. */
    if (right_block == nullptr ||
#ifdef UNIV_DEBUG
        right_block->page.file_page_was_freed ||
        right_block->page.was_stale() ||
#endif
        index_id != btr_page_get_index_id(right_block->frame) ||
        0 == page_header_get_field(right_block->frame, PAGE_N_RECS)) {
      mtr_commit(&mtr);
      continue;
    }

    /* Get last user record from right most leaf page. */
    rec = page_rec_get_prev(page_get_supremum_rec(right_block->frame));
    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED,
                              UT_LOCATION_HERE, &heap);
    range.right.rec = rec_key_fields_copy(
        heap, rec, offsets, index, range.right.extra_len, range.right.data_len);
    mtr_commit(&mtr);

    multi_ranges.push_back(range);
  }
}

/** Dump multi-ranges info to local file.
DATA FORMAT: num1,num2,num3,num4,num5,num6,table+index+left+right
num1: length of table name
num2: length of index name
num3: extra_len of left record
num4: data_len of left record
mum5: extra_len of right record
num6: data_len of right record
table: table name
index: index name
left: left-most record in a range
right: right-most record in a range

@return false	succeed
@return true	failed */
bool dump_multi_ranges(std::vector<range_t> &multi_ranges, FILE *f) {
  ulint size = 0;

  for (uint i = 0; i < multi_ranges.size(); i++) {
    range_t &range = multi_ranges[i];
    /* fprintf() returns a negative value if an output error occurs. */
    if (fprintf(f, SNAPSHOT_LENGTH_FORMAT "%s%s", range.table_name.size(),
                range.index_name.size(), range.left.extra_len,
                range.left.data_len, range.right.extra_len,
                range.right.data_len, range.table_name.c_str(),
                range.index_name.c_str()) < 0) {
      return (true);
    }

    /* fwrite() returns the number of members successfully written,
    if the return value is not equeal to the input number,
    an output error occurs. */
    /* Write left-most record. */
    size = range.left.extra_len + range.left.data_len;
    if (fwrite(range.left.rec, sizeof(rec_t), size, f) != size) {
      return (true);
    }

    /* Write right-most record. */
    size = range.right.extra_len + range.right.data_len;
    if (fwrite(range.right.rec, sizeof(rec_t), size, f) != size) {
      return (true);
    }

    /* Write a magic number to each range end. */
    if (fwrite(&MAGIC_NUM, sizeof(rec_t), MAGIC_LEN, f) != MAGIC_LEN) {
      return (true);
    }
  }

  /* fflush() returns zero to indicates success. */
  if (fflush(f)) {
    return (true);
  }

  return (false);
}

/** Perform a buffer pool snapshot into the file specified by
SNAPSHOT_FILENAME. If any errors occur then the value of
innodb_buffer_pool_snapshot_status will be set accordingly,
see buf_snapshot_status(). */
static void buf_snapshot(bool obey_shutdown) /*!< in: quit if we are in a
                                              shutting down state */
{
#define SHOULD_QUIT() (SHUTTING_DOWN() && obey_shutdown)

  char full_filename[OS_FILE_MAX_PATH];
  char tmp_filename[OS_FILE_MAX_PATH * 2];
  char start_time[32];
  char end_time[32];
  FILE *f;
  ulint i;
  int ret;

  ut_sprintf_timestamp(start_time);

  buf_generate_path(full_filename, sizeof(full_filename), SNAPSHOT_FILENAME);

  snprintf(tmp_filename, sizeof(tmp_filename), "%s.incomplete", full_filename);

  buf_snapshot_status(STATUS_INFO, "Start snapshotting buffer pool(s) to %s",
                      full_filename);

  f = fopen(tmp_filename, "w");
  if (f == nullptr) {
    buf_snapshot_status(STATUS_ERR, "Cannot open '%s' for writing: %s",
                        tmp_filename, strerror(errno));
    return;
  }
  /* else */

  /* Raw page info. */
  std::map<uint, boost::unordered_set<uint>> lru_maps;

  /* Step 1: walk through each buffer pool. All pages are classified
  by spaceid. */
  for (i = 0; i < srv_buf_pool_instances && !SHOULD_QUIT(); i++) {
    buf_pool_t *buf_pool;
    const buf_page_t *bpage;
    buf_dump_t *dump;
    ulint n_pages;
    ulint j;

    buf_pool = buf_pool_from_array(i);

    /* obtain buf_pool mutex before allocate, since
    UT_LIST_GET_LEN(buf_pool->LRU) could change */
    mutex_enter(&buf_pool->LRU_list_mutex);

    n_pages = UT_LIST_GET_LEN(buf_pool->LRU);

    /* Skip empty buffer pools */
    if (n_pages == 0) {
      mutex_exit(&buf_pool->LRU_list_mutex);
      continue;
    }

    if (srv_buffer_pool_snapshot_pct != 100) {
      ut_ad(srv_buffer_pool_snapshot_pct < 100);

      n_pages = n_pages * srv_buffer_pool_snapshot_pct / 100;

      if (n_pages == 0) {
        n_pages = 1;
      }
    }

    dump = static_cast<buf_dump_t *>(
        ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY, n_pages * sizeof(*dump)));

    if (dump == nullptr) {
      mutex_exit(&buf_pool->LRU_list_mutex);
      fclose(f);
      buf_snapshot_status(STATUS_ERR, "Cannot allocate " ULINTPF " bytes: %s",
                          (ulint)(n_pages * sizeof(*dump)), strerror(errno));
      return;
    }

    for (bpage = UT_LIST_GET_FIRST(buf_pool->LRU), j = 0;
         bpage != nullptr && j < n_pages;
         bpage = UT_LIST_GET_NEXT(LRU, bpage), j++) {
      ut_a(buf_page_in_file(bpage));

      dump[j] = BUF_DUMP_CREATE(bpage->id.space(), bpage->id.page_no());
    }

    ut_a(j == n_pages);

    mutex_exit(&buf_pool->LRU_list_mutex);

    for (j = 0; j < n_pages; j++) {
      boost::unordered_set<uint> &hash_set = lru_maps[BUF_DUMP_SPACE(dump[j])];
      hash_set.insert(BUF_DUMP_PAGE(dump[j]));
    }

    ut::free(dump);
  }

  buf_snapshot_status(STATUS_INFO,
                      "LRU list snapshot saved, Start dumping space by space.");

  mem_heap_t *heap = mem_heap_create(UNIV_PAGE_SIZE, UT_LOCATION_HERE);
  std::map<uint, boost::unordered_set<uint>>::iterator iter;
  iter = lru_maps.begin();

  while (iter != lru_maps.end()) {
    /* Step 2: Concatenate pages through a doubly linked list
    spaceid by spaceid. */
    uint space = iter->first;
    std::vector<range_t> multi_ranges;
    link_page(space, lru_maps[space], multi_ranges, heap);

    /* Step 3: Dump each value range spaceid by spaceid. */
    ret = dump_multi_ranges(multi_ranges, f);
    if (ret) {
      fclose(f);
      mem_heap_free(heap);
      buf_snapshot_status(STATUS_ERR, "Cannot write to '%s': %s", tmp_filename,
                          strerror(errno));
      return;
    }

    iter++;
  }

  mem_heap_free(heap);
  ret = fclose(f);
  if (ret != 0) {
    buf_snapshot_status(STATUS_ERR, "Cannot close '%s': %s", tmp_filename,
                        strerror(errno));
    return;
  }
  /* else */

  ret = unlink(full_filename);
  if (ret != 0 && errno != ENOENT) {
    buf_snapshot_status(STATUS_ERR, "Cannot delete '%s': %s", full_filename,
                        strerror(errno));
    /* leave tmp_filename to exist */
    return;
  }
  /* else */

  ret = rename(tmp_filename, full_filename);
  if (ret != 0) {
    buf_snapshot_status(STATUS_ERR, "Cannot rename '%s' to '%s': %s",
                        tmp_filename, full_filename, strerror(errno));
    /* leave tmp_filename to exist */
    return;
  }
  /* else */

  /* success */

  ut_sprintf_timestamp(end_time);
  buf_snapshot_status(STATUS_INFO,
                      "Buffer pool(s) snapshot started at %s, completed at %s",
                      start_time, end_time);
  mysql_mutex_lock(&LOCK_transmit_client_access);
  global_transmit_client.clear();
  mysql_mutex_unlock(&LOCK_transmit_client_access);
}

void insert_page_id(boost::unordered_set<ulonglong> &pages_set,
                    page_id_t page_id) {
  ulonglong page_id_num = page_id.space();
  page_id_num = (page_id_num << 32) + page_id.page_no();
  pages_set.insert(page_id_num);
}

/** Perform a buffer pool recover from the file specified by
SNAPSHOT_FILENAME. If any errors occur then the value of
innodb_buffer_pool_recover_status will be set accordingly. */
static void buf_recover() {
  char full_filename[OS_FILE_MAX_PATH];
  char start_time[32];
  char end_time[32];
  FILE *f = nullptr;
  int fscanf_ret = 0;
  lint file_len = 0;
  lint cur_pos = 0;
  boost::unordered_set<ulonglong> pages_set;
  ulint pages_limit = (srv_buf_pool_size * srv_buffer_pool_recover_pct) /
                      (UNIV_PAGE_SIZE * 100);
  THD *thd = create_thd(false, true, true, 0, 0);

  if (thd == nullptr) {
    buf_recover_status(STATUS_ERR, "THD create failed");
    return;
  }

  ut_sprintf_timestamp(start_time);

  /* Ignore any leftovers from before */
  buf_recover_abort_flag = false;

  buf_generate_path(full_filename, sizeof(full_filename), SNAPSHOT_FILENAME);

  buf_recover_status(STATUS_INFO, "Recovering buffer pool(s) from %s",
                     full_filename);

  f = fopen(full_filename, "r");
  if (f == nullptr) {
    buf_recover_status(STATUS_ERR, "Cannot open '%s' for reading: %s",
                       full_filename, strerror(errno));
    return;
  }

  if (fseek(f, 0L, SEEK_END) || (file_len = ftell(f)) < 0 ||
      fseek(f, 0L, SEEK_SET)) {
    buf_recover_status(STATUS_ERR, "Cannot get '%s' size", full_filename);
    fclose(f);
    return;
  }

  /* Variables for parsing range. */
  record_t left, right;
  ulint table_len = 0;
  ulint index_len = 0;
  char table_name[NAME_LEN + 1] = "";
  char index_name[NAME_LEN + 1] = "";
  dict_table_t *table = nullptr;
  dict_index_t *index = nullptr;
  buf_block_t *left_block = nullptr;
  buf_block_t *right_block = nullptr;
  std::map<std::string, dict_index_t *> index_map;

  /* Variables for locating record to btree leaf page and loading pages. */
  mtr_t mtr;
  btr_pcur_t pcur;
  page_id_t left_page_id(0, 0);
  page_id_t right_page_id(0, 0);
  page_id_t next_page_id(0, 0);
  uint next_page_no = 0;
  dtuple_t *tuple = nullptr;
  void *buf = nullptr;
  mem_heap_t *heap = mem_heap_create(UNIV_PAGE_SIZE, UT_LOCATION_HERE);

  if (!heap) {
    buf_recover_status(STATUS_ERR, "Cannot create mem_heap in buf_reocver");
    goto free_resource;
  }

  do {
    fscanf_ret = fscanf(f, SNAPSHOT_LENGTH_FORMAT, &table_len, &index_len,
                        &left.extra_len, &left.data_len, &right.extra_len,
                        &right.data_len);

    if (fscanf_ret != 6 || table_len > NAME_LEN || index_len > NAME_LEN) {
      if (feof(f)) {
        /* Normal end. */
        ut_sprintf_timestamp(end_time);
        buf_recover_status(
            STATUS_INFO,
            "Buffer pool(s) recover started at %s, completed at %s", start_time,
            end_time);
        break;
      }
      /* else */
      buf_recover_status(STATUS_ERR,
                         "Error parsing '%s', unable to get data length "
                         "or table/index name length not right",
                         full_filename);
      goto free_resource;
    }

    uint total_len = table_len + index_len + left.extra_len + left.data_len +
                     right.extra_len + right.data_len;
    buf = mem_heap_alloc(heap, total_len);

    if (fread(buf, sizeof(rec_t), total_len, f) != total_len) {
      buf_recover_status(STATUS_ERR, "Error parsing '%s', unable to get data",
                         full_filename);
      goto free_resource;
    }

    rec_t magic = 0;
    if (fread(&magic, sizeof(rec_t), MAGIC_LEN, f) != MAGIC_LEN ||
        magic != MAGIC_NUM) {
      buf_recover_status(STATUS_ERR,
                         "Error parsing '%s', unable to get magic number "
                         "or magic number not right",
                         full_filename);
      goto free_resource;
    }

    /* Parse data. */
    memcpy(table_name, (char *)buf, table_len);
    table_name[table_len] = '\0';
    memcpy(index_name, (char *)buf + table_len, index_len);
    index_name[index_len] = '\0';

    left.rec = (rec_t *)buf + table_len + index_len + left.extra_len;
    right.rec = (rec_t *)buf + table_len + index_len + left.extra_len +
                left.data_len + right.extra_len;

    /* Get new table if needed. */
    if (table == nullptr || 0 != strcmp(table->name.m_name, table_name)) {
      /* Close old table. */
      if (table) dict_table_close(table, false, false);

      table = dd_table_open_on_name(thd, nullptr, table_name, false,
                                    DICT_ERR_IGNORE_NONE);

      if (table == nullptr || table->to_be_dropped) {
        buf_recover_status(STATUS_INFO,
                           "Failed loading table:%s, ignore it and continue",
                           table_name);
        continue;
      }

      index_map.clear();
      for (index = table->first_index(); index != nullptr;
           index = index->next()) {
        index_map[(const char *)index->name] = index;
      }
    }

    if (index_map.find(index_name) == index_map.end()) {
      buf_recover_status(
          STATUS_INFO,
          "Failed loading index:%s in table:%s, ignore it and continue",
          index_name, table_name);
      continue;
    }

    index = index_map[index_name];

    /* Locate left record to btree leaf page. */
    tuple = dict_index_build_node_ptr(index, left.rec, 0, heap, 0);

    mtr_start(&mtr);
    pcur.open_on_user_rec(index, tuple, PAGE_CUR_GE, BTR_SEARCH_LEAF, &mtr,
                          UT_LOCATION_HERE);

    left_block = pcur.m_btr_cur.page_cur.block;
    pcur.close();

    /* TODO: Error handling when record does not exist. */
    ut_ad(left_block);

    /* Save left page id to prevent page id change after mtr commit. */
    left_page_id.reset(left_block->page.id.space(),
                       left_block->page.id.page_no());
    insert_page_id(pages_set, left_page_id);
    next_page_no = btr_page_get_next(left_block->frame, &mtr);
    /* Left record and right record may in the same page. to avoid latching
    the same block, we commit mtr here. */
    mtr_commit(&mtr);

    /* Locate right record to btree leaf page. */
    tuple = dict_index_build_node_ptr(index, right.rec, 0, heap, 0);

    mtr_start(&mtr);
    pcur.open_on_user_rec(index, tuple, PAGE_CUR_GE, BTR_SEARCH_LEAF, &mtr,
                          UT_LOCATION_HERE);
    right_block = pcur.m_btr_cur.page_cur.block;
    pcur.close();

    /* TODO: Error handling when record does not exist. */
    ut_ad(right_block);

    /* Save the right_page_id for the page_id comparison. */
    right_page_id.reset(right_block->page.id.space(),
                        right_block->page.id.page_no());
    insert_page_id(pages_set, right_page_id);

    /* Release right_block to avoid dead-lock. */
    mtr_commit(&mtr);

    /* Check if we reach pages limit. */
    if (pages_set.size() >= pages_limit) {
      buf_recover_status(STATUS_INFO,
                         "Buffer pool(s) recover "
                         "aborted because it reached the pages limit");
      goto free_resource;
    }

    if (left_page_id == right_page_id) {
      /* Left record and right record are in the same pace.
      Finished recovering this round.*/
      continue;
    }

    /* Load pages between left block and right block. */
    page_id_t page_id(left_page_id.space(), 0);
    page_size_t page_size(right_block->page.size.physical(),
                          right_block->page.size.logical(),
                          right_block->page.size.is_compressed());

    while (FIL_NULL != next_page_no &&
           next_page_no != right_page_id.page_no()) {
      page_id.set_page_no(next_page_no);
      /* Load page to the start of LRU. */
      mtr_start(&mtr);
      buf_read_page_background(page_id, page_size, true, false);
      buf_block_t *block = btr_block_get(page_id, page_size, RW_S_LATCH,
                                         UT_LOCATION_HERE, nullptr, &mtr);
      /* Abort loading pages when errors occur. */
      if (block == nullptr) {
        mtr_commit(&mtr);
        break;
      }

      next_page_no = btr_page_get_next(block->frame, &mtr);
      mtr_commit(&mtr);

      /* Check if we reach pages limit. */
      insert_page_id(pages_set, page_id);
      if (pages_set.size() >= pages_limit) {
        buf_recover_status(
            STATUS_INFO,
            "Buffer pool(s) recover "
            "aborted because it reached the pages limit, %f%% finished",
            (float)(cur_pos * 100) / (float)file_len);
        goto free_resource;
      }
    }

    if ((cur_pos = ftell(f)) < 0) {
      buf_recover_status(STATUS_ERR, "Cannot get current position of '%s'",
                         full_filename);
      goto free_resource;
    }

    buf_recover_status(STATUS_VERBOSE, "Buffer pool(s) recover Finished %f%%",
                       (float)(cur_pos * 100) / (float)file_len);

    /* Terminate if needed. */
    if (buf_recover_abort_flag) {
      buf_recover_abort_flag = false;
      buf_recover_status(
          STATUS_INFO,
          "Buffer pool(s) recover aborted on request, %f%% finished",
          (float)(cur_pos * 100) / (float)file_len);
      goto free_resource;
    }
  } while (true);

free_resource:
  if (table) dict_table_close(table, false, false);
  if (heap) mem_heap_free(heap);
  if (f) fclose(f);
  if (thd) destroy_thd(thd);
}

/** Aborts a currently running buffer pool recover. This function is called by
MySQL code via buffer_pool_recover_abort() and it should return immediately
because the whole MySQL is frozen during its execution. */
void buf_recover_abort() { buf_recover_abort_flag = true; }

/** This is the main thread for buffer pool snapshot/recover. It waits for an
event and when waked up either performs a snapshot/recover and sleeps
again. */
void buf_synchronize_thread() {
  ut_ad(!srv_read_only_mode);

  buf_snapshot_status(STATUS_VERBOSE, "Snapshoting of buffer pool not started");
  buf_recover_status(STATUS_VERBOSE, "Recovering of buffer pool not started");

  while (!SHUTTING_DOWN()) {
    os_event_wait(srv_buf_synchronize_event);

    if (buf_snapshot_should_start) {
      buf_snapshot_should_start = false;
      buf_snapshot(true /* quit on shutdown */);
    }

    if (buf_recover_should_start) {
      buf_recover_should_start = false;
      buf_recover();
    }

    os_event_reset(srv_buf_synchronize_event);
  }
}

/* Changes from txsql end. */

/** Aborts a currently running buffer pool load. This function is called by
 MySQL code via buffer_pool_load_abort() and it should return immediately
 because the whole MySQL is frozen during its execution. */
void buf_load_abort() { buf_load_abort_flag = true; }

/** This is the main thread for buffer pool dump/load. It waits for an
event and when waked up either performs a dump or load and sleeps
again. */
void buf_dump_thread() {
  ut_ad(!srv_read_only_mode);

  buf_dump_status(STATUS_VERBOSE, "Dumping of buffer pool not started");
  buf_load_status(STATUS_VERBOSE, "Loading of buffer pool not started");

  if (srv_buffer_pool_load_at_startup) {
    buf_load();
  }

  while (!SHUTTING_DOWN()) {
    os_event_wait(srv_buf_dump_event);

    if (buf_dump_should_start) {
      buf_dump_should_start = false;
      buf_dump(true /* quit on shutdown */);
    }

    if (buf_load_should_start) {
      buf_load_should_start = false;
      buf_load();
    }

    os_event_reset(srv_buf_dump_event);
  }

  if (srv_buffer_pool_dump_at_shutdown && srv_fast_shutdown != 2) {
    buf_dump(false /* ignore shutdown down flag,
                keep going even if we are in a shutdown state */);
  }
}

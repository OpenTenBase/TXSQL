/*
 * rpl_slave_ack_thread.cpp
 *
 *  Created on: 2021/4/23
 *      Author: harlylei
 */

#include "rpl_slave_ack_thread.h"
#include "thd_bottom_half.h"

#include "include/my_dbug.h"
#include "log.h"

#include "include/compression.h"
#include "include/mutex_lock.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/psi_memory_bits.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysql/status_var.h"
#include "sql/rpl_channel_service_interface.h"

#include "sql_class.h"
#include "rpl_mi.h"
#include "rpl_msr.h"

extern Multisource_info channel_map;
extern ulonglong sqlasyn_sendto_master;

#ifndef LOG_SUBSYSTEM_TAG
#define LOG_SUBSYSTEM_TAG "rpl_slave_ack_thread"
#endif

extern int flush_master_info(Master_info *mi, bool force, bool need_lock = true,
    bool flush_relay_log = true);

void rpl_slave_ack_thread::flush_and_ack(
    const Thd_Trans_binlog_info &binlog_info) {

//  sql_print_information("get Thd_Trans_binlog_info: %lu,%lu",
//      binlog_info.file_no(), binlog_info.pos());

  channel_map.rdlock();

  Master_info *mi = channel_map.get_default_channel_mi();
  if (likely(mi)) {

    Relay_log_info *rli = mi->rli;
    bool error = rli->relay_log.after_write_to_relay_log(mi, true, true); //write relay log to file and fsync

//    mysql_mutex_lock(mi->rli->relay_log.get_log_lock());
    if (likely(!error)) {
      mysql_mutex_lock(&mi->data_lock);
      if (flush_master_info(mi, true, false, false)) {

        mysql_mutex_unlock(&mi->data_lock);
        //      mysql_mutex_unlock(mi->rli->relay_log.get_log_lock());

        error = true;
        sql_print_error("rpl_slave_ack_thread::flush_and_ack Failed to flush master info");
      } else {

        mysql_mutex_unlock(&mi->data_lock);
        //      mysql_mutex_unlock(mi->rli->relay_log.get_log_lock());

        BinlogPosAns ans;
        char binlog_name[256];
        int name_len = snprintf(binlog_name, sizeof(binlog_name),
            "binlog.%06lu", binlog_info.file_no());
        ans.setFileName(binlog_name, name_len);
        ans.log_pos = binlog_info.pos();
        ans.set_server_id(server_id);

        ans.computeLen();
        mi->sendAnsToMaster(ans);
        ++sqlasyn_sendto_master;
      }
    }

    DBUG_EXECUTE_IF("rpl_slave_ack_thread_flush_and_ack_fail",
            {
                    error = true;
                    DBUG_SET_INITIAL("-d,rpl_slave_ack_thread_flush_and_ack_fail");
            }
    );

    if (unlikely(error)) { //we should stop io slave
      sql_print_error("rpl_slave_ack_thread::flush_and_ack get error,so we should stop io thread when get next binlog");
      set_fail_next_check(true);//notify io thread exit
    }

  }

  channel_map.unlock();
}

void rpl_slave_ack_thread::run() {

  my_thread_init();

  Thd_Trans_binlog_info last_binlog_info;
  while (!m_exit.load()) {

    while (m_queue.pop(last_binlog_info)) { //loop,get last binlog_info
      ; //
    }
    //get last binlog_info
    if (last_binlog_info.is_valid()) {
      flush_and_ack(last_binlog_info);
      last_binlog_info.reset();
    }
    m_queue.loop_pop(last_binlog_info, 10); //may be block 10 msec
//    m_queue.loop_pop(last_binlog_info, 1000 * 3); //may be block 3 msec,just for test
#if !defined(DBUG_OFF)
    debug_stop_check();
#endif
  }

  my_thread_end();

}

#if !defined(DBUG_OFF)
void rpl_slave_ack_thread::debug_stop_check() {
  DBUG_EXECUTE_IF("rpl_slave_ack_thread_sleep",{
          while(!m_exit.load()) {
            sleep(1);
            if(DBUG_EVALUATE_IF("rpl_slave_ack_thread_run",true,false)) {
              break;
            }
            /*sql_print_information("we will sleep");  don't remove this line,sometimes we will print this information for test*/
          }
          DBUG_SET_INITIAL("-d,rpl_slave_ack_thread_sleep");
          DBUG_SET_INITIAL("-d,rpl_slave_ack_thread_run");
          _db_reset_cur_thread_setting_point_global_setting();
          /*sql_print_information("we have break sleep");*/
  };);
}
#endif

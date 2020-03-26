/* Copyright (c) 2015, 2020, Tencent.
Use is subject to license terms

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "rpl_speed_limit_plugin_vars.h"
#include "sql/sql_class.h"                          // THD

extern RplSpeedMonitor  speed_monitor;

int rpl_speed_limit_binlog_dump_start(Binlog_transmit_param *param,
                                      const char *,
                                      my_off_t) {
  THD* thd = current_thd;
  if (speed_monitor.addSlave(thd)) {
    param->set_dont_observe_flag();
    return 1;
  } else {
    /* Tell server it will observe the transimission. */
    param->set_observe_flag();
    return 0;
  }
}

int rpl_speed_limit_binlog_dump_end(Binlog_transmit_param *) {
  THD* thd = current_thd;
  speed_monitor.removeSlave(thd);
  return 0;
}


int rpl_speed_limit_before_send_event(Binlog_transmit_param *,
                                      unsigned char *, unsigned long len,
                                      const char *, my_off_t) {
  THD* thd = current_thd;
  if (speed_monitor.controlSpeed(thd, len)) {
    return 1;
  } else {
    return 0;
  }
}

Binlog_transmit_observer transmit_observer = {
  sizeof(Binlog_transmit_observer),   // len
  rpl_speed_limit_binlog_dump_start,  // start
  rpl_speed_limit_binlog_dump_end,    // stop
  NULL,                               // reserve_header
  rpl_speed_limit_before_send_event,  // before_send_event
  NULL,                               // after_send_event
  NULL,                               // reset
};

static int rpl_speed_limit_master_plugin_init(void *p) {
#ifdef HAVE_PSI_INTERFACE
  init_psi_keys();
#endif
  speed_monitor.init();

  if (register_binlog_transmit_observer(&transmit_observer, p))
    return 1;

  sql_print_information("register speed limit master plugin OK");
  return 0;
}

static int rpl_speed_limit_master_plugin_deinit(void *p) {
  if (unregister_binlog_transmit_observer(&transmit_observer, p)) {
    sql_print_error("unregister_binlog_transmit_observer failed");
    return 1;
  }

  sql_print_information("unregister speed limit master plugin OK");

  speed_monitor.cleanup();
  return 0;
}

struct Mysql_replication rpl_speed_limit_master_plugin= {
  MYSQL_REPLICATION_INTERFACE_VERSION
};

DEF_SHOW_FUNC(clients, SHOW_LONG);

/* plugin status variables */
SHOW_VAR rpl_speed_limit_status_vars[] = {
  { "rpl_speed_limit_master_clients",
  (char*)&SHOW_FNAME(clients),
  SHOW_FUNC, SHOW_SCOPE_GLOBAL },
  { "rpl_speed_limit_master_sleep_time",
  (char*)&rpl_speed_limit_sleep_time,
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL },
  { "rpl_speed_limit_master_sleep_count",
  (char*)&rpl_speed_limit_sleep_count,
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL },
  { "rpl_speed_limit_master_bytes_send",
  (char*)&rpl_speed_limit_bytes,
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL },
  { "rpl_speed_limit_master_bandwidth",
  (char*)&rpl_speed_limit_bandwidth,
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL },
  { NULL, NULL, SHOW_LONG, SHOW_SCOPE_GLOBAL },
};

/*
  Plugin library descriptor
*/
mysql_declare_plugin(rpl_speed_limit_master) {
  MYSQL_REPLICATION_PLUGIN,
  &rpl_speed_limit_master_plugin,
  "rpl_speed_limit_master",
  "zhiyangli",
  "replication speed limit in master",
  PLUGIN_LICENSE_GPL,
  rpl_speed_limit_master_plugin_init,     /* Plugin init */
  NULL,                                   /* Plugin check uninstall */
  rpl_speed_limit_master_plugin_deinit,   /* Plugin deinit */
  0x0100,                                 /* Plugin version*/
  rpl_speed_limit_status_vars,           /* status variables */
  rpl_speed_limit_system_vars,           /* system variables */
  NULL,                                   /* config options */
  0,                                      /* flags */
}
mysql_declare_plugin_end;

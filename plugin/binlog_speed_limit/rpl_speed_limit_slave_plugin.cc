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
#include <mysql.h>

extern RplSpeedMonitor speed_monitor;

int rpl_semi_slave_io_start(Binlog_relay_IO_param *) {
  THD* thd = current_thd;
  if (speed_monitor.addSlave(thd))
    return 1;

  return 0;
}

int rpl_semi_slave_io_end(Binlog_relay_IO_param *) {
  THD* thd = current_thd;
  speed_monitor.removeSlave(thd);
  return 0;
}

/*
 * HOOK after_read_event:
 * Note: we don't use event_buf and event_len, which have been assigned.
 */
int rpl_semi_slave_read_event(Binlog_relay_IO_param *,
                               const char *, unsigned long len,
                               const char **, unsigned long *) {
  THD* thd = current_thd;
  if (speed_monitor.controlSpeed(thd, len))
    return 1;

  return 0;
}

Binlog_relay_IO_observer relay_io_observer = {
    sizeof(Binlog_relay_IO_observer),  // len

    rpl_semi_slave_io_start,      // start
    rpl_semi_slave_io_end,        // stop
    NULL,                          // start sql thread
    NULL,                          // stop sql thread
    NULL,                          // request_transmit
    rpl_semi_slave_read_event,    // after_read_event
    NULL,                          // after_queue_event
    NULL,                          // reset
    NULL                           // apply
};

static int rpl_speed_limit_slave_plugin_init(void *p) {
#ifdef HAVE_PSI_INTERFACE
  init_psi_keys();
#endif
  speed_monitor.init();

  if (register_binlog_relay_io_observer(&relay_io_observer, p))
    return 1;
  sql_print_information("register speed_limit slave plugin OK");
  return 0;
}

static int rpl_speed_limit_slave_plugin_deinit(void *p) {
  speed_monitor.cleanup();

  if (unregister_binlog_relay_io_observer(&relay_io_observer, p))
    return 1;
  sql_print_information("unregister speed limit slave plugin OK");
  return 0;
}

struct Mysql_replication rpl_speed_limit_slave_plugin = {
  MYSQL_REPLICATION_INTERFACE_VERSION
};

DEF_SHOW_FUNC(clients, SHOW_LONG);

/* plugin status variables */
SHOW_VAR rpl_speed_limit_status_vars[] = {
  { "rpl_speed_limit_slave_clients",
  (char*)&SHOW_FNAME(clients),
  SHOW_FUNC, SHOW_SCOPE_GLOBAL },
  { "rpl_speed_limit_slave_sleep_time",
  (char*)&rpl_speed_limit_sleep_time,
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL },
  { "rpl_speed_limit_slave_sleep_count",
  (char*)&rpl_speed_limit_sleep_count,
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL },
  { "rpl_speed_limit_slave_bytes_received",
  (char*)&rpl_speed_limit_bytes,
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL },
  { "rpl_speed_limit_slave_bandwidth",
  (char*)&rpl_speed_limit_bandwidth,
  SHOW_LONGLONG, SHOW_SCOPE_GLOBAL },
  { NULL, NULL, SHOW_LONG, SHOW_SCOPE_GLOBAL },
};

/*
  Plugin library descriptor
*/
mysql_declare_plugin(rpl_speed_limit_slave) {
  MYSQL_REPLICATION_PLUGIN,
  &rpl_speed_limit_slave_plugin,
  "rpl_speed_limit_slave",
  "zhiyangli",
  "replication speed limit in slave",
  PLUGIN_LICENSE_GPL,
  rpl_speed_limit_slave_plugin_init,    /* Plugin init */
  NULL,                                 /* Plugin check uninstall */
  rpl_speed_limit_slave_plugin_deinit,  /* Plugin deinit */
  0x0100,                               /* Plugin version */
  rpl_speed_limit_status_vars,         /* status variables */
  rpl_speed_limit_system_vars,         /* system variables */
  NULL,                                 /* config options */
  0,                                    /* flags */
}
mysql_declare_plugin_end;

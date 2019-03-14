# Copyright (c) 2015, 2017, Tencent and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA


SET(REPLICATION_SPEED_LIMIT_MASTER_SOURCES
 rpl_speed_limit_master_plugin.cc
 rpl_speed_limit_plugin_vars.cc
 rpl_speed_limit_util.cc
 rpl_speed_monitor.cc
 rpl_speed_limit_plugin_vars.h
 rpl_speed_limit_util.h
 rpl_speed_monitor.h)

MYSQL_ADD_PLUGIN(rpl_speed_limit_master ${REPLICATION_SPEED_LIMIT_MASTER_SOURCES}
  MODULE_ONLY MODULE_OUTPUT_NAME "rpl_speed_limit_master")

SET(REPLICATION_SPEED_LIMIT_SLAVE_SOURCES
 rpl_speed_limit_slave_plugin.cc
 rpl_speed_limit_plugin_vars.cc
 rpl_speed_limit_util.cc
 rpl_speed_monitor.cc
 rpl_speed_limit_plugin_vars.h
 rpl_speed_limit_util.h
 rpl_speed_monitor.h)

MYSQL_ADD_PLUGIN(rpl_speed_limit_slave ${REPLICATION_SPEED_LIMIT_SLAVE_SOURCES}
  MODULE_ONLY MODULE_OUTPUT_NAME "rpl_speed_limit_slave")

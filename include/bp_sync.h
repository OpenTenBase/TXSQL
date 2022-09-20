/*
   Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef BP_SYNC_INCLUDED
#define BP_SYNC_INCLUDED

#define BP_SEND_BUFFER_SIZE 1048576
#define SNAPSHOT_FILENAME "ib_bp_info"
#define BP_END_FLAG 0XFFFFFF
#define BP_END_FLAG_LEN 3
#define TRANSMIT_STATUS_LEN 2048
#define TRANSMIT_RETRY_MAX 3
#define SLEEP_UNIT 50000

extern bool innodb_buffer_pool_transmit_enabled;
extern bool innodb_buffer_pool_transmit_exit;
extern bool innodb_buffer_pool_transmit_finished;
extern bool innodb_buffer_pool_current_file_has_sent;
extern ulonglong innodb_buffer_pool_transmit_interval;
extern char innodb_buffer_pool_transmit_status[TRANSMIT_STATUS_LEN];

#endif  // BP_SYNC_INCLUDED

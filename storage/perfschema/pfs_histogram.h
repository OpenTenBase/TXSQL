/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef PFS_HISTOGRAM_H
#define PFS_HISTOGRAM_H

#include <atomic>
#include <vector>
#include "my_compiler.h"
#include "my_inttypes.h"
#include "storage/perfschema/pfs_server.h"

/**
  @file storage/perfschema/pfs_histogram.h
*/

/** Maximum number of buckets used in histograms. */
#define MAX_NUMBER_OF_BUCKETS 450

struct PFS_histogram {
 public:
  void init() {
    m_bucket = std::vector<std::atomic<ulonglong>>(
        pfs_param.m_events_statements_histogram_bucket_number);
  }

  void reset();

  void increment_bucket(uint bucket_index) { m_bucket[bucket_index]++; }

  ulonglong read_bucket(uint bucket_index) { return m_bucket[bucket_index]; }

 private:
  std::vector<std::atomic<ulonglong>> m_bucket;
};

struct PFS_histogram_timers {
  ulonglong m_bucket_timer[MAX_NUMBER_OF_BUCKETS + 1];

  void init();
};

extern struct PFS_histogram_timers g_histogram_pico_timers;

#endif

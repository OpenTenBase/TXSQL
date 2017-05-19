/* Copyright (C) 2007 Google Inc.
   Copyright (C) 2016,2017 Tencent

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

#ifndef SEMISYNC_TIMESPEC_UTIL_H
#define SEMISYNC_TIMESPEC_UTIL_H

#include "my_systime.h"

#define TIME_THOUSAND 1000
#define TIME_MILLION  1000000
#define TIME_BILLION  1000000000

inline unsigned long long timespec_to_usec(const struct timespec *ts)
{
#ifdef __WIN__
  return ts->tv.i64 / 10;
#else
  return (unsigned long long) ts->tv_sec * TIME_MILLION + ts->tv_nsec / TIME_THOUSAND;
#endif
}

inline void timespec_reset(struct timespec *ts)
{
#ifdef __WIN__
  ts->tv.i64= 0;
#else
  ts->tv_sec= 0;
#endif /* _WIN32 */
}

inline bool timespec_is_set(struct timespec *ts)
{
#ifdef __WIN__
  return  ts->tv.i64 != 0;
#else
  return ts->tv_sec != 0;
#endif /* _WIN32 */
}

inline void timespec_add(const struct timespec &start_ts, int add_time,  /*out*/struct timespec *end_ts)
{
  /* Calcuate the waiting period. */
#ifdef __WIN__
  end_ts->tv.i64= start_ts.tv.i64 + (__int64)add_time * TIME_THOUSAND * 10;
  end_ts->max_timeout_msec= (long)add_time;
#else
  end_ts->tv_sec= start_ts.tv_sec + add_time / TIME_THOUSAND;
  end_ts->tv_nsec= start_ts.tv_nsec +
        (add_time % TIME_THOUSAND) * TIME_MILLION;
  if (end_ts->tv_nsec >= TIME_BILLION)
  {
    end_ts->tv_sec++;
    end_ts->tv_nsec -= TIME_BILLION;
  }
#endif /* _WIN32 */
}

/* Get the waiting time given the wait's staring time.
*
* Return:
*  >= 0: the waiting time in microsecons(us)
*   < 0: error in get time or time back traverse
*/
inline int getWaitTime(const struct timespec& start_ts)
{
  unsigned long long start_usecs, end_usecs;
  struct timespec end_ts;

  /* Starting time in microseconds(us). */
  start_usecs= timespec_to_usec(&start_ts);

  /* Get the wait time interval. */
  set_timespec(&end_ts, 0);

  /* Ending time in microseconds(us). */
  end_usecs= timespec_to_usec(&end_ts);

  if (end_usecs < start_usecs)
    return -1;

  return (int)(end_usecs - start_usecs);
}

#endif /* SEMISYNC_TIMESPEC_UTIL_H */

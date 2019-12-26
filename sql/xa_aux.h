/*
   Copyright (c) 2015, 2019, Oracle and/or its affiliates. All Rights Reserved.
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

#ifndef XA_AUX_H
#define XA_AUX_H
#include "m_string.h"  // _dig_vec_lower

/**
  Function serializes XID which is characterized by by four last arguments
  of the function.
  Serialized XID is presented in valid hex format and is returned to
  the caller in a buffer pointed by the first argument.
  The buffer size provived by the caller must be not less than
  8 + 2 * XIDDATASIZE +  4 * sizeof(XID::formatID) + 1, see
  XID::serialize_xid() that is a caller and plugin.h for XID declaration.

  @param buf  pointer to a buffer allocated for storing serialized data
  @param fmt  formatID value
  @param gln  gtrid_length value
  @param bln  bqual_length value
  @param dat  data value

  @return  the value of the buffer pointer
*/

inline char *serialize_xid(char *buf, long fmt, long gln, long bln,
                           const char *dat) {
  int i;
  char *c = buf;
  /*
    Build a string like following pattern:
      X'hex11hex12...hex1m',X'hex21hex22...hex2n',11
    and store it into buf.
    Here hex1i and hex2k are hexadecimals representing XID's internal
    raw bytes (1 <= i <= m, 1 <= k <= n), and `m' and `n' even numbers
    half of which corresponding to the lengths of XID's components.
  */
  *c++ = 'X';
  *c++ = '\'';
  for (i = 0; i < gln; i++) {
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) >> 4];
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) & 0x0f];
  }
  *c++ = '\'';

  *c++ = ',';
  *c++ = 'X';
  *c++ = '\'';
  for (; i < gln + bln; i++) {
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) >> 4];
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) & 0x0f];
  }
  *c++ = '\'';
  sprintf(c, ",%lu", fmt);

  return buf;
}

bool deserialize_xid(const char *buf, long &fmt, long &gln, long &bln, char *dat);

#include <set>
#include <atomic>
#include "my_thread.h"

typedef std::set<std::string> Txnids_t;
class Prepared_xa_txnids {
public:
  class Xa_txnids_instance {
    std::atomic<bool> atomic_locked{false};
    Txnids_t txnids;
    public:
    Xa_txnids_instance() {
      txnids.clear();
    }

    ~Xa_txnids_instance() {
      txnids.clear();
    }

    void lock() {
      bool expected = false;
      while (!atomic_locked.compare_exchange_weak(expected, true)) {
        expected = false;
        my_thread_yield();
      }
    }

    void unlock() {
      atomic_locked = false;
    }

    void clear() {
      lock();
      txnids.clear();
      unlock();
    }

    void add_id(const std::string &id) {
      lock();
      txnids.insert(id);
      unlock();
    }

    void del_id(const std::string &id) {
      lock();
      txnids.erase(id);
      unlock();
    }

    uint32_t serialize(std::string &id) {
      uint32_t count = 0;
      lock();

      for (Txnids_t::iterator i= txnids.begin(); i != txnids.end(); ++i) {
        id+= *i;
        id+= "|";
        count++;
      }

      unlock();

      return count;
    }
  };

  Prepared_xa_txnids() {}
  ~Prepared_xa_txnids() {
    delete [] m_instances;
  }

  uint32_t serialize(std::string &id) {
    id.reserve(1024*4);
    id.push_back('\'');
    uint32_t count = 0;

    for (uint32_t i = 0; i < m_parts; i++) {
      count += m_instances[i].serialize(id);
    }

    id.push_back('\'');

    return count;
  }

  void clear() {
    for (uint32_t i = 0; i < m_parts; i++) {
      m_instances[i].clear();
    }
  }

  void add_id(const std::string &id);

  void del_id(const std::string &id);

  void from_recovery(Txnids_t &prepared, const Txnids_t &committed,
                     const Txnids_t &aborted);

  static int parse(const char *str, Txnids_t &ids);

  void init(uint32_t parts);

private:
  uint32_t get_instance(const std::string &id);

  uint32_t m_parts;

  Xa_txnids_instance *m_instances;
};

extern Prepared_xa_txnids prepared_xa_txnids;

#endif /* XA_AUX_H */

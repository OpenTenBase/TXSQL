/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file sql/histograms/value_vector.cc
  Value vector (implementation).
*/

#include "sql/histograms/value_vector.h"

#include <math.h>
#include <string>  // std::string

#include "mysql_time.h"  // MYSQL_TIME
#include "sql/histograms/value_map.h"
#include "sql/my_decimal.h"      // my_decimal_cmp
#include "sql/psi_memory_key.h"  // key_memory_histograms
#include "sql_string.h"          // String
#include "template_utils.h"      // down_cast

namespace histograms {

Value_vector_base::Value_vector_base(const CHARSET_INFO *,
                                     Value_map_type data_type)
    : m_data_type(data_type) {
  init_sql_alloc(key_memory_histograms, &m_mem_root, 256);
}

void Value_vector_base::init_reservoir(size_t capacity, int seed) {
  assert(capacity > 0);
  m_reservoir.init(static_cast<int>(capacity), seed);
}

void Value_vector_base::init_reservoir(size_t capacity) {
  assert(capacity > 0);
  m_reservoir.init(static_cast<int>(capacity));
}

template <class T>
bool Value_vector_base::add_value(const T &value, bool is_null) {
  /*
    The first N (sample size) sample rows are simply copied into the
    reservoir. Then we start replacing tuples in the sample until
    we reach the end of input.  This algorithm is from Jeff Vitter's
    paper. It works by repeatedly computing the number of tuples to
    skip before selecting a tuple, which replaces a randomly chosen
    element of the reservoir (current set of tuples).  At all times
    the reservoir is a true random sample of the tuples we've passed
    over so far, so when we fall off the end of input we're done.
  */

  Value_vector<T> *value_vector = down_cast<Value_vector<T> *>(this);
  if (!m_reservoir.is_full()) {
    if (value_vector->add_value(value, is_null) ||
        DBUG_EVALUATE_IF("vector_add_value_error", true, false))
      return true;
  } else {
    const longlong key = m_reservoir.choose_element();
    assert(key == -1 || (size_t)key < value_vector->size());
    if (key >= 0 && (size_t)key < value_vector->size()) {
      if (value_vector->replace_value(key, value, is_null) ||
          DBUG_EVALUATE_IF("vector_replace_value_error", true, false))
        return true;
    }
  }
  m_reservoir.increment_processed();

  return false;
}

template <class T>
bool Value_vector<T>::add_value(const T &value, bool is_null) {
  assert(!m_reservoir.is_full() &&
         m_value_vector.size() == m_reservoir.size());
  try {
    m_value_vector.emplace_back(value, is_null);
  } catch (const std::bad_alloc &) {
    // Out of memory.
    return true;  // purecov: inspected
  }
  return false;
}

template <>
bool Value_vector<String>::add_value(const String &value, bool is_null) {
  assert(!m_reservoir.is_full() &&
         m_value_vector.size() == m_reservoir.size());
  try {
    if (is_null) {
      m_value_vector.emplace_back(value, is_null);
    } else {
      String substring = value.substr(0, HISTOGRAM_MAX_COMPARE_LENGTH);
      char *string_data = substring.dup(&m_mem_root);
      if (string_data == nullptr) return true;  // purecov: deadcode

      String string_dup(string_data, substring.length(), substring.charset());
      m_value_vector.emplace_back(string_dup, is_null);
    }
  } catch (const std::bad_alloc &) {
    // Out of memory.
    return true;  // purecov: inspected
  }
  return false;
}

template <class T>
bool Value_vector<T>::replace_value(longlong key, const T &value,
                                    bool is_null) {
  assert(m_reservoir.is_full() &&
         m_value_vector.size() == m_reservoir.size());
  try {
    m_value_vector[key].reset(value, is_null);
  } catch (const std::bad_alloc &) {
    // Out of memory.
    return true;  // purecov: inspected
  }
  return false;
}

template <>
bool Value_vector<String>::replace_value(longlong key, const String &value,
                                         bool is_null) {
  assert(m_reservoir.is_full() &&
         m_value_vector.size() == m_reservoir.size());
  try {
    if (is_null) {
      m_value_vector[key].reset(value, is_null);
    } else {
      String substring = value.substr(0, HISTOGRAM_MAX_COMPARE_LENGTH);
      char *string_data = substring.dup(&m_mem_root);
      if (string_data == nullptr) return true;  // purecov: deadcode

      String string_dup(string_data, substring.length(), substring.charset());
      m_value_vector[key].reset(string_dup, is_null);
    }
  } catch (const std::bad_alloc &) {
    // Out of memory.
    return true;  // purecov: inspected
  }
  return false;
}

template <class T>
bool Value_vector<T>::fill_into_value_map(Value_map_base *value_map) const {
  try {
    for (auto value_node : m_value_vector) {
      if (value_node.is_null) {
        value_map->add_null_values(1);
      } else {
        if (value_map->add_values(value_node.data, 1)) {
          return true;  // purecov: deadcode
        }
      }
    }
  } catch (const std::bad_alloc &) {
    // Out of memory.
    return true;  // purecov: inspected
  }
  return false;
}

// Explicit template instantiations.
template class Vector_node<double>;
template class Vector_node<String>;
template class Vector_node<ulonglong>;
template class Vector_node<longlong>;
template class Vector_node<MYSQL_TIME>;
template class Vector_node<my_decimal>;

template class Value_vector<double>;
template class Value_vector<String>;
template class Value_vector<ulonglong>;
template class Value_vector<longlong>;
template class Value_vector<MYSQL_TIME>;
template class Value_vector<my_decimal>;

template bool Value_vector_base::add_value(const double &, bool);
template bool Value_vector_base::add_value(const String &, bool);
template bool Value_vector_base::add_value(const ulonglong &, bool);
template bool Value_vector_base::add_value(const longlong &, bool);
template bool Value_vector_base::add_value(const MYSQL_TIME &, bool);
template bool Value_vector_base::add_value(const my_decimal &, bool);

template bool Value_vector_base::add_values(const double &, const ha_rows,
                                            bool);
template bool Value_vector_base::add_values(const String &, const ha_rows,
                                            bool);
template bool Value_vector_base::add_values(const ulonglong &, const ha_rows,
                                            bool);
template bool Value_vector_base::add_values(const longlong &, const ha_rows,
                                            bool);
template bool Value_vector_base::add_values(const MYSQL_TIME &, const ha_rows,
                                            bool);
template bool Value_vector_base::add_values(const my_decimal &, const ha_rows,
                                            bool);
}  // namespace histograms

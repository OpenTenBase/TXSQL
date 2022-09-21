#ifndef HISTOGRAMS_VALUE_VECTOR_INCLUDED
#define HISTOGRAMS_VALUE_VECTOR_INCLUDED

/* Copyright (c) 2017, 2020, Oracle and/or its affiliates.

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
  @file sql/histograms/value_vector.h
*/

#include <stddef.h>
#include <string>
#include <vector>

#include "my_alloc.h"
#include "my_base.h"  // ha_rows
#include "mysql_time.h"
#include "sql/histograms/reservoir.h"
#include "sql/histograms/value_map.h"
#include "sql/histograms/value_map_type.h"
#include "sql/mem_root_allocator.h"
#include "sql/thr_malloc.h"

class String;
class my_decimal;
template <class T>
class Mem_root_allocator;

namespace histograms {

/**
  The abstract base class for all Value_vector types.

  Value_vector_base::add_values and Value_vector::add_values looks like the same
  function, but they are not. Value_vector_base::add_values is a small functions
  that helps us cast the Value_vector<T> to the correct type (for instance
  Value_vector<longlong>). Ideally, this function would have been pure virtual,
  but it's not possible to have virtual member function templates.
 */
class Value_vector_base {
 private:
  const Value_map_type m_data_type;

 protected:
  MEM_ROOT m_mem_root;

  Reservoir_state m_reservoir;

 public:
  Value_vector_base(const CHARSET_INFO *, Value_map_type data_type);

  virtual ~Value_vector_base() {}

  /**
    Initialize the reservoir with provided sed.

    @param capacity  Maximum number of elements in reservoir.
    @param seed      To seed the random generator for the reservoir algorithm.
   */
  void init_reservoir(size_t capacity, int seed);

  /**
    Initialize the reservoir with default seed.

    @param capacity  Maximum number of elements in reservoir.
   */
  void init_reservoir(size_t capacity);

  /**
    Returns the number of values in the Value_vector.

    @return The number of values in the Value_vector.
  */
  size_t size() const { return m_reservoir.size(); }

  size_t num_processed() const { return m_reservoir.num_processed(); }

  template <class T>
  bool add_values(const T &value, const ha_rows count, bool is_null) {
    for (ha_rows n = 0; n < count; n++) {
      if (add_value(value, is_null)) return true;
    }
    return false;
  }

  /**
    Iterate the Value_vector and add values to Value_map.

    @param value_map The target Value_map.

    @return false on success, and true in case of errors (OOM).
  */
  virtual bool fill_into_value_map(Value_map_base *value_map) const = 0;

  /// Delete the vector contents.
  virtual void clean() = 0;

  virtual size_t element_overhead() const = 0;

 private:
  /**
    Add a value to this Value_vector.

    @param value The value to add.

    @return false on success, and true in case of errors (OOM).
  */
  template <class T>
  bool add_value(const T &value, bool is_null);

  /**
    Replace a value in this Value_vector with the given value.

    @param key The index of the value to be replaced, like Value_vector[key].
    @param value The value to add.

    @return false on success, and true in case of errors (OOM).
  */
  template <class T>
  bool replace_value(longlong key, const T &value, bool is_null);
};

/**
  Vector node.

  Typical usage is in a "value vector", which contains is_null and data.
  is_null: true means data in this node is NULL.
*/
template <class T>
struct Vector_node {
  Vector_node(T data_arg, bool is_null_arg)
      : data(data_arg), is_null(is_null_arg) {}
  void reset(const T &value_arg, bool is_null_arg) {
    data = value_arg;
    is_null = is_null_arg;
  }

  T data;
  bool is_null;
};

/**
  Value_vector class.

  This class works as a vector. It is a collection of value. The class abstracts
  away things like the underlying container.
*/
template <class T>
class Value_vector final : public Value_vector_base {
 private:
  using value_vector_type =
      std::vector<Vector_node<T>, Mem_root_allocator<Vector_node<T>>>;

  value_vector_type m_value_vector;

 public:
  Value_vector(const CHARSET_INFO *charset, Value_map_type data_type)
      : Value_vector_base(charset, data_type),
        m_value_vector(
            typename value_vector_type::allocator_type(&m_mem_root)) {}

  bool add_value(const T &value, bool is_null);

  bool replace_value(longlong key, const T &value, bool is_null);

  bool fill_into_value_map(Value_map_base *value_map) const override;

  void clean() override {
    value_vector_type vec;
    vec.swap(m_value_vector);
  }

  size_t element_overhead() const override {
    return sizeof(typename value_vector_type::value_type);
  }
};

}  // namespace histograms

#endif

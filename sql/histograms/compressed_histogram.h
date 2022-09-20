#ifndef HISTOGRAMS_COMPRESSED_HISTOGRAM_INCLUDED
#define HISTOGRAMS_COMPRESSED_HISTOGRAM_INCLUDED

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

/**
  @file sql/histograms/compressed_histogram.h
  Compressed histogram.

  This file defines the Compressed histogram. A Compressed stores not only the
  most common values(MCV) frequencies, but also other values' endpoints that
  is same as classic equi-height histogram.

  Compressed histogram is well accepted in PostgreSQL and DB2, and discussed in:

      Poosala, Viswanath, et al.
      Improved histograms for selectivity estimation of range
      predicates.  ACM Sigmod Record 25.2 (1996): 294-305.

  The list of most common values is automatically resized by value significance
  at the low end.

  A Compressed histogram also converted to a JSON object, follows the following
  "schema":

  {
    // Last time the histogram was updated. As of now, this means "when the
    // histogram was created" (incremental updates are not supported). Date/time
    // is given in UTC.
    // -- J_DATETIME
    "last-updated": "2015-11-04 15:19:51.000000",

    // Histogram type. Always "compressed" for compressed histograms.
    // -- J_STRING
    "histogram-type": "compressed",

    // Fraction of NULL values. This is the total fraction of NULL values in the
    // original data set.
    // -- J_DOUBLE
    "null-values": 0.1,

    // MCV list buckets. May be an empty array, if for instance the source
    // only contains NULL values.
    // -- J_ARRAY
    "mcv-list":
    [
      [
        // Value
        // -- Data type depends on the source column.
        42,

        // Cumulative frequency
        // -- J_DOUBLE
        0.001978728666831561
      ]
    ]

    // Equi-height bucket endpoints for non-mcv values. May be an empty array.
    // The first bucket is represented by endpoint 0 and endpoint 1, the second
    // by endpoint 1 and endpoint 2, and the same reasoning applies to
    // all successive buckets. So it may be empty or at least has two endpoints.
    // -- J_ARRAY
    "equi-height":
    [
      // Value
      // -- Data type depends on the source column.
      403,

      // Value
      // -- Data type depends on the source column.
      404
    ]
  }
*/

#include <stddef.h>
#include <map>      // std::map
#include <string>   // std::string
#include <utility>  // std::pair
#include <vector>   // std::vector

#include "my_inttypes.h"
#include "mysql_time.h"
#include "sql/histograms/histogram.h"  // Histogram, Histogram_comparator,
#include "sql/histograms/value_map.h"
#include "sql/histograms/value_map_type.h"
#include "sql/mem_root_allocator.h"
#include "sql/my_decimal.h"
#include "sql_string.h"

class Json_array;
class Json_object;
struct MEM_ROOT;

namespace histograms {

/**
  Compressed_histogram histogram.
*/
struct Histogram_comparator;

template <class T>
class Compressed_histogram : public Histogram {
 private:
  /// String representation of the histogram type COMPRESSED.
  static constexpr const char *compressed_str() { return "compressed"; }

  /// String representation of the MCV buckets.
  static constexpr const char *mcv_buckets_str() { return "mcv-list"; }

  /// String representation of the non-MCV buckets.
  static constexpr const char *other_values_endpoint_str() {
    return "equi-height";
  }

  /// String representation of the ndistinct.
  static constexpr const char *ndistinct_str() { return "distinct-values"; }

  using mcv_buckets_allocator = Mem_root_allocator<std::pair<const T, double>>;

  using mcv_buckets_type =
      std::map<const T, double, Histogram_comparator, mcv_buckets_allocator>;

  /// The buckets for mcv lists. [key, cumulative frequency].
  mcv_buckets_type m_mcv_buckets;

  using other_value_endpoint_allocator = Mem_root_allocator<T>;

  using other_value_endpoint_vector_type =
      std::vector<T, other_value_endpoint_allocator>;

  /// The buckets for other values' endpoints. [key].
  other_value_endpoint_vector_type m_equi_height;
  /// Number of distinct values of the table.
  double m_ndistinct;

  using mcv_type = std::pair<const T *, ha_rows>;
  using mcv_allocator = Mem_root_allocator<mcv_type>;

  using mcv_list_type = std::vector<mcv_type, mcv_allocator>;

 public:
  /**
    Compressed_histogram constructor.

    This will not build the histogram, but only set its properties.

    @param mem_root  the mem_root where the histogram contents will be allocated
    @param db_name   name of the database this histogram represents
    @param tbl_name  name of the table this histogram represents
    @param col_name  name of the column this histogram represents
    @param data_type the type of data that this histogram contains
    @param[out] error is set to true if an error occurs
  */
  Compressed_histogram(MEM_ROOT *mem_root, const std::string &db_name,
                       const std::string &tbl_name, const std::string &col_name,
                       Value_map_type data_type, bool *error);

  /**
    Compressed_histogram copy-constructor

    This will take a copy of the histogram and all of its contents on the
    provided MEM_ROOT.

    @param mem_root the MEM_ROOT to allocate the new histogram on.
    @param other    the histogram to take a copy of
    @param[out] error is set to true if an error occurs
  */
  Compressed_histogram(MEM_ROOT *mem_root,
                       const Compressed_histogram<T> &other,
                       bool *error);

  Compressed_histogram(const Compressed_histogram<T> &other) = delete;

  /**
    Build the Compressed_histogram histogram.

    @param   value_map   values to create the histogram for
    @param   num_buckets the number of buckets specified/requested by the user

    @return  true on error, false otherwise
  */
  bool build_histogram(const Value_map<T> &value_map, size_t num_buckets);

  /**
    Convert this histogram to a JSON object.

    This function will take the contents of the current histogram and put
    it in the output parameter "json_object".

    @param[in,out] json_object output where the histogram is to be stored. The
                   caller is responsible for allocating/deallocating the JSON
                   object

    @return        true on error, false otherwise
  */
  bool histogram_to_json(Json_object *json_object) const override;

  /**
    @return number of values/buckets in this histogram
  */
  size_t get_num_buckets() const override {
    return std::max(m_equi_height.size(), static_cast<size_t>(1)) - 1;
  }

  /**
    Get the estimated number of distinct non-NULL values.
    @return number of distinct non-NULL values
  */
  size_t get_num_distinct_values() const override {
    return static_cast<size_t>(m_ndistinct);
  }

  /**
    Returns the histogram type as a readable string.

    @return a readable string representation of the histogram type
  */
  std::string histogram_type_to_str() const override;

  /**
    Make a clone of this histogram on a MEM_ROOT.

    @param mem_root the MEM_ROOT to allocate the new histogram contents on.

    @return a copy of the histogram allocated on the provided MEM_ROOT.
  */
  Histogram *clone(MEM_ROOT *mem_root) const override;

  /**
    Find the number of values equal to 'value'.

    This function will estimate the number of values that is equal to the
    provided value.

    @param value The value to estimate the selectivity for.

    @return the selectivity between 0.0 and 1.0 inclusive.
  */
  double get_equal_to_selectivity(const T &value) const;

  /**
    Find the number of values less than 'value'.

    This function will estimate the number of values that is less than the
    provided value.

    @param value The value to estimate the selectivity for.

    @return the selectivity between 0.0 and 1.0 inclusive.
  */
  double get_less_than_selectivity(const T &value) const;

  /**
    Find the number of values greater than 'value'.

    This function will estimate the number of values that is greater than the
    provided value.

    @param value The value to estimate the selectivity for.

    @return the selectivity between 0.0 and 1.0 inclusive.
  */
  double get_greater_than_selectivity(const T &value) const;

 private:
  double get_less_than_equal_selectivity(const T &value) const;

  /**
    Analyze the list of common values in the sample rows decide how many are
    worthing storing in the MCV list.

    @param   mcv_list                    every most common value count
    @param   num_distinct_float          dinstinct value count
    @param   null_values_fraction        null fraction of null value
    @param   samplerows                  rows in memory for sampling
    @param   totalrows                   total rows count in whole table

    @return  resized number
  */
  size_t analyze_mcv_list(const mcv_list_type &mcv_list,
                          double num_distinct_float,
                          double null_values_fraction, int samplerows,
                          double totalrows);

  /**
    Add value to a JSON bucket

    This function adds the value to the supplied JSON array.

    @param      value       the value to add
    @param[out] json_bucket a JSON array where the bucket data is to be stored

    @return     true on error, false otherwise
  */
  static bool add_value_json_bucket(const T &value, Json_array *json_bucket);

  /**
    Convert one bucket to a JSON object.

    @param      bucket      the histogram bucket to convert
    @param[out] json_bucket a JSON array where the bucket data is to be stored

    @return     true on error, false otherwise
  */
  static bool create_json_bucket(const std::pair<T, double> &bucket,
                                 Json_array *json_bucket);

  /**
    Convert one bucket to a JSON object.

    @param      bucket      the histogram bucket to convert
    @param[out] json_bucket a JSON array where the bucket data is to be stored

    @return     true on error, false otherwise
  */
  static bool create_json_bucket(const T &bucket, Json_array *json_bucket);

  /**
    Perform linear interpolation of value between lower and upper values.

    Note that all types of data are converted to double for interpolation.

    @param      value     The value to interpolate.
    @param      lower     The lower value.
    @param      upper     The upper value.

    @return     fraction of one bucket.
  */
  double interpolate(const T &value, const T &lower, const T &upper) const;

 protected:
  /**
    Populate this histogram with contents from a JSON object.

    @param json_object  a JSON object that represents an Compressed_histogram
    histogram
    @param context      error context for validation

    @return true on error, false otherwise.
  */
  bool json_to_histogram(const Json_object &json_object,
                         Error_context *context) override;
};

}  // namespace histograms

#endif

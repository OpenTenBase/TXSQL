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
  @file sql/histograms/compressed_histogram.cc
  Compressed_histogram (implementation).
*/

#include "sql/histograms/compressed_histogram.h"

#include <math.h>
#include <iterator>
#include <new>
#include <utility>  // std::make_pair

#include "field_types.h"  // enum_field_types
#include "my_base.h"      // ha_rows
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql_time.h"
#include "sql-common/json_dom.h"              // Json_*
#include "sql/histograms/value_map.h"  // Histogram_comparator
#include "sql/log.h"
#include "template_utils.h"

struct MEM_ROOT;

namespace histograms {

static double convert_string_to_scalar(const char *value, int range_low,
                                       int range_upp) {
  int slen = strlen(value);
  double num, denom, base = 0;

  if (slen <= 0) return 0.0;  // empty string has scalar value 0

  // There seems little point in considering more than a dozen bytes from
  // the string.  Since base is at least 10, that will give us nominal
  // resolution of at least 12 decimal digits, which is surely far more
  // precision than this estimation technique has got anyway (especially in
  // non-C locales).  Also, even with the maximum possible base of 256, this
  // ensures denom cannot grow larger than 256^13 = 2.03e31, which will not
  // overflow on any known machine.
  if (slen > 12) slen = 12;

  /* Convert initial characters to fraction */
  base = range_upp - range_low + 1;
  num = 0.0;
  denom = base;
  while (slen-- > 0) {
    int ch = (unsigned char)*value++;

    if (ch < range_low)
      ch = range_low - 1;
    else if (ch > range_upp)
      ch = range_upp + 1;
    num += ((double)(ch - range_low)) / denom;
    denom *= base;
  }
  return num;
}

template <class T>
Compressed_histogram<T>::Compressed_histogram(MEM_ROOT *mem_root,
                                              const std::string &db_name,
                                              const std::string &tbl_name,
                                              const std::string &col_name,
                                              Value_map_type data_type,
                                              bool *error)
    : Histogram(mem_root, db_name, tbl_name, col_name,
                enum_histogram_type::COMPRESSED, data_type, error),
      m_mcv_buckets(Histogram_comparator(), mcv_buckets_allocator(mem_root)),
      m_equi_height(other_value_endpoint_allocator(mem_root)),
      m_ndistinct(0) {}

template <class T>
Compressed_histogram<T>::Compressed_histogram(
    MEM_ROOT *mem_root, const Compressed_histogram<T> &other, bool *error)
    : Histogram(mem_root, other, error),
      m_mcv_buckets(other.m_mcv_buckets.begin(), other.m_mcv_buckets.end(),
                    Histogram_comparator(), mcv_buckets_allocator(mem_root)),
      m_equi_height(other.m_equi_height.begin(), other.m_equi_height.end(),
                    other_value_endpoint_allocator(mem_root)),
      m_ndistinct(other.m_ndistinct) {}

template <>
Compressed_histogram<String>::Compressed_histogram(
    MEM_ROOT *mem_root, const Compressed_histogram<String> &other, bool *error)
    : Histogram(mem_root, other, error),
      m_mcv_buckets(Histogram_comparator(), mcv_buckets_allocator(mem_root)),
      m_equi_height(other_value_endpoint_allocator(mem_root)),
      m_ndistinct(other.m_ndistinct) {
  // Copy mcv and non-mcv bucket contents. We need to make duplicates of String
  // data, since they are allocated on a MEM_ROOT that most likely will be freed
  // way too early.
  for (const auto &bucket : other.m_mcv_buckets) {
    char *string_data = bucket.first.dup(mem_root);
    if (string_data == nullptr) {
      assert(false); /* purecov: deadcode */
      return;             // OOM
    }

    String string_dup(string_data, bucket.first.length(),
                      bucket.first.charset());
    m_mcv_buckets.emplace(string_dup, bucket.second);
  }

  for (const auto &bucket : other.m_equi_height) {
    char *string_data = bucket.dup(mem_root);
    if (string_data == nullptr) {
      assert(false); /* purecov: deadcode */
      return;             // OOM
    }

    String string_dup(string_data, bucket.length(), bucket.charset());
    m_equi_height.emplace_back(string_dup);
  }
}

template <class T>
size_t Compressed_histogram<T>::analyze_mcv_list(const mcv_list_type &mcv_list,
                                                 double stadistinct,
                                                 double nullfrac,
                                                 int samplerows,
                                                 double totalrows) {
  int num_mcv = mcv_list.size();

  // If the entire table was sampled, keep the whole list.  This also
  // protects us against division by zero in the code below.
  if (samplerows == totalrows || totalrows <= 1.0) return num_mcv;

  // Re-extract the estimated number of distinct nonnull values in table
  double ndistinct_table = stadistinct;
  if (ndistinct_table < 0) ndistinct_table = -ndistinct_table * totalrows;

  // Exclude the least common values from the MCV list, if they are not
  // significantly more common than the estimated selectivity they would
  // have if they weren't in the list.  All non-MCV values are assumed to be
  // equally common, after taking into account the frequencies of all the
  // values in the MCV list and the number of nulls (c.f. eqsel()).
  //
  // Here sumcount tracks the total count of all but the last (least common)
  // value in the MCV list, allowing us to determine the effect of excluding
  // that value from the list.
  //
  // Note that we deliberately do this by removing values from the full
  // list, rather than starting with an empty list and adding values,
  // because the latter approach can fail to add any values if all the most
  // common values have around the same frequency and make up the majority
  // of the table, so that the overall average frequency of all values is
  // roughly the same as that of the common values.  This would lead to any
  // uncommon values being significantly overestimated.
  double sumcount = 0.0;
  for (int i = 0; i < num_mcv - 1; i++) sumcount += mcv_list[i].second;

  while (num_mcv > 0) {
    double selec;
    double otherdistinct;
    double N;
    double n;
    double K;
    double variance;
    double stddev;

    // Estimated selectivity the least common value would have if it
    // wasn't in the MCV list (c.f. eqsel()).
    selec = 1.0 - sumcount / samplerows - nullfrac;
    if (selec < 0.0) selec = 0.0;
    if (selec > 1.0) selec = 1.0;
    otherdistinct = ndistinct_table - (num_mcv - 1);
    if (otherdistinct > 1) selec /= otherdistinct;

    // If the value is kept in the MCV list, its population frequency is
    // assumed to equal its sample frequency.  We use the lower end of a
    // textbook continuity-corrected Wald-type confidence interval to
    // determine if that is significantly more common than the non-MCV
    // frequency --- specifically we assume the population frequency is
    // highly likely to be within around 2 standard errors of the sample
    // frequency, which equates to an interval of 2 standard deviations
    // either side of the sample count, plus an additional 0.5 for the
    // continuity correction.  Since we are sampling without replacement,
    // this is a hypergeometric distribution.
    //
    // XXX: Empirically, this approach seems to work quite well, but it
    // may be worth considering more advanced techniques for estimating
    // the confidence interval of the hypergeometric distribution.
    N = totalrows;
    n = samplerows;
    K = N * mcv_list[num_mcv - 1].second / n;
    variance = n * K * (N - K) * (N - n) / (N * N * (N - 1));
    stddev = sqrt(variance);

    if (mcv_list[num_mcv - 1].second > selec * samplerows + 2 * stddev + 0.5) {
      // The value is significantly more common than the non-MCV
      // selectivity would suggest.  Keep it, and all the other more
      // common values in the list.
      break;
    } else {
      // Discard this value and consider the next least common value
      num_mcv--;
      if (num_mcv == 0) break;
      sumcount -= mcv_list[num_mcv - 1].second;
    }
  }

  return num_mcv;
}

template <class T>
bool Compressed_histogram<T>::build_histogram(const Value_map<T> &value_map,
                                              size_t num_buckets) {
  // Clear any existing data.
  m_mcv_buckets.clear();
  m_equi_height.clear();
  m_ndistinct = 0;
  m_null_values_fraction = INVALID_NULL_VALUES_FRACTION;
  m_sampling_rate = value_map.get_sampling_rate();

  // Set the number of buckets that was specified/requested by the user.
  m_num_buckets_specified = num_buckets;

  // Set the character set for the histogram data.
  m_charset = value_map.get_character_set();

  // No values, nothing to do.
  if (value_map.size() == 0) {
    if (value_map.get_num_null_values() > 0)
      m_null_values_fraction = 1.0;
    else
      m_null_values_fraction = 0.0;
    return false;
  }

  // Some basic statistics gathered in a single run over the value map.
  ha_rows num_non_null_values = 0;
  ha_rows min_repeat_count = ~(ha_rows)0;
  ha_rows ndistinct_1 = 0;
  size_t ndistinct = value_map.size();

  // Extract the list of most frequent values by a min heap. The number of
  // buckets implies the capacity of the list. The list will be resized later.
  size_t heap_limit = num_buckets;
  size_t heap_size = 0;

  Histogram_comparator value_cmp;
  auto mcv_greater = [value_cmp](const mcv_type &a, const mcv_type &b) {
    if (a.second == b.second) return value_cmp(*b.first, *a.first);
    return a.second > b.second;
  };

  MEM_ROOT heap_mem_root{PSI_NOT_INSTRUMENTED, 512};
  mcv_allocator allocator(&heap_mem_root);
  mcv_list_type mcv_list(allocator);

  for (const auto &node : value_map) {
    ha_rows repeat_count = node.second;

    num_non_null_values += repeat_count;
    if (repeat_count < min_repeat_count) min_repeat_count = repeat_count;
    if (repeat_count == 1) ndistinct_1++;

    if (heap_size < heap_limit) {
      heap_size++;
      mcv_list.emplace_back(&node.first, repeat_count);
      std::make_heap(mcv_list.begin(), mcv_list.end(), mcv_greater);
    } else if (repeat_count > mcv_list.front().second) {
      std::pop_heap(mcv_list.begin(), mcv_list.end(), mcv_greater);
      mcv_list.pop_back();
      mcv_list.emplace_back(&node.first, repeat_count);
      std::make_heap(mcv_list.begin(), mcv_list.end(), mcv_greater);
    }
  }

  const ha_rows samplerows =
      value_map.get_num_null_values() + num_non_null_values;
  // The number of rows in the table. TODO: obtain from SE
  double totalrows = ((double)samplerows) / m_sampling_rate;

  // Set the fractions of NULL values.
  m_null_values_fraction =
      value_map.get_num_null_values() / static_cast<double>(samplerows);

  // Sort for analyzing significance of values. It works for any empty list.
  std::sort(mcv_list.begin(), mcv_list.end(), mcv_greater);

#if 0
  // TODO: Interface method get_num_distinct_values() does not support scale.
  if (num_non_null_values == ndistinct) {
    // If we found no repeated non-null values, assume it's a unique
    // column; but be sure to discount for any nulls we found.
    // This special case is represented by a nagative scale.
    m_ndistinct = -1.0 * (1.0 - m_null_values_fraction);
  } else
#endif
  if (min_repeat_count > 1) {
    // Every value in the sample appeared more than once.  Assume the
    // column has just these values.  (This case is meant to address
    // columns with small, fixed sets of possible values, such as
    // boolean or enum columns.  If there are any values that appear
    // just once in the sample, including too-wide values, we should
    // assume that that's not what we're dealing with.)
    m_ndistinct = ndistinct;
  } else {
    // Estimate the number of distinct values using the estimator
    // proposed by Haas and Stokes in IBM Research Report RJ 10025:
    //      	n*d / (n - f1 + f1*n/N)
    // where f1 is the number of distinct values that occurred
    // exactly once in our sample of n rows (from a total of N),
    // and d is the total number of distinct values in the sample.
    // This is their Duj1 estimator; the other estimators they
    // recommend are considerably more complex, and are numerically
    // very unstable when n is much smaller than N.
    //
    // In this calculation, we consider only non-nulls.  We used to
    // include rows with null values in the n and N counts, but that
    // leads to inaccurate answers in columns with many nulls, and
    // it's intuitively bogus anyway considering the desired result is
    // the number of distinct non-null values.
    //
    // TODO: Overwidth heuristic may be added for big string values.
    int f1 = ndistinct_1;
    int d = ndistinct;
    double n = num_non_null_values;
    double N = totalrows * (1.0 - m_null_values_fraction);
    double stadistinct;

    /* N == 0 shouldn't happen, but just in case ... */
    if (N > 0)
      stadistinct = (n * d) / ((n - f1) + f1 * n / N);
    else
      stadistinct = 0;

    /* Clamp to sane range in case of roundoff error */
    if (stadistinct < d) stadistinct = d;
    if (stadistinct > N) stadistinct = N;
    /* And round to integer */
    m_ndistinct = floor(stadistinct + 0.5);
  }

#if 0
  // If we estimated the number of distinct values at more than 10% of
  // the total row count (a very arbitrary limit), then assume that
  // stadistinct should scale with the row count rather than be a fixed
  // value.
  if (m_ndistinct > 0.1 * totalrows)
    m_ndistinct = -(m_ndistinct / totalrows);
#endif

  // Decide how many values are worth storing as most-common values. If
  // we are able to generate a complete MCV list (all the values in the
  // sample will fit, and we think these are all the ones in the table),
  // then do so.  Otherwise, store only those values that are
  // significantly more common than the values not in the list.
  //
  // Note: the first of these cases is meant to address columns with
  // small, fixed sets of possible values, such as boolean or enum
  // columns.  If we can *completely* represent the column population by
  // an MCV list that will fit into the stats target, then we should do
  // so and thus provide the planner with complete information.  But if
  // the MCV list is not complete, it's generally worth being more
  // selective, and not just filling it all the way up to the stats
  // target.
  size_t num_mcv;

  if (heap_size == ndistinct && heap_size <= num_buckets) {
    num_mcv = heap_size;
  } else {
    num_mcv = analyze_mcv_list(mcv_list, m_ndistinct, m_null_values_fraction,
                               samplerows, totalrows);
    while (mcv_list.size() > num_mcv) {
      mcv_list.pop_back();
    }
  }

  // We don't care which value is taken as mcv when they share the same repeat
  // count. However, the order of iterating over value_map implies smaller ones.
  // Judging by min_repeat_count and observed occurrences is sufficient to
  // extract mcv from value_map, and this way saves at most 'num_mcv + actual
  // occurrences' lookups which are otheriwse needed to check against the heap.
  ha_rows mcv_min_repeat_count;
  ha_rows mcv_min_repeat_count_occurrences;

  if (num_mcv > 0) {
    mcv_min_repeat_count = mcv_list[num_mcv - 1].second;
    mcv_min_repeat_count_occurrences = 0;
    for (auto it = mcv_list.rbegin(); it != mcv_list.rend(); ++it) {
      if (it->second == mcv_min_repeat_count) {
        mcv_min_repeat_count_occurrences++;
      } else {
        break;
      }
    }
  } else {
    mcv_min_repeat_count = ~0;
    mcv_min_repeat_count_occurrences = 0;
  }

  // Only the first seen occurrences of min_repeat_count are regarded as
  // belonging to the mcv list.
  ha_rows mcv_min_repeat_count_seen = 0;

  // Determine the height of an equi-height bucket.
  double eh_step;
  size_t num_mcv_values = 0;
  for (auto &it : mcv_list) {
    num_mcv_values += it.second;
  }
  eh_step = (double)(num_non_null_values - num_mcv_values) / num_buckets;
  eh_step = std::max(eh_step, 1.0);

  // Generate buckets.
  ha_rows mcv_cumulative_sum = 0;
  ha_rows eh_cumulative_sum = 0;
  ha_rows eh_endpoint_idx = 0;
  for (const auto &node : value_map) {
    bool is_mcv;
    if (node.second > mcv_min_repeat_count) {
      is_mcv = true;
    } else if (node.second == mcv_min_repeat_count) {
      is_mcv = (mcv_min_repeat_count_seen++ < mcv_min_repeat_count_occurrences);
    } else {
      is_mcv = false;
    }

    if (is_mcv) {
      mcv_cumulative_sum += node.second;
      const double cumulative_frequency =
          mcv_cumulative_sum / static_cast<double>(totalrows);
      m_mcv_buckets.emplace(node.first, cumulative_frequency);
    } else {
      eh_cumulative_sum += node.second;
      if (m_equi_height.empty()) {
        m_equi_height.emplace_back(node.first);
        eh_endpoint_idx = 1;
      }
      while (eh_endpoint_idx * eh_step <= eh_cumulative_sum) {
        m_equi_height.emplace_back(node.first);
        eh_endpoint_idx++;
      }
    }
  }
  // Avoid partial bucket.
  if (m_equi_height.size() == 1)
    m_equi_height.emplace_back(m_equi_height.back());

  return false;
}

template <class T>
bool Compressed_histogram<T>::histogram_to_json(
    Json_object *json_object) const {
  // Call the base class implementation first. This will add the properties that
  // are common among different histogram types, such as "last-updated" and
  //  "histogram-type".
  if (Histogram::histogram_to_json(json_object))
    return true; /* purecov: inspected */

  // Add the most common values buckets.
  Json_array mcv_json_buckets;
  for (const auto &bucket : m_mcv_buckets) {
    Json_array json_bucket;
    if (create_json_bucket(bucket, &json_bucket))
      return true; /* purecov: inspected */
    if (mcv_json_buckets.append_clone(&json_bucket))
      return true; /* purecov: inspected */
  }

  if (json_object->add_clone(mcv_buckets_str(), &mcv_json_buckets))
    return true; /* purecov: inspected */

  // Add the other values' endpoints.
  Json_array non_mcv_json_buckets;
  for (const auto &bucket : m_equi_height)
    if (create_json_bucket(bucket, &non_mcv_json_buckets))
      return true; /* purecov: inspected */

  if (json_object->add_clone(other_values_endpoint_str(),
                             &non_mcv_json_buckets))
    return true; /* purecov: inspected */

  const Json_double ndistinct(m_ndistinct);
  if (json_object->add_clone(ndistinct_str(), &ndistinct))
    return true; /* purecov: inspected */

  if (histogram_data_type_to_json(json_object))
    return true; /* purecov: inspected */
  return false;
}

template <class T>
bool Compressed_histogram<T>::create_json_bucket(
    const std::pair<T, double> &bucket, Json_array *json_bucket) {
  // Value
  if (add_value_json_bucket(bucket.first, json_bucket))
    return true; /* purecov: inspected */

  // Cumulative frequency
  const Json_double frequency(bucket.second);
  if (json_bucket->append_clone(&frequency))
    return true; /* purecov: inspected */
  return false;
}

template <class T>
bool Compressed_histogram<T>::create_json_bucket(const T &bucket,
                                                 Json_array *json_bucket) {
  // Value
  if (add_value_json_bucket(bucket, json_bucket))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Compressed_histogram<double>::add_value_json_bucket(
    const double &value, Json_array *json_bucket) {
  const Json_double json_value(value);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Compressed_histogram<String>::add_value_json_bucket(
    const String &value, Json_array *json_bucket) {
  const Json_opaque json_value(enum_field_types::MYSQL_TYPE_STRING, value.ptr(),
                               value.length());
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Compressed_histogram<ulonglong>::add_value_json_bucket(
    const ulonglong &value, Json_array *json_bucket) {
  const Json_uint json_value(value);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Compressed_histogram<longlong>::add_value_json_bucket(
    const longlong &value, Json_array *json_bucket) {
  const Json_int json_value(value);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Compressed_histogram<MYSQL_TIME>::add_value_json_bucket(
    const MYSQL_TIME &value, Json_array *json_bucket) {
  enum_field_types field_type;
  switch (value.time_type) {
    case MYSQL_TIMESTAMP_DATE:
      field_type = MYSQL_TYPE_DATE;
      break;
    case MYSQL_TIMESTAMP_DATETIME:
      field_type = MYSQL_TYPE_DATETIME;
      break;
    case MYSQL_TIMESTAMP_TIME:
      field_type = MYSQL_TYPE_TIME;
      break;
    default:
      /* purecov: begin deadcode */
      assert(false);
      return true;
      /* purecov: end */
  }

  const Json_datetime json_value(value, field_type);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Compressed_histogram<my_decimal>::add_value_json_bucket(
    const my_decimal &value, Json_array *json_bucket) {
  const Json_decimal json_value(value);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <class T>
std::string Compressed_histogram<T>::histogram_type_to_str() const {
  return compressed_str();
}

template <class T>
bool Compressed_histogram<T>::json_to_histogram(const Json_object &json_object,
                                                Error_context *context) {
  // Extract the common histogram attributes from json and post check.
  if (Histogram::json_to_histogram(json_object, context)) return true;
  const Json_dom *buckets_dom = json_object.get(mcv_buckets_str());
  if (buckets_dom == nullptr) {
    context->report_missing_attribute(mcv_buckets_str());
    return true;
  }
  if (buckets_dom->json_type() != enum_json_type::J_ARRAY) {
    context->report_node(buckets_dom, Message::JSON_WRONG_ATTRIBUTE_TYPE);
    return true;
  }

  // Extract mcv list part and post check the json attributes.
  const Json_array *buckets = down_cast<const Json_array *>(buckets_dom);
  for (size_t i = 0; i < buckets->size(); ++i) {
    const Json_dom *bucket_dom = (*buckets)[i];
    if (bucket_dom == nullptr ||
        bucket_dom->json_type() != enum_json_type::J_ARRAY) {
      context->report_node(bucket_dom, Message::JSON_WRONG_ATTRIBUTE_TYPE);
      return true;
    }
    const Json_array *bucket = down_cast<const Json_array *>(bucket_dom);
    assert(!context->binary() || bucket->size() == 2);
    // Only the first two items are defined, others are simply ignored.
    if (bucket->size() < 2) {
      context->report_node(bucket_dom, Message::JSON_WRONG_BUCKET_TYPE_2);
      return true;
    }
    // First item is the value, second is the cumulative frequency
    const Json_dom *cumulative_frequency_dom = (*bucket)[1];
    if (cumulative_frequency_dom->json_type() != enum_json_type::J_DOUBLE) {
      context->report_node(cumulative_frequency_dom,
                           Message::JSON_WRONG_ATTRIBUTE_TYPE);
      return true;
    }
    const Json_double *cumulative_frequency =
        down_cast<const Json_double *>(cumulative_frequency_dom);
    const Json_dom *value_dom = (*bucket)[0];
    T value;
    if (extract_json_dom_value(value_dom, &value, context)) return true;
      // Bucket extraction post-check
#ifdef DBUG_OFF
    if (!context->internal())
#endif
    {
      // Check items in the bucket.
      // Using json data built from a server without fix to BUG#104108, would
      // break the strict check 'cumulative_frequency->value() > 1.0'. To help
      // migration of histograms across servers, the strict check should be
      // loosened.
      // Without touching real data, endpoint values can only be checked against
      // the field definition.
      if (cumulative_frequency->value() < 0.0 ||
          cumulative_frequency->value() > 1.0) {
        context->report_node(cumulative_frequency_dom,
                             Message::JSON_INVALID_FREQUENCY);
        return true;
      }
      if (context->check_value(&value)) {
        context->report_node(value_dom, Message::JSON_VALUE_OUT_OF_RANGE);
        return true;
      }
      // Check endpoint sequence and frequency sequence.
      if (!m_mcv_buckets.empty()) {
        if (!histograms::Histogram_comparator()(m_mcv_buckets.rbegin()->first,
                                                value)) {
          context->report_node(value_dom, Message::JSON_VALUE_NOT_ASCENDING_1);
          return true;
        }
        if (m_mcv_buckets.rbegin()->second >= cumulative_frequency->value()) {
          context->report_node(
              cumulative_frequency_dom,
              Message::JSON_CUMULATIVE_FREQUENCY_NOT_ASCENDING);
          return true;
        }
      }
    }
    m_mcv_buckets.emplace(value, cumulative_frequency->value());
  }

  // Extract other values' part and post check the json attributes.
  buckets_dom = json_object.get(other_values_endpoint_str());
  if (buckets_dom == nullptr) {
    context->report_missing_attribute(other_values_endpoint_str());
    return true;
  }
  if (buckets_dom->json_type() != enum_json_type::J_ARRAY) {
    context->report_node(buckets_dom, Message::JSON_WRONG_ATTRIBUTE_TYPE);
    return true;
  }
  buckets = down_cast<const Json_array *>(buckets_dom);
  for (size_t i = 0; i < buckets->size(); ++i) {
    T value;
    const Json_dom *bucket_dom = (*buckets)[i];
    if (bucket_dom == nullptr) {
      context->report_node(bucket_dom, Message::JSON_WRONG_ATTRIBUTE_TYPE);
      return true;
    }
    if (extract_json_dom_value(bucket_dom, &value, context)) return true;
      // Bucket extraction post-check
#ifdef DBUG_OFF
    if (!context->internal())
#endif
    {
      if (context->check_value(&value)) {
        context->report_node(bucket_dom, Message::JSON_VALUE_OUT_OF_RANGE);
        return true;
      }
      // Check endpoint sequence and frequency sequence.
      if (!m_equi_height.empty()) {
        // The same value might be present as adjacent endpoints.
        if (histograms::Histogram_comparator()(value,
                                               *m_equi_height.rbegin())) {
          context->report_node(bucket_dom, Message::JSON_VALUE_NOT_ASCENDING_1);
          return true;
        }
      }
    }
    m_equi_height.emplace_back(value);
  }

  // ndistinct
  const Json_dom *ndistinct_dom = json_object.get(ndistinct_str());
  if (ndistinct_dom == nullptr) {
    context->report_missing_attribute(ndistinct_str());
    return true;
  }
  if (ndistinct_dom->json_type() != enum_json_type::J_DOUBLE) {
    context->report_node(ndistinct_dom, Message::JSON_WRONG_ATTRIBUTE_TYPE);
    return true;
  }
  const Json_double *ndistinct = down_cast<const Json_double *>(ndistinct_dom);
  m_ndistinct = ndistinct->value();

  // Global post-check after extract mcv list part and other values' part.
#ifdef DBUG_OFF
  if (!context->internal())
#endif
  {
    // Check the mcv list part cumulative frequency sum.
    if (m_mcv_buckets.empty()) {
      if (get_null_values_fraction() != 1.0 &&
          get_null_values_fraction() != 0.0) {
        context->report_global(Message::JSON_INVALID_TOTAL_FREQUENCY);
        return true;
      }
    } else if (std::abs(m_mcv_buckets.rbegin()->second +
                        get_null_values_fraction()) > 1.0) {
      context->report_global(Message::JSON_INVALID_TOTAL_FREQUENCY);
      return true;
    }
  }

  return false;
}

template <class T>
Histogram *Compressed_histogram<T>::clone(MEM_ROOT *mem_root) const {
  DBUG_EXECUTE_IF("fail_histogram_clone", return nullptr;);
  bool error = false;
  try {
    return new (mem_root) Compressed_histogram<T>(mem_root, *this, &error);
  } catch (const std::bad_alloc &) {
    return nullptr; /* purecov: deadcode */
  }
}

template <class T>
static bool values_are_equal(const T &val1, const T &val2) {
  return (!Histogram_comparator()(val1, val2) &&
          !Histogram_comparator()(val2, val1));
}

template <class T>
double Compressed_histogram<T>::get_equal_to_selectivity(const T &value) const {
  // The value might be an mcv value or not. In the former case, just answer
  // with the mcv frequency, since any mcv is excluded from equi-height.
  // In the latter case, because equi-height does not answer equal-to (it does
  // answer less-than-or-equal-to), the other distinct is explored.

  const auto found = m_mcv_buckets.lower_bound(value);
  if (found != m_mcv_buckets.end() && values_are_equal(value, found->first)) {
    if (found == m_mcv_buckets.begin())
      return found->second;
    else {
      const auto previous = std::prev(found, 1);
      return found->second - previous->second;
    }
  }

  if (m_ndistinct - m_mcv_buckets.size() < 1 || m_equi_height.empty())
    return 0.0;

  double mcv_sel = m_mcv_buckets.empty() ? 0.0 : m_mcv_buckets.rbegin()->second;
  double other_sel = get_non_null_values_fraction() - mcv_sel;

  // TODO The fact of out-of-bound might be stale, and kind of heursitic could
  // be helpful in some cases.
  if (Histogram_comparator()(value, *m_equi_height.begin()) ||
      Histogram_comparator()(*m_equi_height.rbegin(), value))
    return 0.0;
  else
    return other_sel / (double)(m_ndistinct - m_mcv_buckets.size());
}

template <class T>
double Compressed_histogram<T>::get_less_than_equal_selectivity(
    const T &value) const {
  // 'x <= v' is the primary operation provided by an equi-height histogram.

  // Both mcv list and equi height should be examined to estimate the range.
  // However, interpolaton applies only to equi height, it does not apply to
  // mcv list.

  double sel = 0.0;
  double mcv_sel = m_mcv_buckets.empty() ? 0.0 : m_mcv_buckets.rbegin()->second;

  const auto found = m_mcv_buckets.lower_bound(value);
  if (found != m_mcv_buckets.end() && values_are_equal(value, found->first)) {
    // 'x <= v', when v = found
    sel = found->second;
  } else if (found == m_mcv_buckets.end()) {
    // 'x <= mcv_max', when v is bigger than any mcv
    sel = mcv_sel;
  } else if (found != m_mcv_buckets.begin()) {
    // 'x <= a', when a < v < found
    sel = std::prev(found)->second;
  }

  if (m_ndistinct - m_mcv_buckets.size() < 1 || m_equi_height.empty())
    return sel;

  double other_sel = get_non_null_values_fraction() - mcv_sel;
  double other_distinct = m_ndistinct - m_mcv_buckets.size();
  double bin_sel = other_sel;
  if (m_equi_height.size() >= 2)
    bin_sel = other_sel / (m_equi_height.size() - 1);

  const auto found_2 =
      std::lower_bound(m_equi_height.begin(), m_equi_height.end(), value,
                       Histogram_comparator());
  if (found_2 != m_equi_height.end() && values_are_equal(value, *found_2)) {
    // When value is an endpoint, just count up to it, inclusively.
    double bins = std::distance(m_equi_height.begin(), found_2);
    sel += bin_sel * bins;
    // However, when value is endpoint 0, "x <= ep0" is same to "x = ep0".
    if (found_2 == m_equi_height.begin()) {
      sel += (other_sel / other_distinct);
    }
  } else if (found_2 == m_equi_height.end()) {
    // When value is beyond the end, count the whole equi-height histogram.
    sel += other_sel;
  } else if (found_2 != m_equi_height.begin()) {
    // Value is not an endpoint, we need to count all full buckets and
    // interpolate the partial one.
    auto lower_endpoint = std::prev(found_2);
    double bins = std::distance(m_equi_height.begin(), lower_endpoint);
    sel += bin_sel * bins;

    // Even distribution is assumed within a bucket, and it is common pratice
    // to do linear interpolation.
    double ratio;
    if (values_are_equal(*lower_endpoint, *found_2)) {
      if (values_are_equal(value, *lower_endpoint))
        ratio = 1.0;
      else
        ratio = 0.0;
    } else {
      ratio = interpolate(value, *lower_endpoint, *found_2);
    }

    double frac_sel = bin_sel;
    // Note that endpoint 0 is also part of the first bucket. It should be
    // excluded when interpolating the first bucket.
    if (lower_endpoint == m_equi_height.begin())
      frac_sel -= (other_sel / other_distinct);
    frac_sel *= ratio;

    sel += frac_sel;
  }

  return sel;
}

template <class T>
double Compressed_histogram<T>::get_greater_than_selectivity(
    const T &value) const {
  const double less_than_equal = get_less_than_equal_selectivity(value);

  double sel = get_non_null_values_fraction() -
               std::min(less_than_equal, get_non_null_values_fraction());
  sel = std::max(sel, 0.0);
  sel = std::min(sel, 1.0);
  return sel;
}

template <class T>
double Compressed_histogram<T>::get_less_than_selectivity(
    const T &value) const {
  const double less_than_equal = get_less_than_equal_selectivity(value);
  const double equal_to = get_equal_to_selectivity(value);

  double sel = less_than_equal - equal_to;
  sel = std::max(sel, 0.0);
  sel = std::min(sel, 1.0);
  return sel;
}

template <class T>
double Compressed_histogram<T>::interpolate(const T &value, const T &lower,
                                            const T &upper) const {
  double val_double = (double)value;
  double low_double = (double)lower;
  double upp_double = (double)upper;

  return (val_double - low_double) / (upp_double - low_double);
}

template <>  // From PG algorithm.
double Compressed_histogram<String>::interpolate(const String &value,
                                                 const String &lower,
                                                 const String &upper) const {
  int range_low = 0;
  int range_upp = 0;
  const char *sptr = nullptr;
  const char *bound_low = lower.ptr();
  const char *bound_upp = upper.ptr();
  const char *pval = value.ptr();

  range_low = range_upp = (unsigned char)upper.ptr()[0];
  for (sptr = bound_low; *sptr; sptr++) {
    if (range_low > (unsigned char)*sptr) range_low = (unsigned char)*sptr;
    if (range_upp < (unsigned char)*sptr) range_upp = (unsigned char)*sptr;
  }
  for (sptr = bound_upp; *sptr; sptr++) {
    if (range_low > (unsigned char)*sptr) range_low = (unsigned char)*sptr;
    if (range_upp < (unsigned char)*sptr) range_upp = (unsigned char)*sptr;
  }
  // If range includes any upper-case ASCII chars, make it include all.
  if (range_low <= 'Z' && range_upp >= 'A') {
    if (range_low > 'A') range_low = 'A';
    if (range_upp < 'Z') range_upp = 'Z';
  }
  // Ditto lower-case.
  if (range_low <= 'z' && range_upp >= 'a') {
    if (range_low > 'a') range_low = 'a';
    if (range_upp < 'z') range_upp = 'z';
  }
  // Ditto digits.
  if (range_low <= '9' && range_upp >= '0') {
    if (range_low > '0') range_low = '0';
    if (range_upp < '9') range_upp = '9';
  }

  // If range includes less than 10 chars, assume we have not got enough
  // data, and make it include regular ASCII set.
  if (range_upp - range_low < 9) {
    range_low = ' ';
    range_upp = 127;
  }

  // Now strip any common prefix of the three strings.
  while (*bound_low) {
    if (*bound_low != *bound_upp || *bound_low != *sptr) break;
    bound_low++, bound_upp++, sptr++;
  }

  double val_double = convert_string_to_scalar(pval, range_low, range_upp);
  double low_double = convert_string_to_scalar(bound_low, range_low, range_upp);
  double upp_double = convert_string_to_scalar(bound_upp, range_low, range_upp);

  return (val_double - low_double) / (upp_double - low_double);
}

template <>
double Compressed_histogram<MYSQL_TIME>::interpolate(
    const MYSQL_TIME &value, const MYSQL_TIME &lower,
    const MYSQL_TIME &upper) const {
  longlong val_packed = TIME_to_longlong_packed(value);
  longlong low_packed = TIME_to_longlong_packed(lower);
  longlong upp_packed = TIME_to_longlong_packed(upper);

  return ((double)val_packed - (double)low_packed) /
         ((double)upp_packed - (double)low_packed);
}

template <>
double Compressed_histogram<my_decimal>::interpolate(
    const my_decimal &value, const my_decimal &lower,
    const my_decimal &upper) const {
  double val_double;
  double low_double;
  double upp_double;

  my_decimal2double(0, &value, &val_double);
  my_decimal2double(0, &lower, &low_double);
  my_decimal2double(0, &upper, &upp_double);

  return (val_double - low_double) / (upp_double - low_double);
}

// Explicit template instantiations.
template class Compressed_histogram<double>;
template class Compressed_histogram<String>;
template class Compressed_histogram<ulonglong>;
template class Compressed_histogram<longlong>;
template class Compressed_histogram<MYSQL_TIME>;
template class Compressed_histogram<my_decimal>;

}  // namespace histograms

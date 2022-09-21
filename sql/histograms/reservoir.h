#ifndef HISTOGRAMS_SAMPLING_INCLUDED
#define HISTOGRAMS_SAMPLING_INCLUDED

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
  @file sql/histograms/reservoir.h
*/

#include <random>

namespace histograms {

/**
  Implement Algorithm Z from "Random sampling with a reservoir" by Jeffrey
  S. Vitter, in ACM Trans. Math. Softw. 11, 1 (Mar. 1985), Pages 37-57.

  A reservoir algorithm is defined as follows:

  Definition 1. The first step of any reservoir algorithm is to put the first
  n records of the file into a "reservoir." The rest of the records are
  processed sequentially; records can be selected for the reservoir only as
  they are processed. An algorithm is a reservoir algorithm if it maintains
  the invariant that after each record is processed a true random sample of
  size n can be extracted from the current state of the reservoir.

  Vitter describes his algorithm in terms of the count S of records to skip
  before processing another record. With following notations,

    t - the number of records already read
    n - the size of the true random sample (the reservoir)
    W - an extra state between calls computing S for Algorithm Z

  The first n records are just added to the reservoir. For any more record,
  increment t. In addition, if it is not skipped, obtain a new S to skip next
  records, and replace one in the reservoir at random.

  S is primarily based on t. When t is not large enough Algorithm X is applied,
  othewise, Algorithm Z is applied with the extra state W between calls
  computing S.

  Example:

    while not eof
      if !reservoir.is_full()
        append to reservoir
      else
        key = reservoir.choose_element()
        if (key >= 0)
          replace into reservoir
      reservoir.increment_processed();

  The optimized Algorithm Z consists of three parts:

  1) Make the first n records candidates for the sample.

    for j := 0 to n - 1
      READ_NEXT_RECORD(C[j]);
    t := n;

  2) Process records using the method of Algorithm X until t is large enough.
     For each record, compute S to skip over next S records, and if the current
     one is not skipped, replace one in the reservoir at random.

    thresh := T * n;  // When T is initialized to 22, performance is best
    num := 0;         // num is equal to t - n

    while not eof and (t <= thresh)
      V := RANDOM();

      S := 0;
      t := t + 1;
      num := num + 1;
      quot := num / t;

      while quot > V               // Find min S satisfying (4.1)
        S := S + 1;
        t := t + 1;
        num := num + 1;
        quot := (quot * num) / t;

      SKIP_RECORDS(S)              // Skip over the next S records
      if not eof
        // Make the next record a candidate, replacing one at random
        M := TRUNC(n * RANDOM());  // M is uniform in the range 0 <= M <= n - 1
        READ_NEXT_RECORD(C[M]);

  3) Process the rest of the records using the rejection technique.

    W := EXP(- LOG(RANDOM()) / n);  // Generate W
    term := t - n + 1;              // term is always equal to t - n + 1
    while not eof
      loop
        U := RANDOM();
        X := t * (W - 1.0);

        S := TRUNC(X);              // S is tentatively set to floor(X)

        // Test if U <= h(S)/cg(X) in the manner of (6.3)
        lhs := EXP(LOG((U * (((t + 1) / term) ^ 2)) * (term + S)) / (t + X)) /
  n; rhs := (((t + X) / (term + S)) * term) / t if lhs <= rhs W = rhs / lhs;
          break-to-end-loop

        // Test if U <= f(S)/cg(X)
        y := (((U * (t + 1)) / term) * (t + S + 1)) / (t + X);
        if n < S
          denom := t;
          numer_lim := term + S;
        else
          denom := t - n + S;
          numer_lim := t + 1;

        for numer := t + S downto number_lim
          y := (y * numer) / denom;
          denom := denom - 1;

        W := EXP(- LOG(RANDOM()) / n);  // Generate W in advance
      end-loop

      SKIP_RECORDS(S)  // Skip over the next S records
      if not eof
        // Make the next record a candidate, replacing one at random
        M := TRUNC(n * RANDOM());  // M is uniform in the range 0 <= M <= n - 1
        READ_NEXT_RECORD(C[M])

      t := t + S + 1
      term := term + S + 1

 */
class Reservoir_state {
 public:
  typedef long long key_t;

  /**
    Initialize the random generator with provided seed, and compute initial
    W value.

    @param n     Capacity of the reservoir.
    @param seed  Seed for the random generator.
   */
  void init(size_t n, int seed);

  /**
    Initialize the random generator with default seed, and compute initial
    W value.

    @param n     Capacity of the reservoir.
   */
  void init(size_t n);

  bool is_full() const { return m_n > 0 && m_n <= m_t; }

  /// Choose a random element to replace, or -1 for skip.
  key_t choose_element();

  /// Increment the number of records already processed.
  void increment_processed() { m_t += 1; }

  /// Return the real size of the reservoir.
  size_t size() const { return (size_t)(m_t < m_n ? m_t : m_n); }

  /// Return the number of records already processed.
  size_t num_processed() const { return m_t; }

 private:
  /// Return a uniform random variate in the range (0, 1).
  double random();

  /// Determine the number of records to skip before the next record is
  /// processed.
  double get_next_S();

 private:
  /// Size of the true random sample.
  int m_n;

  /// The number of records processed so far.
  int m_t;

  /// The number of records to skip before the next records is processed.
  double m_num_to_skip;

  /// Random variate for Algorithm Z.
  double m_W;

  /// Random generator.
  std::mt19937 m_gen;
};

}  // namespace histograms

#endif

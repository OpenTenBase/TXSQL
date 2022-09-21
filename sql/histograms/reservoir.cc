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
  @file sql/histograms/reservoir.cc
*/

#include "sql/histograms/reservoir.h"

#include "my_dbug.h"

namespace histograms {

static std::uniform_real_distribution<double> s_distribution(0.0, 1.0);

double Reservoir_state::random() {
  double res;

  do {
    // The interval is [0.0, 1.0), so we must reject 0.
    res = s_distribution(m_gen);
  } while (res == 0.0);

  return res;
}

// n sample size
// seed - for the random generator
void Reservoir_state::init(size_t n, int seed) {
  assert(n > 0);
  m_n = static_cast<int>(n);
  m_t = 0;
  m_num_to_skip = -1;

  m_gen.seed(seed);
  m_W = exp(-log(random()) / n);
}

void Reservoir_state::init(size_t n) {
  assert(n > 0);
  m_n = static_cast<int>(n);
  m_t = 0;
  m_num_to_skip = -1;

  m_gen.seed();
  m_W = exp(-log(random()) / n);
}

// Callback per record to get replacement before incrementing t
long long Reservoir_state::choose_element() {
  assert(is_full());

  if (m_num_to_skip < 0) {
    // Call the algorithm to get the number of records to skip.
    m_num_to_skip = get_next_S();
  }

  DBUG_EXECUTE_IF("set_m_rows_to_skip_1", { m_num_to_skip = 1; });
  DBUG_EXECUTE_IF("set_m_rows_to_skip_0", { m_num_to_skip = 0; });

  long long key = -1;
  if (m_num_to_skip <= 0) {
    // Choose a random record to replace with the current one.
    key = (long long)(random() * m_n);
    assert(key >= 0 && key < m_n);
  }

  // Update the skip-over counter.
  m_num_to_skip -= 1;

  return key;
}

double Reservoir_state::get_next_S() {
  double S;

  int n = m_n;
  int t = m_t;

  // In Vitter's paper, the constant T is initialized to 22 for best performance
  const double thresh = 22 * n;

  DBUG_EXECUTE_IF("use_algorithm_z", t = thresh + 1;);
  if (t <= thresh) {
    // Process records using the method of Algorithm X until t is large enough
    double v, quot;
    v = random();
    S = 0;
    t += 1;
    // In Vitter's paper, num is always equal to t-n and quot = num/t
    quot = (t - (double)n) / t;
    // Find min S satisfying (4.1)
    while (quot > v) {
      S += 1;
      t += 1;
      quot *= (t - (double)n) / t;
    }
  } else {
    // Process the rest of the records using the rejection technique

    double W = m_W;

    // term is always equal to t - n + 1
    double term = t - (double)n + 1;
    for (;;) {
      double U, X, lhs, rhs, y, tmp1, tmp2;
      double numer, numer_lim, denom;

      // Generate U and X
      U = random();
      X = t * (W - 1.0);

      // S is tentatively set to floor(X)
      S = floor(X);

      // Test if U <= h(S)/cg(x) in the manner of (6.3)
      tmp1 = (t + 1) / term;
      tmp2 = term + S;
      lhs = exp(log(((U * tmp1 * tmp1) * tmp2) / (t + X)) / n);
      rhs = (((t + X) / tmp2) * term) / t;
      if (lhs <= rhs) {
        W = rhs / lhs;
        break;
      }
      // Test if U <= f(S)/cg(X)
      y = (((U * (t + 1)) / term) * (t + S + 1)) / (t + X);
      if ((double)n < S) {
        denom = t;
        numer_lim = term + S;
      } else {
        denom = t - (double)n + S;
        numer_lim = t + 1;
      }
      for (numer = t + S; numer >= numer_lim; numer -= 1) {
        y = (y * numer) / denom;
        denom -= 1;
      }

      // Generate W in advance
      W = exp(-log(random()) / n);
      if (exp(log(y) / n) <= (t + X) / t) {
        break;
      }
    }

    m_W = W;
  }

  return S;
}

}  // namespace histograms

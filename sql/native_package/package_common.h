/* Copyright (c) 2018, 2021, Alibaba and/or its affiliates. All rights reserved.
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.
   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL/Apsara GalaxyEngine hereby grant you an
   additional permission to link the program and your derivative works with the
   separately licensed software that they have included with
   MySQL/Apsara GalaxyEngine.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef SQL_PACKAGE_PACKAGE_COMMON_INCLUDED
#define SQL_PACKAGE_PACKAGE_COMMON_INCLUDED
#include "map_helpers.h"

/**
  Common definition of package module.

   - Memory usage detection
   - Native package object container structure
*/
namespace im {

/* Package memory P_S key */
extern PSI_memory_key key_memory_package;

extern const char *PACKAGE_SCHEMA;

/**
  PSI memory detect interface;
*/
class PSI_memory_base {
 public:
  PSI_memory_base(PSI_memory_key key) : m_key(key) {}

  virtual ~PSI_memory_base() {}

  /* Getting */
  PSI_memory_key psi_key() { return m_key; }

  /* Setting */
  void set_psi_key(PSI_memory_key key) { m_key = key; }

 private:
  PSI_memory_key m_key;
};

/* Disable the copy and assign construct */
class Disable_copy_base {
 public:
  Disable_copy_base() {}
  virtual ~Disable_copy_base() {}

 private:
  Disable_copy_base(const Disable_copy_base &);
  Disable_copy_base(const Disable_copy_base &&);
  Disable_copy_base &operator=(const Disable_copy_base &);
};

/*
  Pair key map definition
*/
template <typename F, typename S>
using Pair_key_type = std::pair<F, S>;

template <typename F, typename S>
class Pair_key_comparator {
 public:
  bool operator()(const Pair_key_type<F, S> &lhs,
                  const Pair_key_type<F, S> &rhs) const;
};

template <typename F, typename S, typename T>
class Pair_key_unordered_map
    : public malloc_unordered_map<Pair_key_type<F, S>, const T *,
                                  std::hash<Pair_key_type<F, S>>,
                                  Pair_key_comparator<F, S>> {
 public:
  explicit Pair_key_unordered_map(PSI_memory_key key)
      : malloc_unordered_map<Pair_key_type<F, S>, const T *,
                             std::hash<Pair_key_type<F, S>>,
                             Pair_key_comparator<F, S>>(key) {}
};

template <typename F, typename S>
struct Pair_key_icase_hash {
 public:
  typedef Pair_key_type<F, S> argument_type;
  typedef size_t result_type;

  size_t operator()(const Pair_key_type<F, S> &p) const;
};

template <typename F, typename S>
class Pair_key_icase_comparator {
 public:
  bool operator()(const Pair_key_type<F, S> &lhs,
                  const Pair_key_type<F, S> &rhs) const;
};

template <typename F, typename S, typename T>
class Pair_key_icase_unordered_map
    : public malloc_unordered_map<Pair_key_type<F, S>, const T *,
                                  Pair_key_icase_hash<F, S>,
                                  Pair_key_icase_comparator<F, S>> {
 public:
  explicit Pair_key_icase_unordered_map(PSI_memory_key key)
      : malloc_unordered_map<Pair_key_type<F, S>, const T *,
                             Pair_key_icase_hash<F, S>,
                             Pair_key_icase_comparator<F, S>>(key) {}
};

/* Package element map type */
template <typename T>
using Package_element_map =
    Pair_key_icase_unordered_map<std::string, std::string, T>;

} /*  namespace im */

#endif

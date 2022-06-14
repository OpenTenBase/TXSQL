#ifndef PX_ITEM_INCLUDED
#define PX_ITEM_INCLUDED
/* Copyright (c) 2000, 2020, Oracle and/or its affiliates.

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


#include "sql/item.h"
#include "sql/item_sum.h"
#include "sql/sql_optimizer.h"
#include "sql/sql_executor.h"
#include "sql/log.h"


static const Item_sum::Sumfunctype PQ_SUPPORT_AGGR_FUNC[] = {
  Item_sum::SUM_FUNC,
  Item_sum::COUNT_FUNC,
  Item_sum::AVG_FUNC,
  Item_sum::MIN_FUNC,
  Item_sum::MAX_FUNC
};

extern bool check_px_unsafe_item(Item *item);

bool check_xchg_unsafe_field(const Field *field);

bool check_px_unsafe_sum_funcs(JOIN *join);

bool check_xchg_unsafe_sum_funcs(JOIN *join);

bool check_px_unsafe_projector(JOIN *join);

bool check_px_unsafe_group(List<Cached_item> &group_field);

bool check_px_unsafe_order(JOIN *join, ORDER *order, uint ref_slice);

bool check_px_unsafe_cond(JOIN *join, Item *cond, uint ref_slice);

bool check_px_unsafe_temp_param(const Temp_table_param *param);

bool check_xchg_unsafe_temp_param(const Temp_table_param *param);

#endif /* PX_ITEM_INCLUDED */
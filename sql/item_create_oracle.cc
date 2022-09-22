/*
   Copyright (c) 2000, 2021, Tencent and/or its affiliates.

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
  @file sql/item_create_oracle.cc

  Functions to create an item. Used by sql_yacc.yy
*/

class Nvl_instantiator {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Item_func_ifnull(POS(), (*args)[0], (*args)[1], true);
  }
};

class To_number_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1: {
        return new (thd->mem_root) Item_typecast_signed(POS(), (*args)[0], true);
      }
      case 2: {
        return new (thd->mem_root) Item_typecast_decimal(POS(), (*args)[0], (*args)[1], true);
      }
      default:
        assert(false);
        return nullptr;
    }
  }
};

class To_char_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1: {
        return new (thd->mem_root) Item_typecast_char(POS(), (*args)[0],
                                                      thd->variables.max_allowed_packet,
                                                      thd->variables.collation_connection, true);
      }
      case 2: {
        return new (thd->mem_root) Item_func_date_format(POS(), (*args)[0], (*args)[1], Item_func_date_format::TO_CHAR);
      }
      default:
        assert(false);
        return nullptr;
    }
  }
};

class To_date_instantiator {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Item_func_str_to_date(POS(), (*args)[0], (*args)[1], true);
  }
};

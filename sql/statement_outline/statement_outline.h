/*  Copyright (c) 2021, Tencent and/or its affiliates.

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
#ifndef STATEMENT_OUTLINE_INCLUDED
#define STATEMENT_OUTLINE_INCLUDED

/**
  @file statement_outline/statement_outline.h

  The interface declaration part of the Statement Outline. outside modules
  should only include this header.
*/

class THD;

namespace statement_outline {
/**
  @sa Sys_statement_outline_enabled.
*/
extern bool sys_var_enabled;
/**
  @sa Sys_statement_outline_partitions.
*/
extern uint sys_var_partitions;

/**
  This function is called by parser to apply outline rules in memory.
*/
void apply_outline_rules(THD *thd, bool digest_computed);

bool init_statement_outline();
void destroy_statement_outline();

/**
  Reload rules from rule table in disk.

  It will write log if fail to load.
*/
void reload_outline_rules();
}
#endif /* STATEMENT_OUTLINE_INCLUDED */

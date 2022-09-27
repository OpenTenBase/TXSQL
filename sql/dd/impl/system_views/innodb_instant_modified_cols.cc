/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/innodb_instant_modified_cols.h"

namespace dd {
namespace system_views {

const Innodb_instant_modified_cols &Innodb_instant_modified_cols::instance() {
  static Innodb_instant_modified_cols *s_instance =
      new Innodb_instant_modified_cols();
  return *s_instance;
}

Innodb_instant_modified_cols::Innodb_instant_modified_cols() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_TABLE_ID, "TABLE_ID",
                         "CONVERT(GET_DD_COLUMN_PRIVATE_DATA(se_private_data, "
                         "'table_id'),char(100) character set utf8mb4)");
  m_target_def.add_field(FIELD_NAME, "NAME", "name");
  m_target_def.add_field(FIELD_POS, "POS", "ordinal_position");
  m_target_def.add_field(FIELD_VERSION, "VERSION",
                         "CONVERT(GET_DD_COLUMN_PRIVATE_DATA(se_private_data, "
                         "'version'),char(100) character set utf8mb4)");
  m_target_def.add_field(FIELD_MTYPE, "MTYPE",
                         "CONVERT(GET_DD_COLUMN_PRIVATE_DATA(se_private_data, "
                         "'mtype'),char(100) character set utf8mb4)");
  m_target_def.add_field(FIELD_PRTYPE, "PRTYPE",
                         "CONVERT(GET_DD_COLUMN_PRIVATE_DATA(se_private_data, "
                         "'prtype'),char(100) character set utf8mb4)");
  m_target_def.add_field(FIELD_LEN, "LEN",
                         "CONVERT(GET_DD_COLUMN_PRIVATE_DATA(se_private_data, "
                         "'len'),char(100) character set utf8mb4)");

  m_target_def.add_from("mysql.columns");

  m_target_def.add_where("instr(se_private_data, 'modified_version') <> 0");
}
}  // namespace system_views
}  // namespace dd

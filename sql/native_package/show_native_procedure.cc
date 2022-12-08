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


#include "my_config.h"
#include "sql/derror.h"  // ER_THD

#include "sql/native_package/package.h"
#include "sql/native_package/package_common.h"
#include "sql/native_package/show_native_procedure.h"

#include "sql/protocol.h"

// See also sql_prepare(), which also refers to this function in client.cc
const char *fieldtype2str(enum enum_field_types type);

namespace im {

LEX_CSTRING ADMIN_PROC_SCHEMA = {STRING_WITH_LEN("dbms_admin")};

Proc *Show_native_procedure_proc::instance() {
  static Proc *proc = new Show_native_procedure_proc();
  return proc;
}

Sql_cmd *Show_native_procedure_proc::evoke_cmd(THD *thd,
                                               mem_root_deque<Item *> *items) const {
  return new (thd->mem_root) Sql_cmd_type(thd, items, this);
}

bool Sql_cmd_show_native_procedure::pc_execute(THD *) {
  DBUG_TRACE;
  return false;
}

const std::string sql_command_enum_to_string(enum_sql_command e) {
  switch (e) {
    case SQLCOM_ADMIN_PROC:
      return "ADMIN_PROC";
    case SQLCOM_TRANS_PROC:
      return "TRANS_PROC";
    default:
      /* Not allow other sql command other than ADMIN_PROC and TRANS_PROC. */
      assert(0);
      return "UNKNOWN";
  }
}

static std::string field_type_enum_to_string(const enum_field_types e) {
  return fieldtype2str(e);
}

void Sql_cmd_show_native_procedure::send_result(THD *thd, bool error) {
  DBUG_TRACE;
  Protocol *protocol = thd->get_protocol();

  if (error) {
    assert(thd->is_error());
    return;
  }

  if (m_proc->send_result_metadata(thd)) return;

  auto all_m_proc_map = Package::instance()->get_all_element<Proc>();

  std::string schema_name_str, element_name_str, sql_command_code_str,
      params_str;

  const Proc *procit;
  const Sql_cmd *sqlcmdit;

  using mproc_pairkey = Package_element_map<Proc>::key_type;
  std::set<mproc_pairkey> sorted_proc_schema_name;

  for (auto it = all_m_proc_map->cbegin(); it != all_m_proc_map->cend(); ++it) {
    sorted_proc_schema_name.insert(
        mproc_pairkey(it->first.first, it->first.second));
  }

  for (auto it = sorted_proc_schema_name.begin();
       it != sorted_proc_schema_name.end(); ++it) {
    schema_name_str = it->first;
    element_name_str = it->second;
    procit =
        all_m_proc_map->find(mproc_pairkey(schema_name_str, element_name_str))
            ->second;

    /**
      Evoke the sql_cmd object to get sql_command_code, notice that we
      use nullptr to mock mem_root_deque<Item *> items, so the evoked
      sql_cmd cann't be called here.
    */
    sqlcmdit = procit->evoke_cmd(thd, nullptr);
    sql_command_code_str =
        sql_command_enum_to_string(sqlcmdit->sql_command_code());

    params_str = "";
    Proc::Parameters_list parameters_list;
    if (procit->get_parameters_list().size() == 0) {
      parameters_list.push_back(procit->get_parameters());
    } else {
      parameters_list = procit->get_parameters_list();
    }

    int index = 0;
    for (auto parameters : parameters_list) {
      if (index) params_str += " / ";

      std::size_t param_size = parameters->size();
      if (param_size == 0)
        params_str += "NULL";
      else {
        params_str += field_type_enum_to_string(parameters->at(0));
        if (param_size > 1)
          for (std::size_t ii = 1; ii < param_size; ii++) {
            params_str += ", ";
            params_str += field_type_enum_to_string(parameters->at(ii));
          }
      }

      index++;
    }

    protocol->start_row();
    protocol->store_string(schema_name_str.c_str(), schema_name_str.length(),
                           system_charset_info);
    protocol->store_string(element_name_str.c_str(), element_name_str.length(),
                           system_charset_info);
    protocol->store_string(sql_command_code_str.c_str(),
                           sql_command_code_str.length(), system_charset_info);
    protocol->store_string(params_str.c_str(), params_str.length(),
                           system_charset_info);
    if (protocol->end_row()) return;
  }

  my_eof(thd);
}

} /* namespace im */

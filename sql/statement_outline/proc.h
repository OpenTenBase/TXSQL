/* Copyright (c) 2021, Tencent and/or its affiliates.

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

#ifndef STATEMENT_OUTLINE_PROC_INCLUDED
#define STATEMENT_OUTLINE_PROC_INCLUDED

/// @file sql/statement_outline/proc.h Implementation of native package
/// procedures for statement_outline, @sa native_package/proc.h

#include "sql/native_package/proc.h"

namespace statement_outline {
extern const char *proc_qname;
class Sql_cmd_rules_flush : public im::Sql_cmd_admin_proc {
 public:
  explicit Sql_cmd_rules_flush(THD *thd, mem_root_deque<Item *> *list,
                               const im::Proc *proc)
      : im::Sql_cmd_admin_proc(thd, list, proc) {
    set_priv_type(Priv_type::PRIV_NONE_ACL);
  }
  virtual bool pc_execute(THD *thd) override;
};

class Rules_flush : public im::Proc, public im::Disable_copy_base {
  using Sql_cmd_type = Sql_cmd_rules_flush;
 public:
  explicit Rules_flush(PSI_memory_key key);
  ~Rules_flush();
  virtual const std::string qname() const override { return proc_qname; }
  static Proc *instance();
  virtual Sql_cmd *evoke_cmd(THD *thd,
                             mem_root_deque<Item *> *list) const override;

  virtual const std::string str() const override {
    return "statement_outline_flush_rules";
  }
};

class Sql_cmd_rule_add : public im::Sql_cmd_admin_proc {
  longlong added_rule_id{-1};
 public:
  explicit Sql_cmd_rule_add(THD *thd, mem_root_deque<Item *> *list,
                               const im::Proc *proc)
      : im::Sql_cmd_admin_proc(thd, list, proc) {
    set_priv_type(Priv_type::PRIV_NONE_ACL);
  }

  virtual bool pc_execute(THD *thd) override;
  virtual void send_result(THD *thd, bool error) override;
};

class Rule_add : public im::Proc, public im::Disable_copy_base {
  using Sql_cmd_type = Sql_cmd_rule_add;
 public:
  explicit Rule_add(PSI_memory_key key) : Proc(key) {
    m_result_type = Result_type::RESULT_SET;
    m_columns.assign_at(0, {MYSQL_TYPE_LONGLONG, STRING_WITH_LEN("ID"), 0});
    m_parameters.assign_at(0, MYSQL_TYPE_VARCHAR);  // Datbase name
    m_parameters.assign_at(1, MYSQL_TYPE_VARCHAR);  // Query with hints
  }
  ~Rule_add(){}
  virtual const std::string qname() const override { return proc_qname; }
  static Proc *instance();
  virtual Sql_cmd *evoke_cmd(THD *thd,
                             mem_root_deque<Item *> *list) const override;

  virtual const std::string str() const override { return "statement_outline_add_rule"; }
};

class Sql_cmd_rule_delete : public im::Sql_cmd_admin_proc {
 public:
  explicit Sql_cmd_rule_delete(THD *thd, mem_root_deque<Item *> *list,
                               const im::Proc *proc)
      : im::Sql_cmd_admin_proc(thd, list, proc) {
    set_priv_type(Priv_type::PRIV_NONE_ACL);
  }

  virtual bool pc_execute(THD *thd) override;
};

class Rule_delete : public im::Proc, public im::Disable_copy_base {
  using Sql_cmd_type = Sql_cmd_rule_delete;
 public:
  explicit Rule_delete(PSI_memory_key key) : Proc(key) {
    m_result_type = Result_type::RESULT_OK;
    m_parameters.assign_at(0, MYSQL_TYPE_LONGLONG);  // Rule id
  }
  ~Rule_delete(){}
  virtual const std::string qname() const override { return proc_qname; }
  static Proc *instance();

  virtual Sql_cmd *evoke_cmd(THD *thd,
                             mem_root_deque<Item *> *list) const override;

  virtual const std::string str() const override { return "statement_outline_delete_rule"; }
};

class Sql_cmd_rule_enable : public im::Sql_cmd_admin_proc {
 public:
  explicit Sql_cmd_rule_enable(THD *thd, mem_root_deque<Item *> *list,
                               const im::Proc *proc)
      : im::Sql_cmd_admin_proc(thd, list, proc) {
    set_priv_type(Priv_type::PRIV_NONE_ACL);
  }

  virtual bool pc_execute(THD *thd) override;
};

class Rule_enable : public im::Proc, public im::Disable_copy_base {
  using Sql_cmd_type = Sql_cmd_rule_enable;
 public:
  explicit Rule_enable(PSI_memory_key key) : Proc(key) {
    m_result_type = Result_type::RESULT_OK;
    m_parameters.assign_at(0, MYSQL_TYPE_LONGLONG);  // Rule id
    // Enable (1) or Disable (0)
    m_parameters.assign_at(1, MYSQL_TYPE_LONGLONG);
  }
  ~Rule_enable(){}
  virtual const std::string qname() const override { return proc_qname; }
  static Proc *instance();

  virtual Sql_cmd *evoke_cmd(THD *thd,
                             mem_root_deque<Item *> *list) const override;

  virtual const std::string str() const override { return "statement_outline_enable_rule"; }
};

class Sql_cmd_rules_show : public im::Sql_cmd_admin_proc {
 public:
  explicit Sql_cmd_rules_show(THD *thd, mem_root_deque<Item *> *list,
                               const im::Proc *proc)
      : im::Sql_cmd_admin_proc(thd, list, proc) {
    set_priv_type(Priv_type::PRIV_NONE_ACL);
  }

  virtual bool pc_execute(THD *thd) override { return false; }
  virtual void send_result(THD *thd, bool error) override;
};

class Rules_show : public im::Proc, public im::Disable_copy_base {
  using Sql_cmd_type = Sql_cmd_rules_show;
 public:
  explicit Rules_show(PSI_memory_key key) : Proc(key) {
    m_result_type = Result_type::RESULT_SET;
    m_columns.assign_at(0, {MYSQL_TYPE_LONGLONG, STRING_WITH_LEN("ID"), 0});
    m_columns.assign_at(1,
                        {MYSQL_TYPE_VARCHAR, STRING_WITH_LEN("SCHEMA"), 64});
    m_columns.assign_at(2,
                        {MYSQL_TYPE_VARCHAR, STRING_WITH_LEN("DIGEST"), 128});
    m_columns.assign_at(3,
                        {MYSQL_TYPE_VARCHAR, STRING_WITH_LEN("ENABLED"), 3});
    m_columns.assign_at(4, {MYSQL_TYPE_LONGLONG, STRING_WITH_LEN("HITS"), 0});
    m_columns.assign_at(5, {MYSQL_TYPE_VARCHAR, STRING_WITH_LEN("OUTLINE"),
                            Field::MAX_LONG_BLOB_WIDTH});
    m_columns.assign_at(
        6, {MYSQL_TYPE_VARCHAR, STRING_WITH_LEN("DIGEST_TEXT"), 5000});
    m_columns.assign_at(
        7, {MYSQL_TYPE_LONGLONG, STRING_WITH_LEN("HAS_APPLY_FAILURE"), 0});
  }
  ~Rules_show() {}
  virtual const std::string qname() const override { return proc_qname; }
  static Proc *instance();
  virtual Sql_cmd *evoke_cmd(THD *thd,
                             mem_root_deque<Item *> *list) const override;

  virtual const std::string str() const override { return "statement_outline_show_rules"; }
};
}

#endif /* STATEMENT_OUTLINE_PROC_INCLUDED */

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

bool Item_typecast_decimal::parse_format() {
  int len = 0;
  int dec = 0;
  bool have_point = false;
  const char *ptr, *end;
  StringBuffer<STRING_BUFFER_USUAL_SIZE> string_buffer;
  const String *res = args[1]->val_str(&string_buffer);
  ptr = res->ptr();
  end = ptr + res->length();

  for (; ptr != end; ptr++) {
    switch (*ptr) {
      case '9':
        len++;
        if (have_point) dec++;
        break;
      case '.':
        have_point = true;
        break;
      default:
        goto err;
    }
  }
  if (0 == len) goto err;
  set_data_type_decimal(len, dec);
  format_parsed = true;
  return false;

err:
  my_error(ER_WRONG_FORMAT_STRING, MYF(0), res->ptr());
  return true;
}

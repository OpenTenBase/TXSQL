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
  @file sql/item_timefunc_oracle.cc

  Functions to create an item. Used by sql_yacc.yy
*/

/**
 wrap the my_strtoll10 and check the error
 @param res     store the result
 @param nptr    pos of the string  convert to int
 @param endptr  pos of the string  which can't convert to int
 @param error   the error number

 @result true   result is illegal
 @result false  ok

*/
static bool str_to_int(int *res, const char *nptr, const char **endptr,
                       int *error) {
  *res = (int)my_strtoll10(nptr, endptr, error);
  if (*error == ERANGE || *error == EDOM) return true;
  return false;
}

/**
   parse the val string orcording to ptr string

   @param ptr              Points to the current pos of format string
   @param end              Points to the end of format string
   @param val              Points to the current pos the value string
   @param val_end          Points to the end of the value string
   @l_time                 store the result of time
   @yearday                the day of the year in value
   @week_number            the number of week of the year in value
   @week_number_month      the number of week of the month in value
   @weekday                the day of the week in value
   @usa_time               12 hours time
   @daypart                12 hours added by PM
   @error                  error number

   @return true  parse error
   @return false ok
*/
static bool extract_date_time_oracle(const char *&ptr, const char *&end,
                                     const char *&val, const char *&val_end,
                                     MYSQL_TIME *l_time, int &yearday,
                                     int &week_number, int &week_number_month,
                                     int &weekday, bool &usa_time, int &daypart,
                                     int &error) {
  int val_len;
  const char *tmp;
  error = 0;

  if (ptr >= end || val >= val_end) return true;
  val_len = (uint)(val_end - val);
  switch (my_toupper(system_charset_info, *ptr)) {
    case 'A':  // AM or A.M.
      if (!strncasecmp(ptr, "AM", 2)) {
        ptr++;
      } else if (!strncasecmp(ptr, "A.M.", 4)) {
        ptr += 3;
      } else
        return true;
      if (!strncasecmp(val, "AM", 2)) {
        val += 2;
      } else if (!strncasecmp(val, "A.M.", 4)) {
        val += 4;
      } else
        return true;
      if (!usa_time) return true;
      break;
    case 'P':  // PM or P.M.
      if (!strncasecmp(ptr, "PM", 2)) {
        ptr++;
      } else if (!strncasecmp(ptr, "P.M.", 4)) {
        ptr += 3;
      } else
        return true;
      if (!strncasecmp(val, "PM", 2)) {
        val += 2;
      } else if (!strncasecmp(val, "P.M.", 4)) {
        val += 4;
      } else
        return true;
      if (!usa_time) return true;
      daypart = 12;
      break;
    case 'Y': {  // YYYY or YYY or YY or Y
      int year_len = 0;
      if (!strncasecmp(ptr, "YYYY", 4)) {
        year_len = 4;
      } else if (!strncasecmp(ptr, "YYY", 3)) {
        year_len = 3;
      } else if (!strncasecmp(ptr, "YY", 2)) {
        year_len = 2;
      } else {  // "Y"
        year_len = 1;
      }
      tmp = val + min(year_len, val_len);
      if (str_to_int((int *)&l_time->year, val, &tmp, &error)) return true;
      val = tmp;
      // support ('202110','YYYY-MM') or ('2021-10','YYYYMM')
      if (*val == '-' && *(ptr + year_len) != '-') val++;
      if (*(ptr + year_len) == '-' && *val != '-') ptr++;
      ptr += (year_len - 1);  // for loop add a ++
      break;
    }
    case 'R': {  // 'RRRR' or 'RR'
      int year_len = 0;
      if (!strncasecmp(ptr, "RRRR", 4)) {
        year_len = 4;
      } else if (!strncasecmp(ptr, "RR", 2)) {
        year_len = 2;
      } else
        return true;
      tmp = val + min(year_len, val_len);
      if (str_to_int((int *)&l_time->year, val, &tmp, &error)) return true;
      if ((int)(tmp - val) <= 2)
        l_time->year = year_2000_handling_oracle(l_time->year);
      val = tmp;
      ptr += year_len - 1;  // for loop add a ++
      break;
    }
    case 'M':  // 'MM','MON','MONTH, 'MI'
      if (!strncasecmp(ptr, "MM", 2)) {
        tmp = val + min(2, val_len);
        if (str_to_int((int *)&l_time->month, val, &tmp, &error)) return true;
        val = tmp;
        // support ('20211020','YYYYMM-DD') or ('202110-20','YYYYMMDD')
        if (*val == '-' && *(ptr + 2) != '-') val++;
        if (*(ptr + 2) == '-' && *val != '-') ptr++;
        ptr++;
      } else if (!strncasecmp(ptr, "MI", 2)) {
        tmp = val + min(2, val_len);
        if (str_to_int((int *)&l_time->minute, val, &tmp, &error)) return true;
        val = tmp;
        ptr++;
      } else if (!strncasecmp(ptr, "MON", 3)) {
        if (!strncasecmp(ptr, "MONTH", 5)) {
          if ((l_time->month = check_word(my_locale_en_US.month_names, val,
                                          val_end, &val)) <= 0)
            return true;
          ptr += 4;
        } else {
          if ((l_time->month = check_word(my_locale_en_US.ab_month_names, val,
                                          val_end, &val)) <= 0)
            return true;
          ptr += 2;
        }
      } else
        return true;
      break;
    case 'D':  // DDD or DD or DY or DAY or D
      if (!strncasecmp(ptr, "DDD", 3)) {
        tmp = val + min(3, val_len);
        if (str_to_int(&yearday, val, &tmp, &error)) return true;
        val = tmp;
        ptr += 2;
      } else if (!strncasecmp(ptr, "DD", 2)) {
        tmp = val + min(2, val_len);
        if (str_to_int((int *)&l_time->day, val, &tmp, &error)) return true;
        val = tmp;
        ptr++;
      } else if (!strncasecmp(ptr, "DY", 2)) {
        if ((weekday = check_word(my_locale_en_US.ab_day_names, val, val_end,
                                  &val)) <= 0)
          return true;
        ptr++;
      } else if (!strncasecmp(ptr, "DAY", 3)) {
        if ((weekday = check_word(my_locale_en_US.day_names, val, val_end,
                                  &val)) <= 0)
          return true;
        ptr += 2;
      } else {  // "D"
        tmp = val + min(1, val_len);
        if (str_to_int(&weekday, val, &tmp, &error)) return true;
        if (weekday < 1 || weekday > 7) return true;
        val = tmp;
      }
      break;
    case 'H':  // HH24 or HH or HH12
      if (!strncasecmp(ptr, "HH", 2)) {
        if (!strncasecmp(ptr, "HH24", 4)) {
          ptr += 3;
        } else if (!strncasecmp(ptr, "HH12", 4)) {
          usa_time = true;
          ptr += 3;
        } else {
          usa_time = true;
          ptr++;
        }
        tmp = val + min(2, val_len);
        if (str_to_int((int *)&l_time->hour, val, &tmp, &error)) return true;
        val = tmp;
      } else
        return true;
      break;
    case 'S':  // SS
      if (!strncasecmp(ptr, "SS", 2)) {
        tmp = val + min(2, val_len);
        if (str_to_int((int *)&l_time->second, val, &tmp, &error)) return true;
        val = tmp;
      } else
        return true;
      ptr++;
      break;
    case 'W':
      if (!strncasecmp(ptr, "WW", 2)) {
        tmp = val + min(2, val_len);
        if (str_to_int(&week_number, val, &tmp, &error)) return true;
        if (week_number < 1 || week_number > 53) return true;
        val = tmp;
        ptr++;
      } else {  // "W"
        tmp = val + min(1, val_len);
        if (str_to_int(&week_number_month, val, &tmp, &error)) return true;
        if (week_number_month < 1 || week_number_month > 5) return true;
        val = tmp;
      }
      break;
    default:
      if (*val != *ptr) return true;
      val++;
      break;
  }

  return false;
}

/**
   Flip 'quotation_flag' if we found a quote (") character.

   @param cftm             Character or FMT... format descriptor
   @param quotation_flag   Points to 'true' if we are inside a quoted string

   @return true  If we are inside a quoted string or if we found a '"' character
   @return false Otherwise
*/

static inline bool check_quotation(uint16 cfmt, bool *quotation_flag) {
  if (cfmt == '"') {
    *quotation_flag = !*quotation_flag;
    return true;
  }
  return *quotation_flag;
}

#define INVALID_CHARACTER(x)                                                 \
  (((x) >= 'A' && (x) <= 'Z') || ((x) >= '0' && (x) <= '9') || (x) >= 127 || \
   ((x) < 32))

/**
  Special characters are directly output in the result

  @return 0  If found not acceptable character
  @return #  Number of copied characters
*/
static uint parse_special(char cfmt, const char *ptr, const char *end,
                          String *str) {
  int offset = 0;
  char tmp1;

  /* Non-printable character and Multibyte encoded characters */
  if (INVALID_CHARACTER(cfmt)) return 0;

  /*
   * '&' with text is used for variable input, but '&' with other
   * special charaters like '|'. '*' is used as separator
   */
  if (cfmt == '&' && ptr + 1 < end) {
    tmp1 = my_toupper(system_charset_info, *(ptr + 1));
    if (tmp1 >= 'A' && tmp1 <= 'Z') return 0;
  }

  do {
    /*
      Continuously store the special characters in fmt_array until non-special
      characters appear
     */
    str->append((char)*ptr++);
    offset++;
    if (ptr == end) break;
    tmp1 = my_toupper(system_charset_info, *ptr);
  } while (!INVALID_CHARACTER(tmp1) && tmp1 != '"');
  return offset;
}

static inline bool append_val(int val, int size, String *str) {
  ulong len = 0;
  char intbuff[15];

  len = (ulong)(longlong10_to_str(val, intbuff, 10) - intbuff);
  return str->append_with_prefill(intbuff, len, size, '0');
}

/**
  Create a formated date/time value in a string of oracle style.

  @param format    the format string
  @param l_time    the input time
  @param type      mysql timestamep type
  @param str       the result string
*/
bool make_date_time_oracle(Date_time_format *format, MYSQL_TIME *l_time,
                           enum_mysql_timestamp_type type, String *str) {
  char intbuff[15];
  ulong length;
  const char *ptr, *end;
  bool quotation_flag = false;
  uint weekday = 0;
  THD *thd = current_thd;
  MY_LOCALE *locale = thd->variables.lc_time_names;

  str->length(0);
  end = (ptr = format->format.str) + format->format.length;
  if (format->format.length > MAX_DATETIME_FORMAT_MODEL_LEN) goto error;

  for (; ptr < end; ptr++) {
    uint ulen;
    char cfmt, next_char;

    cfmt = my_toupper(system_charset_info, *ptr);

    /*
      Oracle datetime format support text in double quotation marks like
      'YYYY"abc"MM"xyz"DD', When this happens, store the text and quotation
      marks, and use the text as a separator in make_date_time_oracle.

      NOTE: the quotation mark is not print in return value. for example:
      select TO_CHAR(sysdate, 'YYYY"abc"MM"xyzDD"') will return 2021abc01xyz11
     */
    if (check_quotation(cfmt, &quotation_flag)) {
      /* don't display '"' in the result, so if it is '"', skip it */
      if (*ptr != '"') {
        str->append((char)*ptr);
      }
      continue;
    }

    switch (cfmt) {
      case 'A':  // AD/A.D./AM/A.M.
        if (ptr + 1 >= end) goto error;
        next_char = my_toupper(system_charset_info, *(ptr + 1));
        if (next_char == 'D') {
          if (l_time->year > 0) str->append(STRING_WITH_LEN("AD"));
          ptr += 1;
        } else if (next_char == 'M') {
          if (l_time->hour <= 11) str->append("AM", 2);
          ptr += 1;
        } else if (next_char == '.' && ptr + 3 < end && *(ptr + 3) == '.') {
          if (my_toupper(system_charset_info, *(ptr + 2)) == 'D') {
            if (l_time->year > 0) str->append(STRING_WITH_LEN("A.D."));
            ptr += 3;
          } else if (my_toupper(system_charset_info, *(ptr + 2)) == 'M') {
            if (l_time->hour <= 11) str->append(STRING_WITH_LEN("A.M."));
            ptr += 3;
          } else
            goto error;
        } else
          goto error;
        break;
      case 'B':  // BC and B.C
        if (ptr + 1 >= end) goto error;
        next_char = my_toupper(system_charset_info, *(ptr + 1));
        if (next_char == 'C') {
          if (l_time->year <= 0) str->append(STRING_WITH_LEN("BC"));
          ptr += 1;
        } else if (next_char == '.' && ptr + 3 < end &&
                   my_toupper(system_charset_info, *(ptr + 2)) == 'C' &&
                   *(ptr + 3) == '.') {
          if (l_time->year <= 0) str->append(STRING_WITH_LEN("B.C."));
          ptr += 3;
        } else
          goto error;
        break;
      case 'P':  // PM or P.M.
        next_char = my_toupper(system_charset_info, *(ptr + 1));
        if (next_char == 'M') {
          if (l_time->hour > 11) str->append("PM", 2);
          ptr += 1;
        } else if (next_char == '.' &&
                   my_toupper(system_charset_info, *(ptr + 2)) == 'M' &&
                   my_toupper(system_charset_info, *(ptr + 3)) == '.') {
          if (l_time->hour > 11) str->append(STRING_WITH_LEN("P.M."));
          ptr += 3;
        } else
          goto error;
        break;
      case 'Y':  // Y, YY, YYY o YYYYY
        if (ptr + 1 == end ||
            my_toupper(system_charset_info, *(ptr + 1)) != 'Y') {
          if (append_val(l_time->year % 10, 1, str)) goto error;
          break;
        }
        if (ptr + 2 == end ||
            my_toupper(system_charset_info, *(ptr + 2)) != 'Y') { /* YY */
          if (append_val(l_time->year % 100, 2, str)) goto error;
          ulen = 2;
        } else {
          if (ptr + 3 < end &&
              my_toupper(system_charset_info, *(ptr + 3)) == 'Y') {
            if (append_val(l_time->year, 4, str)) goto error;
            ulen = 4;
          } else {
            if (append_val(l_time->year % 1000, 3, str)) goto error;
            ulen = 3;
          }
        }
        ptr += ulen - 1;
        break;

      case 'R':  // RR or RRRR
        if (ptr + 1 == end ||
            my_toupper(system_charset_info, *(ptr + 1)) != 'R')
          goto error;

        if (ptr + 2 == end ||
            my_toupper(system_charset_info, *(ptr + 2)) != 'R') {
          if (append_val(l_time->year % 100, 2, str)) goto error;
          ulen = 2;
        } else {
          if (ptr + 3 >= end ||
              my_toupper(system_charset_info, *(ptr + 3)) != 'R')
            goto error;
          if (append_val(l_time->year, 4, str)) goto error;
          ulen = 4;
        }
        ptr += ulen - 1;
        break;
      case 'M': {
        char tmp1;
        if (ptr + 1 >= end) goto error;

        tmp1 = my_toupper(system_charset_info, *(ptr + 1));
        if (tmp1 == 'M') {
          if (append_val(l_time->month, 2, str)) goto error;
          ptr += 1;
        } else if (tmp1 == 'I') {
          if (append_val(l_time->minute, 2, str)) goto error;
          ptr += 1;
        } else if (tmp1 == 'O') {
          if (ptr + 2 >= end) goto error;
          char tmp2 = my_toupper(system_charset_info, *(ptr + 2));
          if (tmp2 != 'N') goto error;

          if (ptr + 4 >= end ||
              my_toupper(system_charset_info, *(ptr + 3)) != 'T' ||
              my_toupper(system_charset_info, *(ptr + 4)) != 'H') {
            if (l_time->month == 0) {
              str->append("00", 2);
            } else {
              const char *month_name =
                  (locale->ab_month_names->type_names[l_time->month - 1]);
              size_t m_len = strlen(month_name);
              str->append(month_name, m_len, system_charset_info);
            }
            ptr += 2;
          } else {
            if (l_time->month == 0) {
              str->append("00", 2);
            } else {
              const char *month_name =
                  (locale->month_names->type_names[l_time->month - 1]);
              size_t month_byte_len = strlen(month_name);
              size_t month_char_len;
              str->append(month_name, month_byte_len, system_charset_info);
              month_char_len =
                  my_numchars_mb(&my_charset_utf8mb4_general_ci, month_name,
                                 month_name + month_byte_len);
              if (str->fill(str->length() + locale->max_month_name_length -
                                month_char_len,
                            ' '))
                goto error;
            }
            ptr += 4;
          }
        } else
          goto error;
      } break;
      case 'D': {  // DD, DY, or DAY
        if (ptr + 1 >= end) goto error;
        char tmp1 = my_toupper(system_charset_info, *(ptr + 1));

        if (tmp1 == 'D') {
          if (append_val(l_time->day, 2, str)) goto error;
        } else if (tmp1 == 'Y') {
          if (l_time->day == 0)
            str->append("00", 2);
          else {
            weekday = calc_weekday(
                calc_daynr(l_time->year, l_time->month, l_time->day), 0);
            const char *day_name = locale->ab_day_names->type_names[weekday];
            str->append(day_name, strlen(day_name), system_charset_info);
          }
        } else if (tmp1 == 'A') {  // DAY
          if (ptr + 2 == end ||
              my_toupper(system_charset_info, *(ptr + 2)) != 'Y')
            goto error;
          if (l_time->day == 0)
            str->append("00", 2, system_charset_info);
          else {
            const char *day_name;
            size_t day_byte_len, day_char_len;
            weekday = calc_weekday(
                calc_daynr(l_time->year, l_time->month, l_time->day), 0);
            day_name = locale->day_names->type_names[weekday];
            day_byte_len = strlen(day_name);
            str->append(day_name, day_byte_len, system_charset_info);
            day_char_len = my_numchars_mb(&my_charset_utf8mb4_general_ci,
                                          day_name, day_name + day_byte_len);
            if (str->fill(
                    str->length() + locale->max_day_name_length - day_char_len,
                    ' '))
              goto error;
          }
          ptr += 1;
        } else
          goto error;
        ptr += 1;
        break;
      }
      case 'F': {  // FF or FF1 .. FF6
        if (ptr + 1 >= end) goto error;
        char tmp1 = my_toupper(system_charset_info, *(ptr + 1));

        if (tmp1 == 'F') {
          // part len 1..6
          int part_len = 0;
          if ((ptr + 2) < end) {
            part_len = *(ptr + 2) - '0';
            if (part_len < 1 || part_len > 6) goto error;
          }
          unsigned long second_part = l_time->second_part;
          switch (part_len) {
            case 0:  // FF
              break;
            case 1:
              second_part = (second_part / 100000);
              break;
            case 2:
              second_part = (second_part / 10000);
              break;
            case 3:
              second_part = (second_part / 1000);
              break;
            case 4:
              second_part = (second_part / 100);
              break;
            case 5:
              second_part = (second_part / 10);
              break;
            case 6:
              break;
            default:
              assert(false);  // checked before
              goto error;
          }
          length = longlong10_to_str(second_part, intbuff, 10) - intbuff;
          str->append_with_prefill(intbuff, length,
                                   (part_len == 0) ? 6 : part_len, '0');
          ptr += (part_len == 0) ? 2 : 3;
        } else
          goto error;
        break;
      }
      case 'H': {  // HH, HH12 or HH24
        char tmp1, tmp2, tmp3;
        int hours_i = (l_time->hour % 24 + 11) % 12 + 1;
        if (ptr + 1 >= end) goto error;
        tmp1 = my_toupper(system_charset_info, *(ptr + 1));

        if (tmp1 != 'H') goto error;

        if (ptr + 3 >= end) {
          if (append_val(hours_i, 2, str)) goto error;
          ptr += 1;
        } else {
          tmp2 = *(ptr + 2);
          tmp3 = *(ptr + 3);

          if (tmp2 == '1' && tmp3 == '2') {
            if (append_val(hours_i, 2, str)) goto error;
            ptr += 3;
          } else if (tmp2 == '2' && tmp3 == '4') {
            if (append_val(l_time->hour, 2, str)) goto error;
            ptr += 3;
          } else {
            if (append_val(hours_i, 2, str)) goto error;
            ptr += 1;
          }
        }
        break;
      }
      case 'S':  // SS
        if (ptr + 1 == end ||
            my_toupper(system_charset_info, *(ptr + 1)) != 'S')
          goto error;

        if (append_val(l_time->second, 2, str)) goto error;
        ptr += 1;
        break;
      case '|':
        /*
          If only one '|' just ignore it, else append others, for example:
          TO_CHAR('2000-11-05', 'YYYY|MM||||DD') --> 200011|||05
        */
        if (ptr + 1 == end || *(ptr + 1) != '|') {
          break;
        }
        ptr++;  // Skip first '|'
        do {
          str->append((char)*ptr++);
        } while ((ptr < end) && *ptr == '|');
        ptr--;  // Fix ptr for above for loop
        break;

      default:
        int offset = parse_special(cfmt, ptr, end, str);
        if (!offset) goto error;
        /* ptr++ is in the for loop, so we must move ptr to offset-1 */
        ptr += (offset - 1);
        break;
    }
  }

  return false;

error:
  push_warning_printf(
      current_thd, Sql_condition::SL_WARNING, ER_WRONG_FORMAT_STRING,
      ER_THD(current_thd, ER_WRONG_FORMAT_STRING), format->format.str);
  return true;
}

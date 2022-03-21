#pragma once
#include <stdio.h>

#define CUR_YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
    + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define CUR_MONTH ((__DATE__ [2] == 'n' ?  (__DATE__ [1] == 'a' ? 0 : 5) \
    : __DATE__ [2] == 'b' ? 1 \
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
    : __DATE__ [2] == 'y' ? 4 \
    : __DATE__ [2] == 'l' ? 6 \
    : __DATE__ [2] == 'g' ? 7 \
    : __DATE__ [2] == 'p' ? 8 \
    : __DATE__ [2] == 't' ? 9 \
    : __DATE__ [2] == 'v' ? 10 : 11)+1)

#define CUR_DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
    + (__DATE__ [5] - '0'))

#define CUR_HOUR (( __TIME__ [0] - '0') * 10 + (__TIME__ [1] - '0'))
#define CUR_MIN  (( __TIME__ [3] - '0') * 10 + (__TIME__ [4] - '0'))

#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)

const char *CompileTime () {
    static char tmpbuf[1024] = { 0 };
    snprintf (tmpbuf , sizeof(tmpbuf) , "-v18-txsql-2.0.1-V2.0R725D006-%04d%02d%02d-%02d%02d" , CUR_YEAR , CUR_MONTH , CUR_DAY , CUR_HOUR , CUR_MIN);
    return tmpbuf;
}

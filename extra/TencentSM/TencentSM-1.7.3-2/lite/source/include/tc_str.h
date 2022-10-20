/* Copyright 2019, Tencent Technology (Shenzhen) Co Ltd

This file is part of the Tencent SM (Lite Version) Library.

The Tencent SM (Lite Version) Library is free software; you can redistribute it and/or modify
it under the terms of either:

* the GNU Lesser General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your
option) any later version.

or

* the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any
later version.

or both in parallel, as here.

The Tencent SM (Lite Version) Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the Tencent SM (Lite Version) Library.  If not,
see https://www.gnu.org/licenses/.  */

#ifndef TC_STRING_H
#define TC_STRING_H

#include <stdio.h>

void tcsm_lower2upper(void* sBuff, int slen);
void tcsm_upper2lower(void* sBuff, int slen);

size_t tcsm_buf_strlcpy(char *dst, const char *src, size_t size);
int tcsm_secure_memcmp(const volatile void *in_a, const volatile void *in_b, size_t len);

int           tcsm_bin2hex(const unsigned char *buffer, size_t len, char outbuf[], size_t outbuflen);
unsigned char* tcsm_hex2bin(const char* hexstr, size_t* size);

#endif /* TC_STRING_H */

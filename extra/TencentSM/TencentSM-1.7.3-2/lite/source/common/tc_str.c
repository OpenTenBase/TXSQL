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

#include "../include/tc_str.h"
#include "../include/tc.h"
#include <stdlib.h>
#include <string.h>

void tcsm_lower2upper(void* sBuff, int slen)
{
  unsigned char *bsBuff = (unsigned char *)sBuff;
  for (int i=0;i<slen;i++) if((bsBuff[i]>='a') && (bsBuff[i]<='z')) bsBuff[i] &= 0xdf;
}

void tcsm_upper2lower(void* sBuff, int slen)
{
  unsigned char  *bsBuff = (unsigned char *)sBuff;
  for (int i=0;i<slen;i++) if((bsBuff[i]>='A') && (bsBuff[i]<='Z')) bsBuff[i] |= 0x20;
}

size_t tcsm_buf_strlcpy(char *dst, const char *src, size_t size)
{
    size_t l = 0;
    for (; size > 1 && *src; size--) {
        *dst++ = *src++;
        l++;
    }
    if (size)
        *dst = '\0';
    return l + strlen(src);
}

int tcsm_secure_memcmp(const volatile void *in_a, const volatile void *in_b, size_t len)
{
  size_t i;
  const volatile unsigned char *a = in_a;
  const volatile unsigned char *b = in_b;
  unsigned char x = 0;
  
  for (i = 0; i < len; i++)
    x |= a[i] ^ b[i];
  
  return x;
}


int tcsm_bin2hex(const unsigned char *buffer, size_t len, char outbuf[], size_t outbuflen)
{
    const static char hexdig[] = "0123456789ABCDEF";
    char *q = NULL;
    const unsigned char *p = NULL;
    int i = 0;
    
    if (outbuflen < len * 2 + 1)
    {
        return -1;
    }
    
    q = outbuf;
    for (i = 0, p = buffer; i < len; i++, p++) {
        *q++ = hexdig[(*p >> 4) & 0xf];
        *q++ = hexdig[*p & 0xf];
    }
    *q = 0;
    return 0;
}

unsigned char* tcsm_hex2bin(const char* hexstr, size_t* size)
{
    size_t hexstrLen = strlen(hexstr);
    size_t bytesLen = hexstrLen / 2;
    
    unsigned char* bytes = (unsigned char*)tcsm_tc_secure_malloc(bytesLen);
    
    int count = 0;
    const char* pos = hexstr;
    
    for(count = 0; count < bytesLen; count++) {
        // sscanf(pos, "%2hhx", &bytes[count]);//从java端来的>140时为何crash
        bytes[count] = (*pos>'9'? *pos+9 : *pos) << 4;
        pos ++;
        bytes[count] = bytes[count]|((*pos>'9'? *pos+9 : *pos) & 0x0f);
        pos ++;
    }
    
    if( size != NULL )
        *size = bytesLen;
    
    return bytes;
}

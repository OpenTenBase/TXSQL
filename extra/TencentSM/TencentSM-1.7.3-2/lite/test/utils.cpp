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

/**
 * test  interface in sm.h
 */
#include "test_global.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void tc_bin2hex(const unsigned char *buffer, size_t len, char outbuf[], size_t outbuflen)
{
    const static char hexdig[] = "0123456789ABCDEF";
    char *q = NULL;
    const unsigned char *p = NULL;
    int i = 0;
    
    if (outbuflen < len * 2 + 1)
    {
        return;
    }
    
    q = outbuf;
    for (i = 0, p = buffer; i < len; i++, p++) {
        *q++ = hexdig[(*p >> 4) & 0xf];
        *q++ = hexdig[*p & 0xf];
    }
    *q = 0;
}

unsigned char* tc_hex2bin(const char* hexstr, unsigned char* bytes)
{
    size_t hexstrLen = strlen(hexstr);
    size_t bytesLen = hexstrLen / 2;
    
    // unsigned char* bytes = (unsigned char*)malloc(bytesLen);
    
    int count = 0;
    const char* pos = hexstr;
    
    for(count = 0; count < bytesLen; count++) {
        sscanf(pos, "%2hhx", &bytes[count]);
        pos += 2;
    }
    
    return bytes;
}

void printBytes(unsigned char * bytes, int len) {
    char tmpstr[17];
    int lenstr = 17;
    int pos = 0;
    int tmplen = 0;
    
    fprintf(stdout, "--bytes len=%d--\n", len);
    while(pos < len) {
        memset(tmpstr, 0x00, lenstr);
        tmplen = len - pos;
        if(tmplen >= 8) {
            tmplen = 8;
        }
        tc_bin2hex(bytes+pos, tmplen, tmpstr, lenstr);
        fprintf(stdout, "%s\n", tmpstr);
        pos = pos + tmplen;
    }
}

char* tcsm_readPEM(const char* path, const char* filename) {
    char filepath[256];
    int len = (int)strlen(path);
    char* pContent = NULL;

    strncpy(filepath, path,256);
    if(path[len-1] != '/') {
        filepath[len] = '/';
        filepath[len+1] = '\0';
    }
    strncat(filepath, filename,256);
    FILE* file = fopen(filepath, "r");
    if(file != NULL) {
        size_t num = 0;
        fseek(file,0L,SEEK_END);
        num = ftell(file);
        fseek(file,0L,SEEK_SET);
        pContent = (char*)malloc(num+1);
        pContent[num] = '\0';
        num = fread(pContent, 1, num, file); 
        if(num <= 0) {
            free(pContent);
            pContent = NULL;
        }
    }

    return pContent;
}

int tcsm_writePEM(const char* path, const char* filename, char* pem) {
    char filepath[256];
    int len = (int)strlen(path);
    char* pContent = NULL;

    strncpy(filepath, path,256);
    if(path[len-1] != '/') {
        filepath[len] = '/';
        filepath[len+1] = '\0';
    }
    strncat(filepath, filename,256);
    FILE* file = fopen(filepath, "w");
    len = (int)strlen(pem);
    if(file != NULL) {
        size_t num = fwrite(pem, 1, len, file);
        if(num != len) {
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}

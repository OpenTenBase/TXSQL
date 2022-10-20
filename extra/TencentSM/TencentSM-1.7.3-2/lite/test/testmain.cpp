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
#include "test_global.hpp"
#include "string.h"
#include "sm.h"
#include "stdlib.h"

#include "iostream"

using namespace::std;

void testtmp(char* signdatatmp);
int main(int argc, char* argv[])
{
    if(argc < 2) {
        fprintf(stdout, "Usage: %s interface [name]\n     %s ability [name]\n     %s all\n", argv[0], argv[0], argv[0]);
        return 1;
    }

    if(strcmp(argv[1], "interface")==0) {
        // testsm2(256);
        char* name = NULL;
        if(argc >= 3) {
            name = argv[2];
        }
        testInterface(name);
        fprintf(stdout, "Test %s finish!\n", argv[1]);
    } else if(strcmp(argv[1], "ability")==0) {
        // testsm2(256);
        char* name = NULL;
        if(argc >= 3) {
            name = argv[2];
        }
        test_ability(name);
        fprintf(stdout, "Test %s finish!\n", argv[1]);
    } else if(strcmp(argv[1], "all")==0) {
        // testsm2(256);
        test_ability("all");
        testInterface("all");
        fprintf(stdout, "Test %s finish!\n", argv[1]);
    }
    return 0;
}

void testtmp(char* signdatatmp) {
    size_t outlen = 256;
    unsigned char outbuf[outlen];
    sm2_ctx_t pctx;
    const char* msg = "123";
    const char* id = "456";
    const char* pubk = "0401db3739991da4bcaa5eea2175aeae625f5cbcd793bbc8e0f21e1e090f311174183278a975f2735a10e9df69e31397be6f837f209fcf491f80c47d64dc518718";
    const char* prik = "e2240cd3c750e1d1953cfe10746b4c268a63cb116a74e89c6928b0e6a29eb9ed";
    const char* signv = "3045022100dbb3fb1568133ff67f756fe6f85428afa23dde5e56561379ecbec6f2f83048d002200c78a2cfeb3c9e5a427d2c01114b5dc03d073a704d159bbb10cac538fc0e953c";
    unsigned char* signeddata;

    if(signdatatmp == NULL) {
        signv = "3045022100dbb3fb1568133ff67f756fe6f85428afa23dde5e56561379ecbec6f2f83048d002200c78a2cfeb3c9e5a427d2c01114b5dc03d073a704d159bbb10cac538fc0e953c";
    } else {
        signv = signdatatmp;
    }
    SM2InitCtx(&pctx);
    memset(outbuf, 0x00, outlen);

    if(SM2Sign(&pctx, (const unsigned char*)msg, strlen(msg), (const char*)id, strlen(id), 
                    (const char*)pubk, strlen(pubk), (const char*)prik, strlen(prik), 
                    outbuf, &outlen) != 0) {
        fprintf(stdout, "test SM2Sign failed\n");
        goto END;
    }
    printBytes(outbuf, outlen);
    memset(outbuf, 0x00, 256);
    //由于随机数的问题，用verify验证
    outlen = 70;
    signeddata = (unsigned char*)malloc(outlen);
    printf("\n--sign from java:\n%s\n", signv);
    tc_hex2bin(signv, signeddata);
    if(SM2Verify(&pctx, (const unsigned char*)msg, strlen(msg), (const char*)id, strlen(id), 
                                    (const unsigned char*)signeddata, outlen, (const char*)pubk, strlen(pubk)) != 0) {
        fprintf(stdout, "test SM2Verify failed\n");
        goto END;
    }
    fprintf(stdout, "test SM2Verify ok\n");

END:
    if(signeddata != NULL) {
        free(signeddata);
    }
    SM2FreeCtx(&pctx);
}

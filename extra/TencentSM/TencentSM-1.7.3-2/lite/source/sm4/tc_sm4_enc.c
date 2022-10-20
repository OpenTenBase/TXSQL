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

#include "../include/tc.h"
#include "../include/tc_sm4.h"
#include "../include/tc_sm4_lcl.h"

#define ROUND_TBOX(x0, x1, x2, x3, x4, i)      \
    x4 = x1 ^ x2 ^ x3 ^ *(rk + i);        \
    t0 = SMS4_T1[(uint8_t)x4];      \
    x4 >>= 8;            \
    x0 ^= t0;            \
    t0 = SMS4_T2[(uint8_t)x4];      \
    x4 >>= 8;            \
    x0 ^= t0;            \
    t0 = SMS4_T3[(uint8_t)x4];      \
    x4 >>= 8;            \
    x0 ^= t0;            \
    t1 = SMS4_T4[x4];          \
    x4 = x0 ^ t1

#define ROUND ROUND_TBOX

extern const uint32_t SMS4_T1[256];
extern const uint32_t SMS4_T2[256];
extern const uint32_t SMS4_T3[256];
extern const uint32_t SMS4_T4[256];

void tcsm_sms4_encrypt(const unsigned char *in, unsigned char *out, const tcsm_sms4_key_t *key)
{
    const uint32_t *rk = key->rk;
    uint32_t x0, x1, x2, x3, x4;
    uint32_t t0, t1;

    x0 = GET32(in     );
    x1 = GET32(in +  4);
    x2 = GET32(in +  8);
    x3 = GET32(in + 12);

    ROUNDS(x0, x1, x2, x3, x4);

    PUT32(x0, out     );
    PUT32(x4, out +  4);
    PUT32(x3, out +  8);
    PUT32(x2, out + 12);
}

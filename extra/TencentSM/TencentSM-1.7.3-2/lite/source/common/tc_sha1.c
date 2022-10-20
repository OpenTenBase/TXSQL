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

#include "../include/tc_global.h"
#include "../include/tc_sha1.h"

#include <stdio.h>
#include <string.h>

#define DATA_ORDER_IS_BIG_ENDIAN

#if defined(DATA_ORDER_IS_BIG_ENDIAN)

# define HOST_c2l(c,l)  (l =(((unsigned long)(*((c)++)))<<24),          \
                         l|=(((unsigned long)(*((c)++)))<<16),          \
                         l|=(((unsigned long)(*((c)++)))<< 8),          \
                         l|=(((unsigned long)(*((c)++)))    )           )
# define HOST_l2c(l,c)  (*((c)++)=(unsigned char)(((l)>>24)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>>16)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>> 8)&0xff),      \
                         *((c)++)=(unsigned char)(((l)    )&0xff),      \
                         l)

#else

# define HOST_c2l(c,l)  (l =(((unsigned long)(*((c)++)))    ),          \
                         l|=(((unsigned long)(*((c)++)))<< 8),          \
                         l|=(((unsigned long)(*((c)++)))<<16),          \
                         l|=(((unsigned long)(*((c)++)))<<24)           )
# define HOST_l2c(l,c)  (*((c)++)=(unsigned char)(((l)    )&0xff),      \
                         *((c)++)=(unsigned char)(((l)>> 8)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>>16)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>>24)&0xff),      \
                         l)

#endif

#define HASH_MAKE_STRING(c,s)   do {    \
        unsigned long ll;               \
        ll=(c)->h0; (void)HOST_l2c(ll,(s));     \
        ll=(c)->h1; (void)HOST_l2c(ll,(s));     \
        ll=(c)->h2; (void)HOST_l2c(ll,(s));     \
        ll=(c)->h3; (void)HOST_l2c(ll,(s));     \
        ll=(c)->h4; (void)HOST_l2c(ll,(s));     \
        } while (0)

#define INIT_DATA_h0 0x67452301UL
#define INIT_DATA_h1 0xefcdab89UL
#define INIT_DATA_h2 0x98badcfeUL
#define INIT_DATA_h3 0x10325476UL
#define INIT_DATA_h4 0xc3d2e1f0UL

#define K_00_19 0x5a827999UL
#define K_20_39 0x6ed9eba1UL
#define K_40_59 0x8f1bbcdcUL
#define K_60_79 0xca62c1d6UL
/*
 * As pointed out by Wei Dai, F() below can be simplified to the code in
 * F_00_19.  Wei attributes these optimizations to Peter Gutmann's SHS code,
 * and he attributes it to Rich Schroeppel.
 *      #define F(x,y,z) (((x) & (y)) | ((~(x)) & (z)))
 * I've just become aware of another tweak to be made, again from Wei Dai,
 * in F_40_59, (x&a)|(y&a) -> (x|y)&a
 */
#define F_00_19(b,c,d)  ((((c) ^ (d)) & (b)) ^ (d))
#define F_20_39(b,c,d)  ((b) ^ (c) ^ (d))
#define F_40_59(b,c,d)  (((b) & (c)) | (((b)|(c)) & (d)))
#define F_60_79(b,c,d)  F_20_39(b,c,d)

#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))

#define Xupdate(a,ix,ia,ib,ic,id)       ( (a)=(ia^ib^ic^id),    \
                                          ix=(a)=ROTATE((a),1)  \
                                        )

# define BODY_00_15(i,a,b,c,d,e,f,xi) \
        (f)=xi+(e)+K_00_19+ROTATE((a),5)+F_00_19((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_16_19(i,a,b,c,d,e,f,xi,xa,xb,xc,xd) \
        Xupdate(f,xi,xa,xb,xc,xd); \
        (f)+=(e)+K_00_19+ROTATE((a),5)+F_00_19((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_20_31(i,a,b,c,d,e,f,xi,xa,xb,xc,xd) \
        Xupdate(f,xi,xa,xb,xc,xd); \
        (f)+=(e)+K_20_39+ROTATE((a),5)+F_20_39((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_32_39(i,a,b,c,d,e,f,xa,xb,xc,xd) \
        Xupdate(f,xa,xa,xb,xc,xd); \
        (f)+=(e)+K_20_39+ROTATE((a),5)+F_20_39((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_40_59(i,a,b,c,d,e,f,xa,xb,xc,xd) \
        Xupdate(f,xa,xa,xb,xc,xd); \
        (f)+=(e)+K_40_59+ROTATE((a),5)+F_40_59((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_60_79(i,a,b,c,d,e,f,xa,xb,xc,xd) \
        Xupdate(f,xa,xa,xb,xc,xd); \
        (f)=xa+(e)+K_60_79+ROTATE((a),5)+F_60_79((b),(c),(d)); \
        (b)=ROTATE((b),30);


int tcsm_SHA1_init(SHA_CTX *c) {
    memset(c, 0x00, sizeof(*c));
    c->h0 = INIT_DATA_h0;
    c->h1 = INIT_DATA_h1;
    c->h2 = INIT_DATA_h2;
    c->h3 = INIT_DATA_h3;
    c->h4 = INIT_DATA_h4;
    return ERR_TENCENTSM_OK;
}

static void HASH_BLOCK_DATA_ORDER(SHA_CTX *c, const void *p, size_t num)
{
    const unsigned char *data = p;
    register unsigned MD32_REG_T A, B, C, D, E, T, l;
    unsigned MD32_REG_T XX0, XX1, XX2, XX3, XX4, XX5, XX6, XX7,
        XX8, XX9, XX10, XX11, XX12, XX13, XX14, XX15;

    A = c->h0;
    B = c->h1;
    C = c->h2;
    D = c->h3;
    E = c->h4;

    for (;;) {
        const union {
            long one;
            char little;
        } is_endian = {
            1
        };

        if (!is_endian.little && sizeof(SHA_LONG) == 4
            && ((size_t)p % 4) == 0) {
            const SHA_LONG *W = (const SHA_LONG *)data;

            XX0 = W[0];
            XX1 = W[1];
            BODY_00_15(0, A, B, C, D, E, T, XX0);
            XX2 = W[2];
            BODY_00_15(1, T, A, B, C, D, E, XX1);
            XX3 = W[3];
            BODY_00_15(2, E, T, A, B, C, D, XX2);
            XX4 = W[4];
            BODY_00_15(3, D, E, T, A, B, C, XX3);
            XX5 = W[5];
            BODY_00_15(4, C, D, E, T, A, B, XX4);
            XX6 = W[6];
            BODY_00_15(5, B, C, D, E, T, A, XX5);
            XX7 = W[7];
            BODY_00_15(6, A, B, C, D, E, T, XX6);
            XX8 = W[8];
            BODY_00_15(7, T, A, B, C, D, E, XX7);
            XX9 = W[9];
            BODY_00_15(8, E, T, A, B, C, D, XX8);
            XX10 = W[10];
            BODY_00_15(9, D, E, T, A, B, C, XX9);
            XX11 = W[11];
            BODY_00_15(10, C, D, E, T, A, B, XX10);
            XX12 = W[12];
            BODY_00_15(11, B, C, D, E, T, A, XX11);
            XX13 = W[13];
            BODY_00_15(12, A, B, C, D, E, T, XX12);
            XX14 = W[14];
            BODY_00_15(13, T, A, B, C, D, E, XX13);
            XX15 = W[15];
            BODY_00_15(14, E, T, A, B, C, D, XX14);
            BODY_00_15(15, D, E, T, A, B, C, XX15);

            data += SHA_CBLOCK;
        } else {
            (void)HOST_c2l(data, l);
            XX0 = l;
            (void)HOST_c2l(data, l);
            XX1 = l;
            BODY_00_15(0, A, B, C, D, E, T, XX0);
            (void)HOST_c2l(data, l);
            XX2 = l;
            BODY_00_15(1, T, A, B, C, D, E, XX1);
            (void)HOST_c2l(data, l);
            XX3 = l;
            BODY_00_15(2, E, T, A, B, C, D, XX2);
            (void)HOST_c2l(data, l);
            XX4 = l;
            BODY_00_15(3, D, E, T, A, B, C, XX3);
            (void)HOST_c2l(data, l);
            XX5 = l;
            BODY_00_15(4, C, D, E, T, A, B, XX4);
            (void)HOST_c2l(data, l);
            XX6 = l;
            BODY_00_15(5, B, C, D, E, T, A, XX5);
            (void)HOST_c2l(data, l);
            XX7 = l;
            BODY_00_15(6, A, B, C, D, E, T, XX6);
            (void)HOST_c2l(data, l);
            XX8 = l;
            BODY_00_15(7, T, A, B, C, D, E, XX7);
            (void)HOST_c2l(data, l);
            XX9 = l;
            BODY_00_15(8, E, T, A, B, C, D, XX8);
            (void)HOST_c2l(data, l);
            XX10 = l;
            BODY_00_15(9, D, E, T, A, B, C, XX9);
            (void)HOST_c2l(data, l);
            XX11 = l;
            BODY_00_15(10, C, D, E, T, A, B, XX10);
            (void)HOST_c2l(data, l);
            XX12 = l;
            BODY_00_15(11, B, C, D, E, T, A, XX11);
            (void)HOST_c2l(data, l);
            XX13 = l;
            BODY_00_15(12, A, B, C, D, E, T, XX12);
            (void)HOST_c2l(data, l);
            XX14 = l;
            BODY_00_15(13, T, A, B, C, D, E, XX13);
            (void)HOST_c2l(data, l);
            XX15 = l;
            BODY_00_15(14, E, T, A, B, C, D, XX14);
            BODY_00_15(15, D, E, T, A, B, C, XX15);
        }

        BODY_16_19(16, C, D, E, T, A, B, XX0, XX0, XX2, XX8, XX13);
        BODY_16_19(17, B, C, D, E, T, A, XX1, XX1, XX3, XX9, XX14);
        BODY_16_19(18, A, B, C, D, E, T, XX2, XX2, XX4, XX10, XX15);
        BODY_16_19(19, T, A, B, C, D, E, XX3, XX3, XX5, XX11, XX0);

        BODY_20_31(20, E, T, A, B, C, D, XX4, XX4, XX6, XX12, XX1);
        BODY_20_31(21, D, E, T, A, B, C, XX5, XX5, XX7, XX13, XX2);
        BODY_20_31(22, C, D, E, T, A, B, XX6, XX6, XX8, XX14, XX3);
        BODY_20_31(23, B, C, D, E, T, A, XX7, XX7, XX9, XX15, XX4);
        BODY_20_31(24, A, B, C, D, E, T, XX8, XX8, XX10, XX0, XX5);
        BODY_20_31(25, T, A, B, C, D, E, XX9, XX9, XX11, XX1, XX6);
        BODY_20_31(26, E, T, A, B, C, D, XX10, XX10, XX12, XX2, XX7);
        BODY_20_31(27, D, E, T, A, B, C, XX11, XX11, XX13, XX3, XX8);
        BODY_20_31(28, C, D, E, T, A, B, XX12, XX12, XX14, XX4, XX9);
        BODY_20_31(29, B, C, D, E, T, A, XX13, XX13, XX15, XX5, XX10);
        BODY_20_31(30, A, B, C, D, E, T, XX14, XX14, XX0, XX6, XX11);
        BODY_20_31(31, T, A, B, C, D, E, XX15, XX15, XX1, XX7, XX12);

        BODY_32_39(32, E, T, A, B, C, D, XX0, XX2, XX8, XX13);
        BODY_32_39(33, D, E, T, A, B, C, XX1, XX3, XX9, XX14);
        BODY_32_39(34, C, D, E, T, A, B, XX2, XX4, XX10, XX15);
        BODY_32_39(35, B, C, D, E, T, A, XX3, XX5, XX11, XX0);
        BODY_32_39(36, A, B, C, D, E, T, XX4, XX6, XX12, XX1);
        BODY_32_39(37, T, A, B, C, D, E, XX5, XX7, XX13, XX2);
        BODY_32_39(38, E, T, A, B, C, D, XX6, XX8, XX14, XX3);
        BODY_32_39(39, D, E, T, A, B, C, XX7, XX9, XX15, XX4);

        BODY_40_59(40, C, D, E, T, A, B, XX8, XX10, XX0, XX5);
        BODY_40_59(41, B, C, D, E, T, A, XX9, XX11, XX1, XX6);
        BODY_40_59(42, A, B, C, D, E, T, XX10, XX12, XX2, XX7);
        BODY_40_59(43, T, A, B, C, D, E, XX11, XX13, XX3, XX8);
        BODY_40_59(44, E, T, A, B, C, D, XX12, XX14, XX4, XX9);
        BODY_40_59(45, D, E, T, A, B, C, XX13, XX15, XX5, XX10);
        BODY_40_59(46, C, D, E, T, A, B, XX14, XX0, XX6, XX11);
        BODY_40_59(47, B, C, D, E, T, A, XX15, XX1, XX7, XX12);
        BODY_40_59(48, A, B, C, D, E, T, XX0, XX2, XX8, XX13);
        BODY_40_59(49, T, A, B, C, D, E, XX1, XX3, XX9, XX14);
        BODY_40_59(50, E, T, A, B, C, D, XX2, XX4, XX10, XX15);
        BODY_40_59(51, D, E, T, A, B, C, XX3, XX5, XX11, XX0);
        BODY_40_59(52, C, D, E, T, A, B, XX4, XX6, XX12, XX1);
        BODY_40_59(53, B, C, D, E, T, A, XX5, XX7, XX13, XX2);
        BODY_40_59(54, A, B, C, D, E, T, XX6, XX8, XX14, XX3);
        BODY_40_59(55, T, A, B, C, D, E, XX7, XX9, XX15, XX4);
        BODY_40_59(56, E, T, A, B, C, D, XX8, XX10, XX0, XX5);
        BODY_40_59(57, D, E, T, A, B, C, XX9, XX11, XX1, XX6);
        BODY_40_59(58, C, D, E, T, A, B, XX10, XX12, XX2, XX7);
        BODY_40_59(59, B, C, D, E, T, A, XX11, XX13, XX3, XX8);

        BODY_60_79(60, A, B, C, D, E, T, XX12, XX14, XX4, XX9);
        BODY_60_79(61, T, A, B, C, D, E, XX13, XX15, XX5, XX10);
        BODY_60_79(62, E, T, A, B, C, D, XX14, XX0, XX6, XX11);
        BODY_60_79(63, D, E, T, A, B, C, XX15, XX1, XX7, XX12);
        BODY_60_79(64, C, D, E, T, A, B, XX0, XX2, XX8, XX13);
        BODY_60_79(65, B, C, D, E, T, A, XX1, XX3, XX9, XX14);
        BODY_60_79(66, A, B, C, D, E, T, XX2, XX4, XX10, XX15);
        BODY_60_79(67, T, A, B, C, D, E, XX3, XX5, XX11, XX0);
        BODY_60_79(68, E, T, A, B, C, D, XX4, XX6, XX12, XX1);
        BODY_60_79(69, D, E, T, A, B, C, XX5, XX7, XX13, XX2);
        BODY_60_79(70, C, D, E, T, A, B, XX6, XX8, XX14, XX3);
        BODY_60_79(71, B, C, D, E, T, A, XX7, XX9, XX15, XX4);
        BODY_60_79(72, A, B, C, D, E, T, XX8, XX10, XX0, XX5);
        BODY_60_79(73, T, A, B, C, D, E, XX9, XX11, XX1, XX6);
        BODY_60_79(74, E, T, A, B, C, D, XX10, XX12, XX2, XX7);
        BODY_60_79(75, D, E, T, A, B, C, XX11, XX13, XX3, XX8);
        BODY_60_79(76, C, D, E, T, A, B, XX12, XX14, XX4, XX9);
        BODY_60_79(77, B, C, D, E, T, A, XX13, XX15, XX5, XX10);
        BODY_60_79(78, A, B, C, D, E, T, XX14, XX0, XX6, XX11);
        BODY_60_79(79, T, A, B, C, D, E, XX15, XX1, XX7, XX12);

        c->h0 = (c->h0 + E) & 0xffffffffL;
        c->h1 = (c->h1 + T) & 0xffffffffL;
        c->h2 = (c->h2 + A) & 0xffffffffL;
        c->h3 = (c->h3 + B) & 0xffffffffL;
        c->h4 = (c->h4 + C) & 0xffffffffL;

        if (--num == 0)
            break;

        A = c->h0;
        B = c->h1;
        C = c->h2;
        D = c->h3;
        E = c->h4;

    }
}

int tcsm_SHA1_update(SHA_CTX *c, const void *data_, int len) {
    const unsigned char *data = data_;
    unsigned char *p;
    SHA_LONG l;
    size_t n;

    if (len == 0)
        return ERR_TENCENTSM_OK;

    l = (c->Nl + (((SHA_LONG) len) << 3)) & 0xffffffffUL;
    if (l < c->Nl)              /* overflow */
        c->Nh++;
    c->Nh += (SHA_LONG) (len >> 29); /* might cause compiler warning on
                                       * 16-bit */
    c->Nl = l;

    n = c->num;
    if (n != 0) {
        p = (unsigned char *)c->data;

        if (len >= SHA_CBLOCK || len + n >= SHA_CBLOCK) {
            memcpy(p + n, data, SHA_CBLOCK - n);
            HASH_BLOCK_DATA_ORDER(c, p, 1);
            n = SHA_CBLOCK - n;
            data += n;
            len -= n;
            c->num = 0;
            memset(p, 0, SHA_CBLOCK); /* keep it zeroed */
        } else {
            memcpy(p + n, data, len);
            c->num += (unsigned int)len;
            return ERR_TENCENTSM_OK;
        }
    }

    n = len / SHA_CBLOCK;
    if (n > 0) {
        HASH_BLOCK_DATA_ORDER(c, data, n);
        n *= SHA_CBLOCK;
        data += n;
        len -= n;
    }

    if (len != 0) {
        p = (unsigned char *)c->data;
        c->num = (unsigned int)len;
        memcpy(p, data, len);
    }
    return ERR_TENCENTSM_OK;
}

int tcsm_SHA1_final(unsigned char *md, SHA_CTX *c)
{
    unsigned char *p = (unsigned char *)c->data;
    size_t n = c->num;

    p[n] = 0x80;                /* there is always room for one */
    n++;

    if (n > (SHA_CBLOCK - 8)) {
        memset(p + n, 0, SHA_CBLOCK - n);
        n = 0;
        HASH_BLOCK_DATA_ORDER(c, p, 1);
    }
    memset(p + n, 0, SHA_CBLOCK - 8 - n);

    p += SHA_CBLOCK - 8;
#if   defined(DATA_ORDER_IS_BIG_ENDIAN)
    (void)HOST_l2c(c->Nh, p);
    (void)HOST_l2c(c->Nl, p);
#else
    (void)HOST_l2c(c->Nl, p);
    (void)HOST_l2c(c->Nh, p);
#endif
    p -= SHA_CBLOCK;
    HASH_BLOCK_DATA_ORDER(c, p, 1);
    c->num = 0;
    memset(p, 0x00, SHA_CBLOCK);

    HASH_MAKE_STRING(c, md);

    return ERR_TENCENTSM_OK;
}

int tcsm_SHA1(const void* data, int len, unsigned char* out) {
    SHA_CTX ctx;
    if(data == NULL || len <=0 || out == NULL) {
        return ERR_ILLEGAL_ARGUMENT;
    }
    tcsm_SHA1_init(&ctx);
    tcsm_SHA1_update(&ctx, data, len);

    return tcsm_SHA1_final(out, &ctx);
}


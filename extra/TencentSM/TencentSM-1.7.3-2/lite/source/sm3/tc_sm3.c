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

#include <string.h>
#include "../include/tc_sm3.h"
#include "../include/tc_utils.h"

void tcsm_sm3_init_opt(sm3_ctx_t *ctx)
{
  ctx->digest[0] = 0x7380166F;
  ctx->digest[1] = 0x4914B2B9;
  ctx->digest[2] = 0x172442D7;
  ctx->digest[3] = 0xDA8A0600;
  ctx->digest[4] = 0xA96F30BC;
  ctx->digest[5] = 0x163138AA;
  ctx->digest[6] = 0xE38DEE4D;
  ctx->digest[7] = 0xB0FB0E4E;
  
  ctx->nblocks = 0;
  ctx->num = 0;
}

void tcsm_sm3_update_opt(sm3_ctx_t *ctx, const unsigned char* data, size_t data_len)
{
  if (ctx->num) {
    unsigned int left = SM3_BLOCK_SIZE - ctx->num;
    if (data_len < left) {
      memcpy(ctx->block + ctx->num, data, data_len);
      ctx->num += (int)data_len;
      return;
    } else {
      memcpy(ctx->block + ctx->num, data, left);
      tcsm_sm3_compress_opt(ctx->digest, ctx->block);
      ctx->nblocks++;
      data += left;
      data_len -= left;
    }
  }
  while (data_len >= SM3_BLOCK_SIZE) {
    tcsm_sm3_compress_opt(ctx->digest, data);
    ctx->nblocks++;
    data += SM3_BLOCK_SIZE;
    data_len -= SM3_BLOCK_SIZE;
  }
  ctx->num = (int)data_len;
  if (data_len) {
    memcpy(ctx->block, data, data_len);
  }
}

void tcsm_sm3_final_opt(sm3_ctx_t *ctx, unsigned char *digest)
{
  int i;
  uint32_t *pdigest = (uint32_t *)digest;
  uint32_t *count = (uint32_t *)(ctx->block + SM3_BLOCK_SIZE - 8);
  
  ctx->block[ctx->num] = 0x80;
  
  if (ctx->num + 9 <= SM3_BLOCK_SIZE) {
    memset(ctx->block + ctx->num + 1, 0, SM3_BLOCK_SIZE - ctx->num - 9);
  } else {
    memset(ctx->block + ctx->num + 1, 0, SM3_BLOCK_SIZE - ctx->num - 1);
    tcsm_sm3_compress_opt(ctx->digest, ctx->block);
    memset(ctx->block, 0, SM3_BLOCK_SIZE - 8);
  }
  
  count[0] = cpu_to_be32((ctx->nblocks) >> 23);
  count[1] = cpu_to_be32((ctx->nblocks << 9) + (ctx->num << 3));
  
  tcsm_sm3_compress_opt(ctx->digest, ctx->block);
  for (i = 0; i < sizeof(ctx->digest)/sizeof(ctx->digest[0]); i++) {
    pdigest[i] = cpu_to_be32(ctx->digest[i]);
  }
  
  ctx->digest[0] = 0x7380166F;
  ctx->digest[1] = 0x4914B2B9;
  ctx->digest[2] = 0x172442D7;
  ctx->digest[3] = 0xDA8A0600;
  ctx->digest[4] = 0xA96F30BC;
  ctx->digest[5] = 0x163138AA;
  ctx->digest[6] = 0xE38DEE4D;
  ctx->digest[7] = 0xB0FB0E4E;
  
  ctx->nblocks = 0;
  ctx->num = 0;
}


#define T0 (0x79cc4519)
#define T1 (0xf3988a32)
#define T2 (0xe7311465)
#define T3 (0xce6228cb)
#define T4 (0x9cc45197)
#define T5 (0x3988a32f)
#define T6 (0x7311465e)
#define T7 (0xe6228cbc)
#define T8 (0xcc451979)
#define T9 (0x988a32f3)
#define T10 (0x311465e7)
#define T11 (0x6228cbce)
#define T12 (0xc451979c)
#define T13 (0x88a32f39)
#define T14 (0x11465e73)
#define T15 (0x228cbce6)
#define T16 (0x9d8a7a87)
#define T17 (0x3b14f50f)
#define T18 (0x7629ea1e)
#define T19 (0xec53d43c)
#define T20 (0xd8a7a879)
#define T21 (0xb14f50f3)
#define T22 (0x629ea1e7)
#define T23 (0xc53d43ce)
#define T24 (0x8a7a879d)
#define T25 (0x14f50f3b)
#define T26 (0x29ea1e76)
#define T27 (0x53d43cec)
#define T28 (0xa7a879d8)
#define T29 (0x4f50f3b1)
#define T30 (0x9ea1e762)
#define T31 (0x3d43cec5)
#define T32 (0x7a879d8a)
#define T33 (0xf50f3b14)
#define T34 (0xea1e7629)
#define T35 (0xd43cec53)
#define T36 (0xa879d8a7)
#define T37 (0x50f3b14f)
#define T38 (0xa1e7629e)
#define T39 (0x43cec53d)
#define T40 (0x879d8a7a)
#define T41 (0x0f3b14f5)
#define T42 (0x1e7629ea)
#define T43 (0x3cec53d4)
#define T44 (0x79d8a7a8)
#define T45 (0xf3b14f50)
#define T46 (0xe7629ea1)
#define T47 (0xcec53d43)
#define T48 (0x9d8a7a87)
#define T49 (0x3b14f50f)
#define T50 (0x7629ea1e)
#define T51 (0xec53d43c)
#define T52 (0xd8a7a879)
#define T53 (0xb14f50f3)
#define T54 (0x629ea1e7)
#define T55 (0xc53d43ce)
#define T56 (0x8a7a879d)
#define T57 (0x14f50f3b)
#define T58 (0x29ea1e76)
#define T59 (0x53d43cec)
#define T60 (0xa7a879d8)
#define T61 (0x4f50f3b1)
#define T62 (0x9ea1e762)
#define T63 (0x3d43cec5)


const uint32_t Tj[64] = { T0, T1, T2, T3, T4, T5, T6, T7, T8, T9,
  T10, T11, T12, T13, T14, T15, T16, T17, T18, T19,
  T20, T21, T22, T23, T24, T25, T26, T27, T28, T29,
  T30, T31, T32, T33, T34, T35, T36, T37, T38, T39,
  T40, T41, T42, T43, T44, T45, T46, T47, T48, T49,
  T50, T51, T52, T53, T54, T55, T56, T57, T58, T59,
  T60, T61, T62, T63 };


#define ROTATELEFT(X,n)  (((X)<<(n)) | ((X)>>(32-(n))))

#define P0(x) ((x) ^  ROTATELEFT((x),9)  ^ ROTATELEFT((x),17))
#define P1(x) ((x) ^  ROTATELEFT((x),15) ^ ROTATELEFT((x),23))


#define FF0(x,y,z) ( (x) ^ (y) ^ (z))
#define FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))

#define GG0(x,y,z) ( (x) ^ (y) ^ (z))
#define GG1(x,y,z) (((x) & (y)) | ( (~(x)) & (z)) )

#define R(A, B, C, D, E, F, G, H, xx)                \
    TT1 = ROTATELEFT(A, 12);           \
    SS1 = ROTATELEFT((TT1 + E + Tj[i]), 7);        \
    SS2 = SS1 ^ TT1;                \
    TT2 = GG##xx(E, F, G) + H + SS1 + W[i];            \
    H = FF##xx(A, B, C) + D + SS2 + (W[i] ^ W[i + 4]);    \
    B = ROTATELEFT(B, 9);                    \
    F = ROTATELEFT(F, 19);                    \
    D = P0(TT2);                        \
    i++

#define R8(A, B, C, D, E, F, G, H, xx)                \
    R(A, B, C, D, E, F, G, H, xx);                \
    R(H, A, B, C, D, E, F, G, xx);                \
    R(G, H, A, B, C, D, E, F, xx);                \
    R(F, G, H, A, B, C, D, E, xx);                \
    R(E, F, G, H, A, B, C, D, xx);                \
    R(D, E, F, G, H, A, B, C, xx);                \
    R(C, D, E, F, G, H, A, B, xx);                \
    R(B, C, D, E, F, G, H, A, xx)

void tcsm_sm3_compress_opt(uint32_t digest[8], const unsigned char block[64])
{
  uint32_t W[68];
 
  uint32_t A = digest[0];
  uint32_t B = digest[1];
  uint32_t C = digest[2];
  uint32_t D = digest[3];
  uint32_t E = digest[4];
  uint32_t F = digest[5];
  uint32_t G = digest[6];
  uint32_t H = digest[7];
    
 const uint32_t *pblock = (const uint32_t *)block;
 for (int i = 0; i < 16; i++) {
    W[i] = cpu_to_be32(pblock[i]);
 }
 for (int i = 12; i < 64; i = i + 26){
    W[i + 4] = P1(W[i - 12] ^ W[i - 5] ^ ROTATELEFT(W[i + 1], 15)) ^ ROTATELEFT(W[i - 9], 7) ^ W[i - 2];
    W[i + 5] = P1(W[i - 11] ^ W[i - 4] ^ ROTATELEFT(W[i + 2], 15)) ^ ROTATELEFT(W[i - 8], 7) ^ W[i - 1];
    W[i + 6] = P1(W[i - 10] ^ W[i - 3] ^ ROTATELEFT(W[i + 3], 15)) ^ ROTATELEFT(W[i - 7], 7) ^ W[i];
    W[i + 7] = P1(W[i - 9] ^ W[i - 2] ^ ROTATELEFT(W[i + 4], 15)) ^ ROTATELEFT(W[i - 6], 7) ^ W[i + 1];
    W[i + 8] = P1(W[i - 8] ^ W[i - 1] ^ ROTATELEFT(W[i + 5], 15)) ^ ROTATELEFT(W[i - 5], 7) ^ W[i + 2];
    W[i + 9] = P1(W[i - 7] ^ W[i] ^ ROTATELEFT(W[i + 6], 15)) ^ ROTATELEFT(W[i - 4], 7) ^ W[i + 3];
    W[i + 10] = P1(W[i - 6] ^ W[i + 1] ^ ROTATELEFT(W[i + 7], 15)) ^ ROTATELEFT(W[i - 3], 7) ^ W[i + 4];
    W[i + 11] = P1(W[i - 5] ^ W[i + 2] ^ ROTATELEFT(W[i + 8], 15)) ^ ROTATELEFT(W[i - 2], 7) ^ W[i + 5];
    W[i + 12] = P1(W[i - 4] ^ W[i + 3] ^ ROTATELEFT(W[i + 9], 15)) ^ ROTATELEFT(W[i - 1], 7) ^ W[i + 6];
    W[i + 13] = P1(W[i - 3] ^ W[i + 4] ^ ROTATELEFT(W[i + 10], 15)) ^ ROTATELEFT(W[i], 7) ^ W[i + 7];
    W[i + 14] = P1(W[i - 2] ^ W[i + 5] ^ ROTATELEFT(W[i + 11], 15)) ^ ROTATELEFT(W[i + 1], 7) ^ W[i + 8];
    W[i + 15] = P1(W[i - 1] ^ W[i + 6] ^ ROTATELEFT(W[i + 12], 15)) ^ ROTATELEFT(W[i + 2], 7) ^ W[i + 9];
    W[i + 16] = P1(W[i] ^ W[i + 7] ^ ROTATELEFT(W[i + 13], 15)) ^ ROTATELEFT(W[i + 3], 7) ^ W[i + 10];
    W[i + 17] = P1(W[i + 1] ^ W[i + 8] ^ ROTATELEFT(W[i + 14], 15)) ^ ROTATELEFT(W[i + 4], 7) ^ W[i + 11];
    W[i + 18] = P1(W[i + 2] ^ W[i + 9] ^ ROTATELEFT(W[i + 15], 15)) ^ ROTATELEFT(W[i + 5], 7) ^ W[i + 12];
    W[i + 19] = P1(W[i + 3] ^ W[i + 10] ^ ROTATELEFT(W[i + 16], 15)) ^ ROTATELEFT(W[i + 6], 7) ^ W[i + 13];
    W[i + 20] = P1(W[i + 4] ^ W[i + 11] ^ ROTATELEFT(W[i + 17], 15)) ^ ROTATELEFT(W[i + 7], 7) ^ W[i + 14];
    W[i + 21] = P1(W[i + 5] ^ W[i + 12] ^ ROTATELEFT(W[i + 18], 15)) ^ ROTATELEFT(W[i + 8], 7) ^ W[i + 15];
    W[i + 22] = P1(W[i + 6] ^ W[i + 13] ^ ROTATELEFT(W[i + 19], 15)) ^ ROTATELEFT(W[i + 9], 7) ^ W[i + 16];
    W[i + 23] = P1(W[i + 7] ^ W[i + 14] ^ ROTATELEFT(W[i + 20], 15)) ^ ROTATELEFT(W[i + 10], 7) ^ W[i + 17];
    W[i + 24] = P1(W[i + 8] ^ W[i + 15] ^ ROTATELEFT(W[i + 21], 15)) ^ ROTATELEFT(W[i + 11], 7) ^ W[i + 18];
    W[i + 25] = P1(W[i + 9] ^ W[i + 16] ^ ROTATELEFT(W[i + 22], 15)) ^ ROTATELEFT(W[i + 12], 7) ^ W[i + 19];
    W[i + 26] = P1(W[i + 10] ^ W[i + 17] ^ ROTATELEFT(W[i + 23], 15)) ^ ROTATELEFT(W[i + 13], 7) ^ W[i + 20];
    W[i + 27] = P1(W[i + 11] ^ W[i + 18] ^ ROTATELEFT(W[i + 24], 15)) ^ ROTATELEFT(W[i + 14], 7) ^ W[i + 21];
    W[i + 28] = P1(W[i + 12] ^ W[i + 19] ^ ROTATELEFT(W[i + 25], 15)) ^ ROTATELEFT(W[i + 15], 7) ^ W[i + 22];
    W[i + 29] = P1(W[i + 13] ^ W[i + 20] ^ ROTATELEFT(W[i + 26], 15)) ^ ROTATELEFT(W[i + 16], 7) ^ W[i + 23];
  }

  uint32_t TT1, TT2, SS1, SS2;
  int i = 0;
  R8(A, B, C, D, E, F, G, H, 0);
  R8(A, B, C, D, E, F, G, H, 0);
  R8(A, B, C, D, E, F, G, H, 1);
  R8(A, B, C, D, E, F, G, H, 1);
  R8(A, B, C, D, E, F, G, H, 1);
  R8(A, B, C, D, E, F, G, H, 1);
  R8(A, B, C, D, E, F, G, H, 1);
  R8(A, B, C, D, E, F, G, H, 1);

  digest[0] ^= A;
  digest[1] ^= B;
  digest[2] ^= C;
  digest[3] ^= D;
  digest[4] ^= E;
  digest[5] ^= F;
  digest[6] ^= G;
  digest[7] ^= H;
}

void tcsm_sm3opt(const unsigned char *msg, size_t msglen,
            unsigned char dgst[SM3_DIGEST_LENGTH])
{
  sm3_ctx_t ctx;
  
  tcsm_sm3_init_opt(&ctx);
  tcsm_sm3_update_opt(&ctx, (const unsigned char*)msg, msglen);
  tcsm_sm3_final_opt(&ctx, dgst);
  
  memset(&ctx, 0, sizeof(sm3_ctx_t));
}

/**
Copyright 2019, Tencent Technology (Shenzhen) Co Ltd
Description: This file is part of the Tencent SM (Pro Version) Library.
*/

#include "../include/tc_sm4.h"
#include "../include/tc.h"
#include "../include/tc_gcm128_mode.h"
#include "../include/tc_str.h"
#include <string.h>

#define PACK(s)         ((size_t)(s)<<(sizeof(size_t)*8-16))
#define REDUCE1BIT(V)   do { \
        if (sizeof(size_t)==8) { \
                u64 T = U64(0xe100000000000000) & (0-(V.lo&1)); \
                V.lo  = (V.hi<<63)|(V.lo>>1); \
                V.hi  = (V.hi>>1 )^T; \
        } \
        else { \
                u32 T = 0xe1000000U & (0-(u32)(V.lo&1)); \
                V.lo  = (V.hi<<63)|(V.lo>>1); \
                V.hi  = (V.hi>>1 )^((u64)T<<32); \
        } \
} while(0)

# define GCM_MUL(ctx,Xi)   tcsm_gcm_gmult_4bit(ctx->Xi.u,ctx->Htable)

static void tcsm_gcm_init_4bit(u128 Htable[16], u64 H[2])
{
  u128 V;
  
  Htable[0].hi = 0;
  Htable[0].lo = 0;
  V.hi = H[0];
  V.lo = H[1];
  
  Htable[8] = V;
  REDUCE1BIT(V);
  Htable[4] = V;
  REDUCE1BIT(V);
  Htable[2] = V;
  REDUCE1BIT(V);
  Htable[1] = V;
  Htable[3].hi = V.hi ^ Htable[2].hi; Htable[3].lo = V.lo ^ Htable[2].lo;
  V = Htable[4];
  Htable[5].hi = V.hi ^ Htable[1].hi; Htable[5].lo = V.lo ^ Htable[1].lo;
  Htable[6].hi = V.hi ^ Htable[2].hi; Htable[6].lo = V.lo ^ Htable[2].lo;
  Htable[7].hi = V.hi ^ Htable[3].hi; Htable[7].lo = V.lo ^ Htable[3].lo;
  V = Htable[8];
  Htable[9].hi = V.hi ^ Htable[1].hi; Htable[9].lo = V.lo ^ Htable[1].lo;
  Htable[10].hi = V.hi ^ Htable[2].hi; Htable[10].lo = V.lo ^ Htable[2].lo;
  Htable[11].hi = V.hi ^ Htable[3].hi; Htable[11].lo = V.lo ^ Htable[3].lo;
  Htable[12].hi = V.hi ^ Htable[4].hi; Htable[12].lo = V.lo ^ Htable[4].lo;
  Htable[13].hi = V.hi ^ Htable[5].hi; Htable[13].lo = V.lo ^ Htable[5].lo;
  Htable[14].hi = V.hi ^ Htable[6].hi; Htable[14].lo = V.lo ^ Htable[6].lo;
  Htable[15].hi = V.hi ^ Htable[7].hi; Htable[15].lo = V.lo ^ Htable[7].lo;
}

static const size_t rem_4bit[16] = {
  PACK(0x0000), PACK(0x1C20), PACK(0x3840), PACK(0x2460),
  PACK(0x7080), PACK(0x6CA0), PACK(0x48C0), PACK(0x54E0),
  PACK(0xE100), PACK(0xFD20), PACK(0xD940), PACK(0xC560),
  PACK(0x9180), PACK(0x8DA0), PACK(0xA9C0), PACK(0xB5E0)
};

static void tcsm_gcm_gmult_4bit(u64 Xi[2], const u128 Htable[16])
{
  u128 Z;
  int cnt = 15;
  size_t rem, nlo, nhi;
  const union {
    long one;
    char little;
  } is_endian = { 1 };

  nlo = ((const u8 *)Xi)[15];
  nhi = nlo >> 4;
  nlo &= 0xf;

  Z.hi = Htable[nlo].hi;
  Z.lo = Htable[nlo].lo;

  while (1) {
    rem = (size_t)Z.lo & 0xf;
    Z.lo = (Z.hi << 60) | (Z.lo >> 4);
    Z.hi = (Z.hi >> 4);
    if (sizeof(size_t) == 8)
      Z.hi ^= rem_4bit[rem];
    else
      Z.hi ^= (u64)rem_4bit[rem] << 32;

    Z.hi ^= Htable[nhi].hi;
    Z.lo ^= Htable[nhi].lo;

    if (--cnt < 0)
      break;

    nlo = ((const u8 *)Xi)[cnt];
    nhi = nlo >> 4;
    nlo &= 0xf;

    rem = (size_t)Z.lo & 0xf;
    Z.lo = (Z.hi << 60) | (Z.lo >> 4);
    Z.hi = (Z.hi >> 4);
    if (sizeof(size_t) == 8)
      Z.hi ^= rem_4bit[rem];
    else
      Z.hi ^= (u64)rem_4bit[rem] << 32;

    Z.hi ^= Htable[nlo].hi;
    Z.lo ^= Htable[nlo].lo;
  }

  if (is_endian.little) {
    u8 *p = (u8 *)Xi;
    u32 v;
    v = (u32)(Z.hi >> 32);
    PUTU32(p, v);
    v = (u32)(Z.hi);
    PUTU32(p + 4, v);
    v = (u32)(Z.lo >> 32);
    PUTU32(p + 8, v);
    v = (u32)(Z.lo);
    PUTU32(p + 12, v);
  }
  else {
    Xi[0] = Z.hi;
    Xi[1] = Z.lo;
  }
}

void tcsm_CRYPTO_gcm128_init(TCSM_GCM128_CONTEXT *ctx, const void *key, tcsm_block128_f block)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->block = block;
  ctx->key = (void*)key;

  (*block) (ctx->H.c, ctx->H.c, key);
  
  const union {
    long one;
    char little;
  } is_endian = { 1 };
  if (is_endian.little) {
      u8 *p = ctx->H.c;
      u64 hi, lo;
      hi = (u64)GETU32(p) << 32 | GETU32(p + 4);
      lo = (u64)GETU32(p + 8) << 32 | GETU32(p + 12);
      ctx->H.u[0] = hi;
      ctx->H.u[1] = lo;
  }
  tcsm_gcm_init_4bit(ctx->Htable, ctx->H.u);
}

void tcsm_CRYPTO_gcm128_setiv(TCSM_GCM128_CONTEXT *ctx, const unsigned char *iv,size_t len)
{
  const union {
    long one;
    char little;
  } is_endian = { 1 };
  unsigned int ctr;
  
  ctx->Yi.u[0] = 0;
  ctx->Yi.u[1] = 0;
  ctx->Xi.u[0] = 0;
  ctx->Xi.u[1] = 0;
  ctx->len.u[0] = 0;          /* AAD length */
  ctx->len.u[1] = 0;          /* message length */
  ctx->ares = 0;
  ctx->mres = 0;

  if (len == 12) {
    memcpy(ctx->Yi.c, iv, 12);
    ctx->Yi.c[15] = 1;
    ctr = 1;
  }
  else {
    size_t i;
    u64 len0 = len;

    while (len >= 16) {
      for (i = 0; i < 16; ++i)
        ctx->Yi.c[i] ^= iv[i];
      GCM_MUL(ctx, Yi);
      iv += 16;
      len -= 16;
    }
    if (len) {
      for (i = 0; i < len; ++i)
        ctx->Yi.c[i] ^= iv[i];
      GCM_MUL(ctx, Yi);
    }
    len0 <<= 3;
    if (is_endian.little) {
      ctx->Yi.c[8] ^= (u8)(len0 >> 56);
      ctx->Yi.c[9] ^= (u8)(len0 >> 48);
      ctx->Yi.c[10] ^= (u8)(len0 >> 40);
      ctx->Yi.c[11] ^= (u8)(len0 >> 32);
      ctx->Yi.c[12] ^= (u8)(len0 >> 24);
      ctx->Yi.c[13] ^= (u8)(len0 >> 16);
      ctx->Yi.c[14] ^= (u8)(len0 >> 8);
      ctx->Yi.c[15] ^= (u8)(len0);
    }
    else
      ctx->Yi.u[1] ^= len0;

    GCM_MUL(ctx, Yi);

    if (is_endian.little)
      ctr = GETU32(ctx->Yi.c + 12);
    else
      ctr = ctx->Yi.d[3];
  }

  (*ctx->block) (ctx->Yi.c, ctx->EK0.c, ctx->key);
  ++ctr;
  if (is_endian.little)
    PUTU32(ctx->Yi.c + 12, ctr);
  else
    ctx->Yi.d[3] = ctr;
}

int tcsm_CRYPTO_gcm128_aad(TCSM_GCM128_CONTEXT *ctx, const unsigned char *aad,size_t len)
{
  size_t i;
  unsigned int n;
  u64 alen = ctx->len.u[0];
  
  if (ctx->len.u[1])
    return -2;

  alen += len;
  if (alen > (U64(1) << 61) || (sizeof(len) == 8 && alen < len))
    return -1;
  ctx->len.u[0] = alen;

  n = ctx->ares;
  if (n) {
    while (n && len) {
      ctx->Xi.c[n] ^= *(aad++);
      --len;
      n = (n + 1) % 16;
    }
    if (n == 0)
      GCM_MUL(ctx, Xi);
    else {
      ctx->ares = n;
      return 0;
    }
  }
  while (len >= 16) {
    for (i = 0; i < 16; ++i)
      ctx->Xi.c[i] ^= aad[i];
    GCM_MUL(ctx, Xi);
    aad += 16;
    len -= 16;
  }
  if (len) {
    n = (unsigned int)len;
    for (i = 0; i < len; ++i)
      ctx->Xi.c[i] ^= aad[i];
  }

  ctx->ares = n;
  return 0;

}

int tcsm_CRYPTO_gcm128_encrypt(TCSM_GCM128_CONTEXT *ctx,const unsigned char *in, unsigned char *out,size_t len)
{
  const union {
    long one;
    char little;
  } is_endian = { 1 };
  unsigned int n, ctr;
  size_t i;
  u64 mlen = ctx->len.u[1];
  tcsm_block128_f block = ctx->block;
  void *key = ctx->key;
  
  mlen += len;
  if (mlen > ((U64(1) << 36) - 32) || (sizeof(len) == 8 && mlen < len))
    return -1;
  ctx->len.u[1] = mlen;

  if (ctx->ares) {
    GCM_MUL(ctx, Xi);
    ctx->ares = 0;
  }
  
  if (is_endian.little)
    ctr = GETU32(ctx->Yi.c + 12);
  else
    ctr = ctx->Yi.d[3];
  
  n = ctx->mres;
  if (16 % sizeof(size_t) == 0) { /* always true actually */
    do {
      if (n) {
        while (n && len) {
          ctx->Xi.c[n] ^= *(out++) = *(in++) ^ ctx->EKi.c[n];
          --len;
          n = (n + 1) % 16;
        }
        if (n == 0)
          GCM_MUL(ctx, Xi);
        else {
          ctx->mres = n;
          return 0;
        }
      }

      unsigned char *out_t = (unsigned char *)out;
      const unsigned char *in_t = (const unsigned char *)in;
      while (len >= 16) {
          (*block) (ctx->Yi.c, ctx->EKi.c, key);
          for (i = 0; i < 16; ++i) {
            out_t[i] = in_t[i] ^ ctx->EKi.c[i];
          }
          for (i = 0; i < 16; ++i) {
            ctx->Xi.c[i] ^= out_t[i];
          }
          ++ctr;
          if (is_endian.little)
            PUTU32(ctx->Yi.c + 12, ctr);
          else
            ctx->Yi.d[3] = ctr;
          GCM_MUL(ctx, Xi);
          out_t += 16;
          in_t += 16;
          len -= 16;
      }
      if (len) {
        (*block) (ctx->Yi.c, ctx->EKi.c, key);
        ++ctr;
        if (is_endian.little)
          PUTU32(ctx->Yi.c + 12, ctr);
        else
          ctx->Yi.d[3] = ctr;
        while (len--) {
          ctx->Xi.c[n] ^= out_t[n] = in_t[n] ^ ctx->EKi.c[n];
          ++n;
        }
      }

      ctx->mres = n;
      return 0;
    } while (0);
  }
  for (i = 0; i < len; ++i) {
    if (n == 0) {
      (*block) (ctx->Yi.c, ctx->EKi.c, key);
      ++ctr;
      if (is_endian.little)
        PUTU32(ctx->Yi.c + 12, ctr);
      else
        ctx->Yi.d[3] = ctr;
    }
    ctx->Xi.c[n] ^= out[i] = in[i] ^ ctx->EKi.c[n];
    n = (n + 1) % 16;
    if (n == 0)
      GCM_MUL(ctx, Xi);
  }

  ctx->mres = n;
  return 0;
}

int tcsm_CRYPTO_gcm128_decrypt(TCSM_GCM128_CONTEXT *ctx,const unsigned char *in, unsigned char *out,size_t len)
{
  const union {
    long one;
    char little;
  } is_endian = { 1 };
  unsigned int n, ctr;
  size_t i;
  u64 mlen = ctx->len.u[1];
  tcsm_block128_f block = ctx->block;
  void *key = ctx->key;

  mlen += len;
  if (mlen > ((U64(1) << 36) - 32) || (sizeof(len) == 8 && mlen < len))
    return -1;
  ctx->len.u[1] = mlen;

  if (ctx->ares) {
    /* First call to decrypt finalizes GHASH(AAD) */
    GCM_MUL(ctx, Xi);
    ctx->ares = 0;
  }

  if (is_endian.little)
    ctr = GETU32(ctx->Yi.c + 12);
  else
    ctr = ctx->Yi.d[3];

  n = ctx->mres;

  if (16 % sizeof(size_t) == 0) { /* always true actually */
    do {
      if (n) {
        while (n && len) {
          u8 c = *(in++);
          *(out++) = c ^ ctx->EKi.c[n];
          ctx->Xi.c[n] ^= c;
          --len;
          n = (n + 1) % 16;
        }
        if (n == 0)
          GCM_MUL(ctx, Xi);
        else {
          ctx->mres = n;
          return 0;
        }
      }
      unsigned char *out_t = (unsigned char *)out;
      const unsigned char *in_t = (const unsigned char *)in;
      while (len >= 16) {
        (*block) (ctx->Yi.c, ctx->EKi.c, key);
        for (i = 0; i < 16; ++i) {
          out_t[i] = in_t[i] ^ ctx->EKi.c[i];
        }
        ++ctr;
        if (is_endian.little)
          PUTU32(ctx->Yi.c + 12, ctr);
        else
          ctx->Yi.d[3] = ctr;
        for (i = 0; i < 16; ++i) {
          ctx->Xi.c[i] ^= in_t[i];
        }
        GCM_MUL(ctx, Xi);
        out_t += 16;
        in_t += 16;
        len -= 16;
      }
      if (len) {
        (*block) (ctx->Yi.c, ctx->EKi.c, key);
        ++ctr;
        if (is_endian.little)
          PUTU32(ctx->Yi.c + 12, ctr);
        else
          ctx->Yi.d[3] = ctr;
        while (len--) {
          u8 c = in_t[n];
          ctx->Xi.c[n] ^= c;
          out_t[n] = c ^ ctx->EKi.c[n];
          ++n;
        }
      }
      ctx->mres = n;
      return 0;
    } while (0);
  }
  for (i = 0; i < len; ++i) {
    u8 c;
    if (n == 0) {
      (*block) (ctx->Yi.c, ctx->EKi.c, key);
      ++ctr;
      if (is_endian.little)
        PUTU32(ctx->Yi.c + 12, ctr);
      else
        ctx->Yi.d[3] = ctr;
    }
    c = in[i];
    out[i] = c ^ ctx->EKi.c[n];
    ctx->Xi.c[n] ^= c;
    n = (n + 1) % 16;
    if (n == 0)
      GCM_MUL(ctx, Xi);
  }

  ctx->mres = n;
  return 0;
}

int tcsm_CRYPTO_gcm128_finish(TCSM_GCM128_CONTEXT *ctx, const unsigned char *tag,size_t len)
{
  const union {
    long one;
    char little;
  } is_endian = { 1 };
  u64 alen = ctx->len.u[0] << 3;
  u64 clen = ctx->len.u[1] << 3;

  if (ctx->mres || ctx->ares)
    GCM_MUL(ctx, Xi);

  if (is_endian.little) {
    u8 *p = ctx->len.c;

    ctx->len.u[0] = alen;
    ctx->len.u[1] = clen;

    alen = (u64)GETU32(p) << 32 | GETU32(p + 4);
    clen = (u64)GETU32(p + 8) << 32 | GETU32(p + 12);
  }

  ctx->Xi.u[0] ^= alen;
  ctx->Xi.u[1] ^= clen;
  GCM_MUL(ctx, Xi);

  ctx->Xi.u[0] ^= ctx->EK0.u[0];
  ctx->Xi.u[1] ^= ctx->EK0.u[1];
  
  if (tag && len <= sizeof(ctx->Xi))
    return tcsm_secure_memcmp(ctx->Xi.c, tag, len);
  else
    return -1;
}

void tcsm_CRYPTO_gcm128_tag(TCSM_GCM128_CONTEXT *ctx, unsigned char *tag, size_t len)
{
  (void)tcsm_CRYPTO_gcm128_finish(ctx, NULL, 0);
  memcpy(tag, ctx->Xi.c,len <= sizeof(ctx->Xi.c) ? len : sizeof(ctx->Xi.c));
}

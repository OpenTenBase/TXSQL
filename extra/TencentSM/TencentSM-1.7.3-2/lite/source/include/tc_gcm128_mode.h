/*
Copyright 2019, Tencent Technology (Shenzhen) Co Ltd
Description: This file is part of the Tencent SM (Pro Version) Library.
*/

#ifndef tc_gcm128_mode_h
#define tc_gcm128_mode_h

#include "tc_modes.h"

# define GETU32(p)       ((u32)(p)[0]<<24|(u32)(p)[1]<<16|(u32)(p)[2]<<8|(u32)(p)[3])
# define PUTU32(p,v)     ((p)[0]=(u8)((v)>>24),(p)[1]=(u8)((v)>>16),(p)[2]=(u8)((v)>>8),(p)[3]=(u8)(v))

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
typedef __int64 i64;
#ifndef U64_TYPE_DEFINED
typedef unsigned __int64 u64;
#define U64_TYPE_DEFINED
#endif
# define U64(C) C##UI64
#elif defined(__arch64__)
typedef long i64;
#ifndef U64_TYPE_DEFINED
typedef unsigned long u64;
#define U64_TYPE_DEFINED
#endif
# define U64(C) C##UL
#else
typedef long long i64;
#ifndef U64_TYPE_DEFINED
typedef unsigned long long u64;
#define U64_TYPE_DEFINED
#endif
# define U64(C) C##ULL
#endif

typedef unsigned int u32;
typedef unsigned char u8;

typedef struct {
  u64 hi, lo;
} u128;

#define TABLE_BITS 4

struct tcsm_gcm128_context {
  union {
    u64 u[2];
    u32 d[4];
    u8 c[16];
    size_t t[16 / sizeof(size_t)];
  } Yi, EKi, EK0, len, Xi, H;
  
  u128 Htable[16];
  void(*gmult) (u64 Xi[2], const u128 Htable[16]);
  void(*ghash) (u64 Xi[2], const u128 Htable[16], const u8 *inp,
    size_t len);
  
  unsigned int mres, ares;
  tcsm_block128_f block;
  void *key;
};

typedef struct tcsm_gcm128_context TCSM_GCM128_CONTEXT;

void tcsm_CRYPTO_gcm128_init(TCSM_GCM128_CONTEXT *ctx, const void *key, tcsm_block128_f block);
void tcsm_CRYPTO_gcm128_setiv(TCSM_GCM128_CONTEXT *ctx, const unsigned char *iv,size_t len);
int tcsm_CRYPTO_gcm128_aad(TCSM_GCM128_CONTEXT *ctx, const unsigned char *aad,size_t len);
int tcsm_CRYPTO_gcm128_encrypt(TCSM_GCM128_CONTEXT *ctx,const unsigned char *in, unsigned char *out,size_t len);
int tcsm_CRYPTO_gcm128_decrypt(TCSM_GCM128_CONTEXT *ctx,const unsigned char *in, unsigned char *out,size_t len);
int tcsm_CRYPTO_gcm128_finish(TCSM_GCM128_CONTEXT *ctx, const unsigned char *tag,size_t len);
void tcsm_CRYPTO_gcm128_tag(TCSM_GCM128_CONTEXT *ctx, unsigned char *tag, size_t len);

#endif /* tc_gcm128_mode_h */

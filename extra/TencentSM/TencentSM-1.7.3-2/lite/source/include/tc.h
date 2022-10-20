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

#ifndef  TENCENTCRYPTO_H
#define  TENCENTCRYPTO_H

#include "gmp.h"
#include "sm.h"
#include "tc_global.h"

#define TC_SUCCESS     0
#define TC_FAILURE    -1

#define MAX_EC 8
#define MAX_BN 32

#define SM2_CURVE_MAX_BIT 256
#define SM2_PUBKEY_MAX_LEN 130
#define SM2_PRIKEY_MAX_LEN 64

typedef struct tc_bn
{
  mpz_t val;
  unsigned short b_using;
}tc_bn_st;

typedef tc_bn_st tc_bn_t[1];

typedef struct tc_ecc_point
{
  tc_bn_t x;
  tc_bn_t y;
  unsigned short b_using;
}tc_ecc_point_st;

typedef struct tc_ecc_jcb_point
{
  tc_bn_t x;
  tc_bn_t y;
  tc_bn_t z;
}tc_ecc_jcb_point_st;

typedef struct tc_ecc_group
{
  tc_bn_t p;
  tc_bn_t a;
  tc_bn_t b;
  tc_bn_t n;
  tc_bn_t Gx;
  tc_bn_t Gy;
  tc_bn_t h;
  sm2_ctx_t* ctx;
}tc_ecc_group_st;

typedef tc_ecc_point_st tc_ec_t[1];
typedef tc_ecc_jcb_point_st tc_ec_jcb_t[1];
typedef tc_ecc_group_st tc_ec_group_t[1];

/* structure for precomputed multiples of the generator */
typedef struct ec_pre_comp_st {
  const tc_ecc_group_st *group;
  size_t blocksize;
  size_t numblocks;
  size_t w;
  tc_ecc_point_st **points;
  size_t num;
  int references;
} tc_ec_pre_comp_info;

/* Memory Functions */
void *tcsm_tc_malloc(size_t size);
void tcsm_tc_free(void *p);
void* tcsm_tc_secure_malloc(size_t size);
void tcsm_tc_secure_free(void *p);

/* BIG NUMBER Functions */
void tcsm_tc_bn_init(tc_bn_t a);
void tcsm_tc_bn_clear(tc_bn_t a);

int tcsm_tc_bn_size_byte(tc_bn_t op);
int tcsm_tc_bn_set_bin(tc_bn_t a, char *bin, int len);
int tcsm_tc_bn_set_str(tc_bn_t a, char *str, int redix);

void tcsm_a2i(char *str, unsigned char *bin, unsigned int *binlen);
unsigned char *tcsm_tc_bn_get_bin(unsigned char *bin, unsigned int *len, tc_bn_t op, unsigned int n);
unsigned long int tcsm_tc_bn_get_ui(tc_bn_t op);

char *tcsm_tc_bn_get_str(char *str, tc_bn_t op);
void tcsm_tc_bn_cpy(tc_bn_t dst, tc_bn_t src);
int tcsm_tc_bn_cmp(tc_bn_t op1, tc_bn_t op2);
int tcsm_tc_bn_sgn(tc_bn_t op);
int tcsm_tc_bn_section(tc_bn_t op1, tc_bn_t op2, tc_bn_t op3);
int tcsm_tc_bn_is_bit_set(tc_bn_t a, int n);
int tcsm_tc_bn_is_zero(tc_bn_t op);
int tcsm_tc_bn_is_negtive(tc_bn_t bn);
void tcsm_tc_bn_set_words(tc_bn_t a,unsigned long * words,int nums);

void tcsm_tc_bn_add(tc_bn_t sum, tc_bn_t a, tc_bn_t b);
void tcsm_tc_bn_mul(tc_bn_t out, tc_bn_t a, tc_bn_t b);
void tcsm_tc_bn_mul_ui(tc_bn_t out, tc_bn_t a,unsigned long int n);
void tcsm_tc_bn_pow_ui(tc_bn_t rop, tc_bn_t base, unsigned long int exp);
void tcsm_tc_bn_sub(tc_bn_t sub, tc_bn_t a, tc_bn_t b);
void tcsm_tc_bn_fdiv_q(tc_bn_t q, tc_bn_t n, tc_bn_t d);

int tcsm_tc_bn_invert(tc_bn_t rop, tc_bn_t a, tc_bn_t n);
int tcsm_tc_bn_num_bits(tc_bn_t bn);

void tcsm_tc_bn_mod(tc_bn_t out, tc_bn_t a, tc_bn_t n);
void tcsm_tc_bn_mod_ui(tc_bn_t out, tc_bn_t a, unsigned long int n);

void tcsm_tc_bn_modadd(tc_bn_t sum, tc_bn_t a, tc_bn_t b, tc_bn_t n);
void tcsm_tc_bn_modsub(tc_bn_t sub, tc_bn_t a, tc_bn_t b, tc_bn_t n);
void tcsm_tc_bn_modmul(tc_bn_t out, tc_bn_t a, tc_bn_t b, tc_bn_t n);
void tcsm_tc_bn_powm(tc_bn_t rop, tc_bn_t base, tc_bn_t exp, tc_bn_t mod);
void tcsm_tc_bn_powm_ui(tc_bn_t rop, tc_bn_t base, unsigned long int exp, tc_bn_t mod);

/* ECC Functions */

void tcsm_tc_ec_init(tc_ec_t p);
void tcsm_tc_ec_jcb_init(tc_ec_jcb_t p);
void tcsm_tc_ec_init_generator(tc_ec_group_t group, tc_ec_t p);
void tcsm_tc_ec_jcb_init_generator(tc_ec_group_t group, tc_ec_jcb_t p);
void tcsm_tc_ec_clear(tc_ec_t p);
void tcsm_tc_ec_jcb_clear(tc_ec_jcb_t p);
void tcsm_tc_ec_cpy(tc_ec_t dst, tc_ec_t src);
void tcsm_tc_ec_jcb_cpy(tc_ec_jcb_t dst, tc_ec_jcb_t src);
void tcsm_tc_ec_jcb_to_afn(tc_ec_group_t group,tc_ec_t ROP, tc_ec_jcb_t P, tc_ec_group_t curve);

int tcsm_tc_ec_cmp(tc_ec_t op1, tc_ec_t op2);
int tcsm_tc_ec_jcb_cmp(tc_ec_jcb_t op1, tc_ec_jcb_t op2);
int tcsm_tc_ec_set_bin(tc_ec_t p, char *x, int xlen, char *y, int ylen);
int tcsm_tc_ec_set_str(tc_ec_t p, char *x, int xredix, char *y, int yredix);
int tcsm_tc_ec_jcb_set_str(tc_ec_jcb_t p, char *x, int xredix, char *y, int yredix,char *z, int zredix);

void tcsm_tc_ec_set_tcbn(tc_ec_t rop, tc_bn_t x, tc_bn_t y);
void tcsm_tc_ec_get_tcbn(tc_bn_t x, tc_bn_t y, tc_ec_t p);
void tcsm_tc_ec_get_bin(unsigned char *x, int *xlen, unsigned char *y, int *ylen, tc_ec_t p, unsigned int n);
void tcsm_tc_ec_get_str(char *x, char *y, tc_ec_t p);

// void tc_ec_create_group(tc_ec_group_t group, tc_bn_t p, tc_bn_t a, tc_bn_t b, tc_bn_t Gx, tc_bn_t Gy, tc_bn_t n, tc_bn_t h);
void tcsm_tc_ec_clear_group(tc_ec_group_t group);
void tcsm_tc_ec_add_inv(tc_ec_group_t group, tc_ec_t r, tc_ec_t p);
void tcsm_tc_ec_jcb_add_inv(tc_ec_group_t group, tc_ec_jcb_t r, tc_ec_jcb_t p);

int tcsm_tc_ec_double(tc_ec_group_t group, tc_ec_t r, tc_ec_t p);
int tcsm_tc_ec_jcb_double(tc_ec_group_t group, tc_ec_jcb_t ROP, tc_ec_jcb_t P);
int tcsm_tc_ec_invert(tc_ec_group_t group, tc_ec_t r, tc_ec_t p);
int tcsm_tc_ec_jcb_invert(tc_ec_group_t group, tc_ec_jcb_t r, tc_ec_jcb_t p);
int tcsm_tc_ec_mul(tc_ec_group_t group, tc_ec_t r, tc_ec_t p, tc_bn_t k);
int tcsm_tc_ec_precompute_mul(tc_ec_group_t group,tc_ec_t point,tc_ec_pre_comp_info** ptr_info);
int tcsm_tc_ec_add(tc_ec_group_t group, tc_ec_t r, tc_ec_t p, tc_ec_t q);
int tcsm_tc_ec_jcb_add(tc_ec_group_t group, tc_ec_jcb_t ROP, tc_ec_jcb_t P, tc_ec_jcb_t Q);

/* Calculate Optimize Functions */
void tcsm_init_calculate_context(sm2_ctx_t *ctx);
void tcsm_destroy_calculate_context(sm2_ctx_t *ctx);

tc_bn_t* tcsm_lock_temp_bn(sm2_ctx_t *ctx,int* index);
void tcsm_unlock_temp_bn(sm2_ctx_t *ctx,int index);
tc_ec_t* tcsm_lock_temp_ec(sm2_ctx_t *ctx,int* index);
void tcsm_unlock_temp_ec(sm2_ctx_t *ctx,int index);

#if (defined _WIN32 || defined __ANDROID__) 
FILE *fmemopen(void *buf, size_t len, const char *type);
#endif

#endif  // TENCENTCRYPTO_H



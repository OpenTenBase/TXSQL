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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/tc_global.h"
#include "../include/tc.h"
#include "../include/tc_sm2.h"
#include "../include/tc_sm3.h"
#include "../include/tc_kdf.h"
#include "../include/tc_str.h"
#include "../include/tc_rand.h"
#include "../include/tc_digest.h"
#include "../include/tc_ec_mul.h"

#ifdef CPU_BIGENDIAN
#define cpu_to_be16(v) (v)
#define cpu_to_be32(v) (v)
#else
#define cpu_to_be16(v) ((v << 8) | (v >> 8))
#define cpu_to_be32(v) ((cpu_to_be16(v) << 16) | cpu_to_be16(v >> 16))
#endif

#define SM_MD_CTX_SIZE(sm_md_type) \
((sm_md_type == SM_MD_SM3) ? sizeof(sm3_ctx_t) : 0)

#define eccparam_byte_len 32

static const unsigned char sm2_curve_a[64] =  {
  0xFF,0xFF,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,
  0x00,0x00,0x00,0x00,
  0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFC,
};

static const unsigned char sm2_curve_b[64] =  {
  0x28,0xE9,0xFA,0x9E,
  0x9D,0x9F,0x5E,0x34,
  0x4D,0x5A,0x9E,0x4B,
  0xCF,0x65,0x09,0xA7,
  0xF3,0x97,0x89,0xF5,
  0x15,0xAB,0x8F,0x92,
  0xDD,0xBC,0xBD,0x41,
  0x4D,0x94,0x0E,0x93,
};

static const unsigned char sm2_curve_Gx[64] =  {
  0x32,0xC4,0xAE,0x2C,
  0x1F,0x19,0x81,0x19,
  0x5F,0x99,0x04,0x46,
  0x6A,0x39,0xC9,0x94,
  0x8F,0xE3,0x0B,0xBF,
  0xF2,0x66,0x0B,0xE1,
  0x71,0x5A,0x45,0x89,
  0x33,0x4C,0x74,0xC7,
};

static const unsigned char sm2_curve_Gy[64] =  {
  0xBC,0x37,0x36,0xA2,
  0xF4,0xF6,0x77,0x9C,
  0x59,0xBD,0xCE,0xE3,
  0x6B,0x69,0x21,0x53,
  0xD0,0xA9,0x87,0x7C,
  0xC6,0x2A,0x47,0x40,
  0x02,0xDF,0x32,0xE5,
  0x21,0x39,0xF0,0xA0,
};

static sm2_ecc_parameters_t sm2_256_parameter =
{
  /* a */
  "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC",
  /* b */
  "28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93",
  /* p */
  "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF",
  /* Gx */
  "32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7",
  /* Gy */
  "BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0",
  /* n */
  "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123",
  /* h */
  "1",
};


static sm2_parameters_t sm2_parameters_list[] =
{
  {TCSM_NID_sm2p256v1, &sm2_256_parameter},
  {0, NULL},
};


int tcsm_sm2_create_group(tc_ec_group_t group, int nid)
{
  int i;
  sm2_ecc_parameters_t *p_param = NULL;
  tc_bn_t p, a, b, Gx, Gy, n, h;
  
  for (i = 0; sm2_parameters_list[i].nid != 0; i++)
  {
    if (sm2_parameters_list[i].nid == nid)
    {
      p_param  = sm2_parameters_list[i].param;
      break;
    }
  }
  
  if (p_param   == NULL)
  {
    return ERR_ECC_NID_NO_PARAMS;
  }
  
  tcsm_tc_bn_init(group->Gx);
  tcsm_tc_bn_init(group->Gy);
  tcsm_tc_bn_init(group->p);
  tcsm_tc_bn_init(group->a);
  tcsm_tc_bn_init(group->b);
  tcsm_tc_bn_init(group->n);
  tcsm_tc_bn_init(group->h);
  
  tcsm_tc_bn_set_str(group->Gx, p_param->Gx , 16);
  tcsm_tc_bn_set_str(group->Gy, p_param->Gy , 16);
  tcsm_tc_bn_set_str(group->p, p_param->p, 16);
  tcsm_tc_bn_set_str(group->a, p_param->a, 16);
  tcsm_tc_bn_set_str(group->b, p_param->b, 16);
  tcsm_tc_bn_set_str(group->n, p_param->n, 16);
  tcsm_tc_bn_set_str(group->h, p_param->h, 16);
  
//   tc_ec_create_group(group, p, a, b, Gx, Gy, n, h);
  
  return ERR_TENCENTSM_OK;
}

int tcsm_sm2_generate_key(tc_ec_group_t group, tc_ec_t publickey, tc_bn_t privatekey)
{
  tcsm_tc_rand_bignum(group->ctx->rand_ctx,privatekey, group->n);
  int ret = tcsm_ec_mul_for_G(group, publickey, privatekey);
  return ret;
}

int tcsm_sm2_get_cipher_size(sm2_cipher *cipher)
{
  return SIZEOF_sm2_cipher() + cipher->CipherLen - 1;
}

int tcsm_sm2_encrypt(sm2_ctx_t *ctx, tc_ec_t publickey, unsigned char *plain, unsigned int plainlen, sm2_cipher *cipher)
{
  int ret = ERR_TENCENTSM_OK,xlen, ylen,i = 0;
  unsigned char *x2y2, *t;
  
  tc_ecc_group_st* group = (tc_ecc_group_st*)ctx->group;
  
  int k_i,c1_i,kPb_i = 0;
  tc_bn_t* k = tcsm_lock_temp_bn(ctx, &k_i);
  tc_ec_t* c1 = tcsm_lock_temp_ec(ctx, &c1_i);
  tc_ec_t* kPb = tcsm_lock_temp_ec(ctx, &kPb_i);
  
  if ((x2y2 = tcsm_tc_secure_malloc((eccparam_byte_len << 1) + plainlen)) == NULL)
  {
    ret = ERR_TC_MALLOC;
    goto x2y2out;
  }
  
  if ((t = tcsm_tc_secure_malloc(plainlen)) == NULL)
  {
    ret = ERR_TC_MALLOC;
    goto tout;
  }
  
A1:
  /* A1 generate k */
#ifdef FIXED_RANDOM_NUM
  tcsm_tc_bn_set_str(*k, "12345", 16);//固定随机数
#else
  tcsm_tc_rand_bignum(ctx->rand_ctx,*k, group->n);
#endif
  
  /* A2 calculate C1 */
  
  if ((ret = tcsm_ec_mul_for_G(group, *c1, *k)) < ERR_TENCENTSM_OK)
    goto out;
  
  tcsm_tc_ec_get_bin(cipher->XCoordinate, &xlen, cipher->YCoordinate, &ylen, *c1, eccparam_byte_len);
  /* A3 S = h * Pb */
  /*
   if ((ret = tc_ec_mul_for_arbitrary_point(group, S, publickey, group->h)) < 0)
   goto out;
   
   if (!tcsm_tc_bn_cmp(S->x, zero) && !tcsm_tc_bn_cmp(S->y, zero))
   {
   ret = -3;
   goto out;
   }
   */
  
  /* A4 kPb = k * Pb */
  if (tcsm_ec_get_pre_comp_pubkey_info(group)) {
    if ((ret = tcsm_ec_mul_for_pubkey(group, (*kPb),publickey, *k)) < ERR_TENCENTSM_OK)
      goto out;
  }else{
    if ((ret = tcsm_ec_mul_for_point(group, (*kPb), publickey, *k)) < ERR_TENCENTSM_OK)
      goto out;
  }
  /* A5 calculate t = kdf(x2||y2, klen) */
  /* A5.1 get x2||y2 */
  tcsm_tc_ec_get_bin(x2y2, &xlen, x2y2 + eccparam_byte_len, &ylen, (*kPb), eccparam_byte_len);
  
  
  /* A5.2 t = kdf(x2||y2, klen) */
  if ((ret = tcsm_x9_63_kdf_sm3(x2y2, eccparam_byte_len << 1, t, plainlen)) < ERR_TENCENTSM_OK)
    goto out;
  
  static unsigned char z = 0x00;
  int tbn_is_z = 1;
  for (int i = 0; i < plainlen; i++) {
    if (memcmp((unsigned char*)t + i, &z, 1) != 0) {
      tbn_is_z = 0;
      break;
    }
  }
  if (tbn_is_z) {
    goto A1;
  }
  /*
   tcsm_tc_bn_set_bin(*tbn, (char*)t, plainlen);
   if (tcsm_tc_bn_is_zero(*tbn))
   goto A1;
   */
  
  /* A6 C2 = M ^ t */
  for (i = 0; i < plainlen; i++)
  {
    cipher->Cipher[i] = plain[i] ^ t[i];
  }    /* -------- end for -------- */
  cipher->CipherLen = plainlen;
  
  /* A7 C3 = sm3(x2 || M || y2) */
  tcsm_tc_bn_get_bin(x2y2, (unsigned int*)&xlen, (*kPb)->x, eccparam_byte_len);
  memcpy(x2y2 + eccparam_byte_len, plain, plainlen);
  tcsm_tc_bn_get_bin((unsigned char *)x2y2 + eccparam_byte_len + plainlen, (unsigned int *)&ylen, (*kPb)->y, eccparam_byte_len);
  tcsm_sm3opt((const unsigned char*)x2y2, (eccparam_byte_len << 1) + plainlen, cipher->HASH);
out:
  tcsm_unlock_temp_bn(ctx, k_i);
  tcsm_unlock_temp_ec(ctx, kPb_i);
  tcsm_unlock_temp_ec(ctx, c1_i);
  tcsm_tc_secure_free(t);
tout:
  tcsm_tc_secure_free(x2y2);
x2y2out:
  return ret;
}

int tcsm_sm2_decrypt(sm2_ctx_t *ctx, sm2_cipher *cipher, tc_bn_t privatekey, unsigned char *plain, unsigned int *plainlen)
{
  int ret = ERR_TENCENTSM_OK;
  unsigned char *x2y2, *t;
  unsigned int i, xlen, ylen;
  unsigned char u[SM3_DIGEST_SIZE] = {0};
  
  tc_ecc_group_st* group = (tc_ecc_group_st*)ctx->group;
  
  int c1_i,dBc1_i = 0;
  tc_ec_t* c1 = tcsm_lock_temp_ec(ctx, &c1_i);
  tc_ec_t* dBc1 = tcsm_lock_temp_ec(ctx, &dBc1_i);
  
  if ((x2y2 = tcsm_tc_secure_malloc(eccparam_byte_len * 2 + cipher->CipherLen)) == NULL)
  {
    ret = ERR_TC_MALLOC;
    goto x2y2out;
  }
  
  
  if ((t = tcsm_tc_secure_malloc(cipher->CipherLen)) == NULL)
  {
    ret = ERR_TC_MALLOC;
    goto tout;
  }
  
  
  /* B1 get c1 */
  tcsm_tc_ec_set_bin(*c1, (char *)cipher->XCoordinate, eccparam_byte_len, (char *)cipher->YCoordinate, eccparam_byte_len);
  
  /* B2 calculate S = h * c1 */
  /*
   if ((ret = tcsm_tc_ec_mul(group, S, c1, group->h)) < 0)
   goto out;
   
   if (!tcsm_tc_bn_cmp(S->x, zero) && !tcsm_tc_bn_cmp(S->y, zero))
   {
   ret = -3;
   goto out;
   }
   */
  
  /* B3 x2y2 = dB * C1 */
  if ((ret = tcsm_ec_mul_for_point(group, *dBc1, *c1, privatekey)) < ERR_TENCENTSM_OK)
    goto out;
  
  /* B4 calculate t = kdf(x2||y2, klen) */
  tcsm_tc_ec_get_bin(x2y2, (int*)&xlen, x2y2 + eccparam_byte_len, (int*)&ylen, *dBc1, eccparam_byte_len);
  if ((ret = tcsm_x9_63_kdf_sm3(x2y2, eccparam_byte_len * 2, t, cipher->CipherLen)) < ERR_TENCENTSM_OK)
    goto out;
  
  static unsigned char z = 0x00;
  int tbn_is_z = 1;
  for (int i = 0; i < cipher->CipherLen; i++) {
    if (memcmp((unsigned char*)t + i, &z, 1) != 0) {
      tbn_is_z = 0;
      break;
    }
  }
  if (tbn_is_z) {
    ret = ERR_SM2_DEC_TBN_IS_Z;
    goto out;
  }
  
  /* B5 M` = c2 ^ t */
  for (i = 0; i < cipher->CipherLen; i++)
  {
    plain[i] = t[i] ^ cipher->Cipher[i];
  }    /* -------- end for -------- */
  *plainlen = cipher->CipherLen;
  
  /* B6 u = Hash(x2 || M` || y2 ) */
  tcsm_tc_bn_get_bin(x2y2, &xlen, (*dBc1)->x, eccparam_byte_len);
  memcpy(x2y2 + eccparam_byte_len, plain, *plainlen);
  tcsm_tc_bn_get_bin(x2y2 + eccparam_byte_len + *plainlen, &ylen, (*dBc1)->y, eccparam_byte_len);
  tcsm_sm3opt((const unsigned char*)x2y2, *plainlen + eccparam_byte_len * 2, u);
  /* B7 u == C3 */
  if (tcsm_secure_memcmp(u, cipher->HASH, SM3_DIGEST_SIZE))
  {
    ret = ERR_SM2_DECODE;
  }
out:
  tcsm_unlock_temp_ec(ctx, c1_i);
  tcsm_unlock_temp_ec(ctx, dBc1_i);
  tcsm_tc_secure_free(t);
tout:
  tcsm_tc_secure_free(x2y2);
x2y2out:
  return ret;
}

int tcsm_sm2_compute_message_digest(tc_ec_group_t group,tc_ec_t pubkey,SM_MD_TYPE id_md_type, SM_MD_TYPE msg_md_type,
                               const void *msg, size_t msglen, const char* id, size_t idlen, unsigned char *dgst,
                               size_t *dgstlen)
{
  int ret = ERR_TENCENTSM_OK;
  unsigned char buf[SM3_DIGEST_LENGTH];
  unsigned int len = SM3_DIGEST_LENGTH;
  void *sm_md_ctx = NULL;
  
  sm_md_ctx = tcsm_tc_secure_malloc(SM_MD_CTX_SIZE(msg_md_type));
  
  if ((ret = tcsm_SM_DigestInit(msg_md_type, sm_md_ctx)) != ERR_TENCENTSM_OK) {
    goto err;
  }
  
  if ((ret = tcsm_sm2_getz(group, id, (unsigned int)idlen, pubkey, buf)) != ERR_TENCENTSM_OK) {
    goto err;
  }
  
  if ((ret = tcsm_SM_DigestUpdate(msg_md_type, sm_md_ctx, buf, len)) != ERR_TENCENTSM_OK) {
    goto err;
  }
  
  if ((ret = tcsm_SM_DigestUpdate(msg_md_type, sm_md_ctx, msg, msglen)) != ERR_TENCENTSM_OK) {
    goto err;
  }
  
  if ((ret = tcsm_SM_DigestFinal(msg_md_type, sm_md_ctx, dgst, &len)) != ERR_TENCENTSM_OK) {
    goto err;
  }
  
  *dgstlen = len;
  ret = ERR_TENCENTSM_OK;
err:
  if (sm_md_ctx) tcsm_tc_secure_free(sm_md_ctx);
  return ret;
}


int tcsm_sm2_sign(sm2_ctx_t *ctx, tc_bn_t privatekey, unsigned char *digst, unsigned int digstlen, sm2_signature *sign)
{
  int ret = ERR_TENCENTSM_OK;
  int len = 0;
  
  tc_ecc_group_st* group = (tc_ecc_group_st*)ctx->group;
  
  int e_i,k_i,r_i,s_i,r_k_i,one_i,dA_1_i,dA_1_invert_i,rda_i,k_rda_i = 0;
  tc_bn_t* e = tcsm_lock_temp_bn(ctx, &e_i);
  tc_bn_t* k = tcsm_lock_temp_bn(ctx, &k_i);
  tc_bn_t* r = tcsm_lock_temp_bn(ctx, &r_i);
  tc_bn_t* s = tcsm_lock_temp_bn(ctx, &s_i);
  tc_bn_t* one = tcsm_lock_temp_bn(ctx, &one_i);
  tc_bn_t* dA_1 = tcsm_lock_temp_bn(ctx, &dA_1_i);
  tc_bn_t* dA_1_invert = tcsm_lock_temp_bn(ctx, &dA_1_invert_i);
  tc_bn_t* rda = tcsm_lock_temp_bn(ctx, &rda_i);
  tc_bn_t* r_k = tcsm_lock_temp_bn(ctx, &r_k_i);
  tc_bn_t* k_rda = tcsm_lock_temp_bn(ctx, &k_rda_i);
  
  int x1y1_i = 0;
  tc_ec_t* x1y1 = tcsm_lock_temp_ec(ctx, &x1y1_i);
  
  tcsm_tc_bn_set_str(*one, "1", 16);
  
  /* A2 e */
  tcsm_tc_bn_set_bin(*e, (char*)digst, digstlen);
  
A3:
  /* A3 generate k */
  if (ctx->sign_random != NULL) {
    tcsm_tc_bn_set_str(*k, (char*)ctx->sign_random, 16);
    tcsm_tc_bn_mod(*k, *k, group->n);
  }else{
#ifdef FIXED_RANDOM_NUM
    tcsm_tc_bn_set_str(*k, "12345", 16);//固定随机数
#else
    tcsm_tc_rand_bignum(ctx->rand_ctx,*k, group->n);
#endif
  }
  
  /* A4 (x1,y1) = k * G */
  if ((ret = tcsm_ec_mul_for_G(group, *x1y1, *k)) != ERR_TENCENTSM_OK)
    goto out;
  
  /* A5 r = (e + x1) mod n */
  tcsm_tc_bn_modadd(*r, *e, (*x1y1)->x, group->n);
  
  /* A5.1 if r == 0, return to A3 */
  if (tcsm_tc_bn_is_zero(*r))
    goto A3;
  
  /* A5.1 if r + k == n, return to A3 */
  tcsm_tc_bn_add(*r_k, *r, *k);
  if (tcsm_tc_bn_cmp(*r_k, group->n) == 0)
    goto A3;
  
  /* A6 s = ((1 + dA)^-1 * (k - r*da)) mod n */
  /* A6.1 1 + dA */
  tcsm_tc_bn_add(*dA_1, *one, privatekey);
  /* A6.2 (1 + dA)^-1 */
  if ((ret = tcsm_tc_bn_invert(*dA_1_invert, *dA_1, group->n)) != ERR_TENCENTSM_OK)
    goto out;
  /* A6.3 r*da */
  tcsm_tc_bn_mul(*rda, *r, privatekey);
  /* A6.4 k - r*da */
  tcsm_tc_bn_sub(*k_rda, *k, *rda);
  /* A6.5 ((1 + dA)^-1 * (k - r*da)) */
  tcsm_tc_bn_mul(*s, *dA_1_invert, *k_rda);
  /* A6.6 s = ((1 + dA)^-1 * (k - r*da)) mod n */
  tcsm_tc_bn_mod(*s, *s, group->n);
  /* A6.7 if s == 0 return A3 */
  if (tcsm_tc_bn_is_zero(*s))
    goto A3;
  
  tcsm_tc_bn_get_bin(sign->r, (unsigned int *)&len, *r, eccparam_byte_len);
  tcsm_tc_bn_get_bin(sign->s, (unsigned int *)&len, *s, eccparam_byte_len);
  ret = ERR_TENCENTSM_OK;
out:
  tcsm_unlock_temp_bn(ctx, e_i);
  tcsm_unlock_temp_bn(ctx, k_i);
  tcsm_unlock_temp_bn(ctx, r_i);
  tcsm_unlock_temp_bn(ctx, s_i);
  tcsm_unlock_temp_bn(ctx, one_i);
  tcsm_unlock_temp_bn(ctx, dA_1_i);
  tcsm_unlock_temp_bn(ctx, dA_1_invert_i);
  tcsm_unlock_temp_bn(ctx, rda_i);
  tcsm_unlock_temp_bn(ctx, r_k_i);
  tcsm_unlock_temp_bn(ctx, k_rda_i);
  tcsm_unlock_temp_ec(ctx, x1y1_i);
  return ret;
}

int tcsm_sm2_verify(sm2_ctx_t *ctx, tc_ec_t publickey, sm2_signature *sign, unsigned char *digst, unsigned int digstlen)
{
  int ret = ERR_TENCENTSM_OK;
  
  tc_ecc_group_st* group = (tc_ecc_group_st*)ctx->group;
  
  int x1y1_i,sG_i,tPa_i = 0;
  tc_ec_t* x1y1 = tcsm_lock_temp_ec(ctx, &x1y1_i);
  tc_ec_t* sG = tcsm_lock_temp_ec(ctx, &sG_i);
  tc_ec_t* tPa = tcsm_lock_temp_ec(ctx, &tPa_i);
  
  int e_i,r_i,s_i,one_i,t_i,R_i = 0;
  tc_bn_t* e = tcsm_lock_temp_bn(ctx, &e_i);
  tc_bn_t* r = tcsm_lock_temp_bn(ctx, &r_i);
  tc_bn_t* s = tcsm_lock_temp_bn(ctx, &s_i);
  tc_bn_t* one = tcsm_lock_temp_bn(ctx, &one_i);
  tc_bn_t* t = tcsm_lock_temp_bn(ctx, &t_i);
  tc_bn_t* R = tcsm_lock_temp_bn(ctx, &R_i);
  
  tcsm_tc_bn_set_str(*one, "1", 16);
  tcsm_tc_bn_set_bin(*r, (char*)sign->r, 32);
  tcsm_tc_bn_set_bin(*s, (char*)sign->s, 32);
  /* B1 1 <= r <= n - 1 */
  if ((ret = tcsm_tc_bn_section(*one, *r, group->n)) != ERR_TENCENTSM_OK)
    goto out;
  
  /* B2 1 <= s <= n - 1 */
  if ((ret = tcsm_tc_bn_section(*one, *s, group->n)) != ERR_TENCENTSM_OK)
    goto out;
  
  /* B5 t = r + s */
  tcsm_tc_bn_modadd(*t, *r, *s, group->n);
  if (tcsm_tc_bn_is_zero(*t))
  {
    ret = ERR_SM2_VERIFY;
    goto out;
  }
  
  /* B6 (x1,y1) = s * G + t * Pa */
  /* B6.1 sG = s * G */
  if ((ret = tcsm_ec_mul_for_G(group, *sG, *s)) != ERR_TENCENTSM_OK)
  {
    goto out;
  }
  
  /* B6.2 tPa = t * Pa */
  if (tcsm_ec_get_pre_comp_pubkey_info(group)) {
    if ((ret = tcsm_ec_mul_for_pubkey(group, *tPa,publickey, *t)) < ERR_TENCENTSM_OK)
      goto out;
  }else{
    if ((ret = tcsm_ec_mul_for_point(group, *tPa, publickey, *t)) != ERR_TENCENTSM_OK)
    {
      goto out;
    }
  }
  
  
  /* B6.3 (x1,y1) = sG + tPa */
  if ((ret = tcsm_tc_ec_add(group, (*x1y1), *sG, *tPa)) != ERR_TENCENTSM_OK)
  {
    goto out;
  }
  
  /* B7 r == R, R = (e + x1) mod n */
  tcsm_tc_bn_set_bin(*e, (char*)digst, digstlen);
  tcsm_tc_bn_modadd(*R, *e, (*x1y1)->x, group->n);
  
  if (tcsm_tc_bn_cmp(*R, *r) == 0)
  {
    ret = ERR_TENCENTSM_OK;
  }
  else
  {
    ret = ERR_SM2_VERIFY;
  }    /* -------- end else -------- */
  
out:
  tcsm_unlock_temp_bn(ctx, e_i);
  tcsm_unlock_temp_bn(ctx, t_i);
  tcsm_unlock_temp_bn(ctx, r_i);
  tcsm_unlock_temp_bn(ctx, s_i);
  tcsm_unlock_temp_bn(ctx, one_i);
  tcsm_unlock_temp_bn(ctx, R_i);
  tcsm_unlock_temp_ec(ctx, x1y1_i);
  tcsm_unlock_temp_ec(ctx, sG_i);
  tcsm_unlock_temp_ec(ctx, tPa_i);
  return ret;
}

int tcsm_sm2_point_is_on_curve(sm2_ctx_t *ctx,tc_ec_t point)
{
    tc_ecc_group_st* group = (tc_ecc_group_st*)ctx->group;
    
    int ret = ERR_TENCENTSM_OK;
    int a1_i,a2_i,a3_i,right_i,left_i = 0;
    tc_bn_t* a1 = tcsm_lock_temp_bn(ctx, &a1_i);
    tc_bn_t* a2 = tcsm_lock_temp_bn(ctx, &a2_i);
    tc_bn_t* a3 = tcsm_lock_temp_bn(ctx, &a3_i);
    tc_bn_t* right = tcsm_lock_temp_bn(ctx, &right_i);
    tc_bn_t* left = tcsm_lock_temp_bn(ctx, &left_i);
    
    tcsm_tc_bn_powm_ui(*left, point->y, 2, group->p);
    
    tcsm_tc_bn_powm_ui(*a1, point->x, 3, group->p);
    
    tcsm_tc_bn_modmul(*a2, point->x, group->a, group->p);
    
    tcsm_tc_bn_modadd(*a3, *a1, *a2, group->p);
    
    tcsm_tc_bn_modadd(*right, *a3, group->b, group->p);
    
    if(tcsm_tc_bn_cmp(*left, *right) != 0) {
        ret = ERR_ECC_POINT_NOTINCURVE;
    }
    
    tcsm_unlock_temp_bn(ctx, a1_i);
    tcsm_unlock_temp_bn(ctx, a2_i);
    tcsm_unlock_temp_bn(ctx, a3_i);
    tcsm_unlock_temp_bn(ctx, right_i);
    tcsm_unlock_temp_bn(ctx, left_i);
    
    return ret;
}

int tcsm_sm2_key_exchange_U(sm2_ctx_t *ctx,tc_ec_t RB,tc_ec_t PB ,tc_bn_t tA,tc_ec_t U)
{
    char debug_hex[1024] = {0};
    int ret = ERR_TENCENTSM_OK;
    
    tc_ecc_group_st* group = (tc_ecc_group_st*)ctx->group;
    
    int tmp_i,_x2_i = 0;
    tc_bn_t* tmp = tcsm_lock_temp_bn(ctx, &tmp_i);
    tc_bn_t* _x2 = tcsm_lock_temp_bn(ctx, &_x2_i);
    
    int w_power_of_2_i,w_power_of_2_sub_1_i = 0;//w = 127
    tc_bn_t* w_power_of_2 = tcsm_lock_temp_bn(ctx, &w_power_of_2_i);
    tc_bn_t* w_power_of_2_sub_1 = tcsm_lock_temp_bn(ctx, &w_power_of_2_sub_1_i);
    tcsm_tc_bn_set_str(*w_power_of_2, "80000000000000000000000000000000", 16);
    tcsm_tc_bn_set_str(*w_power_of_2_sub_1, "7fffffffffffffffffffffffffffffff", 16);
    
    unsigned char * bin = tcsm_tc_secure_malloc(32);
    unsigned int len = 32;
    
    unsigned char * bin_x2 = tcsm_tc_secure_malloc(32);
    unsigned int len_x2 = 32;
    
    unsigned char * bin_w_1 = tcsm_tc_secure_malloc(32);
    unsigned int len_w_1 = 32;
    
    tcsm_tc_bn_get_bin(bin_w_1, &len_w_1, *w_power_of_2_sub_1, 32);
    
    tcsm_tc_bn_get_bin(bin_x2, &len_x2, RB->x, 32);
    
    for (int i = 0; i < len; i ++) {
        bin[i] = bin_x2[i] & bin_w_1[i];
    }
    
    tcsm_tc_bn_set_bin(*tmp, (char*)bin, len);
    tcsm_tc_bn_modadd(*_x2, *w_power_of_2, *tmp, group->n);
    
    tcsm_tc_secure_free(bin);tcsm_tc_secure_free(bin_x2);tcsm_tc_secure_free(bin_w_1);
    
    int point1_i,point2_i = 0;
    tc_ec_t* point1 = tcsm_lock_temp_ec(ctx, &point1_i);
    tc_ec_t* point2 = tcsm_lock_temp_ec(ctx, &point2_i);
    
    if ((ret = tcsm_ec_mul_for_point(group, *point1, RB, *_x2)) != ERR_TENCENTSM_OK)
    {
      goto out;
    }
    
    if ((ret = tcsm_tc_ec_add(group, *point2, PB, *point1)) != ERR_TENCENTSM_OK)
    {
      goto out;
    }
    
    if ((ret = tcsm_ec_mul_for_point(group, U, *point2, tA)) != ERR_TENCENTSM_OK)
    {
      goto out;
    }
    
    tcsm_tc_bn_get_str(debug_hex,U->y);
out:
    tcsm_unlock_temp_bn(ctx, tmp_i);
    tcsm_unlock_temp_bn(ctx, _x2_i);
    tcsm_unlock_temp_bn(ctx, w_power_of_2_i);
    tcsm_unlock_temp_bn(ctx, w_power_of_2_sub_1_i);
    tcsm_unlock_temp_ec(ctx, point1_i);
    tcsm_unlock_temp_ec(ctx, point2_i);
    return ret;
}

int tcsm_sm2_key_exchange_tA(sm2_ctx_t *ctx,tc_ec_t RA,tc_bn_t rA,tc_bn_t dA,tc_bn_t tA)
{
    char debug_hex[1024] = {0};
    int ret = ERR_TENCENTSM_OK;
  
    tc_ecc_group_st* group = (tc_ecc_group_st*)ctx->group;
    
    int tmp_i,_x1_i = 0;
    tc_bn_t* tmp = tcsm_lock_temp_bn(ctx, &tmp_i);
    tc_bn_t* _x1 = tcsm_lock_temp_bn(ctx, &_x1_i);
    
    int w_power_of_2_i,w_power_of_2_sub_1_i = 0;//w = 127
    tc_bn_t* w_power_of_2 = tcsm_lock_temp_bn(ctx, &w_power_of_2_i);
    tc_bn_t* w_power_of_2_sub_1 = tcsm_lock_temp_bn(ctx, &w_power_of_2_sub_1_i);
    tcsm_tc_bn_set_str(*w_power_of_2, "80000000000000000000000000000000", 16);
    tcsm_tc_bn_set_str(*w_power_of_2_sub_1, "7fffffffffffffffffffffffffffffff", 16);
    
    unsigned char * bin = tcsm_tc_secure_malloc(32);
    unsigned int len = 32;
    
    unsigned char * bin_x1 = tcsm_tc_secure_malloc(32);
    unsigned int len_x1 = 32;
    
    unsigned char * bin_w_1 = tcsm_tc_secure_malloc(32);
    unsigned int len_w_1 = 32;
    
    tcsm_tc_bn_get_bin(bin_w_1, &len_w_1, *w_power_of_2_sub_1, 32);
    
    tcsm_tc_bn_get_bin(bin_x1, &len_x1, RA->x, 32);
    
    for (int i = 0; i < len; i ++) {
        bin[i] = bin_x1[i] & bin_w_1[i];
    }
    
    tcsm_tc_bn_set_bin(*tmp, (char*)bin, len);
    tcsm_tc_bn_modadd(*_x1, *w_power_of_2, *tmp, group->n);
    
    tcsm_tc_secure_free(bin);tcsm_tc_secure_free(bin_x1);tcsm_tc_secure_free(bin_w_1);
    
    tcsm_tc_bn_modmul(*tmp, *_x1, rA, group->n);
    
    tcsm_tc_bn_modadd(tA, dA, *tmp, group->n);
    
    tcsm_tc_bn_get_str(debug_hex, tA);
    
    tcsm_unlock_temp_bn(ctx, tmp_i);
    tcsm_unlock_temp_bn(ctx, _x1_i);
    tcsm_unlock_temp_bn(ctx, w_power_of_2_i);
    tcsm_unlock_temp_bn(ctx, w_power_of_2_sub_1_i);
    
    return ret;
}

int tcsm_sm2_key_exchange_kdf(sm2_ctx_t *ctx,tc_ec_t U,tc_ec_t PA,tc_ec_t PB,const char* idA, size_t idAlen,const char* idB, size_t idBlen,size_t klen,unsigned char* KA,int A_is_initiator)
{
    int ret = ERR_TENCENTSM_OK;
    
    unsigned char buf[2*eccparam_byte_len + 2*SM3_DIGEST_LENGTH] = {0};
    
    tc_ecc_group_st* group = (tc_ecc_group_st*)ctx->group;
    
    unsigned int U_x_len = eccparam_byte_len;
    unsigned int U_y_len = eccparam_byte_len;
    
    tcsm_tc_bn_get_bin(buf, &U_x_len, U->x, eccparam_byte_len);
    tcsm_tc_bn_get_bin(buf + eccparam_byte_len, &U_y_len, U->y,eccparam_byte_len);
    
    if (A_is_initiator) {
        if ((ret = tcsm_sm2_getz(group, idA, (unsigned int)idAlen, PA, buf + 2*eccparam_byte_len))) {
          goto err;
        }
        if ((ret = tcsm_sm2_getz(group, idB, (unsigned int)idBlen, PB, buf + 2*eccparam_byte_len + SM3_DIGEST_LENGTH))) {
          goto err;
        }
    }else{
        if ((ret = tcsm_sm2_getz(group, idB, (unsigned int)idBlen, PB, buf + 2*eccparam_byte_len))) {
          goto err;
        }
        if ((ret = tcsm_sm2_getz(group, idA, (unsigned int)idAlen, PA, buf + 2*eccparam_byte_len + SM3_DIGEST_LENGTH))) {
          goto err;
        }
    }
    
    if ((ret = tcsm_x9_63_kdf_sm3(buf, 2*eccparam_byte_len + 2*SM3_DIGEST_LENGTH, KA, klen)) < ERR_TENCENTSM_OK)
      goto err;
    
    char hex[1024] = {0};
    tcsm_bin2hex(buf, 128, hex, 1024);
    
err:
    return ret;
}

int tcsm_sm2_getz(tc_ec_group_t group, const char *id, unsigned int id_len, tc_ec_t publickey, unsigned char *za)
{
  unsigned char *p, *z;
  unsigned int tmplen;
  unsigned int len;
  unsigned short idBitLen = id_len * 8;
  
  if ((z = tcsm_tc_secure_malloc(2 + id_len + eccparam_byte_len * 6 + 1)) == NULL)
    return ERR_TC_MALLOC;
  
  p = z;
  *p = (idBitLen >> 8) & 0xff;
  *(p + 1) = idBitLen & 0xff;
  p += sizeof(idBitLen);
  
  memcpy(p, id, id_len);
  p += id_len;
  
  memcpy(p, sm2_curve_a, 32);p += 32;
  memcpy(p, sm2_curve_b, 32);p += 32;
  memcpy(p, sm2_curve_Gx, 32);p += 32;
  memcpy(p, sm2_curve_Gy, 32);p += 32;
  
  if (group->ctx->pubkey_x && group->ctx->pubkey_y) {
    memcpy(p, group->ctx->pubkey_x, 32);p += 32;
    memcpy(p, group->ctx->pubkey_y, 32);p += 32;
  }else{
    tcsm_tc_bn_get_bin(p, &tmplen, publickey->x, eccparam_byte_len);
    p += eccparam_byte_len;
    
    tcsm_tc_bn_get_bin(p, &tmplen, publickey->y, eccparam_byte_len);
    p += eccparam_byte_len;
  }
  
  len = 2 + id_len + eccparam_byte_len * 6;
  tcsm_sm3opt((const unsigned char*)z, len, za);
  tcsm_tc_secure_free(z);
  return ERR_TENCENTSM_OK;
}

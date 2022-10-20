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

#include "include/tc_global.h"
#include "include/sm.h"
#include "include/tc.h"
#include "include/tc_err.h"
#include "include/tc_naf.h"
#include "include/tc_sm2.h"
#include "include/tc_sm3.h"
#include "include/tc_sm4.h"
#include "include/tc_str.h"
#include "include/tc_rand.h"
#include "include/tc_utils.h"
#include "include/tc_ec_mul.h"
#include "include/tc_asn1.h"
#include "include/sm_jni.h"
#include "include/tc_kdf.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

#ifdef _TSM_LOGFILE
    FILE* gflog;
#endif

const char* version()
{
  SM_LOGA("TencentSM VERSION:%s [%s %s]\n", TENCENTSM_VERSION,  __DATE__, __TIME__);
#ifdef _OPT_ASM_ECC
    LOGV("defined _OPT_ASM_ECC-------------\n");
#endif

  return TENCENTSM_VERSION;
}

int SM2CtxSize()
{
  return sizeof(sm2_ctx_t);
}

int SM2InitCtxInner(sm2_ctx_t *ctx)
{
  tc_ec_group_t* group = (tc_ec_group_t*)tcsm_tc_secure_malloc(sizeof(tc_ec_group_t));
  tc_bn_t* bns = (tc_bn_t*)tcsm_tc_secure_malloc(sizeof(tc_bn_t)*MAX_BN);
  tc_ec_t* ecs = (tc_ec_t*)tcsm_tc_secure_malloc(sizeof(tc_ec_t)*MAX_EC);
  void* r_ctx = tcsm_tc_rand_init();
  
  LOGV("------------------SM2InitCtxInner ecs=%ld\n", ecs);
  tcsm_sm2_create_group(*group, TCSM_NID_sm2p256v1);
  
  (*group)->ctx= ctx;
  ctx->group = (void*)group;
  ctx->bn_vars = bns;
  ctx->ec_vars = ecs;
  ctx->rand_ctx = r_ctx;
  
  tc_ecc_point_st* generator = (tc_ecc_point_st*)tcsm_tc_secure_malloc(sizeof(tc_ecc_point_st));
  tcsm_tc_ec_init_generator(ctx->group, generator);
  ctx->generator = generator;
  
  tc_ecc_jcb_point_st* jcb_generator = (tc_ecc_jcb_point_st*)tcsm_tc_secure_malloc(sizeof(tc_ecc_jcb_point_st));
  tcsm_tc_ec_jcb_init_generator(ctx->group, jcb_generator);
  ctx->jcb_generator = jcb_generator;
  
  ctx->jcb_compute_var = (tc_ecc_jcb_point_st*)tcsm_tc_secure_malloc(sizeof(tc_ecc_jcb_point_st));
  
  tcsm_init_calculate_context(ctx);

  tc_ec_pre_comp_info* pre_comp_g = ctx->pre_comp_g;
  tcsm_tc_ec_precompute_mul(ctx->group,generator,&pre_comp_g);
  ctx->pre_comp_g = pre_comp_g;
  ctx->pre_comp_p = NULL;
  
  ctx->pubkey_x = NULL;
  ctx->pubkey_y = NULL;
  ctx->sign_random = NULL;
    return ERR_TENCENTSM_OK;
}
int SM2InitCtx(sm2_ctx_t *ctx)
{
    return SM2InitCtxInner(ctx);
}

int SM2InitCtxWithPubKey(sm2_ctx_t *ctx,const char* pubkey)
{
  int iret = SM2InitCtx(ctx);
  if(iret != ERR_TENCENTSM_OK) {
      return iret;
  }
  
  tc_ec_t pubk;
  tcsm_tc_ec_init(pubk);
  tcsm_public_key_set_str(pubkey, pubk);
  
  tcsm_ec_mul_precompute_for_pubkey(ctx->group, pubk);
  
  unsigned int tmplen;
  ctx->pubkey_x = tcsm_tc_secure_malloc(32);
  ctx->pubkey_y = tcsm_tc_secure_malloc(32);
  tcsm_tc_bn_get_bin(ctx->pubkey_x, &tmplen, pubk->x, 32);
  tcsm_tc_bn_get_bin(ctx->pubkey_y, &tmplen, pubk->y, 32);
  tcsm_tc_ec_clear(pubk);
  return ERR_TENCENTSM_OK;
}

int SM2FreeCtxInner(sm2_ctx_t *ctx)
{
  tcsm_destroy_calculate_context(ctx);
  
  const tc_ec_pre_comp_info *pre_comp = ctx->pre_comp_g;
  if (pre_comp) {
    if (pre_comp->points) {
      
      for (int i = 0; i < PRE_COMPUTE_POINTS_COUNT; i++) {
        tcsm_tc_ec_clear((pre_comp->points)[i]);
        tcsm_tc_secure_free((pre_comp->points)[i]);
      }
      
      tcsm_tc_secure_free(pre_comp->points);
    }
    tcsm_tc_secure_free((void*)pre_comp);
  }
  tcsm_tc_ec_clear_group(*(tc_ec_group_t*)ctx->group);
  tcsm_tc_secure_free(ctx->group);
  ctx->group = NULL;
  
  tcsm_tc_secure_free(ctx->bn_vars);
  tcsm_tc_secure_free(ctx->ec_vars);
  tcsm_tc_ec_clear(ctx->generator);
  tcsm_tc_secure_free(ctx->generator);
  tcsm_tc_ec_jcb_clear(ctx->jcb_generator);
  tcsm_tc_secure_free(ctx->jcb_generator);
  tcsm_tc_secure_free(ctx->jcb_compute_var);
  tcsm_tc_rand_clear(ctx->rand_ctx);
  tcsm_tc_secure_free(ctx->rand_ctx);
  
  if (ctx->sign_random != NULL)
  {
      tcsm_tc_secure_free(ctx->sign_random);
      ctx->sign_random = NULL;
  }
    
  const tc_ec_pre_comp_info *pre_comp_pubkey = ctx->pre_comp_p;
  if (pre_comp_pubkey) {
#ifdef _OPT_ASM_ECC
    tcsm_tc_secure_free((void*)pre_comp_pubkey);
#else
    if (pre_comp_pubkey->points) {
      
      for (int i = 0; i < PRE_COMPUTE_POINTS_COUNT; i++) {
        tcsm_tc_ec_clear((pre_comp_pubkey->points)[i]);
        tcsm_tc_secure_free((pre_comp_pubkey->points)[i]);
      }
      
      tcsm_tc_secure_free(pre_comp_pubkey->points);
    }
    tcsm_tc_secure_free((void*)pre_comp_pubkey);
#endif
    tcsm_tc_secure_free(ctx->pubkey_x);
    tcsm_tc_secure_free(ctx->pubkey_y);
  }
  return ERR_TENCENTSM_OK;
}
int SM2FreeCtx(sm2_ctx_t *ctx)
{
    return SM2FreeCtxInner(ctx);
}

int SM2SetRandomDataCtx(sm2_ctx_t *ctx, const char *sign_random)
{
  size_t sign_random_len = strlen(sign_random);
  
  if (ctx->sign_random != NULL)
  {
      tcsm_tc_secure_free(ctx->sign_random);
      ctx->sign_random = NULL;
  }
  
  ctx->sign_random = tcsm_tc_secure_malloc(sign_random_len + 1);
  
  memset(ctx->sign_random, 0, sign_random_len + 1);
  memcpy(ctx->sign_random, sign_random, sign_random_len);
    return ERR_TENCENTSM_OK;
}

int IsSM2CtxRandomDataVaild(sm2_ctx_t *sm2ctx)
{
  int ret = ERR_TENCENTSM_OK;
  int i_k = 0;
  
  tc_ecc_group_st* group = (tc_ecc_group_st*)sm2ctx->group;
  
  tc_bn_t* k = tcsm_lock_temp_bn(sm2ctx, &i_k);

  if (sm2ctx == NULL || sm2ctx->group == NULL || sm2ctx->sign_random == NULL)
  {
    ret = ERR_ILLEGAL_ARGUMENT;
    goto err;
  }
  
  tcsm_tc_bn_set_str(*k, (char*)sm2ctx->sign_random, 16);
  tcsm_tc_bn_mod(*k, *k, group->n);
  
  if(tcsm_tc_bn_is_zero(*k))
  {
    ret = ERR_ILLEGAL_ARGUMENT;
    goto err;
  }

err:
  tcsm_unlock_temp_bn(sm2ctx, i_k);

  return ret;
}

int generatePrivateKey(sm2_ctx_t *ctx, char *out)
{
    int prik_i = 0;

    tc_bn_t* prik = tcsm_lock_temp_bn(ctx, &prik_i);
    tc_ec_group_t* group = (tc_ec_group_t*)ctx->group;
    tcsm_tc_rand_bignum(ctx->rand_ctx,*prik, (*group)->n);
    tcsm_private_key_get_str(out, *prik);
    tcsm_unlock_temp_bn(ctx, prik_i);
    return ERR_TENCENTSM_OK;
}

int generatePublicKey(sm2_ctx_t *ctx, const char *privateKey, char *outPubKey)
{
  int pubk_i = 0,prik_i = 0;
  
  tc_ec_group_t* group = (tc_ec_group_t*)ctx->group;
  
  if (privateKey != NULL && strlen(privateKey) != 64) {
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  tc_ec_t* pubk = tcsm_lock_temp_ec(ctx, &pubk_i);
  tc_bn_t* prik = tcsm_lock_temp_bn(ctx, &prik_i);
  
  tcsm_tc_bn_set_str(*prik, (char*)privateKey, 16);
  
  int ret = tcsm_ec_mul_for_G(*group, *pubk, *prik);
  if (ret == ERR_TENCENTSM_OK) {
    tcsm_public_key_get_str(outPubKey, *pubk);
  }else{
    LOGV("sm2_generate_public_key failed with errcode:%d",ret);
  }
  tcsm_unlock_temp_ec(ctx, pubk_i);
  tcsm_unlock_temp_bn(ctx, prik_i);
  return ret;
}

int generateKeyPair(sm2_ctx_t *ctx, char *outPriKey, char *outPubKey)
{
  int pubk_i = 0,prik_i = 0;
  
  tc_ec_t* pubk = tcsm_lock_temp_ec(ctx, &pubk_i);
  tc_bn_t* prik = tcsm_lock_temp_bn(ctx, &prik_i);
  
  int ret = tcsm_sm2_generate_key(*(tc_ec_group_t*)ctx->group, *pubk, *prik);
  if (ret == ERR_TENCENTSM_OK) {
    tcsm_private_key_get_str(outPriKey, *prik);
    tcsm_public_key_get_str(outPubKey, *pubk);
  }else{
    LOGV("tcsm_sm2_generate_key failed with errcode:%d",ret);
  }
  tcsm_unlock_temp_ec(ctx, pubk_i);
  tcsm_unlock_temp_bn(ctx, prik_i);

  return ret;
}

int SM2Encrypt(sm2_ctx_t *ctx, const unsigned char *in, size_t inlen, const char *strPubKey, size_t pubkeyLen, unsigned char *out, size_t *outlen)
{
  return SM2EncryptWithMode(ctx, in, inlen, strPubKey, pubkeyLen, out, outlen, SM2CipherMode_C1C3C2_ASN1);
}

int SM2EncryptWithMode(sm2_ctx_t *ctx, const unsigned char *in, size_t inlen, const char *strPubKey, size_t pubkeyLen, unsigned char *out, size_t *outlen,SM2CipherMode mode)
{
  int ret = ERR_TENCENTSM_OK;
  
  if (strlen(strPubKey) != SM2_PUBKEY_MAX_LEN) {
    LOGV("sm2 encrypt argument pubkey length error.\n", ret);
    ret = ERR_ILLEGAL_ARGUMENT;
    goto illegal;
  }
  
  if (inlen == 0) {
    LOGV("sm2 encrypt argument plain length error.\n", ret);
    ret = ERR_ILLEGAL_ARGUMENT;
    goto illegal;
  }
  
  int pub_i = 0;
  tc_ec_t* pubk = tcsm_lock_temp_ec(ctx, &pub_i);
  tcsm_public_key_set_str(strPubKey, *pubk);

  sm2_cipher *cipher = (sm2_cipher *)tcsm_tc_secure_malloc(sizeof(sm2_cipher)+inlen);
  
  if ((ret = tcsm_sm2_encrypt(ctx, *pubk, (unsigned char *)in, (unsigned int)inlen, cipher)) != ERR_TENCENTSM_OK)
  {
    LOGV("sm2 encrypt error ! ret = %d\n", ret);
    goto end;
  }
  
  switch (mode) {
    case SM2CipherMode_C1C3C2_ASN1:
      tc_asn1_encode_sm2_cipher_c1c3c2(cipher, out, outlen);
      break;
    case SM2CipherMode_C1C3C2:
      *outlen = tcsm_sm2_get_cipher_size(cipher) - 4;
      memcpy(out, cipher, TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE);
      memcpy(out + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE, cipher->Cipher, inlen);
      break;
    case SM2CipherMode_C1C2C3_ASN1:
      tc_asn1_encode_sm2_cipher_c1c2c3(cipher, out, outlen);
      break;
    case SM2CipherMode_C1C2C3:
      *outlen = tcsm_sm2_get_cipher_size(cipher) - 4;
      memcpy(out, cipher, TC_ECCref_MAX_LEN * 2);
      memcpy(out + TC_ECCref_MAX_LEN * 2, cipher->Cipher, inlen);
      memcpy(out + TC_ECCref_MAX_LEN * 2 + inlen, cipher->HASH, SM3_DIGEST_SIZE);
      break;
    case SM2CipherMode_04C1C3C2:
      *outlen = tcsm_sm2_get_cipher_size(cipher) - 4 + 1;
      memset(out, 0x04, 1);
      memcpy(out + 1, cipher, TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE);
      memcpy(out + 1 + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE, cipher->Cipher, inlen);
      break;
    case SM2CipherMode_04C1C2C3:
      *outlen = tcsm_sm2_get_cipher_size(cipher) - 4 + 1;
      memset(out, 0x04, 1);
      memcpy(out + 1, cipher, TC_ECCref_MAX_LEN * 2);
      memcpy(out + 1 + TC_ECCref_MAX_LEN * 2, cipher->Cipher, inlen);
      memcpy(out + 1 + TC_ECCref_MAX_LEN * 2 + inlen, cipher->HASH, SM3_DIGEST_SIZE);
      break;
    default:
      break;
  }
  
  end:
    tcsm_unlock_temp_ec(ctx, pub_i);
    tcsm_tc_secure_free(cipher);
  illegal:
    return ret;
}

int SM2Decrypt(sm2_ctx_t *ctx, const unsigned char *in, size_t inlen, const char *strPriKey, size_t prikeyLen, unsigned char *out, size_t *outlen)
{
  return SM2DecryptWithMode(ctx, in, inlen, strPriKey, prikeyLen, out, outlen, SM2CipherMode_C1C3C2_ASN1);
}

int SM2DecryptWithMode(sm2_ctx_t *ctx, const unsigned char *in, size_t inlen, const char *strPriKey, size_t prikeyLen,unsigned char *out, size_t *outlen,SM2CipherMode mode)
{
  int ret = ERR_TENCENTSM_OK;
  
  if (strlen(strPriKey) != SM2_PRIKEY_MAX_LEN) {
    LOGV("sm2 encrypt argument priKey length error.  %d \n", ret,strlen(strPriKey));
    ret = ERR_ILLEGAL_ARGUMENT;
    goto illegal;
  }
  
  if (inlen <= TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE) {
    LOGV("sm2 encrypt argument in length error.\n");
    ret = ERR_ILLEGAL_ARGUMENT;
    goto illegal;
  }
  
  int pri_i = 0;
  tc_bn_t* prik = tcsm_lock_temp_bn(ctx, &pri_i);
  tcsm_private_key_set_str(strPriKey, *prik);
  
  char * cipher = tcsm_tc_secure_malloc(inlen + sizeof(unsigned int));
  
  switch (mode) {
    case SM2CipherMode_C1C3C2_ASN1:
      ret = tc_asn1_decode_sm2_cipher_c1c3c2(in, (int)inlen, (unsigned char*)cipher);
      break;
    case SM2CipherMode_C1C3C2:
    {
      unsigned int ciphertext_len = (unsigned int)(inlen - (TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE));
      memcpy(cipher, in, TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE);
      memcpy(cipher + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE, (char*)&ciphertext_len, sizeof(unsigned int));
      memcpy(cipher + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE + sizeof(unsigned int), in + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE, ciphertext_len);
    }
      break;
    case SM2CipherMode_C1C2C3_ASN1:
      ret = tc_asn1_decode_sm2_cipher_c1c2c3(in, (int)inlen, (unsigned char*)cipher);
      break;
    case SM2CipherMode_C1C2C3:
    {
      unsigned int ciphertext_len = (unsigned int)(inlen - (TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE));
      memcpy(cipher, in, TC_ECCref_MAX_LEN * 2);
      memcpy(cipher + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE, (char*)&ciphertext_len, sizeof(unsigned int));
      memcpy(cipher + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE + sizeof(unsigned int), in + TC_ECCref_MAX_LEN * 2, ciphertext_len);
      memcpy(cipher + TC_ECCref_MAX_LEN * 2, in + TC_ECCref_MAX_LEN * 2 + ciphertext_len, SM3_DIGEST_SIZE);
    }
      break;
    case SM2CipherMode_04C1C3C2:
    {
      unsigned int ciphertext_len = (unsigned int)(inlen - (TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE) - 1);
      memcpy(cipher, in + 1, TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE);
      memcpy(cipher + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE, (char*)&ciphertext_len, sizeof(unsigned int));
      memcpy(cipher + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE + sizeof(unsigned int), in + 1 + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE, ciphertext_len);
    }
      break;
    case SM2CipherMode_04C1C2C3:
    {
      unsigned int ciphertext_len = (unsigned int)(inlen - (TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE) - 1);
      memcpy(cipher, in + 1, TC_ECCref_MAX_LEN * 2);
      memcpy(cipher + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE, (char*)&ciphertext_len, sizeof(unsigned int));
      memcpy(cipher + TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE + sizeof(unsigned int), in + 1 + TC_ECCref_MAX_LEN * 2, ciphertext_len);
      memcpy(cipher + TC_ECCref_MAX_LEN * 2, in + 1 + TC_ECCref_MAX_LEN * 2 + ciphertext_len, SM3_DIGEST_SIZE);
    }
      break;
    default:
      break;
  }
  
  if (ret != ERR_TENCENTSM_OK) {
    LOGV("sm2 decrypt error ! ret = %d\n", ret);
    goto end;
  }

  if ((ret = tcsm_sm2_decrypt(ctx, (sm2_cipher *)cipher, *prik, (unsigned char *)out, (unsigned int *)outlen)) < ERR_TENCENTSM_OK)
  {
    LOGV("sm2 decrypt error ! ret = %d\n", ret);
    goto end;
  }
  
end:
  tcsm_unlock_temp_bn(ctx, pri_i);
  tcsm_tc_secure_free(cipher);
illegal:
  return ret;
}

int SM2Sign(sm2_ctx_t *ctx, const unsigned char *msg, size_t msglen, const char *id, size_t idlen, const char *strPubKey, size_t pubkeyLen, const char *strPriKey, size_t prikeyLen,unsigned char *sig, size_t *siglen)
{
  return SM2SignWithMode(ctx, msg, msglen, id, idlen, strPubKey, pubkeyLen, strPriKey, prikeyLen, sig, siglen, SM2SignMode_RS_ASN1);
}

int SM2SignWithMode(sm2_ctx_t *ctx, const unsigned char *msg, size_t msglen, const char *id, size_t idlen, const char *strPubKey, size_t pubkeyLen, const char *strPriKey, size_t prikeyLen,unsigned char *sig, size_t *siglen,SM2SignMode mode)
{
  int ret = ERR_TENCENTSM_OK;
  size_t dgstlen;
  
  if (strlen(strPriKey) != SM2_PRIKEY_MAX_LEN) {
    LOGV("sm2 sign argument priKey length error.  %d %d %s \n", pubkeyLen,prikeyLen,strPriKey);
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  tc_ec_group_t* sm2group = (tc_ec_group_t*)ctx->group;
  
  int pub_i,pri_i = 0;
  tc_bn_t* prik = tcsm_lock_temp_bn(ctx, &pri_i);
  tc_ec_t* pubk = tcsm_lock_temp_ec(ctx, &pub_i);
  tcsm_private_key_set_str(strPriKey,*prik);
  tcsm_public_key_set_str(strPubKey, *pubk);
  
  unsigned char dgst[SM3_DIGEST_LENGTH];
  dgstlen = sizeof(dgst);
  
  if ((ret = tcsm_sm2_compute_message_digest(*sm2group, *pubk, SM_MD_SM3, SM_MD_SM3, msg, msglen, id, idlen, dgst, &dgstlen)) != ERR_TENCENTSM_OK)
  {
    LOGV("sm2 compute msg digest error ! ret = %d!\n", ret);
    goto end;
  }
  
  sm2_signature sign;
  if ((ret = tcsm_sm2_sign(ctx, *prik, (unsigned char*)dgst, (unsigned int)dgstlen, &sign)) != ERR_TENCENTSM_OK)
  {
    LOGV("sm2 sign error ! ret = %d!\n", ret);
    goto end;
  }
  
  switch (mode) {
    case SM2SignMode_RS_ASN1:
    {
      unsigned char r_encode[40];
      int r_encode_len = 0;
      tc_asn1_encode_integer(sign.r, TC_ECCref_MAX_LEN, r_encode, &r_encode_len);
      
      unsigned char s_encode[40];
      int s_encode_len = 0;
      tc_asn1_encode_integer(sign.s, TC_ECCref_MAX_LEN, s_encode, &s_encode_len);
      
      unsigned char rs_encode[80];
      memcpy(rs_encode, r_encode, r_encode_len);
      memcpy(rs_encode + r_encode_len , s_encode, s_encode_len);
      
      int encode_len = 0;
      tc_asn1_encode_sequence(rs_encode, r_encode_len + s_encode_len, sig, &encode_len);
      *siglen = encode_len;
    }
      break;
    case SM2SignMode_RS:
    {
      memcpy(sig, sign.r, TC_ECCref_MAX_LEN);
      memcpy(sig + TC_ECCref_MAX_LEN, sign.s, TC_ECCref_MAX_LEN);
      *siglen = 2*TC_ECCref_MAX_LEN;
    }
      break;
    default:
      break;
  }

end:
  tcsm_unlock_temp_ec(ctx, pub_i);
  tcsm_unlock_temp_bn(ctx, pri_i);
  return ret;
}

int SM2Verify(sm2_ctx_t *ctx, const unsigned char *msg, size_t msglen,const char *id, size_t idlen, const unsigned char *sig, size_t siglen, const char *strPubKey, size_t pubkeyLen)
{
  return SM2VerifyWithMode(ctx, msg, msglen, id, idlen, sig, siglen, strPubKey, pubkeyLen, SM2SignMode_RS_ASN1);
}

int SM2VerifyWithModeInner(sm2_ctx_t *ctx, const unsigned char *msg, size_t msglen,const char *id, size_t idlen, const unsigned char *sig, size_t siglen, const char *strPubKey, size_t pubkeyLen,SM2SignMode mode)
{
  int ret = ERR_TENCENTSM_OK;

  if (strlen(strPubKey) != SM2_PUBKEY_MAX_LEN) {
    LOGV("sm2 verify argument public length error.  %d \n", ret,strlen(strPubKey));
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  if (siglen < 9) {
    LOGV("sm2 verify argument sig length error.\n", ret);
  }
  
  sm2_signature sign;
  
  switch (mode) {
    case SM2SignMode_RS_ASN1:
    {
      int rs_offset = 0;
      int rs_outlen = 0;
      
      ret = tc_asn1_decode_object(sig, (int)siglen, &rs_offset, &rs_outlen);
      
      if (ret != ERR_TENCENTSM_OK) {
          LOGV("sm2 verify argument sig format error. \n", ret);
          return ERR_ASN1_DECODE_OBJ;
      }else{
          
          int r_offset = 0;
          int r_outlen = 0;
          
          ret = tc_asn1_decode_object(sig + rs_offset, rs_outlen, &r_offset, &r_outlen);
          
          if (ret != ERR_TENCENTSM_OK || r_outlen > 33) {
              LOGV("sm2 verify argument sig format error. \n", ret);
              return ERR_ASN1_FORMAT_ERROR;
          }
          
          if (r_outlen == 33) {
              memcpy(sign.r, sig + rs_offset + r_offset + 1, 32);
          }else if(r_outlen == 32){
              memcpy(sign.r, sig + rs_offset + r_offset, 32);
          }else{
            int z = 32 - r_outlen;
            memset(sign.r, 0x00, z);
            memcpy(sign.r + z, sig + rs_offset + r_offset, r_outlen);
          }
          
          int s_offset = 0;
          int s_outlen = 0;
          
          ret = tc_asn1_decode_object(sig + rs_offset + r_offset + r_outlen, rs_outlen - r_offset - r_outlen, &s_offset, &s_outlen);
          
          if (ret != ERR_TENCENTSM_OK || s_outlen > 33) {
              LOGV("sm2 verify argument sig format error. \n", ret);
              return ERR_ASN1_FORMAT_ERROR;
          }
          
          if (s_outlen == 33) {
              memcpy(sign.s, sig + rs_offset + r_offset + r_outlen + s_offset + 1, 32);
          }else if(s_outlen == 32){
              memcpy(sign.s, sig + rs_offset + r_offset + r_outlen + s_offset, 32);
          }else{
            int z = 32 - s_outlen;
            memset(sign.s, 0x00, z);
            memcpy(sign.s + z, sig + rs_offset + r_offset + r_outlen + s_offset, s_outlen);
          }

          {
            unsigned char plainbuf[TC_ECCref_MAX_LEN*2];
            char plainbufstr[TC_ECCref_MAX_LEN*4+1];
            memset(plainbuf, 0x00, TC_ECCref_MAX_LEN*2);
            memset(plainbufstr, 0x00, TC_ECCref_MAX_LEN*4+1);
            memcpy(plainbuf, sign.r, TC_ECCref_MAX_LEN);
            memcpy(plainbuf+TC_ECCref_MAX_LEN, sign.s, TC_ECCref_MAX_LEN);
            tcsm_bin2hex((const unsigned char*)plainbuf, TC_ECCref_MAX_LEN * 2, plainbufstr, TC_ECCref_MAX_LEN*4+1);
            // LOGV("-----signeddata decoded:\n");
            // LOGV("%s\n-----\n", plainbufstr);
          }
      }
    }
      break;
    case SM2SignMode_RS:
    {
      memcpy(sign.r, sig, TC_ECCref_MAX_LEN);
      memcpy(sign.s, sig + TC_ECCref_MAX_LEN, TC_ECCref_MAX_LEN);
    }
      break;
    default:
      break;
  }
  
  tc_ec_group_t* sm2group = (tc_ec_group_t*)ctx->group;
  
  int pub_i = 0;
  tc_ec_t* pubk = tcsm_lock_temp_ec(ctx, &pub_i);
  tcsm_public_key_set_str(strPubKey, *pubk);
  
  unsigned char dgst[SM3_DIGEST_LENGTH];
  size_t dgstlen = sizeof(dgst);
  
  if ((ret = tcsm_sm2_compute_message_digest(*sm2group, *pubk, SM_MD_SM3, SM_MD_SM3, msg, msglen, id, idlen, dgst, &dgstlen)) != ERR_TENCENTSM_OK)
  {
    LOGV("sm2 compute msg digest error ! ret = %d!\n", ret);
    goto end;
  }
  
  if ((ret = tcsm_sm2_verify(ctx, *pubk, &sign, dgst, (unsigned int)dgstlen)) != ERR_TENCENTSM_OK)
  {
    LOGV("sm2 verify error ! ret = %d!\n", ret);
    goto end;
  }
end:
  tcsm_unlock_temp_ec(ctx, pub_i);
  return ret;
}
int SM2VerifyWithMode(sm2_ctx_t *ctx, const unsigned char *msg, size_t msglen,const char *id, size_t idlen, const unsigned char *sig, size_t siglen, const char *strPubKey, size_t pubkeyLen,SM2SignMode mode)
{
  return SM2VerifyWithModeInner(ctx, msg, msglen, id, idlen, sig, siglen, strPubKey, pubkeyLen, mode);
}

int SM3CtxSize()
{
  return sizeof(sm3_ctx_t);
}

int SM3Init(sm3_ctx_t *ctx)
{
  tcsm_sm3_init_opt(ctx);
  return ERR_TENCENTSM_OK;
}

int SM3Update(sm3_ctx_t *ctx, const unsigned char* data, size_t data_len)
{
  tcsm_sm3_update_opt(ctx, (const unsigned char*)data, data_len);
  return ERR_TENCENTSM_OK;
}

int SM3Final(sm3_ctx_t *ctx, unsigned char *digest)
{
  tcsm_sm3_final_opt(ctx, (unsigned char *)digest);
  return ERR_TENCENTSM_OK;
}

int SM3(const unsigned char *data, size_t datalen, unsigned char *digest)
{
  tcsm_sm3opt(data, datalen, (unsigned char*)digest);
  return ERR_TENCENTSM_OK;
}

int SM3KDF(const unsigned char *share, size_t sharelen, unsigned char *outkey, size_t keylen)
{
  return tcsm_x9_63_kdf_sm3(share, sharelen, outkey, keylen);
}

int generateSM4Key(unsigned char *outKey)
{
  void* r_ctx = tcsm_tc_rand_init();
  tcsm_tc_rand_bytes(r_ctx,outKey,16);
  tcsm_tc_rand_clear(r_ctx);
  tcsm_tc_secure_free(r_ctx);
  return ERR_TENCENTSM_OK;
}

int SM4_ECB_Encrypt(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_encrypt_key(&keyctx, (const unsigned char*)key);
  return tcsm_sms4_ecb_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, &keyctx, 1,0);
}

int SM4_ECB_Decrypt(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  if (inlen%SMS4_BLOCK_SIZE != 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }

  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_decrypt_key(&keyctx, (const unsigned char*)key);
  return tcsm_sms4_ecb_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, &keyctx, 0,0);
}

int SM4_ECB_Encrypt_NoPadding(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  if (inlen%SMS4_BLOCK_SIZE != 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }
    
  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_encrypt_key(&keyctx, (const unsigned char*)key);
  return tcsm_sms4_ecb_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, &keyctx, 1,1);
}

int SM4_ECB_Decrypt_NoPadding(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  if (inlen%SMS4_BLOCK_SIZE != 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }

  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_decrypt_key(&keyctx, (const unsigned char*)key);
  return tcsm_sms4_ecb_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, &keyctx, 0,1);
}


int SM4_CBC_Encrypt(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key,const unsigned char *iv)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_encrypt_key(&keyctx, (const unsigned char*)key);
  
  // tcsm_sms4_cbc_encrypt will change the value of iv, so use a tmp memory.
  char *iv_tmp = (char*)tcsm_tc_secure_malloc(SMS4_IV_LENGTH * sizeof(char));
  memset(iv_tmp, 0, SMS4_IV_LENGTH);
  memcpy(iv_tmp, iv, SMS4_IV_LENGTH);
  
  int ret = tcsm_sms4_cbc_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, &keyctx, (unsigned char*)iv_tmp, 1,0);
  
  tcsm_tc_secure_free(iv_tmp);
  return ret;
}

int SM4_CBC_Decrypt(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key,const unsigned char *iv)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  if (inlen%SMS4_BLOCK_SIZE != 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }

  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_decrypt_key(&keyctx, (const unsigned char*)key);
  
  // tcsm_sms4_cbc_encrypt will change the value of iv, so use a tmp memory.
  char *iv_tmp = (char*)tcsm_tc_secure_malloc(SMS4_IV_LENGTH * sizeof(char));
  memset(iv_tmp, 0, SMS4_IV_LENGTH);
  memcpy(iv_tmp, iv, SMS4_IV_LENGTH);
  
  int ret = tcsm_sms4_cbc_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, &keyctx, (unsigned char*)iv_tmp, 0,0);
  
  tcsm_tc_secure_free(iv_tmp);
  return ret;
}

int SM4_CBC_Encrypt_NoPadding(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key, const unsigned char *iv)
{
    if (inlen <= 0) {
        *outlen = 0;
        return ERR_ILLEGAL_ARGUMENT;
    }
  
    if (inlen%SMS4_BLOCK_SIZE != 0)
    {
        *outlen = 0;
        return ERR_ILLEGAL_ARGUMENT;
    }
    
    tcsm_sms4_key_t keyctx;
    tcsm_sms4_set_encrypt_key(&keyctx, (const unsigned char*)key);
    
    // tcsm_sms4_cbc_encrypt will change the value of iv, so use a tmp memory.
    char *iv_tmp = (char*)tcsm_tc_secure_malloc(SMS4_IV_LENGTH * sizeof(char));
    memset(iv_tmp, 0, SMS4_IV_LENGTH);
    memcpy(iv_tmp, iv, SMS4_IV_LENGTH);
    
    int ret = tcsm_sms4_cbc_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, &keyctx, (unsigned char*)iv_tmp, 1,1);
    
    tcsm_tc_secure_free(iv_tmp);
    return ret;
}

int SM4_CBC_Decrypt_NoPadding(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key,const unsigned char *iv)
{
    if (inlen <= 0) {
        *outlen = 0;
        return ERR_ILLEGAL_ARGUMENT;
    }
  
    if (inlen%SMS4_BLOCK_SIZE != 0) {
        *outlen = 0;
        return ERR_ILLEGAL_ARGUMENT;
    }
    
    tcsm_sms4_key_t keyctx;
    tcsm_sms4_set_decrypt_key(&keyctx, (const unsigned char*)key);
    
    // tcsm_sms4_cbc_encrypt will change the value of iv, so use a tmp memory.
    char *iv_tmp = (char*)tcsm_tc_secure_malloc(SMS4_IV_LENGTH * sizeof(char));
    memset(iv_tmp, 0, SMS4_IV_LENGTH);
    memcpy(iv_tmp, iv, SMS4_IV_LENGTH);
    
    int ret = tcsm_sms4_cbc_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, &keyctx, (unsigned char*)iv_tmp, 0,1);
    
    tcsm_tc_secure_free(iv_tmp);
    return ret;
}

int SM4_CTR_Encrypt_NoPadding(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key, const unsigned char *iv)
{
    if (inlen <= 0) {
        *outlen = 0;
        return ERR_ILLEGAL_ARGUMENT;
    }
    
    tcsm_sms4_key_t keyctx;
    tcsm_sms4_set_encrypt_key(&keyctx, (const unsigned char*)key);
    return tcsm_sms4_ctr_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, &keyctx, iv);
}

int SM4_CTR_Decrypt_NoPadding(const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen, const unsigned char *key,const unsigned char *iv)
{
    return SM4_CTR_Encrypt_NoPadding(in, inlen, out, outlen, key, iv);
}

int SM4_GCM_Encrypt_NIST_SP800_38D(const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen, unsigned char *tag, size_t *taglen, const unsigned char *key, const unsigned char *iv,size_t ivlen,const unsigned char *aad, size_t aadlen)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }

  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_encrypt_key(&keyctx, (const unsigned char*)key);

  char *iv_tmp = (char*)tcsm_tc_secure_malloc(ivlen);
  memset(iv_tmp, 0, ivlen);
  memcpy(iv_tmp, iv, ivlen);

  int ret = tcsm_sms4_gcm_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, tag, taglen, &keyctx, (unsigned char*)iv_tmp, ivlen,aad, aadlen, 1, 0);

  tcsm_tc_secure_free(iv_tmp);
  
  return ret;
}

int SM4_GCM_Decrypt_NIST_SP800_38D(const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen,const unsigned char *tag, size_t taglen, const unsigned char *key, const unsigned char *iv,size_t ivlen,const unsigned char *aad, size_t aadlen)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }
  
  if (inlen%SMS4_BLOCK_SIZE != 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }

  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_encrypt_key(&keyctx, (const unsigned char*)key);

  char *iv_tmp = (char*)tcsm_tc_secure_malloc(ivlen);
  memset(iv_tmp, 0, ivlen);
  memcpy(iv_tmp, iv, ivlen);
  
  size_t taglen_tmp = taglen;

  int ret = tcsm_sms4_gcm_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, (unsigned char*)tag, &taglen_tmp, &keyctx, (unsigned char*)iv_tmp,ivlen, aad, aadlen, 0, 0);

  tcsm_tc_secure_free(iv_tmp);
  
  return ret;
}

int SM4_GCM_Encrypt_NoPadding_NIST_SP800_38D(const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen, unsigned char *tag, size_t *taglen, const unsigned char *key, const unsigned char *iv,size_t ivlen,const unsigned char *aad, size_t aadlen)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }

  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_encrypt_key(&keyctx, (const unsigned char*)key);

  char *iv_tmp = (char*)tcsm_tc_secure_malloc(ivlen);
  memset(iv_tmp, 0, ivlen);
  memcpy(iv_tmp, iv, ivlen);

  int ret = tcsm_sms4_gcm_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, tag, taglen, &keyctx, (unsigned char*)iv_tmp,ivlen, aad, aadlen, 1, 1);

  tcsm_tc_secure_free(iv_tmp);
  return ret;
}

int SM4_GCM_Decrypt_NoPadding_NIST_SP800_38D(const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen,const unsigned char *tag, size_t taglen, const unsigned char *key, const unsigned char *iv,size_t ivlen,const unsigned char *aad, size_t aadlen)
{
  if (inlen <= 0) {
    *outlen = 0;
    return ERR_ILLEGAL_ARGUMENT;
  }

  tcsm_sms4_key_t keyctx;
  tcsm_sms4_set_encrypt_key(&keyctx, (const unsigned char*)key);

  char *iv_tmp = (char*)tcsm_tc_secure_malloc(ivlen);
  memset(iv_tmp, 0, ivlen);
  memcpy(iv_tmp, iv, ivlen);
  
  size_t taglen_tmp = taglen;

  int ret = tcsm_sms4_gcm_encrypt((const unsigned char*)in, inlen, (unsigned char*)out, outlen, (unsigned char*)tag, &taglen_tmp, &keyctx, (unsigned char*)iv_tmp,ivlen, aad, aadlen, 0, 1);

  tcsm_tc_secure_free(iv_tmp);
  
  return ret;
}

int SM4_GCM_Encrypt(const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen, unsigned char *tag, size_t *taglen, const unsigned char *key, const unsigned char *iv,const unsigned char *aad, size_t aadlen)
{
  return SM4_GCM_Encrypt_NIST_SP800_38D(in, inlen, out, outlen, tag, taglen, key, iv, 8, aad, aadlen);
}

int SM4_GCM_Decrypt(const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen,const unsigned char *tag, size_t taglen, const unsigned char *key, const unsigned char *iv,const unsigned char *aad, size_t aadlen)
{
  return SM4_GCM_Decrypt_NIST_SP800_38D(in, inlen, out, outlen, tag, taglen, key, iv, 8, aad, aadlen);
}

int SM4_GCM_Encrypt_NoPadding(const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen, unsigned char *tag, size_t *taglen, const unsigned char *key, const unsigned char *iv,const unsigned char *aad, size_t aadlen)
{
  return SM4_GCM_Encrypt_NoPadding_NIST_SP800_38D(in, inlen, out, outlen, tag, taglen, key, iv, 8, aad, aadlen);
}

int SM4_GCM_Decrypt_NoPadding(const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen,const unsigned char *tag, size_t taglen, const unsigned char *key, const unsigned char *iv,const unsigned char *aad, size_t aadlen)
{
  return SM4_GCM_Decrypt_NoPadding_NIST_SP800_38D(in, inlen, out, outlen, tag, taglen, key, iv, 8, aad, aadlen);
}

//-----------------------SM3 hmac-----------------------------------  
/**
 * @brief 基于sm3算法计算HMAC值 ctx init
 * 
 * @param key HMAC用的秘钥
 * @param key_len 秘钥长度
 * 
 * @return 0 -- OK
 */
TstHmacSm3Ctx* SM3_HMAC_Init(const unsigned char *key, size_t key_len){
    return tcsm_sm3_hmac_init(key, key_len);
}
/**
 * @brief 基于sm3算法计算HMAC值 update数据
 * 
 * @param ctx hmac上下文结构指针
 * @param data 做HMAC计算的数据
 * @param data_len 数据长度
 * 
 * @return 0 -- OK
 */
int SM3_HMAC_Update(TstHmacSm3Ctx *ctx,const unsigned char *data, size_t data_len)
{
    return tcsm_sm3_hmac_update(ctx, data, data_len);
}
/**
 * @brief 基于sm3算法计算HMAC值 最终计算HMAC值
 * 
 * @param ctx hmac上下文结构指针
 * @param mac 输出的HMAC字节码
 * 
 * @return 0 -- OK
 */
int SM3_HMAC_Final(TstHmacSm3Ctx *ctx, unsigned char mac[SM3_HMAC_SIZE])
{
    return tcsm_sm3_hmac_final(ctx, mac);
}
/**
 * @brief 基于sm3算法计算HMAC值
 * 
 * @param data 做HMAC计算的数据
 * @param data_len 数据长度
 * @param key HMAC用的秘钥
 * @param key_len 秘钥长度
 * @param mac 输出的HMAC字节码
 * 
 * @return 0 -- OK
 */
int SM3_HMAC(const unsigned char *data, size_t data_len,const unsigned char *key, size_t key_len,unsigned char mac[SM3_HMAC_SIZE])
{
    return tcsm_sm3_hmac(data, data_len, key, key_len, mac);
}

/******************************************↓↓↓JNI 接口定义↓↓↓***********************************************************/
#ifdef JNI_INTERFACE
jstring Java_com_tenpay_utils_SMUtils_version(JNIEnv* env, jclass jclassObj) {
    const char* result = version();
    return (*env)->NewStringUTF(env, result);
}  

//SM2
jlong  Java_com_tenpay_utils_SMUtils_SM2InitCtx(JNIEnv* env, jobject jclassObj){
    void* pCtx = malloc(SM2CtxSize());
    SM2InitCtx(pCtx);
    return (intptr_t)pCtx;
}
jlong   Java_com_tenpay_utils_SMUtils_SM2InitCtxWithPubKey(JNIEnv* env, jobject jclassObj, jstring strPubKey){
    const char* chars = (*env)->GetStringUTFChars(env, strPubKey, NULL);
    void* pCtx = malloc(SM2CtxSize());
    SM2InitCtxWithPubKey(pCtx, chars);
    (*env)->ReleaseStringUTFChars(env, strPubKey, chars);
    return (intptr_t)pCtx;
}
void    Java_com_tenpay_utils_SMUtils_SM2FreeCtx(JNIEnv* env, jobject jclassObj, jlong sm2Handler){
    if(sm2Handler > 0) {
        void* pSm2Ctx = (void *)(intptr_t)sm2Handler;
        SM2FreeCtx(pSm2Ctx);
        free(pSm2Ctx);
    }
}
jobjectArray    Java_com_tenpay_utils_SMUtils_SM2GenKeyPair(JNIEnv* env, jobject jclassObj, jlong sm2Handler){
    char pPrikey[SM2_PRIVATE_KEY_LENGTH];
    char pPubkey[SM2_PUBLICK_KEY_LENGTH]; 
    memset(pPubkey, 0x00, SM2_PUBLICK_KEY_LENGTH);
    memset(pPrikey, 0x00, SM2_PRIVATE_KEY_LENGTH);
    sm2_ctx_t* pCtx = (void*)(intptr_t)sm2Handler;
    int result = generateKeyPair(pCtx, pPrikey, pPubkey);
    if(result != 0) {
        return NULL;
    } else {
        jobjectArray result;
        jclass stringarrCls = (*env)->FindClass(env, "java/lang/String");
        result = (*env)->NewObjectArray(env, 2, stringarrCls, NULL);
        (*env)->SetObjectArrayElement(env, result, 0, (*env)->NewStringUTF(env, pPrikey));
        (*env)->SetObjectArrayElement(env, result, 1, (*env)->NewStringUTF(env, pPubkey));
        return result;
    }
}
jbyteArray    Java_com_tenpay_utils_SMUtils_SM2Encrypt(JNIEnv* env, jobject jclassObj, jlong sm2Handler, jbyteArray in, 
                          jstring strPubKey){
    sm2_ctx_t* pCtx = (void*)(intptr_t)sm2Handler;
    int size = (*env)->GetArrayLength(env, in);
    size_t cipherlen = size+200;
    char charCipher[cipherlen]; 
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    const char* charPubkey;
    int keylen = 0;
    if(strPubKey == NULL) {
        charPubkey = NULL;
    } else {
        charPubkey = (*env)->GetStringUTFChars(env, strPubKey, NULL);
        keylen = (*env)->GetStringLength(env, strPubKey);
    }
    int result = SM2Encrypt(pCtx, indata, size, charPubkey, keylen, charCipher, &cipherlen);
    LOGV("Java_com_tenpay_utils_SMUtils_SM2Encrypt 001 ");
    if(result != 0) {
        return NULL;
    } else {
        jbyteArray jarrRV =(*env)->NewByteArray(env,cipherlen);
        (*env)->SetByteArrayRegion(env,jarrRV, 0,cipherlen, charCipher);
        return jarrRV;
    }
}

jbyteArray    Java_com_tenpay_utils_SMUtils_SM2Decrypt(JNIEnv* env, jobject jclassObj, jlong sm2Handler, jbyteArray in, jstring strPriKey) {
    sm2_ctx_t* pCtx = (void*)(intptr_t)sm2Handler;
    int size = (*env)->GetArrayLength(env, in);
    size_t txtlen = size;
    char charPlaintxt[txtlen]; 
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    const char* charPrikey = (*env)->GetStringUTFChars(env, strPriKey, NULL);
    int result = SM2Decrypt(pCtx, indata, size, charPrikey, (*env)->GetStringLength(env, strPriKey), charPlaintxt, &txtlen);
    if(result != 0) {
        return NULL;
    } else {
        jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
        (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charPlaintxt);
        return jarrRV;
    }
}

jbyteArray Java_com_tenpay_utils_SMUtils_SM2Sign(JNIEnv* env, jobject jclassObj, jlong sm2Handler, jbyteArray msg, 
                          jbyteArray id, jstring strPubKey, jstring strPriKey){
    LOGV("Java_com_tenpay_utils_SMUtils_SM2Sign 000");
    sm2_ctx_t* pCtx = (void*)(intptr_t)sm2Handler;
    int size = (*env)->GetArrayLength(env, msg);
    jbyte* indata = (*env)->GetByteArrayElements(env, msg, JNI_FALSE);
    const char* charPubkey = (*env)->GetStringUTFChars(env, strPubKey, NULL);
    const char* charPrikey = (*env)->GetStringUTFChars(env, strPriKey, NULL);
    jbyte* charId = (*env)->GetByteArrayElements(env, id, JNI_FALSE);
    int id_len = (*env)->GetArrayLength(env, id);
    
    size_t txtlen = SM2_SIGN_LENGTH;
    char charSignedtxt[txtlen]; 
    int result = SM2Sign(pCtx, indata, size, charId, id_len,
            charPubkey, (*env)->GetStringLength(env, strPubKey),
            charPrikey, (*env)->GetStringLength(env, strPriKey), charSignedtxt, &txtlen);
    if(result != 0) {
        return NULL;
    } else {
        jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
        (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charSignedtxt);
        return jarrRV;
    }
}
jint    Java_com_tenpay_utils_SMUtils_SM2Verify(JNIEnv* env, jobject jclassObj, jlong sm2Handler, jbyteArray msg, 
                          jbyteArray id, jstring strPubKey, jbyteArray sig){
    LOGV("Java_com_tenpay_utils_SMUtils_SM2Verify 000");
    sm2_ctx_t* pCtx = (void*)(intptr_t)sm2Handler;
    int size = (*env)->GetArrayLength(env, msg);
    jbyte* indata = (*env)->GetByteArrayElements(env, msg, JNI_FALSE);
    int sigsize = (*env)->GetArrayLength(env, sig);
    jbyte* signData = (*env)->GetByteArrayElements(env, sig, JNI_FALSE);
    const char* charPubkey = (*env)->GetStringUTFChars(env, strPubKey, NULL);
    jbyte* charId = (*env)->GetByteArrayElements(env, id, JNI_FALSE);
    int id_len = (*env)->GetArrayLength(env, id);
    return SM2Verify(pCtx, indata, size, charId, id_len,
            signData, sigsize, charPubkey, (*env)->GetStringLength(env, strPubKey));
}

//SM3
jlong   Java_com_tenpay_utils_SMUtils_SM3Init(JNIEnv* env, jobject jclassObj){
    void* pSM3Ctx = malloc(SM3CtxSize());
    SM3Init(pSM3Ctx);
    return (intptr_t)pSM3Ctx;
}
void    Java_com_tenpay_utils_SMUtils_SM3Update(JNIEnv* env, jobject jclassObj, jlong sm3Handler, jbyteArray data){
    sm3_ctx_t* pCtx = (void*)(intptr_t)sm3Handler;
    int size = (*env)->GetArrayLength(env, data);
    jbyte* indata = (*env)->GetByteArrayElements(env, data, JNI_FALSE);
    SM3Update(pCtx, indata, size);
}
jbyteArray Java_com_tenpay_utils_SMUtils_SM3Final(JNIEnv* env, jobject jclassObj, jlong sm3Handler){
    sm3_ctx_t* pCtx = (void*)(intptr_t)sm3Handler;
    size_t txtlen = SM3_DIGEST_LENGTH;
    char charHash[txtlen]; 
    SM3Final(pCtx, charHash);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charHash);
    return jarrRV;
}
void   Java_com_tenpay_utils_SMUtils_SM3Free(JNIEnv* env, jobject jclassObj, jlong sm3Handler){
    LOGV("SM3FREE handler : %ld", sm3Handler); 
    if(sm3Handler > 0) {
      sm3_ctx_t* pCtx = (void*)(intptr_t)sm3Handler;
      free(pCtx);
    }
}

jbyteArray    Java_com_tenpay_utils_SMUtils_SM3(JNIEnv* env, jobject jclassObj, jbyteArray data){
    int size = (*env)->GetArrayLength(env, data);
    jbyte* indata = (*env)->GetByteArrayElements(env, data, JNI_FALSE);
    size_t txtlen = SM3_DIGEST_LENGTH;
    char charHash[txtlen]; 
    SM3(indata, size, charHash);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charHash);
    return jarrRV;
}

/**
 * @brief 基于sm3算法计算HMAC值
 * @param data 做HMAC计算的数据
 * @param key HMAC用的秘钥
 * @param mac 输出的HMAC字节码
 */
jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM3HMAC(JNIEnv* env , jobject jclassObj, jbyteArray data, jbyteArray key) {
    int data_len = (*env)->GetArrayLength(env, data);
    jbyte* indata = (*env)->GetByteArrayElements(env, data, JNI_FALSE);
    int key_len = (*env)->GetArrayLength(env, key);
    jbyte* inkey = (*env)->GetByteArrayElements(env, key, JNI_FALSE);
    
    size_t txtlen = SM3_HMAC_SIZE;
    char charHash[txtlen]; 
    int ret = SM3_HMAC(indata, data_len, inkey, key_len, charHash);
    if(ret) {
        LOGV("SM3_HMAC err=%d", ret);
        return NULL;
    }
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charHash);
    return jarrRV;
}

//SM4
jbyteArray    Java_com_tenpay_utils_SMUtils_SM4GenKey(JNIEnv* env, jobject jclassObj){
    size_t txtlen = SM4_KEYBYTE_LENGTH;
    char charKey[txtlen]; 
    generateSM4Key(charKey);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charKey);
    return jarrRV;
}
jbyteArray    Java_com_tenpay_utils_SMUtils_SM4CBCEncrypt(JNIEnv* env, jobject jclassObj, jbyteArray in, jbyteArray key, jbyteArray iv){
    int size = (*env)->GetArrayLength(env, in);
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    int keysize = (*env)->GetArrayLength(env, key);
    jbyte* keydata = (*env)->GetByteArrayElements(env, key, JNI_FALSE);
    int ivsize = (*env)->GetArrayLength(env, iv);
    jbyte* ivdata = (*env)->GetByteArrayElements(env, iv, JNI_FALSE);
    //iv如果不够16字节补全
    unsigned char ivchars[SMS4_IV_LENGTH];
    memset(ivchars, 0x00, SMS4_IV_LENGTH);
    if(ivsize < SMS4_IV_LENGTH) {
        memcpy(ivchars, ivdata, ivsize);
    } else {
        memcpy(ivchars, ivdata, SMS4_IV_LENGTH);
    }
    size_t txtlen = (size/16+1)*16;
    char charCipher[txtlen]; 
    SM4_CBC_Encrypt(indata, size, charCipher, &txtlen, keydata, ivchars);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charCipher);
    return jarrRV;
}
jbyteArray    Java_com_tenpay_utils_SMUtils_SM4CBCDecrypt(JNIEnv* env, jobject jclassObj, jbyteArray in, jbyteArray key, jbyteArray iv){
    int size = (*env)->GetArrayLength(env, in);
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    int keysize = (*env)->GetArrayLength(env, key);
    jbyte* keydata = (*env)->GetByteArrayElements(env, key, JNI_FALSE);
    int ivsize = (*env)->GetArrayLength(env, iv);
    jbyte* ivdata = (*env)->GetByteArrayElements(env, iv, JNI_FALSE);
    //iv如果不够16字节补全
    unsigned char ivchars[SMS4_IV_LENGTH];
    memset(ivchars, 0x00, SMS4_IV_LENGTH);
    if(ivsize < SMS4_IV_LENGTH) {
        memcpy(ivchars, ivdata, ivsize);
    } else {
        memcpy(ivchars, ivdata, SMS4_IV_LENGTH);
    }
    size_t txtlen = size;
    char charPlain[txtlen]; 
    SM4_CBC_Decrypt(indata, size, charPlain, &txtlen, keydata, ivchars);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charPlain);
    return jarrRV;
}
jbyteArray    Java_com_tenpay_utils_SMUtils_SM4CBCEncryptNoPadding(JNIEnv* env, jobject jclassObj, jbyteArray in, jbyteArray key, jbyteArray iv){
    int size = (*env)->GetArrayLength(env, in);
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    int keysize = (*env)->GetArrayLength(env, key);
    jbyte* keydata = (*env)->GetByteArrayElements(env, key, JNI_FALSE);
    int ivsize = (*env)->GetArrayLength(env, iv);
    jbyte* ivdata = (*env)->GetByteArrayElements(env, iv, JNI_FALSE);
    //iv如果不够16字节补全
    unsigned char ivchars[SMS4_IV_LENGTH];
    memset(ivchars, 0x00, SMS4_IV_LENGTH);
    if(ivsize < SMS4_IV_LENGTH) {
        memcpy(ivchars, ivdata, ivsize);
    } else {
        memcpy(ivchars, ivdata, SMS4_IV_LENGTH);
    }
    size_t txtlen = (size/16+1)*16;
    char charCipher[txtlen]; 
    SM4_CBC_Encrypt_NoPadding(indata, size, charCipher, &txtlen, keydata, ivchars);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charCipher);
    return jarrRV;
}
jbyteArray    Java_com_tenpay_utils_SMUtils_SM4CBCDecryptNoPadding(JNIEnv* env, jobject jclassObj, jbyteArray in, jbyteArray key, jbyteArray iv){
    int size = (*env)->GetArrayLength(env, in);
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    int keysize = (*env)->GetArrayLength(env, key);
    jbyte* keydata = (*env)->GetByteArrayElements(env, key, JNI_FALSE);
    int ivsize = (*env)->GetArrayLength(env, iv);
    jbyte* ivdata = (*env)->GetByteArrayElements(env, iv, JNI_FALSE);
    //iv如果不够16字节补全
    unsigned char ivchars[SMS4_IV_LENGTH];
    memset(ivchars, 0x00, SMS4_IV_LENGTH);
    if(ivsize < SMS4_IV_LENGTH) {
        memcpy(ivchars, ivdata, ivsize);
    } else {
        memcpy(ivchars, ivdata, SMS4_IV_LENGTH);
    }
    size_t txtlen = size;
    char charPlain[txtlen]; 
    SM4_CBC_Decrypt_NoPadding(indata, size, charPlain, &txtlen, keydata, ivchars);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charPlain);
    return jarrRV;
}
jbyteArray    Java_com_tenpay_utils_SMUtils_SM4ECBEncrypt(JNIEnv* env, jobject jclassObj, jbyteArray in, jbyteArray key){
    int size = (*env)->GetArrayLength(env, in);
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    int keysize = (*env)->GetArrayLength(env, key);
    jbyte* keydata = (*env)->GetByteArrayElements(env, key, JNI_FALSE);
    size_t txtlen = (size/16+1)*16;
    char charCipher[txtlen]; 
    SM4_ECB_Encrypt(indata, size, charCipher, &txtlen, keydata);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charCipher);
    return jarrRV;
}
jbyteArray    Java_com_tenpay_utils_SMUtils_SM4ECBDecrypt(JNIEnv* env, jobject jclassObj, jbyteArray in, jbyteArray key){
    int size = (*env)->GetArrayLength(env, in);
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    int keysize = (*env)->GetArrayLength(env, key);
    jbyte* keydata = (*env)->GetByteArrayElements(env, key, JNI_FALSE);
    size_t txtlen = size;
    char charPlain[txtlen]; 
    SM4_ECB_Decrypt(indata, size, charPlain, &txtlen, keydata);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charPlain);
    return jarrRV;
}
jbyteArray    Java_com_tenpay_utils_SMUtils_SM4ECBEncryptNoPadding(JNIEnv* env, jobject jclassObj, jbyteArray in, jbyteArray key){
    int size = (*env)->GetArrayLength(env, in);
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    int keysize = (*env)->GetArrayLength(env, key);
    jbyte* keydata = (*env)->GetByteArrayElements(env, key, JNI_FALSE);
    size_t txtlen = (size/16+1)*16;
    char charCipher[txtlen]; 
    SM4_ECB_Encrypt_NoPadding(indata, size, charCipher, &txtlen, keydata);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charCipher);
    return jarrRV;
}
jbyteArray    Java_com_tenpay_utils_SMUtils_SM4ECBDecryptNoPadding(JNIEnv* env, jobject jclassObj, jbyteArray in, jbyteArray key){
    int size = (*env)->GetArrayLength(env, in);
    jbyte* indata = (*env)->GetByteArrayElements(env, in, JNI_FALSE);
    int keysize = (*env)->GetArrayLength(env, key);
    jbyte* keydata = (*env)->GetByteArrayElements(env, key, JNI_FALSE);
    size_t txtlen = size;
    char charPlain[txtlen]; 
    SM4_ECB_Decrypt_NoPadding(indata, size, charPlain, &txtlen, keydata);
    jbyteArray jarrRV =(*env)->NewByteArray(env,txtlen);
    (*env)->SetByteArrayRegion(env,jarrRV, 0,txtlen, charPlain);
    return jarrRV;
}

#endif //JNI_INTERFACE



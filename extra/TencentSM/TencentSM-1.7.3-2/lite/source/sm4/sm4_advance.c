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
#include "../include/sm.h"
#include "../include/tc.h"
#include "../include/tc_err.h"
#include "../include/tc_sm4.h"
#include "../include/tc_gcm128_mode.h"
#include "../include/sm4_advance.h"

//-----------------ECB 加密-----------------
int SM4_ECB_Encrypt_Init(tcsm_sm4_ecb_t *ctx, const unsigned char *key, int no_padding)
{
  ctx->rk = (tcsm_sms4_key_t*)tcsm_tc_secure_malloc(sizeof(tcsm_sms4_key_t));
  if(NULL == ctx->rk)
  {
      return ERR_TC_MALLOC;
  }
  memset(ctx->remain_c, 0x00, 16);
  if((1 == no_padding) || (0 == no_padding)) {
    ctx->no_padding = no_padding;
  } else {
    tcsm_tc_secure_free(ctx->rk);
    return ERR_ILLEGAL_ARGUMENT;
  }
  ctx->mlen = 0;
  ctx->mres = 0;
  tcsm_sms4_set_encrypt_key((tcsm_sms4_key_t *)ctx->rk, (const unsigned char*)key);
  return ERR_TENCENTSM_OK;
}

int SM4_ECB_Encrypt_Update(tcsm_sm4_ecb_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
  size_t cipherlen = 0;
  u64 mlen = ctx->mlen;
  mlen += inlen;
  if ((0 == inlen) || (mlen > ((U64(1) << 36) - 32)) || ((sizeof(inlen) == 8) && (mlen < inlen))) {
    *outlen = 0;
    tcsm_tc_secure_free(ctx->rk);
    return ERR_ILLEGAL_ARGUMENT;
  }
  ctx->mlen = mlen;
    
  unsigned int n = ctx->mres;
  if (n) {
    while (n && inlen) {
      ctx->remain_c[n] = *(in++);
      --inlen;
      n = (n + 1) % 16;
    }
    if (n == 0) {
      tcsm_sms4_encrypt(ctx->remain_c, out, ctx->rk);
      out += 16;
      cipherlen = 16;
    } else {
      ctx->mres = n;
      *outlen = 0;
      return ERR_TENCENTSM_OK;
    }
  }
  while (inlen >= 16) {
    tcsm_sms4_encrypt(in, out, ctx->rk);
    out += 16;
    in += 16;
    inlen -= 16;
    cipherlen += 16;
  }
  if (inlen) {
    while (inlen--) {
      ctx->remain_c[n] = *(in++);
      ++n;
    }
  }
  ctx->mres = n;
  *outlen = cipherlen;
  return ERR_TENCENTSM_OK;
}

int SM4_ECB_Encrypt_Final(tcsm_sm4_ecb_t *ctx, unsigned char *out, size_t *outlen)
{
  int ret = ERR_TENCENTSM_OK;
  if(1 == ctx->no_padding) {
    *outlen = 0;
    if (ctx->mres) {
      ret = ERR_SM4_ECB_ILLEGAL_MSGLEN;
    }
  } else {
    *outlen = 16;
    size_t padding_len = SMS4_BLOCK_SIZE - ctx->mres;
    memset(ctx->remain_c + ctx->mres, (int)(padding_len), padding_len);
    tcsm_sms4_encrypt(ctx->remain_c, out, ctx->rk);
  }

  tcsm_tc_secure_free(ctx->rk);
  memset(ctx->remain_c, 0x00, 16);
  ctx->mlen = 0;
  ctx->mres = 0;
  return ret;
}

//-----------------ECB 解密-----------------
int SM4_ECB_Decrypt_Init(tcsm_sm4_ecb_t *ctx, const unsigned char *key, int no_padding)
{
  ctx->rk = (tcsm_sms4_key_t*)tcsm_tc_secure_malloc(sizeof(tcsm_sms4_key_t));
  if(NULL == ctx->rk)
  {
      return ERR_TC_MALLOC;
  }
  memset(ctx->remain_c, 0x00, 16);
  memset(ctx->cipher_buf, 0x00, 16);
  if((1 == no_padding) || (0 == no_padding)) {
    ctx->no_padding = no_padding;
  } else {
    tcsm_tc_secure_free(ctx->rk);
    return ERR_ILLEGAL_ARGUMENT;
  }
  ctx->mlen = 0;
  ctx->mres = 0;
  ctx->is_one_block_cached = 0;
  tcsm_sms4_set_decrypt_key((tcsm_sms4_key_t *)ctx->rk, (const unsigned char*)key);
  return ERR_TENCENTSM_OK;
}

int SM4_ECB_Decrypt_Update(tcsm_sm4_ecb_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
  size_t textlen = 0;
    
  u64 mlen = ctx->mlen;
  mlen += inlen;
  if ((0 == inlen) || (mlen > ((U64(1) << 36) - 32)) || ((sizeof(inlen) == 8) && (mlen < inlen))) {
    *outlen = textlen;
    tcsm_tc_secure_free(ctx->rk);
    return ERR_ILLEGAL_ARGUMENT;
  }
  ctx->mlen = mlen;

  if(1 == ctx->is_one_block_cached) {
    memcpy(out, ctx->cipher_buf, 16);
    out += 16;
    textlen = 16;
    ctx->is_one_block_cached = 0;
  }
    
  unsigned int n = ctx->mres;
  if (n) {
    while (n && inlen) {
      ctx->remain_c[n] = *(in++);
      --inlen;
      n = (n + 1) % 16;
    }
    if (n != 0) {
      ctx->mres = n;
      *outlen = textlen;
      return ERR_TENCENTSM_OK;
    } else {
      if((1 == ctx->no_padding) || (inlen > 0)) {
        tcsm_sms4_encrypt(ctx->remain_c, out, ctx->rk);
        out += 16;
        textlen += 16;
      } else {
        tcsm_sms4_encrypt(ctx->remain_c, ctx->cipher_buf, ctx->rk);
        ctx->mres = n;
        *outlen = textlen;
        ctx->is_one_block_cached = 1;
        return ERR_TENCENTSM_OK;
      }
    }
  }

  while (inlen > 16) {
    tcsm_sms4_encrypt(in, out, ctx->rk);
    out += 16;
    in += 16;
    inlen -= 16;
    textlen += 16;
  }
  if (16 == inlen) {
    if (1 == ctx->no_padding) {
      tcsm_sms4_encrypt(in, out, ctx->rk);
      textlen += 16;
    } else {
      tcsm_sms4_encrypt(in, ctx->cipher_buf, ctx->rk);
      ctx->is_one_block_cached = 1;
    }
  } else {
    while (inlen--) {
      ctx->remain_c[n] = *(in++);
      ++n;
    }
  }
    
  ctx->mres = n;
  *outlen = textlen;
  return ERR_TENCENTSM_OK;
}
 
int SM4_ECB_Decrypt_Final(tcsm_sm4_ecb_t *ctx, unsigned char *out, size_t *outlen)
{
  int ret = ERR_TENCENTSM_OK;
  
  if (ctx->mres) {
    ret = ERR_SM4_ECB_ILLEGAL_MSGLEN;
  }
  if(1 == ctx->no_padding) {
    *outlen = 0;
  } else {
    size_t pad_value_check = 0;
    size_t padding_len = 0;
    if(0 == ctx->is_one_block_cached) {
      pad_value_check = 1;
    } else {
      padding_len = ctx->cipher_buf[15];
      /* test for valid padding value */
      if((1 > padding_len) || (SMS4_BLOCK_SIZE < padding_len)) {
        pad_value_check = 1;
      }else{
        size_t i = 16 - padding_len;
        for(; i  < 16 ; i++){
          if(ctx->cipher_buf[i] != padding_len){
            pad_value_check = 1;
            break;
          }
        }
      }
    }
    if(1 == pad_value_check) {
      ret = ERR_SM4_PKCS7_PADDING_VERIFY_FAILED;
    }else{
      *outlen = 16 - padding_len;
      memcpy(out, ctx->cipher_buf, *outlen);
    }
  }
  tcsm_tc_secure_free(ctx->rk);
  memset(ctx->remain_c, 0x00, 16);
  memset(ctx->cipher_buf, 0x00, 16);
  ctx->mlen = 0;
  ctx->mres = 0;
  return ret;
}

//-----------------CBC 加密-----------------
int SM4_CBC_Encrypt_Init(tcsm_sm4_cbc_t *ctx, const unsigned char *key, const unsigned char *iv, int no_padding)
{
  ctx->rk = (tcsm_sms4_key_t*)tcsm_tc_secure_malloc(sizeof(tcsm_sms4_key_t));
  if(NULL == ctx->rk)
  {
      return ERR_TC_MALLOC;
  }
  memcpy(ctx->iv, iv, 16);
  memset(ctx->remain_c, 0x00, 16);
  if((1 == no_padding) || (0 == no_padding)) {
    ctx->no_padding = no_padding;
  } else {
    tcsm_tc_secure_free(ctx->rk);
    return ERR_ILLEGAL_ARGUMENT;
  }
  ctx->mlen = 0;
  ctx->mres = 0;
  tcsm_sms4_set_encrypt_key((tcsm_sms4_key_t *)ctx->rk, (const unsigned char*)key);
  return ERR_TENCENTSM_OK;
}

int SM4_CBC_Encrypt_Update(tcsm_sm4_cbc_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
  size_t cipherlen = 0;
  u64 mlen = ctx->mlen;
  mlen += inlen;
  if ((0 == inlen) || (mlen > ((U64(1) << 36) - 32)) || ((sizeof(inlen) == 8) && (mlen < inlen))) {
    *outlen = 0;
    tcsm_tc_secure_free(ctx->rk);
    return ERR_ILLEGAL_ARGUMENT;
  }
  ctx->mlen = mlen;
    
  unsigned int n = ctx->mres;
  if (n) {
    while (n && inlen) {
      ctx->remain_c[n] = *(in++);
      --inlen;
      n = (n + 1) % 16;
    }
    if (n == 0) {
      for (size_t i = 0; i < 16; i += sizeof(size_t))
        *(size_t *)(out + i) = *(size_t *)(ctx->remain_c + i) ^ *(size_t *)(ctx->iv + i);
      tcsm_sms4_encrypt(out, out, ctx->rk);
      memcpy(ctx->iv, out, 16);
      out += 16;
      cipherlen = 16;
    } else {
      ctx->mres = n;
      *outlen = 0;
      return ERR_TENCENTSM_OK;
    }
  }
  while (inlen >= 16) {
    for (size_t i = 0; i < 16; i += sizeof(size_t))
      *(size_t *)(out + i) = *(size_t *)(in + i) ^ *(size_t *)(ctx->iv + i);
    tcsm_sms4_encrypt(out, out, ctx->rk);
    memcpy(ctx->iv, out, 16);
    out += 16;
    in += 16;
    inlen -= 16;
    cipherlen += 16;
  }
  if (inlen) {
    while (inlen--) {
      ctx->remain_c[n] = *(in++);
      ++n;
    }
  }
  ctx->mres = n;
  *outlen = cipherlen;
  return ERR_TENCENTSM_OK;
}

int SM4_CBC_Encrypt_Final(tcsm_sm4_cbc_t *ctx, unsigned char *out, size_t *outlen)
{
  int ret = ERR_TENCENTSM_OK;
  if(1 == ctx->no_padding) {
    *outlen = 0;
    if (ctx->mres) {
      ret = ERR_SM4_ECB_ILLEGAL_MSGLEN;
    }
  } else {
    *outlen = 16;
    size_t padding_len = SMS4_BLOCK_SIZE - ctx->mres;
    memset(ctx->remain_c + ctx->mres, (int)(padding_len), padding_len);
    for (size_t i = 0; i < 16; i += sizeof(size_t))
      *(size_t *)(out + i) = *(size_t *)(ctx->remain_c + i) ^ *(size_t *)(ctx->iv + i);
    tcsm_sms4_encrypt(out, out, ctx->rk);
  }

  tcsm_tc_secure_free(ctx->rk);
  memset(ctx->remain_c, 0x00, 16);
  memset(ctx->iv, 0x00, 16);
  ctx->mlen = 0;
  ctx->mres = 0;
  return ret;
}

//-----------------CBC 解密-----------------
int SM4_CBC_Decrypt_Init(tcsm_sm4_cbc_t *ctx, const unsigned char *key, const unsigned char *iv, int no_padding)
{
  ctx->rk = (tcsm_sms4_key_t*)tcsm_tc_secure_malloc(sizeof(tcsm_sms4_key_t));
  if(NULL == ctx->rk)
  {
      return ERR_TC_MALLOC;
  }
  memcpy(ctx->iv, iv, 16);
  memset(ctx->remain_c, 0x00, 16);
  if((1 == no_padding) || (0 == no_padding)) {
    ctx->no_padding = no_padding;
  } else {
    tcsm_tc_secure_free(ctx->rk);
    return ERR_ILLEGAL_ARGUMENT;
  }
  ctx->mlen = 0;
  ctx->mres = 0;
  ctx->is_one_block_cached = 0;
  tcsm_sms4_set_decrypt_key((tcsm_sms4_key_t *)ctx->rk, (const unsigned char*)key);
  return ERR_TENCENTSM_OK;
}

int SM4_CBC_Decrypt_Update(tcsm_sm4_cbc_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
  size_t textlen = 0;
    
  u64 mlen = ctx->mlen;
  mlen += inlen;
  if ((0 == inlen) || (mlen > ((U64(1) << 36) - 32)) || ((sizeof(inlen) == 8) && (mlen < inlen))) {
    *outlen = textlen;
    tcsm_tc_secure_free(ctx->rk);
    return ERR_ILLEGAL_ARGUMENT;
  }
  ctx->mlen = mlen;

  if(1 == ctx->is_one_block_cached) {
    memcpy(out, ctx->cipher_buf, 16);
    out += 16;
    textlen = 16;
    ctx->is_one_block_cached = 0;
  }
    
  unsigned int n = ctx->mres;
  if (n) {
    while (n && inlen) {
      ctx->remain_c[n] = *(in++);
      --inlen;
      n = (n + 1) % 16;
    }
    if (n != 0) {
      ctx->mres = n;
      *outlen = textlen;
      return ERR_TENCENTSM_OK;
    } else {
      if((1 == ctx->no_padding) || (inlen > 0)) {
        tcsm_sms4_encrypt(ctx->remain_c, out, ctx->rk);
        for (size_t i = 0; i < 16; i += sizeof(size_t))
          *(size_t *)(out + i) = *(size_t *)(out + i) ^ *(size_t *)(ctx->iv + i);
        memcpy(ctx->iv, ctx->remain_c, 16);
        out += 16;
        textlen += 16;
      } else {
        tcsm_sms4_encrypt(ctx->remain_c, ctx->cipher_buf, ctx->rk);
        for (size_t i = 0; i < 16; i += sizeof(size_t))
          *(size_t *)(ctx->cipher_buf + i) = *(size_t *)(ctx->cipher_buf + i) ^ *(size_t *)(ctx->iv + i);
        memcpy(ctx->iv, ctx->remain_c, 16);
        ctx->mres = n;
        *outlen = textlen;
        ctx->is_one_block_cached = 1;
        return ERR_TENCENTSM_OK;
      }
    }
  }

  while (inlen > 16) {
    tcsm_sms4_encrypt(in, out, ctx->rk);
    for (size_t i = 0; i < 16; i += sizeof(size_t))
      *(size_t *)(out + i) = *(size_t *)(out + i) ^ *(size_t *)(ctx->iv + i);
    memcpy(ctx->iv, in, 16);
    out += 16;
    in += 16;
    inlen -= 16;
    textlen += 16;
  }
  if (16 == inlen) {
    if (1 == ctx->no_padding) {
      tcsm_sms4_encrypt(in, out, ctx->rk);
      for (size_t i = 0; i < 16; i += sizeof(size_t))
        *(size_t *)(out + i) = *(size_t *)(out + i) ^ *(size_t *)(ctx->iv + i);
      memcpy(ctx->iv, in, 16);
      textlen += 16;
    } else {
      tcsm_sms4_encrypt(in, ctx->cipher_buf, ctx->rk);
      for (size_t i = 0; i < 16; i += sizeof(size_t))
        *(size_t *)(ctx->cipher_buf + i) = *(size_t *)(ctx->cipher_buf + i) ^ *(size_t *)(ctx->iv + i);
      memcpy(ctx->iv, in, 16);
      ctx->is_one_block_cached = 1;
    }
  } else {
    while (inlen--) {
      ctx->remain_c[n] = *(in++);
      ++n;
    }
  }
    
  ctx->mres = n;
  *outlen = textlen;
  return ERR_TENCENTSM_OK;
}
 
int SM4_CBC_Decrypt_Final(tcsm_sm4_cbc_t *ctx, unsigned char *out, size_t *outlen)
{
  int ret = ERR_TENCENTSM_OK;
  
  if (ctx->mres) {
    ret = ERR_SM4_ECB_ILLEGAL_MSGLEN;
  }
  if(1 == ctx->no_padding) {
    *outlen = 0;
  } else {
    size_t pad_value_check = 0;
    size_t padding_len = 0;
    if(0 == ctx->is_one_block_cached) {
      pad_value_check = 1;
    } else {
      padding_len = ctx->cipher_buf[15];
      /* test for valid padding value */
      if((1 > padding_len) || (SMS4_BLOCK_SIZE < padding_len)) {
        pad_value_check = 1;
      }else{
        size_t i = 16 - padding_len;
        for(; i  < 16 ; i++){
          if(ctx->cipher_buf[i] != padding_len){
            pad_value_check = 1;
            break;
          }
        }
      }
    }
    if(1 == pad_value_check) {
      ret = ERR_SM4_PKCS7_PADDING_VERIFY_FAILED;
    }else{
      *outlen = 16 - padding_len;
      memcpy(out, ctx->cipher_buf, *outlen);
    }
  }
  tcsm_tc_secure_free(ctx->rk);
  memset(ctx->remain_c, 0x00, 16);
  memset(ctx->cipher_buf, 0x00, 16);
  ctx->mlen = 0;
  ctx->mres = 0;
  return ret;
}

//-----------------CTR 加密-----------------

/* increment counter (128-bit int) by 1 */
static void SM4_CTR_Count_Update(unsigned char *count)
{
  unsigned int n = 16;
  do {
    --n;
    count[n] += 1u;
    if (count[n] >= 1u)
    {
        break;
    }
  } while (n);
}

int SM4_CTR_Encrypt_Init(tcsm_sm4_ctr_t *ctx, const unsigned char *key, const unsigned char *iv)
{
  ctx->rk = (tcsm_sms4_key_t*)tcsm_tc_secure_malloc(sizeof(tcsm_sms4_key_t));
  if(NULL == ctx->rk)
  {
      return ERR_TC_MALLOC;
  }
  memcpy(ctx->iv, iv, 16);
  memset(ctx->ekcnt, 0x00, 16);
  ctx->mlen = 0;
  ctx->mres = 0;
  tcsm_sms4_set_encrypt_key((tcsm_sms4_key_t *)ctx->rk, (const unsigned char*)key);
  return ERR_TENCENTSM_OK;
}

int SM4_CTR_Encrypt_Update(tcsm_sm4_ctr_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
  u64 mlen = ctx->mlen;
  mlen += inlen;
  if ((0 == inlen) || (mlen > ((U64(1) << 36) - 32)) || ((sizeof(inlen) == 8) && (mlen < inlen))) {
    *outlen = 0;
    tcsm_tc_secure_free(ctx->rk);
    return ERR_ILLEGAL_ARGUMENT;
  }
  *outlen = inlen;
  ctx->mlen = mlen;
    
  unsigned int n = ctx->mres;
  if (n) {
    while (n && inlen) {
      *(out++) = *(in++) ^ ctx->ekcnt[n];
      --inlen;
      n = (n + 1) % 16;
    }
    if(inlen == 0){
      ctx->mres = n;
      return ERR_TENCENTSM_OK;
    }
  }
 
  while (inlen >= 16) {
    tcsm_sms4_encrypt(ctx->iv, ctx->ekcnt, ctx->rk);
    for (size_t i = 0; i < 16; ++i)
      out[i] = in[i] ^ ctx->ekcnt[i];
    SM4_CTR_Count_Update(ctx->iv);
    out += 16;
    in += 16;
    inlen -= 16;
  }
  if (inlen) {
    tcsm_sms4_encrypt(ctx->iv, ctx->ekcnt, ctx->rk);
    SM4_CTR_Count_Update(ctx->iv);
    while (inlen--) {
      out[n] = in[n] ^ ctx->ekcnt[n];
      ++n;
    }
  }
  ctx->mres = n;
  return ERR_TENCENTSM_OK;
}

int SM4_CTR_Encrypt_Final(tcsm_sm4_ctr_t *ctx)
{
  tcsm_tc_secure_free(ctx->rk);
  memset(ctx->iv, 0x00, 16);
  memset(ctx->ekcnt, 0x00, 16);
  ctx->mlen = 0;
  ctx->mres = 0;
  return ERR_TENCENTSM_OK;
}

//-----------------CTR 解密-----------------

int SM4_CTR_Decrypt_Init(tcsm_sm4_ctr_t *ctx, const unsigned char *key, const unsigned char *iv)
{
  return SM4_CTR_Encrypt_Init(ctx, key, iv);
}

int SM4_CTR_Decrypt_Update(tcsm_sm4_ctr_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
  return SM4_CTR_Encrypt_Update(ctx, in, inlen, out, outlen);
}

int SM4_CTR_Decrypt_Final(tcsm_sm4_ctr_t *ctx)
{
  return SM4_CTR_Encrypt_Final(ctx);
}

//-----------------GCM 加密-----------------

int SM4_GCM_Encrypt_Init(tcsm_sm4_gcm_t *ctx, const unsigned char *key, const unsigned char *iv, size_t ivlen, const unsigned char *aad, size_t aadlen)
{
  ctx->rk = (tcsm_sms4_key_t*)tcsm_tc_secure_malloc(sizeof(tcsm_sms4_key_t));
  if(NULL == ctx->rk)
  {
    return ERR_TC_MALLOC;
  }
  ctx->gcm_ctx = (TCSM_GCM128_CONTEXT*)tcsm_tc_secure_malloc(sizeof(TCSM_GCM128_CONTEXT));
  if(NULL == ctx->gcm_ctx)
  {
    tcsm_tc_secure_free(ctx->rk);
    return ERR_TC_MALLOC;
  }
  tcsm_sms4_set_encrypt_key(ctx->rk, (const unsigned char*)key);
  tcsm_CRYPTO_gcm128_init(ctx->gcm_ctx, ctx->rk, (tcsm_block128_f)tcsm_sms4_encrypt);
    
  tcsm_CRYPTO_gcm128_setiv(ctx->gcm_ctx, iv, ivlen);
  if (aad != NULL) {
    if (0 != tcsm_CRYPTO_gcm128_aad(ctx->gcm_ctx, aad, aadlen)) {
      tcsm_tc_secure_free(ctx->rk);
      tcsm_tc_secure_free(ctx->gcm_ctx);
      return ERR_SM4_GCM_ILLEGAL_AADLEN;
    }
  }
  return ERR_TENCENTSM_OK;
}

int SM4_GCM_Encrypt_Update(tcsm_sm4_gcm_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
  int ret = ERR_TENCENTSM_OK;
  *outlen = 0;
  if (0 != inlen) {
    if (0 == tcsm_CRYPTO_gcm128_encrypt(ctx->gcm_ctx, in, out, inlen)) {
      *outlen = inlen;
      ret = ERR_TENCENTSM_OK;
    } else {
      tcsm_tc_secure_free(ctx->rk);
      tcsm_tc_secure_free(ctx->gcm_ctx);
      ret = ERR_SM4_GCM_ILLEGAL_MSGLEN;
    }
  } else {
    tcsm_tc_secure_free(ctx->rk);
    tcsm_tc_secure_free(ctx->gcm_ctx);
    ret = ERR_SM4_GCM_ILLEGAL_MSGLEN;
  }
  return ret;
}

int SM4_GCM_Encrypt_Final(tcsm_sm4_gcm_t *ctx, unsigned char *tag, size_t taglen)
{
  int ret = ERR_TENCENTSM_OK;
  if((0 == taglen) || (GCM_TAG_MAXLEN < taglen))
    ret = ERR_SM4_GCM_ILLEGAL_TAGLEN;
  else {
      tcsm_CRYPTO_gcm128_tag(ctx->gcm_ctx, tag, taglen);
  }
  tcsm_tc_secure_free(ctx->rk);
  tcsm_tc_secure_free(ctx->gcm_ctx);
  return ret;
}

//-----------------GCM 解密-----------------

int SM4_GCM_Decrypt_Init(tcsm_sm4_gcm_t *ctx, const unsigned char *key, const unsigned char *iv, size_t ivlen, const unsigned char *aad, size_t aadlen)
{
  return SM4_GCM_Encrypt_Init(ctx, key, iv, ivlen, aad, aadlen);
}

int SM4_GCM_Decrypt_Update(tcsm_sm4_gcm_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
  int ret = ERR_TENCENTSM_OK;
  *outlen = 0;
  if (0 != inlen) {
    if (0 == tcsm_CRYPTO_gcm128_decrypt(ctx->gcm_ctx, in, out, inlen)) {
      *outlen = inlen;
      ret = ERR_TENCENTSM_OK;
    } else {
      tcsm_tc_secure_free(ctx->rk);
      tcsm_tc_secure_free(ctx->gcm_ctx);
      ret = ERR_SM4_GCM_ILLEGAL_MSGLEN;
    }
  } else {
    tcsm_tc_secure_free(ctx->rk);
    tcsm_tc_secure_free(ctx->gcm_ctx);
    ret = ERR_SM4_GCM_ILLEGAL_MSGLEN;
  }
  return ret;
}

int SM4_GCM_Decrypt_Final(tcsm_sm4_gcm_t *ctx, const unsigned char *tag, size_t taglen)
{
  int ret = ERR_TENCENTSM_OK;
  if((0 == taglen) || (GCM_TAG_MAXLEN < taglen))
    ret = ERR_SM4_GCM_ILLEGAL_TAGLEN;
  else {
    if (0 != tcsm_CRYPTO_gcm128_finish(ctx->gcm_ctx, tag, taglen))
      ret = ERR_SM4_GCM_TAG_VERIFY_FAILED;
  }
  tcsm_tc_secure_free(ctx->rk);
  tcsm_tc_secure_free(ctx->gcm_ctx);
  return ret;
}

/*
Copyright 2019, Tencent Technology (Shenzhen) Co Ltd
Description: This file is part of the Tencent SM (Pro Version) Library.
*/

#include <stdlib.h>
#include "../include/tc.h"
#include "../include/tc_modes.h"
#include "../include/tc_gcm128_mode.h"
#include "../include/tc_sm4.h"

#define GCM_MAX_TAGLEN 16

int tcsm_sms4_gcm_encrypt(const unsigned char *in, size_t inlen, unsigned char *out,size_t *outlen, unsigned char *tag, size_t *taglen, const tcsm_sms4_key_t *key, unsigned char *iv,size_t ivlen,const unsigned char *aad, size_t aadlen, int enc, int no_padding) {

  TCSM_GCM128_CONTEXT ctx;

  size_t auth_len = (*taglen > GCM_MAX_TAGLEN) ? GCM_MAX_TAGLEN : *taglen;
  int ret = 0;
  tcsm_CRYPTO_gcm128_init(&ctx, key, (tcsm_block128_f)tcsm_sms4_encrypt);
  tcsm_CRYPTO_gcm128_setiv(&ctx, iv, ivlen);
  if (aad != NULL) {
    if ((ret = tcsm_CRYPTO_gcm128_aad(&ctx, aad, aadlen)) != 0)
      return ERR_SM4_GCM_ILLEGAL_AADLEN;
  }
  if (enc) {
    // PKCS#7 padding
    size_t len_after_padding = 0;
    if (no_padding == 0) {
      len_after_padding = (SMS4_BLOCK_SIZE - inlen % SMS4_BLOCK_SIZE) + inlen;
    }
    else {
      len_after_padding = inlen;
    }
    unsigned char *buf = (unsigned char*)tcsm_tc_secure_malloc(len_after_padding);
    if(NULL == buf) {
      return ERR_TC_MALLOC;
    }
    memcpy(buf, in, inlen);
    memset(buf + inlen, (int)(len_after_padding - inlen), len_after_padding - inlen);

    if ((ret = tcsm_CRYPTO_gcm128_encrypt(&ctx, buf, out, len_after_padding)) != 0)
      return ERR_SM4_GCM_ILLEGAL_MSGLEN;
    *outlen = len_after_padding;
    tcsm_tc_secure_free(buf);
    if(auth_len != 0) {
      tcsm_CRYPTO_gcm128_tag(&ctx, tag, auth_len);
      *taglen = auth_len;
    }else {
      return ERR_SM4_GCM_ILLEGAL_TAGLEN;
    }
  }
  else
  {
    if ((ret = tcsm_CRYPTO_gcm128_decrypt(&ctx, in, out, inlen)) != 0) {
      return ERR_SM4_GCM_ILLEGAL_MSGLEN;
    }
      
    if(auth_len != 0) {
      if ((ret = tcsm_CRYPTO_gcm128_finish(&ctx, tag, auth_len)) != 0) {
        return ERR_SM4_GCM_TAG_VERIFY_FAILED;
      }
    }else {
      return ERR_SM4_GCM_ILLEGAL_TAGLEN;
    }

    size_t padding_len = 0;
    if (no_padding == 0) {
      size_t pad_value_check = 0;
      padding_len = out[inlen - 1];

      /* test for valid padding value */
      if((1 > padding_len) || (SMS4_BLOCK_SIZE < padding_len)) {
        pad_value_check = 1;
      }else if(inlen <= padding_len){
        pad_value_check = 1;
      }else{
        size_t i = inlen - padding_len;
        for(; i  < inlen ; i++) {
          if(out[i] != padding_len){
            pad_value_check = 1;
            break;
          }
        }
      }
      if(1 == pad_value_check) {
        memset(out, 0, inlen);
        *outlen = 0;
        return ERR_SM4_PKCS7_PADDING_VERIFY_FAILED;
      }
    }
    else {
      padding_len = 0;
    }

    if (inlen <= padding_len) {
      *outlen = 0;
      return ERR_SM4_PKCS7_PADDING_VERIFY_FAILED;
    }

    *outlen = inlen - padding_len;
    memset(out + inlen - padding_len, 0, padding_len);
  }
  return ERR_TENCENTSM_OK;
}


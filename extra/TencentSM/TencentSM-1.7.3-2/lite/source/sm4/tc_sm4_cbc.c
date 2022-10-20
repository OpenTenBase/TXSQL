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
#include "../include/tc_modes.h"
#include "../include/tc_sm4.h"
#include "../include/tc.h"

int tcsm_sms4_cbc_encrypt(const unsigned char *in, size_t inlen, unsigned char *out,
    size_t *outlen, const tcsm_sms4_key_t *key, unsigned char *iv, int enc,int no_padding)
{
  if (enc) {
    // PKCS#7 padding
    size_t len_after_padding = 0;
    if (no_padding == 0) {
      len_after_padding = (SMS4_BLOCK_SIZE - inlen % SMS4_BLOCK_SIZE) + inlen;
    }else{
      len_after_padding = inlen;
    }
    unsigned char *buf = (unsigned char*)tcsm_tc_secure_malloc(len_after_padding);
    if(NULL == buf) {
      return ERR_TC_MALLOC;
    }
    memcpy(buf, in, inlen);
    memset(buf+ inlen, (int)(len_after_padding- inlen), len_after_padding- inlen);

    tcsm_CRYPTO_cbc128_encrypt(buf, out, len_after_padding, key, iv, (tcsm_block128_f)tcsm_sms4_encrypt);
    *outlen = len_after_padding;
    tcsm_tc_secure_free(buf);
  }else{
    tcsm_CRYPTO_cbc128_decrypt(in, out, inlen, key, iv, (tcsm_block128_f)tcsm_sms4_encrypt);
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
        for(; i  < inlen ; i++){
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
    }else{
      padding_len = 0;
    }

    if (inlen <= padding_len)
    {
      *outlen = 0;
      return ERR_SM4_PKCS7_PADDING_VERIFY_FAILED;
    }

    *outlen = inlen - padding_len;
    memset(out + inlen - padding_len, 0, padding_len);
  }
  return ERR_TENCENTSM_OK;
}

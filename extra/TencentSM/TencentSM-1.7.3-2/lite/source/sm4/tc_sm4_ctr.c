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

#include "stdlib.h"
#include <stdlib.h>
#include "../include/tc.h"
#include "../include/tc_modes.h"
#include "../include/tc_sm4.h"
#include "../include/tc_sm4_lcl.h"

/* increment counter (128-bit int) by 1 */

static void ctr128_inc(unsigned char *out, unsigned char *in)
{
  unsigned int n = 16, c = 1;
  do {
    --n;
    c += in[n];
    out[n] = (unsigned char)c;
    c >>= 8;
  } while (n);
}

int tcsm_sms4_ctr_encrypt(const unsigned char *in, size_t inlen, unsigned char *out,
    size_t *outlen, const tcsm_sms4_key_t *key, const unsigned char *iv)
{
  unsigned char *buf = (unsigned char*)malloc(inlen);
  if(NULL == buf) {
    return ERR_TC_MALLOC;
  }
  memcpy(buf, in, inlen);

  unsigned char ctr[8*16];
  memcpy(ctr, iv, 16);
  unsigned char *p_buff = buf;
  unsigned char *p_out = out;
  size_t i = 0;
  for(; i < inlen /(8 * SMS4_BLOCK_SIZE); i++)
  {
    ctr128_inc(ctr+16, ctr);
    ctr128_inc(ctr+32, ctr+16);
    ctr128_inc(ctr+48, ctr+32);
    ctr128_inc(ctr+64, ctr+48);
    ctr128_inc(ctr+80, ctr+64);
    ctr128_inc(ctr+96, ctr+80);
    ctr128_inc(ctr+112, ctr+96);
    size_t j = 0;
    for (; j < 8; ++j) {
      tcsm_sms4_encrypt(ctr+j*SMS4_BLOCK_SIZE, p_out+j*SMS4_BLOCK_SIZE, key);
    }
    size_t k = 0;
    for (; k < (8 * SMS4_BLOCK_SIZE); ++k) {
        p_out[k] ^= p_buff[k];
    }
    ctr128_inc(ctr, ctr+112);
    p_buff += 8 * SMS4_BLOCK_SIZE;
    p_out += 8 * SMS4_BLOCK_SIZE;
  }

  size_t bytelen = inlen % (8 * SMS4_BLOCK_SIZE);
  for (i = 0; i < bytelen/SMS4_BLOCK_SIZE; ++i) {
    tcsm_sms4_encrypt(ctr, p_out, key);
    size_t j = 0;
    for (; j < 16; ++j) {
      p_out[j] ^= p_buff[j];
    }
    ctr128_inc(ctr, ctr);
    p_buff += 16;
    p_out += 16;
  }
  //last block processing
  bytelen = inlen % SMS4_BLOCK_SIZE;
  if (0u != bytelen) {
    tcsm_sms4_encrypt(ctr, ctr, key);
    size_t j = 0;
    for (; j < bytelen; ++j) {
      p_out[j] = ctr[j] ^ p_buff[j];
    }
  }
  *outlen = inlen;
  tcsm_tc_free(buf);
  return ERR_TENCENTSM_OK;
}

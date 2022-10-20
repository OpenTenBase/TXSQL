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

#ifndef HEADER_TC_SM3_H
#define HEADER_TC_SM3_H

#include <sys/types.h>
#include <string.h>
#include "sm.h"
#include "tc_global.h"

#ifdef __cplusplus
extern "C" {
#endif
  struct stHmacSm3Ctx {
    sm3_ctx_t ctx;
    unsigned char key[SM3_BLOCK_SIZE];
  };
  
  void tcsm_sm3opt(const unsigned char *data, size_t datalen, unsigned char digest[SM3_DIGEST_LENGTH]);
  
  void tcsm_sm3_init_opt(sm3_ctx_t *ctx);
  void tcsm_sm3_update_opt(sm3_ctx_t *ctx, const unsigned char* data, size_t data_len);
  void tcsm_sm3_final_opt(sm3_ctx_t *ctx, unsigned char *digest);
  void tcsm_sm3_compress_opt(uint32_t digest[8], const unsigned char block[SM3_BLOCK_SIZE]);
  
int tcsm_sm3_hmac(const unsigned char *data, size_t data_len,
	const unsigned char *key, size_t key_len,
	unsigned char mac[SM3_HMAC_SIZE]);

TstHmacSm3Ctx* tcsm_sm3_hmac_init(const unsigned char *key, size_t key_len);
int tcsm_sm3_hmac_update(TstHmacSm3Ctx *ctx,
	const unsigned char *data, size_t data_len);
int tcsm_sm3_hmac_final(TstHmacSm3Ctx *ctx, unsigned char mac[SM3_HMAC_SIZE]);    
#ifdef __cplusplus
}
#endif
#endif

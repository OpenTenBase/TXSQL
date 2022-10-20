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

#ifndef HEADER_SMS4_H
#define HEADER_SMS4_H

#define SMS4_KEY_LENGTH		16
#define SMS4_BLOCK_SIZE		16
#define SMS4_IV_LENGTH		(SMS4_BLOCK_SIZE)
#define SMS4_NUM_ROUNDS		32

#include <sys/types.h>
#include <stdint.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t rk[SMS4_NUM_ROUNDS];
} tcsm_sms4_key_t;

void tcsm_sms4_set_encrypt_key(tcsm_sms4_key_t *key, const unsigned char *user_key);
void tcsm_sms4_set_decrypt_key(tcsm_sms4_key_t *key, const unsigned char *user_key);
void tcsm_sms4_encrypt(const unsigned char *in, unsigned char *out, const tcsm_sms4_key_t *key);
#define tcsm_sms4_decrypt(in,out,key)  tcsm_sms4_encrypt(in,out,key)

int tcsm_sms4_ecb_encrypt(const unsigned char *in, size_t inlen, unsigned char *out,
                      size_t *outlen, const tcsm_sms4_key_t *key, int enc,int no_padding);
int tcsm_sms4_cbc_encrypt(const unsigned char *in, size_t inlen, unsigned char *out,
                      size_t *outlen, const tcsm_sms4_key_t *key, unsigned char *iv, int enc,int no_padding);

int tcsm_sms4_ctr_encrypt(const unsigned char *in, size_t inlen, unsigned char *out,
                   size_t *outlen, const tcsm_sms4_key_t *key, const unsigned char *iv);

int tcsm_sms4_gcm_encrypt(const unsigned char *in, size_t inlen, unsigned char *out,size_t *outlen, unsigned char *tag, size_t *taglen, const tcsm_sms4_key_t *key, unsigned char *iv,size_t ivlen,const unsigned char *aad, size_t aadlen, int enc, int no_padding);

#ifdef __cplusplus
}
#endif
#endif


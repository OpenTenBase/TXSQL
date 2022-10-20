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

#ifndef  tcSM2_H
#define  tcSM2_H

#include "tc.h"
#include "sm.h"

#define     TC_ECCref_MAX_LEN    32
#define     SM3_DIGEST_SIZE      32
#define     TC_SM2_PARAMETER_LEN  128

#define     SIZEOF_sm2_cipher()  (TC_ECCref_MAX_LEN * 2 + SM3_DIGEST_SIZE + 4 + 1) 

#define     TCSM_NID_sm2p256v1      1

typedef struct tc_ECCCipher_st
{
  unsigned char XCoordinate[TC_ECCref_MAX_LEN];
  unsigned char YCoordinate[TC_ECCref_MAX_LEN];
  unsigned char HASH[SM3_DIGEST_SIZE];
  unsigned int CipherLen;
  unsigned char Cipher[1];//this is a pointer
}sm2_cipher;

typedef struct tc_ECCSignature_st
{
  unsigned char r[TC_ECCref_MAX_LEN];
  unsigned char s[TC_ECCref_MAX_LEN];
}sm2_signature;

typedef struct sm2_ecc_parameters
{
  char a[TC_SM2_PARAMETER_LEN];
  char b[TC_SM2_PARAMETER_LEN];
  char p[TC_SM2_PARAMETER_LEN];
  char Gx[TC_SM2_PARAMETER_LEN];
  char Gy[TC_SM2_PARAMETER_LEN];
  char n[TC_SM2_PARAMETER_LEN];
  char h[TC_SM2_PARAMETER_LEN];
}sm2_ecc_parameters_t;

typedef struct sm2_parameters
{
  unsigned int nid;
  sm2_ecc_parameters_t *param;
}sm2_parameters_t;

int tcsm_sm2_get_cipher_size(sm2_cipher *cipher);

int tcsm_sm2_create_group(tc_ec_group_t group, int nid);
int tcsm_sm2_generate_key(tc_ec_group_t group, tc_ec_t publickey, tc_bn_t privatekey);
int tcsm_sm2_encrypt(sm2_ctx_t *ctx, tc_ec_t publickey, unsigned char *plain, unsigned int plainlen, sm2_cipher *cipher);
int tcsm_sm2_decrypt(sm2_ctx_t *ctx, sm2_cipher *cipher, tc_bn_t privatekey, unsigned char *plain, unsigned int *plainlen);
int tcsm_sm2_sign(sm2_ctx_t *ctx, tc_bn_t privatekey, unsigned char *digst, unsigned int digstlen, sm2_signature *sign);
int tcsm_sm2_verify(sm2_ctx_t *ctx, tc_ec_t publickey, sm2_signature *sign, unsigned char *digst, unsigned int digstlen);

int tcsm_sm2_getz(tc_ec_group_t group, const char *id, unsigned int id_len, tc_ec_t publickey, unsigned char *za);
int tcsm_sm2_compute_message_digest(tc_ec_group_t group,tc_ec_t pubkey,SM_MD_TYPE id_md_type, SM_MD_TYPE msg_md_type,const void *msg, size_t msglen, const char* id, size_t idlen, unsigned char *dgst,size_t *dgstlen);

int tcsm_sm2_point_is_on_curve(sm2_ctx_t *ctx,tc_ec_t point);
int tcsm_sm2_key_exchange_tA(sm2_ctx_t *ctx,tc_ec_t RA,tc_bn_t rA,tc_bn_t dA,tc_bn_t tA);
int tcsm_sm2_key_exchange_U(sm2_ctx_t *ctx,tc_ec_t RB,tc_ec_t PB ,tc_bn_t tA,tc_ec_t U);
int tcsm_sm2_key_exchange_kdf(sm2_ctx_t *ctx,tc_ec_t U,tc_ec_t PA,tc_ec_t PB,const char* idA, size_t idAlen,const char* idB, size_t idBlen,size_t klen,unsigned char* KA,int A_is_initiator);

#endif  // TCSM2_H

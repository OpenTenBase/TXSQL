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
#include "../include/tc_digest.h"
#include "../include/tc_sm3.h"

int tcsm_SM_DigestInit(SM_MD_TYPE type, void* md_ctx)
{
  if (type == SM_MD_SM3) {
    sm3_ctx_t ctx;
    tcsm_sm3_init_opt(&ctx);
    
    memcpy(md_ctx, &ctx, sizeof(sm3_ctx_t));
    return ERR_TENCENTSM_OK;
  }
  return ERR_SM3_DIGEST_ARGUMENT;
}

int tcsm_SM_DigestUpdate(SM_MD_TYPE type, void* md_ctx, const void *in, size_t inlen)
{
  if (md_ctx == NULL)
    return ERR_SM3_DIGEST_ARGUMENT;
  
  if (type == SM_MD_SM3) {
    tcsm_sm3_update_opt((sm3_ctx_t*)md_ctx, (const unsigned char*)in, inlen);
    return ERR_TENCENTSM_OK;
  }
  return ERR_SM3_DIGEST_ARGUMENT;
}

int tcsm_SM_DigestFinal(SM_MD_TYPE type, void* md_ctx, unsigned char *md, unsigned int *len)
{
  if (md_ctx == NULL)
    return ERR_SM3_DIGEST_ARGUMENT;
  
  if (type == SM_MD_SM3) {
    tcsm_sm3_final_opt((sm3_ctx_t*)md_ctx, md);
    *len = SM3_DIGEST_LENGTH;
    return ERR_TENCENTSM_OK;
  }
  return ERR_SM3_DIGEST_ARGUMENT;
  }

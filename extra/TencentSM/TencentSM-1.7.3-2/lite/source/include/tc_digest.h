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

#ifndef TC_DIGEST_T
#define TC_DIGEST_T

#include "sm.h"

#ifdef __cplusplus
extern "C" {
#endif
  int tcsm_SM_DigestInit(SM_MD_TYPE type, void* md_ctx);
  
  int tcsm_SM_DigestUpdate(SM_MD_TYPE type, void* md_ctx, const void *in, size_t inlen);
  
  int tcsm_SM_DigestFinal(SM_MD_TYPE type, void* md_ctx, unsigned char *md, unsigned int *len);
#ifdef __cplusplus
}
#endif

#endif /* TC_DIGEST_T */

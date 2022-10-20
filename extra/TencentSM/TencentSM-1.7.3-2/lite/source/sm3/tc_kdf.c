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
#include "../include/tc_kdf.h"
#include "../include/tc_sm3.h"
#include "../include/tc_utils.h"

int tcsm_x9_63_kdf_sm3(const unsigned char *share, size_t sharelen, unsigned char *outkey, size_t keylen)
{
    unsigned int new_counter = 1;
    unsigned int new_counter_be = 1;
    unsigned char dgst[32];
    unsigned int dgstlen=32;
    int rlen = (int)keylen;
    unsigned char *SkdfZ = NULL;
    
    if (keylen > (0xFFFFFFFF * 32))
    {
        return ERR_SM3_KDF_ARGUMENT;
    }
    if(sharelen > 1024)
    {
        return ERR_SM3_KDF_ARGUMENT;
    }
    if ((SkdfZ = (unsigned char *)tcsm_tc_secure_malloc((int)sharelen+4)) == NULL)
        return ERR_TC_MALLOC;
    
    memcpy(SkdfZ, share, sharelen);
    while (rlen > 0)
    {
        new_counter_be = cpu_to_be32(new_counter);
        memcpy(SkdfZ+sharelen, &new_counter_be, 4);
        
        tcsm_sm3opt((const unsigned char*)SkdfZ, (int)sharelen + (int)sizeof(new_counter_be), dgst);
        
        if (rlen <= dgstlen) {
            memcpy(outkey, dgst, rlen);
        }else{
            memcpy(outkey, dgst, (keylen>=dgstlen ? dgstlen:keylen));
        }
        
        rlen -= dgstlen;
        outkey += dgstlen;
        new_counter++;
    }
    tcsm_tc_secure_free(SkdfZ);
    return ERR_TENCENTSM_OK;
}

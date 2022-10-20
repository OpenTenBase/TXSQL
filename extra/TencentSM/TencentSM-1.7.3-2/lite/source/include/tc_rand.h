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

#ifndef TC_RAND_H
#define TC_RAND_H

#define STATE_SIZE     1023
#define MD_DIGEST_LENGTH 32
#define ENTROPY_NEEDED   32

typedef struct {
    size_t state_num;
    size_t state_index;
    unsigned char state[STATE_SIZE + MD_DIGEST_LENGTH];
    unsigned char md[MD_DIGEST_LENGTH];
    long md_count[2];
    double entropy;
    int initialized;
} rand_ctx_t;

/* Random Functions */
void* tcsm_tc_rand_init(void);
void tcsm_tc_rand_bignum(void* ctx,tc_bn_t op, tc_bn_t n);
void tcsm_tc_rand_bytes(void* ctx,unsigned char *buf, int num);
void tcsm_tc_rand_clear(void* ctx);

void tcsm_cs_rand_add(const void *buf, int num, double add,rand_ctx_t* ctx);

#endif /* TC_RAND_H */

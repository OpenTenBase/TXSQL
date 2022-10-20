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

#ifndef TC_EC_MUL_H
#define TC_EC_MUL_H

#include "tc.h"

int tcsm_ec_mul_for_G(tc_ec_group_t group, tc_ec_t r, tc_bn_t k);
int tcsm_ec_mul_for_pubkey(tc_ec_group_t group, tc_ec_t r, tc_ec_t p,tc_bn_t k);
int tcsm_ec_mul_for_point(tc_ec_group_t group, tc_ec_t r, tc_ec_t p, tc_bn_t k);
/* Pre Compute */
int tcsm_ec_mul_precompute_for_pubkey(tc_ec_group_t group,tc_ec_t r);

const tc_ec_pre_comp_info* tcsm_ec_get_pre_comp_info(tc_ecc_group_st *group);
const tc_ec_pre_comp_info* tcsm_ec_get_pre_comp_pubkey_info(tc_ecc_group_st *group);

#endif /* TC_EC_MUL_H */

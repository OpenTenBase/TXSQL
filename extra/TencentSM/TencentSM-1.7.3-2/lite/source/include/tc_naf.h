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

#ifndef __TC_NAF_H__
#define __TC_NAF_H__

#include <math.h>
#include "gmp.h"

#define NAF_WINDOW 4
#define PRE_COMPUTE_NAF_WINDOW 7
#define PRE_COMPUTE_POINTS_COUNT 2048

int   tcsm_naf_convert_inverse(char[], int);
void  tcsm_naf_init_bits(mpz_ptr, int);
void  tcsm_naf_set_bit(mpz_ptr, int, short);
void  tcsm_naf_get_substr(mpz_ptr, char[], int, int);
void  tcsm_naf_convert(mpz_ptr, mpz_ptr, int *);
short tcsm_naf_get_bit(mpz_ptr, int);

void  tcsm_wnaf_init_bits(mpz_ptr, int);
void  tcsm_wnaf_set_bit(mpz_ptr, int, short);
short tcsm_wnaf_get_bit(mpz_ptr, int);

#endif

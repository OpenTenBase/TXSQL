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

#include <stdio.h>

#include "../include/tc.h"
#include "../include/tc_naf.h"

void tcsm_naf_init_bits(mpz_ptr k, int max_bits)
{
  mpz_init2(k, max_bits*2);
}

void tcsm_naf_set_bit(mpz_ptr k_naf, int bit, short val)
{
  bit <<= 2; // 2*bit
  if(val == 1)
    mpz_setbit(k_naf, bit);
  else if(val == -1) // -1 => {11}
  {
    mpz_setbit(k_naf, bit);
    mpz_setbit(k_naf, bit+1);
  }
}

short tcsm_naf_get_bit(mpz_ptr naf_k, int bit)
{
  bit <<= 2; // 2*bit
  if(mpz_tstbit(naf_k, bit))
  {
    if(mpz_tstbit(naf_k, bit+1))
      return -1;
    else
      return 1;
  }
  else
    return 0;
}

void tcsm_naf_get_substr(mpz_ptr naf_k, char naf_subk[], int bgn, int end)
{
  for(int i = 0; i < end; i++)
  {
    if(tcsm_naf_get_bit(naf_k, bgn-i) == 1)
      naf_subk[i] = '1';
    else if(tcsm_naf_get_bit(naf_k, bgn-i) == -1)
      naf_subk[i] = '-';
    else
      naf_subk[i] = '0';
  }
  naf_subk[end] = '\0';
}

void tcsm_naf_convert(mpz_ptr k_naf, mpz_ptr k, int *size)
{
  int i = 0;
  mpz_t kmod, ki;
  mpz_inits(kmod, ki, NULL);
  
  while(mpz_sgn(k)) // k > 0
  {
    if(mpz_odd_p(k))
    {
      mpz_tdiv_r_ui(kmod, k, 4); // kmod = k%4
      mpz_ui_sub(ki, 2, kmod);
      mpz_sub(k, k, ki);
    }
    else
      mpz_set_ui(ki, 0);
    
    mpz_cdiv_q_2exp(k, k, 1); // k = k/(2^1) <- right shifts
    tcsm_naf_set_bit(k_naf, i, mpz_get_si(ki)); // set i-th bit (actually, i and i+1)
    i++;
  }
  *size = i;
  
  mpz_clears(kmod, ki, NULL);
}

int tcsm_naf_convert_inverse(char naf_str[], int size)
{
  int i, v = 0;
  for(i = size-1; i >= 0; i--)
  {
    if(naf_str[i] == '-')
      v = v - (1 << (size-i-1));
    else if(naf_str[i] == '1')
      v = v + (1 << (size-i-1));
  }
  return v;
}


void naf_print(mpz_ptr naf_k, int limit)
{
  int i;
  for(i = limit-1; i >= 0; i--)
    LOGV("%d", tcsm_naf_get_bit(naf_k, i));
  LOGV("\n");
}


void tcsm_wnaf_init_bits(mpz_ptr k, int max_bits)
{
  mpz_init2(k, max_bits*NAF_WINDOW);
}

void tcsm_wnaf_set_bit(mpz_ptr k_naf, int bit_pos, short val)
{
  short bit_val, bit_count;
  bit_pos <<= NAF_WINDOW;
  if(val < 0)
  {
    mpz_setbit(k_naf, bit_pos);
    val = -val;
  }

  bit_val = 0;
  bit_count = 1;
  while(val > 0)
  {
    bit_val = val%2;
    if(bit_val)
      mpz_setbit(k_naf, bit_pos + bit_count);
    val >>= 1; // val /= 2
    bit_count++;
  }
}

short tcsm_wnaf_get_bit(mpz_ptr naf_k, int bit_pos)
{
  short bit_count, val;
  bit_pos <<= NAF_WINDOW;
  val = 0;
  for(bit_count = 1; bit_count <= NAF_WINDOW; bit_count++)
    if(mpz_tstbit(naf_k, bit_pos + bit_count))
      val += (1 << (bit_count-1));

  if(mpz_tstbit(naf_k, bit_pos))
    val *= -1;
  return val;
}

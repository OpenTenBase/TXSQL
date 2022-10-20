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
#include "../include/tc_naf.h"
#include "../include/tc_ec_mul.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#ifdef _OPT_ASM_ECC
#include "../include/ecp_sm2z256_macro.h"
extern int ecp_sm2z256_precompute_for_point(tc_ecc_group_st *group,tc_ec_t point);
extern int ecp_sm2z256_points_mul_for_generator(tc_ecc_group_st *group, tc_ecc_point_st* r, tc_bn_st* scalar);
extern int ecp_sm2z256_points_mul_for_pubkey(tc_ecc_group_st *group, tc_ecc_point_st* r, tc_bn_st* scalar);
extern int ecp_sm2z256_points_mul_for_point(tc_ecc_group_st *group, tc_ecc_point_st* r,P256_POINT *point, tc_bn_st *scalar);
#endif

#define WNAF_DEBUG 0

#define NAF_UPPER_BOUND_PI  1 << (NAF_WINDOW-1)

#define EC_window_bits_for_scalar_size(b) \
((size_t) \
((b) >= 2000 ? 6 : \
(b) >=  800 ? 5 : \
(b) >=  300 ? 4 : \
(b) >=   70 ? 3 : \
(b) >=   20 ? 2 : \
1))

int tcsm_ec_mul_precompute_for_pubkey(tc_ec_group_t group,tc_ec_t pubkey)
{
#ifdef _OPT_ASM_ECC
  return ecp_sm2z256_precompute_for_point(group,pubkey);
#else
  tc_ec_pre_comp_info* pre_comp_p = group->ctx->pre_comp_p;
  int ret = tcsm_tc_ec_precompute_mul(group,pubkey,&pre_comp_p);
  group->ctx->pre_comp_p = pre_comp_p;
  return ret;
#endif
}

static tc_ec_pre_comp_info *tc_ec_pre_comp_new(tc_ecc_group_st *group)
{
  tc_ec_pre_comp_info *ret = NULL;
  
  if (!group)
    return NULL;
  
  ret = (tc_ec_pre_comp_info *)tcsm_tc_secure_malloc(sizeof(tc_ec_pre_comp_info));
  if (!ret) {
    return ret;
  }
  ret->group = group;
  ret->blocksize = 8;         /* default */
  ret->numblocks = 0;
  ret->w = 4;                 /* default */
  ret->points = NULL;
  ret->num = 0;
  ret->references = 1;
  return ret;
}

int tcsm_tc_ec_precompute_mul(tc_ec_group_t group,tc_ec_t point,tc_ec_pre_comp_info** ptr_info)
{
  int i = 0;
  int blocksize = 8;
  int numblocks = 0;
  size_t num = 0;
  size_t pre_points_per_block = 0;
  
  tc_ec_t tmp_point;tcsm_tc_ec_init(tmp_point);
  
  tc_ec_t base;tcsm_tc_ec_init(base);tcsm_tc_ec_cpy(base, point);
  
  *ptr_info = tc_ec_pre_comp_new(group);
  
  tc_ecc_point_st** precompute_points = tcsm_tc_secure_malloc(PRE_COMPUTE_POINTS_COUNT * sizeof(tc_ecc_point_st*));
  
  tc_ecc_point_st** var = precompute_points;
  
  for (int i = 0 ; i < PRE_COMPUTE_POINTS_COUNT; i++) {
    precompute_points[i] = tcsm_tc_secure_malloc(sizeof(tc_ec_t));
  }
  
  for (int k = 0; k < PRE_COMPUTE_POINTS_COUNT; k++) {
    tcsm_tc_ec_init(precompute_points[k]);
  }
  
  int bits = tcsm_tc_bn_num_bits(group->n);
  /*
   * The following parameters mean we precompute (approximately) one point
   * per bit. TBD: The combination 8, 4 is perfect for 160 bits; for other
   * bit lengths, other parameter combinations might provide better
   * efficiency.
   */
  blocksize = 8;
  int w = PRE_COMPUTE_NAF_WINDOW;
  if (EC_window_bits_for_scalar_size(bits) > w) {
    /* let's not make the window too small ... */
    w = (int)EC_window_bits_for_scalar_size((int)bits);
  }
  
  numblocks = (bits + blocksize - 1) / blocksize; /* max. number of blocks
                                                   * to use for wNAF
                                                   * splitting */
  
  pre_points_per_block = (size_t)1 << (w - 1);
  num = pre_points_per_block * numblocks; /* number of points to compute
                                           * and store */
  
#if WNAF_DEBUG
  int index = 0;
  int multi = 1;
  int tmp_multi = 0;
  int base_multi = 1;
#endif
  
  /* do the precomputation */
  for (i = 0; i < numblocks; i++) {
    size_t j;
    
#if WNAF_DEBUG
    tmp_multi = 2*base_multi;
    multi = base_multi;
    LOGV("index:%d multi:%d\n",index++,multi);
#endif
    
    tcsm_tc_ec_double(group, tmp_point, base);
    tcsm_tc_ec_cpy(*var++, base);
    
    numblocks = (SM2_CURVE_MAX_BIT + blocksize - 1) / blocksize; /* max. number of blocks
                                                                  * to use for wNAF
                                                                  * splitting */
    
    pre_points_per_block = (size_t)1 << (PRE_COMPUTE_NAF_WINDOW - 1);
    num = pre_points_per_block * numblocks; /* number of points to compute
                                             * and store */
    
    for (j = 1; j < pre_points_per_block; j++, var++) {
      /*
       * calculate odd multiples of the current base point
       */
      tcsm_tc_ec_add(group, *var, tmp_point, *(var - 1));
      
 #if WNAF_DEBUG
      multi = tmp_multi + multi;
      LOGV("index:%d multi:%d\n",index,multi);
      index++;
#endif
    }
    
    if (i < numblocks - 1) {
      /*
       * get the next base (multiply current one by 2^blocksize)
       */
      size_t k;
#if WNAF_DEBUG
      base_multi = 2*tmp_multi;
#endif
      tcsm_tc_ec_double(group, base, tmp_point);
      for (k = 2; k < blocksize; k++) {
 #if WNAF_DEBUG
        base_multi = 2*base_multi;
#endif
        tcsm_tc_ec_double(group, base, base);
      }
    }
  }
  
  (*ptr_info)->group = group;
  (*ptr_info)->blocksize = blocksize;
  (*ptr_info)->numblocks = numblocks;
  (*ptr_info)->w = PRE_COMPUTE_NAF_WINDOW;
  (*ptr_info)->points = precompute_points;
  (*ptr_info)->num = num;
  
  tcsm_tc_ec_clear(tmp_point);
  tcsm_tc_ec_clear(base);
  
  return 0;
}

const tc_ec_pre_comp_info* tcsm_ec_get_pre_comp_info(tc_ecc_group_st *group)
{
  return group->ctx->pre_comp_g;
}

const tc_ec_pre_comp_info* tcsm_ec_get_pre_comp_pubkey_info(tc_ecc_group_st *group)
{
  return group->ctx->pre_comp_p;;
}

static signed char *tc_compute_wnaf(tc_bn_st *scalar, int w, size_t *ret_len)
{
  int window_val;
  int ok = 0;
  signed char *r = NULL;
  int sign = 1;
  int bit, next_bit, mask;
  size_t len = 0, j;
  
  if (tcsm_tc_bn_get_ui(scalar) == 0) {
    r = tcsm_tc_secure_malloc(1);
    if (!r) {
      goto err;
    }
    r[0] = 0;
    *ret_len = 1;
    return r;
  }
  
  if (w <= 0 || w > 7) {      /* 'signed char' can represent integers with
                               * absolute values less than 2^7 */
    goto err;
  }
  bit = 1 << w;               /* at most 128 */
  next_bit = bit << 1;        /* at most 256 */
  mask = next_bit - 1;        /* at most 255 */
  
  if (tcsm_tc_bn_sgn(scalar) < 0) {
    sign = -1;
  }
  
  len = tcsm_tc_bn_num_bits(scalar);
  r = tcsm_tc_secure_malloc((size_t)len + 1); /* modified wNAF may be one digit longer
                                   * than binary representation (*ret_len will
                                   * be set to the actual length, i.e. at most
                                   * BN_num_bits(scalar) + 1) */
  if (r == NULL) {
    goto err;
  }
  window_val = scalar->val->_mp_d[0] & mask;
  j = 0;
  while ((window_val != 0) || (j + w + 1 < len)) { /* if j+w+1 >= len,
                                                    * window_val will not
                                                    * increase */
    int digit = 0;
    
    /* 0 <= window_val <= 2^(w+1) */
    
    if (window_val & 1) {
      /* 0 < window_val < 2^(w+1) */
      
      if (window_val & bit) {
        digit = window_val - next_bit; /* -2^w < digit < 0 */
        if (j + w + 1 >= len) {
          /*
           * special case for generating modified wNAFs: no new
           * bits will be added into window_val, so using a
           * positive digit here will decrease the total length of
           * the representation
           */
          
          digit = window_val & (mask >> 1); /* 0 < digit < 2^w */
        }
      } else {
        digit = window_val; /* 0 < digit < 2^w */
      }
      
      if (digit <= -bit || digit >= bit || !(digit & 1)) {
        goto err;
      }
      
      window_val -= digit;
      /*
       * now window_val is 0 or 2^(w+1) in standard wNAF generation;
       * for modified window NAFs, it may also be 2^w
       */
      if (window_val != 0 && window_val != next_bit
          && window_val != bit) {
        goto err;
      }
    }
    r[j++] = sign * digit;
    window_val >>= 1;
    window_val += bit * tcsm_tc_bn_is_bit_set(scalar, (int)j + w);
    
    if (window_val > next_bit) {
      goto err;
    }
  }
  
  if (j > len + 1) {
    goto err;
  }
  len = j;
  ok = 1;
err:
  if (!ok) {
    tcsm_tc_secure_free(r);
    r = NULL;
  }
  if (ok)
    *ret_len = len;
  return r;
}

int tc_ec_wnaf_splitting_mul(tc_ecc_group_st *group, tc_ecc_point_st* r, tc_bn_st* scalar,
                          size_t num, tc_ecc_point_st *points[], tc_bn_st *scalars[],const tc_ec_pre_comp_info* pre_comp,tc_ec_t precomp_point)
{
  tc_ec_t tmp;tcsm_tc_ec_init(tmp);
  
  size_t totalnum;
  size_t blocksize = 0, numblocks = 0; /* for wNAF splitting */
  size_t pre_points_per_block = 0;
  size_t i, j;
  int k;
  int r_is_inverted = 0;
  int r_is_at_infinity = 1;
  size_t *wsize = NULL;      /* individual window sizes */
  signed char **wNAF = NULL; /* individual wNAFs */
  size_t *wNAF_len = NULL;
  size_t max_len = 0;
  size_t num_val;
  tc_ecc_point_st **val = NULL; /* precomputation */
  tc_ecc_point_st **v;
  tc_ecc_point_st ***val_sub = NULL; /* pointers to sub-arrays of 'val' or 'pre_comp->points' */
//  const tc_ec_pre_comp_info *pre_comp = NULL;
  int num_scalar = 0; /* flag: will be set to 1 if 'scalar' must be treated like
                       * other scalars,
                       * i.e. precomputation is not available */
  int ret = ERR_ECC_POINTS_MUL;
  
  if (scalar != NULL) {
    /* look if we can use precomputed multiples of generator */
//    pre_comp = tcsm_ec_get_pre_comp_info(group);
    
    if (pre_comp && pre_comp->numblocks && (tcsm_tc_ec_cmp(precomp_point, pre_comp->points[0]) == 0)) {
      blocksize = pre_comp->blocksize;
      /* determine maximum number of blocks that wNAF splitting may yield
       * (NB: maximum wNAF length is bit length plus one) */
      numblocks = (tcsm_tc_bn_num_bits(scalar) / blocksize) + 1;
      /* we cannot use more blocks than we have precomputation for */
      if (numblocks > pre_comp->numblocks)
        numblocks = pre_comp->numblocks;
      pre_points_per_block = (size_t)1 << (pre_comp->w - 1);
      /* check that pre_comp looks sane */
      if (pre_comp->num != (pre_comp->numblocks * pre_points_per_block)) {
        goto err;
      }
    } else {
      /* can't use precomputation */
      pre_comp = NULL;
      numblocks = 1;
      num_scalar = 1; /* treat 'scalar' like 'num'-th element of 'scalars' */
    }
  }
  
  totalnum = num + numblocks;
  wsize = tcsm_tc_secure_malloc((size_t)totalnum * sizeof wsize[0]);
  wNAF_len = tcsm_tc_secure_malloc((size_t)totalnum * sizeof wNAF_len[0]);
  wNAF = tcsm_tc_secure_malloc(((size_t)totalnum + 1) * sizeof wNAF[0]); /* includes space for pivot */
  val_sub = tcsm_tc_secure_malloc((size_t)totalnum * sizeof val_sub[0]);
  /* Ensure wNAF is initialised in case we end up going to err. */
  if (wNAF) {
    wNAF[0] = NULL; /* preliminary pivot */
  }
  if (!wsize || !wNAF_len || !wNAF || !val_sub) {
    goto err;
  }
  
  /* num_val will be the total number of temporarily precomputed points */
  num_val = 0;
  for (i = 0; i < num + num_scalar; i++) {
    size_t bits;
    bits = i < num ? tcsm_tc_bn_num_bits(scalars[i]) : tcsm_tc_bn_num_bits(scalar);
    wsize[i] = EC_window_bits_for_scalar_size(bits);
    num_val += (size_t)1 << (wsize[i] - 1);
    wNAF[i + 1] = NULL; /* make sure we always have a pivot */
    wNAF[i] =
    tc_compute_wnaf((i < num ? scalars[i] : scalar), (int)wsize[i], &wNAF_len[i]);
    if (wNAF[i] == NULL)
      goto err;
    if (wNAF_len[i] > max_len)
      max_len = wNAF_len[i];
  }
  
  if (numblocks) {
    /* we go here if scalar != NULL */
    if (pre_comp == NULL) {
      if (num_scalar != 1) {
        goto err;
      }
      /* we have already generated a wNAF for 'scalar' */
    } else {
      signed char *tmp_wNAF = NULL;
      size_t tmp_len = 0;
      if (num_scalar != 0) {
        goto err;
      }
      /* use the window size for which we have precomputation */
      wsize[num] = pre_comp->w;
      tmp_wNAF = tc_compute_wnaf(scalar, (int)wsize[num], &tmp_len);
      if (!tmp_wNAF)
        goto err;
      if (tmp_len <= max_len) {
        /* One of the other wNAFs is at least as long
         * as the wNAF belonging to the generator,
         * so wNAF splitting will not buy us anything. */
        numblocks = 1; /* don't use wNAF splitting */
        totalnum = num + numblocks;
        wNAF[num] = tmp_wNAF;
        wNAF[num + 1] = NULL;
        wNAF_len[num] = tmp_len;
        /* pre_comp->points starts with the points that we need here: */
        val_sub[num] = pre_comp->points;
      } else {
        /* don't include tmp_wNAF directly into wNAF array
         * - use wNAF splitting and include the blocks */
        signed char *pp;
        tc_ecc_point_st **tmp_points;
        if (tmp_len < numblocks * blocksize) {
          /* possibly we can do with fewer blocks than estimated */
          numblocks = (tmp_len + blocksize - 1) / blocksize;
          if (numblocks > pre_comp->numblocks) {
            goto err;
          }
          totalnum = num + numblocks;
        }
        /* split wNAF in 'numblocks' parts */
        pp = tmp_wNAF;
        tmp_points = pre_comp->points;
        for (i = num; i < totalnum; i++) {
          if (i < totalnum - 1) {
            wNAF_len[i] = blocksize;
            if (tmp_len < blocksize) {
              goto err;
            }
            tmp_len -= blocksize;
          } else
          /* last block gets whatever is left
           * (this could be more or less than 'blocksize'!) */
            wNAF_len[i] = tmp_len;
          wNAF[i + 1] = NULL;
          wNAF[i] = tcsm_tc_secure_malloc(wNAF_len[i]);
          if (wNAF[i] == NULL) {
            tcsm_tc_secure_free(tmp_wNAF);
            goto err;
          }
          memcpy(wNAF[i], pp, wNAF_len[i]);
          if (wNAF_len[i] > max_len)
            max_len = wNAF_len[i];
          if (*tmp_points == NULL) {
            tcsm_tc_secure_free(tmp_wNAF);
            goto err;
          }
          val_sub[i] = tmp_points;
          tmp_points += pre_points_per_block;
          pp += blocksize;
        }
        tcsm_tc_secure_free(tmp_wNAF);
      }
    }
  }
  /* All points we precompute now go into a single array 'val'.
   * 'val_sub[i]' is a pointer to the subarray for the i-th point,
   * or to a subarray of 'pre_comp->points' if we already have precomputation.
   */
  val = tcsm_tc_secure_malloc((num_val + 1) * sizeof val[0]);
  if (val == NULL) {
    goto err;
  }
  val[num_val] = NULL; /* pivot element */
  /* allocate points for precomputation */
  v = val;
  for (i = 0; i < num + num_scalar; i++) {
    val_sub[i] = v;
    for (j = 0; j < ((size_t)1 << (wsize[i] - 1)); j++) {
      
      tc_ec_t* tc_ec_t_v_ptr = tcsm_tc_secure_malloc(sizeof(tc_ec_t));
      tcsm_tc_ec_init(*tc_ec_t_v_ptr);
      *v = *tc_ec_t_v_ptr;
      if (*v == NULL)
        goto err;
      v++;
    }
  }
  if (!(v == val + num_val)) {
    goto err;
  }
  /* prepare precomputed values:
   *    val_sub[i][0] :=     points[i]
   *    val_sub[i][1] := 3 * points[i]
   *    val_sub[i][2] := 5 * points[i]
   *    ...
   */
  tc_ec_t* curve_generator =  group->ctx->generator;
  for (i = 0; i < num + num_scalar; i++) {
    if (i < num) {
      tcsm_tc_ec_cpy(val_sub[i][0], points[i]);
    } else {
      tcsm_tc_ec_cpy(val_sub[i][0], *curve_generator);
    }
    if (wsize[i] > 1) {
      tcsm_tc_ec_double(group, tmp, val_sub[i][0]);
      for (j = 1; j < ((size_t)1 << (wsize[i] - 1)); j++) {
        tcsm_tc_ec_add(group, val_sub[i][j], val_sub[i][j - 1], tmp);
      }
    }
  }
  
  r_is_at_infinity = 1;
  for (k = (int)max_len - 1; k >= 0; k--) {
    if (!r_is_at_infinity) {
      tcsm_tc_ec_double(group, r, r);
    }
    for (i = 0; i < totalnum; i++) {
      if (wNAF_len[i] > (size_t)k) {
        int digit = wNAF[i][k];
        int is_neg;
        if (digit) {
          is_neg = digit < 0;
          if (is_neg)
            digit = -digit;
          if (is_neg != r_is_inverted) {
            if (!r_is_at_infinity) {
              tcsm_tc_ec_invert(group, r, r);
            }
            r_is_inverted = !r_is_inverted;
          }
          /* digit > 0 */
          if (r_is_at_infinity) {
            tcsm_tc_ec_cpy(r, val_sub[i][digit >> 1]);
            r_is_at_infinity = 0;
          } else {
            tcsm_tc_ec_add(group, r, r, val_sub[i][digit >> 1]);
          }
        }
      }
    }
  }
  if (r_is_at_infinity) {
    tcsm_tc_ec_set_str(r, "0", 10, "0", 10);
  } else {
    if (r_is_inverted)
      tcsm_tc_ec_invert(group, r, r);
  }
  ret = ERR_TENCENTSM_OK;
err:
  tcsm_tc_ec_clear(tmp);
  if (wsize != NULL)
    tcsm_tc_secure_free(wsize);
  if (wNAF_len != NULL)
    tcsm_tc_secure_free(wNAF_len);
  if (wNAF != NULL) {
    signed char **w;
    for (w = wNAF; *w != NULL; w++)
      tcsm_tc_secure_free(*w);
    tcsm_tc_secure_free(wNAF);
  }
  if (val != NULL) {
    for (v = val; *v != NULL; v++)
    {
      tcsm_tc_ec_clear(*v);
      tcsm_tc_secure_free(*v);
    }
    tcsm_tc_secure_free(val);
  }
  if (val_sub != NULL) {
    tcsm_tc_secure_free(val_sub);
  }
  return ret;
}

int tc_ec_jcb_wnaf_splitting_mul(tc_ecc_group_st *group, tc_ecc_jcb_point_st* r,
                             size_t num, tc_ecc_jcb_point_st *points[], tc_bn_st *scalars[])
{
  tc_ec_jcb_t tmp;tcsm_tc_ec_jcb_init(tmp);
  
  size_t totalnum;
  size_t numblocks = 0; /* for wNAF splitting */
  size_t i, j;
  int k;
  int r_is_inverted = 0;
  int r_is_at_infinity = 1;
  size_t *wsize = NULL;      /* individual window sizes */
  signed char **wNAF = NULL; /* individual wNAFs */
  size_t *wNAF_len = NULL;
  size_t max_len = 0;
  size_t num_val;
  tc_ecc_jcb_point_st **val = NULL; /* precomputation */
  tc_ecc_jcb_point_st **v;
  tc_ecc_jcb_point_st ***val_sub = NULL; /* pointers to sub-arrays of 'val' or 'pre_comp->points' */
  int num_scalar = 0; /* flag: will be set to 1 if 'scalar' must be treated like
                       * other scalars,
                       * i.e. precomputation is not available */
  int ret = ERR_ECC_POINTS_MUL;
  
  totalnum = num + numblocks;
  wsize = tcsm_tc_secure_malloc((size_t)totalnum * sizeof wsize[0]);
  wNAF_len = tcsm_tc_secure_malloc((size_t)totalnum * sizeof wNAF_len[0]);
  wNAF = tcsm_tc_secure_malloc(((size_t)totalnum + 1) * sizeof wNAF[0]); /* includes space for pivot */
  val_sub = tcsm_tc_secure_malloc((size_t)totalnum * sizeof val_sub[0]);
  /* Ensure wNAF is initialised in case we end up going to err. */
  if (wNAF) {
    wNAF[0] = NULL; /* preliminary pivot */
  }
  if (!wsize || !wNAF_len || !wNAF || !val_sub) {
    goto err;
  }
  
  /* num_val will be the total number of temporarily precomputed points */
  num_val = 0;
  for (i = 0; i < num; i++) {
    size_t bits;
    bits = tcsm_tc_bn_num_bits(scalars[i]);
    wsize[i] = EC_window_bits_for_scalar_size(bits);
    num_val += (size_t)1 << (wsize[i] - 1);
    wNAF[i + 1] = NULL; /* make sure we always have a pivot */
    wNAF[i] =
    tc_compute_wnaf((scalars[i]), (int)wsize[i], &wNAF_len[i]);
    if (wNAF[i] == NULL)
      goto err;
    if (wNAF_len[i] > max_len)
      max_len = wNAF_len[i];
  }
  
  /* All points we precompute now go into a single array 'val'.
   * 'val_sub[i]' is a pointer to the subarray for the i-th point,
   * or to a subarray of 'pre_comp->points' if we already have precomputation.
   */
  val = tcsm_tc_secure_malloc((num_val + 1) * sizeof val[0]);
  if (val == NULL) {
    goto err;
  }
  val[num_val] = NULL; /* pivot element */
  /* allocate points for precomputation */
  v = val;
  
  for (i = 0; i < num + num_scalar; i++) {
    val_sub[i] = v;
    for (j = 0; j < ((size_t)1 << (wsize[i] - 1)); j++) {
      
      tc_ec_jcb_t* tc_ec_t_v_ptr = tcsm_tc_secure_malloc(sizeof(tc_ec_jcb_t));
      tcsm_tc_ec_jcb_init(*tc_ec_t_v_ptr);
      *v = *tc_ec_t_v_ptr;
      if (*v == NULL)
        goto err;
      v++;
    }
  }
  if (!(v == val + num_val)) {
    goto err;
  }
  /* prepare precomputed values:
   *    val_sub[i][0] :=     points[i]
   *    val_sub[i][1] := 3 * points[i]
   *    val_sub[i][2] := 5 * points[i]
   *    ...
   */
  for (i = 0; i < num + num_scalar; i++) {
    if (i < num) {
      tcsm_tc_ec_jcb_cpy(val_sub[i][0], points[i]);
    } else {
      tcsm_tc_ec_jcb_cpy(val_sub[i][0], group->ctx->jcb_generator);
    }
    if (wsize[i] > 1) {
      tcsm_tc_ec_jcb_double(group, tmp, val_sub[i][0]);
      for (j = 1; j < ((size_t)1 << (wsize[i] - 1)); j++) {
        tcsm_tc_ec_jcb_add(group, val_sub[i][j], val_sub[i][j - 1], tmp);
      }
    }
  }
  
  r_is_at_infinity = 1;
  for (k = (int)max_len - 1; k >= 0; k--) {
    if (!r_is_at_infinity) {
      tcsm_tc_ec_jcb_double(group, r, r);
    }
    for (i = 0; i < totalnum; i++) {
      if (wNAF_len[i] > (size_t)k) {
        int digit = wNAF[i][k];
        int is_neg;
        if (digit) {
          is_neg = digit < 0;
          if (is_neg)
            digit = -digit;
          if (is_neg != r_is_inverted) {
            if (!r_is_at_infinity) {
              tcsm_tc_ec_jcb_invert(group, r, r);
            }
            r_is_inverted = !r_is_inverted;
          }
          /* digit > 0 */
          if (r_is_at_infinity) {
            tcsm_tc_ec_jcb_cpy(r, val_sub[i][digit >> 1]);
            r_is_at_infinity = 0;
          } else {
            tcsm_tc_ec_jcb_add(group, group->ctx->jcb_compute_var, r, val_sub[i][digit >> 1]);
            tcsm_tc_ec_jcb_cpy(r, group->ctx->jcb_compute_var);
          }
        }
      }
    }
  }
  if (r_is_at_infinity) {
    tcsm_tc_ec_jcb_set_str(r, "0", 10, "0", 10,"10",10);
  } else {
    if (r_is_inverted)
      tcsm_tc_ec_jcb_invert(group, r, r);
  }
  ret = ERR_TENCENTSM_OK;
err:
  tcsm_tc_ec_jcb_clear(tmp);
  if (wsize != NULL)
    tcsm_tc_secure_free(wsize);
  if (wNAF_len != NULL)
    tcsm_tc_secure_free(wNAF_len);
  if (wNAF != NULL) {
    signed char **w;
    for (w = wNAF; *w != NULL; w++)
      tcsm_tc_secure_free(*w);
    tcsm_tc_secure_free(wNAF);
  }
  if (val != NULL) {
    for (v = val; *v != NULL; v++)
    {
      tcsm_tc_ec_jcb_clear(*v);
      tcsm_tc_secure_free(*v);
    }
    tcsm_tc_secure_free(val);
  }
  if (val_sub != NULL) {
    tcsm_tc_secure_free(val_sub);
  }
  return ret;
}

int tcsm_ec_mul_for_G(tc_ec_group_t group, tc_ec_t r, tc_bn_t k)
{
#ifdef _OPT_ASM_ECC
  return ecp_sm2z256_points_mul_for_generator(group, r, k);
#else
  tc_ec_t* curve_generator = group->ctx->generator;
  return tc_ec_wnaf_splitting_mul(group, r, k, 0, NULL, NULL,tcsm_ec_get_pre_comp_info(group),*curve_generator);
#endif
}

int tcsm_ec_mul_for_pubkey(tc_ec_group_t group, tc_ec_t r, tc_ec_t p,tc_bn_t k)
{
#ifdef _OPT_ASM_ECC
  return ecp_sm2z256_points_mul_for_pubkey(group, r, k);
#else
  return tc_ec_wnaf_splitting_mul(group, r, k, 0, NULL, NULL,tcsm_ec_get_pre_comp_pubkey_info(group),p);
#endif
}

int tcsm_ec_mul_for_point(tc_ec_group_t group, tc_ec_t r, tc_ec_t p, tc_bn_t k)
{
#ifdef _OPT_ASM_ECC
  P256_POINT point;
  for (int j = 0; j < P256_LIMBS; j++) {
    point.X[j] = p->x->val->_mp_d[j];
    point.Y[j] = p->y->val->_mp_d[j];
  }
  
  point.Z[0] = 1;
  point.Z[1] = 0;
  point.Z[2] = 0;
  point.Z[3] = 0;
  return ecp_sm2z256_points_mul_for_point(group, r, &point, k);
#else
  tc_ec_jcb_t r_jcb;
  tc_ec_jcb_t p_jcb;
  tcsm_tc_ec_jcb_init(r_jcb);
  tcsm_tc_ec_jcb_init(p_jcb);
  
  tcsm_tc_bn_cpy(p_jcb->x,p->x);
  tcsm_tc_bn_cpy(p_jcb->y,p->y);
  tcsm_tc_bn_set_str(p_jcb->z, "1", 10);

  tc_ecc_jcb_point_st *jcb_points[1];
  tc_bn_st * jcb_scalars[1];

  jcb_points[0] = p_jcb;
  jcb_scalars[0] = k;

  int ret = tc_ec_jcb_wnaf_splitting_mul(group, r_jcb, 1, jcb_points, jcb_scalars);
  tcsm_tc_ec_jcb_to_afn(group,r, r_jcb, group);
  
  tcsm_tc_ec_jcb_clear(r_jcb);
  tcsm_tc_ec_jcb_clear(p_jcb);
  return ret;
#endif
}

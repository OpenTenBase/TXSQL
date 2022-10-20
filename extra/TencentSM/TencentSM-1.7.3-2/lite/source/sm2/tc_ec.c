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

#include "../include/tc.h"
#include "../include/sm.h"
#include "../include/tc_naf.h"
#include "../include/tc_ec_mul.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#include <pthread.h>

void tcsm_init_calculate_context(sm2_ctx_t *ctx)
{
  tc_bn_t* bns = ctx->bn_vars;
  tc_ec_t* ecs = ctx->ec_vars;
  
  for (int i = 0; i < MAX_BN; i++) {
    tcsm_tc_bn_init(bns[i]);
  }
  
  for (int i = 0; i < MAX_EC; i++) {
    tcsm_tc_ec_init(ecs[i]);
  }
  
  tcsm_tc_ec_jcb_init(ctx->jcb_compute_var);
}

void tcsm_destroy_calculate_context(sm2_ctx_t *ctx)
{
  tc_bn_t* bns = ctx->bn_vars;
  tc_ec_t* ecs = ctx->ec_vars;
  
  for (int i = 0; i < MAX_BN; i++) {
    tcsm_tc_bn_clear(bns[i]);
  }
  
  for (int i = 0; i < MAX_EC; i++) {
    tcsm_tc_ec_clear(ecs[i]);
  }

  tcsm_tc_ec_jcb_clear(ctx->jcb_compute_var);
}

tc_bn_t* tcsm_lock_temp_bn(sm2_ctx_t *ctx,int* index)
{
  tc_bn_t* temp_bn = ctx->bn_vars;
  
  for (int i = 0; i < MAX_BN; i++) {
    if (temp_bn[i]->b_using == 0) {
      temp_bn[i]->b_using = 1;
      *index = i;
      return &(temp_bn[i]);
    }
  }
  LOGV("[ERROR] temp bn is not enough...\n");
  return &(temp_bn[MAX_BN-1]);
}

void tcsm_unlock_temp_bn(sm2_ctx_t *ctx,int index)
{
  tc_bn_t* temp_bn = ctx->bn_vars;
  temp_bn[index]->b_using = 0;
#ifdef _MEMORY_ERASE_PROTECTION
  tcsm_tc_bn_set_str(temp_bn[index], "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16);
#endif
}

tc_ec_t* tcsm_lock_temp_ec(sm2_ctx_t *ctx,int* index)
{
  tc_ec_t* temp_ec = ctx->ec_vars;
  for (int i = 0; i < MAX_EC; i++) {
    if (temp_ec[i]->b_using == 0) {
      temp_ec[i]->b_using = 1;
      *index = i;
      return &(temp_ec[i]);
    }
  }
  LOGV("[ERROR] temp ec is not enough...\n");
  return &(temp_ec[MAX_EC-1]);
}

void tcsm_unlock_temp_ec(sm2_ctx_t *ctx,int index)
{
  tc_ec_t* temp_ec = ctx->ec_vars;
  temp_ec[index]->b_using = 0;
#ifdef _MEMORY_ERASE_PROTECTION
  tcsm_tc_bn_set_str(temp_ec[index]->x, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16);
  tcsm_tc_bn_set_str(temp_ec[index]->y, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16);
#endif
}

int get_slope_equal(tc_ec_group_t group, tc_bn_t slope, tc_ec_t p)
{
  int ret = ERR_TENCENTSM_OK;
  
  /* slope = (3x^2 + a) / (2y) */
  
  sm2_ctx_t* ctx = group->ctx;
  
  int rop_index = 0;
  tc_bn_t* rop = tcsm_lock_temp_bn(ctx,&rop_index);

  /* 1. x^2 */
  tcsm_tc_bn_pow_ui(*rop, p->x, 2);
  
  /* 2. 3x^2 */
  tcsm_tc_bn_mul_ui(*rop, *rop, 3);
  
  /* 3. 3x^2 + a */
  tcsm_tc_bn_add(*rop, *rop, group->a);
  //tcsm_tc_bn_sub(rop, rop, group->p);
  
  int y2_index = 0;
  tc_bn_t* y2 = tcsm_lock_temp_bn(ctx,&y2_index);
  /* 5. 2y */
  tcsm_tc_bn_mul_ui(*y2, p->y, 2);
  
  int y2_invert_index = 0;
  tc_bn_t* y2_invert = tcsm_lock_temp_bn(ctx,&y2_invert_index);
  
  /* 6. (2y)^-1 */
  if ((ret = tcsm_tc_bn_invert(*y2_invert, *y2, group->p)) < ERR_TENCENTSM_OK)
    goto out;
  
  /* 7. (3x^2 + a) * (2y)^-1 */
  tcsm_tc_bn_mul(slope, *rop, *y2_invert);
  tcsm_tc_bn_mod(slope, slope, group->p);
  
out:
  tcsm_unlock_temp_bn(ctx,rop_index);
  tcsm_unlock_temp_bn(ctx,y2_index);
  tcsm_unlock_temp_bn(ctx,y2_invert_index);
  return ret;
}


int get_slope_other(tc_ec_group_t group, tc_bn_t slope, tc_ec_t p, tc_ec_t q)
{
  int ret = ERR_TENCENTSM_OK;
  /* slope = (Yq - Yp) / (Xq - Xp) */
  
  sm2_ctx_t* ctx = group->ctx;
  
  /* 1. a = Yq - Yp*/
  
  int a_index = 0;
  tc_bn_t * a = tcsm_lock_temp_bn(ctx,&a_index);
  
  tcsm_tc_bn_sub(*a, q->y, p->y);
  
  int b_index = 0;
  tc_bn_t * b = tcsm_lock_temp_bn(ctx,&b_index);
  
  /* 2. b = Xq - Xp*/
  tcsm_tc_bn_sub(*b, q->x, p->x);
  
  int b_1_index = 0;
  tc_bn_t * b_1 = tcsm_lock_temp_bn(ctx,&b_1_index);

  /* 3. b^-1 */
  if ((ret = tcsm_tc_bn_invert(*b_1, *b, group->p) < ERR_TENCENTSM_OK))
    goto out;

  /* 4. slope = a * b^-1 */
  tcsm_tc_bn_mul(slope, *a, *b_1);
  tcsm_tc_bn_mod(slope, slope, group->p);
  
out:
  tcsm_unlock_temp_bn(ctx,b_1_index);
  tcsm_unlock_temp_bn(ctx,b_index);
  tcsm_unlock_temp_bn(ctx,a_index);
  return ret;
}

void calculate_Xr(tc_ec_group_t group, tc_ec_t r, tc_bn_t slope, tc_ec_t p, tc_ec_t q)
{
  sm2_ctx_t* ctx = group->ctx;
  /* Xr = slope^2 - Xp - Xq */
  /* 1. slope^2 */
  int Xr_index = 0;
  tc_bn_t * Xr = tcsm_lock_temp_bn(ctx,&Xr_index);
  
  tcsm_tc_bn_pow_ui(*Xr, slope, 2);
  
  /* 2. slope^2 - Xp*/
  tcsm_tc_bn_sub(r->x, *Xr, p->x);
  
  tcsm_unlock_temp_bn(ctx,Xr_index);
  
  /* 2. slope^2 - Xp - Xq*/
  tcsm_tc_bn_sub(r->x, r->x, q->x);
  tcsm_tc_bn_mod(r->x, r->x, group->p);

  return;
}

void calculate_Yr(tc_ec_group_t group, tc_ec_t r, tc_bn_t slope, tc_ec_t p, tc_ec_t q)
{
  /* Yr = slope*(Xp - Rx) - Yp */
  
  /* 1. Xp - Rx */
  tcsm_tc_bn_sub(r->y, p->x, r->x);
  
  /* 2. slope*(Xp - Rx) */
  tcsm_tc_bn_mul(r->y, r->y, slope);
  
  /* 3. slope*(Xp - Rx) - Yp*/
  tcsm_tc_bn_sub(r->y, r->y, p->y);
  tcsm_tc_bn_mod(r->y, r->y, group->p);
  return;
}

void tcsm_tc_ec_init(tc_ec_t p)
{
  tcsm_tc_bn_init(p->x);
  tcsm_tc_bn_init(p->y);
  p->b_using = 0;
  return;
}

void tcsm_tc_ec_jcb_init(tc_ec_jcb_t p)
{
  tcsm_tc_bn_init(p->x);
  tcsm_tc_bn_init(p->y);
  tcsm_tc_bn_init(p->z);
  return;
}

void tcsm_tc_ec_init_generator(tc_ec_group_t group, tc_ec_t p)
{
  tcsm_tc_bn_init(p->x);
  tcsm_tc_bn_init(p->y);
  tcsm_tc_bn_cpy(p->x, group->Gx);
  tcsm_tc_bn_cpy(p->y, group->Gy);
  p->b_using = 0;
  return;
}

void tcsm_tc_ec_jcb_init_generator(tc_ec_group_t group, tc_ec_jcb_t p)
{
  tcsm_tc_bn_init(p->x);
  tcsm_tc_bn_init(p->y);
  tcsm_tc_bn_init(p->z);
  tcsm_tc_bn_cpy(p->x, group->Gx);
  tcsm_tc_bn_cpy(p->y, group->Gy);
  tcsm_tc_bn_set_str(p->z, "1", 16);
  return;
}

void tcsm_tc_ec_clear(tc_ec_t p)
{
  tcsm_tc_bn_clear(p->x);
  tcsm_tc_bn_clear(p->y);
  return;
}

void tcsm_tc_ec_jcb_clear(tc_ec_jcb_t p)
{
  tcsm_tc_bn_clear(p->x);
  tcsm_tc_bn_clear(p->y);
  tcsm_tc_bn_clear(p->z);
  return;
}

void tcsm_tc_ec_cpy(tc_ec_t dst, tc_ec_t src)
{
  tcsm_tc_bn_cpy(dst->x, src->x);
  tcsm_tc_bn_cpy(dst->y, src->y);
  return;
}

void tcsm_tc_ec_jcb_cpy(tc_ec_jcb_t dst, tc_ec_jcb_t src)
{
  tcsm_tc_bn_cpy(dst->x, src->x);
  tcsm_tc_bn_cpy(dst->y, src->y);
  tcsm_tc_bn_cpy(dst->z, src->z);
  return;
}

int tcsm_tc_ec_cmp(tc_ec_t op1, tc_ec_t op2)
{
  return mpz_cmp(op1->x->val, op2->x->val) && mpz_cmp(op1->y->val, op2->y->val);
}

int tcsm_tc_ec_jcb_cmp(tc_ec_jcb_t op1, tc_ec_jcb_t op2)
{
  return mpz_cmp(op1->x->val, op2->x->val) && mpz_cmp(op1->y->val, op2->y->val) && mpz_cmp(op1->z->val, op2->z->val);
}

void tcsm_tc_ec_set_tcbn(tc_ec_t rop, tc_bn_t x, tc_bn_t y)
{
  tcsm_tc_bn_cpy(rop->x, x);
  tcsm_tc_bn_cpy(rop->y, y);
}

int tcsm_tc_ec_set_bin(tc_ec_t p, char *x, int xlen, char *y, int ylen)
{
  tcsm_tc_bn_set_bin(p->x, x, xlen);
  return tcsm_tc_bn_set_bin(p->y, y, ylen);
}

int tcsm_tc_ec_set_str(tc_ec_t p, char *x, int xredix, char *y, int yredix)
{
  tcsm_tc_bn_set_str(p->x, x, xredix);
  return tcsm_tc_bn_set_str(p->y, y, yredix);
}

int tcsm_tc_ec_jcb_set_str(tc_ec_jcb_t p, char *x, int xredix, char *y, int yredix,char *z, int zredix)
{
  tcsm_tc_bn_set_str(p->x, x, xredix);
  tcsm_tc_bn_set_str(p->y, y, yredix);
  return tcsm_tc_bn_set_str(p->z, z, zredix);
}

void tcsm_tc_ec_get_tcbn(tc_bn_t x, tc_bn_t y, tc_ec_t p)
{
  tcsm_tc_bn_cpy(x, p->x);
  tcsm_tc_bn_cpy(y, p->y);
  return;
}

void tcsm_tc_ec_get_bin(unsigned char *x, int *xlen, unsigned char *y, int *ylen, tc_ec_t p, unsigned int n)
{
  tcsm_tc_bn_get_bin(x, (unsigned int *)xlen, p->x, n);
  tcsm_tc_bn_get_bin(y, (unsigned int *)ylen, p->y, n);
}

void tcsm_tc_ec_get_str(char *x, char *y, tc_ec_t p)
{
  if (x == NULL)
    x = tcsm_tc_bn_get_str(x, p->x);
  else
    tcsm_tc_bn_get_str(x, p->x);
  
  if (y == NULL)
    y = tcsm_tc_bn_get_str(y, p->y);
  else
    tcsm_tc_bn_get_str(y, p->y);
  
}

// void tc_ec_create_group(tc_ec_group_t group, tc_bn_t p, tc_bn_t a, tc_bn_t b, tc_bn_t Gx, tc_bn_t Gy, tc_bn_t n, tc_bn_t h)
// {
//   tcsm_tc_bn_init(group->Gx);
//   tcsm_tc_bn_init(group->Gy);
//   tcsm_tc_bn_init(group->a);
//   tcsm_tc_bn_init(group->b);
//   tcsm_tc_bn_init(group->n);
//   tcsm_tc_bn_init(group->p);
//   tcsm_tc_bn_init(group->h);
  
  
//   tcsm_tc_bn_cpy(group->Gx, Gx);
//   tcsm_tc_bn_cpy(group->Gy, Gy);
//   tcsm_tc_bn_cpy(group->a, a);
//   tcsm_tc_bn_cpy(group->b, b);
//   tcsm_tc_bn_cpy(group->n, n);
//   tcsm_tc_bn_cpy(group->p, p);
//   tcsm_tc_bn_cpy(group->h, h);
//   return;
// }

void tcsm_tc_ec_clear_group(tc_ec_group_t group)
{
  tcsm_tc_bn_clear(group->Gx);
  tcsm_tc_bn_clear(group->Gy);
  tcsm_tc_bn_clear(group->a);
  tcsm_tc_bn_clear(group->b);
  tcsm_tc_bn_clear(group->n);
  tcsm_tc_bn_clear(group->p);
  tcsm_tc_bn_clear(group->h);
  return;
}

int tcsm_tc_ec_invert(tc_ec_group_t group, tc_ec_t r, tc_ec_t p)
{
  tcsm_tc_bn_cpy(r->x, p->x);
  tcsm_tc_bn_sub(r->y, group->p, p->y);
  return 0;
}

int tcsm_tc_ec_jcb_invert(tc_ec_group_t group, tc_ec_jcb_t r, tc_ec_jcb_t p)
{
  tcsm_tc_bn_cpy(r->x, p->x);
  tcsm_tc_bn_cpy(r->z, p->z);
  tcsm_tc_bn_sub(r->y, group->p, p->y);
  return 0;
}

void fp_add_mpz(mpz_ptr rop, mpz_ptr u, mpz_ptr v, mpz_ptr p)
{
  mpz_add(rop, u, v);
}

void fp_sub_mpz(mpz_ptr rop, mpz_ptr u, mpz_ptr v, mpz_ptr p)
{
  mpz_sub(rop, u, v);
}

void fp_mul_mpz(mpz_ptr rop, mpz_ptr u, mpz_ptr v, mpz_ptr p)
{
  mpz_mul(rop, u, v);
  mpz_mod(rop, rop, p);
}

void fp_pow_si(mpz_ptr rop, mpz_ptr u, long v, mpz_ptr p)
{
  mpz_powm_ui(rop, u, v, p);
}

void fp_mul_si(mpz_ptr rop, mpz_ptr u, long v, mpz_ptr p)
{
  mpz_mul_si(rop, u, v);
}

void tcsm_tc_ec_jcb_to_afn(tc_ec_group_t group,tc_ec_t ROP, tc_ec_jcb_t P, tc_ec_group_t curve)
{
  int g_index,s_index,inv_index,tmp_index = 0;
  tc_bn_t* g = tcsm_lock_temp_bn(group->ctx,&g_index);
  tc_bn_t* s = tcsm_lock_temp_bn(group->ctx,&s_index);
  tc_bn_t* inv = tcsm_lock_temp_bn(group->ctx,&inv_index);
  tc_bn_t* tmp = tcsm_lock_temp_bn(group->ctx,&tmp_index);
  
  
  mpz_gcdext((*g)->val, (*s)->val, (*inv)->val, curve->p->val, P->z->val); // inv = Z^1
  fp_mul_mpz((*tmp)->val, (*inv)->val, (*inv)->val, curve->p->val); // (Z^1)^2
  fp_mul_mpz(ROP->x->val, P->x->val, (*tmp)->val, curve->p->val); // X*((Z^1)^2)
  fp_mul_mpz((*tmp)->val, (*tmp)->val, (*inv)->val, curve->p->val); // (Z^1)^3
  fp_mul_mpz(ROP->y->val, P->y->val, (*tmp)->val, curve->p->val); // Y*((Z^1)^3)
  
  tcsm_unlock_temp_bn(group->ctx,g_index);
  tcsm_unlock_temp_bn(group->ctx,s_index);
  tcsm_unlock_temp_bn(group->ctx,inv_index);
  tcsm_unlock_temp_bn(group->ctx,tmp_index);
}

int tcsm_tc_ec_add(tc_ec_group_t group, tc_ec_t r, tc_ec_t p, tc_ec_t q)
{
  int ret = ERR_TENCENTSM_OK;
  
  sm2_ctx_t* ctx = group->ctx;
  
  int cal_sum_index = 0;
  tc_ec_t* cal_sum = tcsm_lock_temp_ec(ctx,&cal_sum_index);
  
  int zero_index = 0;
  tc_bn_t* zero = tcsm_lock_temp_bn(ctx,&zero_index);
  tcsm_tc_bn_set_str(*zero, "0", 10);
  
  int Yq_index = 0;
  tc_bn_t* Yq = tcsm_lock_temp_bn(ctx,&Yq_index);

  tcsm_tc_bn_modsub(*Yq, *zero, q->y, group->p);
  
  if (!tcsm_tc_bn_cmp(p->x, *zero) && !tcsm_tc_bn_cmp(p->y, *zero))
  {
    tcsm_tc_bn_cpy((*cal_sum)->x, q->x);
    tcsm_tc_bn_cpy((*cal_sum)->y, q->y);
    goto out;
  }
  else if (!tcsm_tc_bn_cmp(q->x, *zero) && !tcsm_tc_bn_cmp(q->y, *zero))
  {
    tcsm_tc_bn_cpy((*cal_sum)->x, p->x);
    tcsm_tc_bn_cpy((*cal_sum)->y, p->y);
    goto out;
  }
  else if((!tcsm_tc_bn_cmp(p->x, q->x)) && (!tcsm_tc_bn_cmp(p->y, *Yq)))
  {
    tcsm_tc_bn_cpy((*cal_sum)->x, *zero);
    tcsm_tc_bn_cpy((*cal_sum)->y, *zero);
    goto out;
  }
  else
  {
    int slope_index = 0;
    tc_bn_t* slope = tcsm_lock_temp_bn(ctx,&slope_index);
    
    if ((!tcsm_tc_bn_cmp(p->x, q->x)) && (!tcsm_tc_bn_cmp(p->y, q->y)))
    {
      /* 4.1.1 slope = (3x^2 + a) / (2y) */
      if ((ret = get_slope_equal(group, *slope, p)) < ERR_TENCENTSM_OK)
        goto errout;
    }
    else
    {
      /* 4.1.2 slope = (Yq - Yp) / (Xq - Xp) */
      if ((ret = get_slope_other(group, *slope, p, q)) < ERR_TENCENTSM_OK)
        goto errout;
    }    /* -------- end else -------- */
    
    /* 4.3 Xr = slope^2 - Xp - Xq */
    calculate_Xr(group, (*cal_sum), *slope, p, q);
    
    /* 4.4 Yr = slope*(Xp - Rx) - Yp */
    calculate_Yr(group, (*cal_sum), *slope, p, q);
    tcsm_unlock_temp_bn(ctx,slope_index);
    
  }
out:
  tcsm_tc_ec_cpy(r, (*cal_sum));
  
  tcsm_unlock_temp_bn(ctx,zero_index);
  tcsm_unlock_temp_ec(ctx,cal_sum_index);
  tcsm_unlock_temp_bn(ctx,Yq_index);

errout:
  return ret;
}

int tcsm_tc_ec_jcb_add(tc_ec_group_t group, tc_ec_jcb_t ROP, tc_ec_jcb_t P, tc_ec_jcb_t Q)
{
  if((mpz_cmp(P->x->val, Q->x->val) == 0) && (mpz_cmp(P->y->val, Q->y->val) == 0) && (mpz_cmp(P->z->val, Q->z->val) == 0))
  {
    tcsm_tc_ec_jcb_double(group, ROP, P);
  }
  else if((mpz_sgn(P->x->val) == 0) && (mpz_sgn(P->y->val) == 0) && (mpz_sgn(P->z->val) == 0))
  {
    tcsm_tc_ec_jcb_cpy(ROP, Q);
  }
  else if((mpz_sgn(Q->x->val) == 0) && (mpz_sgn(Q->y->val) == 0) && (mpz_sgn(Q->z->val) == 0))
  {
    tcsm_tc_ec_jcb_cpy(ROP, P);
  }
  else
  {
    int alfa_index,Z2_pow2_index,beta_pow2_index,beta_index,tmp1_index,tmp2_index = 0;
    tc_bn_t* alpha = tcsm_lock_temp_bn(group->ctx,&alfa_index);
    tc_bn_t* Z2_pow2 = tcsm_lock_temp_bn(group->ctx,&Z2_pow2_index);
    tc_bn_t* beta_pow2 = tcsm_lock_temp_bn(group->ctx,&beta_pow2_index);
    tc_bn_t* beta = tcsm_lock_temp_bn(group->ctx,&beta_index);
    tc_bn_t* tmp1 = tcsm_lock_temp_bn(group->ctx,&tmp1_index);
    tc_bn_t* tmp2 = tcsm_lock_temp_bn(group->ctx,&tmp2_index);
    
    
    /* Z2^2 */
    mpz_powm_ui((*Z2_pow2)->val, Q->z->val, 2,  group->p->val);
//    fp_mul_mpz((*Z2_pow2)->val, Q->z->val, Q->z->val,  group->p->val); // Z1^2
    
    /* alpha = Z1^3*Y2 - Z2^3*Y1 */
    mpz_powm_ui((*alpha)->val, P->z->val, 3,  group->p->val);
    fp_mul_mpz((*alpha)->val, (*alpha)->val, Q->y->val,  group->p->val);
    mpz_powm_ui((*tmp1)->val, Q->z->val, 3,  group->p->val);
//    fp_mul_mpz((*tmp1)->val, (*tmp1)->val, P->y->val,  group->p->val);
//    mpz_sub((*alpha)->val, (*alpha)->val, (*tmp1)->val);
    
    mpz_submul((*alpha)->val,(*tmp1)->val,P->y->val);
    mpz_mod((*alpha)->val,(*alpha)->val,group->p->val);
    
    /* beta = Z1^2*X2 - Z2^2*X1 */
    mpz_powm_ui((*beta)->val, P->z->val, 2,  group->p->val);
    fp_mul_mpz((*beta)->val, (*beta)->val, Q->x->val,  group->p->val);
    mpz_powm_ui((*tmp1)->val, Q->z->val, 2,  group->p->val);
//    fp_mul_mpz((*tmp1)->val, (*tmp1)->val, P->x->val,  group->p->val);
//    mpz_sub((*beta)->val, (*beta)->val, (*tmp1)->val);
    
    mpz_submul((*beta)->val,(*tmp1)->val,P->x->val);
    mpz_mod((*beta)->val,(*beta)->val,group->p->val);
    
    /* beta^2 */
    mpz_powm_ui((*beta_pow2)->val, (*beta)->val, 2,  group->p->val);
    
    /* X3 = alpha^2 - beta^3 - 2*Z2^2*X1*beta^2 */
    mpz_powm_ui((*tmp1)->val, (*alpha)->val, 2,  group->p->val);
    mpz_powm_ui((*tmp2)->val, (*beta)->val, 3,  group->p->val);
    mpz_sub((*tmp1)->val, (*tmp1)->val, (*tmp2)->val);
    mpz_mul_ui((*tmp2)->val, (*Z2_pow2)->val, 2);
    fp_mul_mpz((*tmp2)->val, (*tmp2)->val, P->x->val,  group->p->val);
    fp_mul_mpz((*tmp2)->val, (*tmp2)->val, (*beta_pow2)->val,  group->p->val);
    mpz_sub(ROP->x->val, (*tmp1)->val, (*tmp2)->val);
    
    /* Y3 = alpha*(Z2^2*X1*beta^2 - X3) - Z2^3*Y1*beta^3*/
    fp_mul_mpz((*tmp1)->val, (*Z2_pow2)->val, P->x->val,  group->p->val);
    fp_mul_mpz((*tmp1)->val, (*tmp1)->val, (*beta_pow2)->val,  group->p->val);
    mpz_sub((*tmp1)->val, (*tmp1)->val, ROP->x->val);
    
    fp_mul_mpz((*tmp1)->val, (*tmp1)->val, (*alpha)->val,  group->p->val);
    fp_mul_mpz((*tmp2)->val, (*Z2_pow2)->val, Q->z->val,  group->p->val);
    fp_mul_mpz((*tmp2)->val, (*tmp2)->val, P->y->val,  group->p->val);
    fp_mul_mpz((*tmp2)->val, (*tmp2)->val, (*beta_pow2)->val,  group->p->val);
    fp_mul_mpz((*tmp2)->val, (*tmp2)->val, (*beta)->val,  group->p->val);
    mpz_sub(ROP->y->val, (*tmp1)->val, (*tmp2)->val);
    
    /* Z3 = Z1*Z2*beta */
    fp_mul_mpz((*tmp1)->val, P->z->val, Q->z->val,  group->p->val);
    fp_mul_mpz(ROP->z->val, (*tmp1)->val, (*beta)->val,  group->p->val);
    
    tcsm_unlock_temp_bn(group->ctx,alfa_index);
    tcsm_unlock_temp_bn(group->ctx,Z2_pow2_index);
    tcsm_unlock_temp_bn(group->ctx,beta_pow2_index);
    tcsm_unlock_temp_bn(group->ctx,beta_index);
    tcsm_unlock_temp_bn(group->ctx,tmp1_index);
    tcsm_unlock_temp_bn(group->ctx,tmp2_index);
  }
  return 0;
}

void tcsm_tc_ec_add_inv(tc_ec_group_t group, tc_ec_t r, tc_ec_t p){
  tcsm_tc_bn_cpy(r->x, p->x);
  mpz_mul_si(r->y->val, p->y->val, -1);
  mpz_mod(r->y->val, r->y->val, group->p->val);
}

void tcsm_tc_ec_jcb_add_inv(tc_ec_group_t group, tc_ec_jcb_t r, tc_ec_jcb_t p){
  tcsm_tc_bn_cpy(r->x, p->x);
  tcsm_tc_bn_cpy(r->z, p->z);
  mpz_mul_si(r->y->val, p->y->val, -1);
  mpz_mod(r->y->val, r->y->val, group->p->val);
}

int tcsm_tc_ec_double(tc_ec_group_t group, tc_ec_t r, tc_ec_t p)
{
  int ret = 0;
  
  sm2_ctx_t* ctx = group->ctx;
  
  int cal_sum_index = 0;
  tc_ec_t* cal_sum = tcsm_lock_temp_ec(ctx,&cal_sum_index);
  
  int zero_index = 0;
  tc_bn_t* zero = tcsm_lock_temp_bn(ctx,&zero_index);
  tcsm_tc_bn_set_str(*zero, "0", 10);

  if (!tcsm_tc_bn_cmp(p->x, *zero) && !tcsm_tc_bn_cmp(p->y, *zero))
  {
    tcsm_tc_bn_cpy((*cal_sum)->x, p->x);
    tcsm_tc_bn_cpy((*cal_sum)->y, p->y);
    goto out;
  }
  else
  {
    int slope_index = 0;
    tc_bn_t* slope = tcsm_lock_temp_bn(ctx,&slope_index);
    
    if ((ret = get_slope_equal(group, *slope, p)) < 0)
      goto errout;
    
    /* 4.3 Xr = slope^2 - Xp - Xq */
    mpz_pow_ui((*cal_sum)->x->val, (*slope)->val, 2);
    mpz_submul_ui((*cal_sum)->x->val,p->x->val,2);
    mpz_mod((*cal_sum)->x->val, (*cal_sum)->x->val, group->p->val);
    
    /* 4.4 Yr = slope*(Xp - Rx) - Yp */
    mpz_sub((*cal_sum)->y->val, p->x->val, (*cal_sum)->x->val);
    mpz_mul((*cal_sum)->y->val, (*cal_sum)->y->val, (*slope)->val);
    
    mpz_sub((*cal_sum)->y->val, (*cal_sum)->y->val, p->y->val);
    mpz_mod((*cal_sum)->y->val, (*cal_sum)->y->val, group->p->val);
    
    tcsm_unlock_temp_bn(ctx,slope_index);
  }
out:
  tcsm_tc_ec_cpy(r, (*cal_sum));
errout:
  tcsm_unlock_temp_bn(ctx,zero_index);
  tcsm_unlock_temp_ec(ctx,cal_sum_index);
  return ret;
}

int tcsm_tc_ec_jcb_double(tc_ec_group_t group, tc_ec_jcb_t ROP, tc_ec_jcb_t P)
{
//  if((strcmp(mpz_get_str(NULL, 10,P->x->val), "0") == 0) &&
//     (strcmp(mpz_get_str(NULL, 10, P->y->val), "0") == 0) &&
//     (strcmp(mpz_get_str(NULL, 10, P->z->val), "0") == 0))
//  {
//    tcsm_tc_ec_jcb_cpy(ROP, P);
//  }
//  else
//  {
    int alfa1_index,alfa2_index,y2_index,beta_index,y3_2_index = 0;
    tc_bn_t* alfa1 = tcsm_lock_temp_bn(group->ctx,&alfa1_index);
    tc_bn_t* alfa2 = tcsm_lock_temp_bn(group->ctx,&alfa2_index);
    tc_bn_t* y2 = tcsm_lock_temp_bn(group->ctx,&y2_index);
    tc_bn_t* beta = tcsm_lock_temp_bn(group->ctx,&beta_index);
    tc_bn_t* y3_2 = tcsm_lock_temp_bn(group->ctx,&y3_2_index);
  
    // alfa = 3*(x1)**2 + a * (z1**4)
    mpz_powm_ui((*alfa1)->val, P->x->val, 2,group->p->val);
    mpz_powm_ui((*alfa2)->val, P->z->val, 2,group->p->val);
  
    mpz_powm_ui((*alfa2)->val, (*alfa2)->val,2, group->p->val);
  
    fp_mul_mpz((*alfa2)->val, (*alfa2)->val, group->a->val, group->p->val);
  
    mpz_addmul_ui((*alfa2)->val,(*alfa1)->val,3);
    
    // y2 = y1**2
    mpz_powm_ui((*y2)->val, P->y->val, 2,group->p->val);
  
    mpz_mul_ui((*beta)->val,(*y2)->val,4);
    mpz_mul((*beta)->val,(*beta)->val,P->x->val);
    mpz_mod((*beta)->val,(*beta)->val,group->p->val);
  
      // z3 = y1*z1
      // z3 = 2*y1*z1

    mpz_mul_ui(ROP->z->val,P->z->val,2);
    mpz_mul(ROP->z->val,ROP->z->val, P->y->val);
    mpz_mod(ROP->z->val,ROP->z->val,group->p->val);

    // x3_2 = 2*beta
    
    // x3 = alfa**2 - 2*beta
    mpz_powm_ui(ROP->x->val, (*alfa2)->val, 2,group->p->val);
  
    mpz_submul_ui(ROP->x->val,(*beta)->val,2);
  
    mpz_mod(ROP->x->val,ROP->x->val,group->p->val);
  
    // y3 = alfa(beta -x3)
    mpz_sub(ROP->y->val, (*beta)->val, ROP->x->val);
    fp_mul_mpz(ROP->y->val, ROP->y->val, (*alfa2)->val, group->p->val);
  
    
    // y3_2 = 8*y1**4
    mpz_powm_ui((*y3_2)->val, (*y2)->val, 2,group->p->val);
  
    // y3 = alfa(beta -x3) - 8*y1**4
    mpz_submul_ui(ROP->y->val,(*y3_2)->val,8);
  
  tcsm_unlock_temp_bn(group->ctx,alfa1_index);
  tcsm_unlock_temp_bn(group->ctx,alfa2_index);
  tcsm_unlock_temp_bn(group->ctx,y2_index);
  tcsm_unlock_temp_bn(group->ctx,beta_index);
  tcsm_unlock_temp_bn(group->ctx,y3_2_index);
//  }
  return 0;
}

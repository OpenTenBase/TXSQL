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

#include <string.h>
#include <stdlib.h>

void *tcsm_tc_malloc(size_t size)
{
  return calloc(1, size);
}

void tcsm_tc_free(void *p)
{
  return free(p);
}

void* tcsm_tc_secure_malloc(size_t size)
{
  size_t real_size = size + sizeof(size_t);
  
  unsigned char * p = malloc(real_size);
  
  size_t* p_size = (size_t*)p;
  *p_size = size;
  
  return (p + sizeof(size_t));
}

void tcsm_tc_secure_free(void *p)
{
  if(NULL != p) {
    size_t* p_size = (size_t*)(p - sizeof(size_t));
    size_t real_size = *p_size + sizeof(size_t);
  
    unsigned char* p_origin = p - sizeof(size_t);
  
    memset(p_origin, 0, real_size);

    free(p_origin);
  }
}

void i2a_byte(char b, char *dst)
{
  int i;
  unsigned char val;
  
  for (i = 0; i < 2; i++)
  {
    val = (b & 0xf0) >> 4;
    if (0 <= val && val <= 9)
      dst[i] = val + '0';
    else
      dst[i] = val - 10 + 'A';
    b = b << 4;
  }
}

void i2a(char *str, unsigned char *hex, unsigned int len)
{
  int i;
  for (i = 0; i < len; i ++)
  {
    i2a_byte(hex[i], str+ (i<<1));
  }
  str[len * 2] = '\0';
}

//static char* s_mpz_set_bin_memory = NULL;
int mpz_set_bin(mpz_t a, unsigned char *bin, int len)
{
  int ret;
  char* str = NULL;
  
  
//  if (s_mpz_set_bin_memory == NULL) {
//    s_mpz_set_bin_memory = (char *)tcsm_tc_secure_malloc(513);
//  }
//
//  if (s_mpz_set_bin_memory == NULL)
//  {
//    return TC_FAILURE;
//  }
  
//  if (len > 256)
//  {
    str = (char *)tcsm_tc_secure_malloc((len << 1) + 1);
//  }else{
//    str = s_mpz_set_bin_memory;
//  }
  
  i2a(str, bin, len);
  ret = mpz_set_str(a, str, 16);
//  if (len > 256) {
    tcsm_tc_secure_free(str);
//  }
  return ret;
}

void a2i_byte(char *src, unsigned char *b, int cnt)
{
  int i;
  unsigned char val;
  
  *b = 0;
  for (i = 0; i < cnt && src[i] != '\0'; i++)
  {
    *b = *b << 4;
    val = src[i];
    if ('0' <= val && val <= '9')
      *b |= val - '0';
    else
      *b |= val - 87;
  }
}

void tcsm_a2i(char *str, unsigned char *bin, unsigned int *binlen)
{
  int i;
  int slen = (int)strlen(str);
  
  if (slen % 2)
  {
    a2i_byte(str, bin, 1);
    str++;
    bin++;
    (*binlen)++;
  }
  int c = (slen >> 1);
  
  for (i = 0; i < c; i++)
  {
    a2i_byte(str+(i << 1), bin + i, 2);
  }
  
  *binlen += i;
}

unsigned char *mpz_get_bin(unsigned char *bin, unsigned int *binlen, mpz_t op, unsigned int n)
{
  char *str = NULL;
  int str_bytlen = 0;
  int slen = 0;
  int right_offset = 0;
  
  *binlen = 0;
  str = mpz_get_str(str, 16, op);
  if (bin == NULL)
  {
    bin = (unsigned char *)tcsm_tc_secure_malloc(n);
    if (bin == NULL)
    {
      return NULL;
    }
  }
  slen = (int)strlen(str);
  str_bytlen = (slen >> 1) + slen % 2;
  
  if (n > str_bytlen)
  {
    right_offset = n - str_bytlen;
    *binlen +=  n - str_bytlen;
    memset(bin, 0x00, *binlen);
  }
  
  tcsm_a2i(str, bin + right_offset, binlen);
  tcsm_tc_free(str);
  return bin;
}

int tcsm_tc_bn_set_bin(tc_bn_t a, char *bin, int len)
{
  return mpz_set_bin(a->val, (unsigned char *)bin, len);
}

int tcsm_tc_bn_set_str(tc_bn_t a, char *str, int redix)
{
  return mpz_set_str(a->val, str, redix);
}

char *tcsm_tc_bn_get_str(char *str, tc_bn_t op)
{
  return mpz_get_str(str, 16, op->val);
}

unsigned char *tcsm_tc_bn_get_bin(unsigned char *bin, unsigned int *len, tc_bn_t op, unsigned int n)
{
  return mpz_get_bin(bin, len, op->val, n);
}

int tcsm_tc_bn_size_byte(tc_bn_t op)
{
  int bytelen = (int)mpz_sizeinbase(op->val, 16);
  return bytelen % 2 ? bytelen / 2 + 1 : bytelen / 2;
}

unsigned long int tcsm_tc_bn_get_ui(tc_bn_t op)
{
  return mpz_get_ui(op->val);
}

void tcsm_tc_bn_cpy(tc_bn_t dst, tc_bn_t src)
{
  mpz_set(dst->val, src->val);
}

int tcsm_tc_bn_cmp(tc_bn_t op1, tc_bn_t op2)
{
  return mpz_cmp(op1->val, op2->val);
}

int tcsm_tc_bn_sgn(tc_bn_t op)
{
  return mpz_sgn(op->val);
}

int tcsm_tc_bn_section(tc_bn_t op1, tc_bn_t op2, tc_bn_t op3)
{
  if ((tcsm_tc_bn_cmp(op1, op2) <= 0) && (tcsm_tc_bn_cmp(op2, op3) <= 0))
    return ERR_TENCENTSM_OK;
  else
    return ERR_BN_SECTION;
}

void tcsm_tc_bn_init(tc_bn_t a)
{
  a->b_using = 0;
  mpz_init(a->val);
}

void tcsm_tc_bn_clear(tc_bn_t a)
{
#ifdef _MEMORY_ERASE_PROTECTION
  mpz_set_str(a->val, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16);
#endif
  return mpz_clear(a->val);
}

void tcsm_tc_bn_add(tc_bn_t sum, tc_bn_t a, tc_bn_t b)
{
  return mpz_add(sum->val, a->val, b->val);
}

void tcsm_tc_bn_sub(tc_bn_t sub, tc_bn_t a, tc_bn_t b)
{
  return mpz_sub(sub->val, a->val, b->val);
}

void tcsm_tc_bn_fdiv_q(tc_bn_t q, tc_bn_t n, tc_bn_t d)
{
  return mpz_fdiv_r(q->val, n->val, d->val);
}

void tc_bn_fdiv_q_ui(tc_bn_t q, tc_bn_t r, unsigned long int n)
{
  mpz_fdiv_q_ui (q->val, r->val, n);
  return;
}

void tcsm_tc_bn_pow_ui(tc_bn_t rop, tc_bn_t base, unsigned long int exp)
{
  return mpz_pow_ui(rop->val, base->val, exp);
}

void tcsm_tc_bn_mul(tc_bn_t out, tc_bn_t a, tc_bn_t b)
{
  return mpz_mul(out->val, a->val, b->val);
}

void tcsm_tc_bn_mul_ui(tc_bn_t out, tc_bn_t a,unsigned long int n)
{
  return mpz_mul_ui(out->val, a->val, n);
}

int tcsm_tc_bn_invert(tc_bn_t rop, tc_bn_t a, tc_bn_t n)
{
  if (mpz_invert(rop->val, a->val, n->val))
    return ERR_TENCENTSM_OK;
  else
    return ERR_BN_INVERT;
}

int tcsm_tc_bn_num_bits(tc_bn_t bn)
{
  int bits = sizeof(mp_limb_t)*8;
  int ret = (bn->val->_mp_size * bits);
  return ret;
}

int tcsm_tc_bn_is_negtive(tc_bn_t bn)
{
  return bn->val->_mp_size < 0;
}

int tcsm_tc_bn_is_zero(tc_bn_t op)
{
  return (op->val->_mp_size == 0) && (op->val->_mp_d[0] == 0);
}

void tcsm_tc_bn_set_words(tc_bn_t a,unsigned long * words,int nums)
{
  if (a->val->_mp_alloc < 5) {
    mpz_set_str(a->val, "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123", 16);
  }
  memcpy(a->val->_mp_d, words, sizeof(unsigned long) * nums);
}

int tcsm_tc_bn_is_bit_set(tc_bn_t a, int n)
{
  int bits = sizeof(mp_limb_t)*8;
  
  int i, j;
  if (n < 0)
    return 0;
  i = n / bits;
  j = n % bits;
  
  if (abs(a->val->_mp_size) <= i)
    return 0;
  return (int)(((a->val->_mp_d[i]) >> j) & ((mp_limb_t)1));
}

void tcsm_tc_bn_mod(tc_bn_t out, tc_bn_t a, tc_bn_t n)
{
  return mpz_mod(out->val, a->val, n->val);
}

void tcsm_tc_bn_mod_ui(tc_bn_t out, tc_bn_t a, unsigned long int n)
{
  mpz_mod_ui(out->val, a->val, n);
  return;
}

void tcsm_tc_bn_powm(tc_bn_t rop, tc_bn_t base, tc_bn_t exp, tc_bn_t mod)
{
  return mpz_powm(rop->val, base->val, exp->val, mod->val);
}

void tcsm_tc_bn_powm_ui(tc_bn_t rop, tc_bn_t base, unsigned long int exp, tc_bn_t mod)
{
  return mpz_powm_ui(rop->val, base->val, exp, mod->val);
}

void tcsm_tc_bn_modadd(tc_bn_t sum, tc_bn_t a, tc_bn_t b, tc_bn_t n)
{
  mpz_add(sum->val, a->val, b->val);
  return mpz_mod(sum->val, sum->val, n->val);
  
}

void tcsm_tc_bn_modsub(tc_bn_t sub, tc_bn_t a, tc_bn_t b, tc_bn_t n)
{
  mpz_sub(sub->val, a->val, b->val);
  return mpz_mod(sub->val, sub->val, n->val);
}

void tcsm_tc_bn_modmul(tc_bn_t out, tc_bn_t a, tc_bn_t b, tc_bn_t n)
{
  mpz_mul(out->val, a->val, b->val);
  return mpz_mod(out->val, out->val, n->val);
}

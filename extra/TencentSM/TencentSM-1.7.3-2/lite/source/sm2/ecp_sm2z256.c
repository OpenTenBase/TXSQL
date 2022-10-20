#include "../include/tc_global.h"
#ifdef _OPT_ASM_ECC

#include "../include/ecp_sm2z256_macro.h"
#include <string.h>

extern const PRECOMP256_ROW ecp_sm2z256_precomputed[37];

static unsigned int _booth_recode_w5(unsigned int in)
{
  unsigned int s, d;
  
  s = ~((in >> 5) - 1);
  d = (1 << 6) - in - 1;
  d = (d & s) | (in & ~s);
  d = (d >> 1) + (d & 1);
  
  return (d << 1) + (s & 1);
}

static unsigned int _booth_recode_w7(unsigned int in)
{
  unsigned int s, d;
  
  s = ~((in >> 7) - 1);
  d = (1 << 8) - in - 1;
  d = (d & s) | (in & ~s);
  d = (d >> 1) + (d & 1);
  
  return (d << 1) + (s & 1);
}

static void copy_conditional(BN_ULONG dst[P256_LIMBS],
                             const BN_ULONG src[P256_LIMBS], BN_ULONG move)
{
  BN_ULONG mask1 = 0-move;
  BN_ULONG mask2 = ~mask1;
  
  dst[0] = (src[0] & mask1) ^ (dst[0] & mask2);
  dst[1] = (src[1] & mask1) ^ (dst[1] & mask2);
  dst[2] = (src[2] & mask1) ^ (dst[2] & mask2);
  dst[3] = (src[3] & mask1) ^ (dst[3] & mask2);
}

static BN_ULONG is_zero(BN_ULONG in)
{
  in |= (0 - in);
  in = ~in;
  in >>= BN_BITS2 - 1;
  return in;
}

static BN_ULONG is_equal(const BN_ULONG a[P256_LIMBS],
                         const BN_ULONG b[P256_LIMBS])
{
  BN_ULONG res;
  
  res = a[0] ^ b[0];
  res |= a[1] ^ b[1];
  res |= a[2] ^ b[2];
  res |= a[3] ^ b[3];
  
  return is_zero(res);
}

/* Modular add: res = a+b mod P   */
void ecp_sm2z256_add(BN_ULONG res[P256_LIMBS],
                     const BN_ULONG a[P256_LIMBS],
                     const BN_ULONG b[P256_LIMBS]);
/* Modular mul by 2: res = 2*a mod P */
void ecp_sm2z256_mul_by_2(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS]);
/* Modular mul by 3: res = 3*a mod P */
void ecp_sm2z256_mul_by_3(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS]);

/* Modular div by 2: res = a/2 mod P */
void ecp_sm2z256_div_by_2(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS]);
/* Modular sub: res = a-b mod P   */
void ecp_sm2z256_sub(BN_ULONG res[P256_LIMBS],
                     const BN_ULONG a[P256_LIMBS],
                     const BN_ULONG b[P256_LIMBS]);
/* Modular neg: res = -a mod P    */
void ecp_sm2z256_neg(BN_ULONG res[P256_LIMBS], const BN_ULONG a[P256_LIMBS]);
/* Montgomery mul: res = a*b*2^-256 mod P */
void ecp_sm2z256_mul_mont(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS],
                          const BN_ULONG b[P256_LIMBS]);
/* Montgomery sqr: res = a*a*2^-256 mod P */
void ecp_sm2z256_sqr_mont(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS]);
/* Convert a number from Montgomery domain, by multiplying with 1 */
void ecp_sm2z256_from_mont(BN_ULONG res[P256_LIMBS],
                           const BN_ULONG in[P256_LIMBS]);
/* Convert a number to Montgomery domain, by multiplying with 2^512 mod P*/
void ecp_sm2z256_to_mont(BN_ULONG res[P256_LIMBS],
                         const BN_ULONG in[P256_LIMBS]);
/* Functions that perform constant time access to the precomputed tables */
void ecp_sm2z256_scatter_w5(P256_POINT *val,
                            const P256_POINT *in_t, int idx);
void ecp_sm2z256_gather_w5(P256_POINT *val,
                           const P256_POINT *in_t, int idx);
void ecp_sm2z256_scatter_w7(P256_POINT_AFFINE *val,
                            const P256_POINT_AFFINE *in_t, int idx);
void ecp_sm2z256_gather_w7(P256_POINT_AFFINE *val,
                           const P256_POINT_AFFINE *in_t, int idx);

#ifdef ECP_SM2Z256_REFERENCE_IMPLEMENTATION
// the following functions are not correct in asm
void ecp_sm2z256_point_double(P256_POINT *r, const P256_POINT *a);
void ecp_sm2z256_point_add(P256_POINT *r,
                           const P256_POINT *a, const P256_POINT *b);
void ecp_sm2z256_point_add_affine(P256_POINT *r,
                                  const P256_POINT *a,
                                  const P256_POINT_AFFINE *b);
#else
/* Point double: r = 2*a */
static void ecp_sm2z256_point_double(P256_POINT *r, const P256_POINT *a)
{
  BN_ULONG S[P256_LIMBS];
  BN_ULONG M[P256_LIMBS];
  BN_ULONG Zsqr[P256_LIMBS];
  BN_ULONG tmp0[P256_LIMBS];
  
  const BN_ULONG *in_x = a->X;
  const BN_ULONG *in_y = a->Y;
  const BN_ULONG *in_z = a->Z;
  
  BN_ULONG *res_x = r->X;
  BN_ULONG *res_y = r->Y;
  BN_ULONG *res_z = r->Z;
  

  ecp_sm2z256_mul_by_2(S, in_y);
  
  ecp_sm2z256_sqr_mont(Zsqr, in_z);
  ecp_sm2z256_sqr_mont(S, S);

  ecp_sm2z256_mul_mont(res_z, in_z, in_y);
  ecp_sm2z256_mul_by_2(res_z, res_z);
  
  ecp_sm2z256_add(M, in_x, Zsqr);
  
  ecp_sm2z256_sub(Zsqr, in_x, Zsqr);
  
  ecp_sm2z256_sqr_mont(res_y, S);
  
  ecp_sm2z256_div_by_2(res_y, res_y);
  
  ecp_sm2z256_mul_mont(M, M, Zsqr);
  ecp_sm2z256_mul_by_3(M, M);
  
  ecp_sm2z256_mul_mont(S, S, in_x);
  ecp_sm2z256_mul_by_2(tmp0, S);
  
  ecp_sm2z256_sqr_mont(res_x, M);
  
  ecp_sm2z256_sub(res_x, res_x, tmp0);
  ecp_sm2z256_sub(S, S, res_x);
  
  ecp_sm2z256_mul_mont(S, S, M);
  ecp_sm2z256_sub(res_y, S, res_y);
}

/* Point addition: r = a+b */
static void ecp_sm2z256_point_add(P256_POINT *r,
                                  const P256_POINT *a, const P256_POINT *b)
{
  BN_ULONG U2[P256_LIMBS], S2[P256_LIMBS];
  BN_ULONG U1[P256_LIMBS], S1[P256_LIMBS];
  BN_ULONG Z1sqr[P256_LIMBS];
  BN_ULONG Z2sqr[P256_LIMBS];
  BN_ULONG H[P256_LIMBS], R[P256_LIMBS];
  BN_ULONG Hsqr[P256_LIMBS];
  BN_ULONG Rsqr[P256_LIMBS];
  BN_ULONG Hcub[P256_LIMBS];
  
  BN_ULONG res_x[P256_LIMBS];
  BN_ULONG res_y[P256_LIMBS];
  BN_ULONG res_z[P256_LIMBS];
  
  BN_ULONG in1infty, in2infty;
  
  const BN_ULONG *in1_x = a->X;
  const BN_ULONG *in1_y = a->Y;
  const BN_ULONG *in1_z = a->Z;
  
  const BN_ULONG *in2_x = b->X;
  const BN_ULONG *in2_y = b->Y;
  const BN_ULONG *in2_z = b->Z;
  
  /*
   * Infinity in encoded as (,,0)
   */
  in1infty = (in1_z[0] | in1_z[1] | in1_z[2] | in1_z[3]);
  in2infty = (in2_z[0] | in2_z[1] | in2_z[2] | in2_z[3]);
  
  in1infty = is_zero(in1infty);
  in2infty = is_zero(in2infty);
  
  ecp_sm2z256_sqr_mont(Z2sqr, in2_z);        /* Z2^2 */
  ecp_sm2z256_sqr_mont(Z1sqr, in1_z);        /* Z1^2 */
  
  ecp_sm2z256_mul_mont(S1, Z2sqr, in2_z);    /* S1 = Z2^3 */
  ecp_sm2z256_mul_mont(S2, Z1sqr, in1_z);    /* S2 = Z1^3 */
  
  ecp_sm2z256_mul_mont(S1, S1, in1_y);       /* S1 = Y1*Z2^3 */
  ecp_sm2z256_mul_mont(S2, S2, in2_y);       /* S2 = Y2*Z1^3 */
  ecp_sm2z256_sub(R, S2, S1);                /* R = S2 - S1 */
  
  ecp_sm2z256_mul_mont(U1, in1_x, Z2sqr);    /* U1 = X1*Z2^2 */
  ecp_sm2z256_mul_mont(U2, in2_x, Z1sqr);    /* U2 = X2*Z1^2 */
  ecp_sm2z256_sub(H, U2, U1);                /* H = U2 - U1 */
  
  /*
   * This should not happen during sign/ecdh, so no constant time violation
   */
  if (is_equal(U1, U2) && !in1infty && !in2infty) {
    if (is_equal(S1, S2)) {
      ecp_sm2z256_point_double(r, a);
      return;
    } else {
      memset(r, 0, sizeof(*r));
      return;
    }
  }
  
  ecp_sm2z256_sqr_mont(Rsqr, R);             /* R^2 */
  ecp_sm2z256_mul_mont(res_z, H, in1_z);     /* Z3 = H*Z1*Z2 */
  ecp_sm2z256_sqr_mont(Hsqr, H);             /* H^2 */
  ecp_sm2z256_mul_mont(res_z, res_z, in2_z); /* Z3 = H*Z1*Z2 */
  ecp_sm2z256_mul_mont(Hcub, Hsqr, H);       /* H^3 */
  
  ecp_sm2z256_mul_mont(U2, U1, Hsqr);        /* U1*H^2 */
  ecp_sm2z256_mul_by_2(Hsqr, U2);            /* 2*U1*H^2 */
  
  ecp_sm2z256_sub(res_x, Rsqr, Hsqr);
  ecp_sm2z256_sub(res_x, res_x, Hcub);
  
  ecp_sm2z256_sub(res_y, U2, res_x);
  
  ecp_sm2z256_mul_mont(S2, S1, Hcub);
  ecp_sm2z256_mul_mont(res_y, R, res_y);
  ecp_sm2z256_sub(res_y, res_y, S2);
  
  copy_conditional(res_x, in2_x, in1infty);
  copy_conditional(res_y, in2_y, in1infty);
  copy_conditional(res_z, in2_z, in1infty);
  
  copy_conditional(res_x, in1_x, in2infty);
  copy_conditional(res_y, in1_y, in2infty);
  copy_conditional(res_z, in1_z, in2infty);
  
  memcpy(r->X, res_x, sizeof(res_x));
  memcpy(r->Y, res_y, sizeof(res_y));
  memcpy(r->Z, res_z, sizeof(res_z));
}

/* Point addition when b is known to be affine: r = a+b */
static void ecp_sm2z256_point_add_affine(P256_POINT *r,
                                         const P256_POINT *a,
                                         const P256_POINT_AFFINE *b)
{
  BN_ULONG U2[P256_LIMBS], S2[P256_LIMBS];
  BN_ULONG Z1sqr[P256_LIMBS];
  BN_ULONG H[P256_LIMBS], R[P256_LIMBS];
  BN_ULONG Hsqr[P256_LIMBS];
  BN_ULONG Rsqr[P256_LIMBS];
  BN_ULONG Hcub[P256_LIMBS];
  
  BN_ULONG res_x[P256_LIMBS];
  BN_ULONG res_y[P256_LIMBS];
  BN_ULONG res_z[P256_LIMBS];
  
  BN_ULONG in1infty, in2infty;
  
  const BN_ULONG *in1_x = a->X;
  const BN_ULONG *in1_y = a->Y;
  const BN_ULONG *in1_z = a->Z;
  
  const BN_ULONG *in2_x = b->X;
  const BN_ULONG *in2_y = b->Y;
  
  /*
   * Infinity in encoded as (,,0)
   */
  in1infty = (in1_z[0] | in1_z[1] | in1_z[2] | in1_z[3]);
  
  /*
   * In affine representation we encode infinity as (0,0), which is
   * not on the curve, so it is OK
   */
  in2infty = (in2_x[0] | in2_x[1] | in2_x[2] | in2_x[3] |
              in2_y[0] | in2_y[1] | in2_y[2] | in2_y[3]);
  
  in1infty = is_zero(in1infty);
  in2infty = is_zero(in2infty);
  
  ecp_sm2z256_sqr_mont(Z1sqr, in1_z);        /* Z1^2 */
  
  ecp_sm2z256_mul_mont(U2, in2_x, Z1sqr);    /* U2 = X2*Z1^2 */
  ecp_sm2z256_sub(H, U2, in1_x);             /* H = U2 - U1 */
  
  ecp_sm2z256_mul_mont(S2, Z1sqr, in1_z);    /* S2 = Z1^3 */
  
  ecp_sm2z256_mul_mont(res_z, H, in1_z);     /* Z3 = H*Z1*Z2 */
  
  ecp_sm2z256_mul_mont(S2, S2, in2_y);       /* S2 = Y2*Z1^3 */
  ecp_sm2z256_sub(R, S2, in1_y);             /* R = S2 - S1 */
  
  ecp_sm2z256_sqr_mont(Hsqr, H);             /* H^2 */
  ecp_sm2z256_sqr_mont(Rsqr, R);             /* R^2 */
  ecp_sm2z256_mul_mont(Hcub, Hsqr, H);       /* H^3 */
  
  ecp_sm2z256_mul_mont(U2, in1_x, Hsqr);     /* U1*H^2 */
  ecp_sm2z256_mul_by_2(Hsqr, U2);            /* 2*U1*H^2 */
  
  ecp_sm2z256_sub(res_x, Rsqr, Hsqr);
  ecp_sm2z256_sub(res_x, res_x, Hcub);
  ecp_sm2z256_sub(H, U2, res_x);
  
  ecp_sm2z256_mul_mont(S2, in1_y, Hcub);
  ecp_sm2z256_mul_mont(H, H, R);
  ecp_sm2z256_sub(res_y, H, S2);
  
  copy_conditional(res_x, in2_x, in1infty);
  copy_conditional(res_x, in1_x, in2infty);
  
  copy_conditional(res_y, in2_y, in1infty);
  copy_conditional(res_y, in1_y, in2infty);
  
  copy_conditional(res_z, ONE, in1infty);
  copy_conditional(res_z, in1_z, in2infty);
  
  memcpy(r->X, res_x, sizeof(res_x));
  memcpy(r->Y, res_y, sizeof(res_y));
  memcpy(r->Z, res_z, sizeof(res_z));
}
#endif

static void ecp_sm2z256_mod_inverse(BN_ULONG r[P256_LIMBS],
                                    const BN_ULONG in[P256_LIMBS]);

int ecp_sm2z256_P256_POINT_2_P256_POINT_AFFINE(tc_ecc_group_st *group,
                                     P256_POINT *point,
                                     P256_POINT_AFFINE *afn_point)
{
  BN_ULONG z_inv2[P256_LIMBS];
  BN_ULONG z_inv3[P256_LIMBS];
  BN_ULONG x_aff[P256_LIMBS];
  BN_ULONG y_aff[P256_LIMBS];
  BN_ULONG point_x[P256_LIMBS], point_y[P256_LIMBS], point_z[P256_LIMBS];
  BN_ULONG x_ret[P256_LIMBS], y_ret[P256_LIMBS];
  
  for (int i = 0; i < 4; i++) {
    point_x[i] = point->X[i];
    point_y[i] = point->Y[i];
    point_z[i] = point->Z[i];
  }
  
  ecp_sm2z256_mod_inverse(z_inv3, point_z);
  ecp_sm2z256_sqr_mont(z_inv2, z_inv3);
  ecp_sm2z256_mul_mont(x_aff, z_inv2, point_x);
  
  if (afn_point != NULL) {
    ecp_sm2z256_from_mont(x_ret, x_aff);
    
    for (int i = 0; i < 4; i++) {
      afn_point->X[i] = x_ret[i];
    }
  }
  
  if (afn_point != NULL) {
    ecp_sm2z256_mul_mont(z_inv3, z_inv3, z_inv2);
    ecp_sm2z256_mul_mont(y_aff, z_inv3, point_y);
    ecp_sm2z256_from_mont(y_ret, y_aff);
    for (int i = 0; i < 4; i++) {
      afn_point->Y[i] = y_ret[i];
    }
  }
  
  return 0;
}

static int ecp_sm2z256_mult_precompute(tc_ecc_group_st *group,P256_POINT *precompute_point,void ** pre_comp)
{
  P256_POINT P;
  P256_POINT T;
  int i, j, k, ret = 0;
  size_t w;
  
  PRECOMP256_ROW *preComputedTable = NULL;
  unsigned char *precomp_storage = NULL;
  
  w = 7;
  
  precomp_storage = tcsm_tc_secure_malloc(37 * 64 * sizeof(P256_POINT_AFFINE) + 64);
  
  preComputedTable = (void *)ALIGNPTR(precomp_storage, 64);
  
  for (int i = 0; i < 4; i++) {
    T.X[i] = precompute_point->X[i];
    T.Y[i] = precompute_point->Y[i];
    T.Z[i] = precompute_point->Z[i];
  }

  for (k = 0; k < 64; k++) {
    for (int i = 0; i < 4; i++) {
      P.X[i] = T.X[i];
      P.Y[i] = T.Y[i];
      P.Z[i] = T.Z[i];
    }
    for (j = 0; j < 37; j++) {
      P256_POINT_AFFINE temp;
      ecp_sm2z256_P256_POINT_2_P256_POINT_AFFINE(group, &P, &temp);
      
      ecp_sm2z256_to_mont(temp.X, temp.X);
      ecp_sm2z256_to_mont(temp.Y, temp.Y);
      
#ifdef _X86_64_OPT_ASM_ECC
      ecp_sm2z256_scatter_w7((P256_POINT_AFFINE *)preComputedTable[j], &temp, k);
#else
      printf("fatal error for macro define!\n");
#endif
      for (i = 0; i < 7; i++) {
        ecp_sm2z256_point_double(&P, &P);
      }
    }
    ecp_sm2z256_point_add(&T, &T, precompute_point);
  }
  

  ret = 1;
err:
#ifdef _X86_64_OPT_ASM_ECC
  *pre_comp = precomp_storage;
#endif
  return ret;
}

static void ecp_sm2z256_mod_inverse(BN_ULONG r[P256_LIMBS],
                                    const BN_ULONG in[P256_LIMBS])
{
  BN_ULONG a1[P256_LIMBS];
  BN_ULONG a2[P256_LIMBS];
  BN_ULONG a3[P256_LIMBS];
  BN_ULONG a4[P256_LIMBS];
  BN_ULONG a5[P256_LIMBS];
  int i;
  
  ecp_sm2z256_sqr_mont(a1, in);
  ecp_sm2z256_mul_mont(a2, a1, in);
  ecp_sm2z256_sqr_mont(a3, a2);
  ecp_sm2z256_sqr_mont(a3, a3);
  ecp_sm2z256_mul_mont(a3, a3, a2);
  ecp_sm2z256_sqr_mont(a4, a3);
  ecp_sm2z256_sqr_mont(a4, a4);
  ecp_sm2z256_sqr_mont(a4, a4);
  ecp_sm2z256_sqr_mont(a4, a4);
  ecp_sm2z256_mul_mont(a4, a4, a3);
  ecp_sm2z256_sqr_mont(a5, a4);
  for (i = 0; i < 7; i++) {
    ecp_sm2z256_sqr_mont(a5, a5);
  }
  ecp_sm2z256_mul_mont(a5, a5, a4);
  for (i = 0; i < 8; i++) {
    ecp_sm2z256_sqr_mont(a5, a5);
  }
  ecp_sm2z256_mul_mont(a5, a5, a4);
  ecp_sm2z256_sqr_mont(a5, a5);
  ecp_sm2z256_sqr_mont(a5, a5);
  ecp_sm2z256_sqr_mont(a5, a5);
  ecp_sm2z256_sqr_mont(a5, a5);
  ecp_sm2z256_mul_mont(a5, a5, a3);
  ecp_sm2z256_sqr_mont(a5, a5);
  ecp_sm2z256_sqr_mont(a5, a5);
  ecp_sm2z256_mul_mont(a5, a5, a2);
  ecp_sm2z256_sqr_mont(a5, a5);
  ecp_sm2z256_mul_mont(a5, a5, in);
  ecp_sm2z256_sqr_mont(a4, a5);
  ecp_sm2z256_mul_mont(a3, a4, a1);
  ecp_sm2z256_sqr_mont(a5, a4);
  for (i = 0; i < 30; i++) {
    ecp_sm2z256_sqr_mont(a5, a5);
  }
  ecp_sm2z256_mul_mont(a4, a5, a4);
  ecp_sm2z256_sqr_mont(a4, a4);
  ecp_sm2z256_mul_mont(a4, a4, in);
  ecp_sm2z256_mul_mont(a3, a4, a2);
  for (i = 0; i < 33; i++) {
    ecp_sm2z256_sqr_mont(a5, a5);
  }
  ecp_sm2z256_mul_mont(a2, a5, a3);
  ecp_sm2z256_mul_mont(a3, a2, a3);
  for (i = 0; i < 32; i++) {
    ecp_sm2z256_sqr_mont(a5, a5);
  }
  ecp_sm2z256_mul_mont(a2, a5, a3);
  ecp_sm2z256_mul_mont(a3, a2, a3);
  ecp_sm2z256_mul_mont(a4, a2, a4);
  for (i = 0; i < 32; i++) {
    ecp_sm2z256_sqr_mont(a5, a5);
  }
  ecp_sm2z256_mul_mont(a2, a5, a3);
  ecp_sm2z256_mul_mont(a3, a2, a3);
  ecp_sm2z256_mul_mont(a4, a2, a4);
  for (i = 0; i < 32; i++) {
    ecp_sm2z256_sqr_mont(a5, a5);
  }
  ecp_sm2z256_mul_mont(a2, a5, a3);
  ecp_sm2z256_mul_mont(a3, a2, a3);
  ecp_sm2z256_mul_mont(a4, a2, a4);
  for (i = 0; i < 32; i++) {
    ecp_sm2z256_sqr_mont(a5, a5);
  }
  ecp_sm2z256_mul_mont(a2, a5, a3);
  ecp_sm2z256_mul_mont(a3, a2, a3);
  ecp_sm2z256_mul_mont(a4, a2, a4);
  for (i = 0; i < 32; i++) {
    ecp_sm2z256_sqr_mont(a5, a5);
  }
  ecp_sm2z256_mul_mont(r, a4, a5);
}

int ecp_sm2z256_bignum_to_field_elem(BN_ULONG out[P256_LIMBS],
                                            tc_bn_t in)
{
  out[0] = in->val->_mp_d[0];
  out[1] = in->val->_mp_d[1];
  out[2] = in->val->_mp_d[2];
  out[3] = in->val->_mp_d[3];
  return 1;
}

int ecp_sm2z256_get_affine(tc_ecc_group_st *group,
                                  tc_ecc_jcb_point_st *point,
                                  tc_bn_t x, tc_bn_t y)
{
  BN_ULONG z_inv2[P256_LIMBS];
  BN_ULONG z_inv3[P256_LIMBS];
  BN_ULONG x_aff[P256_LIMBS];
  BN_ULONG y_aff[P256_LIMBS];
  BN_ULONG point_x[P256_LIMBS], point_y[P256_LIMBS], point_z[P256_LIMBS];
  BN_ULONG x_ret[P256_LIMBS], y_ret[P256_LIMBS];
  
  if (!ecp_sm2z256_bignum_to_field_elem(point_x, point->x) ||
      !ecp_sm2z256_bignum_to_field_elem(point_y, point->y) ||
      !ecp_sm2z256_bignum_to_field_elem(point_z, point->z)) {
    return -1;
  }

  
  ecp_sm2z256_mod_inverse(z_inv3, point_z);
  ecp_sm2z256_sqr_mont(z_inv2, z_inv3);
  ecp_sm2z256_mul_mont(x_aff, z_inv2, point_x);
  
  if (x != NULL) {
    ecp_sm2z256_from_mont(x_ret, x_aff);
    tcsm_tc_bn_set_words(x, x_ret, P256_LIMBS);
  }
  
  if (y != NULL) {
    ecp_sm2z256_mul_mont(z_inv3, z_inv3, z_inv2);
    ecp_sm2z256_mul_mont(y_aff, z_inv3, point_y);
    ecp_sm2z256_from_mont(y_ret, y_aff);
    tcsm_tc_bn_set_words(y, y_ret, P256_LIMBS);
  }
  
  return 0;
}

int ecp_sm2z256_precompute_for_point(tc_ecc_group_st *group,tc_ec_t point)
{
  P256_POINT fixed_point;

  for (int j = 0; j < P256_LIMBS; j++) {
    fixed_point.X[j] = point->x->val->_mp_d[j];
    fixed_point.Y[j] = point->y->val->_mp_d[j];
  }

  fixed_point.Z[0] = 1;
  fixed_point.Z[1] = 0;
  fixed_point.Z[2] = 0;
  fixed_point.Z[3] = 0;
  
  ecp_sm2z256_to_mont(fixed_point.X, fixed_point.X);
  ecp_sm2z256_to_mont(fixed_point.Y, fixed_point.Y);
  ecp_sm2z256_to_mont(fixed_point.Z, fixed_point.Z);

  void *fixed_point_pre_comp = NULL;
  int ret = ecp_sm2z256_mult_precompute(group, &fixed_point,&fixed_point_pre_comp);
  group->ctx->pre_comp_p = (void*)fixed_point_pre_comp;
  return ret;
}

int ecp_sm2z256_points_mul_for_pubkey(tc_ecc_group_st *group, tc_ecc_point_st* r, tc_bn_st* scalar)
{
  int i = 0, ret = ERR_ECC_POINTS_MUL, p_is_infinity = 0;
  
  unsigned char p_str[33] = { 0 };
  const PRECOMP256_ROW *preComputedTable = (void *)ALIGNPTR(group->ctx->pre_comp_p, 64);
  unsigned int idx = 0;
  const unsigned int window_size = 7;
  const unsigned int mask = (1 << (window_size + 1)) - 1;
  unsigned int wvalue;
  ALIGN32 union {
    P256_POINT p;
    P256_POINT_AFFINE a;
  } t, p;
  int tmp_scalar_index;
  tc_bn_t *tmp_scalar = tcsm_lock_temp_bn(group->ctx,&tmp_scalar_index);
  
  if (scalar) {
    
    if (preComputedTable) {
      if ((tcsm_tc_bn_num_bits(scalar) > 256)
          || tcsm_tc_bn_is_negtive(scalar)) {
        
        tcsm_tc_bn_mod(*tmp_scalar, scalar, group->p);
        scalar = *tmp_scalar;
      }
      
      for (i = 0; i < scalar->val->_mp_size * BN_BYTES; i += BN_BYTES) {
        
        BN_ULONG d = scalar->val->_mp_d[i / BN_BYTES];
        
        p_str[i + 0] = (unsigned char)d;
        p_str[i + 1] = (unsigned char)(d >> 8);
        p_str[i + 2] = (unsigned char)(d >> 16);
        p_str[i + 3] = (unsigned char)(d >>= 24);
        if (BN_BYTES == 8) {
          d >>= 8;
          p_str[i + 4] = (unsigned char)d;
          p_str[i + 5] = (unsigned char)(d >> 8);
          p_str[i + 6] = (unsigned char)(d >> 16);
          p_str[i + 7] = (unsigned char)(d >> 24);
        }
      }
      
      for (; i < 33; i++)
        p_str[i] = 0;
      
      BN_ULONG infty;
      
      /* First window */
      wvalue = (p_str[0] << 1) & mask;
      idx += window_size;
      
      wvalue = _booth_recode_w7(wvalue);
      
      ecp_sm2z256_gather_w7(&p.a, preComputedTable[0],
                            wvalue >> 1);
      
      ecp_sm2z256_neg(p.p.Z, p.p.Y);
      copy_conditional(p.p.Y, p.p.Z, wvalue & 1);
      
      infty = (p.p.X[0] | p.p.X[1] | p.p.X[2] | p.p.X[3] |
               p.p.Y[0] | p.p.Y[1] | p.p.Y[2] | p.p.Y[3]);
      
      infty = 0 - is_zero(infty);
      infty = ~infty;
      
      p.p.Z[0] = ONE[0] & infty;
      p.p.Z[1] = ONE[1] & infty;
      p.p.Z[2] = ONE[2] & infty;
      p.p.Z[3] = ONE[3] & infty;
      
      for (i = 1; i < 37; i++) {
        unsigned int off = (idx - 1) / 8;
        wvalue = p_str[off] | p_str[off + 1] << 8;
        wvalue = (wvalue >> ((idx - 1) % 8)) & mask;
        idx += window_size;
        
        wvalue = _booth_recode_w7(wvalue);
        
        ecp_sm2z256_gather_w7(&t.a,
                              preComputedTable[i], wvalue >> 1);
        
        ecp_sm2z256_neg(t.p.Z, t.a.Y);
        copy_conditional(t.a.Y, t.p.Z, wvalue & 1);
        
        ecp_sm2z256_point_add_affine(&p.p, &p.p, &t.a);
      }
    } else {
      p_is_infinity = 1;
    }
  } else
    p_is_infinity = 1;
  
  tc_ecc_jcb_point_st* jcb = group->ctx->jcb_compute_var;
  tcsm_tc_bn_set_words(jcb->x, p.p.X, P256_LIMBS);
  tcsm_tc_bn_set_words(jcb->y, p.p.Y, P256_LIMBS);
  tcsm_tc_bn_set_words(jcb->z, p.p.Z, P256_LIMBS);
  
  
  
  
  ecp_sm2z256_get_affine(group, jcb, r->x, r->y);
  ret = ERR_TENCENTSM_OK;
err:
  tcsm_unlock_temp_bn(group->ctx, tmp_scalar_index);
  return ret;
}

int ecp_sm2z256_points_mul_for_generator(tc_ecc_group_st *group, tc_ecc_point_st* r, tc_bn_st* scalar)
{
  int i = 0, ret = ERR_ECC_POINTS_MUL, p_is_infinity = 0;
  
  unsigned char p_str[33] = { 0 };
  const PRECOMP256_ROW *preComputedTable = ecp_sm2z256_precomputed;
  unsigned int idx = 0;
  const unsigned int window_size = 7;
  const unsigned int mask = (1 << (window_size + 1)) - 1;
  unsigned int wvalue;
  ALIGN32 union {
    P256_POINT p;
    P256_POINT_AFFINE a;
  } t, p;
  int tmp_scalar_index;
  tc_bn_t *tmp_scalar = tcsm_lock_temp_bn(group->ctx,&tmp_scalar_index);
  
  if (scalar) {
    
    if (preComputedTable) {
      if ((tcsm_tc_bn_num_bits(scalar) > 256)
          || tcsm_tc_bn_is_negtive(scalar)) {
        
        tcsm_tc_bn_mod(*tmp_scalar, scalar, group->p);
        scalar = *tmp_scalar;
      }
      
      for (i = 0; i < scalar->val->_mp_size * BN_BYTES; i += BN_BYTES) {
        
        BN_ULONG d = scalar->val->_mp_d[i / BN_BYTES];
        
        p_str[i + 0] = (unsigned char)d;
        p_str[i + 1] = (unsigned char)(d >> 8);
        p_str[i + 2] = (unsigned char)(d >> 16);
        p_str[i + 3] = (unsigned char)(d >>= 24);
        if (BN_BYTES == 8) {
          d >>= 8;
          p_str[i + 4] = (unsigned char)d;
          p_str[i + 5] = (unsigned char)(d >> 8);
          p_str[i + 6] = (unsigned char)(d >> 16);
          p_str[i + 7] = (unsigned char)(d >> 24);
        }
      }
      
      for (; i < 33; i++)
        p_str[i] = 0;
      
      BN_ULONG infty;
      
      /* First window */
      wvalue = (p_str[0] << 1) & mask;
      idx += window_size;
      
      wvalue = _booth_recode_w7(wvalue);
      
      ecp_sm2z256_gather_w7(&p.a, preComputedTable[0],
                            wvalue >> 1);
      
      ecp_sm2z256_neg(p.p.Z, p.p.Y);
      copy_conditional(p.p.Y, p.p.Z, wvalue & 1);
      
      infty = (p.p.X[0] | p.p.X[1] | p.p.X[2] | p.p.X[3] |
               p.p.Y[0] | p.p.Y[1] | p.p.Y[2] | p.p.Y[3]);
      
      infty = 0 - is_zero(infty);
      infty = ~infty;
      
      p.p.Z[0] = ONE[0] & infty;
      p.p.Z[1] = ONE[1] & infty;
      p.p.Z[2] = ONE[2] & infty;
      p.p.Z[3] = ONE[3] & infty;
      
      for (i = 1; i < 37; i++) {
        unsigned int off = (idx - 1) / 8;
        wvalue = p_str[off] | p_str[off + 1] << 8;
        wvalue = (wvalue >> ((idx - 1) % 8)) & mask;
        idx += window_size;
        
        wvalue = _booth_recode_w7(wvalue);
        
        ecp_sm2z256_gather_w7(&t.a,
                              preComputedTable[i], wvalue >> 1);
        
        ecp_sm2z256_neg(t.p.Z, t.a.Y);
        copy_conditional(t.a.Y, t.p.Z, wvalue & 1);
        
        ecp_sm2z256_point_add_affine(&p.p, &p.p, &t.a);
      }
    } else {
      p_is_infinity = 1;
    }
  } else
    p_is_infinity = 1;
  
  tc_ecc_jcb_point_st* jcb = group->ctx->jcb_compute_var;
  tcsm_tc_bn_set_words(jcb->x, p.p.X, P256_LIMBS);
  tcsm_tc_bn_set_words(jcb->y, p.p.Y, P256_LIMBS);
  tcsm_tc_bn_set_words(jcb->z, p.p.Z, P256_LIMBS);
  ecp_sm2z256_get_affine(group, jcb, r->x, r->y);
  ret = ERR_TENCENTSM_OK;
err:
  tcsm_unlock_temp_bn(group->ctx, tmp_scalar_index);
  return ret;
}

int ecp_sm2z256_windowed_mul_for_point(tc_ecc_group_st *group,
                             P256_POINT *r,
                             tc_bn_st *scalar,
                             P256_POINT *point)
{
  size_t i;
  int j, ret = ERR_ECC_POINTS_MUL;
  unsigned int idx;
  unsigned char (*p_str)[33] = NULL;
  const unsigned int window_size = 5;
  const unsigned int mask = (1 << (window_size + 1)) - 1;
  unsigned int wvalue;
  P256_POINT *temp;           /* place for 5 temporary points */
  const tc_bn_st **scalars = NULL;
  P256_POINT (*table)[16] = NULL;
  
  int alloc_size = ( 16 + 5 ) * sizeof(P256_POINT) + 64*2;
  void *table_storage = tcsm_tc_secure_malloc(alloc_size);
  if (table_storage == NULL) {
    return ERR_TC_MALLOC;
  }
  
  p_str = tcsm_tc_secure_malloc( 33 * sizeof(unsigned char));
  if (p_str == NULL) {
    tcsm_tc_secure_free(table_storage);
    return ERR_TC_MALLOC;
  }
  
  scalars = tcsm_tc_secure_malloc(sizeof(tc_bn_st *));
  if (scalars == NULL) {
    tcsm_tc_secure_free(table_storage);
    tcsm_tc_secure_free(p_str);
    return ERR_TC_MALLOC;
  }
  
  table = (void *)ALIGNPTR(table_storage, 64);
  temp = (P256_POINT *)(table + 1);
  
  int tmp_scalar_index;
  tc_bn_t *tmp_scalar = tcsm_lock_temp_bn(group->ctx,&tmp_scalar_index);
  
  for (i = 0; i < 1; i++) {
    P256_POINT *row = table[i];
    
    /* This is an unusual input, we don't guarantee constant-timeness. */
    if ((tcsm_tc_bn_num_bits(scalar) > 256) || tcsm_tc_bn_is_negtive(scalar)) {
      tcsm_tc_bn_mod(*tmp_scalar, scalar, group->p);
      scalars[i] = *tmp_scalar;
      
    } else
      scalars[i] = scalar;
    
    for (j = 0; j < scalar->val->_mp_size * BN_BYTES; j += BN_BYTES) {
      BN_ULONG d = (scalar)->val->_mp_d[j / BN_BYTES];
      
      p_str[i][j + 0] = (unsigned char)d;
      p_str[i][j + 1] = (unsigned char)(d >> 8);
      p_str[i][j + 2] = (unsigned char)(d >> 16);
      p_str[i][j + 3] = (unsigned char)(d >>= 24);
      if (BN_BYTES == 8) {
        d >>= 8;
        p_str[i][j + 4] = (unsigned char)d;
        p_str[i][j + 5] = (unsigned char)(d >> 8);
        p_str[i][j + 6] = (unsigned char)(d >> 16);
        p_str[i][j + 7] = (unsigned char)(d >> 24);
      }
    }
    for (; j < 33; j++)
      p_str[i][j] = 0;
    
    for (int j = 0; j < P256_LIMBS; j++) {
      temp[0].X[j] = point->X[j];
      temp[0].Y[j] = point->Y[j];
    }
    
    temp[0].Z[0] = 1;
    temp[0].Z[1] = 0;
    temp[0].Z[2] = 0;
    temp[0].Z[3] = 0;

    ecp_sm2z256_to_mont(temp[0].X, temp[0].X);
    ecp_sm2z256_to_mont(temp[0].Y, temp[0].Y);
    ecp_sm2z256_to_mont(temp[0].Z, temp[0].Z);
    
    /*
     * row[0] is implicitly (0,0,0) (the point at infinity), therefore it
     * is not stored. All other values are actually stored with an offset
     * of -1 in table.
     */
    
    ecp_sm2z256_scatter_w5  (row, &temp[0], 1);
    ecp_sm2z256_point_double(&temp[1], &temp[0]);              /*1+1=2  */
    ecp_sm2z256_scatter_w5  (row, &temp[1], 2);
    ecp_sm2z256_point_add   (&temp[2], &temp[1], &temp[0]);    /*2+1=3  */
    ecp_sm2z256_scatter_w5  (row, &temp[2], 3);
    ecp_sm2z256_point_double(&temp[1], &temp[1]);              /*2*2=4  */
    ecp_sm2z256_scatter_w5  (row, &temp[1], 4);
    ecp_sm2z256_point_double(&temp[2], &temp[2]);              /*2*3=6  */
    ecp_sm2z256_scatter_w5  (row, &temp[2], 6);
    ecp_sm2z256_point_add   (&temp[3], &temp[1], &temp[0]);    /*4+1=5  */
    ecp_sm2z256_scatter_w5  (row, &temp[3], 5);
    ecp_sm2z256_point_add   (&temp[4], &temp[2], &temp[0]);    /*6+1=7  */
    ecp_sm2z256_scatter_w5  (row, &temp[4], 7);
    ecp_sm2z256_point_double(&temp[1], &temp[1]);              /*2*4=8  */
    ecp_sm2z256_scatter_w5  (row, &temp[1], 8);
    ecp_sm2z256_point_double(&temp[2], &temp[2]);              /*2*6=12 */
    ecp_sm2z256_scatter_w5  (row, &temp[2], 12);
    ecp_sm2z256_point_double(&temp[3], &temp[3]);              /*2*5=10 */
    ecp_sm2z256_scatter_w5  (row, &temp[3], 10);
    ecp_sm2z256_point_double(&temp[4], &temp[4]);              /*2*7=14 */
    ecp_sm2z256_scatter_w5  (row, &temp[4], 14);
    ecp_sm2z256_point_add   (&temp[2], &temp[2], &temp[0]);    /*12+1=13*/
    ecp_sm2z256_scatter_w5  (row, &temp[2], 13);
    ecp_sm2z256_point_add   (&temp[3], &temp[3], &temp[0]);    /*10+1=11*/
    ecp_sm2z256_scatter_w5  (row, &temp[3], 11);
    ecp_sm2z256_point_add   (&temp[4], &temp[4], &temp[0]);    /*14+1=15*/
    ecp_sm2z256_scatter_w5  (row, &temp[4], 15);
    ecp_sm2z256_point_add   (&temp[2], &temp[1], &temp[0]);    /*8+1=9  */
    ecp_sm2z256_scatter_w5  (row, &temp[2], 9);
    ecp_sm2z256_point_double(&temp[1], &temp[1]);              /*2*8=16 */
    ecp_sm2z256_scatter_w5  (row, &temp[1], 16);
  }
  
  idx = 255;
  
  wvalue = p_str[0][(idx - 1) / 8];
  wvalue = (wvalue >> ((idx - 1) % 8)) & mask;
  
  /*
   * We gather to temp[0], because we know it's position relative
   * to table
   */
  ecp_sm2z256_gather_w5(&temp[0], table[0], _booth_recode_w5(wvalue) >> 1);
  memcpy(r, &temp[0], sizeof(temp[0]));
  
  while (idx >= 5) {
    for (i = (idx == 255 ? 1 : 0); i < 1; i++) {
      unsigned int off = (idx - 1) / 8;
      
      wvalue = p_str[i][off] | p_str[i][off + 1] << 8;
      wvalue = (wvalue >> ((idx - 1) % 8)) & mask;
      
      wvalue = _booth_recode_w5(wvalue);
      
      ecp_sm2z256_gather_w5(&temp[0], table[i], wvalue >> 1);
      
      ecp_sm2z256_neg(temp[1].Y, temp[0].Y);
      copy_conditional(temp[0].Y, temp[1].Y, (wvalue & 1));
      
      ecp_sm2z256_point_add(r, r, &temp[0]);
    }
    
    idx -= window_size;
    
    ecp_sm2z256_point_double(r, r);
    ecp_sm2z256_point_double(r, r);
    ecp_sm2z256_point_double(r, r);
    ecp_sm2z256_point_double(r, r);
    ecp_sm2z256_point_double(r, r);
  }
  
  /* Final window */
  wvalue = p_str[0][0];
  wvalue = (wvalue << 1) & mask;
  
  wvalue = _booth_recode_w5(wvalue);
  
  ecp_sm2z256_gather_w5(&temp[0], table[0], wvalue >> 1);
  
  ecp_sm2z256_neg(temp[1].Y, temp[0].Y);
  copy_conditional(temp[0].Y, temp[1].Y, wvalue & 1);
  
  ecp_sm2z256_point_add(r, r, &temp[0]);
  
  ret = ERR_TENCENTSM_OK;
err:
  tcsm_unlock_temp_bn(group->ctx, tmp_scalar_index);
  tcsm_tc_secure_free(table_storage);
  tcsm_tc_secure_free(p_str);
  tcsm_tc_secure_free(scalars);
  return ret;
}

int ecp_sm2z256_points_mul_for_point(tc_ecc_group_st *group, tc_ecc_point_st* r,P256_POINT *point, tc_bn_st *scalar)
{
  int ret = ERR_ECC_POINTS_MUL;
  ALIGN32 union {
    P256_POINT p;
    P256_POINT_AFFINE a;
  } t, p;

  P256_POINT *out = &t.p;
  out = &p.p;
  
  if (ecp_sm2z256_windowed_mul_for_point(group, out, scalar, point) != ERR_TENCENTSM_OK)
    goto err;

  //to afn
  BN_ULONG z_inv2[P256_LIMBS];
  BN_ULONG z_inv3[P256_LIMBS];
  BN_ULONG x_aff[P256_LIMBS];
  BN_ULONG y_aff[P256_LIMBS];
  BN_ULONG x_ret[P256_LIMBS], y_ret[P256_LIMBS];
  
  ecp_sm2z256_mod_inverse(z_inv3, p.p.Z);
  ecp_sm2z256_sqr_mont(z_inv2, z_inv3);
  ecp_sm2z256_mul_mont(x_aff, z_inv2, p.p.X);
  
  ecp_sm2z256_from_mont(x_ret, x_aff);
  tcsm_tc_bn_set_words(r->x, x_ret, P256_LIMBS);
  
  ecp_sm2z256_mul_mont(z_inv3, z_inv3, z_inv2);
  ecp_sm2z256_mul_mont(y_aff, z_inv3, p.p.Y);
  ecp_sm2z256_from_mont(y_ret, y_aff);
  tcsm_tc_bn_set_words(r->y, y_ret, P256_LIMBS);
  
  ret = ERR_TENCENTSM_OK;
err:
  return ret;
}
/*
int ecp_sm2z256_windowed_mul(tc_ecc_group_st *group,
                             P256_POINT *r,
                             tc_bn_st *scalar[],
                             tc_ecc_jcb_point_st *point[],
                             size_t num)
{
  size_t i;
  int j, ret = 0;
  unsigned int idx;
  unsigned char (*p_str)[33] = NULL;
  const unsigned int window_size = 5;
  const unsigned int mask = (1 << (window_size + 1)) - 1;
  unsigned int wvalue;
  P256_POINT *temp;
  const tc_bn_st **scalars = NULL;
  P256_POINT (*table)[16] = NULL;
  void *table_storage = tcsm_tc_secure_malloc((num * 16 + 5) * sizeof(P256_POINT) + 64);
  
  p_str = tcsm_tc_secure_malloc(num * 33 * sizeof(unsigned char));
  scalars = tcsm_tc_secure_malloc(num * sizeof(tc_bn_st *));
  table = (void *)ALIGNPTR(table_storage, 64);
  temp = (P256_POINT *)(table + num);
  
  int tmp_scalar_index;
  tc_bn_t *tmp_scalar = tcsm_lock_temp_bn(group->ctx,&tmp_scalar_index);
  
  for (i = 0; i < num; i++) {
    P256_POINT *row = table[i];

    if ((tcsm_tc_bn_num_bits(scalar[i]) > 256) || tcsm_tc_bn_is_negtive(scalar[i])) {
      tcsm_tc_bn_mod(*tmp_scalar, scalar[i], group->p);
      scalars[i] = *tmp_scalar;
      
    } else
      scalars[i] = scalar[i];
    
    for (j = 0; j < scalar[i]->val->_mp_size * BN_BYTES; j += BN_BYTES) {
      BN_ULONG d = (scalar[i])->val->_mp_d[j / BN_BYTES];
      
      p_str[i][j + 0] = (unsigned char)d;
      p_str[i][j + 1] = (unsigned char)(d >> 8);
      p_str[i][j + 2] = (unsigned char)(d >> 16);
      p_str[i][j + 3] = (unsigned char)(d >>= 24);
      if (BN_BYTES == 8) {
        d >>= 8;
        p_str[i][j + 4] = (unsigned char)d;
        p_str[i][j + 5] = (unsigned char)(d >> 8);
        p_str[i][j + 6] = (unsigned char)(d >> 16);
        p_str[i][j + 7] = (unsigned char)(d >> 24);
      }
    }
    for (; j < 33; j++)
      p_str[i][j] = 0;
    
    for (int j = 0; j < P256_LIMBS; j++) {
      temp[0].X[j] = point[i]->x->val->_mp_d[j];
      temp[0].Y[j] = point[i]->y->val->_mp_d[j];
    }
    
    temp[0].Z[0] = 1;
    temp[0].Z[1] = 0;
    temp[0].Z[2] = 0;
    temp[0].Z[3] = 0;
    
    ecp_sm2z256_to_mont(temp[0].X, temp[0].X);
    ecp_sm2z256_to_mont(temp[0].Y, temp[0].Y);
    ecp_sm2z256_to_mont(temp[0].Z, temp[0].Z);
    
    ecp_sm2z256_scatter_w5  (row, &temp[0], 1);
    ecp_sm2z256_point_double(&temp[1], &temp[0]);
    ecp_sm2z256_scatter_w5  (row, &temp[1], 2);
    ecp_sm2z256_point_add   (&temp[2], &temp[1], &temp[0]);
    ecp_sm2z256_scatter_w5  (row, &temp[2], 3);
    ecp_sm2z256_point_double(&temp[1], &temp[1]);
    ecp_sm2z256_scatter_w5  (row, &temp[1], 4);
    ecp_sm2z256_point_double(&temp[2], &temp[2]);
    ecp_sm2z256_scatter_w5  (row, &temp[2], 6);
    ecp_sm2z256_point_add   (&temp[3], &temp[1], &temp[0]);
    ecp_sm2z256_scatter_w5  (row, &temp[3], 5);
    ecp_sm2z256_point_add   (&temp[4], &temp[2], &temp[0]);
    ecp_sm2z256_scatter_w5  (row, &temp[4], 7);
    ecp_sm2z256_point_double(&temp[1], &temp[1]);
    ecp_sm2z256_scatter_w5  (row, &temp[1], 8);
    ecp_sm2z256_point_double(&temp[2], &temp[2]);
    ecp_sm2z256_scatter_w5  (row, &temp[2], 12);
    ecp_sm2z256_point_double(&temp[3], &temp[3]);
    ecp_sm2z256_scatter_w5  (row, &temp[3], 10);
    ecp_sm2z256_point_double(&temp[4], &temp[4]);
    ecp_sm2z256_scatter_w5  (row, &temp[4], 14);
    ecp_sm2z256_point_add   (&temp[2], &temp[2], &temp[0]);
    ecp_sm2z256_scatter_w5  (row, &temp[2], 13);
    ecp_sm2z256_point_add   (&temp[3], &temp[3], &temp[0]);
    ecp_sm2z256_scatter_w5  (row, &temp[3], 11);
    ecp_sm2z256_point_add   (&temp[4], &temp[4], &temp[0]);
    ecp_sm2z256_scatter_w5  (row, &temp[4], 15);
    ecp_sm2z256_point_add   (&temp[2], &temp[1], &temp[0]);
    ecp_sm2z256_scatter_w5  (row, &temp[2], 9);
    ecp_sm2z256_point_double(&temp[1], &temp[1]);
    ecp_sm2z256_scatter_w5  (row, &temp[1], 16);
  }
  
  idx = 255;
  
  wvalue = p_str[0][(idx - 1) / 8];
  wvalue = (wvalue >> ((idx - 1) % 8)) & mask;
  
  ecp_sm2z256_gather_w5(&temp[0], table[0], _booth_recode_w5(wvalue) >> 1);
  memcpy(r, &temp[0], sizeof(temp[0]));
  
  while (idx >= 5) {
    for (i = (idx == 255 ? 1 : 0); i < num; i++) {
      unsigned int off = (idx - 1) / 8;
      
      wvalue = p_str[i][off] | p_str[i][off + 1] << 8;
      wvalue = (wvalue >> ((idx - 1) % 8)) & mask;
      
      wvalue = _booth_recode_w5(wvalue);
      
      ecp_sm2z256_gather_w5(&temp[0], table[i], wvalue >> 1);
      
      ecp_sm2z256_neg(temp[1].Y, temp[0].Y);
      copy_conditional(temp[0].Y, temp[1].Y, (wvalue & 1));
      
      ecp_sm2z256_point_add(r, r, &temp[0]);
    }
    
    idx -= window_size;
    
    ecp_sm2z256_point_double(r, r);
    ecp_sm2z256_point_double(r, r);
    ecp_sm2z256_point_double(r, r);
    ecp_sm2z256_point_double(r, r);
    ecp_sm2z256_point_double(r, r);
  }
  
  for (i = 0; i < num; i++) {
    wvalue = p_str[i][0];
    wvalue = (wvalue << 1) & mask;
    
    wvalue = _booth_recode_w5(wvalue);
    
    ecp_sm2z256_gather_w5(&temp[0], table[i], wvalue >> 1);
    
    ecp_sm2z256_neg(temp[1].Y, temp[0].Y);
    copy_conditional(temp[0].Y, temp[1].Y, wvalue & 1);
    
    ecp_sm2z256_point_add(r, r, &temp[0]);
  }
  
  ret = 1;
err:
  tcsm_unlock_temp_bn(group->ctx, tmp_scalar_index);
  tcsm_tc_secure_free(table_storage);
  tcsm_tc_secure_free(p_str);
  tcsm_tc_secure_free(scalars);
  return ret;
}
 */

/*
int ecp_sm2z256_points_mul(tc_ecc_group_st *group, tc_ecc_point_st* r, tc_bn_st* scalar,
                           size_t num, tc_ecc_jcb_point_st *points[], tc_bn_st *scalars[])
{
  int i = 0, ret = -1, p_is_infinity = 0;
  
  unsigned char p_str[33] = { 0 };
  const PRECOMP256_ROW *preComputedTable = NULL;
  
  const tc_bn_st **new_scalars = NULL;
  const tc_ecc_jcb_point_st **new_points = NULL;
  unsigned int idx = 0;
  const unsigned int window_size = 7;
  const unsigned int mask = (1 << (window_size + 1)) - 1;
  unsigned int wvalue;
  ALIGN32 union {
    P256_POINT p;
    P256_POINT_AFFINE a;
  } t, p;
  int tmp_scalar_index;
  tc_bn_t *tmp_scalar = tcsm_lock_temp_bn(group->ctx,&tmp_scalar_index);
  
  if (scalar) {
    
    if (preComputedTable == NULL) {
      preComputedTable = ecp_sm2z256_precomputed;
    }
    
    if (preComputedTable) {
      if ((tcsm_tc_bn_num_bits(scalar) > 256)
          || tcsm_tc_bn_is_negtive(scalar)) {
        
        tcsm_tc_bn_mod(*tmp_scalar, scalar, group->p);
        scalar = *tmp_scalar;
      }
      
      for (i = 0; i < scalar->val->_mp_size * BN_BYTES; i += BN_BYTES) {
        
        BN_ULONG d = scalar->val->_mp_d[i / BN_BYTES];
        
        p_str[i + 0] = (unsigned char)d;
        p_str[i + 1] = (unsigned char)(d >> 8);
        p_str[i + 2] = (unsigned char)(d >> 16);
        p_str[i + 3] = (unsigned char)(d >>= 24);
        if (BN_BYTES == 8) {
          d >>= 8;
          p_str[i + 4] = (unsigned char)d;
          p_str[i + 5] = (unsigned char)(d >> 8);
          p_str[i + 6] = (unsigned char)(d >> 16);
          p_str[i + 7] = (unsigned char)(d >> 24);
        }
      }
      
      for (; i < 33; i++)
        p_str[i] = 0;
      
      BN_ULONG infty;
 
      wvalue = (p_str[0] << 1) & mask;
      idx += window_size;
      
      wvalue = _booth_recode_w7(wvalue);
      
      ecp_sm2z256_gather_w7(&p.a, preComputedTable[0],
                            wvalue >> 1);
      
      ecp_sm2z256_neg(p.p.Z, p.p.Y);
      copy_conditional(p.p.Y, p.p.Z, wvalue & 1);
      
      infty = (p.p.X[0] | p.p.X[1] | p.p.X[2] | p.p.X[3] |
               p.p.Y[0] | p.p.Y[1] | p.p.Y[2] | p.p.Y[3]);
      
      infty = 0 - is_zero(infty);
      infty = ~infty;
      
      p.p.Z[0] = ONE[0] & infty;
      p.p.Z[1] = ONE[1] & infty;
      p.p.Z[2] = ONE[2] & infty;
      p.p.Z[3] = ONE[3] & infty;
      
      for (i = 1; i < 37; i++) {
        unsigned int off = (idx - 1) / 8;
        wvalue = p_str[off] | p_str[off + 1] << 8;
        wvalue = (wvalue >> ((idx - 1) % 8)) & mask;
        idx += window_size;
        
        wvalue = _booth_recode_w7(wvalue);
        
        ecp_sm2z256_gather_w7(&t.a,
                              preComputedTable[i], wvalue >> 1);
        
        ecp_sm2z256_neg(t.p.Z, t.a.Y);
        copy_conditional(t.a.Y, t.p.Z, wvalue & 1);
        
        ecp_sm2z256_point_add_affine(&p.p, &p.p, &t.a);
      }
    } else {
      p_is_infinity = 1;
    }
  } else
    p_is_infinity = 1;
  
  if (num) {
    P256_POINT *out = &t.p;
    if (p_is_infinity)
      out = &p.p;
    
    if (!ecp_sm2z256_windowed_mul(group, out, scalars, points, num))
      goto err;
    
    if (!p_is_infinity)
      ecp_sm2z256_point_add(&p.p, &p.p, out);
  }
  
  //to afn
  BN_ULONG z_inv2[P256_LIMBS];
  BN_ULONG z_inv3[P256_LIMBS];
  BN_ULONG x_aff[P256_LIMBS];
  BN_ULONG y_aff[P256_LIMBS];
  BN_ULONG x_ret[P256_LIMBS], y_ret[P256_LIMBS];
  
  ecp_sm2z256_mod_inverse(z_inv3, p.p.Z);
  ecp_sm2z256_sqr_mont(z_inv2, z_inv3);
  ecp_sm2z256_mul_mont(x_aff, z_inv2, p.p.X);
  
  ecp_sm2z256_from_mont(x_ret, x_aff);
  tcsm_tc_bn_set_words(r->x, x_ret, P256_LIMBS);
  
  ecp_sm2z256_mul_mont(z_inv3, z_inv3, z_inv2);
  ecp_sm2z256_mul_mont(y_aff, z_inv3, p.p.Y);
  ecp_sm2z256_from_mont(y_ret, y_aff);
  tcsm_tc_bn_set_words(r->y, y_ret, P256_LIMBS);
  
  ret = 0;
err:
  tcsm_unlock_temp_bn(group->ctx, tmp_scalar_index);
  tcsm_tc_secure_free(new_points);
  tcsm_tc_secure_free(new_scalars);
  return ret;
}
*/
#endif

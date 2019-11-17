/*
 * quickselect.cpp
 *
 * Code generation for function 'quickselect'
 *
 */

/* Include files */
#include "quickselect.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"
#include "sort3.h"

/* Function Declarations */
static int div_nzp_s32(int numerator, int denominator);
static void med3(float v[7680], int nv, int ia, int ib);
static void medmed(float v[7680], int nv, int ia);
static int pivot(float v[7680], int *ip, int ia, int ib);
static int thirdOfFive(const float v[7680], int ia, int ib);

/* Function Definitions */
static int div_nzp_s32(int numerator, int denominator) {
  int quotient;
  unsigned int absNumerator;
  unsigned int absDenominator;
  boolean_T quotientNeedsNegation;
  if (numerator < 0) {
    absNumerator = ~(unsigned int)numerator + 1U;
  } else {
    absNumerator = (unsigned int)numerator;
  }

  if (denominator < 0) {
    absDenominator = ~(unsigned int)denominator + 1U;
  } else {
    absDenominator = (unsigned int)denominator;
  }

  quotientNeedsNegation = ((numerator < 0) != (denominator < 0));
  absNumerator /= absDenominator;
  if (quotientNeedsNegation) {
    quotient = -(int)absNumerator;
  } else {
    quotient = (int)absNumerator;
  }

  return quotient;
}

static void med3(float v[7680], int nv, int ia, int ib) {
  int ic;
  int unusedU0;
  int jc;
  int unusedU1;
  float tmp;
  if (nv >= 3) {
    ic = nv - 1;
    ic = ia + (ic >> 1);
    sort3(ia, v[ia - 1], ic, v[ic - 1], ib, v[ib - 1], &unusedU0, &jc,
          &unusedU1);
    if (jc > ia) {
      tmp = v[ia - 1];
      v[ia - 1] = v[jc - 1];
      v[jc - 1] = tmp;
    }
  }
}

static void medmed(float v[7680], int nv, int ia) {
  int ngroupsof5;
  int nlast;
  int k;
  int i1;
  int destidx;
  float tmp;
  while (nv > 1) {
    ngroupsof5 = div_nzp_s32(nv, 5);
    nlast = nv - ngroupsof5 * 5;
    nv = ngroupsof5;
    for (k = 0; k + 1 <= ngroupsof5; k++) {
      i1 = ia + k * 5;
      i1 = thirdOfFive(v, i1, i1 + 4) - 1;
      destidx = (ia + k) - 1;
      tmp = v[destidx];
      v[destidx] = v[i1];
      v[i1] = tmp;
    }

    if (nlast > 0) {
      i1 = ia + ngroupsof5 * 5;
      i1 = thirdOfFive(v, i1, (i1 + nlast) - 1) - 1;
      destidx = (ia + ngroupsof5) - 1;
      tmp = v[destidx];
      v[destidx] = v[i1];
      v[i1] = tmp;
      nv = ngroupsof5 + 1;
    }
  }
}

static int pivot(float v[7680], int *ip, int ia, int ib) {
  int reps;
  float vref;
  int k;
  float vk;
  vref = v[*ip - 1];
  v[*ip - 1] = v[ib - 1];
  v[ib - 1] = vref;
  *ip = ia;
  reps = 0;
  for (k = ia - 1; k + 1 < ib; k++) {
    vk = v[k];
    if (v[k] == vref) {
      v[k] = v[*ip - 1];
      v[*ip - 1] = vk;
      reps++;
      (*ip)++;
    } else {
      if (v[k] < vref) {
        v[k] = v[*ip - 1];
        v[*ip - 1] = vk;
        (*ip)++;
      }
    }
  }

  v[ib - 1] = v[*ip - 1];
  v[*ip - 1] = vref;
  return reps;
}

static int thirdOfFive(const float v[7680], int ia, int ib) {
  int im;
  int b_j1;
  int j3;
  int j4;
  int j5;
  float v4;
  float v5;
  if ((ia == ib) || (ia + 1 == ib)) {
    im = ia;
  } else if ((ia + 2 == ib) || (ia + 3 == ib)) {
    sort3(ia, v[ia - 1], ia + 1, v[ia], ia + 2, v[ia + 1], &b_j1, &im, &j3);
  } else {
    sort3(ia, v[ia - 1], ia + 1, v[ia], ia + 2, v[ia + 1], &b_j1, &im, &j3);
    j4 = ia;
    j5 = ia + 1;
    v4 = v[ia + 2];
    v5 = v[ia + 3];
    if (v[ia + 3] < v[ia + 2]) {
      j4 = ia + 1;
      j5 = ia;
      v5 = v[ia + 2];
      v4 = v[ia + 3];
    }

    if (v5 < v[b_j1 - 1]) {
      im = b_j1;
    } else if (v5 < v[im - 1]) {
      im = j5 + 3;
    } else {
      if (!(v4 < v[im - 1])) {
        if (v4 < v[j3 - 1]) {
          im = j4 + 3;
        } else {
          im = j3;
        }
      }
    }
  }

  return im;
}

void quickselect(float v[7680], int n, int vlen, float *vn, int *nfirst,
                 int *nlast) {
  int ipiv;
  int ia;
  int ib;
  int oldnv;
  boolean_T checkspeed;
  boolean_T isslow;
  boolean_T exitg1;
  int reps;
  boolean_T guard1 = false;
  if (n > vlen) {
    *vn = ((real32_T)rtNaN);
    *nfirst = 0;
    *nlast = 0;
  } else {
    ipiv = n;
    ia = 1;
    ib = vlen;
    *nfirst = 1;
    *nlast = vlen;
    oldnv = vlen;
    checkspeed = false;
    isslow = false;
    exitg1 = false;
    while ((!exitg1) && (ia < ib)) {
      reps = pivot(v, &ipiv, ia, ib);
      *nlast = ipiv;
      guard1 = false;
      if (n <= ipiv) {
        *nfirst = ipiv - reps;
        if (n >= *nfirst) {
          exitg1 = true;
        } else {
          ib = ipiv - 1;
          guard1 = true;
        }
      } else {
        ia = ipiv + 1;
        guard1 = true;
      }

      if (guard1) {
        reps = (ib - ia) + 1;
        if (checkspeed) {
          isslow = (reps > ((oldnv + (oldnv < 0)) >> 1));
          oldnv = reps;
        }

        checkspeed = !checkspeed;
        if (isslow) {
          medmed(v, reps, ia);
        } else {
          med3(v, reps, ia, ib);
        }

        ipiv = ia;
        *nfirst = ia;
        *nlast = ib;
      }
    }

    *vn = v[*nlast - 1];
  }
}

/* End of code generation (quickselect.cpp) */

/*
 * sum.cpp
 *
 * Code generation for function 'sum'
 *
 */

/* Include files */
#include "sum.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"

/* Function Definitions */
double b_sum(const double x[240]) {
  double y;
  int k;
  y = x[0];
  for (k = 0; k < 239; k++) {
    y += x[k + 1];
  }

  return y;
}

void c_sum(const boolean_T x[7680], double y[240]) {
  int i;
  int xpageoffset;
  int k;
  for (i = 0; i < 240; i++) {
    xpageoffset = i << 5;
    y[i] = x[xpageoffset];
    for (k = 0; k < 31; k++) {
      y[i] += (double)x[(xpageoffset + k) + 1];
    }
  }
}

void sum(const boolean_T x[15360], double y[240]) {
  int i;
  int xpageoffset;
  int k;
  for (i = 0; i < 240; i++) {
    xpageoffset = i << 6;
    y[i] = x[xpageoffset];
    for (k = 0; k < 63; k++) {
      y[i] += (double)x[(xpageoffset + k) + 1];
    }
  }
}

/* End of code generation (sum.cpp) */

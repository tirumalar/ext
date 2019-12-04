/*
 * EESM_Coverage.cpp
 *
 * Code generation for function 'EESM_Coverage'
 *
 */

/* Include files */
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"
#include "sum.h"

/* Function Definitions */
void EESM_Coverage(const unsigned char M[30720], float C[3]) {
  int k;
  boolean_T b_M[15360];
  double x[240];
  int i0;
  double y;
  double b_y;

  /* Compute proportion of mask coverage of left half (lower eyelid region) of
   * flat */
  /* iris image. */
  for (k = 0; k < 240; k++) {
    for (i0 = 0; i0 < 64; i0++) {
      b_M[i0 + (k << 6)] = (M[i0 + (k << 6)] != 0);
    }
  }

  sum(b_M, x);
  y = x[0];
  for (k = 0; k < 239; k++) {
    y += x[k + 1];
  }

  y /= 64.0;
  C[2] = (float)(y / 240.0);

  /* Compute proportion of mask coverage of right half (upper eyelid region) of
   * flat */
  /* iris image. */
  for (k = 0; k < 240; k++) {
    for (i0 = 0; i0 < 64; i0++) {
      b_M[i0 + (k << 6)] = (M[i0 + ((240 + k) << 6)] != 0);
    }
  }

  sum(b_M, x);
  b_y = x[0];
  for (k = 0; k < 239; k++) {
    b_y += x[k + 1];
  }

  b_y /= 64.0;
  C[1] = (float)(b_y / 240.0);

  /* Compute proportion of mask coverage of total flat iris image. */
  C[0] = ((float)(b_y / 240.0) + (float)(y / 240.0)) / 2.0F;
}

/* End of code generation (EESM_Coverage.cpp) */

/*
 * convn.cpp
 *
 * Code generation for function 'convn'
 *
 */

/* Include files */
#include "convn.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"

/* Function Definitions */
void convn(const double A[8228], const double B[9], double C[8228]) {
  int j;
  int lastColA;
  int k;
  int b_j;
  int iC;
  int iA;
  int iB;
  int i;
  int firstRowA;
  int b_i;
  int a_length;
  int cidx;
  int r;
  memset(&C[0], 0, 8228U * sizeof(double));
  for (j = 0; j < 3; j++) {
    if (j + 241 < 242) {
      lastColA = 241;
    } else {
      lastColA = 242 - j;
    }

    for (k = (j < 1); k <= lastColA; k++) {
      if (j + k > 1) {
        b_j = (j + k) - 1;
      } else {
        b_j = 0;
      }

      iC = b_j * 34;
      iA = k * 34;
      iB = j * 3;
      for (i = 0; i < 3; i++) {
        firstRowA = (i < 1);
        if (i + 34 <= 34) {
          b_i = 34;
        } else {
          b_i = 35 - i;
        }

        a_length = b_i - firstRowA;
        firstRowA += iA;
        cidx = iC;
        for (r = 1; r <= a_length; r++) {
          C[cidx] += B[iB] * A[firstRowA];
          firstRowA++;
          cidx++;
        }

        iB++;
        if (i >= 1) {
          iC++;
        }
      }
    }
  }
}

/* End of code generation (convn.cpp) */

/*
 * abs.cpp
 *
 * Code generation for function 'abs'
 *
 */

/* Include files */
#include "abs.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"

/* Function Definitions */
void b_abs(const float x[7680], float y[7680]) {
  int k;
  for (k = 0; k < 7680; k++) {
    y[k] = std::abs(x[k]);
  }
}

/* End of code generation (abs.cpp) */

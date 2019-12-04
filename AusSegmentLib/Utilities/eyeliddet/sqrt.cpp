/*
 * sqrt.cpp
 *
 * Code generation for function 'sqrt'
 *
 */

/* Include files */
#include "sqrt.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"

/* Function Definitions */
void b_sqrt(float x[7680]) {
  int k;
  for (k = 0; k < 7680; k++) {
    x[k] = std::sqrt(x[k]);
  }
}

/* End of code generation (sqrt.cpp) */

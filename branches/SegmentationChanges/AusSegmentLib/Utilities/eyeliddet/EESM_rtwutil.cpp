/*
 * EESM_rtwutil.cpp
 *
 * Code generation for function 'EESM_rtwutil'
 *
 */

/* Include files */
#include "EESM_rtwutil.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"

/* Function Definitions */
double rt_roundd_snf(double u) {
  double y;
  if (std::abs(u) < 4.503599627370496E+15) {
    if (u >= 0.5) {
      y = std::floor(u + 0.5);
    } else if (u > -0.5) {
      y = u * 0.0;
    } else {
      y = std::ceil(u - 0.5);
    }
  } else {
    y = u;
  }

  return y;
}

/* End of code generation (EESM_rtwutil.cpp) */

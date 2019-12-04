/*
 * median.cpp
 *
 * Code generation for function 'median'
 *
 */

/* Include files */
#include "median.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "quickselect.h"
#include "rt_nonfinite.h"

/* Function Definitions */
float median(const float x[7680]) {
  float y;
  int k;
  int exitg1;
  float v[7680];
  int ilast;
  float b;
  int unusedU2;
  k = 1;
  do {
    exitg1 = 0;
    if (k < 7681) {
      if (rtIsNaNF(x[k - 1])) {
        y = ((real32_T)rtNaN);
        exitg1 = 1;
      } else {
        k++;
      }
    } else {
      memcpy(&v[0], &x[0], 7680U * sizeof(float));
      quickselect(v, 3841, 7680, &y, &k, &ilast);
      if (3840 < k) {
        quickselect(v, 3840, ilast - 1, &b, &k, &unusedU2);
        if (rtIsInfF(y) || rtIsInfF(b)) {
          y = (y + b) / 2.0F;
        } else {
          y += (b - y) / 2.0F;
        }
      }

      exitg1 = 1;
    }
  } while (exitg1 == 0);

  return y;
}

/* End of code generation (median.cpp) */

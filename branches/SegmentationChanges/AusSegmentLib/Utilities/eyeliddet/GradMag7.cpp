/*
 * GradMag7.cpp
 *
 * Code generation for function 'GradMag7'
 *
 */

/* Include files */
#include "GradMag7.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "abs.h"
#include "imfilter.h"
#include "median.h"
#include "rt_nonfinite.h"
#include "sqrt.h"

/* Function Definitions */
void GradMag7(EESMStackData *SD, const float J[7680], float gth,
              boolean_T R[7680]) {
  int i;
  int ixstart;
  int ix;
  float Med;
  float maxval[240];
  boolean_T exitg1;
  int b_ix;
  float fv0[7680];
  unsigned short r;
  unsigned short c;

  /*  */
  /* 2018/02/07 -- modified to minimize use of double-precsion floating point */
  /*  */
  /* Define Sobel kernel */
  /* Find vertical gradient magnitude. */
  imfilter(SD, J, SD->f2.J1);

  /* Find horizontal gradient magnitude. */
  b_imfilter(SD, J, SD->f2.J2);

  /* Computer total gradient magnitude and normalize to range of 0 to 1.0 */
  for (i = 0; i < 7680; i++) {
    SD->f2.J1[i] = SD->f2.J1[i] * SD->f2.J1[i] + SD->f2.J2[i] * SD->f2.J2[i];
  }

  b_sqrt(SD->f2.J1);
  for (i = 0; i < 240; i++) {
    ix = i << 5;
    ixstart = (i << 5) + 1;
    Med = SD->f2.J1[ix];
    if (rtIsNaNF(SD->f2.J1[ix])) {
      b_ix = ixstart + 1;
      exitg1 = false;
      while ((!exitg1) && (b_ix <= ix + 32)) {
        ixstart = b_ix;
        if (!rtIsNaNF(SD->f2.J1[b_ix - 1])) {
          Med = SD->f2.J1[b_ix - 1];
          exitg1 = true;
        } else {
          b_ix++;
        }
      }
    }

    if (ixstart < ix + 32) {
      while (ixstart + 1 <= ix + 32) {
        if (SD->f2.J1[ixstart] > Med) {
          Med = SD->f2.J1[ixstart];
        }

        ixstart++;
      }
    }

    maxval[i] = Med;
  }

  ixstart = 1;
  Med = maxval[0];
  if (rtIsNaNF(maxval[0])) {
    ix = 2;
    exitg1 = false;
    while ((!exitg1) && (ix < 241)) {
      ixstart = ix;
      if (!rtIsNaNF(maxval[ix - 1])) {
        Med = maxval[ix - 1];
        exitg1 = true;
      } else {
        ix++;
      }
    }
  }

  if (ixstart < 240) {
    while (ixstart + 1 < 241) {
      if (maxval[ixstart] > Med) {
        Med = maxval[ixstart];
      }

      ixstart++;
    }
  }

  for (i = 0; i < 7680; i++) {
    SD->f2.J1[i] /= Med;
  }

  /* imtool(Gmag_small); */
  /* Compute Median and Median Absolute Deviation (MAD). */
  Med = median(SD->f2.J1);

  /* Define threshold for high gradient magnitude */
  for (i = 0; i < 7680; i++) {
    SD->f2.J2[i] = SD->f2.J1[i] - Med;
  }

  b_abs(SD->f2.J2, fv0);
  for (i = 0; i < 7680; i++) {
    SD->f2.J2[i] = fv0[i];
    R[i] = false;
  }

  Med += gth * median(SD->f2.J2);

  /* Compute binary (logical) mask for high gradient magnitude. */
  for (r = 1; r < 33; r++) {
    for (c = 1; c < 241; c++) {
      if (SD->f2.J1[(r + ((c - 1) << 5)) - 1] > Med) {
        R[(r + ((c - 1) << 5)) - 1] = true;
      }
    }
  }

  /* imtool(R); */
}

/* End of code generation (GradMag7.cpp) */

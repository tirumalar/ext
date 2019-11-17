/*
 * NormIntensity2.cpp
 *
 * Code generation for function 'NormIntensity2'
 *
 */

/* Include files */
#include "NormIntensity2.h"
#include "EESM_Coverage.h"
#include "EESM_rtwutil.h"
#include "EESMask6.h"
#include "imhist.h"
#include "rt_nonfinite.h"

/* Function Definitions */
void NormIntensity2(const unsigned char I[7680], double xthresh,
                    unsigned char N[7680]) {
  double counts[256];
  int i;
  double d1;
  short b_counts[256];
  short i4;
  double y;
  short Sum;
  short DarkCount;
  short j;
  short b_j;
  boolean_T exitg1;
  short k;
  float IntensitySum;
  unsigned char u1;

  /* xthresh is proportion from 0 to 0.49 */
  /*  */
  /* 2018/02/07 -- modified to minimize use of double-precsion floating point */
  /*  */
  /* Compute histogram */
  imhist(I, counts);
  for (i = 0; i < 256; i++) {
    d1 = rt_roundd_snf(counts[i]);
    if (d1 < 32768.0) {
      if (d1 >= -32768.0) {
        i4 = (short)d1;
      } else {
        i4 = MIN_int16_T;
      }
    } else if (d1 >= 32768.0) {
      i4 = MAX_int16_T;
    } else {
      i4 = 0;
    }

    b_counts[i] = i4;
  }

  /* Zero counts of extreme dark (value = 0) and bright (value = 255) pixels. */
  b_counts[0] = 0;
  b_counts[255] = 0;

  /* Sum remaining counts. */
  y = 0.0;
  for (i = 0; i < 255; i++) {
    y += (double)b_counts[i + 1];
  }

  if (y < 32768.0) {
    if (y >= -32768.0) {
      Sum = (short)y;
    } else {
      Sum = MIN_int16_T;
    }
  } else {
    Sum = MAX_int16_T;
  }

  /* Find bin with the darkest xthresh of summed pixels to the left of it. */
  DarkCount = 0;
  j = 2;
  b_j = 2;
  exitg1 = false;
  while ((!exitg1) && (b_j < 257)) {
    j = b_j;
    d1 = rt_roundd_snf(xthresh * (double)Sum);
    if (d1 < 32768.0) {
      if (d1 >= -32768.0) {
        i4 = (short)d1;
      } else {
        i4 = MIN_int16_T;
      }
    } else if (d1 >= 32768.0) {
      i4 = MAX_int16_T;
    } else {
      i4 = 0;
    }

    if (DarkCount < i4) {
      i = DarkCount + b_counts[b_j - 1];
      if (i > 32767) {
        i = 32767;
      } else {
        if (i < -32768) {
          i = -32768;
        }
      }

      DarkCount = (short)i;
      b_j++;
    } else {
      exitg1 = true;
    }
  }

  /* Find bin with the brightest xthresh of summed pixels to the right of it. */
  DarkCount = 0;
  k = 255;
  b_j = 255;
  exitg1 = false;
  while ((!exitg1) && (b_j > 0)) {
    k = b_j;
    d1 = rt_roundd_snf(xthresh * (double)Sum);
    if (d1 < 32768.0) {
      if (d1 >= -32768.0) {
        i4 = (short)d1;
      } else {
        i4 = MIN_int16_T;
      }
    } else if (d1 >= 32768.0) {
      i4 = MAX_int16_T;
    } else {
      i4 = 0;
    }

    if (DarkCount < i4) {
      i = DarkCount + b_counts[b_j - 1];
      if (i > 32767) {
        i = 32767;
      } else {
        if (i < -32768) {
          i = -32768;
        }
      }

      DarkCount = (short)i;
      b_j--;
    } else {
      exitg1 = true;
    }
  }

  /* Compute the mean of the (1 - 2*xthresh) of summed pixels having mid-range
   * values. */
  DarkCount = 0;
  IntensitySum = 0.0F;
  while (j <= k) {
    i = DarkCount + b_counts[j - 1];
    if (i > 32767) {
      i = 32767;
    } else {
      if (i < -32768) {
        i = -32768;
      }
    }

    DarkCount = (short)i;
    IntensitySum += (float)((j - 1) * b_counts[j - 1]);
    j++;
  }

  /* Normalize image to make 127 the mean of mid-range pixels. */
  y = 127.0 / (IntensitySum / (float)DarkCount);
  for (i = 0; i < 7680; i++) {
    d1 = rt_roundd_snf(y * (double)I[i]);
    if (d1 < 256.0) {
      if (d1 >= 0.0) {
        u1 = (unsigned char)d1;
      } else {
        u1 = 0;
      }
    } else if (d1 >= 256.0) {
      u1 = MAX_uint8_T;
    } else {
      u1 = 0;
    }

    N[i] = u1;
  }
}

/* End of code generation (NormIntensity2.cpp) */

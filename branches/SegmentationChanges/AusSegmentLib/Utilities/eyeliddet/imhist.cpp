/*
 * imhist.cpp
 *
 * Code generation for function 'imhist'
 *
 */

/* Include files */
#include "imhist.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"

/* Function Definitions */
void imhist(const unsigned char varargin_1[7680], double yout[256]) {
  double localBins1[256];
  double localBins2[256];
  double localBins3[256];
  int i;
  memset(&yout[0], 0, sizeof(double) << 8);
  memset(&localBins1[0], 0, sizeof(double) << 8);
  memset(&localBins2[0], 0, sizeof(double) << 8);
  memset(&localBins3[0], 0, sizeof(double) << 8);
  for (i = 0; i + 4 <= 7680; i += 4) {
    localBins1[varargin_1[i]]++;
    localBins2[varargin_1[i + 1]]++;
    localBins3[varargin_1[i + 2]]++;
    yout[varargin_1[i + 3]]++;
  }

  while (i + 1 <= 7680) {
    yout[varargin_1[i]]++;
    i++;
  }

  for (i = 0; i < 256; i++) {
    yout[i] = ((yout[i] + localBins1[i]) + localBins2[i]) + localBins3[i];
  }
}

/* End of code generation (imhist.cpp) */

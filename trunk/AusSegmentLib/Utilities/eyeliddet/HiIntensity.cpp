/*
 * HiIntensity.cpp
 *
 * Code generation for function 'HiIntensity'
 *
 */

/* Include files */
#include "HiIntensity.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"

/* Function Definitions */
void HiIntensity(unsigned char I[3840], unsigned char k) {
  unsigned short i;
  unsigned short j;
  for (i = 1; i < 33; i++) {
    for (j = 1; j < 121; j++) {
      if (I[(i + ((j - 1) << 5)) - 1] > k) {
        I[(i + ((j - 1) << 5)) - 1] = MAX_uint8_T;
      } else {
        I[(i + ((j - 1) << 5)) - 1] = 0;
      }
    }
  }
}

void b_HiIntensity(unsigned char I[7680], unsigned char k) {
  unsigned short i;
  unsigned short j;
  for (i = 1; i < 33; i++) {
    for (j = 1; j < 241; j++) {
      if (I[(i + ((j - 1) << 5)) - 1] > k) {
        I[(i + ((j - 1) << 5)) - 1] = MAX_uint8_T;
      } else {
        I[(i + ((j - 1) << 5)) - 1] = 0;
      }
    }
  }
}

/* End of code generation (HiIntensity.cpp) */

/*
 * sort3.cpp
 *
 * Code generation for function 'sort3'
 *
 */

/* Include files */
#include "sort3.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"

/* Function Definitions */
void sort3(int i1, float v1, int i2, float v2, int i3, float v3, int *b_j1,
           int *j2, int *j3) {
  if (v1 < v2) {
    if (v2 < v3) {
      *b_j1 = i1;
      *j2 = i2;
      *j3 = i3;
    } else if (v1 < v3) {
      *b_j1 = i1;
      *j2 = i3;
      *j3 = i2;
    } else {
      *b_j1 = i3;
      *j2 = i1;
      *j3 = i2;
    }
  } else if (v1 < v3) {
    *b_j1 = i2;
    *j2 = i1;
    *j3 = i3;
  } else if (v2 < v3) {
    *b_j1 = i2;
    *j2 = i3;
    *j3 = i1;
  } else {
    *b_j1 = i3;
    *j2 = i2;
    *j3 = i1;
  }
}

/* End of code generation (sort3.cpp) */

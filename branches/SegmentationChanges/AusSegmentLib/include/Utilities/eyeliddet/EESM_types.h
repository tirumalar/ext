/*
 * EESM_types.h
 *
 * Code generation for function 'EESM_Coverage'
 *
 */

#ifndef EESM_TYPES_H
#define EESM_TYPES_H

/* Include files */
#include "rtwtypes.h"

/* Type Definitions */
typedef struct {
  union {
    struct {
      double a[8228];
      double dv12[8228];
    } f0;

    struct {
      double a[8228];
      double dv10[8228];
    } f1;
  } u1;

  struct {
    float J1[7680];
    float J2[7680];
  } f2;

  struct {
    double dv8[7680];
    unsigned char I[30720];
    float K[7680];
    boolean_T EESM[30720];
  } f3;
} EESMStackData;

typedef struct {
  unsigned char f1[7680];
} cell_wrap_0;

typedef struct {
  cell_wrap_0 f1[30];
} cell_wrap_1;

typedef struct {
  boolean_T f1[7680];
} cell_wrap_2;

typedef struct {
  cell_wrap_2 f1[30];
} cell_wrap_3;

#ifndef struct_emxArray_int32_T
#define struct_emxArray_int32_T

struct emxArray_int32_T {
  int *data;
  int *size;
  int allocatedSize;
  int numDimensions;
  boolean_T canFreeData;
};

#endif /*struct_emxArray_int32_T*/
#endif

/* End of code generation (EESM_types.h) */

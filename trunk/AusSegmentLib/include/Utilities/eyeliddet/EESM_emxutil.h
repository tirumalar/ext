/*
 * EESM_emxutil.h
 *
 * Code generation for function 'EESM_emxutil'
 *
 */

#ifndef EESM_EMXUTIL_H
#define EESM_EMXUTIL_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include "EESM_types.h"
#include "rt_nonfinite.h"
#include "rtwtypes.h"

/* Function Declarations */
extern void emxEnsureCapacity_int32_T(emxArray_int32_T *emxArray, int oldNumel);
extern void emxFree_int32_T(emxArray_int32_T **pEmxArray);
extern void emxInit_int32_T(emxArray_int32_T **pEmxArray, int numDimensions);

#endif

/* End of code generation (EESM_emxutil.h) */

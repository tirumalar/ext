/*
 * quickselect.h
 *
 * Code generation for function 'quickselect'
 *
 */

#ifndef QUICKSELECT_H
#define QUICKSELECT_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include "EESM_types.h"
#include "rt_nonfinite.h"
#include "rtwtypes.h"

/* Function Declarations */
extern void quickselect(float v[7680], int n, int vlen, float *vn, int *nfirst,
                        int *nlast);

#endif

/* End of code generation (quickselect.h) */

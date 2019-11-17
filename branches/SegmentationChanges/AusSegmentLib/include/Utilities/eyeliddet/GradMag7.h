/*
 * GradMag7.h
 *
 * Code generation for function 'GradMag7'
 *
 */

#ifndef GRADMAG7_H
#define GRADMAG7_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include "EESM_types.h"
#include "rt_nonfinite.h"
#include "rtwtypes.h"

/* Function Declarations */
extern void GradMag7(EESMStackData *SD, const float J[7680], float gth,
                     boolean_T R[7680]);

#endif

/* End of code generation (GradMag7.h) */

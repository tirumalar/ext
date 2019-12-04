/*
 * imfilter.h
 *
 * Code generation for function 'imfilter'
 *
 */

#ifndef IMFILTER_H
#define IMFILTER_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include "EESM_types.h"
#include "rt_nonfinite.h"
#include "rtwtypes.h"

/* Function Declarations */
extern void b_imfilter(EESMStackData *SD, const float varargin_1[7680],
                       float b[7680]);
extern void imfilter(EESMStackData *SD, const float varargin_1[7680],
                     float b[7680]);

#endif

/* End of code generation (imfilter.h) */

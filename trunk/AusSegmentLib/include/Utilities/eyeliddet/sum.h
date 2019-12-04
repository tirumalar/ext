/*
 * sum.h
 *
 * Code generation for function 'sum'
 *
 */

#ifndef SUM_H
#define SUM_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include "EESM_types.h"
#include "rt_nonfinite.h"
#include "rtwtypes.h"

/* Function Declarations */
extern double b_sum(const double x[240]);
extern void c_sum(const boolean_T x[7680], double y[240]);
extern void sum(const boolean_T x[15360], double y[240]);

#endif

/* End of code generation (sum.h) */

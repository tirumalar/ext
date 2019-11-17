/*
 * EESMask6.h
 *
 * Code generation for function 'EESMask6'
 *
 */

#ifndef EESMASK6_H
#define EESMASK6_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include "EESM_types.h"
#include "rt_nonfinite.h"
#include "rtwtypes.h"

/* Function Declarations */
extern void EESMask6(EESMStackData *SD, const unsigned char I[30720],
                     const cell_wrap_1 CMs[2], const cell_wrap_3 CMb[2],
                     double xi_disgard, float gth, unsigned char CML_Bth,
                     unsigned char CML_Dth, unsigned char CMU_Bth,
                     unsigned char CMU_Dth, float cthU, float cthL,
                     float ArcthU, float ArcthL, unsigned char Dth,
                     unsigned char Bth, boolean_T diag, float *SpecCov,
                     unsigned char EESM[30720]);

#endif

/* End of code generation (EESMask6.h) */

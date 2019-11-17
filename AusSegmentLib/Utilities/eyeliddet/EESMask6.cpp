/*
 * EESMask6.cpp
 *
 * Code generation for function 'EESMask6'
 *
 */

/* Include files */
#include "EESMask6.h"
#include "EESM_Coverage.h"
#include "EESM_rtwutil.h"
#include "GradMag7.h"
#include "HiIntensity.h"
#include "LowIntensity.h"
#include "NormIntensity2.h"
#include "bwlabel.h"
#include "rt_nonfinite.h"
#include "sum.h"

/* Function Definitions */
void EESMask6(EESMStackData *SD, const unsigned char I[30720],
              const cell_wrap_1 CMs[2], const cell_wrap_3 CMb[2],
              double xi_disgard, float gth, unsigned char CML_Bth,
              unsigned char CML_Dth, unsigned char CMU_Bth,
              unsigned char CMU_Dth, float cthU, float cthL, float ArcthU,
              float ArcthL, unsigned char Dth, unsigned char Bth, boolean_T,
              float *SpecCov, unsigned char EESM[30720]) {
  unsigned short i;
  unsigned short j;
  int i1;
  unsigned char I_smallgray[7680];
  unsigned char K[7680];
  unsigned short Count;
  boolean_T GM[7680];
  unsigned char uv0[3840];
  int i2;
  unsigned char uv1[3840];
  unsigned char uv2[3840];
  unsigned char uv3[3840];
  int i3;
  boolean_T CM[7680];
  boolean_T b_CM[7680];
  unsigned char BestMask;
  unsigned char b_i;
  unsigned char CMU[7680];
  double dv0[240];
  double dv1[240];
  unsigned char BestArc;
  double dv2[240];
  boolean_T Arc;
  boolean_T b_Arc[7680];
  double dv3[240];
  unsigned char CML[7680];
  double dv4[240];
  double dv5[240];
  double dv6[240];
  double dv7[240];
  double d0;
  unsigned short L[7680];
  unsigned int u0;

  /*  */
  /*  Eyelid and Eyelash Segmentation Mask (EESM) Calculation -- uses bright
   * pixels */
  /*  (eyelid), dark pixels (eyelashes), and high-gradient pixels (for the fast
   */
  /*  transitions between eyelid and eyelash regions) as the primary */
  /*  discriminators */
  /*   */
  /*  2018/02/07 -- modified to minimize use of double-precsion floating point
   */
  /*   */
  /*  2018/02/27 -- modified to return SpecCov, the proportion of I that is */
  /*  covered by specular reflection of illumination. */
  /*   */
  /*  Input parameters: */
  /*   */
  /*  I is 64x480 flat iris image. */
  /*   */
  /*  CMs is a data structure (2-D Matlab cell array) containing circular masks
   * for matching to upper */
  /*  and lower eyelid occlusions. */
  /*   */
  /*  CMb is the logical or binary version of CMs. */
  /*  */
  /*  xi_disgard is proportion of dark or bright pixels that are disregarded */
  /*  for intensity normalization. */
  /*  */
  /*  gth is a threshold for gradient magnitude. */
  /*  */
  /*  CML_Bth is a threshlod for inclusion of bright pixels in finding a */
  /*  circular mask for the lower eyelid. */
  /*  */
  /*  CML_Dth is a threshlod for inclusion of dark pixels in finding a */
  /*  circular mask for the lower eyelid. */
  /*  */
  /*  CMU_Bth is a threshlod for inclusion of bright pixels in finding a */
  /*  circular mask for the upper eyelid. */
  /*  */
  /*  CMU_Dth is a threshlod for inclusion of dark pixels in finding a */
  /*  circular mask for the upper eyelid. */
  /*  */
  /*  cthU is a threshold for finding a circular mask for the upper eyelid by */
  /*  comparison to the sum of bright, dark, and high-gradient pixels. */
  /*  */
  /*  cthL is a threshold for finding a circular mask for the lower eyelid by */
  /*  comparison to the sum of bright, dark, and high-gradient pixels. */
  /*  */
  /*  ArcthU is a threshold for finding a high-gradient arc matching the edge */
  /*  of the upper eyelid. */
  /*  */
  /*  ArcthL is a threshold for finding a high-gradient arc matching the edge */
  /*  of the lower eyelid. */
  /*  */
  /*  Dth is a threshold for adding dark pixels of eyelashes to the final mask.
   */
  /*  */
  /*  Bth is a threshold for adding bright pixels of specular reflection to the
   * final mask. */
  /*  */
  /*  diag is switch to turn on diagnostic mode that displays intermediate */
  /*  images using %imtool (not available in C/C++ code versions). */
  /*  */
  /*  xi_disgard = single(xi_disgard); */
  memcpy(&SD->f3.I[0], &I[0], 30720U * sizeof(unsigned char));

  /*  */
  /*  Reduce resolution of flat iris image by 2x in both dimensions */
  /* floating point version */
  for (i = 1; i < 33; i++) {
    for (j = 1; j < 241; j++) {
      i1 = (int)rt_roundd_snf(
          (double)(((SD->f3.I[((i << 1) + (((j << 1) - 2) << 6)) - 2] +
                     SD->f3.I[((i << 1) + (((j << 1) - 1) << 6)) - 2]) +
                    SD->f3.I[((i << 1) + (((j << 1) - 2) << 6)) - 1]) +
                   SD->f3.I[((i << 1) + (((j << 1) - 1) << 6)) - 1]) /
          4.0);
      Count = (unsigned short)i1;
      I_smallgray[(i + ((j - 1) << 5)) - 1] = (unsigned char)Count;
    }
  }

  /* 8-bit integer version */
  /*  */
  /* Normalize intensity of reduced version of flat iris image */
  NormIntensity2(I_smallgray, xi_disgard, K);

  /* 8-bit integer version */
  /* floating point version */
  /*  */
  /* Compute mask for high gradient magnitude pixels */
  for (i1 = 0; i1 < 7680; i1++) {
    SD->f3.K[i1] = (float)K[i1] / 255.0F;
  }

  GradMag7(SD, SD->f3.K, gth, GM);

  /*  */
  /* Mask bright and dark pixels in lower eyelid half of flat iris image */
  /* Mask bright and dark pixels in upper eyelid half of flat iris image */
  /*  */
  /* Create combined binary (logical) mask of high-gradient, bright, and dark
   * pixels */
  for (i1 = 0; i1 < 120; i1++) {
    for (i2 = 0; i2 < 32; i2++) {
      uv0[i2 + (i1 << 5)] = K[i2 + (i1 << 5)];
    }
  }

  HiIntensity(uv0, CML_Bth);
  for (i1 = 0; i1 < 120; i1++) {
    for (i2 = 0; i2 < 32; i2++) {
      uv1[i2 + (i1 << 5)] = K[i2 + (i1 << 5)];
    }
  }

  LowIntensity(uv1, CML_Dth);
  for (i1 = 0; i1 < 120; i1++) {
    for (i2 = 0; i2 < 32; i2++) {
      uv2[i2 + (i1 << 5)] = K[i2 + ((120 + i1) << 5)];
    }
  }

  HiIntensity(uv2, CMU_Bth);
  for (i1 = 0; i1 < 120; i1++) {
    for (i2 = 0; i2 < 32; i2++) {
      uv3[i2 + (i1 << 5)] = K[i2 + ((120 + i1) << 5)];
    }
  }

  LowIntensity(uv3, CMU_Dth);
  for (i1 = 0; i1 < 120; i1++) {
    for (i2 = 0; i2 < 32; i2++) {
      i3 = (int)((unsigned int)uv0[i2 + (i1 << 5)] + uv1[i2 + (i1 << 5)]);
      if ((unsigned int)i3 > 255U) {
        i3 = 255;
      }

      b_CM[i2 + (i1 << 5)] = (i3 != 0);
      i3 = (int)((unsigned int)uv2[i2 + (i1 << 5)] + uv3[i2 + (i1 << 5)]);
      if ((unsigned int)i3 > 255U) {
        i3 = 255;
      }

      b_CM[i2 + ((i1 + 120) << 5)] = (i3 != 0);
    }
  }

  for (i1 = 0; i1 < 240; i1++) {
    for (i2 = 0; i2 < 32; i2++) {
      CM[i2 + (i1 << 5)] = (GM[i2 + (i1 << 5)] || b_CM[i2 + (i1 << 5)]);
    }
  }

  /*  */
  /* Find largest circle mask that matches above the threshold cthU with
   * combined mask in upper eyelid */
  /* region. */
  BestMask = 0;
  for (b_i = 1; b_i < 31; b_i++) {
    for (i1 = 0; i1 < 7680; i1++) {
      b_CM[i1] = (CM[i1] && CMb[0].f1[b_i - 1].f1[i1]);
    }

    c_sum(b_CM, dv0);
    c_sum(CMb[0].f1[b_i - 1].f1, dv1);
    if ((float)(b_sum(dv0) / b_sum(dv1)) >= cthU) {
      BestMask = b_i;
    }
  }

  if (BestMask > 0) {
    memcpy(&CMU[0], &CMs[0].f1[BestMask - 1].f1[0],
           7680U * sizeof(unsigned char));
  } else {
    memcpy(&CMU[0], &CMs[0].f1[1].f1[0], 7680U * sizeof(unsigned char));
    BestMask = 2;
  }

  /* Use arcs formed from circle masks to detect a high-gradient curve caused by
   * an */
  /* exposed upper eyelid boundary higher than the best circle mask */
  /* correlation. */
  BestArc = 0;
  for (b_i = (unsigned char)(BestMask + 2); b_i < 17; b_i++) {
    for (i1 = 0; i1 < 7680; i1++) {
      Arc = ((CMb[0].f1[b_i - 1].f1[i1] ^ CMb[0].f1[b_i - 2].f1[i1]) != 0);
      b_CM[i1] = (GM[i1] && Arc);
      b_Arc[i1] = Arc;
    }

    c_sum(b_CM, dv2);
    c_sum(b_Arc, dv3);
    if ((float)(b_sum(dv2) / b_sum(dv3)) >= ArcthU) {
      BestArc = b_i;
    }
  }

  if (BestArc > BestMask) {
    memcpy(&CMU[0], &CMs[0].f1[BestArc - 1].f1[0],
           7680U * sizeof(unsigned char));
  }

  /*  */
  /* Find largest circle mask that matches above the threshold cthL with
   * combined mask in lower eyelid */
  /* region. */
  BestMask = 0;
  for (b_i = 1; b_i < 31; b_i++) {
    for (i1 = 0; i1 < 7680; i1++) {
      b_CM[i1] = (CM[i1] && CMb[1].f1[b_i - 1].f1[i1]);
    }

    c_sum(b_CM, dv4);
    c_sum(CMb[1].f1[b_i - 1].f1, dv5);
    if ((float)(b_sum(dv4) / b_sum(dv5)) >= cthL) {
      BestMask = b_i;
    }
  }

  if (BestMask > 0) {
    memcpy(&CML[0], &CMs[1].f1[BestMask - 1].f1[0],
           7680U * sizeof(unsigned char));
  } else {
    memcpy(&CML[0], &CMs[1].f1[0].f1[0], 7680U * sizeof(unsigned char));
    BestMask = 1;
  }

  /* Use arcs formed from circle masks to detect a high-gradient curve caused by
   * an */
  /* exposed lower eyelid boundary higher than the best circle mask */
  /* correlation. */
  BestArc = 0;
  for (b_i = (unsigned char)(BestMask + 2); b_i < 17; b_i++) {
    /*     Arc = CML_big-CML_small; */
    for (i1 = 0; i1 < 7680; i1++) {
      Arc = ((CMb[1].f1[b_i - 1].f1[i1] ^ CMb[1].f1[b_i - 2].f1[i1]) != 0);
      b_CM[i1] = (GM[i1] && Arc);
      b_Arc[i1] = Arc;
    }

    c_sum(b_CM, dv6);
    c_sum(b_Arc, dv7);
    if ((float)(b_sum(dv6) / b_sum(dv7)) >= ArcthL) {
      BestArc = b_i;
    }
  }

  if (BestArc > BestMask) {
    memcpy(&CML[0], &CMs[1].f1[BestArc - 1].f1[0],
           7680U * sizeof(unsigned char));
  }

  /*  */
  /* Form total mask from upper eyelid circle mask, lower eyelid circle mask, */
  /* and dark pixels likely to be eyelashes. */
  b_LowIntensity(K, Dth);
  for (i1 = 0; i1 < 7680; i1++) {
    GM[i1] = ((CMU[i1] != 0) || (CML[i1] != 0) || (K[i1] != 0));
  }

  /*  */
  /* Use Connected Components Labeling to remove isolated dark regions not */
  /* connected to a circle mask from the total mask. */
  bwlabel(GM, SD->f3.dv8);
  for (i1 = 0; i1 < 7680; i1++) {
    d0 = rt_roundd_snf(SD->f3.dv8[i1]);
    if (d0 < 65536.0) {
      if (d0 >= 0.0) {
        Count = (unsigned short)d0;
      } else {
        Count = 0;
      }
    } else if (d0 >= 65536.0) {
      Count = MAX_uint16_T;
    } else {
      Count = 0;
    }

    L[i1] = Count;
  }

  for (Count = 1; Count < 33; Count++) {
    for (i = 1; i < 241; i++) {
      if ((L[(Count + ((i - 1) << 5)) - 1] != L[5759]) &&
          (L[(Count + ((i - 1) << 5)) - 1] != L[1919])) {
        GM[(Count + ((i - 1) << 5)) - 1] = false;
      }
    }
  }

  /*  */
  /*  Find bright pixels likely to be specular reflections. */
  b_HiIntensity(I_smallgray, Bth);

  /*  Count specular pixels and add to total mask, EESMb. */
  Count = 0;
  for (i = 1; i < 33; i++) {
    for (j = 1; j < 241; j++) {
      if ((I_smallgray[(i + ((j - 1) << 5)) - 1] == 255) &&
          (!GM[(i + ((j - 1) << 5)) - 1])) {
        u0 = Count + 1U;
        if (u0 > 65535U) {
          u0 = 65535U;
        }

        Count = (unsigned short)u0;
        GM[(i + ((j - 1) << 5)) - 1] = true;
      }
    }
  }

  /*  Compute proportion of I that is occluded by specularities. */
  *SpecCov = (float)Count / 32.0F / 240.0F;

  /*  */
  /* Enlarge total mask to the original resolution of flat iris image, I. */
  memset(&SD->f3.EESM[0], 0, 30720U * sizeof(boolean_T));
  for (i = 1; i < 33; i++) {
    for (j = 1; j < 241; j++) {
      SD->f3.EESM[((i << 1) + (((j << 1) - 2) << 6)) - 2] =
          GM[(i + ((j - 1) << 5)) - 1];
      SD->f3.EESM[((i << 1) + (((j << 1) - 1) << 6)) - 2] =
          GM[(i + ((j - 1) << 5)) - 1];
      SD->f3.EESM[((i << 1) + (((j << 1) - 2) << 6)) - 1] =
          GM[(i + ((j - 1) << 5)) - 1];
      SD->f3.EESM[((i << 1) + (((j << 1) - 1) << 6)) - 1] =
          GM[(i + ((j - 1) << 5)) - 1];
    }
  }

  /*  */
  /* Return 8-bit version of total mask. */
  for (i1 = 0; i1 < 30720; i1++) {
    EESM[i1] = (unsigned char)(SD->f3.EESM[i1] * 255U);
  }
}

/* End of code generation (EESMask6.cpp) */

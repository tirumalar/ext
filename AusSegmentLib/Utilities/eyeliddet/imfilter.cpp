/*
 * imfilter.cpp
 *
 * Code generation for function 'imfilter'
 *
 */

/* Include files */
#include "imfilter.h"
#include "EESM_Coverage.h"
#include "EESMask6.h"
#include "convn.h"
#include "rt_nonfinite.h"

/* Function Declarations */
static void padImage(const float a_tmp[7680], float a[8228]);

/* Function Definitions */
static void padImage(const float a_tmp[7680], float a[8228]) {
  int j;
  int i;
  static const unsigned char idxA[484] = {
      1U,   1U,   2U,   3U,   4U,   5U,   6U,   7U,   8U,   9U,   10U,  11U,
      12U,  13U,  14U,  15U,  16U,  17U,  18U,  19U,  20U,  21U,  22U,  23U,
      24U,  25U,  26U,  27U,  28U,  29U,  30U,  31U,  32U,  32U,  0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,   0U,
      0U,   0U,   1U,   1U,   2U,   3U,   4U,   5U,   6U,   7U,   8U,   9U,
      10U,  11U,  12U,  13U,  14U,  15U,  16U,  17U,  18U,  19U,  20U,  21U,
      22U,  23U,  24U,  25U,  26U,  27U,  28U,  29U,  30U,  31U,  32U,  33U,
      34U,  35U,  36U,  37U,  38U,  39U,  40U,  41U,  42U,  43U,  44U,  45U,
      46U,  47U,  48U,  49U,  50U,  51U,  52U,  53U,  54U,  55U,  56U,  57U,
      58U,  59U,  60U,  61U,  62U,  63U,  64U,  65U,  66U,  67U,  68U,  69U,
      70U,  71U,  72U,  73U,  74U,  75U,  76U,  77U,  78U,  79U,  80U,  81U,
      82U,  83U,  84U,  85U,  86U,  87U,  88U,  89U,  90U,  91U,  92U,  93U,
      94U,  95U,  96U,  97U,  98U,  99U,  100U, 101U, 102U, 103U, 104U, 105U,
      106U, 107U, 108U, 109U, 110U, 111U, 112U, 113U, 114U, 115U, 116U, 117U,
      118U, 119U, 120U, 121U, 122U, 123U, 124U, 125U, 126U, 127U, 128U, 129U,
      130U, 131U, 132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 140U, 141U,
      142U, 143U, 144U, 145U, 146U, 147U, 148U, 149U, 150U, 151U, 152U, 153U,
      154U, 155U, 156U, 157U, 158U, 159U, 160U, 161U, 162U, 163U, 164U, 165U,
      166U, 167U, 168U, 169U, 170U, 171U, 172U, 173U, 174U, 175U, 176U, 177U,
      178U, 179U, 180U, 181U, 182U, 183U, 184U, 185U, 186U, 187U, 188U, 189U,
      190U, 191U, 192U, 193U, 194U, 195U, 196U, 197U, 198U, 199U, 200U, 201U,
      202U, 203U, 204U, 205U, 206U, 207U, 208U, 209U, 210U, 211U, 212U, 213U,
      214U, 215U, 216U, 217U, 218U, 219U, 220U, 221U, 222U, 223U, 224U, 225U,
      226U, 227U, 228U, 229U, 230U, 231U, 232U, 233U, 234U, 235U, 236U, 237U,
      238U, 239U, 240U, 240U};

  for (j = 0; j < 242; j++) {
    for (i = 0; i < 34; i++) {
      a[i + 34 * j] = a_tmp[(idxA[i] + ((idxA[242 + j] - 1) << 5)) - 1];
    }
  }
}

void b_imfilter(EESMStackData *SD, const float varargin_1[7680],
                float b[7680]) {
  float a[8228];
  int i7;
  static const double dv11[9] = {-1.0, -2.0, -1.0, 0.0, 0.0,
                                 0.0,  1.0,  2.0,  1.0};

  int i8;
  padImage(varargin_1, a);
  for (i7 = 0; i7 < 8228; i7++) {
    SD->u1.f0.a[i7] = a[i7];
  }

  convn(SD->u1.f0.a, dv11, SD->u1.f0.dv12);
  for (i7 = 0; i7 < 240; i7++) {
    for (i8 = 0; i8 < 32; i8++) {
      b[i8 + (i7 << 5)] = (float)SD->u1.f0.dv12[(i8 + 34 * (1 + i7)) + 1];
    }
  }
}

void imfilter(EESMStackData *SD, const float varargin_1[7680], float b[7680]) {
  float a[8228];
  int i5;
  static const double dv9[9] = {-1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0};

  int i6;
  padImage(varargin_1, a);
  for (i5 = 0; i5 < 8228; i5++) {
    SD->u1.f1.a[i5] = a[i5];
  }

  convn(SD->u1.f1.a, dv9, SD->u1.f1.dv10);
  for (i5 = 0; i5 < 240; i5++) {
    for (i6 = 0; i6 < 32; i6++) {
      b[i6 + (i5 << 5)] = (float)SD->u1.f1.dv10[(i6 + 34 * (1 + i5)) + 1];
    }
  }
}

/* End of code generation (imfilter.cpp) */

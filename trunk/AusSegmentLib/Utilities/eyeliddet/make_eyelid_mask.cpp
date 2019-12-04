#include "make_eyelid_mask.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include "EESM_Coverage.h"
#include "EESM_initialize.h"
#include "EESM_terminate.h"
#include "EESMask6.h"

/**
 * @brief Create the circle masks used by eyelid detection
 *
 * @param CMs pixel mask images using 0 or 255 for pixel values
 * @param CMb boolean pixel mask using 0 or 1 for pixel values
 * @param cm_cnt number of images in CMs[].f1
 *
 * @file
 */
static void generate_circle_masks(cell_wrap_1 CMs[2], cell_wrap_3 CMb[2],
                                  unsigned cm_cnt) {
  for (unsigned u = 0; u < cm_cnt; u++) {
    unsigned vlen = 2;

    for (unsigned v = 0; v < vlen; v++) {
      const int height = 32, width = 240, pixel_cnt = height * width;
      const int c = v ? 60 : width - 60;

      const int R = 60, r = 90 - 2 * u;

      for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
          int rmi = r - (i + 1);
          int cmj = c - (j + 1);

          if ((rmi * rmi + cmj * cmj) <= R * R) {
            int off = j * height + i;
            assert(off < pixel_cnt);
            CMs[v].f1[u].f1[off] = 255;
            CMb[v].f1[u].f1[off] = 1;
          }
        }
      }
    }
  }
}

/**
 * @brief Transpose a 2D matrix
 *
 * @param T a 2D matrix pointer
 * @param rows number of rows
 * @param cols number of columns
 */
template <typename T>
static void transpose(T *a, int rows, int cols) {
  for (int i = 0; i < rows * cols;
       i++)  // https://www.careercup.com/question?id=12339553
  {
    int idx = i;
    do {  // calculate index in the original array
      idx = (idx % rows) * cols + (idx / rows);
    } while (idx < i);
    std::swap(a[i], a[idx]);
  }
}

/**
 * @brief Create an eyelid mask and coverage values
 *
 * @param flatiris flat iris input image
 * @param width input image width
 * @param height input image height
 * @param mask output mask
 * @param coverage three element double array of mask coverage proportions
 * @param speccov specularity coverage
 *
 * coverage[0] mask coverage proportion of total flat iris image.  coverage[1]
 * mask coverage proportion of right half (upper eyelid region) of flat iris
 * image. coverage[2] mask coverage proportion of left half (lower eyelid
 * region) of flat iris image.
 */
int make_eyelid_mask(const unsigned char *flatiris, unsigned width,
                     unsigned height, unsigned char *mask, double coverage[3],
                     double &speccov) {
  static bool once;
  static cell_wrap_1 CMs[2];  // cell_wrap_0 f1[30]; -> unsigned char f1[7680];
  static cell_wrap_3 CMb[2];  // cell_wrap_2 f1[30]; -> boolean_T f1[7680];

  float speccovf = 0;
  EESMStackData *sd = new EESMStackData;

  assert(sizeof CMs / sizeof CMs[0] == sizeof CMb / sizeof CMb[0]);
  assert(sizeof CMs[0].f1 / sizeof CMs[0].f1[0] ==
         sizeof CMb[0].f1 / sizeof CMb[0].f1[0]);

  if (!once) {
    once = true;
    unsigned cm_cnt = sizeof CMs[0].f1 / sizeof CMs[0].f1[0];
    generate_circle_masks(CMs, CMb, cm_cnt);
    EESM_initialize();
  }

  std::vector<unsigned char> iris(flatiris, flatiris + height * width);

  transpose(iris.data(), height, width);  // (row major order to column major)

  EESMask6(sd, iris.data(), CMs, CMb, 0.3, 0.85, 180, 90, 160, 105, 0.87, 0.87,
           0.63, 0.63, 90, 200, 0, &speccovf, mask);

  speccov = speccovf;

  float cov[3];
  EESM_Coverage(mask, cov);
  for (int i = 0; i < 3; i++) coverage[i] = cov[i];

  transpose(mask, width, height);  // (column major order to row major)

  delete sd;

  return 0;
}

/**
 * @brief Create an eyelid mask and coverage values
 *
 * @param flatiris flat iris input image
 * @param width input image width
 * @param height input image height
 * @param mask output mask
 * @param coverage three element double array of mask coverage proportions
 *
 * coverage[0] mask coverage proportion of total flat iris image.  coverage[1]
 * mask coverage proportion of right half (upper eyelid region) of flat iris
 * image. coverage[2] mask coverage proportion of left half (lower eyelid
 * region) of flat iris image.
 */
int make_eyelid_mask(const unsigned char *flatiris, unsigned width,
                     unsigned height, unsigned char *mask, double coverage[3]) {
  double speccov;
  return make_eyelid_mask(flatiris, width, height, mask, coverage, speccov);
}

/**
 * @brief Create an eyelid mask and coverage values
 *
 * @param flatiris flat iris input image
 * @param width input image width
 * @param height input image height
 * @param mask output mask
 */
int make_eyelid_mask(const unsigned char *flatiris, unsigned width,
                     unsigned height, unsigned char *mask) {
  double unused[3];
  return make_eyelid_mask(flatiris, width, height, mask, unused);
}

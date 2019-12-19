#ifndef eyelid_H
#define eyelid_H

#include "compiler.h"
#include "eyeliddet/make_eyelid_mask.h"
#include "helper.h"
#include "irisfind.h"
#include "stdio.h"
#include "TemplatePipelineError.h"

#include <math.h>    // for sqrtf(), sin(), cos(), abs()
#include <string.h>  // for memcpy()


#define EYELID_MASK_SPEC_THRESH 200
// glasses detect = percent of flat iris that is specularity
#define GLASSES_SPEC_THRESH 5.0f

#define EYELID_LOWER_EYELID_HEIGHT_DEFAULT 16
#define EYELID_UPPER_EYELID_HEIGHT_DEFAULT 40
#define EYELID_LOWER_EYELID_HEIGHT_MIN 10
#define EYELID_UPPER_EYELID_HEIGHT_MIN 22

#define FLAT_IMAGE_WIDTH 480
#define FLAT_IMAGE_HEIGHT 64
#define FLAT_IMAGE_SIZE (FLAT_IMAGE_WIDTH * FLAT_IMAGE_HEIGHT)

#define UNCOVERED 0

class Irisfind;
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// structs

struct Eyelid_struct {
  bool enable_eyelid_detect;

  uint8 bufMask[2][FLAT_IMAGE_SIZE];  // for the iris mask
  PLINE lineptrMask[2][FLAT_IMAGE_HEIGHT];
  uint16 width;
  uint16 height;

  // int8  bufGrad[FLAT_IMAGE_SIZE];
  // PLINE lineptrGrad[FLAT_IMAGE_HEIGHT];

  //
  // for test
  //
  uint8 test_useSpecMask;
  uint8 test_useEyelidMask;

  float coverage[2];
};

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

void ek_eyelid_session_init(Eyelid_struct &eyelid, bool enable_eyelid_detect);
void ek_eyelid_init(Eyelid_struct &eyelid);

TemplatePipelineError ek_eyelid_main(PLINE *lineptrCrop, size_t eyecrop_width,
                    size_t eyecrop_height, size_t flat_iris_width,
                    size_t flat_iris_height, Irisfind* irisfind,
                    Eyelid_struct &eyelid, PLINE *lineptrFlat,
                    PLINE *lineptrMask, float radiusSampling, float *cosTable,
                    float *sinTable);

float createEyelidMask(Eyelid_struct &eyelid, PLINE *lineptrIris, uint16 width,
                       uint16 height, PLINE *lineptrIrisMask);

// for test
void draw_two_eyelid_circles(PLINE *lineptr, int16 width, int16 height,
                             int16 heightLowerEyelid, int16 heightUpperEyelid);

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

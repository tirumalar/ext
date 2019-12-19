#pragma once
#include <math.h>    // for sqrtf(), atan2()
#include <stdlib.h>  // srand, rand
#include <iostream>
#include "compiler.h"  // compiler specific, data types, macros
#include "string.h"

enum class IRLEDSide { none, left, right, both };

inline IRLEDSide IRLEDSideFromU8(uint8_t val) noexcept {
  IRLEDSide side = IRLEDSide::none;
  switch (val) {
    case 1:
      side = IRLEDSide::left;
      break;
    case 2:
      side = IRLEDSide::right;
      break;
    case 3:
      side = IRLEDSide::both;
      break;
    default:
      break;
  }
  return side;
}

inline std::ostream& operator<<(std::ostream& os, const IRLEDSide& side) {
  switch (side) {
    case IRLEDSide::none:
      os << "none";
      break;
    case IRLEDSide::left:
      os << "left";
      break;
    case IRLEDSide::right:
      os << "right";
      break;
    case IRLEDSide::both:
      os << "both";
      break;
  }
  return os;
}

//
// IRLED orientation
//
enum class IRLEDOrientation { horizontal, vertical };

inline std::ostream& operator<<(std::ostream& os,
                                const IRLEDOrientation& orientation) {
  switch (orientation) {
    case IRLEDOrientation::horizontal:
      os << "horizontal";
      break;
    case IRLEDOrientation::vertical:
      os << "vertical";
      break;
  }
  return os;
}

struct PointXY {
  uint16 x;
  uint16 y;
};

struct Messages {
  uint32 code;
  float32 value;
  uint8* string[64];
};

//
// for the sort algorithm
//

#ifndef ELEM_SWAP
#define ELEM_SWAP(a, b)         \
  {                             \
    register elem_type t = (a); \
    (a) = (b);                  \
    (b) = t;                    \
  }
#endif

typedef uint16 elem_type;

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

// sort algorithm
elem_type median_value(elem_type dataArr[], uint16 n);

void ek_version(void);
void ek_error(uint32 code);

float32 faceAngle_degrees(float32 x1, float32 y1, float32 x2, float32 y2);

uint32 mark_low_values_on_image_dense(PLINE* lineptr, int16 imagewidth,
                                      int16 imageheight, int16 xc, int16 yc,
                                      int16 roi, float32 darkRatio,
                                      PLINE* lineptrDark);

uint32 mark_low_values_on_image(PLINE* lineptr, int16 imagewidth,
                                int16 imageheight, int16 xc, int16 yc,
                                int16 roi, float32 darkRatio,
                                PLINE* lineptrDark, uint8* avgDarkValue);

float32 pvar(PLINE* hptr8, int16 imagewidth, int16 imageheight, int16 xc,
             int16 yc, int16 searchBox);

void hor_gradient_inverted(PLINE* lineptr, uint16 width, uint16 height,
                           PLINE* lineptrGrad);
void vert_gradient_inverted(PLINE* lineptr, uint16 width, uint16 height,
                            PLINE* lineptrGrad);
void hor_gradient(PLINE* lineptr, uint16 width, uint16 height,
                  PLINE* lineptrGrad);
void vert_gradient(PLINE* lineptr, uint16 width, uint16 height,
                   PLINE* lineptrGrad);
void hor_gradient_clipped(PLINE* lineptr, uint16 width, uint16 height,
                          int32 maxgrad, int32 mingrad, PLINE* lineptrGrad);
void vert_gradient_clipped(PLINE* lineptr, uint16 width, uint16 height,
                           int32 maxgrad, int32 mingrad, PLINE* lineptrGrad);

void copy_buf(PLINE* lineptrin, uint16 width, uint16 height, PLINE* lineptrout);

void test_buf_add_noise(uint8* buf, uint32 size, uint32 rmax, uint32 seed);
void test_buf_add_noise_buf(uint8* buf1, uint8* bufnoise, uint32 size,
                            uint8* bufout);
void test_buf_subtract(uint8* buf1, uint8* buf2, uint32 size, uint8* bufout);

void test_buf_val(PLINE* hptr, uint16 width, uint16 height, uint8 val);
void test_buf_16_val(PLINE* hptr, uint16 width, uint16 height, uint16 val);
void test_buf_random(PLINE* hptr, uint16 width, uint16 height, uint32 seed);
void test_buf_add_vert_stripe(PLINE* hptr, uint16 width, uint16 height);

void integralImage(PLINE* hptr, uint16 width, uint16 height, PLINE* ihptr);

void get_lineptrs_8(uint8* buf, uint16 width, uint16 height, PLINE* lineptr);
void get_lineptrs_16(uint16* buf, uint16 width, uint16 height, PLINE* lineptr);
void get_lineptrs_32(uint32* buf, uint16 width, uint16 height, PLINE* lineptr);
void get_lineptrs_crop(uint8* buf, uint16 bufwidth, uint16 x0, uint16 y0,
                       uint16 cropheight, PLINE* lineptr);

void pyro4(PLINE* hptr, uint16 width, uint16 height, PLINE* phptr);

void crop_in_place(uint8* pSrcImg, uint16 width, uint16 height, uint16 xc,
                   uint16 yc, uint16 cropWidth, uint16 cropHeight,
                   PLINE* lineptrCrop);
void crop_out(uint8* pSrcImg, uint16 width, uint16 height, uint16 xc, uint16 yc,
              uint16 cropWidth, uint16 cropHeight, uint8* pptrCrop, int Rotate);

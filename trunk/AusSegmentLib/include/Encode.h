#pragma once
#include "helper.h"
#include "string.h"
#include "TemplatePipelineError.h"

#define TEMPLATE_MIN_VALID_BITS 1500
#define ENCODE_CORING_SCALE_FACTOR 1.0

class Encode {
 private:
  int _base_scale;
  size_t _flat_iris_width;
  size_t _flat_iris_height;
  size_t _template_width;
  size_t _template_height;

  size_t _feature_filter_length;
  size_t _wrap_image_width;
  size_t _wrap_image_height;
  size_t _wrap_integral_width;
  size_t _wrap_integral_height;

  float* _frequency_response_threshold;

  size_t _vertical_template_sample_size;
  size_t _horizontal_template_sample_size;

  PLINE* _line_ptr_flat;
  PLINE* _line_ptr_mask;
  PLINE* _line_ptr_template_encode;
  PLINE* _line_ptr_template_mask;

  uint8_t* _buf_wrap;
  uint8_t* _buf_mask_wrap;
  PLINE* _line_ptr_buf_wrap;
  PLINE* _line_ptr_buf_mask_wrap;

  uint32_t* _buf_integral_wrap;
  uint32_t* _buf_mask_integral_wrap;
  PLINE* _line_ptr_buf_integral_wrap;
  PLINE* _line_ptr_buf_mask_integral_wrap;

  void HorizontalBorderWrap(PLINE* linePtrIn, uint16 width, uint16 height,
                            uint8 filterLength, PLINE* linePtrOut);
  void ExtractFeatures(PLINE* lineptrWrapInt, PLINE* lineptrMaskWrapInt,
                       size_t wrapWidth, size_t filterLength,
                       PLINE* lineptrEncoded, PLINE* lineptrTag,
                       size_t encodeWidth, size_t encodeHeight,
                       uint32& validBitsInTemplate,
                       uint32& unmaskedBitsInTemplate,
                       float& validBitRatioInTemplate);

 public:
  Encode(int base_scale, size_t flat_iris_width, size_t flat_iris_height,
         size_t template_width, size_t template_height);
  ~Encode();
  int EncodeFlatIris(uint8_t* flat_iris, uint8_t* partial_mask,
                     uint8_t* template_encode, uint8_t* template_mask);
};

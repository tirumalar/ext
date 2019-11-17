#pragma once
#include <stdint.h>
#include "eyelid.h"
#include "eyelock-core.h"
#include "frame.h"
#include "irisfind.h"
#include "math.h"

class AusIris {
 public:
	AusIris(size_t eyecrop_width, size_t eyecrop_height, size_t flat_iris_width,
       size_t flat_iris_height);

  ~AusIris();

  void AusIris_init();

  float IrisFocus(uint8_t Iris[FLAT_IMAGE_SIZE], uint8_t Mask[FLAT_IMAGE_SIZE]);
  int GenerateFlatIris(uint8_t* eyecrop, uint8_t* flat_iris,
                       uint8_t* partial_mask);

 private:
  size_t _eyecrop_width;
  size_t _eyecrop_height;

  size_t _flat_iris_width;
  size_t _flat_iris_height;

  PLINE* _line_ptr_crop;
  PLINE* _line_ptr_flat;
  PLINE* _line_ptr_mask;

  Irisfind_struct _iris_find;
  Eyelid_struct _eyelid;
  LiveDetection_struct _live_detection;

  float _radius_sampling;
  float* _cos_table;
  float* _sin_table;
};

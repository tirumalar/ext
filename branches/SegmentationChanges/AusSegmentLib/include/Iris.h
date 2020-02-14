#pragma once
#include <stdint.h>
#include "eyelid.h"
#include "eyelock-core.h"
#include "frame.h"
#include "irisfind.h"
#include "math.h"

class IrisSegmentation {
 public:
	IrisSegmentation(size_t eyecrop_width, size_t eyecrop_height, size_t flat_iris_width,
       size_t flat_iris_height, float gaze_radius_thresh, float PorportionOfIrisVisibleThreshold);

  ~IrisSegmentation();

  void IrisSegmentation_init(size_t eyecrop_width, size_t eyecrop_height,
		  unsigned short int MinIrisDiameter, unsigned short int MaxIrisDiameter,
		  unsigned short int MinPupilDiameter, unsigned short int MaxPupilDiameter,
		  unsigned short int MinSpecDiameter, unsigned short int MaxSpecDiameter);

  float IrisFocus(uint8_t Iris[FLAT_IMAGE_SIZE], uint8_t Mask[FLAT_IMAGE_SIZE]);
  int GenerateFlatIris(uint8_t* eyecrop, uint8_t* flat_iris, uint8_t* partial_mask, size_t eyecrop_width, size_t eyecrop_height, IrisFindParameters& IrisPupilParams);

  int m_LogSegInfo;

  int LoadEyelockConfigINIFile();
 private:
  size_t _eyecrop_width;
  size_t _eyecrop_height;

  size_t _flat_iris_width;
  size_t _flat_iris_height;

  PLINE* _line_ptr_crop;
  PLINE* _line_ptr_flat;
  PLINE* _line_ptr_mask;

  Irisfind *m_iris_find; // Class to initialize the memory for variables which are 320x240 and 640x480 eyecrops

  Eyelid_struct _eyelid;
  LiveDetection_struct _live_detection;

  float _radius_sampling;
  float* _cos_table;
  float* _sin_table;

  float _PIVThreshold; // PorportionOfIrisVisibleThreshold
};

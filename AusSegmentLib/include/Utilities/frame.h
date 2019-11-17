#pragma once
#include <stdint.h>

#include "helper.h"

#ifdef ELENG
const auto MaxEyes = 5;  // FIXME DetOutMaxEyeCropCount()
#else
const auto MaxEyes = 2;
#endif
#ifndef SIZE_D
const auto SIZE_D = 1280;
#endif

typedef struct image_frame {
  uint32_t FrameNumber;
  uint32_t Timestamp;
  uint8_t Distance;
  uint8_t Illuminator;
  uint8_t *Data;
  uint8_t *PreviewData;
  int Brightness16;
  bool IsUseableImageBrightness;
  IRLEDSide IrledSide;
} IMAGE_FRAME;

typedef struct eyecrop_frame {
  eyecrop_frame()
      : LeftEyeCrop(nullptr),
        RightEyeCrop(nullptr),
        LeftEyeDetected(0),
        RightEyeDetected(0) {}

  void Clear() {
    if (LeftEyeDetected) {
      delete[] LeftEyeCrop;
      LeftEyeCrop = nullptr;
    }
    if (RightEyeDetected) {
      delete[] RightEyeCrop;
      RightEyeCrop = nullptr;
    }
  }
  uint32_t FrameNumber;
  uint32_t Timestamp;
  uint8_t UserTooLeft;
  uint8_t UserTooRight;
  uint8_t UserTooHigh;
  uint8_t UserTooLow;
  uint8_t UserTooClose;
  uint8_t UserTooFar;
  uint8_t LeftEyeDetected;
  uint8_t RightEyeDetected;
  unsigned EyeCropSize;
  uint8_t *LeftEyeCrop;
  uint8_t *RightEyeCrop;
  bool BadEyeCropFocus[MaxEyes];
  bool brightnessValid;
  bool useDefaultValues;
  uint8_t brightness;
  IRLEDSide IrledSide;
} EYECROP_FRAME;

typedef uint8_t TEMPLATE[SIZE_D];

typedef struct template_frame {
  uint32_t FrameNumber;
  uint32_t Timestamp;
  uint8_t GazeOff;
  uint8_t Dilated;
  uint8_t QualityScore;
  uint8_t LeftEyeDetected;
  uint8_t RightEyeDetected;
  uint8_t *LeftEyeCrop;
  uint8_t *RightEyeCrop;
  TEMPLATE *TemplateEncode;
  TEMPLATE *TemplateTag;
  float LeftIrisCoverage;
  float RightIrisCoverage;
  float LeftIrisFocus;
  float RightIrisFocus;
  bool brightnessValid;
  uint8_t brightness;
  bool LiveDet_FlatPaper;
} TEMPLATE_FRAME;

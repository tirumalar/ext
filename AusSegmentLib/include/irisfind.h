#pragma once

#include <mutex>
#include "compiler.h"  // compiler specific, data types, macros

using namespace std;

#define IRISFIND_DARK_SEARCHBOX 50
#define IRISFIND_SPEC_SEARCHBOX 10
#define IRISFIND_PUPIL_SEARCHBOX 50
#define IRISFIND_IRIS_SEARCHBOX 50

#define IRISFIND_DARK_ROI \
  80  // 100, was 80 (faster), but missed eyes too close to camera

#define IRISFIND_DARK_RATIO 0.125

// size is number of pixels in area
#define IRISFIND_DARK_PUPIL_MIN_SIZE 100

// multiply dark score just to make it in range of pupil and iris scores
// for nicer histograms
#define DARK_SCORE_ARBITRARY_MULTIPLIER 8
#define IRISFIND_SPECMASK_MULTIPLIER 1.0

#define MIN_SCORE_VARIANCE 1000  // 1000

#define IRISFIND_MIN_IRIS_GRADIENT 2
#define IRISFIND_MAX_IRIS_GRADIENT 16

#define IRISFIND_MIN_PUPIL_GRADIENT 8
#define IRISFIND_MAX_PUPIL_GRADIENT 16

#define IRISFIND_MIN_REDPUPIL_GRADIENT 4
#define IRISFIND_MAX_REDPUPIL_GRADIENT 6

#define IRISFIND_MIN_SPEC_GRADIENT 4
#define IRISFIND_MAX_SPEC_GRADIENT 8

#define IRISFIND_IMAGE_WIDTH 320
#define IRISFIND_IMAGE_HEIGHT 240
#define IRISFIND_IMAGE_SIZE (IRISFIND_IMAGE_WIDTH * IRISFIND_IMAGE_HEIGHT)
#define IRISFIND_MAX_LUT_CIRCLE_DIAMETER 200

#define IRISFIND__PUPIL_70_PERCENT_OF_IRIS__MIN_MULTIPLIER 1.5
#define IRISFIND__PUPIL_14_PERCENT_OF_IRIS__MAX_MULTIPLIER 7.0
#define IRISFIND__PUPIL_TO_IRIS__MIN_MULTIPLIER \
  IRISFIND__PUPIL_70_PERCENT_OF_IRIS__MIN_MULTIPLIER
#define IRISFIND__PUPIL_TO_IRIS__MAX_MULTIPLIER \
  IRISFIND__PUPIL_14_PERCENT_OF_IRIS__MAX_MULTIPLIER

#define PUPIL_TO_IRIS_RATIO_MIN 0.15
#define PUPIL_TO_IRIS_RATIO_MAX 0.6

#define IRISFIND_MAX_IRIS_DIAMETER 180
#define IRISFIND_MIN_IRIS_DIAMETER 60
#define IRISFIND_MAX_PUPIL_DIAMETER \
  60  // 70, was 60 (faster), but missed eyes too close to camera
#define IRISFIND_MIN_PUPIL_DIAMETER 16

#define IRISFIND_MAX_SPEC_DIAMETER 20
#define IRISFIND_MIN_SPEC_DIAMETER 8

// radius is used in the algorithm, not diameter
#define IRISFIND_MAX_LUT_CIRCLE_RADIUS (IRISFIND_MAX_LUT_CIRCLE_DIAMETER >> 1)
#define IRISFIND_MAX_IRIS_RADIUS (IRISFIND_MAX_IRIS_DIAMETER >> 1)
#define IRISFIND_MIN_IRIS_RADIUS (IRISFIND_MIN_IRIS_DIAMETER >> 1)
#define IRISFIND_MAX_PUPIL_RADIUS (IRISFIND_MAX_PUPIL_DIAMETER >> 1)
#define IRISFIND_MIN_PUPIL_RADIUS (IRISFIND_MIN_PUPIL_DIAMETER >> 1)
#define IRISFIND_MAX_SPEC_RADIUS (IRISFIND_MAX_SPEC_DIAMETER >> 1)
#define IRISFIND_MIN_SPEC_RADIUS (IRISFIND_MIN_SPEC_DIAMETER >> 1)

#define IRISFIND_MAX_RADIUS_CNT \
  (IRISFIND_MAX_LUT_CIRCLE_RADIUS + 1)  // +1 because index = radius
#define IRISFIND_CIRCLE_MAX_RADIUS IRISFIND_MAX_RADIUS_CNT

#define IRISFIND_QUARTERCIRCLE_MAX_LUT_SIZE \
  (2 * IRISFIND_CIRCLE_MAX_RADIUS)  // >= (r * pi/2)
#define IRISFIND_FULLCIRCLE_MAX_LUT_SIZE \
  (6 * IRISFIND_CIRCLE_MAX_RADIUS)  // c = 2*pi*r

#define IRISFIND_SEARCH_BOX_MAX_SIZE 200
#define LIVEDET_MAX_LIVE_EYE_COUNT 2

#ifdef __cplusplus
extern "C" {
#endif

// signed so it can be hold a flag = -1
struct PointXY_signed {
  int16 x;
  int16 y;
};

struct PointXYZ_float {
  float32 x;
  float32 y;
  float32 z;  // z = radius
  uint32 s;   // score
};

// For sub-pixel score cube.
// Also used for recording a list of results.
struct ScoreXYZS {
  uint32 x;
  uint32 y;
  uint32 z;
  uint32 s;
};

// One Circle_struct per radius.
// Circle_struct contains the LUT for one radius and the
// circle match winner info for that radius.
// Winner info is reused for spec, pupil, iris.
struct Circle_struct {
  struct PointXY_signed lutPos[IRISFIND_FULLCIRCLE_MAX_LUT_SIZE];
  uint16 lutSize;
  uint16 radius;
  uint16
      edgeCnt;  // for debug // number of samples in circle edge used in Hough
  struct PointXY_signed winnerPos;
  uint32 winnerScore;
  uint32 winnerNoise;
  bool error;

  struct PointXY_signed vTopRight[IRISFIND_FULLCIRCLE_MAX_LUT_SIZE >> 3];
  uint16 vTopRightSize;
  struct PointXY_signed hRight[IRISFIND_FULLCIRCLE_MAX_LUT_SIZE >> 2];
  uint16 hRightSize;
  struct PointXY_signed vBottomRight[IRISFIND_FULLCIRCLE_MAX_LUT_SIZE >> 3];
  uint16 vBottomRightSize;
};

struct Irisfind_struct {
  int8 maxSpecGrad;
  int8 minSpecGrad;
  int8 maxPupilGrad;
  int8 minPupilGrad;
  int8 maxRedPupilGrad;
  int8 minRedPupilGrad;
  int8 maxIrisGrad;
  int8 minIrisGrad;

  uint32 minScoreVariance;

  // adjust the size of the mask so that it is larger than the actual spec
  float32 specMaskMultiplier;  // IRISFIND_SPECMASK_MULTIPLIER

  int8 hgradSpec[IRISFIND_IMAGE_SIZE];
  PLINE lineptrHgradSpec[IRISFIND_IMAGE_HEIGHT];
  int8 vgradSpec[IRISFIND_IMAGE_SIZE];
  PLINE lineptrVgradSpec[IRISFIND_IMAGE_HEIGHT];

  int8 hgradPupil[IRISFIND_IMAGE_SIZE];
  PLINE lineptrHgradPupil[IRISFIND_IMAGE_HEIGHT];
  int8 vgradPupil[IRISFIND_IMAGE_SIZE];
  PLINE lineptrVgradPupil[IRISFIND_IMAGE_HEIGHT];

  int8 hgradIris[IRISFIND_IMAGE_SIZE];
  PLINE lineptrHgradIris[IRISFIND_IMAGE_HEIGHT];
  int8 vgradIris[IRISFIND_IMAGE_SIZE];
  PLINE lineptrVgradIris[IRISFIND_IMAGE_HEIGHT];

  int8 hgrad[IRISFIND_IMAGE_SIZE];
  PLINE lineptrHgrad[IRISFIND_IMAGE_HEIGHT];
  int8 vgrad[IRISFIND_IMAGE_SIZE];
  PLINE lineptrVgrad[IRISFIND_IMAGE_HEIGHT];

  uint8 bufDark[IRISFIND_IMAGE_SIZE];
  PLINE lineptrDark[IRISFIND_IMAGE_HEIGHT];

  uint8 bufMask[2][IRISFIND_IMAGE_SIZE];
  PLINE lineptrMask[2][IRISFIND_IMAGE_HEIGHT];

  uint16 width;
  uint16 height;

  uint16 maxIrisRadius;
  uint16 minIrisRadius;
  uint16 maxPupilRadius;
  uint16 minPupilRadius;
  uint16 maxSpecRadius;
  uint16 minSpecRadius;
  uint16 lutCnt;
  uint16 maxLutCnt;
  uint16 maxLutSize;

  struct Circle_struct circle[IRISFIND_MAX_RADIUS_CNT];

  struct PointXYZ_float darkPos[2];
  uint32 darkScore[2];
  bool darkFound[2];
  struct PointXYZ_float specPos[2];
  uint32 specScore[2];
  bool specFound[2];
  struct PointXYZ_float pupilPos[2];
  uint32 pupilScore[2];
  bool pupilFound[2];
  struct PointXYZ_float irisPos[2];
  uint32 irisScore[2];
  bool irisFound[2];

  struct PointXYZ_float prev_specPos[2];

  bool findRedEye;
  bool autoRedEye;

  uint16 errorCnt;
  uint16 tempLUTbuf16[IRISFIND_FULLCIRCLE_MAX_LUT_SIZE];
  struct ScoreXYZS scoreCube[3][3][3];
  float32 faceDistance[2];
  float32 pupilToIrisRatio[2];

  bool glasses_lowEyelid_detected[2];

  float32 gaze_radius_thresh;

  struct PointXYZ_float gaze[2];

  bool good_gaze[2];

  bool headRotationError;
  bool bystanderError;

  uint8 irisBrightness;
  uint8 pupilBrightness;
  uint8 pupilBrightnessLR[2];
  uint32 pupilEdge;
  bool brightnessValid;
  float32 focus_unfiltered;
  struct PointXYZ_float eyePosMM[2];
  struct PointXYZ_float eyePos[2];
};

enum class IRLEDSequence {
  left_right,
  left_right_both,
};

struct SpoofDet_struct {
  // save the first result for quick authentication
  // requires 3 images
  bool first_test_ready{false};
  bool first_flat_paper{false};

  // update spoof flag for every sample
  bool continuous_test_ready{false};
  bool continuous_flat_paper{false};

  // avg n spoof results
  bool avg_test_ready{false};
  bool avg_flat_paper{false};

  // create single signal to flag good enroll/auth
  // update with most recent avg spoof result
  // no need to qualify with "test ready" flag
  bool last_human{false};

  // number of images attempted to test
  uint32 image_cnt{0};

  // number of samples considered in spoof tests
  uint32 sample_cnt{0};
};

#define SPOOF_RESULTS_MAX 8
#define EYE_XY_DELTA_MAX 20.0
#define EYE_Z_DELTAMM_MAX 30.0
#define GAZE_XY_DELTA_MAX 2.0
#define HUMAN_SPEC_XY_DELTA_MIN 5.0

struct LiveDetection_struct {
  // set to false to skip liveness detection, i.e. flag all templates as live
  bool enabled{true};

  // IRLED_struct irled{ };
  SpoofDet_struct spoofdet{};

  // image cnt
  int image_num[2]{0};
  int prev1_image_num[2]{0};
  int prev2_image_num[2]{0};

  // IRLED side
  // IRLEDSide side[2]{ IRLEDSide::none };
  // IRLEDSide prev_side[2]{ IRLEDSide::none };

  // eye locations
  float ex[2]{0.0};
  float ey[2]{0.0};
  float ez[2]{0.0};
  float prev_ex[2]{0.0};
  float prev_ey[2]{0.0};
  float prev_ez[2]{0.0};

  // spec locations (use gaze values)
  float sx[2]{0.0};
  float sy[2]{0.0};
  float prev1_sx[2]{0.0};
  float prev1_sy[2]{0.0};
  float prev2_sx[2]{0.0};
  float prev2_sy[2]{0.0};

  // a few results to average
  int spoof_results[SPOOF_RESULTS_MAX]{0};

  // used with first test result (fast)
  bool firstResult{false};

  // used with avg test results
  int avgTestCnt{0};

  bool continuousAuthentication{false};

  LiveDetection_struct() {}
  // LiveDetection_struct(IRLEDOrientation orientation) : irled(orientation) { }
};

uint8 subpixel_from_scoreCube(struct ScoreXYZS (*sCube)[3][3][3], uint16 xc,
                              uint16 yc, uint16 zc, uint32 minScoreVariance,
                              float32* psx, float32* psy, float32* psz);

void init_circles(Irisfind_struct& irisfind, uint16 lutCnt);

void search_for_circles(PLINE* hGradLptr, PLINE* vGradLptr, PLINE* mLptr,
                        PLINE* dLptr, int8 minGrad, int8 maxGrad,
                        struct Circle_struct* circleR, uint16 minRadius,
                        uint16 maxRadius, int16 width, int16 height, int16 xc,
                        int16 yc, int16 searchBox, struct PointXYZ_float* wpos,
                        bool dark, bool ccl_dark, bool spec, bool pupil,
                        bool iris);

void search_for_circles_subpixel(PLINE* hGradLptr, PLINE* vGradLptr,
                                 PLINE* mLptr, PLINE* dLptr, int8 minGrad,
                                 int8 maxGrad, struct Circle_struct* circleR,
                                 uint16 centerRadius, int16 xc, int16 yc,
                                 bool dark, bool spec, bool pupil, bool iris,
                                 struct ScoreXYZS (*sCube)[3][3][3]);

void make_spec_mask(PLINE* lineptr, uint16 width, uint16 height,
                    struct PointXYZ_float* prevpos, struct PointXYZ_float* pos,
                    float32 specBoxMultiplier, bool newSpec);

void ek_irisfind_session_init(Irisfind_struct& irisfind);
void ek_irisfind_init(Irisfind_struct& irisfind);

void ek_irisfind_main(PLINE* line_ptr_eyecrop, PLINE* line_ptr_flat_iris,
                      Irisfind_struct& irisfind, size_t eyecrop_width,
                      size_t eyecrop_height, size_t flat_iris_width,
                      size_t flat_iris_height);

uint8 make_xy_search_area(int16 width, int16 height, int16 xc, int16 yc,
                          int16 radius, int16 searchBox, int16* xmin,
                          int16* xmax, int16* ymin, int16* ymax);

float32 irisfind_distanceToFace_cm(float32 iris_radius_pixels);

#ifdef __cplusplus
}
#endif

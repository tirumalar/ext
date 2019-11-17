
#include "irisfind.h"
#include <cstddef>
#include "compiler.h"
#include "eyelid.h"
#include "helper.h"

#include <math.h>  // for sqrtf()
#ifdef API_STAT
#include "statistic.h"
#endif

#include <iostream>

void ek_irisfind_session_init(Irisfind_struct& irisfind) {
  // init

  irisfind.findRedEye = false;
  irisfind.autoRedEye = false;

  // set up buffers

  irisfind.width = IRISFIND_IMAGE_WIDTH;
  irisfind.height = IRISFIND_IMAGE_HEIGHT;

  get_lineptrs_8((uint8*)irisfind.hgradSpec, irisfind.width, irisfind.height,
                 irisfind.lineptrHgradSpec);
  get_lineptrs_8((uint8*)irisfind.vgradSpec, irisfind.width, irisfind.height,
                 irisfind.lineptrVgradSpec);
  get_lineptrs_8((uint8*)irisfind.hgradPupil, irisfind.width, irisfind.height,
                 irisfind.lineptrHgradPupil);
  get_lineptrs_8((uint8*)irisfind.vgradPupil, irisfind.width, irisfind.height,
                 irisfind.lineptrVgradPupil);
  get_lineptrs_8((uint8*)irisfind.hgradIris, irisfind.width, irisfind.height,
                 irisfind.lineptrHgradIris);
  get_lineptrs_8((uint8*)irisfind.vgradIris, irisfind.width, irisfind.height,
                 irisfind.lineptrVgradIris);

  get_lineptrs_8((uint8*)irisfind.hgrad, irisfind.width, irisfind.height,
                 irisfind.lineptrHgrad);
  get_lineptrs_8((uint8*)irisfind.vgrad, irisfind.width, irisfind.height,
                 irisfind.lineptrVgrad);

  get_lineptrs_8((uint8*)irisfind.bufDark, irisfind.width, irisfind.height,
                 irisfind.lineptrDark);

  //--------------------------------------------------------------
  //
  // init mask
  //

  for (uint8 eye = 0; eye < 2; eye++) {
    get_lineptrs_8(irisfind.bufMask[eye], irisfind.width, irisfind.height,
                   irisfind.lineptrMask[eye]);
    // clear mask buffer
    for (uint32 i = 0; i < IRISFIND_IMAGE_SIZE; i++)
      irisfind.bufMask[eye][i] = 0;
    // for erasing previous mask instead of clearing whole buffer
    // init a previous dummy spec
    irisfind.prev_specPos[eye].x =
        (float32)(irisfind.width / 2);  // arbitrary coordinate
    irisfind.prev_specPos[eye].y =
        (float32)(irisfind.height / 2);  // arbitrary coordinate
    irisfind.prev_specPos[eye].z = 2;    // arbitrary small number
  }

  //--------------------------------------------------------------
  //
  // init circle templates
  //

  irisfind.maxLutCnt = IRISFIND_MAX_RADIUS_CNT;
  irisfind.maxLutSize = IRISFIND_FULLCIRCLE_MAX_LUT_SIZE;

  // init once per session, not init per scale change
  // number of circle templates
  // use index i (LUT[i]) to be the same as radius = i
  // so that circle index = radius
  irisfind.lutCnt = irisfind.maxLutCnt;
  if (irisfind.lutCnt > irisfind.maxLutCnt)
    irisfind.lutCnt = irisfind.maxLutCnt;
  init_circles(irisfind, irisfind.lutCnt);
  //
  //--------------------------------------------------------------
  //
  // init history lists
  //

  // irisfind.pupilHistory.goodHistory = false;
}
//
void ek_irisfind_init(Irisfind_struct& irisfind) {
  uint32 i;

  // init errors

  irisfind.errorCnt = 0;

  // init variables

  // init irisfind results

  irisfind.irisBrightness = 0;
  irisfind.pupilBrightness = 0;
  irisfind.pupilBrightnessLR[0] = 0;
  irisfind.pupilBrightnessLR[1] = 0;
  irisfind.pupilEdge = 0;
  irisfind.brightnessValid = false;
  irisfind.focus_unfiltered = 0.0f;
  irisfind.gaze_radius_thresh = GAZE_RADIUS_THRESH;
  irisfind.headRotationError = false;
  irisfind.bystanderError = false;

  for (i = 0; i <= 1; i++) {
    irisfind.specFound[i] = false;
    irisfind.darkFound[i] = false;
    irisfind.pupilFound[i] = false;
    irisfind.irisFound[i] = false;

    irisfind.darkScore[i] = 0;
    irisfind.specScore[i] = 0;
    irisfind.pupilScore[i] = 0;
    irisfind.irisScore[i] = 0;

    irisfind.faceDistance[i] = 0;
    irisfind.pupilToIrisRatio[i] = 0;

    irisfind.glasses_lowEyelid_detected[i] = 0;

    irisfind.gaze[i].x = 0.0;
    irisfind.gaze[i].y = 0.0;
    irisfind.gaze[i].z = 0.0;
    irisfind.good_gaze[i] = true;
  }

  // clear mask buffer
  // for(uint32 i = 0;i < IRISFIND_IMAGE_SIZE;i++) irisfind.mask[eye][i] =  0;

  // settings

  // irisfind.maskThresh = IRISFIND_MASK_THRESH;

  // TBD: adjust with contrast and lighting

  irisfind.maxSpecGrad = IRISFIND_MAX_SPEC_GRADIENT;
  irisfind.minSpecGrad = IRISFIND_MIN_SPEC_GRADIENT;
  irisfind.maxPupilGrad = IRISFIND_MAX_PUPIL_GRADIENT;
  irisfind.minPupilGrad = IRISFIND_MIN_PUPIL_GRADIENT;
  irisfind.maxRedPupilGrad = IRISFIND_MAX_REDPUPIL_GRADIENT;
  irisfind.minRedPupilGrad = IRISFIND_MIN_REDPUPIL_GRADIENT;
  irisfind.maxIrisGrad = IRISFIND_MAX_IRIS_GRADIENT;
  irisfind.minIrisGrad = IRISFIND_MIN_IRIS_GRADIENT;

  irisfind.minScoreVariance = MIN_SCORE_VARIANCE;

  // TBD: adjust with zoom

  // irisfind.searchBox = 40; // must be less than IRISFIND_SEARCH_BOX_MAX_SIZE
  // irisfind.maxRadius = IRISFIND_MAX_RADIUS_CNT; // (maxRadius - minRadius)
  // must be less than IRISFIND_MAX_RADIUS_CNT irisfind.minRadius = 5; //
  // maxRadius LUT size must be less than IRISFIND_QUARTERCIRCLE_MAX_LUT_SIZE

  //
  // radius = circle LUT index
  // max radius = IRISFIND_MAX_RADIUS_CNT
  //

  // irisfind.searchBoxIris = irisfind.searchBox;
  irisfind.maxIrisRadius = IRISFIND_MAX_IRIS_RADIUS;
  irisfind.minIrisRadius = IRISFIND_MIN_IRIS_RADIUS;

  // irisfind.searchBoxPupil = 10;
  irisfind.maxPupilRadius = IRISFIND_MAX_PUPIL_RADIUS;
  irisfind.minPupilRadius = IRISFIND_MIN_PUPIL_RADIUS;

  irisfind.maxSpecRadius = IRISFIND_MAX_SPEC_RADIUS;
  irisfind.minSpecRadius = IRISFIND_MIN_SPEC_RADIUS;

  // TBD
  // irisfind.searchBoxIris = irisfind.maxIrisRadius;
  // irisfind.searchBoxPupil = irisfind.maxPupilRadius;

  // TBD: adjust with zoom and noise

  // irisfind.maxCircleNoise_pupil =  IRISFIND_MAX_CIRCLE_NOISE_PUPIL;
  // irisfind.maxCircleNoise_iris =  IRISFIND_MAX_CIRCLE_NOISE_IRIS;

  // TBD: adjust with zoom or size of spec

  // adjust the size of the mask so that it is larger than the actual spec
  irisfind.specMaskMultiplier = IRISFIND_SPECMASK_MULTIPLIER;
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// spec mask
//
//
//
void make_spec_mask(PLINE* lineptr, uint16 /*width*/, uint16 /*height*/,
                    struct PointXYZ_float* prevpos, struct PointXYZ_float* pos,
                    float32 specBoxMultiplier, bool newSpec) {
  uint8* pptr;
  uint16 x, y, xmax, xmin, ymax, ymin;
  // prepare to erase previous spec mask
  float32 specBoxRadius = specBoxMultiplier * prevpos->z;
  xmin = (uint16)((prevpos->x - specBoxRadius) - 0.5);
  xmax = (uint16)((prevpos->x + specBoxRadius) + 0.5);
  ymin = (uint16)((prevpos->y - specBoxRadius) - 0.5);
  ymax = (uint16)((prevpos->y + specBoxRadius) + 0.5);
  // draw mask area
  for (y = ymin; y < ymax; y++) {
    pptr = (uint8*)lineptr[y];
    for (x = xmin; x < xmax; x++) {
      pptr[x] = 0;
    }
  }
  //
  // set new spec mask area
  if (newSpec) {
    specBoxRadius = specBoxMultiplier * pos->z;
    xmin = (uint16)((pos->x - specBoxRadius) - 0.5);
    xmax = (uint16)((pos->x + specBoxRadius) + 0.5);
    ymin = (uint16)((pos->y - specBoxRadius) - 0.5);
    ymax = (uint16)((pos->y + specBoxRadius) + 0.5);
    // draw mask area
    for (y = ymin; y < ymax; y++) {
      pptr = (uint8*)lineptr[y];
      for (x = xmin; x < xmax; x++) {
        pptr[x] = 0xff;
      }
    }
    // save new spec pos
    prevpos->x = pos->x;
    prevpos->y = pos->y;
    prevpos->z = pos->z;
  }
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// spec search
//
//
void spec_search(Irisfind_struct& irisfind, uint8 eyeCropIndex) {
  // search type
  bool dark = false;
  bool ccl_dark = false;
  bool spec = true;
  bool pupil = false;
  bool iris = false;
  // return search winner circle (x, y, z)
  struct PointXYZ_float* wpos = &irisfind.specPos[eyeCropIndex];
  //---------------------------------------------------
  // search area
  // TBD: the following parameters could change with distance
  // set min and max radius
  uint16 minRadius = irisfind.minSpecRadius;
  uint16 maxRadius = irisfind.maxSpecRadius;
  // set gradient min and max
  int8 minGrad = irisfind.minSpecGrad;
  int8 maxGrad = irisfind.maxSpecGrad;
  // set center of search area (xc, yc)
  int16 xc = irisfind.width >> 1;
  int16 yc = irisfind.height >> 1;
  // set search box size
  int16 searchBox = IRISFIND_SPEC_SEARCHBOX;
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // eyecrop width and height
  int16 width = irisfind.width;
  int16 height = irisfind.height;
  // set gradient of eye crop
  PLINE* hGradLptr = irisfind.lineptrHgrad;
  PLINE* vGradLptr = irisfind.lineptrVgrad;
  PLINE* mLptr = NULL;
  PLINE* dLptr = irisfind.lineptrDark;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = irisfind.circle;
  // search the gradient for circles
  search_for_circles(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                     circleR, minRadius, maxRadius, width, height, xc, yc,
                     searchBox, wpos, dark, ccl_dark, spec, pupil, iris);
  //
  // output results to use in pupil search
  // return search winner circle (x, y, z)
  uint16 winnerRadius = (uint16)wpos->z;  // search winner (x, y, z)
  irisfind.specScore[eyeCropIndex] = wpos->s;
  if (winnerRadius > 0)
    irisfind.specFound[eyeCropIndex] = true;  // TBD: how to test for good spec?
  if (irisfind.specScore[eyeCropIndex] < IRISFIND_SPEC_MIN_SCORE)
    irisfind.specFound[eyeCropIndex] = false;
}
//
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// dark search for rough pupil area (dark pixel values) detection
//
//
void dark_search(Irisfind_struct& irisfind, uint8 eyeCropIndex) {
  // search type
  bool dark = true;
  bool ccl_dark = false;
  bool spec = false;
  bool pupil = false;
  bool iris = false;
  // return search winner circle (x, y, z)
  struct PointXYZ_float* wpos = &irisfind.darkPos[eyeCropIndex];
  //---------------------------------------------------
  // search area
  // TBD: the following parameters could change with distance
  // set min and max radius
  uint16 minRadius = irisfind.minPupilRadius;
  uint16 maxRadius = irisfind.maxPupilRadius;
  // set gradient min and max
  int8 minGrad = 0;
  int8 maxGrad = 0;

  // set center of search area (xc, yc)
  int16 xc = (int16)irisfind.specPos[eyeCropIndex].x;
  int16 yc = (int16)irisfind.specPos[eyeCropIndex].y;
  // set search box size
  // Dark searchBox will be calculated in search_for_circles(); in
  // ek_irisfind_circles.cpp
  int16 searchBox = (int16)irisfind.specPos[eyeCropIndex]
                        .z;  // spec radius used later to make dark searchbox

  // set center of search area (xc, yc)
  // int16 xc = irisfind.width >> 1;
  // int16 yc = irisfind.height >> 1;
  // set search box size
  // int16 searchBox = IRISFIND_DARK_SEARCHBOX;
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // eyecrop width and height
  int16 width = irisfind.width;
  int16 height = irisfind.height;
  // set gradient of eye crop
  PLINE* hGradLptr = NULL;
  PLINE* vGradLptr = NULL;
  PLINE* mLptr = NULL;                  // spec mask ptr
  PLINE* dLptr = irisfind.lineptrDark;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = irisfind.circle;
  // search the gradient for circles
  search_for_circles(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                     circleR, minRadius, maxRadius, width, height, xc, yc,
                     searchBox, wpos, dark, ccl_dark, spec, pupil, iris);
  //
  // output results to use in pupil search
  // return search winner circle (x, y, z)
  uint16 winnerRadius = (uint16)wpos->z;  // search winner (x, y, z)
  // arbitrary multipler make dark score range similar to pupil and iris range
  irisfind.darkScore[eyeCropIndex] = wpos->s * DARK_SCORE_ARBITRARY_MULTIPLIER;
  if (winnerRadius > 0)
    irisfind.darkFound[eyeCropIndex] = true;  // TBD: how to test for good spec?
  if (irisfind.darkScore[eyeCropIndex] < IRISFIND_DARK_MIN_SCORE)
    irisfind.darkFound[eyeCropIndex] = false;
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// pupil search
//
//
void pupil_search(Irisfind_struct& irisfind, uint8 eyeCropIndex) {
  // search type
  bool dark = false;
  bool ccl_dark = false;
  bool spec = false;
  bool pupil = true;
  bool iris = false;
  // return search winner circle (x, y, z)
  struct PointXYZ_float* wpos = &irisfind.pupilPos[eyeCropIndex];
  //---------------------------------------------------
  // search area
  // TBD: the following parameters could change with distance
  // set min and max radius
  // uint16 minRadius = irisfind.minPupilRadius;
  // uint16 maxRadius = irisfind.maxPupilRadius;
  //
  // calculate search area from dark location
  //
  uint16 darkRadius = (uint16)irisfind.darkPos[eyeCropIndex].z;
  uint16 minRadius = (uint16)((float32)darkRadius * 0.8);
  uint16 maxRadius = (uint16)((float32)darkRadius * 1.25);

  // set center of search area (xc, yc)
  int16 xc = (int16)irisfind.darkPos[eyeCropIndex].x;
  int16 yc = (int16)irisfind.darkPos[eyeCropIndex].y;
  // set search box size
  int16 searchBox = 2;  //(float32)darkRadius * 0.5;

  //
  // set gradient min and max
  int8 minGrad = irisfind.minPupilGrad;
  int8 maxGrad = irisfind.maxPupilGrad;
  //

  /*
  //
  // calculate search area from spec location
  //
  // set center of search area (xc, yc)
  int16 xc = irisfind.specPos[eyeCropIndex].x;
  int16 yc = irisfind.specPos[eyeCropIndex].y;
  // set search box size
  // Pupil searchBox will be calculated in search_for_circles(); in
  ek_irisfind_circles.cpp
  // Setting pupil searchbox to (2 * (pupil radius + spec radius)).
  int16 searchBox = irisfind.specPos[eyeCropIndex].z; // spec radius used later
  to make pupil searchbox
  */
  //
  // calculate search area from default settings
  //
  // set center of search area (xc, yc)
  // int16 xc = irisfind.width >> 1;
  // int16 yc = irisfind.height >> 1;
  // set search box size
  // int16 searchBox = IRISFIND_PUPIL_SEARCHBOX;
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // eyecrop width and height
  int16 width = irisfind.width;
  int16 height = irisfind.height;
  // set gradient of eye crop
  PLINE* hGradLptr = irisfind.lineptrHgrad;
  PLINE* vGradLptr = irisfind.lineptrVgrad;
  PLINE* mLptr = irisfind.lineptrMask[eyeCropIndex];
  PLINE* dLptr = NULL;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = irisfind.circle;
  // search the gradient for circles
  search_for_circles(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                     circleR, minRadius, maxRadius, width, height, xc, yc,
                     searchBox, wpos, dark, ccl_dark, spec, pupil, iris);
  //
  // output results to use in iris search
  // return search winner circle (x, y, z)
  uint16 winnerRadius = (uint16)wpos->z;  // search winner (x, y, z)
  irisfind.pupilScore[eyeCropIndex] = wpos->s;
  if (winnerRadius > 0)
    irisfind.pupilFound[eyeCropIndex] =
        true;  // TBD: how to test for good pupil?
  if (irisfind.pupilScore[eyeCropIndex] < IRISFIND_PUPIL_MIN_SCORE)
    irisfind.pupilFound[eyeCropIndex] = false;
}
//
//-------------------
//
uint8 pupil_search_subpixel(Irisfind_struct& irisfind, uint8 eyeCropIndex) {
  //---------------------------------------------------
  //
  // only these parameters change between pupil and iris subpixel
  //
  // search type
  bool dark = false;
  bool spec = false;
  bool pupil = true;
  bool iris = false;
  // set xyz input from search winner
  struct PointXYZ_float* wpos = &irisfind.pupilPos[eyeCropIndex];
  // set gradient min and max
  int8 minGrad = irisfind.minPupilGrad;
  int8 maxGrad = irisfind.maxPupilGrad;
  //---------------------------------------------------
  //
  // calculate search area from search winner
  //
  // check circle radius against xy boundary
  // detect parameters that would cause errors
  // do not process if already at radius limit
  // small radius
  if (wpos->z <= 1) return 0;  // out of bounds
  // large radius
  if (wpos->z >= IRISFIND_CIRCLE_MAX_RADIUS) return 0;  // out of bounds
  // radius ok
  uint16 centerRadius = (uint16)wpos->z;
  uint16 maxRadius = (uint16)wpos->z + 1;
  // set center of search area (xc, yc)
  int16 xc = (int16)wpos->x;
  int16 yc = (int16)wpos->y;
  // search inputs
  // the following are the same for spec, pupil, iris
  // eyecrop width and height
  int16 width = irisfind.width;
  int16 height = irisfind.height;
  // set search box size
  int16 searchBox = 3;
  // test for out-of-bounds
  int16 xmin, xmax, ymin, ymax;  // not used
  uint8 inbounds = make_xy_search_area(width, height, xc, yc, maxRadius,
                                       searchBox, &xmin, &xmax, &ymin, &ymax);
  if (inbounds == 0) return 0;  // out of bounds
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // set gradient of eye crop
  PLINE* hGradLptr = irisfind.lineptrHgrad;
  PLINE* vGradLptr = irisfind.lineptrVgrad;
  PLINE* mLptr = irisfind.lineptrMask[eyeCropIndex];
  PLINE* dLptr = NULL;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = irisfind.circle;
  // set results return struct
  struct ScoreXYZS(*sCube)[3][3][3] = &irisfind.scoreCube;
  // search the gradient for circles
  search_for_circles_subpixel(hGradLptr, vGradLptr, mLptr, dLptr, minGrad,
                              maxGrad, circleR, centerRadius, xc, yc, dark,
                              spec, pupil, iris, sCube);

  //
  // output results are in scoreCube
  //
  // convert scoreCube to the weighted center (x, y, z) value, float32.
  //
  // TBD: return error code for subpixel_from_scoreCube()? score variance too
  // low? (irisfind.minScoreVariance = MIN_SCORE_VARIANCE)
  // subpixel_from_scoreCube(sCube, xc, yc, centerRadius,
  // irisfind.minScoreVariance, &psx, &psy, &psz)
  //
  // TBD: make correct score variance test for iris/pupil or discard variance
  // test
  //
  float32 psx, psy, psz;
  if (subpixel_from_scoreCube(sCube, xc, yc, centerRadius, 100, &psx, &psy,
                              &psz)) {
    // overwrite integer results in search winner
    wpos->x = psx;
    wpos->y = psy;
    wpos->z = psz;
  }
  //
  return 1;  // good subpixel
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// iris search
//
//
void iris_search(Irisfind_struct& irisfind, uint8 eyeCropIndex) {
  // search type
  bool dark = false;
  bool ccl_dark = false;
  bool spec = false;
  bool pupil = false;
  bool iris = true;
  // return search winner circle (x, y, z)
  struct PointXYZ_float* wpos = &irisfind.irisPos[eyeCropIndex];
  //---------------------------------------------------
  // search area
  // TBD: the following parameters could change with distance
  // set min and max radius
  // uint16 minRadius = irisfind.minIrisRadius;
  // uint16 maxRadius = irisfind.maxIrisRadius;
  uint16 minRadius = (uint16)((float32)irisfind.pupilPos[eyeCropIndex].z *
                              IRISFIND__PUPIL_TO_IRIS__MIN_MULTIPLIER);
  uint16 maxRadius = (uint16)((float32)irisfind.pupilPos[eyeCropIndex].z *
                              IRISFIND__PUPIL_TO_IRIS__MAX_MULTIPLIER);
  if ((minRadius > irisfind.maxIrisRadius) ||
      (minRadius < irisfind.minIrisRadius))
    minRadius = irisfind.minIrisRadius;
  if ((maxRadius > irisfind.maxIrisRadius) ||
      (maxRadius < irisfind.minIrisRadius))
    maxRadius = irisfind.maxIrisRadius;
  // set gradient min and max
  int8 minGrad = irisfind.minIrisGrad;
  int8 maxGrad = irisfind.maxIrisGrad;
  // set center of search area (xc, yc)
  // int16 xc = irisfind.width >> 1;
  // int16 yc = irisfind.height >> 1;
  int16 xc = (int16)irisfind.pupilPos[eyeCropIndex].x;
  int16 yc = (int16)irisfind.pupilPos[eyeCropIndex].y;
  // set search box size
  // int16 searchBox = IRISFIND_IRIS_SEARCHBOX;
  // For iris, set (max searchBox) here.
  // Iris searchBox will be calculated in search_for_circles(); in
  // ek_irisfind_circles.cpp TBD: relate iris offset to (distance to face) and
  // pupil/iris size ratio
  int16 searchBox =
      (int16)irisfind.pupilPos[eyeCropIndex]
          .z;  //* 0.5;// *
               //IRISFIND__PUPIL_RADIUS_MULTIPLIER_TO_MAKE_IRIS_MAX_SEARCHBOX;
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // eyecrop width and height
  int16 width = irisfind.width;
  int16 height = irisfind.height;
  // set gradient of eye crop
  PLINE* hGradLptr = irisfind.lineptrHgrad;
  PLINE* vGradLptr = irisfind.lineptrVgrad;
  PLINE* mLptr = irisfind.lineptrMask[eyeCropIndex];
  PLINE* dLptr = NULL;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = irisfind.circle;
  // search the gradient for circles
  search_for_circles(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                     circleR, minRadius, maxRadius, width, height, xc, yc,
                     searchBox, wpos, dark, ccl_dark, spec, pupil, iris);
  //
  // return search winner circle (x, y, z)
  uint16 winnerRadius = (uint16)wpos->z;  // search winner (x, y, z)
  irisfind.irisScore[eyeCropIndex] = wpos->s;
  if (winnerRadius > 0)
    irisfind.irisFound[eyeCropIndex] = true;  // TBD: how to test for good iris?
  if (irisfind.irisScore[eyeCropIndex] < IRISFIND_IRIS_MIN_SCORE)
    irisfind.irisFound[eyeCropIndex] = false;
}
//
//-------------------
//
uint8 iris_search_subpixel(Irisfind_struct& irisfind, uint8 eyeCropIndex) {
  //---------------------------------------------------
  //
  // only these parameters change between pupil and iris subpixel
  //
  // search type
  bool dark = false;
  bool spec = false;
  bool pupil = false;
  bool iris = true;
  // set xyz input from search winner
  struct PointXYZ_float* wpos = &irisfind.irisPos[eyeCropIndex];
  // set gradient min and max
  int8 minGrad = irisfind.minIrisGrad;
  int8 maxGrad = irisfind.maxIrisGrad;
  //---------------------------------------------------
  //
  // calculate search area from search winner
  //
  // check circle radius against xy boundary
  // detect parameters that would cause errors
  // do not process if already at radius limit
  // small radius
  if (wpos->z <= 1) return 0;  // out of bounds
  // large radius
  if (wpos->z >= IRISFIND_CIRCLE_MAX_RADIUS) return 0;  // out of bounds
  // radius ok
  uint16 centerRadius = (uint16)wpos->z;
  uint16 maxRadius = (uint16)wpos->z + 1;
  // set center of search area (xc, yc)
  int16 xc = (int16)wpos->x;
  int16 yc = (int16)wpos->y;
  // search inputs
  // the following are the same for spec, pupil, iris
  // eyecrop width and height
  int16 width = irisfind.width;
  int16 height = irisfind.height;
  // set search box size
  int16 searchBox = 3;
  // test for out-of-bounds
  int16 xmin, xmax, ymin, ymax;  // not used
  uint8 inbounds = make_xy_search_area(width, height, xc, yc, maxRadius,
                                       searchBox, &xmin, &xmax, &ymin, &ymax);
  if (inbounds == 0) return 0;  // out of bounds
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // set gradient of eye crop
  PLINE* hGradLptr = irisfind.lineptrHgrad;
  PLINE* vGradLptr = irisfind.lineptrVgrad;
  PLINE* mLptr = irisfind.lineptrMask[eyeCropIndex];
  PLINE* dLptr = NULL;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = irisfind.circle;
  // set results return struct
  struct ScoreXYZS(*sCube)[3][3][3] = &irisfind.scoreCube;
  // search the gradient for circles
  search_for_circles_subpixel(hGradLptr, vGradLptr, mLptr, dLptr, minGrad,
                              maxGrad, circleR, centerRadius, xc, yc, dark,
                              spec, pupil, iris, sCube);

  //
  // output results are in scoreCube
  //
  // convert scoreCube to the weighted center (x, y, z) value, float32.
  //
  // TBD: return error code for subpixel_from_scoreCube()? score variance too
  // low? (irisfind.minScoreVariance = MIN_SCORE_VARIANCE)
  // subpixel_from_scoreCube(sCube, xc, yc, centerRadius,
  // irisfind.minScoreVariance, &psx, &psy, &psz)
  //
  // TBD: make correct score variance test for iris/pupil or discard variance
  // test
  //
  float32 psx, psy, psz;
  if (subpixel_from_scoreCube(sCube, xc, yc, centerRadius, 100, &psx, &psy,
                              &psz)) {
    // overwrite integer results in search winner
    wpos->x = psx;
    wpos->y = psy;
    wpos->z = psz;
  }
  //
  return 1;  // good subpixel
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// analytics
//
//
//
// distance to face in cm, using iris diameter = 11.8 mm
//
float32 irisfind_distanceToFace_cm(float32 iris_radius_pixels) {
  float32 distance = -1.0f;
  float32 iris_diameter_pixels = 2.0f * iris_radius_pixels;
  float32 userIris_mm =
      IRIS_TYPICAL_DIAMETER_IN_MM;  // use actual measured user iris diameter in
                                    // mm, or derive from TOF distance
  float32 userIris_cm = userIris_mm / 10.0f;
  float32 userIris_pixels_at_50cm = userIris_cm * PIXELS_PER_CM_AT_50CM;
  if (iris_diameter_pixels > 0.0f) {
    float32 ratio = userIris_pixels_at_50cm / iris_diameter_pixels;
    distance = ratio * 50.0f;  // using pixels_per_cm_at_50cm
  }
  return distance;
}
//
// pupil radius to iris radius, (pupil/iris) ratio for candidate eyes
//
float32 pupilToIrisRatio(float32 pupil_rad, float32 iris_rad) {
  float32 ratio = -1.0f;
  if (iris_rad > 0.0f) ratio = pupil_rad / iris_rad;
  return ratio;
}

void ek_irisfind_main(PLINE* line_ptr_eyecrop, PLINE* line_ptr_flat_iris,
                      Irisfind_struct& irisfind, size_t eyecrop_width,
                      size_t eyecrop_height, size_t flat_iris_width,
                      size_t flat_iris_height)

{
  //---------------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------------
  //
  // pupil and iris circle search
  //
  //

  // *** TBD: set dark searchbox by distance to face
  // *** TBD: set pupil searchbox by distance to face and illumination (pupil
  // size)
  // *** TBD: set iris searchbox by distance to face and pupil size

  /*for (uint8 eyeCropIndex = 0; eyeCropIndex <= 1; eyeCropIndex++)
  {
          // search for spec, pupil, iris
          if (eyefind.eyeFound[eyeCropIndex])
          {*/

  // blur() for test purposes
  // blur(eyefind, eyefind.lineptrCrop[eyeCropIndex], eyefind.cropWidth,
  // eyefind.cropHeight, 2);

  //
  // search for dark area that might be pupil
  //

  int16 xc = eyecrop_width >> 1;
  int16 yc = eyecrop_height >> 1;
  int16 roi = IRISFIND_DARK_ROI;

  mark_low_values_on_image(line_ptr_eyecrop, eyecrop_width, eyecrop_height, xc,
                           yc, roi, (float32)IRISFIND_DARK_RATIO,
                           irisfind.lineptrDark,
                           &irisfind.pupilBrightnessLR[0]);

  // test using spec to guide dark circle search
  irisfind.specPos[0].x = (float)(xc);
  irisfind.specPos[0].y = (float)(yc);
  irisfind.specPos[0].z = 8;

  dark_search(irisfind, 0);

  if (irisfind.darkFound[0]) {
    // make gradients of eye crop
    hor_gradient(line_ptr_eyecrop, irisfind.width, irisfind.height,
                 irisfind.lineptrHgrad);
    vert_gradient(line_ptr_eyecrop, irisfind.width, irisfind.height,
                  irisfind.lineptrVgrad);

    //
    // search for specularity
    //

    spec_search(irisfind,
                0);  // TBD: bad method if spec is not round or single.

    // if(!irisfind.specFound[eyeCropIndex]) // TBD: what to do here?
    // clear old spec mask, make new spec mask
    // TBD: fix the order of parameters in call
    make_spec_mask(irisfind.lineptrMask[0], irisfind.width, irisfind.height,
                   &irisfind.prev_specPos[0], &irisfind.specPos[0],
                   irisfind.specMaskMultiplier, irisfind.specFound[0]);
    //
    // search for pupil and iris
    //
    //
    // TBD: note that segmentation is allowed to continue if spec search fails
    //
    if (irisfind.specFound[0]) {
      pupil_search(irisfind, 0);
      if (irisfind.pupilFound[0]) {
        iris_search(irisfind, 0);
      }
    }

    //------------------------------------------------------------------------------------
    //
    // refine segmentation
    //
    //
    // Previous integer segmentation search winner (x, y, r) results
    // are weighted by surrounding 3x3x3 array values and overwritten with
    // float32.
    //
    if (irisfind.irisFound[0]) {
      pupil_search_subpixel(irisfind, 0);

      iris_search_subpixel(irisfind, 0);
    }

    //------------------------------------------------------------------------------------
    //
    // face distance: from iris diameter
    // pupil dilation: ratio of pupil radius to iris radius
    // gaze: compare pupil location to spec location
    //
    if (irisfind.irisFound[0]) {
      // possible face distance in cm from camera for candidate eyes
      irisfind.faceDistance[0] =
          irisfind_distanceToFace_cm(irisfind.irisPos[0].z);
      // TODO: REPLACE WITH DISTANCE CLASS
      /*if (irisfind.faceDistance[0] > eyefind.maxFaceDistance) {
              eyefind.tooFar = true;
      }
      else if (irisfind.faceDistance[0] > 0.0 &&
              irisfind.faceDistance[0] < eyefind.minFaceDistance) {
              eyefind.tooClose = true;
}*/

      // pupil radius to iris radius, (pupil/iris) ratio for candidate eyes
      irisfind.pupilToIrisRatio[0] =
          pupilToIrisRatio(irisfind.pupilPos[0].z, irisfind.irisPos[0].z);

      // gaze: compare pupil location to spec location
      irisfind.gaze[0].x = irisfind.pupilPos[0].x - irisfind.specPos[0].x;
      irisfind.gaze[0].y = irisfind.pupilPos[0].y - irisfind.specPos[0].y;
      irisfind.gaze[0].z =
          sqrtf(pow(irisfind.gaze[0].x, 2) + pow(irisfind.gaze[0].y, 2));

      irisfind.good_gaze[0] =
          (irisfind.gaze[0].z < irisfind.gaze_radius_thresh);
    }
    //-------------------------------------------------------------------------------
  }

  //}

  //}
  //------------------------------------------------------------------------------------
  //
  //
  // prevent bystander from authenticating
  //
  // test if iris are at different distances from camera
  // If one iris is further from camera, invalidate it.
  //
  if (irisfind.irisFound[0]) {
    /*float32 d0 = irisfind.faceDistance[0];
    float32 d1 = irisfind.faceDistance[1];
    // eye 0 is closest to camera
    float32 diff = d1 - d0;
    uint8 nearEye = 0;
    uint8 farEye = 1;
    // eye 1 is closest to camera
    if (d0 > d1) {
            diff = d0 - d1;
            nearEye = 1;
            farEye = 0;
    }
    if (diff > IRISFIND_MAX_IRIS_DISTANCE_DIFF_CM) {

            // invalidate far eye
            irisfind.headRotationError = true;
            irisfind.bystanderError = true;
}*/

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // flat paper spoof detect with image labeling
    //
    // from Craig Word doc dump
    // added 20180814 MJR
    //

    /*auto& spoofdet = liveDetection.spoofdet;
    auto& irled = liveDetection.irled;

    if (liveDetection.enabled)
    {
            // image center plus crop center distance
            float32 image_x0c = (float32)((cam.width / 2) + (eyefind.cropWidth /
2)); float32 image_y0c = (float32)((cam.height / 2) + (eyefind.cropHeight / 2));

            for (int i = 0; i < LIVEDET_MAX_LIVE_EYE_COUNT; i++) {
                    if (irisfind.irisFound[i]) {

                            // find eye (x, y) pixels offset from image center
                            irisfind.eyePos[i].x = irisfind.pupilPos[i].x +
(float32)eyefind.pos[i].x - image_x0c; irisfind.eyePos[i].y =
irisfind.pupilPos[i].y + (float32)eyefind.pos[i].y - image_y0c;

                            // converted face distance cm to mm
                            irisfind.eyePosMM[i].z = irisfind.faceDistance[i]
* 10.0f;
                    }
            }

            // init for each enroll/auth
            if (spoofdet.image_cnt == 0) {
                    for (int ei = 0; ei < LIVEDET_MAX_LIVE_EYE_COUNT; ei++) {
                            liveDetection.image_num[ei] = -100;
                            liveDetection.prev1_image_num[ei] = -1000;
                            liveDetection.prev2_image_num[ei] = -10000;
                    }
            }
            spoofdet.image_cnt++;

            // detect user movement
            bool goodEye[LIVEDET_MAX_LIVE_EYE_COUNT] = { false, false };
            bool goodGaze[LIVEDET_MAX_LIVE_EYE_COUNT] = { false, false };

            bool spoof_flag = false;
            bool testComplete = false;

            for (int ei = 0; ei < LIVEDET_MAX_LIVE_EYE_COUNT; ei++) {
                    // Make sure the gaze and eye location values are stable.
                    // First test irisfind.irisFound[] for a good iris capture.
                    if (irisfind.irisFound[ei]) {
                            //Log("---new eye");
                            liveDetection.image_num[ei] = spoofdet.image_cnt;
                            liveDetection.side[ei] = irled.firingSide;
                            liveDetection.ex[ei] = irisfind.eyePos[ei].x;
                            liveDetection.ey[ei] = irisfind.eyePos[ei].y;
                            liveDetection.ez[ei] = irisfind.eyePosMM[ei].z;
                            liveDetection.sx[ei] = irisfind.gaze[ei].x;
                            liveDetection.sy[ei] = irisfind.gaze[ei].y;
                            //
                            // optional
                            // test for a continuous sequence of 3 images
                            int image_num_delta = liveDetection.image_num[ei] -
liveDetection.prev2_image_num[ei]; if (image_num_delta ==
LIVEDET_MAX_LIVE_EYE_COUNT) {
                                    // check change in locations of eye and spec
                                    float edx = liveDetection.ex[ei] -
liveDetection.prev_ex[ei]; float edy = liveDetection.ey[ei] -
liveDetection.prev_ey[ei]; float edz = liveDetection.ez[ei] -
liveDetection.prev_ez[ei]; float sdx = liveDetection.sx[ei] -
liveDetection.prev2_sx[ei]; float sdy = liveDetection.sy[ei] -
liveDetection.prev2_sy[ei];
                                    //
                                    bool goodTest = true;
                                    //
                                    // tests to make sure spec states are
correct
                                    //
                                    // make sure IRLED has changed state
                                    if (liveDetection.side[ei] ==
liveDetection.prev_side[ei]) goodTest = false;
                                    //
                                    // eye stability test
                                    // check for user face/eye movement
                                    //
                                    // test for eye motion
                                    if (fabsf(edx) > EYE_XY_DELTA_MAX) goodTest
= false; else if (fabsf(edy) > EYE_XY_DELTA_MAX) goodTest = false; else if
(fabsf(edz) > EYE_Z_DELTAMM_MAX) goodTest = false; else goodEye[ei] = true;
                                    // test for gaze motion
                                    if (fabsf(sdx) > GAZE_XY_DELTA_MAX) goodTest
= false; else if (fabsf(sdy) > GAZE_XY_DELTA_MAX) goodTest = false; else
goodGaze[ei] = true;
                                    //
                                    //
                                    // compare change in spec location to a
threshold if (goodTest) {
                                            //
                                            testComplete = true;
                                            //
                                            // find distance between specs
                                            //
                                            float ssd;
                                            float ssdx =
fabsf(liveDetection.sx[ei] - liveDetection.prev1_sx[ei]); float ssdy =
fabsf(liveDetection.sy[ei] - liveDetection.prev1_sy[ei]);
                                            //
                                            //Log("ssdxy: " + FloatToStr(ssdx,
2) + ", " + FloatToStr(ssdy, 2));
                                            //
                                            // use x or y direction depending on
IRLED orientation
                                            //
                                            if (irled.orientation ==
IRLEDOrientation::horizontal) { ssd = ssdx;
                                            }
                                            else { // irled.orientation ==
IRLEDOrientation::vertical ssd = ssdy;
                                            }
                                            //
                                            //Log("ssd: " + FloatToStr(ssd, 2));
                                            //
                                            // circular buffer index
                                            int spoof_results_cnt =
(spoofdet.sample_cnt % SPOOF_RESULTS_MAX);
                                            //
                                            // current result for the most
recent eye samples if (ssd < HUMAN_SPEC_XY_DELTA_MIN) {
                                                    // single flag
                                                    spoof_flag = true;
                                                    // or average a few results
                                                    liveDetection.spoof_results[spoof_results_cnt]
= 1;
                                            }
                                            else {
                                                    liveDetection.spoof_results[spoof_results_cnt]
= 0;
                                            }
                                            //
                                            // increment test cnt
                                            spoofdet.sample_cnt++;
                                            //
                                    }
                                    else {
                                            //Log(IntToStr(spoofdet.image_cnt) +
" *** bad eye");
                                    }
                                    //
                            }
                            //else Log(IntToStr(ei) + " no sequence");
                            //
                            // save sequence of values
                            liveDetection.prev_side[ei] =
liveDetection.side[ei]; liveDetection.prev_ex[ei] = liveDetection.ex[ei];
                            liveDetection.prev_ey[ei] = liveDetection.ey[ei];
                            liveDetection.prev_ez[ei] = liveDetection.ez[ei];
                            liveDetection.prev2_sx[ei] =
liveDetection.prev1_sx[ei]; liveDetection.prev2_sy[ei] =
liveDetection.prev1_sy[ei]; liveDetection.prev1_sx[ei] = liveDetection.sx[ei];
                            liveDetection.prev1_sy[ei] = liveDetection.sy[ei];
                            //
                            liveDetection.prev2_image_num[ei] =
liveDetection.prev1_image_num[ei]; liveDetection.prev1_image_num[ei] =
liveDetection.image_num[ei];
                            //
                    }
            }

            //-----------------------------------
            //
            // immediate current test results
            //

            //
            // set the last_human flag if all of the following conditions are
true:
            //   at least one iris has been found
            //   the test has completed
            //   the spoof flag is false
            //
            // otherwise, clear the last_human flag
            //
            //   -- MJR 20180831
            //
            spoofdet.last_human =
                    (irisfind.irisFound[0] || irisfind.irisFound[1]) &&
                    testComplete &&
                    !spoof_flag;
            std::cout << "last: " << (spoofdet.last_human ? "human" : "spoof") \
                    << "   if: " << (irisfind.irisFound[0] ? "0" : "-") << ":"
<< (irisfind.irisFound[1] ? "0" : "-") \
                    << "   fs: " << liveDetection.irled.firingSide \
                    << "   tc: " << (testComplete ? "T" : "F") \
                    << "   sf: " << (spoof_flag ? "S" : "H") \
                    << std::endl;


}*/

    // else clause is executed iff spoof/liveness detection is not enabled
    // else
    //{
    // std::cout << "liveness detection DISABLED - flagging template as human"
    // << std::endl; spoofdet.last_human = true;
    //}
  }
}

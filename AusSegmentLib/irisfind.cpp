
#include "irisfind.h"
#include <cstddef>
#include "compiler.h"
#include "eyelid.h"
#include "helper.h"
#include <stddef.h>

#include <math.h>  // for sqrtf()
#ifdef API_STAT
#include "statistic.h"
#endif

#include <iostream>

Irisfind::Irisfind(size_t eyecrop_width, size_t eyecrop_height, float gaze_radius_thresh)
:errorCnt(0)
,irisBrightness(0)
,pupilBrightness(0)
,pupilEdge(0)
,brightnessValid(false)
,focus_unfiltered(0.0f)
,gaze_radius_thresh(gaze_radius_thresh)
,headRotationError(false)
,bystanderError(false)
,m_Irisfind_min_Iris_Diameter(60)
,m_Irisfind_max_Iris_Diameter(180)
,m_Irisfind_min_pupil_Diameter(16)
,m_Irisfind_max_pupil_Diameter(60)  // 70, was 60 (faster), but missed eyes too close to camera
,m_Irisfind_min_spec_Diameter(8)
,m_Irisfind_max_spec_Diameter(20)
,maxIrisRadius(90)
,minIrisRadius(30)
,maxPupilRadius(30)
,minPupilRadius(8)
,maxSpecRadius(10)
,minSpecRadius(4)
{
	irisfind_image_size = eyecrop_width * eyecrop_height;

	hgradSpec = new int8[irisfind_image_size];

	lineptrHgradSpec = new PLINE[eyecrop_height];

	vgradSpec = new int8[irisfind_image_size];

	lineptrVgradSpec = new PLINE[eyecrop_height];

	hgradPupil = new int8[irisfind_image_size];

	lineptrHgradPupil = new PLINE[eyecrop_height];

	vgradPupil = new int8[irisfind_image_size];

	lineptrVgradPupil = new PLINE[eyecrop_height];

	hgradIris = new int8[irisfind_image_size];

	lineptrHgradIris = new PLINE[eyecrop_height];

	vgradIris = new int8[irisfind_image_size];

	lineptrVgradIris = new PLINE[eyecrop_height];

	hgrad = new int8[irisfind_image_size];

	lineptrHgrad = new PLINE[eyecrop_height];

	vgrad = new int8[irisfind_image_size];

	lineptrVgrad = new PLINE[eyecrop_height];

	bufDark = new uint8[irisfind_image_size];

	lineptrDark = new PLINE[eyecrop_height];

	bufMask = new uint8*[2];
	for(int i = 0; i < 2; ++i)
		bufMask[i] = new uint8[irisfind_image_size];

	lineptrMask = new PLINE*[2];
		for(int i = 0; i < 2; ++i)
			lineptrMask[i] = new PLINE[eyecrop_height];

}

Irisfind::~Irisfind()
{
	delete [] hgradSpec;
	delete [] lineptrHgradSpec;
	delete [] vgradSpec;
	delete [] lineptrVgradSpec;
	delete [] lineptrHgradPupil;
	delete [] vgradPupil;
	delete [] lineptrVgradPupil;
	delete [] hgradIris;
	delete [] lineptrHgradIris;
	delete [] vgradIris;
	delete [] lineptrVgradIris;
	delete [] hgrad;
	delete [] lineptrHgrad;
	delete [] vgrad;
	delete [] lineptrVgrad;
	delete [] bufDark;
	delete [] lineptrDark;

	for(int i = 0; i < 2; ++i) {
		delete [] bufMask[i];
	}
	delete [] bufMask;

	for(int i = 0; i < 2; ++i) {
		delete [] lineptrMask[i];
	}
	delete [] lineptrMask;
}

void Irisfind::SetIrisfind_Iris_Diameter(uint16 MinIrisDiameter, uint16 MaxIrisDiameter)
{
	m_Irisfind_min_Iris_Diameter = MinIrisDiameter;
	m_Irisfind_max_Iris_Diameter = MaxIrisDiameter;
}

void Irisfind::SetIrisfind_Pupil_Diameter(uint16 MinPupilDiameter, uint16 MaxPupilDiameter)
{
	m_Irisfind_min_pupil_Diameter = MinPupilDiameter;
	m_Irisfind_max_pupil_Diameter = MaxPupilDiameter;  // 70, was 60 (faster), but missed eyes too close to camera
}

void Irisfind::SetIrisfind_Spec_Diameter(uint16 MinSpecDiameter, uint16 MaxSpecDiameter)
{
	m_Irisfind_min_spec_Diameter = MinSpecDiameter;
	m_Irisfind_max_spec_Diameter = MaxSpecDiameter;
}

void Irisfind::ek_irisfind_session_init(size_t eyecrop_width, size_t eyecrop_height)
{
  // init
  findRedEye = false;
  autoRedEye = false;

  // set up buffers
  size_t eyecrop_image_size = (eyecrop_width * eyecrop_height);

  get_lineptrs_8((uint8*)hgradSpec, eyecrop_width, eyecrop_height, lineptrHgradSpec);
  get_lineptrs_8((uint8*)vgradSpec, eyecrop_width, eyecrop_height, lineptrVgradSpec);
  get_lineptrs_8((uint8*)hgradPupil, eyecrop_width, eyecrop_height, lineptrHgradPupil);
  get_lineptrs_8((uint8*)vgradPupil, eyecrop_width, eyecrop_height, lineptrVgradPupil);
  get_lineptrs_8((uint8*)hgradIris, eyecrop_width, eyecrop_height, lineptrHgradIris);
  get_lineptrs_8((uint8*)vgradIris, eyecrop_width, eyecrop_height, lineptrVgradIris);

  get_lineptrs_8((uint8*)hgrad, eyecrop_width, eyecrop_height, lineptrHgrad);
  get_lineptrs_8((uint8*)vgrad, eyecrop_width, eyecrop_height, lineptrVgrad);

  get_lineptrs_8((uint8*)bufDark, eyecrop_width, eyecrop_height, lineptrDark);

  //--------------------------------------------------------------
  //
  // init mask
  //

  for (uint8 eye = 0; eye < 2; eye++) {
    get_lineptrs_8(bufMask[eye], eyecrop_width, eyecrop_height, lineptrMask[eye]);
    // clear mask buffer
    for (uint32 i = 0; i < eyecrop_image_size; i++)
    	bufMask[eye][i] = 0;
    // for erasing previous mask instead of clearing whole buffer
    // init a previous dummy spec
    prev_specPos[eye].x = (float32)(eyecrop_width / 2);  // arbitrary coordinate
    prev_specPos[eye].y = (float32)(eyecrop_height / 2);  // arbitrary coordinate
    prev_specPos[eye].z = 2;    // arbitrary small number
  }

  //--------------------------------------------------------------
  //
  // init circle templates
  //

  maxLutCnt = IRISFIND_MAX_RADIUS_CNT;
  maxLutSize = IRISFIND_FULLCIRCLE_MAX_LUT_SIZE;

  // init once per session, not init per scale change
  // number of circle templates
  // use index i (LUT[i]) to be the same as radius = i
  // so that circle index = radius
  lutCnt = maxLutCnt;
  if (lutCnt > maxLutCnt)
	  lutCnt = maxLutCnt;

  init_circles(lutCnt);
  //
  //--------------------------------------------------------------
  //
  // init history lists
  //

  // irisfind.pupilHistory.goodHistory = false;
}

//
// return size of LUT
uint32 make_circle_lut(uint32 radius, uint32 lutMaxSize,
                       struct PointXY_signed* pos,
                       uint32(cfunc)(uint32, uint32, struct PointXY_signed*,
                                     uint32)) {
  uint32 x = 0;
  uint32 y = radius;
  float32 p = (float32)((5.0 - ((float32)radius * 4.0)) / 4.0);
  uint32 cnt = 0;
  cnt = cfunc(x, y, pos, cnt);
  while (x < y) {
    x++;
    if (p < 0.0) {
      p += (float32)((2.0 * (float32)x) + 1.0);
    } else {
      y--;
      p += (float32)(2.0 * ((float32)x - (float32)y) + 1.0);
    }
    // old test for a different circle routine to prevent LUT overrun
    if ((cnt + 8) < lutMaxSize) {
      cnt = cfunc(x, y, pos, cnt);
    }
  }
  return cnt;
}

//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// Create circle coordinate LUTs
//
//
/*
See notes in init_circles() for circles LUT allocated size in bytes
*/
//
uint32 circlePoints_topRight(uint32 x, uint32 y, struct PointXY_signed* pos,
                             uint32 i) {
  if (x == 0) {
    // pos[i].x = 0; pos[i].y = -y; i++;
  } else if (x < y) {
    pos[i].x = (int16_t)x;
    pos[i].y = -(int16)y;
    i++;
  }
  return i;
}
//

uint32 circlePoints_rightBottom(uint32 x, uint32 y, struct PointXY_signed* pos,
                                uint32 i) {
  if (x == 0) {
    pos[i].x = y;
    pos[i].y = x;
    i++;
  } else if (x < y) {
    pos[i].x = y;
    pos[i].y = x;
    i++;
  }
  return i;
}

//
void reverse_order_and_extend_lut(uint32 lutSize, struct PointXY_signed* pos,
                                  uint16* tempbuf) {
  uint32 i;
  for (i = 0; i < lutSize; i++) {
    tempbuf[i] = pos[i].x;
  }
  for (i = 0; i < lutSize; i++) {
    pos[i].x = tempbuf[lutSize - i - 1];
    pos[i + lutSize].x = tempbuf[i];
  }
  for (i = 0; i < lutSize; i++) {
    tempbuf[i] = pos[i].y;
  }
  for (i = 0; i < lutSize; i++) {
    pos[i].y = -tempbuf[lutSize - i - 1];
    pos[i + lutSize].y = tempbuf[i];
  }
}


//
void Irisfind::init_circles(uint16 lutCnt) {
  uint32 i, j;
  uint32 memsize = 0;
  uint32 allocated = 0;
  // radius index i same as radius
  // radius = i
  // circle[i] contains LUT for radius i circle
  //
  /*
  // pupil and iris arcs
  for(i = 0;i < lutCnt;i++){
      irisfind.circle[i].radius = i;
      //
      // left and right arcs
      // horizontal gradient
      // hArc, 1/8 circle
      irisfind.circle[i].hArcSize = make_circle_lut(irisfind.circle[i].radius,
  irisfind.maxLutSize >> 2, irisfind.circle[i].hArc, circlePoints_horizontal);
      //Log(IntToStr(i) + ": " + IntToStr(irisfind.circle[i].hArcSize));
      memsize += irisfind.circle[i].hArcSize;
      allocated += irisfind.maxLutSize >> 3;
      //
      // top and bottom arcs
      // vertical gradient
      // vArc, 1/8 circle
      irisfind.circle[i].vArcSize = make_circle_lut(irisfind.circle[i].radius,
  irisfind.maxLutSize >> 2, irisfind.circle[i].vArc, circlePoints_vertical);
      //Log(IntToStr(i) + ": " + IntToStr(irisfind.circle[i].vArcSize));
      memsize += irisfind.circle[i].vArcSize;
      allocated += irisfind.maxLutSize >> 3;
      // reverse order for easy weighting from horizontal center out
      reverse_order_lut(irisfind.circle[i].vArcSize, irisfind.circle[i].vArc,
  irisfind.tempLUTbuf16);
  }
  Log("required memsize hv arcs: " + IntToStr(memsize));
  Log("required memsize hv arcs in bytes: " + IntToStr(memsize * 4));
  */
  //
  // eyelid arcs
  //
  // right half of circle
  // ordered from top to bottom
  memsize = 0;
  for (i = 0; i < lutCnt; i++) {
    circle[i].radius = i;
    //
    // top right arc
    // vTopRight, 1/8 circle
    circle[i].vTopRightSize =
        make_circle_lut(circle[i].radius, maxLutSize >> 3,
                        circle[i].vTopRight, circlePoints_topRight);
    // Log(IntToStr(i) + ": " + IntToStr(irisfind.circle[i].vTopRightSize));
    memsize += circle[i].vTopRightSize;
    allocated += maxLutSize >> 3;
    //
    // right arc
    // hRight, 1/4 circle
    circle[i].hRightSize =
        make_circle_lut(circle[i].radius, maxLutSize >> 3,
                        circle[i].hRight, circlePoints_rightBottom);
    // reverse order for easy filtering from top down
    reverse_order_and_extend_lut(circle[i].hRightSize,
                                 circle[i].hRight,
                                 tempLUTbuf16);
    circle[i].hRightSize *= 2;
    // Log(IntToStr(i) + ": " + IntToStr(irisfind.circle[i].hRightSize));
    memsize += circle[i].hRightSize;
    allocated += maxLutSize >> 2;
    //
    // bottom right arc
    // vBottomRight, 1/8 circle
    uint16 size = circle[i].vTopRightSize;
    circle[i].vBottomRightSize = size;
    for (j = 0; j < circle[i].vBottomRightSize; j++) {
      size--;
      circle[i].vBottomRight[j].x =
          circle[i].vTopRight[size].x;
      circle[i].vBottomRight[j].y =
          -circle[i].vTopRight[size].y;
    }
    // Log(IntToStr(i) + ": " + IntToStr(irisfind.circle[i].vBottomRightSize));
    memsize += circle[i].vBottomRightSize;
    allocated += maxLutSize >> 3;
  }
  // Log("required memsize eyelid arcs: " + IntToStr(memsize));
  // Log("required memsize eyelid arcs in bytes: " + IntToStr(memsize * 4));
  //
  /*
  // left half of circle
  // ordered from top to bottom
  for(i = 0;i < lutCnt;i++){
      irisfind.circle[i].radius = i;
      //
      // top left arc
      // vTopLeft, 1/8 circle
      irisfind.circle[i].vTopLeftSize = irisfind.circle[i].vTopRightSize;
      for(j = 0;j < irisfind.circle[i].vTopLeftSize;j++){
          irisfind.circle[i].vTopLeft[j].x = -irisfind.circle[i].vTopRight[j].x;
          irisfind.circle[i].vTopLeft[j].y = irisfind.circle[i].vTopRight[j].y;
      }
      //Log(IntToStr(i) + ": " + IntToStr(irisfind.circle[i].vTopLeftSize));
      memsize += irisfind.circle[i].vTopLeftSize;
      allocated += irisfind.maxLutSize >> 3;
      //
      // left arc
      // hLeft, 1/4 circle
      irisfind.circle[i].hLeftSize = irisfind.circle[i].hRightSize;
      for(j = 0;j < irisfind.circle[i].hLeftSize;j++){
          irisfind.circle[i].hLeft[j].x = -irisfind.circle[i].hRight[j].x;
          irisfind.circle[i].hLeft[j].y = irisfind.circle[i].hRight[j].y;
      }
      //Log(IntToStr(i) + ": " + IntToStr(irisfind.circle[i].hLeftSize));
      memsize += irisfind.circle[i].hLeftSize;
      allocated += irisfind.maxLutSize >> 2;
      //
      // bottom left arc
      // vBottomLeft, 1/8 circle
      irisfind.circle[i].vBottomLeftSize = irisfind.circle[i].vBottomRightSize;
      for(j = 0;j < irisfind.circle[i].vBottomLeftSize;j++){
          irisfind.circle[i].vBottomLeft[j].x =
  -irisfind.circle[i].vBottomRight[j].x; irisfind.circle[i].vBottomLeft[j].y =
  irisfind.circle[i].vBottomRight[j].y;
      }
      //Log(IntToStr(i) + ": " + IntToStr(irisfind.circle[i].vBottomLeftSize));
      memsize += irisfind.circle[i].vBottomLeftSize;
      allocated += irisfind.maxLutSize >> 3;
  }
  */
  /*
  required memsize eyelid arcs: 14144
  required memsize eyelid arcs in bytes: 56576
  allocated circle LUTs in bytes: 121604
  */
  // Log("required memsize eyelid arcs: " + IntToStr(memsize));
  // Log("required memsize eyelid arcs in bytes: " + IntToStr(memsize * 4));
  // Log("allocated circle LUTs in bytes: " + IntToStr(allocated * 4));
}

//
void Irisfind::ek_irisfind_init()
{
  uint32 i;
  errorCnt = 0;

  irisBrightness = 0;
  pupilBrightness = 0;
  pupilBrightnessLR[0] = 0;
  pupilBrightnessLR[1] = 0;
  pupilEdge = 0;
  brightnessValid = false;
  focus_unfiltered = 0.0f;
  headRotationError = false;
  bystanderError = false;

  for (i = 0; i <= 1; i++) {
    specFound[i] = false;
    darkFound[i] = false;
    pupilFound[i] = false;
    irisFound[i] = false;

    darkScore[i] = 0;
    specScore[i] = 0;
    pupilScore[i] = 0;
    irisScore[i] = 0;

    faceDistance[i] = 0;
    m_fpupilToIrisRatio[i] = 0;

    glasses_lowEyelid_detected[i] = 0;

    gaze[i].x = 0.0;
    gaze[i].y = 0.0;
    gaze[i].z = 0.0;
    good_gaze[i] = true;
  }

  // clear mask buffer
  // for(uint32 i = 0;i < IRISFIND_IMAGE_SIZE;i++) irisfind.mask[eye][i] =  0;

  // settings

  // irisfind.maskThresh = IRISFIND_MASK_THRESH;

  // TBD: adjust with contrast and lighting

  maxSpecGrad = IRISFIND_MAX_SPEC_GRADIENT;
  minSpecGrad = IRISFIND_MIN_SPEC_GRADIENT;
  maxPupilGrad = IRISFIND_MAX_PUPIL_GRADIENT;
  minPupilGrad = IRISFIND_MIN_PUPIL_GRADIENT;
  maxRedPupilGrad = IRISFIND_MAX_REDPUPIL_GRADIENT;
  minRedPupilGrad = IRISFIND_MIN_REDPUPIL_GRADIENT;
  maxIrisGrad = IRISFIND_MAX_IRIS_GRADIENT;
  minIrisGrad = IRISFIND_MIN_IRIS_GRADIENT;

  minScoreVariance = MIN_SCORE_VARIANCE;

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


  // radius is used in the algorithm, not diameter
  maxIrisRadius = (m_Irisfind_max_Iris_Diameter >> 1);
  minIrisRadius = (m_Irisfind_min_Iris_Diameter >> 1);

  // irisfind.searchBoxPupil = 10;
  maxPupilRadius = (m_Irisfind_max_pupil_Diameter >> 1);
  minPupilRadius = (m_Irisfind_min_pupil_Diameter >> 1);

  maxSpecRadius = (m_Irisfind_max_spec_Diameter >> 1);
  minSpecRadius = (m_Irisfind_min_spec_Diameter >> 1);

  m_Irisfind_max_pupil_radius = (m_Irisfind_max_pupil_Diameter >> 1);

  // TBD
  // irisfind.searchBoxIris = irisfind.maxIrisRadius;
  // irisfind.searchBoxPupil = irisfind.maxPupilRadius;

  // TBD: adjust with zoom and noise

  // irisfind.maxCircleNoise_pupil =  IRISFIND_MAX_CIRCLE_NOISE_PUPIL;
  // irisfind.maxCircleNoise_iris =  IRISFIND_MAX_CIRCLE_NOISE_IRIS;

  // TBD: adjust with zoom or size of spec

  // adjust the size of the mask so that it is larger than the actual spec
  specMaskMultiplier = IRISFIND_SPECMASK_MULTIPLIER;
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
void Irisfind::spec_search(uint8 eyeCropIndex, size_t eyecrop_width, size_t eyecrop_height)
{
  // search type
  bool dark = false;
  bool ccl_dark = false;
  bool spec = true;
  bool pupil = false;
  bool iris = false;
  // return search winner circle (x, y, z)
  struct PointXYZ_float* wpos = &specPos[eyeCropIndex];
  //---------------------------------------------------
  // search area
  // TBD: the following parameters could change with distance
  // set min and max radius
  uint16 minRadius = minSpecRadius;
  uint16 maxRadius = maxSpecRadius;
  // set gradient min and max
  int8 minGrad = minSpecGrad;
  int8 maxGrad = maxSpecGrad;
  // set center of search area (xc, yc)
  int16 xc = eyecrop_width >> 1;
  int16 yc = eyecrop_height >> 1;
  // set search box size
  int16 searchBox = IRISFIND_SPEC_SEARCHBOX;
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // set gradient of eye crop
  PLINE* hGradLptr = lineptrHgrad;
  PLINE* vGradLptr = lineptrVgrad;
  PLINE* mLptr = NULL;
  PLINE* dLptr = lineptrDark;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = circle;
  // search the gradient for circles
  search_for_circles(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                     circleR, minRadius, maxRadius, eyecrop_width, eyecrop_height, xc, yc,
                     searchBox, wpos, dark, ccl_dark, spec, pupil, iris, m_Irisfind_max_pupil_radius);
  //
  // output results to use in pupil search
  // return search winner circle (x, y, z)
  uint16 winnerRadius = (uint16)wpos->z;  // search winner (x, y, z)
  specScore[eyeCropIndex] = wpos->s;
  if (winnerRadius > 0)
	  specFound[eyeCropIndex] = true;  // TBD: how to test for good spec?
  if (specScore[eyeCropIndex] < IRISFIND_SPEC_MIN_SCORE)
	  specFound[eyeCropIndex] = false;
}
//
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// dark search for rough pupil area (dark pixel values) detection
//
//
void Irisfind::dark_search(uint8 eyeCropIndex, size_t eyecrop_width, size_t eyecrop_height)
{
  // search type
  bool dark = true;
  bool ccl_dark = false;
  bool spec = false;
  bool pupil = false;
  bool iris = false;
  // return search winner circle (x, y, z)
  struct PointXYZ_float* wpos = &darkPos[eyeCropIndex];
  //---------------------------------------------------
  // search area
  // TBD: the following parameters could change with distance
  // set min and max radius
  uint16 minRadius = minPupilRadius;
  uint16 maxRadius = maxPupilRadius;
  // set gradient min and max
  int8 minGrad = 0;
  int8 maxGrad = 0;

  // set center of search area (xc, yc)
  int16 xc = (int16)specPos[eyeCropIndex].x;
  int16 yc = (int16)specPos[eyeCropIndex].y;
  // set search box size
  // Dark searchBox will be calculated in search_for_circles(); in
  // ek_irisfind_circles.cpp
  int16 searchBox = (int16)specPos[eyeCropIndex].z;  // spec radius used later to make dark searchbox

  // set center of search area (xc, yc)
  // int16 xc = irisfind.width >> 1;
  // int16 yc = irisfind.height >> 1;
  // set search box size
  // int16 searchBox = IRISFIND_DARK_SEARCHBOX;
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // set gradient of eye crop
  PLINE* hGradLptr = NULL;
  PLINE* vGradLptr = NULL;
  PLINE* mLptr = NULL;                  // spec mask ptr
  PLINE* dLptr = lineptrDark;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = circle;
  // search the gradient for circles
  search_for_circles(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                     circleR, minRadius, maxRadius, eyecrop_width, eyecrop_height, xc, yc,
                     searchBox, wpos, dark, ccl_dark, spec, pupil, iris, m_Irisfind_max_pupil_radius);
  //
  // output results to use in pupil search
  // return search winner circle (x, y, z)
  uint16 winnerRadius = (uint16)wpos->z;  // search winner (x, y, z)
  // arbitrary multipler make dark score range similar to pupil and iris range
  darkScore[eyeCropIndex] = wpos->s * DARK_SCORE_ARBITRARY_MULTIPLIER;
  if (winnerRadius > 0)
	  darkFound[eyeCropIndex] = true;  // TBD: how to test for good spec?
  if (darkScore[eyeCropIndex] < IRISFIND_DARK_MIN_SCORE)
	  darkFound[eyeCropIndex] = false;
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// pupil search
//
//
void Irisfind::pupil_search(uint8 eyeCropIndex, size_t eyecrop_width, size_t eyecrop_height)
{
  // search type
  bool dark = false;
  bool ccl_dark = false;
  bool spec = false;
  bool pupil = true;
  bool iris = false;
  // return search winner circle (x, y, z)
  struct PointXYZ_float* wpos = &pupilPos[eyeCropIndex];
  // printf(" pupil_search %f\n", wpos->z);

  //---------------------------------------------------
  // search area
  // TBD: the following parameters could change with distance
  // set min and max radius
  // uint16 minRadius = irisfind.minPupilRadius;
  // uint16 maxRadius = irisfind.maxPupilRadius;
  //
  // calculate search area from dark location
  //
  uint16 darkRadius = (uint16)darkPos[eyeCropIndex].z;
  uint16 minRadius = (uint16)((float32)darkRadius * 0.8);
  uint16 maxRadius = (uint16)((float32)darkRadius * 1.25);

  // set center of search area (xc, yc)
  int16 xc = (int16)darkPos[eyeCropIndex].x;
  int16 yc = (int16)darkPos[eyeCropIndex].y;
  // set search box size
  int16 searchBox = 2;  //(float32)darkRadius * 0.5;

  //
  // set gradient min and max
  int8 minGrad = minPupilGrad;
  int8 maxGrad = maxPupilGrad;
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
  // set gradient of eye crop
  PLINE* hGradLptr = lineptrHgrad;
  PLINE* vGradLptr = lineptrVgrad;
  PLINE* mLptr = lineptrMask[eyeCropIndex];
  PLINE* dLptr = NULL;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = circle;
  // search the gradient for circles
  search_for_circles(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                     circleR, minRadius, maxRadius, eyecrop_width, eyecrop_height, xc, yc,
                     searchBox, wpos, dark, ccl_dark, spec, pupil, iris, m_Irisfind_max_pupil_radius);
  //
  // output results to use in iris search
  // return search winner circle (x, y, z)
  uint16 winnerRadius = (uint16)wpos->z;  // search winner (x, y, z)
  pupilScore[eyeCropIndex] = wpos->s;
  if (winnerRadius > 0)
	  pupilFound[eyeCropIndex] =
        true;  // TBD: how to test for good pupil?
  if (pupilScore[eyeCropIndex] < IRISFIND_PUPIL_MIN_SCORE)
    pupilFound[eyeCropIndex] = false;
}
//
//-------------------
//
TemplatePipelineError Irisfind::pupil_search_subpixel(uint8 eyeCropIndex, size_t eyecrop_width, size_t eyecrop_height)
{
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
  struct PointXYZ_float* wpos = &pupilPos[eyeCropIndex];

  // printf("pupil_search_subpixel...%f\n",  wpos->z);
  // set gradient min and max
  int8 minGrad = minPupilGrad;
  int8 maxGrad = maxPupilGrad;
  //---------------------------------------------------
  //
  // calculate search area from search winner
  //
  // check circle radius against xy boundary
  // detect parameters that would cause errors
  // do not process if already at radius limit
  // small radius
  if (wpos->z <= 1)
	  return TemplatePipelineError::Pupil_Diameter_out_of_range;  // out of bounds
  // large radius
  if (wpos->z >= IRISFIND_CIRCLE_MAX_RADIUS)
	  return TemplatePipelineError::Pupil_Diameter_out_of_range;  // out of bounds
  // radius ok
  uint16 centerRadius = (uint16)wpos->z;
  uint16 maxRadius = (uint16)wpos->z + 1;
  // set center of search area (xc, yc)
  int16 xc = (int16)wpos->x;
  int16 yc = (int16)wpos->y;
  // search inputs
  // the following are the same for spec, pupil, iris
   // set search box size
  int16 searchBox = 3;
  // test for out-of-bounds
  int16 xmin, xmax, ymin, ymax;  // not used
  uint8 inbounds = make_xy_search_area(eyecrop_width, eyecrop_height, xc, yc, maxRadius,
                                       searchBox, &xmin, &xmax, &ymin, &ymax);
  if (inbounds == 0)
	  return TemplatePipelineError::Pupil_Diameter_out_of_range;  // out of bounds
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // set gradient of eye crop
  PLINE* hGradLptr = lineptrHgrad;
  PLINE* vGradLptr = lineptrVgrad;
  PLINE* mLptr = lineptrMask[eyeCropIndex];
  PLINE* dLptr = NULL;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = circle;
  // set results return struct
  struct ScoreXYZS(*sCube)[3][3][3] = &scoreCube;
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
  return TemplatePipelineError::Segmentation_Successful;  // good subpixel
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// iris search
//
//
TemplatePipelineError Irisfind::iris_search(uint8 eyeCropIndex, size_t eyecrop_width, size_t eyecrop_height) {
  // search type
  bool dark = false;
  bool ccl_dark = false;
  bool spec = false;
  bool pupil = false;
  bool iris = true;
  // return search winner circle (x, y, z)
  struct PointXYZ_float* wpos = &irisPos[eyeCropIndex];

  // printf("iris_search...%f\n",  wpos->z);
  //---------------------------------------------------
  // search area
  // TBD: the following parameters could change with distance
  // set min and max radius
  // uint16 minRadius = irisfind.minIrisRadius;
  // uint16 maxRadius = irisfind.maxIrisRadius;
  uint16 minRadius = (uint16)((float32)pupilPos[eyeCropIndex].z *
                              IRISFIND__PUPIL_TO_IRIS__MIN_MULTIPLIER);
  uint16 maxRadius = (uint16)((float32)pupilPos[eyeCropIndex].z *
                              IRISFIND__PUPIL_TO_IRIS__MAX_MULTIPLIER);
  if ((minRadius > maxIrisRadius) ||
      (minRadius < minIrisRadius))
    minRadius = minIrisRadius;
  if ((maxRadius > maxIrisRadius) ||
      (maxRadius < minIrisRadius))
    maxRadius = maxIrisRadius;
  // set gradient min and max
  int8 minGrad = minIrisGrad;
  int8 maxGrad = maxIrisGrad;
  // set center of search area (xc, yc)
  // int16 xc = irisfind.width >> 1;
  // int16 yc = irisfind.height >> 1;
  int16 xc = (int16)pupilPos[eyeCropIndex].x;
  int16 yc = (int16)pupilPos[eyeCropIndex].y;
  // set search box size
  // int16 searchBox = IRISFIND_IRIS_SEARCHBOX;
  // For iris, set (max searchBox) here.
  // Iris searchBox will be calculated in search_for_circles(); in
  // ek_irisfind_circles.cpp TBD: relate iris offset to (distance to face) and
  // pupil/iris size ratio
  int16 searchBox =
      (int16)pupilPos[eyeCropIndex].z;  //* 0.5;// *
               //IRISFIND__PUPIL_RADIUS_MULTIPLIER_TO_MAKE_IRIS_MAX_SEARCHBOX;
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // eyecrop width and height
  //int16 width = m_width;
  //int16 height = m_height;
  // set gradient of eye crop
  PLINE* hGradLptr = lineptrHgrad;
  PLINE* vGradLptr = lineptrVgrad;
  PLINE* mLptr = lineptrMask[eyeCropIndex];
  PLINE* dLptr = NULL;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = circle;
  // search the gradient for circles
  search_for_circles(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                     circleR, minRadius, maxRadius, eyecrop_width, eyecrop_height, xc, yc,
                     searchBox, wpos, dark, ccl_dark, spec, pupil, iris, m_Irisfind_max_pupil_radius);
  //
  // return search winner circle (x, y, z)
  uint16 winnerRadius = (uint16)wpos->z;  // search winner (x, y, z)
  irisScore[eyeCropIndex] = wpos->s;
  if (winnerRadius > 0)
    irisFound[eyeCropIndex] = true;  // TBD: how to test for good iris?
  if (irisScore[eyeCropIndex] < IRISFIND_IRIS_MIN_SCORE)
    irisFound[eyeCropIndex] = false;

  return TemplatePipelineError::Segmentation_Successful;
}
//
//-------------------
//
TemplatePipelineError Irisfind::iris_search_subpixel(uint8 eyeCropIndex, size_t eyecrop_width, size_t eyecrop_height) {
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
  struct PointXYZ_float* wpos = &irisPos[eyeCropIndex];

  // printf("iris_search_subpixel...%f\n",  wpos->z);
  // set gradient min and max
  int8 minGrad = minIrisGrad;
  int8 maxGrad = maxIrisGrad;
  //---------------------------------------------------
  //
  // calculate search area from search winner
  //
  // check circle radius against xy boundary
  // detect parameters that would cause errors
  // do not process if already at radius limit
  // small radius
  if (wpos->z <= 1)
	  return TemplatePipelineError::Iris_Diameter_out_of_range;  // out of bounds
  // large radius
  if (wpos->z >= IRISFIND_CIRCLE_MAX_RADIUS)
	  return TemplatePipelineError::Iris_Diameter_out_of_range;  // out of bounds
  // radius ok
  uint16 centerRadius = (uint16)wpos->z;
  uint16 maxRadius = (uint16)wpos->z + 1;
  // set center of search area (xc, yc)
  int16 xc = (int16)wpos->x;
  int16 yc = (int16)wpos->y;
  // search inputs
  // the following are the same for spec, pupil, iris
  // set search box size
  int16 searchBox = 3;
  // test for out-of-bounds
  int16 xmin, xmax, ymin, ymax;  // not used
  uint8 inbounds = make_xy_search_area(eyecrop_width, eyecrop_height, xc, yc, maxRadius,
                                       searchBox, &xmin, &xmax, &ymin, &ymax);
  if (inbounds == 0)
	  return TemplatePipelineError::Iris_Diameter_out_of_range;  // out of bounds
  //---------------------------------------------------
  // search inputs
  // the following are the same for spec, pupil, iris
  // set gradient of eye crop
  PLINE* hGradLptr = lineptrHgrad;
  PLINE* vGradLptr = lineptrVgrad;
  PLINE* mLptr = lineptrMask[eyeCropIndex];
  PLINE* dLptr = NULL;  // dark ptr
  // set circle LUTs
  struct Circle_struct* circleR = circle;
  // set results return struct
  struct ScoreXYZS(*sCube)[3][3][3] = &scoreCube;
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
  return TemplatePipelineError::Segmentation_Successful;  // good subpixel
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

TemplatePipelineError Irisfind::ek_irisfind_main(PLINE* line_ptr_eyecrop, PLINE* line_ptr_flat_iris,
                      size_t eyecrop_width,
                      size_t eyecrop_height, size_t flat_iris_width,
                      size_t flat_iris_height, IrisFindParameters& IrisPupilParams)

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
                           lineptrDark,
                           &pupilBrightnessLR[0]);

  // test using spec to guide dark circle search
  specPos[0].x = (float)(xc);
  specPos[0].y = (float)(yc);
  specPos[0].z = 8;

  dark_search(0, eyecrop_width, eyecrop_height);

  if (darkFound[0]) {
    // make gradients of eye crop
    hor_gradient(line_ptr_eyecrop, eyecrop_width, eyecrop_height, lineptrHgrad);
    vert_gradient(line_ptr_eyecrop, eyecrop_width, eyecrop_height, lineptrVgrad);

    //
    // search for specularity
    //

    spec_search(0, eyecrop_width, eyecrop_height);  // TBD: bad method if spec is not round or single.

    // if(!irisfind.specFound[eyeCropIndex]) // TBD: what to do here?
    // clear old spec mask, make new spec mask
    // TBD: fix the order of parameters in call
    make_spec_mask(lineptrMask[0], eyecrop_width, eyecrop_height,
                   &prev_specPos[0], &specPos[0],
                   specMaskMultiplier, specFound[0]);
    //
    // search for pupil and iris
    //
    //
    // TBD: note that segmentation is allowed to continue if spec search fails
    //
    if (specFound[0]) {
      pupil_search(0, eyecrop_width, eyecrop_height);
      if (pupilFound[0]) {
        iris_search(0, eyecrop_width, eyecrop_height);
      }else{
      	return TemplatePipelineError::Pupil_Iris_Boundary_Not_Found;
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
    if (irisFound[0]) {
      pupil_search_subpixel(0, eyecrop_width, eyecrop_height);

      iris_search_subpixel(0, eyecrop_width, eyecrop_height);
    }else{
    	return TemplatePipelineError::Iris_Sclera_Boundary_Not_Found;
    }

    // Get the values for sorting
	//  printf("irisPos %f %f %f\n", irisPos[0].x, irisPos[0].y, irisPos[0].z);
	IrisPupilParams.ip.x = irisPos[0].x;
	IrisPupilParams.ip.y = irisPos[0].y;
	IrisPupilParams.ip.r = irisPos[0].z;

	IrisPupilParams.pp.x = pupilPos[0].x;
	IrisPupilParams.pp.y = pupilPos[0].y;
	IrisPupilParams.pp.r = pupilPos[0].z;

    //------------------------------------------------------------------------------------
    //
    // face distance: from iris diameter
    // pupil dilation: ratio of pupil radius to iris radius
    // gaze: compare pupil location to spec location
    //
    if (irisFound[0]) {
      // possible face distance in cm from camera for candidate eyes
      faceDistance[0] =
          irisfind_distanceToFace_cm(irisPos[0].z);
      // TODO: REPLACE WITH DISTANCE CLASS
      /*if (irisfind.faceDistance[0] > eyefind.maxFaceDistance) {
              eyefind.tooFar = true;
      }
      else if (irisfind.faceDistance[0] > 0.0 &&
              irisfind.faceDistance[0] < eyefind.minFaceDistance) {
              eyefind.tooClose = true;
}*/

      // pupil radius to iris radius, (pupil/iris) ratio for candidate eyes
      m_fpupilToIrisRatio[0] = pupilToIrisRatio(pupilPos[0].z, irisPos[0].z);

      if(m_fpupilToIrisRatio[0] < 0.40){
    	  // return IRISERROR::Percentage_Of_Iris_Visible_too_small; // Ignore for Authentication
      }

      // gaze: compare pupil location to spec location
      gaze[0].x = pupilPos[0].x - specPos[0].x;
      gaze[0].y = pupilPos[0].y - specPos[0].y;
      gaze[0].z =
          sqrtf(pow(gaze[0].x, 2) + pow(gaze[0].y, 2));

      good_gaze[0] = (gaze[0].z < gaze_radius_thresh);

      // Anita for logging
      IrisPupilParams.PupilToIrisRatio = m_fpupilToIrisRatio[0] ;
      IrisPupilParams.GazeVal = gaze[0].z;
      IrisPupilParams.IrisRadius = irisPos[0].z;
      IrisPupilParams.PupilRadius = pupilPos[0].z;
      IrisPupilParams.darkScore = darkScore[0];
      IrisPupilParams.SpecScore = specScore[0];
      IrisPupilParams.IrisScore = irisScore[0];
      IrisPupilParams.PupilScore = pupilScore[0];

      // printf("gaze....%f\n", gaze_radius_thresh);

      if(gaze[0].z > gaze_radius_thresh)
    	  return TemplatePipelineError::Gaze_out_of_range;

      // Anita - Check for pupil and Iris Diameters
     /* if(!(irisPos[0].z > (m_Irisfind_min_Iris_Diameter >> 1) && irisPos[0].z < (m_Irisfind_max_Iris_Diameter  >> 1)))
    	  return TemplatePipelineError::Iris_Diameter_out_of_range;

      if(!(pupilPos[0].z > (m_Irisfind_min_pupil_Diameter  >> 1) && pupilPos[0].z  < (m_Irisfind_max_pupil_Diameter >> 1)))
    	  return TemplatePipelineError::Pupil_Diameter_out_of_range;*/

    }else{
    	return TemplatePipelineError::Iris_Sclera_Boundary_Not_Found;
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
  if (irisFound[0]) {
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
  return TemplatePipelineError::Segmentation_Successful;
}



